/*******************************************************************************
 *  @file      FileTransferSocket.h 2014\8\30 13:31:33 $
 *  @author    快刀<kuaidao@mogujie.com>
 *  @brief     文件传输socket
 ******************************************************************************/

#ifndef FILETRANSFERSOCKET_612C0628_0BB6_4FF0_BE0F_F5DE2C35836D_H__
#define FILETRANSFERSOCKET_612C0628_0BB6_4FF0_BE0F_F5DE2C35836D_H__

#include "GlobalDefine.h"
#include "TTLogic/IOperation.h"
#include "TTLogic/IEvent.h"
#include "TTLogic/ITcpClientModule.h"

/******************************************************************************/
class CImPdu;
class FileTransferSocket;
class FileTransTaskBase;

namespace logic
{
	class TcpClientSocket;
	struct IOperation;
	struct ITimerEvent;
}

class PingFileSevTimer : public logic::ITimerEvent
{
public:
	PingFileSevTimer(FileTransferSocket* pTransSocket);
	virtual void process();
	virtual void release();

private:
	FileTransferSocket* m_pFileTransSocket;
};

/**
* The class <code>文件传输socket</code>
*
*/
class FileTransferSocket :public logic::ITcpClientSocketCallback
{
public:
	FileTransferSocket(std::string& taskId);
	~FileTransferSocket(void);

public:
	BOOL startFileTransLink();
	void stopfileTransLink();
	void sendPacket(CImPdu* pdu);

private:
	//创建连接
	virtual BOOL create();
	virtual BOOL connect(const CString &linkaddr, UInt16 port);
	virtual BOOL shutdown();

	//心跳包
	virtual void startHeartbeat();
	virtual void stopHeartbeat();

	//回调接口
	virtual void registerCB(logic::ITcpClientSocketCallback *cb);
	virtual void unRegisterCB();
	virtual logic::ITcpClientSocketCallback* getCallback();

	virtual void onClose(int error);
	virtual void onReceiveData(const char* data, UInt32 size);
	virtual void onParseError();
	virtual void onConnectDone();

private:
	/**@name 服务器端拆包*/
	//@{
	void _fileLoginResponse(CImPdu* pdu);
	void _filePullDataReqResponse(CImPdu* pdu);
	void _filePullDataRspResponse(CImPdu* pdu);
	void _fileState(CImPdu* pdu);
	//@}

public:
	std::string							m_sTaskId;

private:
	logic::TcpClientSocket*             m_pLinkSocket;
	PingFileSevTimer*                   m_pPingTimer;
	ITcpClientSocketCallback*           m_Icallback;
	UInt32                              m_progressRefreshMark;
};
/******************************************************************************/
#endif// #define FILETRANSFERSOCKET_612C0628_0BB6_4FF0_BE0F_F5DE2C35836D_H__