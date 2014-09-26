/*******************************************************************************
 *  @file      IDatabaseModule.h 2014\8\3 10:38:47 $
 *  @author    快刀<kuaidao@mogujie.com>
 *  @brief     借助sqlite实现本地数据存储模块，如用户信息等
 ******************************************************************************/

#ifndef IDATABASEMODULE_086C113C_CEE3_423B_81D1_D771B443A991_H__
#define IDATABASEMODULE_086C113C_CEE3_423B_81D1_D771B443A991_H__

#include "GlobalDefine.h"
#include "TTLogic/IModule.h"
#include "Modules/ModuleDll.h"
/******************************************************************************/
NAMESPACE_BEGIN(module)
struct ImImageEntity
{
	UInt32				hashcode;		//根据urlPath计算的hash值
	std::string			filename;		//图片本地存储名称
	std::string			urlPath;		//图片url
};

/**
 * The class <code>IDatabaseModule</code> 
 *
 */
class MODULE_API IDatabaseModule : public logic::IModule
{
public:
    /** @name Constructors and Destructor*/

    //@{
    /**
     * Constructor 
     */
	IDatabaseModule()
	{
		m_moduleId = MODULE_ID_DATABASE;
	}
    //@}

public:
	virtual BOOL openDB(const utf8char* pPath) = 0;
	virtual BOOL isDBOpen() = 0;

	/**@name 图片存储相关*/
	//@{
	virtual BOOL sqlInsertImImageEntity(const ImImageEntity& entity) = 0;
	virtual BOOL sqlGetImImageEntityByHashcode(UInt32 hashcode,ImImageEntity& entity) = 0;
	virtual BOOL sqlUpdateImImageEntity(UInt32 hashcode, module::ImImageEntity& entity) = 0;
	//@}
};

MODULE_API IDatabaseModule* getDatabaseModule();

NAMESPACE_END(module)
/******************************************************************************/
#endif// IDATABASEMODULE_086C113C_CEE3_423B_81D1_D771B443A991_H__
