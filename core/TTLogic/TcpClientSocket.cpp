/******************************************************************************* 
 *  @file      TcpClientSocket.cpp 2014\7\29 13:36:38 $
 *  @author    快刀<kuaidao@mogujie.com>
 *  @brief     
 ******************************************************************************/

#include "stdafx.h"
#include "TTLogic/TcpClientSocket.h"
#include "TTLogic/ITcpClientModule.h"
#include "utility/utilCommonAPI.h"
#include "utility/utilStrCodeAPI.h"

/******************************************************************************/
NAMESPACE_BEGIN(logic)
namespace
{
	const BOOL g_bIsEncrypt = FALSE;
	const UInt32 SOCK_BUFF_SIZE = 1024 * 128;
}

// -----------------------------------------------------------------------------
//  TcpClientSocket: Public, Constructor

TcpClientSocket::TcpClientSocket()
:m_pTcpClientSocketCB(0)
{

}

// -----------------------------------------------------------------------------
//  TcpClientSocket: Public, Destructor

TcpClientSocket::~TcpClientSocket()
{
	unRegisterCB();
}

void TcpClientSocket::onAccept(int nErrorCode)
{

}

void TcpClientSocket::onClose(int nErrorCode)
{
	APP_LOG(LOG_INFO, TRUE, _T("TcpClientSocket onClose, %d"), nErrorCode);
	if (m_pTcpClientSocketCB)
		m_pTcpClientSocketCB->onClose(nErrorCode);
}

void TcpClientSocket::onConnect(int nErrorCode)
{	
	if (0 != nErrorCode)
	{
		APP_LOG(LOG_ERROR, TRUE, _T("TcpClientSocket connected error, %d"), nErrorCode);
		if (m_pTcpClientSocketCB)
			m_pTcpClientSocketCB->onClose(nErrorCode);
		return;
	}
	APP_LOG(LOG_INFO, _T("TcpClientSocket connected, %d"), nErrorCode);
	if (m_pTcpClientSocketCB)
		m_pTcpClientSocketCB->onConnectDone();
}

void TcpClientSocket::onReceive(int nErrorCode)
{
	if (0 != nErrorCode)
	{
		APP_LOG(LOG_ERROR, TRUE, _T("TcpClientSocket onReceive error, %d"), nErrorCode);
		m_pTcpClientSocketCB->onClose(nErrorCode);
		return;
	}

	char buf[SOCK_BUFF_SIZE];
	int recved = SOCKET_ERROR;
	try
	{
		recved = __super::Receive(buf, SOCK_BUFF_SIZE, 0);
	}
	catch (...)
	{
		assert(FALSE);
	}
	if (SOCKET_ERROR == recved || recved == 0)
	{
		APP_LOG(LOG_ERROR, TRUE, _T("TcpClientSocket onReceive recved %d"), recved);
		return;
	}

	if (g_bIsEncrypt)
	{
		char tmpbuf[SOCK_BUFF_SIZE];
		//m_decin->Process(buf, tmpbuf, recved);
		m_InBuffer.append(tmpbuf, recved);
	}
	else
	{
		m_InBuffer.append(buf, recved);
	}
	
	//parse data packet
	while (!m_InBuffer.empty())
	{
		if (m_InBuffer.size() < 14) // need more
		{
			APP_LOG(LOG_DEBUG, _T("TcpClientSocket parseBuffer need more"));
			return;
		}

		UInt32 packetsize = ntohl(*(UInt32*)m_InBuffer.data());
		if (packetsize > m_InBuffer.size())
		{
			//APP_LOG(LOG_ERROR, _T("TcpClientSocket parseBuffer packetsize > m_InBuffer size"));
			break;
		}

		try
		{
			if (m_pTcpClientSocketCB)
				m_pTcpClientSocketCB->onReceiveData(m_InBuffer.data(), packetsize);
			m_InBuffer.erase(0, packetsize);
		}
		catch (std::exception& ex)
		{
			ASSERT(FALSE);
			CString cs = util::stringToCString(ex.what());
			APP_LOG(LOG_ERROR, TRUE, _T("TcpClientSocket parseBuffer exception msg:%s"), cs);
			m_InBuffer.erase(0, packetsize);
			if (m_pTcpClientSocketCB)
				m_pTcpClientSocketCB->onParseError();
		}
	}
}

void TcpClientSocket::onSend(int nErrorCode)
{
	util::TTAutoLock lock(&m_OutBufferLock);
	if (nErrorCode != 0)
	{
		APP_LOG(LOG_ERROR, FALSE, _T("TcpClientSocket onSend error,%d"), nErrorCode);
		if (m_pTcpClientSocketCB != NULL)
			m_pTcpClientSocketCB->onClose(nErrorCode);
		return;
	}
	if (m_OutBuffer.size() == 0)
		return;

	int sended = SOCKET_ERROR;
	try
	{
		sended = __super::Send(m_OutBuffer.data(), m_OutBuffer.size());
	}
	catch (...)
	{
		assert(false);
	}
	if (SOCKET_ERROR == sended)
	{
		if (_wouldBlock())
			return;

		closeSocket();
		int error = GetLastError();
		APP_LOG(LOG_ERROR, FALSE, _T("TcpClientSocket onSend error,last error id:%d"), error);
		if (m_pTcpClientSocketCB)
			m_pTcpClientSocketCB->onClose(error);
	}
	if (sended >= 0)
		m_OutBuffer.erase(0, sended);
}

BOOL TcpClientSocket::create()
{
	return CAsyncSocketEx::Create();
}

BOOL TcpClientSocket::connect(LPCTSTR lpszHostAddress, UInt32 nHostPort)
{
	if (!CAsyncSocketEx::Connect(lpszHostAddress, nHostPort))
	{
		if (_wouldBlock())
			return TRUE;
		else
			return FALSE;
	}
	return TRUE;
}

void TcpClientSocket::send(const char* lpBuf, int nBufLen)
{
	util::TTAutoLock lock(&m_OutBufferLock);
	if (g_bIsEncrypt)
	{
		std::string sbuf;
		#pragma message(__FILE__ ": warning : 通讯加密算法，有时间在实现")
		//m_decout->encrypt(lpBuf,nBufLen,sbuf);
		//m_OutBuffer.append(sbuf.c_str(), nBufLen);	
		sbuf.clear();
	}
	else
	{
		m_OutBuffer.append(lpBuf, nBufLen);
	}
	int sended = 0;
	try
	{
		sended = __super::Send(m_OutBuffer.data(), m_OutBuffer.size());
	}
	catch (...)
	{
		assert(false);
		sended = SOCKET_ERROR;
	}

	if (SOCKET_ERROR == sended)
	{
		if (_wouldBlock()) {
			APP_LOG(LOG_DETAIL, FALSE, TEXT("Would Block in TcpClientSocket::Write"));
			return;
		}
		APP_LOG(LOG_ERROR, FALSE, TEXT("Write packet error in TcpClientSocket::Write:%d"), m_OutBuffer.size());
		int error = GetLastError();
		closeSocket();
		if (0 != m_pTcpClientSocketCB)
			m_pTcpClientSocketCB->onClose(error);
	}
	if (sended >= 0)
		m_OutBuffer.erase(0, sended);
}

void TcpClientSocket::closeSocket()
{
	m_OutBuffer.erase(0, m_OutBuffer.size());
	CAsyncSocketEx::Close();
}

BOOL TcpClientSocket::shutdown()
{
	return CAsyncSocketEx::ShutDown(2);
}

BOOL TcpClientSocket::_wouldBlock()
{
	DWORD err = WSAGetLastError();
	if (WSAEWOULDBLOCK == err || WSAEINPROGRESS == err)
		return TRUE;

	return FALSE;
}

NAMESPACE_END(logic)
/******************************************************************************/