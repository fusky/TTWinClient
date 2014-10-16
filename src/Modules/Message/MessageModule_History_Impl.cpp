/******************************************************************************* 
 *  @file      HistoryMsgModule_Impl.cpp 2014\8\3 11:14:33 $
 *  @author    快刀<kuaidao@mogujie.com>
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
#include "Modules/MessageEntity.h"
#include "json/writer.h"
#include "json/reader.h"
/******************************************************************************/
namespace
{
	const std::string createHistoryMSGSql =
		"CREATE TABLE IF NOT EXISTS immessage"
		"("
		"    [id]                           INTEGER PRIMARY KEY,"
		"    [sessionid]                    TEXT NOT NULL,"
		"    [talkerid]						TEXT DEFAULT NULL,"
		"    [reserve1]                     TEXT DEFAULT NULL,"
		"    [reserve2]                     INTEGER DEFAULT NULL,"
		"    [reserve3]                     INTEGER DEFAULT NULL,"
		"    [content]                      TEXT NOT NULL,"
		"    [rendertype]                   INTEGER DEFAULT 2 NOT NULL,"
		"    [sessiontype]                  INTEGER DEFAULT 1 NOT NULL,"
		"    [msgtime]                      INTEGER DEFAULT 0 NOT NULL,"
		"    [createtime]                   INTEGER NOT NULL"
		");";
	const std::string createHistoryMsgIndex =
		"CREATE INDEX IF NOT EXISTS sessionid_idx ON immessage(sessionid);";

	const std::string insertHistoryMSGSql
		= "INSERT INTO immessage(sessionId, talkerid,reserve1,reserve2,reserve3,content,rendertype,sessiontype,msgtime,createtime) "
		"VALUES(?, ? ,? ,? ,?,?,?,?,?,?);";
	const std::string getHistoryMSGBySIdSql
		= "select * from immessage where sessionId=? order by id desc limit ?,?";

	const std::string BeginInsert
		= "BEGIN TRANSACTION;";
	const std::string EndInsert
		= "COMMIT TRANSACTION;";

	const std::string RollBack
		= "ROLLBACK TRANSACTION";
}

BOOL MessageModule_Impl::openDB()
{
	CString dbHisMSGPath = module::getMiscModule()->getTTCommonAppdataUserDir();
	util::createAllDirectories(dbHisMSGPath);
	dbHisMSGPath += _T("\\msg.db");
	std::string path = util::cStringToString(dbHisMSGPath, CP_UTF8);

	assert(m_pHistoryMSGDB);
	try
	{
		m_pHistoryMSGDB->open(path.c_str());
		m_pHistoryMSGDB->execDML(createHistoryMSGSql.c_str());
		m_pHistoryMSGDB->execDML(createHistoryMsgIndex.c_str());
		execFileTransferHistoryDML();
	}
	catch (CppSQLite3Exception& sqliteException)
	{
		CString csErrMsg = util::stringToCString(sqliteException.errorMessage(), CP_UTF8);
		APP_LOG(LOG_ERROR, TRUE, _T("MessageModule_Impl open database failed,error msg:%s")
			, csErrMsg);
#ifdef _DEBUG
		MessageBoxA(0, sqliteException.errorMessage(), "BD ERROR", MB_OK | MB_ICONHAND);
#endif	
		return FALSE;
	}
	catch (...)
	{
		APP_LOG(LOG_ERROR, _T("MessageModule_Impl open database failed"));
		return FALSE;
	}

	APP_LOG(LOG_INFO, _T("MessageModule_Impl open database done"));
	return TRUE;
}

void MessageModule_Impl::_closeDB()
{
	try
	{
		m_pHistoryMSGDB->close();
	}
	catch (CppSQLite3Exception& sqliteException)
	{
		CString csErrMsg = util::stringToCString(sqliteException.errorMessage(), CP_UTF8);
		APP_LOG(LOG_ERROR, TRUE, _T("MessageModule_Impl close  database failed,error msg:%s")
			, csErrMsg);
#ifdef _DEBUG
		MessageBoxA(0, sqliteException.errorMessage(), "BD ERROR", MB_OK | MB_ICONHAND);
#endif
		return;
	}
	catch (...)
	{
		APP_LOG(LOG_ERROR, _T("MessageModule_Impl close database failed"));
		return;
	}
	APP_LOG(LOG_INFO, _T("MessageModule_Impl close database done"));
}

void MessageModule_Impl::countMsgOffset(const std::string& sId, Int32 v)
{
	auto iter = m_mapMsgOffset.find(sId);
	if (iter != m_mapMsgOffset.end())
	{
		iter->second += v;
	}
	else
	{
		m_mapMsgOffset[sId] = v;
	}
}

void MessageModule_Impl::clearMsgOffset(const std::string& sId)
{
	auto iter = m_mapMsgOffset.find(sId);
	if (iter == m_mapMsgOffset.end())
		return;
	m_mapMsgOffset[sId] = 0;
}

BOOL MessageModule_Impl::sqlInsertHistoryMsg(IN MessageEntity& msg)
{
	if (logic::TCPCLIENT_STATE_DISCONNECT == logic::getTcpClientModule()->getTcpClientNetState()
		|| MESSAGE_RENDERTYPE_SYSTEMTIPS == msg.msgRenderType)
	{
		return FALSE;
	}
	try
	{
		CppSQLite3Statement stmt = m_pHistoryMSGDB->compileStatement(insertHistoryMSGSql.c_str());
		stmt.bind(1, msg.sessionId.c_str());
		stmt.bind(2, msg.talkerSid.c_str());
		//对语音消息做个特殊处理，content存储的是json格式字符串
		if (MESSAGE_RENDERTYPE_AUDIO == msg.msgRenderType)
		{
			Json::Value root;
			root["msgAudioTime"] = msg.msgAudioTime;
			root["msgAudioId"] = msg.content;
			Json::FastWriter fstWrite;
			std::string audioContent = fstWrite.write(root);
			stmt.bind(6, audioContent.c_str());
		}
		else
		{
			stmt.bind(6, msg.content.c_str());
		}
		stmt.bind(7, msg.msgRenderType);
		stmt.bind(8, msg.msgFromType);
		stmt.bind(9, (Int32)msg.msgTime);
		stmt.bind(10, time(0));
		stmt.execDML();
	}

	catch (CppSQLite3Exception& sqliteException)
	{
#ifdef _DEBUG
		MessageBoxA(0, sqliteException.errorMessage(), "BD ERROR", MB_OK | MB_ICONHAND);
#endif
		CString csErrMsg = util::stringToCString(sqliteException.errorMessage(), CP_UTF8);
		APP_LOG(LOG_ERROR, _T("MessageModule_Impl db sqlInsertHistoryMsg failed,error msg:%s"), csErrMsg);
		_msgToTrace(msg);
		return FALSE;
	}
	catch (...)
	{
		APP_LOG(LOG_ERROR, _T("MessageModule_Impl db sqlInsertHistoryMsg unknown exception"));
		_msgToTrace(msg);
		return FALSE;
	}

	return TRUE;
}

void MessageModule_Impl::_msgToTrace(const MessageEntity& msg)
{
	CTime time(msg.msgTime);
	CString csTime = time.Format(_T("%Y-%m-%d %H:%M:%S"));

	APP_LOG(LOG_ERROR,
		TRUE,
		_T("msgType = %d,msgRenderType = %d,msgSessionType = %d,msgTime = %s,content = %s,imageId = %s,sessionId = %s"),
		msg.msgType,
		msg.msgRenderType,
		msg.msgFromType,
		csTime,
		util::stringToCString(msg.content),
		util::stringToCString(msg.imageId),
		util::stringToCString(msg.sessionId));
}

BOOL MessageModule_Impl::sqlGetHistoryMsg(IN std::string sId,IN UInt32 nMsgCount, OUT std::vector<MessageEntity>& msgList)
{
	std::map<std::string, UInt32>::iterator iter
		= m_mapMsgOffset.find(sId);
	UInt32 offset = 0;
	if (iter != m_mapMsgOffset.end())
	{
		offset = iter->second;
	}

	try
	{
		CppSQLite3Statement stmt;
		stmt = m_pHistoryMSGDB->compileStatement(getHistoryMSGBySIdSql.c_str());
		stmt.bind(1, sId.c_str());
		stmt.bind(2, (int)offset);
		stmt.bind(3, (int)nMsgCount);

		CppSQLite3Query query = stmt.execQuery();
		while (!query.eof())
		{
			MessageEntity msg;
			msg.msgType = MESSAGE_TYPE_HISTORY;
			msg.msgRenderType = query.getIntField(7);
			//对语音消息做个特殊处理，content存储的是json格式字符串
			if (MESSAGE_RENDERTYPE_AUDIO == msg.msgRenderType)
			{
				std::string jsonAudioContent = query.getStringField(6);
				Json::Reader reader;
				Json::Value root;
				if (reader.parse(jsonAudioContent, root))
				{
					msg.msgAudioTime = (root.get("msgAudioTime", "")).asUInt();
					msg.content = (root.get("msgAudioId", "")).asString();
				}
			}
			else
			{
				msg.content = query.getStringField(6);
			}
			msg.msgFromType = query.getIntField(8);
			msg.msgTime = query.getIntField(9);
			msg.talkerSid = query.getStringField(2);
			msg.msgAudioReaded = TRUE;
			msgList.push_back(msg);

			query.nextRow();
		}
	}
	catch (CppSQLite3Exception& sqliteException)
	{
#ifdef _DEBUG
		MessageBoxA(0, sqliteException.errorMessage(), "BD ERROR", MB_OK | MB_ICONHAND);
#endif
		CString csErrMsg = util::stringToCString(sqliteException.errorMessage(), CP_UTF8);
		APP_LOG(LOG_ERROR, _T("MessageModule_Impl db sqlGetHistoryMsg failed,error msg:%s"),
			csErrMsg);
		return FALSE;
	}
	catch (...)
	{
		return FALSE;
	}

	return TRUE;
}

BOOL MessageModule_Impl::sqlBatchInsertHistoryMsg(IN std::list<MessageEntity>& msgList)
{
	if (logic::TCPCLIENT_STATE_DISCONNECT == logic::getTcpClientModule()->getTcpClientNetState())
	{
		return FALSE;
	}
	MessageEntity msg;
	try
	{
		CppSQLite3Statement stmtBegin = m_pHistoryMSGDB->compileStatement(BeginInsert.c_str());
		stmtBegin.execDML();

		std::list<MessageEntity>::iterator iter = msgList.begin();
		for (; iter != msgList.end(); ++iter)
		{
			msg = *iter;
			if (msg.msgRenderType == MESSAGE_RENDERTYPE_SYSTEMTIPS)
			{
				countMsgOffset(msg.sessionId, -1);
				continue;
			}
			CppSQLite3Statement stmt = m_pHistoryMSGDB->compileStatement(insertHistoryMSGSql.c_str());
			stmt.bind(1, msg.sessionId.c_str());
			stmt.bind(2, msg.talkerSid.c_str());
			//对语音消息做个特殊处理，content存储的是json格式字符串
			if (MESSAGE_RENDERTYPE_AUDIO == msg.msgRenderType)
			{
				Json::Value root;
				root["msgAudioTime"] = msg.msgAudioTime;
				root["msgAudioId"] = msg.content;
				Json::FastWriter fstWrite;
				std::string jsonAudioContent = fstWrite.write(root);
				stmt.bind(6, jsonAudioContent.c_str());
			}
			else
			{
				stmt.bind(6, msg.content.c_str());
			}
			stmt.bind(7, msg.msgRenderType);
			stmt.bind(8, msg.msgFromType);
			stmt.bind(9, (Int32)msg.msgTime);
			stmt.bind(10, time(0));
			stmt.execDML();
		}

		CppSQLite3Statement stmtEnd = m_pHistoryMSGDB->compileStatement(EndInsert.c_str());
		stmtEnd.execDML();
	}
	catch (CppSQLite3Exception& e)
	{
		CString csErrMsg = util::stringToCString(e.errorMessage());
		APP_LOG(LOG_ERROR, TRUE, _T("MessageModule_Impl db sqlBatchInsertHistoryMsg failed,error msg:%s"), csErrMsg);
		CppSQLite3Statement stmtRollback = m_pHistoryMSGDB->compileStatement(RollBack.c_str());
		stmtRollback.execDML();
		_msgToTrace(msg);
		return FALSE;
	}
	catch (...)
	{
		APP_LOG(LOG_ERROR, TRUE, _T("MessageModule_Impl db sqlBatchInsertHistoryMsg unknown exception"));
		_msgToTrace(msg);
		return FALSE;
	}

	return TRUE;
}

/******************************************************************************/