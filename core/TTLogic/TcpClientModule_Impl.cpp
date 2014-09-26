/******************************************************************************* 
 *  @file      TcpClientModule_Impl.cpp 2014\7\29 13:16:48 $
 *  @author    快刀<kuaidao@mogujie.com>
 *  @brief     
 ******************************************************************************/

#include "stdafx.h"
#include "TcpClientModule_Impl.h"
#include "TTLogic/TcpClientSocket.h"
#include "src/base/ImPduClient.h"
#include "utility/utilStrCodeAPI.h"

/******************************************************************************/
NAMESPACE_BEGIN(logic)

ITcpClientModule* getTcpClientModule()
{
	return (ITcpClientModule*)logic::GetLogic()->getModule(MODULE_ID_TCPCLIENT);
}

namespace
{
	UInt16 g_seqNum = 0;
}

// -----------------------------------------------------------------------------
//  TcpClientModule_Impl: Public, Constructor

TcpClientModule_Impl::TcpClientModule_Impl()
:m_tcpClientState(TCPCLIENT_STATE_OK)
,m_pCurrSeqImPdu(0)
,m_pHearbeatTimer(0)
,m_pServerPingTimer(0)
,m_bDoReloginServerNow(FALSE)
{
	m_pClientSocket = new TcpClientSocket;
	m_pClientSocket->registerCB(this);

	m_currSeqEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	m_eventConnected = CreateEvent(NULL, FALSE, FALSE, NULL);
}

// -----------------------------------------------------------------------------
//  TcpClientModule_Impl: Public, Destructor

TcpClientModule_Impl::~TcpClientModule_Impl()
{
	m_pClientSocket->unRegisterCB();

	delete m_pClientSocket;
	m_pClientSocket = 0;

	if (m_pCurrSeqImPdu)
		delete m_pCurrSeqImPdu;
	m_pCurrSeqImPdu = 0;
	CloseHandle(m_eventConnected);
	CloseHandle(m_currSeqEvent);
}

void TcpClientModule_Impl::release()
{
	delete this;
}

void TcpClientModule_Impl::onClose(int error)
{
	APP_LOG(LOG_ERROR, _T("TcpClientModule_Impl socket onClose(%d)"), error);
	m_tcpClientState = TCPCLIENT_STATE_DISCONNECT;
	GetLogic()->asynNotifyObserver(KEY_TCPCLIENT_STATE);
	_stopHearbeat();
}

void TcpClientModule_Impl::onReceiveData(const char* data, UInt32 size)
{
	if (m_pServerPingTimer)
		m_pServerPingTimer->m_bHasReceivedPing = TRUE;

	PduHeader_t pduHead;
	CImPdu::ReadPduHeader((uchar_t*)data,(uint32_t)size, &pduHead);
	if (IM_PDU_TYPE_HEARTBEAT == pduHead.command_id && SID_OTHER == pduHead.module_id)
	{
		//模块器端过来的心跳包，不跳到业务层派发
		return;
	}

	if (g_seqNum == pduHead.reserved)
	{
		try
		{
			m_pCurrSeqImPdu = CImPdu::ReadPdu((uchar_t*)data, (uint32_t)size);
		}
		catch (CPduException e)
		{
			APP_LOG(LOG_ERROR, TRUE, _T("Logic_Impl onPacket CPduException serviceId:%d,commandId:%d,errCode:%d")
				, e.GetModuleId(), e.GetCommandId(), e.GetErrorCode());
			return;
		}
		catch (...)
		{
			APP_LOG(LOG_ERROR, TRUE, _T("Logic_Impl onPacket unknown exception"));
			return;
		}

		::SetEvent(m_currSeqEvent);
		return;
	}

	//将网络包包装成任务放到逻辑任务队列里面去
	_handlePacketOperation(data, size);
}

void TcpClientModule_Impl::onParseError()
{
}

void TcpClientModule_Impl::onConnectDone()
{
	m_tcpClientState = TCPCLIENT_STATE_OK;
	::SetEvent(m_eventConnected);

	m_bDoReloginServerNow = FALSE;
	if (!m_pServerPingTimer)
	{
		_startServerPingTimer();
	}
}

BOOL TcpClientModule_Impl::create()
{
	return m_pClientSocket->create();
}

CImPdu* TcpClientModule_Impl::doLogin(CString &linkaddr, UInt16 port
	,CString& uName,std::string& pass)
{
	BOOL bRet = m_pClientSocket->connect(linkaddr, port);
	PTR_NULL(bRet);

	CImPduLoginResponse* pduLoginResp = 0;
	if (_waitConnectedNotify())
	{
		string name = util::cStringToString(uName);
		CImPduLoginRequest pduLogin(name.c_str(), pass.c_str(), USER_STATUS_ONLINE, CLIENT_TYPE_WINDOWS);
		pduLoginResp = (CImPduLoginResponse*)_sendPacketAndWaitResponse(&pduLogin);
	}

	return pduLoginResp;
}

void TcpClientModule_Impl::closeSocket()
{
	m_pClientSocket->closeSocket();
}

void TcpClientModule_Impl::shutdown()
{
	m_pClientSocket->shutdown();
	_stopHearbeat();
	_stopServerPingTimer();
}

CImPdu* TcpClientModule_Impl::_sendPacketAndWaitResponse(CImPdu* pdu, UInt32 timeout /*= 20*/)
{
	if (TCPCLIENT_STATE_OK != m_tcpClientState)
		return 0;
	pdu->SetReserved(++g_seqNum);
	sendPacket(pdu);

	if (m_pCurrSeqImPdu)
		delete m_pCurrSeqImPdu;
	m_pCurrSeqImPdu = 0;
	UInt32 t = 0;
	timeout *= 1000;
	UInt32 waitResult;
	do
	{
		int timeWaiter = 500;
		t += timeWaiter;
		waitResult = WaitForSingleObject(m_currSeqEvent, timeWaiter);
	} while ((WAIT_TIMEOUT == waitResult) && (t < timeout));

	return m_pCurrSeqImPdu;
}

void TcpClientModule_Impl::sendPacket(CImPdu* pdu)
{
	m_pClientSocket->send((const char*)pdu->GetBuffer(), pdu->GetLength());
}

BOOL TcpClientModule_Impl::_waitConnectedNotify()
{
	int timeout = 10000;        //10秒
	int t = 0;
	DWORD waitResult = WAIT_FAILED;
	do
	{
		int timeWaiter = 500;
		t += timeWaiter;
		waitResult = WaitForSingleObject(m_eventConnected, timeWaiter);
	} while ((WAIT_TIMEOUT == waitResult) && (t < timeout));

	return (WAIT_OBJECT_0 == waitResult);
}

void TcpClientModule_Impl::startHeartbeat()
{
	GetLogic()->scheduleTimerWithLambda(5, TRUE,
		[=]()
	{
		GetLogic()->pushBackOperationWithLambda(
			[=]()
		{
			CImPduHeartbeat pduHeartbeat;
			sendPacket(&pduHeartbeat);
		}
		);
	}
	, &m_pHearbeatTimer);
}

void TcpClientModule_Impl::_stopHearbeat()
{
	GetLogic()->killTimer(m_pHearbeatTimer);
	m_pHearbeatTimer = 0;
}

void TcpClientModule_Impl::_handlePacketOperation(const char* data, UInt32 size)
{
	std::string copyInBuffer(data, size);
	GetLogic()->pushBackOperationWithLambda(
		[=]()
	{
		CImPdu* pPdu = 0;
		try
		{
			pPdu = CImPdu::ReadPdu((uchar_t*)copyInBuffer.data(), (uint32_t)copyInBuffer.size());
		}
		catch (CPduException e)
		{
			APP_LOG(LOG_ERROR, TRUE, _T("_handlePacketOperation CPduException moduleId:%d,commandId:%d,errCode:%d")
				, e.GetModuleId(), e.GetCommandId(), e.GetErrorCode());
		}
		catch (...)
		{
			APP_LOG(LOG_ERROR, TRUE, _T("_handlePacketOperation unknown exception"));
		}
		PTR_VOID(pPdu);

		IPduAsyncSocketModule* pModule
			= (IPduAsyncSocketModule*)GetLogic()->getModule(pPdu->GetModuleId());
		if (!pModule)
		{
			assert(FALSE);
			APP_LOG(LOG_ERROR, TRUE, _T("_handlePacketOperation module is null, moduleId:%d,commandId:%d")
				, pPdu->GetModuleId(), pPdu->GetCommandId());
			return;
		}
		std::auto_ptr<CImPdu> raiiImPdu(pPdu);
		pModule->onPacket(raiiImPdu);
	});
}

void TcpClientModule_Impl::_startServerPingTimer()
{
	if (!m_pServerPingTimer)
		m_pServerPingTimer = new ServerPingTimer(this);
	logic::GetLogic()->scheduleTimer(m_pServerPingTimer, 60, TRUE);
}

void TcpClientModule_Impl::_stopServerPingTimer()
{
	if (m_pServerPingTimer)
		logic::GetLogic()->killTimer(m_pServerPingTimer);
	m_pServerPingTimer = 0;
}

void TcpClientModule_Impl::_doReloginServer()
{
	APP_LOG(LOG_INFO, TRUE, _T("doReloginServer now!!!"));
	m_bDoReloginServerNow = TRUE;
	onClose(0);
}

UInt8 TcpClientModule_Impl::getTcpClientNetState() const
{
	return m_tcpClientState;
}

ServerPingTimer::ServerPingTimer(TcpClientModule_Impl* pTcpClient)
:m_pTcpClient(pTcpClient)
,m_bHasReceivedPing(FALSE)
{

}

void ServerPingTimer::process()
{
	if (!m_bHasReceivedPing && !m_pTcpClient->m_bDoReloginServerNow)
	{
		m_pTcpClient->_doReloginServer();
	}
	m_bHasReceivedPing = FALSE;
}

void ServerPingTimer::release()
{
	delete this;
}

NAMESPACE_END(logic)
/******************************************************************************/