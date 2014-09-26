/*******************************************************************************
 *  @file      TcpClientModule_Impl.h 2014\7\29 13:16:06 $
 *  @author    快刀<kuaidao@mogujie.com>
 *  @brief     客户端TCP网络长连接模块实现，对TcpClientScoket做一个适配
 ******************************************************************************/

#ifndef TCPCLIENTMODULE_IMPL_7C021D09_7902_44DE_BE96_F22561B99198_H__
#define TCPCLIENTMODULE_IMPL_7C021D09_7902_44DE_BE96_F22561B99198_H__

#include "TTLogic/ITcpClientModule.h"
#include "TTLogic/TcpClientSocket.h"
/******************************************************************************/
NAMESPACE_BEGIN(logic)
class TcpClientSocket;
class TcpClientModule_Impl;

/**
* The class <code>模块器端的一个心跳的维护</code>
*
*/
class ServerPingTimer : public ITimerEvent
{
public:
	ServerPingTimer(TcpClientModule_Impl* pTcpClient);
	virtual void process();
	virtual void release();

public:
	BOOL									m_bHasReceivedPing;
	TcpClientModule_Impl*					m_pTcpClient;
};

/**
 * The class <code>客户端TCP网络长连接模块实现，对TcpClientScoket做一个适配</code> 
 *
 */
class TcpClientModule_Impl final : public logic::ITcpClientModule
								  ,public ITcpClientSocketCallback
{
	friend class ServerPingTimer;
public:
    /** @name Constructors and Destructor*/

    //@{
    /**
     * Constructor 
     */
	TcpClientModule_Impl();
    /**
     * Destructor
     */
    virtual ~TcpClientModule_Impl();
    //@}
	virtual void release();

public:
	virtual BOOL create();
	virtual CImPdu* doLogin(CString &linkaddr, UInt16 port
		, CString& uName, std::string& pass);
	virtual void closeSocket();
	virtual void shutdown();
	virtual void sendPacket(CImPdu* pdu);
	virtual void startHeartbeat();
	virtual UInt8 getTcpClientNetState()const;

public:
	virtual void onClose(int error);
	virtual void onReceiveData(const char* data, UInt32 size);
	virtual void onParseError();
	virtual void onConnectDone();

private:
	BOOL _waitConnectedNotify();
	CImPdu* _sendPacketAndWaitResponse(CImPdu* pdu, UInt32 timeout = 10);
	void _stopHearbeat();
	void _handlePacketOperation(const char* data, UInt32 size);
	void _startServerPingTimer();
	void _stopServerPingTimer();
	void _doReloginServer();

private:
	CImPdu*							m_pCurrSeqImPdu;
	TcpClientSocket*				m_pClientSocket;
	ITimerEvent*					m_pHearbeatTimer;
	ServerPingTimer*				m_pServerPingTimer;
	BOOL							m_bDoReloginServerNow;
	UInt8							m_tcpClientState;
	HANDLE							m_eventConnected;
	HANDLE							m_currSeqEvent;
};

NAMESPACE_END(logic)
/******************************************************************************/
#endif// TCPCLIENTMODULE_IMPL_7c021d09-7902-44de-be96-f22561b99198_H__
