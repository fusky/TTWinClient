/******************************************************************************* 
 *  @file      FileTransferUIThread.cpp 2014\9\17 16:32:18 $
 *  @author    快刀<kuaidao@mogujie.com>
 *  @brief     包含多个socket的文件传输UI thread
 ******************************************************************************/

#include "stdafx.h"
#include "FileTransferUIThread.h"
#include "FileTransferSocket.h"
#include "TransferManager.h"
#include "utility/utilCommonAPI.h"
#include "src/base/ImPduFile.h"
#include "src/base/ImPduClient.h"
#include <algorithm>
/******************************************************************************/
namespace
{
	#define LOGIC_EVNET_MSG             _T("___LogicEventDisptacherMessageTest")
	#define LOGIC_WINDOW_CLASSNAME      _T("___LogicEventDisptacherWndClassTest")
	#define LOGIC_WINDOW_NAME           _T("")
	const UInt32 FILE_TRANSFER_BLOCK_SIZE = 1024 * 40;
}

// -----------------------------------------------------------------------------
//  FileTransferUIThread: Public, Constructor

FileTransferUIThread::FileTransferUIThread()
:m_hWnd(0)
{
}

// -----------------------------------------------------------------------------
//  FileTransferUIThread: Public, Destructor

FileTransferUIThread::~FileTransferUIThread()
{

}

void FileTransferUIThread::Shutdown()
{
	::PostThreadMessage(getThreadId(), WM_QUIT, 0, 0);
	if (!wait(5000))
		destory();
}

void FileTransferUIThread::_releaseWnd()
{
	if (0 != m_hWnd)
	{
		::DestroyWindow(m_hWnd);
		m_hWnd = 0;
	}
}

UInt32 FileTransferUIThread::process()
{
	m_hWnd = _createWnd();
	util::messagePump();

	_closeAllFileSockets();
	_releaseWnd();

	return 0;
}

HWND FileTransferUIThread::_createWnd()
{
	HWND hwnd = 0;

	WNDCLASS wc;
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = _WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = ::AfxGetInstanceHandle();
	wc.hIcon = 0;
	wc.hCursor = 0;
	wc.hbrBackground = 0;
	wc.lpszMenuName = 0;
	wc.lpszClassName = LOGIC_WINDOW_CLASSNAME;

	if (!::RegisterClass(&wc))
		return 0;
	hwnd = ::CreateWindowEx(0, LOGIC_WINDOW_CLASSNAME, LOGIC_WINDOW_NAME,
		0, 0, 0, 1, 1, HWND_MESSAGE, 0, 0, 0);

	return hwnd;
}

LRESULT _stdcall FileTransferUIThread::_WndProc(HWND hWnd, UINT message, WPARAM wparam, LPARAM lparam)
{
	if (WM_FILE_TRANSFER == message)
	{
		FileTransferSocket* pFileSocket = (FileTransferSocket*)lparam;
		pFileSocket->startFileTransLink();
	}

	return ::DefWindowProc(hWnd, message, wparam, lparam);
}

void FileTransferUIThread::openFileSocketByTaskId(std::string& taskId)
{
	FileTransferSocket* pFileSocket = _findFileSocketByTaskId(taskId);
	if (!pFileSocket)
	{
		pFileSocket = new FileTransferSocket(taskId);
		m_lstFileTransSockets.push_back(pFileSocket);
		::PostMessage(m_hWnd, WM_FILE_TRANSFER, 0, (LPARAM)pFileSocket);
	}
}

void FileTransferUIThread::closeFileSocketByTaskId(std::string& taskId)
{
	TTAutoLock lock(&m_lock);
	auto fileSockIter = std::remove_if(m_lstFileTransSockets.begin()
		,m_lstFileTransSockets.end()
		,[=](FileTransferSocket* pFileSocket)
	{
		return(taskId == pFileSocket->m_sTaskId);
	});
	if (fileSockIter != m_lstFileTransSockets.end())
	{
		FileTransferSocket* pFileSocket = *fileSockIter;
		pFileSocket->stopfileTransLink();
		delete pFileSocket;
		pFileSocket = 0;
		m_lstFileTransSockets.erase(fileSockIter, m_lstFileTransSockets.end());
	}
}

BOOL FileTransferUIThread::acceptFileTransfer(const std::string& taskId)
{
	FileTransferSocket* pFileSocket = _findFileSocketByTaskId(taskId);
	if (pFileSocket)
	{
		logic::GetLogic()->pushBackOperationWithLambda(
			[=]()
		{
			TransferFileEntity fileEntity;
			if (TransferFileEntityManager::getInstance()->getFileInfoByTaskId(taskId, fileEntity))
			{
				int mode = fileEntity.nClientMode == CLIENT_OFFLINE_DOWNLOAD ? FILE_TYPE_OFFLINE : FILE_TYPE_ONLINE;
				CImPduClientFilePullDataReq pduPullDataReq(taskId.c_str(), fileEntity.sToID.c_str()
					, mode, 0, FILE_TRANSFER_BLOCK_SIZE);
				pFileSocket->sendPacket(&pduPullDataReq);
			}
		});
	}

	return FALSE;
}

BOOL FileTransferUIThread::rejectFileTransfer(const std::string& taskId)
{
	FileTransferSocket* pFileSocket = _findFileSocketByTaskId(taskId);
	if (pFileSocket)
	{
		logic::GetLogic()->pushBackOperationWithLambda(
			[=]()
		{
			TransferFileEntity fileEntity;
			if (TransferFileEntityManager::getInstance()->getFileInfoByTaskId(taskId, fileEntity))
			{
				CImPduClientFileState pduRejectData(CLIENT_FILE_REFUSE, taskId.c_str(), fileEntity.sToID.c_str());
				pFileSocket->sendPacket(&pduRejectData);
			}
		});
	}

	return FALSE;
}

BOOL FileTransferUIThread::cancelFileTransfer(const std::string& taskId)
{
	FileTransferSocket* pFileSocket = _findFileSocketByTaskId(taskId);
	if (pFileSocket)
	{
		logic::GetLogic()->pushBackOperationWithLambda(
			[=]()
		{
			TransferFileEntity fileEntity;
			if (TransferFileEntityManager::getInstance()->getFileInfoByTaskId(taskId, fileEntity))
			{
				std::string userid;
				if (fileEntity.nClientMode == CLIENT_REALTIME_SENDER
					|| CLIENT_OFFLINE_UPLOAD == fileEntity.nClientMode)
				{
					userid = fileEntity.sFromID;
				}
				else
				{
					userid = fileEntity.sToID;
				}
				CImPduClientFileState pduRejectData(CLIENT_FILE_CANCEL, taskId.c_str(), userid.c_str());
				pFileSocket->sendPacket(&pduRejectData);
			}
		});
	}

	return FALSE;
}

FileTransferSocket* FileTransferUIThread::_findFileSocketByTaskId(const std::string& taskId)
{
	TTAutoLock lock(&m_lock);
	auto iter = std::find_if(m_lstFileTransSockets.begin(),m_lstFileTransSockets.end()
		, [=](FileTransferSocket* fileSock)
	{
		return (taskId == fileSock->m_sTaskId);
	});

	if (iter != m_lstFileTransSockets.end())
	{
		FileTransferSocket* fileSocket = *iter;
		return fileSocket;
	}

	return 0;
}

void FileTransferUIThread::_closeAllFileSockets()
{
	for (FileTransferSocket* pFileSock : m_lstFileTransSockets)
	{
		pFileSock->stopfileTransLink();
		delete pFileSock;
		pFileSock = 0;
	}
	m_lstFileTransSockets.clear();
}

/******************************************************************************/