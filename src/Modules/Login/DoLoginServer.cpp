/******************************************************************************* 
 *  @file      LoginServerLink.cpp 2013\8\28 14:56:32 $
 *  @author    快刀<kuaidao@mogujie.com>
 *  @brief   
 ******************************************************************************/

#include "stdafx.h"
#include "DoLoginServer.h"
#include "TTLogic/ILogic.h"
#include "TTLogic/TcpClientSocket.h"
#include "utility/utilCommonAPI.h"
#include "src/base/ImPduClient.h"
#include "Modules/ISysConfigModule.h"

/******************************************************************************/
namespace
{
    DoLoginServer* g_doLoginServer = 0;
}

// -----------------------------------------------------------------------------
//  LoginServerLink: Public, Constructor

DoLoginServer::DoLoginServer()
:m_pLinkSocket(0)
,m_pImPdu(0)
,m_eventReceived(0)
,m_eventConnected(0)
{
}

// -----------------------------------------------------------------------------
//  LoginServerLink: Public, Destructor

DoLoginServer::~DoLoginServer()
{

}

CImPdu* DoLoginServer::doLogin()
{
    m_pImPdu = 0;
    PTR_NULL(m_pLinkSocket);

	//登录服务器地址
	module::TTConfig* pCfg = module::getSysConfigModule()->getSystemConfig();
	BOOL bRet = m_pLinkSocket->connect(pCfg->loginServIP, pCfg->loginServPort);
	PTR_NULL(bRet);

    if(_waitConnectedNotify())
    {
        //请求消息服务器信息
        CImPduMsgServRequest pduMsgServReq;
        m_pImPdu = _sendPacketAndWaitResponse(&pduMsgServReq);
    }

    return m_pImPdu;
}

void DoLoginServer::onClose(int error)
{
    APP_LOG(LOG_ERROR,_T("LoginServerLink onDisConnected error code:%d"),error);
    //::SetEvent(m_eventConnected);
}

void DoLoginServer::onReceiveData(const char* data, UInt32 size)
{
    m_serailBuffer.clear();
	m_serailBuffer.append(data, size);
    APP_LOG(LOG_DEBUG,_T("LoginServerLink onPacket size：%d"),m_serailBuffer.size());
    try
    {
        m_pImPdu = CImPdu::ReadPdu((uchar_t*)m_serailBuffer.data(),(uint32_t)m_serailBuffer.size());
    }
    catch(CPduException e)
    {
		APP_LOG(LOG_ERROR, _T("LoginServerLink onPacket CPduException serviceId:%d,commandId:%d,errCode:%d")
            ,e.GetModuleId(),e.GetCommandId(),e.GetErrorCode());
        return;
    }
    catch(...)
    {
		APP_LOG(LOG_ERROR, TRUE, _T("LoginServerLink onPacket unknown exception"));
        return;
    }

    PTR_VOID(m_pImPdu);

    ::SetEvent(m_eventReceived);
}

void DoLoginServer::onParseError()
{
	APP_LOG(LOG_ERROR, TRUE, _T("LoginServerLink onParseError error"));
    //::SetEvent(m_eventConnected);
}

void DoLoginServer::onConnectDone()
{
    ::SetEvent(m_eventConnected);
}

DoLoginServer* DoLoginServer::getInstance()
{
    if(0 == g_doLoginServer)
    {
        g_doLoginServer = new DoLoginServer;
    }
    return g_doLoginServer;
}

BOOL DoLoginServer::createLink()
{
    m_pLinkSocket = new logic::TcpClientSocket;
    m_pLinkSocket->registerCB(this);

    m_eventReceived = CreateEvent( NULL, FALSE, FALSE, NULL );
    m_eventConnected = CreateEvent( NULL, FALSE, FALSE, NULL );

    BOOL bret = m_pLinkSocket->create();
    PTR_FALSE(bret);

    return TRUE;
}

CImPdu* DoLoginServer::_sendPacketAndWaitResponse(CImPdu* pdu,UInt32 timeout/*=10*/)
{
	m_pLinkSocket->send((const char*)pdu->GetBuffer(), pdu->GetLength());

	delete m_pImPdu;
	m_pImPdu = 0;
	UInt32 t = 0;
	timeout *= 1000;
	UInt32 waitResult = 0;
	do
	{
		int timeWaiter = 500;
		t += timeWaiter;
		waitResult = WaitForSingleObject(m_eventReceived, timeWaiter);
	} while ((WAIT_TIMEOUT == waitResult) && (t < timeout));

    return m_pImPdu;
}

void DoLoginServer::shutdown()
{
    delete m_pImPdu;
    m_pImPdu = 0;
    if(m_eventReceived)
        CloseHandle(m_eventReceived);
    m_eventReceived = 0;
    if(m_eventConnected)
        CloseHandle(m_eventConnected);
    m_eventConnected = 0;

    if(m_pLinkSocket)
    {
        m_pLinkSocket->closeSocket();
        m_pLinkSocket->shutdown();
    }
    delete m_pLinkSocket;
    m_pLinkSocket = 0;
}

void DoLoginServer::releaseInstance()
{
    delete g_doLoginServer;
    g_doLoginServer = 0;
}

BOOL DoLoginServer::_waitConnectedNotify()
{
    int timeout = 10000;        //10秒
    int t = 0;
    DWORD waitResult = WAIT_FAILED;
    do 
    {
        int timeWaiter = 500 ;
        t += timeWaiter;
        waitResult =  WaitForSingleObject(m_eventConnected, timeWaiter);		
    }while( (WAIT_TIMEOUT == waitResult) && (t < timeout));

    return (WAIT_OBJECT_0  == waitResult);
}

/******************************************************************************/