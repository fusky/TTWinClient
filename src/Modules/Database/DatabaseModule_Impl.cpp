/******************************************************************************* 
 *  @file      DatabaseModule_Impl.cpp 2014\8\3 10:43:17 $
 *  @author    快刀<kuaidao@mogujie.com>
 *  @brief     
 ******************************************************************************/

#include "stdafx.h"
#include "DatabaseModule_Impl.h"
#include "utility/utilStrCodeAPI.h"
#include "utility/CppSQLite3.h"
#include "utility/utilCommonAPI.h"
#include "Modules/IMiscModule.h"
/******************************************************************************/
namespace module
{
	IDatabaseModule* getDatabaseModule()
	{
		return (IDatabaseModule*)logic::GetLogic()->getModule(MODULE_ID_DATABASE);
	}
}

namespace
{
	const std::string createImImageSql = 
		"CREATE TABLE IF NOT EXISTS imimage"
		"("
		"    [id]                   INTEGER PRIMARY KEY,"
		"    [hashcode]             INTEGER DEFAULT 0 NOT NULL UNIQUE,"		//根据url生成的hash code
		"    [localPath]			TEXT NOT NULL,"							//图片本地相对路径
		"    [urlPath]				TEXT NOT NULL"							//图片url地址
		");";
	const std::string createImImageIndex =
		"CREATE INDEX IF NOT EXISTS hashcode_idx ON imimage(hashcode);";
	const std::string insertImImageSql
		= "INSERT INTO imimage(hashcode,localPath,urlPath) "
		"VALUES(?, ? ,? );";
	const std::string getImImageByHashcodeSql
		= "select * from imimage where hashcode=? limit 1";
	const std::string updateImImageByHashcodeSql
		= "update imimage set localPath=? where hashcode=?";
}
// -----------------------------------------------------------------------------
//  DatabaseModule_Impl: Public, Constructor

DatabaseModule_Impl::DatabaseModule_Impl()
:m_pSqliteDB(new CppSQLite3DB())
{

}

// -----------------------------------------------------------------------------
//  DatabaseModule_Impl: Public, Destructor

DatabaseModule_Impl::~DatabaseModule_Impl()
{
	delete m_pSqliteDB;
}

void DatabaseModule_Impl::release()
{
	delete this;
}

BOOL DatabaseModule_Impl::openDB(const utf8char* pPath)
{
	assert(pPath);
	assert(m_pSqliteDB);
	try
	{
		m_pSqliteDB->open(pPath);
		m_pSqliteDB->execDML(createImImageSql.c_str());
		m_pSqliteDB->execDML(createImImageIndex.c_str());
	}
	catch (CppSQLite3Exception& sqliteException)
	{
		CString csErrMsg = util::stringToCString(sqliteException.errorMessage(), CP_UTF8);
		APP_LOG(LOG_ERROR, TRUE, _T("DatabaseModule open database failed,error msg:%s")
			, csErrMsg);
#ifdef _DEBUG
		MessageBoxA(0, sqliteException.errorMessage(), "BD ERROR", MB_OK | MB_ICONHAND);
#endif	
		return FALSE;
	}
	catch (...)
	{
		APP_LOG(LOG_ERROR, TRUE, _T("DatabaseModule open database failed"));
		return FALSE;
	}

	APP_LOG(LOG_ERROR, TRUE, _T("DatabaseModule open  database done"));
	return TRUE;
}

void DatabaseModule_Impl::_closeDB()
{
	try
	{
		m_pSqliteDB->close();
	}
	catch (CppSQLite3Exception& sqliteException)
	{
		CString csErrMsg = util::stringToCString(sqliteException.errorMessage(), CP_UTF8);
		APP_LOG(LOG_ERROR, TRUE, _T("DatabaseModule close database failed,error msg:%s")
			, csErrMsg);
#ifdef _DEBUG
		MessageBoxA(0, sqliteException.errorMessage(), "BD ERROR", MB_OK | MB_ICONHAND);
#endif
	}
	catch (...)
	{
		APP_LOG(LOG_ERROR, TRUE, _T("DatabaseModule close database failed"));
	}
	APP_LOG(LOG_INFO, _T("DatabaseModule close database done"));
}

BOOL DatabaseModule_Impl::isDBOpen()
{
	return m_pSqliteDB->isOpen();
}

logic::LogicErrorCode DatabaseModule_Impl::onLoadModule()
{
	CString dbPath = module::getMiscModule()->getTTCommonAppdataUserDir();
	util::createAllDirectories(dbPath);
	dbPath += _T("\\storage.db");
	std::string sDbPath;
	sDbPath = util::cStringToString(dbPath);
	if (!openDB(sDbPath.c_str()))
	{
		APP_LOG(LOG_ERROR, _T("DatabaseModule_Impl open db failed,%s"),dbPath);
	}

	return logic::LOGIC_OK;
}

logic::LogicErrorCode DatabaseModule_Impl::onUnLoadModule()
{
	_closeDB();
	return logic::LOGIC_OK;
}

BOOL DatabaseModule_Impl::sqlInsertImImageEntity(const module::ImImageEntity& entity)
{
	try
	{
		CppSQLite3Statement stmt = m_pSqliteDB->compileStatement(insertImImageSql.c_str());
		stmt.bind(1, (int)entity.hashcode);
		stmt.bind(2, entity.filename.c_str());
		stmt.bind(3, entity.urlPath.c_str());
		stmt.execDML();
	}
	catch (CppSQLite3Exception& sqliteException)
	{
#ifdef _DEBUG
		MessageBoxA(0, sqliteException.errorMessage(), "BD ERROR", MB_OK | MB_ICONHAND);
#endif
		CString csErrMsg = util::stringToCString(sqliteException.errorMessage(), CP_UTF8);
		APP_LOG(LOG_ERROR, _T("DatabaseModule_Impl db sqlInsertImImageEntity failed,error msg:%s"), csErrMsg);
		return FALSE;
	}
	catch (...)
	{
		return FALSE;
	}

	return TRUE;
}

BOOL DatabaseModule_Impl::sqlGetImImageEntityByHashcode(UInt32 hashcode, module::ImImageEntity& entity)
{
	try
	{
		CppSQLite3Statement stmt;
		stmt = m_pSqliteDB->compileStatement(getImImageByHashcodeSql.c_str());
		stmt.bind(1, (int)hashcode);

		CppSQLite3Query query = stmt.execQuery();
		std::string sId;
		std::string avatarPath;
		if (!query.eof())
		{
			entity.hashcode = hashcode;
			entity.filename = query.getStringField(2);
			entity.urlPath = query.getStringField(3);
		}
		else
		{
			return FALSE;
		}
	}
	catch (CppSQLite3Exception& sqliteException)
	{
#ifdef _DEBUG
		MessageBoxA(0, sqliteException.errorMessage(), "BD ERROR", MB_OK | MB_ICONHAND);
#endif
		CString csErrMsg = util::stringToCString(sqliteException.errorMessage(), CP_UTF8);
		APP_LOG(LOG_ERROR, _T("DatabaseModule_Impl db sqlGetImImageEntityByHashcode failed,error msg:%s"),
			csErrMsg);
		return FALSE;
	}
	catch (...)
	{
		return FALSE;
	}

	return TRUE;
}

BOOL DatabaseModule_Impl::sqlUpdateImImageEntity(UInt32 hashcode, module::ImImageEntity& entity)
{
	try
	{
		CppSQLite3Statement stmt = m_pSqliteDB->compileStatement(updateImImageByHashcodeSql.c_str());
		stmt.bind(1, entity.filename.c_str());
		stmt.bind(2, (int)hashcode);
		int countUpdate = stmt.execDML();
		if (0 == countUpdate)
		{
			return FALSE;
		}
	}
	catch (CppSQLite3Exception& sqliteException)
	{
#ifdef _DEBUG
		MessageBoxA(0, sqliteException.errorMessage(), "BD ERROR", MB_OK | MB_ICONHAND);
#endif
		CString csErrMsg = util::stringToCString(sqliteException.errorMessage(), CP_UTF8);
		APP_LOG(LOG_ERROR, TRUE, _T("DatabaseModule_Impl db sqlUpdateImImageEntity failed,error msg:%s"), csErrMsg);
		return FALSE;
	}
	catch (...)
	{
		return FALSE;
	}

	return TRUE;
}

/******************************************************************************/