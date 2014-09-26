/******************************************************************************* 
 *  @file      FileTransfer_Impl.cpp 2014\8\26 11:53:09 $
 *  @author    快刀<kuaidao@mogujie.com>
 *  @brief     
 ******************************************************************************/

#include "stdafx.h"
#include "src/base/ImPduFile.h"
#include "TTLogic/ITcpClientModule.h"
#include "Modules/ISysConfigModule.h"
#include "FileTransferModule_Impl.h"
#include "FileTransferDialog.h"
#include "TransferFile.h"
#include "utility/Multilingual.h"
#include "FileTransfer/TransferManager.h"

/******************************************************************************/
namespace module
{
	IFileTransferModule* getFileTransferModule()
	{
		return (IFileTransferModule*)logic::GetLogic()->getModule(MODULE_ID_FILETRANSFER);
	}
}

// -----------------------------------------------------------------------------
//  FileTransfer_Impl: Public, Constructor

FileTransferModule_Impl::FileTransferModule_Impl()
:m_pFileTransferDialog(0)
{
	
}

// -----------------------------------------------------------------------------
//  FileTransfer_Impl: Public, Destructor

FileTransferModule_Impl::~FileTransferModule_Impl()
{

}

void FileTransferModule_Impl::onPacket(std::auto_ptr<CImPdu> pdu)
{
	CImPdu* pPdu = pdu.get();
	PTR_VOID(pPdu);
	switch (pdu->GetCommandId())
	{
	case CID_FILE_RESPONSE://发送“文件请求/离线文件”-返回
		_sendfileResponse(pPdu);
		break;
	case CID_FILE_NOTIFY://收到“发送文件请求”
		_fileNotify(pPdu);
		break;
	case CID_FILE_HAS_OFFLINE_RES:
		_hasOfflineRes(pPdu);
		break;
	default:
		return;
	}
}

void FileTransferModule_Impl::release()
{
	delete this;
}

void FileTransferModule_Impl::showFileTransferDialog()
{
	BOOL bRet = FALSE;
	if (!m_pFileTransferDialog)
	{
		m_pFileTransferDialog = new FileTransferDialog;
		CString csTitle = util::getMultilingual()->getStringViaID(_T("STRID_FILETRANSFERDIALOG_CAPTION"));
		m_pFileTransferDialog->Create(NULL, csTitle, UI_CLASSSTYLE_DIALOG, WS_EX_STATICEDGE | WS_EX_APPWINDOW, 0, 0, 0, 0);
		m_pFileTransferDialog->CenterWindow();
	}

	m_pFileTransferDialog->BringToTop();
}
void FileTransferModule_Impl::OnFileTransferModuleEvent(UInt16 moduleId, UInt32 keyId, MKO_TUPLE_PARAM mkoParam)
{
	if (module::KEY_FILETRANSFER_REQUEST == keyId
		|| module::KEY_FILETRANSFER_SENDFILE == keyId)
	{
		if (m_pFileTransferDialog)
		{
			std::string& sFileId = std::get<MKO_STRING>(mkoParam);
			m_pFileTransferDialog->AddTransderItemToList(sFileId);
			m_pFileTransferDialog->BringToTop();
		}
		else
		{
			showFileTransferDialog();
		}
	}
}

BOOL FileTransferModule_Impl::sendFile(IN const CString& sFilePath, IN const std::string& sSendToSID,IN BOOL bOnlineMode)
{
	if (TransferFileEntityManager::getInstance()->checkIfIsSending(sFilePath))
	{
		return FALSE;
	}
	TransferFileEntity fileEntity;
	fileEntity.nFileSize = (UInt32)util::getFileSize(sFilePath);
	if (0 != fileEntity.nFileSize)
	{
		CString strFileName = sFilePath;
		strFileName.Replace(_T("\\"), _T("/"));//mac上对于路径字符“\”需要做特殊处理，windows上则可以识别
		fileEntity.sFileName = util::cStringToString(strFileName);
		fileEntity.sFromID = module::getSysConfigModule()->userID();
		fileEntity.sToID = sSendToSID;
		uint32_t transMode = 0;
		transMode = bOnlineMode ? FILE_TYPE_ONLINE : FILE_TYPE_OFFLINE;
		
		APP_LOG(LOG_DEBUG,_T("FileTransferSevice_Impl::sendFile sTaskID = %s"), util::stringToCString(fileEntity.sTaskID));

		logic::GetLogic()->pushBackOperationWithLambda(
			[=]()
		{
			CImPduClientFileRequest pduSendFileRequestMsg(fileEntity.sFromID.c_str()
				, fileEntity.sToID.c_str()
				, fileEntity.sFileName.c_str()
				, fileEntity.nFileSize
				, transMode);
			logic::getTcpClientModule()->sendPacket(&pduSendFileRequestMsg);
		});
		
		return TRUE;
	}
	return FALSE;
}

void FileTransferModule_Impl::_sendfileResponse(CImPdu* pdu)
{
	CImPduClientFileResponse* pduData = (CImPduClientFileResponse*)pdu;
	UInt32 nResult = pduData->GetResult();//成功，失败
	if (nResult != 0)
	{
		APP_LOG(LOG_ERROR, _T("_sendfileResponse result != 0"));
		logic::GetLogic()->asynNotifyObserver(module::KEY_FILESEVER_UPDATA_FAILED);
	}

	TransferFileEntity fileEntity;
	fileEntity.sTaskID.assign(pduData->GetTaskId(), pduData->GetTaskIdLen());
	fileEntity.sFromID.assign(pduData->GetFromId(),pduData->GetFromIdLen());
	fileEntity.sToID.assign(pduData->GetToId(), pduData->GetToIdLen());
	fileEntity.sFileName.assign(pduData->GetFileName(), pduData->GetFileNameLen());
	fileEntity.setSaveFilePath(util::stringToCString(fileEntity.sFileName));//发送方文件地址，就是保存地址
	fileEntity.time = static_cast<UInt32>(time(0));
	uint32_t transMode = pduData->GetTransMode();
	if (FILE_TYPE_ONLINE == transMode)
	{
		fileEntity.nClientMode = CLIENT_REALTIME_SENDER;
	}
	else if (FILE_TYPE_OFFLINE == transMode)
	{
		fileEntity.nClientMode = CLIENT_OFFLINE_UPLOAD;
	}
	fileEntity.pFileObject = new TransferFile(util::stringToCString(fileEntity.sFileName),FALSE);
	if (fileEntity.pFileObject)
	{
		fileEntity.nFileSize = fileEntity.pFileObject->length();
	}
	
	UINT32 nIPCount = pduData->GetIPAddrCnt();
	ip_addr_t* pIPList = pduData->GetIPAddrList();
	if (nIPCount <= 0
		|| NULL == pIPList)
	{
		return;
	}
	fileEntity.sIP.assign(pIPList[0].ip, pIPList[0].ip_len);
	fileEntity.nPort = pIPList[0].port;

	if (!TransferFileEntityManager::getInstance()->pushTransferFileEntity(fileEntity))
		TransferFileEntityManager::getInstance()->updateFileInfoBysTaskID(fileEntity);

	APP_LOG(LOG_DEBUG, _T("FileTransferSevice_Impl::准备连接文件服务器 sTaskId = %s"), util::stringToCString(fileEntity.sTaskID));
	TransferFileEntityManager::getInstance()->openFileSocketByTaskId(fileEntity.sTaskID);
}

void FileTransferModule_Impl::_fileNotify(CImPdu* pdu)
{
	CImPduClientFileNotify* pduData = (CImPduClientFileNotify*)pdu;
	TransferFileEntity file;
	if (pduData->GetFileName())
	{
		file.sFileName.assign(pduData->GetFileName(), pduData->GetFileNameLen());
	}
	if (pduData->GetFromId())
	{
		file.sFromID.assign(pduData->GetFromId(), pduData->GetFromIdLen());
	}
	if (pduData->GetToId())
	{
		file.sToID.assign(pduData->GetToId(), pduData->GetToIdLen());
	}
	if (pduData->GetTaskId())
	{
		file.sTaskID.assign(pduData->GetTaskId(), pduData->GetTaskIdLen());
	}
	file.nFileSize = pduData->GetFileSize();

	UINT32 nIPCount = pduData->GetIPAddrCnt();
	ip_addr_t* pIPList = pduData->GetIPAddrList();
	if (nIPCount <= 0
		|| NULL == pIPList)
	{
		return;
	}
	file.sIP.assign(pIPList[0].ip, pIPList[0].ip_len);
	file.nPort = pIPList[0].port;

	uint32_t transMode = pduData->GetTransMode();
	if (FILE_TYPE_ONLINE == transMode)
	{
		file.nClientMode = CLIENT_REALTIME_RECVER;
	}
	else if (FILE_TYPE_OFFLINE == transMode)
	{
		file.nClientMode = CLIENT_OFFLINE_DOWNLOAD;
	}
	file.time = static_cast<UInt32>(time(0));
	TransferFileEntityManager::getInstance()->pushTransferFileEntity(file);
	APP_LOG(LOG_DEBUG, _T("FileTransferSevice_Impl::给你发文件 sFileID = %s"), util::stringToCString(file.sTaskID));

	if (1 == pduData->GetOfflineReady())
	{
		//TODO离线文件传输结束
	}

	//连接服务器
	TransferFileEntityManager::getInstance()->openFileSocketByTaskId(file.sTaskID);
}

BOOL FileTransferModule_Impl::acceptFileTransfer(IN const std::string& sTaskId, IN const BOOL bAccept /*= TRUE*/)
{
	TransferFileEntity FileInfo;
	if (TransferFileEntityManager::getInstance()->getFileInfoByTaskId(sTaskId, FileInfo))
	{
		if (bAccept)//接收文件
		{
			FileInfo.pFileObject = new TransferFile(FileInfo.getSaveFilePath(), TRUE);
			FileInfo.time = static_cast<UInt32>(time(0));
			TransferFileEntityManager::getInstance()->updateFileInfoBysTaskID(FileInfo);
			APP_LOG(LOG_DEBUG, _T("FileTransferSevice_Impl::acceptFileTransfer 接收文件 sFileID = %s"), util::stringToCString(FileInfo.sTaskID));
			TransferFileEntityManager::getInstance()->acceptFileTransfer(std::string(sTaskId));
		}
		else//拒绝文件
		{
			if (CLIENT_REALTIME_RECVER == FileInfo.nClientMode)//在线文件
			{
				APP_LOG(LOG_DEBUG, _T("FileTransferSevice_Impl::acceptFileTransfer 拒绝文件 sFileID = %s"), util::stringToCString(FileInfo.sTaskID));
				TransferFileEntityManager::getInstance()->rejectFileTransfer(sTaskId);
			}
			else if (CLIENT_OFFLINE_DOWNLOAD == FileInfo.nClientMode)//离线文件
			{
				APP_LOG(LOG_DEBUG, _T("FileTransferSevice_Impl::acceptFileTransfer 拒绝离线文件 sFileID = %s"), util::stringToCString(FileInfo.sTaskID));
				logic::GetLogic()->pushBackOperationWithLambda(
					[=]()
				{
					CImPduClientFileDelOfflineReq pduDelOfflineReqMsg(FileInfo.sFromID.c_str()
						, FileInfo.sToID.c_str()
						, FileInfo.sTaskID.c_str());
					logic::getTcpClientModule()->sendPacket(&pduDelOfflineReqMsg);
				});
			}
			logic::GetLogic()->asynNotifyObserver(module::KEY_FILESEVER_UPDATA_REJECT, FileInfo.sTaskID);
		}
		return TRUE;
	}
	return FALSE;
}

BOOL FileTransferModule_Impl::doCancel(IN const std::string& sTaskId)
{
	TransferFileEntity fileinfo;
	if (TransferFileEntityManager::getInstance()->getFileInfoByTaskId(sTaskId, fileinfo))
	{
		TransferFileEntityManager::getInstance()->cancelFileTransfer(sTaskId);
		return TRUE;
	}
	return FALSE;
}

void FileTransferModule_Impl::_hasOfflineRes(CImPdu* pdu)
{
	CImPduClientFileHasOfflineRes* pduData = (CImPduClientFileHasOfflineRes*)pdu;
	UINT32 nIPCount = pduData->GetIPAddrCnt();
	ip_addr_t* pIPList = pduData->GetIPAddrList();
	if (nIPCount <= 0 || NULL == pIPList)
	{
		return;
	}
	UINT32 nFileNum = pduData->GetFileCnt();
	client_offline_file_t* pfileList = pduData->GetFileList();
	for (UINT32 i = nFileNum; i > 0; --i)
	{
		client_offline_file_t OfflineFile = pfileList[i - 1];
		TransferFileEntity fileInfo;
		fileInfo.sFromID.assign(OfflineFile.from_id_url, OfflineFile.from_id_len);
		fileInfo.sToID = module::getSysConfigModule()->userID();
		fileInfo.nFileSize = OfflineFile.file_size;
		fileInfo.sTaskID.assign(OfflineFile.task_id, OfflineFile.task_id_len);
		fileInfo.sFileName.assign(OfflineFile.file_name, OfflineFile.file_name_len);
		fileInfo.nClientMode = CLIENT_OFFLINE_DOWNLOAD;
		fileInfo.sIP.assign(pIPList[0].ip, pIPList[0].ip_len);//只挑第一个
		fileInfo.nPort = pIPList[0].port;
		fileInfo.time = static_cast<UInt32>(time(0));
		if (TransferFileEntityManager::getInstance()->pushTransferFileEntity(fileInfo))
		{
			APP_LOG(LOG_INFO, _T("离线文件 sFileID = %s"), util::stringToCString(fileInfo.sTaskID));
			TransferFileEntityManager::getInstance()->openFileSocketByTaskId(fileInfo.sTaskID);
		}
	}
}

logic::LogicErrorCode FileTransferModule_Impl::onUnLoadModule()
{
	logic::GetLogic()->removeObserver(this);
	std::list<std::string> fileIdList;
	TransferFileEntityManager::getInstance()->getAllTransferFileEntityFileID(fileIdList);
	for (std::string fileId : fileIdList)
	{
		doCancel(fileId);
	}
	TransferFileEntityManager::getInstance()->shutdown();

	return logic::LOGIC_OK;
}

logic::LogicErrorCode FileTransferModule_Impl::onLoadModule()
{
	TransferFileEntityManager::getInstance()->startup();

	logic::GetLogic()->addObserver(this, MODULE_ID_FILETRANSFER
		, fastdelegate::MakeDelegate(this, &FileTransferModule_Impl::OnFileTransferModuleEvent));
	return logic::LOGIC_OK;
}

/******************************************************************************/