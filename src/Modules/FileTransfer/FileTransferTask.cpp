/******************************************************************************* 
 *  @file      FileTransferTask.cpp 2014\4\10 13:26:42 $
 *  @author    快刀<kuaidao@mogujie.com>
 *  @summary   
 ******************************************************************************/

#include "stdafx.h"
#include "FileObject.h"
#include "FileTransferTask.h"
#include "FileTransferSocket.h"
#include "TransferManager.h"
#include "TTLogic/ILogic.h"
#include "Modules/IFileTransferModule.h"
#include "Modules/ISysConfigModule.h"
#include "src/base/ImPduFile.h"
#include "utility/utilCommonAPI.h"

/******************************************************************************/
// -----------------------------------------------------------------------------
//  FileTransferTask: Public, Constructor

FileReceiveTask::FileReceiveTask(const std::string& sfId)
:FileTransTaskBase(sfId)
{

}

// -----------------------------------------------------------------------------
//  FileTransferTask: Public, Destructor

FileReceiveTask::~FileReceiveTask()
{

}

void FileReceiveTask::release()
{
    delete this;
}

void FileReceiveTask::process()
{
 //   TransferFileEntity info;
 //   TransferFileEntityManager::getInstance()->getFileInfoBySId(m_sTaskId,info);    
	//logic::GetLogic()->pushBackOperationWithLambda(
	//	[=]()
	//{
	//	CImPduClientFileState pduTransferFileMsg(
	//		CLIENT_FILE_PEER_READY,
	//		info.sTaskID.c_str(),
	//		info.sToID.c_str()
	//		);
	//	logic::getTcpClientModule()->sendPacket(&pduTransferFileMsg);
	//}
	//);

 //   MSG msg;
 //   while(::GetMessage(&msg, NULL, 0, 0) > 0)
 //   {
 //       ::TranslateMessage(&msg);
 //       ::DispatchMessage(&msg);
 //   }
}

/******************************************************************************/

FileTransTaskBase::FileTransTaskBase(const std::string& sfId)
:m_pFileTransSocket(0)
,m_sTaskId(sfId)
,m_bContinue(TRUE)
{

}

FileTransTaskBase::~FileTransTaskBase()
{

}

void FileTransTaskBase::release()
{
    delete this;
}

void FileSendTask::process()
{
	TransferFileEntity info;
	if (!TransferFileEntityManager::getInstance()->getFileInfoBySId(m_sTaskId, info))
		return;

	//拉模式，传输taskid、token、client_mode
	CImPduClientFileLoginReq pduFileLoginReq(module::getSysConfigModule()->userID().c_str()
		, "", info.sTaskID.c_str(), CLIENT_OFFLINE_UPLOAD);
	m_pFileTransSocket->sendPacket(&pduFileLoginReq);
	util::messagePump();

//    TransferFileEntity info;
//	if (!TransferFileEntityManager::getInstance()->getFileInfoBySId(m_sfId, info))
//        return;
//    CString csFileId = util::stringToCString(info.sTaskID);
//
//    FileObject* pObject = new FileObject;
//    HANDLE hFile = 0;
//    try
//    {
//		hFile = pObject->OpenSendFile(util::stringToCString(info.sFileName, CP_UTF8));
//        if(!hFile)
//        {
//            APP_LOG(LOG_ERROR,TRUE,_T("FileSendTask open file failed,%s"),csFileId);
//            TransferFileEntityManager::getInstance()->kickMapFileItemToVecFile(info.sTaskID);
//			logic::GetLogic()->asynNotifyObserver(module::KEY_FILESEVER_UPDATA_FAILED, info.sTaskID);
//            goto exit;
//        }
//
//        LPVOID lpBufferRead = NULL;
//        DWORD dwBytesRead = 0;
//        UInt32 progressRefreshMark = 0;
//        do 
//        {
//            if(!m_bContinue)
//            {
//                APP_LOG(LOG_INFO,_T("FileSendTask continue false and exit,%s"),csFileId);
//				TransferFileEntityManager::getInstance()->kickMapFileItemToVecFile(info.sTaskID);
//				logic::GetLogic()->asynNotifyObserver(module::KEY_FILESEVER_UPDATA_FAILED, info.sTaskID);
//                goto exit;
//            }
//
//            lpBufferRead = NULL;
//            dwBytesRead = 0;
//            if(!pObject->GetSendFileBlock(lpBufferRead,dwBytesRead))
//            {
//                APP_LOG(LOG_INFO,_T("FileSendTask GetSendFileBlock failed,%s"),csFileId);
//				TransferFileEntityManager::getInstance()->kickMapFileItemToVecFile(info.sTaskID);
//				logic::GetLogic()->asynNotifyObserver(module::KEY_FILESEVER_UPDATA_FAILED, info.sTaskID);
//                goto exit;
//            }
//            if (lpBufferRead != NULL && dwBytesRead != 0)
//            { 
//                //CImPduClientFileData pduSendFileDataMsg(info.sFromID.c_str()
//                //    ,info.sToID.c_str()
//                //    ,info.sFileName.c_str()
//                //    ,info.nFileSize
//                //    ,pObject->GetFIleOffset()
//                //    ,1
//                //    ,dwBytesRead
//                //    ,(uchar_t*)lpBufferRead);
//                //m_pFileTransSocket->sendPacket(&pduSendFileDataMsg);
//
//                if(++progressRefreshMark % 5 == 0)
//                {
//                    info.nProgress = pObject->GetFIleOffset();
//					TransferFileEntityManager::getInstance()->updateFileInfoBysTaskID(info);
//					logic::GetLogic()->asynNotifyObserver(module::KEY_FILESEVER_UPDATA_PROGRESSBAR, info.sTaskID);
//                }
//            }
//        } while (0 != dwBytesRead);
//    }
//    catch(...)
//    {
//        APP_LOG(LOG_ERROR,_T("FileSendTask unknwon exception,%s"),csFileId);
//		TransferFileEntityManager::getInstance()->kickMapFileItemToVecFile(info.sTaskID);
//		//更新进度条
//		logic::GetLogic()->asynNotifyObserver(module::KEY_FILESEVER_UPDATA_FAILED, info.sTaskID);
//        goto exit;
//    }
//
//    APP_LOG(LOG_INFO,_T("FileSendTask send succ,%s"),csFileId);
//	TransferFileEntityManager::getInstance()->kickMapFileItemToVecFile(info.sTaskID);
//	//删除进度条
//	logic::GetLogic()->asynNotifyObserver(module::KEY_FILESEVER_PROGRESSBAR_FINISHED, info.sTaskID);
//
//	//离线文件传输 
//	if (3 == info.nSendType)
//    {
//		logic::GetLogic()->asynNotifyObserver(module::KEY_FILESEVER_UPLOAD_OFFLINE_FINISH, info.sTaskID);
//    }
//
//exit:
//    if(INVALID_HANDLE_VALUE != hFile)
//        pObject->CloseSendFile();
//    delete pObject;
//    pObject = 0;
}

void FileSendTask::release()
{
    delete this;
}

FileSendTask::FileSendTask( const std::string& sfId )
:FileTransTaskBase(sfId)
{
    
}

FileSendTask::~FileSendTask()
{
}

OfflineFileReqTask::OfflineFileReqTask( const std::string& sfId )
:FileTransTaskBase(sfId)
{

}

OfflineFileReqTask::~OfflineFileReqTask()
{

}

void OfflineFileReqTask::process()
{
	TransferFileEntity info;
	if (!TransferFileEntityManager::getInstance()->getFileInfoBySId(m_sTaskId, info))
		return;

    //CImPduClientFileGetOfflineReq pduClientFileGetOfflineReqMsg(info.sFromID.c_str(),info.sPathOfflineFileOnSev.c_str());
    //m_pFileTransSocket->sendPacket(&pduClientFileGetOfflineReqMsg);

    MSG msg;
    while(::GetMessage(&msg, NULL, 0, 0) > 0)
    {
        ::TranslateMessage(&msg);
        ::DispatchMessage(&msg);
    }
}

void OfflineFileReqTask::release()
{
    delete this;
}
