/******************************************************************************* 
 *  @file      FileTransferSocket.cpp 2014\8\30 13:32:37 $
 *  @author    快刀<kuaidao@mogujie.com>
 *  @brief     文件传输socket
 ******************************************************************************/

#include "StdAfx.h"
#include "FileTransferSocket.h"
#include "FileTransferTask.h"
#include "TransferManager.h"
#include "TTLogic/ILogic.h"
#include "TTLogic/TcpClientSocket.h"
#include "src/base/ImPduFile.h"
#include "src/base/ImPduClient.h"
#include "Modules/IFileTransferModule.h"
#include "Modules/IMiscModule.h"
#include "Modules/ISysConfigModule.h"
#include "utility/utilCommonAPI.h"
#include "TransferFile.h"
/******************************************************************************/
FileTransferSocket::FileTransferSocket(std::string& taskId)
:m_pLinkSocket(0)
, m_pPingTimer(0)
, m_Icallback(0)
, m_sTaskId(taskId)
{
	m_pLinkSocket = new logic::TcpClientSocket;
}

FileTransferSocket::~FileTransferSocket(void)
{
	delete m_pLinkSocket;
	m_pLinkSocket = 0;
}

BOOL FileTransferSocket::create()
{
	m_pLinkSocket->registerCB(this);

	BOOL bret = m_pLinkSocket->create();
	PTR_FALSE(bret);

	return TRUE;
}

BOOL FileTransferSocket::connect(const CString &linkaddr, UInt16 port)
{
	BOOL bRet = m_pLinkSocket->connect(linkaddr, port);
	PTR_NULL(bRet);

	return TRUE;
}

BOOL FileTransferSocket::shutdown()
{
	if (m_pLinkSocket)
	{
		m_pLinkSocket->closeSocket();
		m_pLinkSocket->shutdown();
		delete m_pLinkSocket;
		m_pLinkSocket = 0;
	}
	return TRUE;
}

void FileTransferSocket::sendPacket(CImPdu* pdu)
{
	m_pLinkSocket->send((const char*)pdu->GetBuffer(), pdu->GetLength());
}

void FileTransferSocket::startHeartbeat()
{
	if (!m_pPingTimer)
		m_pPingTimer = new PingFileSevTimer(this);
	logic::GetLogic()->scheduleTimer(m_pPingTimer, 5, TRUE);
}

void FileTransferSocket::stopHeartbeat()
{
	if (m_pPingTimer)
		logic::GetLogic()->killTimer(m_pPingTimer);
	m_pPingTimer = 0;
}

void FileTransferSocket::onReceiveData(const char* data, UInt32 size)
{
	CImPdu* pPdu = 0;
	try
	{
		pPdu = CImPdu::ReadPdu((uchar_t*)data, (uint32_t)size);
		PTR_VOID(pPdu);
		if (IM_PDU_TYPE_HEARTBEAT == pPdu->GetCommandId() && SID_OTHER == pPdu->GetModuleId())
			return;
	}
	catch (CPduException e)
	{
		APP_LOG(LOG_ERROR, _T("FileTransferSocket onPacket CPduException serviceId:%d,commandId:%d,errCode:%d")
			, e.GetModuleId(), e.GetCommandId(), e.GetErrorCode());
		return;
	}
	catch (...)
	{
		APP_LOG(LOG_ERROR, _T("FileTransferSocket onPacket unknown exception"));
		return;
	}
	UInt16 ncmdid = pPdu->GetCommandId();
	switch (pPdu->GetCommandId())
	{
	case CID_FILE_LOGIN_RES:
		_fileLoginResponse(pPdu);
		break;
	case CID_FILE_PULL_DATA_REQ:
		_filePullDataReqResponse(pPdu);
		break;
	case CID_FILE_PULL_DATA_RSP:
		_filePullDataRspResponse(pPdu);
		break;
	case CID_FILE_STATE:
		_fileState(pPdu);
	default:
		break;
	}
	delete pPdu;
	pPdu = 0;
}

void FileTransferSocket::onParseError()
{
	APP_LOG(LOG_ERROR, _T("FileTransferSocket onParseError error"));
}

void FileTransferSocket::onConnectDone()
{
	APP_LOG(LOG_INFO, _T("FileTransferSocket::onConnected()"));
	startHeartbeat();

	TransferFileEntity info;
	if (!TransferFileEntityManager::getInstance()->getFileInfoByTaskId(m_sTaskId, info))
		return;

	//拉模式文件传输，传输taskid、token、client_mode
	CImPduClientFileLoginReq pduFileLoginReq(module::getSysConfigModule()->userID().c_str()
		, "", info.sTaskID.c_str(), info.nClientMode);
	sendPacket(&pduFileLoginReq);
}
void FileTransferSocket::onClose(int error)
{
	APP_LOG(LOG_INFO, _T("FileTransferSocket::onClose error code:%d"), error);
	stopHeartbeat();
}

void FileTransferSocket::registerCB(logic::ITcpClientSocketCallback *cb)
{
	m_Icallback = cb;
}

void FileTransferSocket::unRegisterCB()
{
	m_Icallback = 0;
}

logic::ITcpClientSocketCallback* FileTransferSocket::getCallback()
{
	return m_Icallback;
}

BOOL FileTransferSocket::startFileTransLink()
{
	create();
	TransferFileEntity FileInfo;
	if (TransferFileEntityManager::getInstance()->getFileInfoByTaskId(m_sTaskId, FileInfo))
	{
		//大佛：使用msg server 传过来的IP和端口
		APP_LOG(LOG_INFO, _T("FileTransferSocket::startFileTransLink connect IP=%s,Port=%d"), util::stringToCString(FileInfo.sIP), FileInfo.nPort);
		connect(util::stringToCString(FileInfo.sIP), FileInfo.nPort);
		//connect(util::stringToCString(module::FILETRANSFER_IP), module::FILETRANSFER_PORT);
		return TRUE;
	}
	APP_LOG(LOG_ERROR, _T("FileTransferSocket::startFileTransLink can't find the TaskId"));
	return FALSE;
}

void FileTransferSocket::stopfileTransLink()
{
	shutdown();
	stopHeartbeat();
}

void FileTransferSocket::_fileLoginResponse(CImPdu* pdu)
{
	CImPduClientFileLoginRes* pData = (CImPduClientFileLoginRes*)pdu;
	if (!pData || pData->GetResult() != 0)
	{
		APP_LOG(LOG_ERROR, _T("file server login failed! "));
		return;
	}
	//打开文件
	std::string taskId(pData->GetTaskId(), pData->GetTaskIdLen());
	TransferFileEntity fileEntity;
	if (!TransferFileEntityManager::getInstance()->getFileInfoByTaskId(taskId, fileEntity))
	{
		APP_LOG(LOG_ERROR, _T("file server login:can't find the fileInfo "));
		return;
	}

	APP_LOG(LOG_INFO, _T("file server login succeed"));
	//提示界面,界面上插入该项
	if (CLIENT_REALTIME_SENDER == fileEntity.nClientMode
		|| CLIENT_OFFLINE_UPLOAD == fileEntity.nClientMode)
	{
		logic::GetLogic()->asynNotifyObserver(module::KEY_FILETRANSFER_SENDFILE, fileEntity.sTaskID);
	}
	else if (CLIENT_REALTIME_RECVER == fileEntity.nClientMode
		|| CLIENT_OFFLINE_DOWNLOAD == fileEntity.nClientMode)
	{
		logic::GetLogic()->asynNotifyObserver(module::KEY_FILETRANSFER_REQUEST, fileEntity.sTaskID);
	}
}

void FileTransferSocket::_filePullDataReqResponse(CImPdu* pdu)//发
{
	CImPduClientFilePullDataReq* pData = (CImPduClientFilePullDataReq*)pdu;
	UInt32 fileSize = pData->GetDataSize();
	UInt32 fileOffset = pData->GetOffset();
	std::string taskId = std::string(pData->GetTaskId(),pData->GetTaskIdLen());
	
	TransferFileEntity fileEntity;
	if (!TransferFileEntityManager::getInstance()->getFileInfoByTaskId(taskId, fileEntity))
	{
		APP_LOG(LOG_ERROR, _T("PullDataReqResponse: can't find the fileInfo"));
		return;
	}
	APP_LOG(LOG_DEBUG, _T("send:taskId=%s,filesize=%d,name=%s")
		,util::stringToCString(fileEntity.sTaskID)
		,fileEntity.nFileSize
		,fileEntity.getRealFileName());
	std::string buff;
	if (nullptr == fileEntity.pFileObject)
	{
		APP_LOG(LOG_ERROR, _T("PullDataReqResponse: file boject Destoryed!"));
		return;
	}
	fileEntity.pFileObject->readBlock(fileOffset, fileSize, buff);
	CImPduClientFilePullDataRsp pduPullDataRep(taskId.c_str()
		, fileEntity.sFromID.c_str(), fileOffset, fileSize, (uchar_t*)buff.data());
	sendPacket(&pduPullDataRep);

	fileEntity.nProgress = fileOffset + fileSize;
	if (fileEntity.nProgress <= fileEntity.nFileSize)
	{
		//更新进度条
		TransferFileEntityManager::getInstance()->updateFileInfoBysTaskID(fileEntity);//保存当前进度
		logic::GetLogic()->asynNotifyObserver(module::KEY_FILESEVER_UPDATA_PROGRESSBAR, fileEntity.sTaskID);
	}
	else//传输完成
	{
		if (fileEntity.pFileObject)
		{
			delete fileEntity.pFileObject;
			fileEntity.pFileObject = nullptr;
		}
		logic::GetLogic()->asynNotifyObserver(module::KEY_FILESEVER_PROGRESSBAR_FINISHED, fileEntity.sTaskID);
	}
	TransferFileEntityManager::getInstance()->updateFileInfoBysTaskID(fileEntity);
}

void FileTransferSocket::_filePullDataRspResponse(CImPdu* pdu)//收
{
	CImPduClientFilePullDataRsp* pDataRsp = (CImPduClientFilePullDataRsp*)pdu;
	UInt32 nRes = pDataRsp->GetResult();
	if (FILE_SERVER_ERRNO_OK != nRes)
	{
		APP_LOG(LOG_ERROR, _T("PullDataRspResponse: error result:%d"),nRes);
		return;
	}
	std::string taskId(pDataRsp->GetTaskId(), pDataRsp->GetTaskIdLen());
	void* pData = pDataRsp->GetData();
	UInt32 fileSize = pDataRsp->GetDataSize();
	UInt32 fileOffset = pDataRsp->GetOffset();
	TransferFileEntity fileEntity;
	if (!TransferFileEntityManager::getInstance()->getFileInfoByTaskId(taskId, fileEntity))
	{
		APP_LOG(LOG_ERROR, _T("PullDataRspResponse: can't find the fileInfo"));
		return;
	}
	APP_LOG(LOG_DEBUG, _T("receive:taskId=%s,filesize=%d,name=%s")
		, util::stringToCString(fileEntity.sTaskID)
		, fileEntity.nFileSize
		, fileEntity.getRealFileName());

	//存文件...
	if (!fileEntity.pFileObject->writeBlock(fileOffset, fileSize, pData))
	{
		APP_LOG(LOG_DEBUG, _T("FileTransferSocket::_filePullDataRspResponse-writeBlock failed "));
		return;
	}

	fileEntity.nProgress = fileOffset + fileSize;
	if (fileEntity.nProgress <= fileEntity.nFileSize)
	{
		//更新进度条
		TransferFileEntityManager::getInstance()->updateFileInfoBysTaskID(fileEntity);//保存当前进度
		logic::GetLogic()->asynNotifyObserver(module::KEY_FILESEVER_UPDATA_PROGRESSBAR, fileEntity.sTaskID);

		//继续发file block req...
		int mode = fileEntity.nClientMode == CLIENT_OFFLINE_DOWNLOAD ? FILE_TYPE_OFFLINE : FILE_TYPE_ONLINE;
		CImPduClientFilePullDataReq pduPullDataReq(taskId.c_str(), fileEntity.sToID.c_str()
			, mode, fileEntity.nProgress, fileSize);
		sendPacket(&pduPullDataReq);
	}
	else//传输完成
	{
		if (fileEntity.pFileObject)
		{
			delete fileEntity.pFileObject;
			fileEntity.pFileObject = nullptr;
		}
		TransferFileEntityManager::getInstance()->updateFileInfoBysTaskID(fileEntity);
		logic::GetLogic()->asynNotifyObserver(module::KEY_FILESEVER_PROGRESSBAR_FINISHED, fileEntity.sTaskID);
	}
}

void FileTransferSocket::_fileState(CImPdu* pdu)
{
	CImPduClientFileState* pData = (CImPduClientFileState*)pdu;
	UINT32 nfileState = pData->GetState();

	std::string taskId(pData->GetTaskId(), pData->GetTaskIdLen());
	TransferFileEntity fileEntity;
	if (!TransferFileEntityManager::getInstance()->getFileInfoByTaskId(taskId, fileEntity))
	{
		APP_LOG(LOG_ERROR, _T("_fileState:can't find the fileInfo "));
		return;
	}

	switch (nfileState)
	{
	case CLIENT_FILE_PEER_READY:
		APP_LOG(LOG_DEBUG, _T("FileTransferSocket::_fileState--CLIENT_FILE_PEER_READY "));
		break;
	case CLIENT_FILE_CANCEL://取消的了文件传输
		APP_LOG(LOG_DEBUG, _T("FileTransferSocket::_fileState--CLIENT_FILE_CANCEL "));
		{
			delete fileEntity.pFileObject;
			fileEntity.pFileObject = nullptr;
			TransferFileEntityManager::getInstance()->updateFileInfoBysTaskID(fileEntity);
			logic::GetLogic()->asynNotifyObserver(module::KEY_FILESEVER_UPDATA_CANCEL, fileEntity.sTaskID);
		}
		break;
	case CLIENT_FILE_REFUSE:
		APP_LOG(LOG_DEBUG, _T("FileTransferSocket::_fileState--CLIENT_FILE_REFUSE "));
		{
			delete fileEntity.pFileObject;
			fileEntity.pFileObject = nullptr;
			TransferFileEntityManager::getInstance()->updateFileInfoBysTaskID(fileEntity);
			logic::GetLogic()->asynNotifyObserver(module::KEY_FILESEVER_UPDATA_REJECT, fileEntity.sTaskID);
		}
		break;
	case CLIENT_FILE_DONE:
		APP_LOG(LOG_DEBUG, _T("FileTransferSocket::_fileState--CLIENT_FILE_DONE "));
		if (fileEntity.pFileObject)
		{
			delete fileEntity.pFileObject;
			fileEntity.pFileObject = nullptr;
		}
		TransferFileEntityManager::getInstance()->updateFileInfoBysTaskID(fileEntity);
		logic::GetLogic()->asynNotifyObserver(module::KEY_FILESEVER_PROGRESSBAR_FINISHED, fileEntity.sTaskID);
		break;
	default:
		break;
	}
}

//////////////////////////////////////////////////////////////////////////
void PingFileSevTimer::release()
{
	delete this;
}

void PingFileSevTimer::process()
{
	logic::GetLogic()->pushBackOperationWithLambda(
		[=]()
	{
		CImPduHeartbeat pduHeartbeat;
		m_pFileTransSocket->sendPacket(&pduHeartbeat);
	}
		);
}

PingFileSevTimer::PingFileSevTimer(FileTransferSocket* pTransSocket)
:m_pFileTransSocket(pTransSocket)
{

}

/******************************************************************************/
