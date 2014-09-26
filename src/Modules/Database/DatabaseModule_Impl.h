/*******************************************************************************
 *  @file      DatabaseModule_Impl.h 2014\8\3 10:43:14 $
 *  @author    ¿ìµ¶<kuaidao@mogujie.com>
 *  @brief     
 ******************************************************************************/

#ifndef DATABASEMODULE_IMPL_11F97834_E808_4523_A566_B8903038A8EB_H__
#define DATABASEMODULE_IMPL_11F97834_E808_4523_A566_B8903038A8EB_H__

#include "Modules/IDatabaseModule.h"
/******************************************************************************/
class CppSQLite3DB;

/**
 * The class <code>DatabaseModule_Impl</code> 
 *
 */
class DatabaseModule_Impl : public module::IDatabaseModule
{
public:
    /** @name Constructors and Destructor*/

    //@{
    /**
     * Constructor 
     */
    DatabaseModule_Impl();
    /**
     * Destructor
     */
    virtual ~DatabaseModule_Impl();
    //@}
	virtual void release();
	virtual logic::LogicErrorCode onLoadModule();
	virtual logic::LogicErrorCode onUnLoadModule();

public:
	virtual BOOL openDB(const utf8char* pPath);
	virtual BOOL isDBOpen();

	/**@name Í¼Æ¬´æ´¢Ïà¹Ø*/
	//@{
	virtual BOOL sqlInsertImImageEntity(const module::ImImageEntity& entity);
	virtual BOOL sqlGetImImageEntityByHashcode(UInt32 hashcode, module::ImImageEntity& entity);
	virtual BOOL sqlUpdateImImageEntity(UInt32 hashcode, module::ImImageEntity& entity);
	//@}

private:
	void _closeDB();

private:
	CppSQLite3DB*				m_pSqliteDB;
};
/******************************************************************************/
#endif// DATABASEMODULE_IMPL_11F97834_E808_4523_A566_B8903038A8EB_H__
