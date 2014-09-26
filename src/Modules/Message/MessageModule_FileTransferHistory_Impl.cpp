/******************************************************************************* 
 *  @file      MessageModule_FileTransferHistory_Impl.cpp 2014\9\19 9:16:57 $
 *  @author    ´ó·ð<dafo@mogujie.com>
 *  @brief     
 ******************************************************************************/
#include "stdafx.h"
#include "MessageModule_Impl.h"
#include "utility/CppSQLite3.h"
#include "utility/utilStrCodeAPI.h"
#include "Modules/IMiscModule.h"

#include "ReceiveMsgManage.h"
#include "SendMsgManage.h"
#include "TTLogic/ITcpClientModule.h"
#include "json/writer.h"
#include "json/reader.h"
#include "src/base/ImPduFile.h"
/******************************************************************************/
namespace
{
	const std::string createFileTransferHistorySql =
		"CREATE TABLE IF NOT EXISTS fileTransferMsg"
		"("
		"    [id]                           INTEGER PRIMARY KEY,"
		"    [taskid]						TEXT NOT NULL,"
		"    [fromid]						TEXT DEFAULT NULL,"
		"    [filename]						TEXT DEFAULT NULL,"
		"    [reserve1]                     TEXT DEFAULT NULL,"
		"    [reserve2]                     INTEGER DEFAULT NULL,"
		"    [reserve3]                     INTEGER DEFAULT NULL,"
		"    [savepath]                     TEXT NOT NULL,"
		"    [filesize]                     INTEGER DEFAULT 0 NOT NULL,"
		"    [finishtime]                   INTEGER DEFAULT 0 NOT NULL"
		");";

	const std::string insertFileTransferHistorySql
		= "INSERT INTO fileTransferMsg(taskid, fromid,filename,reserve1,reserve2,reserve3,savepath,filesize,finishtime) "
		"VALUES(?, ? ,? ,? ,?,?,?,?,?);";
	const std::string getFileTransferHistoryBySIdSql
		= "select * from fileTransferMsg order by id desc limit ?";
}

BOOL MessageModule_Impl::execFileTransferHistoryDML()
{
	try
	{
		m_pHistoryMSGDB->execDML(createFileTransferHistorySql.c_str());
	}
	catch (CppSQLite3Exception& sqliteException)
	{
		CString csErrMsg = util::stringToCString(sqliteException.errorMessage(), CP_UTF8);
		APP_LOG(LOG_ERROR, TRUE, _T("MessageModule_Impl execFileTransferHistoryDML failed,error msg:%s")
			, csErrMsg);
#ifdef _DEBUG
		MessageBoxA(0, sqliteException.errorMessage(), "BD ERROR", MB_OK | MB_ICONHAND);
#endif	
		return FALSE;
	}
	catch (...)
	{
		APP_LOG(LOG_ERROR, _T("MessageModule_Impl execFileTransferHistoryDML failed"));
		return FALSE;
	}

	APP_LOG(LOG_INFO, _T("MessageModule_Impl execFileTransferHistoryDML done"));
	return TRUE;
}

BOOL MessageModule_Impl::sqlInsertFileTransferHistory(IN TransferFileEntity& fileInfo)
{
	if (fileInfo.nClientMode == CLIENT_OFFLINE_UPLOAD
		|| fileInfo.nClientMode == CLIENT_REALTIME_SENDER)
	{
		APP_LOG(LOG_INFO, _T("MessageModule_Impl sqlInsertFileTransferHistory-fileInfo.nClientMode not fixed"));
		return FALSE;
	}
	try
	{
		CppSQLite3Statement stmt = m_pHistoryMSGDB->compileStatement(insertFileTransferHistorySql.c_str());
		stmt.bind(1, fileInfo.sTaskID.c_str());
		stmt.bind(2, fileInfo.sFromID.c_str());
		std::string filename = util::cStringToString(fileInfo.getRealFileName());
		stmt.bind(3, filename.c_str());
		std::string savePath = util::cStringToString(fileInfo.getSaveFilePath());
		stmt.bind(7, savePath.c_str());
		stmt.bind(8, (Int32)fileInfo.nFileSize);
		stmt.bind(9, time(0));
		stmt.execDML();
	}

	catch (CppSQLite3Exception& sqliteException)
	{
#ifdef _DEBUG
		MessageBoxA(0, sqliteException.errorMessage(), "BD ERROR", MB_OK | MB_ICONHAND);
#endif
		CString csErrMsg = util::stringToCString(sqliteException.errorMessage(), CP_UTF8);
		APP_LOG(LOG_ERROR, _T("MessageModule_Impl sqlInsertFileTransferHistoryMsg failed,error msg:%s"), csErrMsg);
		return FALSE;
	}
	catch (...)
	{
		APP_LOG(LOG_ERROR, _T("MessageModule_Impl sqlInsertFileTransferHistoryMsg unknown exception"));
		return FALSE;
	}

	return TRUE;
}

BOOL MessageModule_Impl::sqlGetFileTransferHistory(OUT std::vector<TransferFileEntity>& fileList)
{
	try
	{
		CppSQLite3Statement stmt;
		stmt = m_pHistoryMSGDB->compileStatement(getFileTransferHistoryBySIdSql.c_str());
		stmt.bind(1, 20);

		CppSQLite3Query query = stmt.execQuery();
		while (!query.eof())
		{
			TransferFileEntity fileInfo;
			fileInfo.sTaskID = query.getStringField(1);
			fileInfo.sFromID = query.getStringField(2);
			fileInfo.sFileName = query.getStringField(3);
			CString strSavePath = util::stringToCString(query.getStringField(7));
			fileInfo.setSaveFilePath(strSavePath);
			fileInfo.nFileSize = query.getIntField(8);
			fileInfo.time = query.getIntField(9);
			fileList.push_back(fileInfo);
			query.nextRow();
		}
	}
	catch (CppSQLite3Exception& sqliteException)
	{
#ifdef _DEBUG
		MessageBoxA(0, sqliteException.errorMessage(), "BD ERROR", MB_OK | MB_ICONHAND);
#endif
		CString csErrMsg = util::stringToCString(sqliteException.errorMessage(), CP_UTF8);
		APP_LOG(LOG_ERROR, _T("MessageModule_Impl db sqlGetFileTransferHistoryMsg failed,error msg:%s"),
			csErrMsg);
		return FALSE;
	}
	catch (...)
	{
		return FALSE;
	}

	return TRUE;
}

/******************************************************************************/