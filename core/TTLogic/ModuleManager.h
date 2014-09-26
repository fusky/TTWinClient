/*******************************************************************************
 *  @file      ModuleManager.h 2014\7\16 17:56:18 $
 *  @author    快刀<kuaidao@mogujie.com>
 *  @brief   整个工程所有Module接口的管理类
 ******************************************************************************/

#ifndef MODULEMANAGER_ED8A7F0B_61EE_43A5_86AD_8A5545678674_H__
#define MODULEMANAGER_ED8A7F0B_61EE_43A5_86AD_8A5545678674_H__

#include "GlobalDefine.h"
#include "TTLogic/ErrorCode.h"
/******************************************************************************/
NAMESPACE_BEGIN(logic)

class IModule;

typedef std::vector<IModule*>  VecModule;

/**
 * The class <code>ModuleManager</code> 
 *
 */
class ModuleManager final
{
public:
	ModuleManager();
	~ModuleManager();

private:
	ModuleManager(const ModuleManager&);
	ModuleManager& operator=(const ModuleManager&);

public:
	LogicErrorCode  registerModule(IModule* pModule)throw();
	LogicErrorCode  unRegisterModule(IModule* pModule)throw();
	LogicErrorCode	loadModule(IModule* pModule);
	IModule*		getModule(Int16 moduleId);

public:
	void _removeAllModules();

private:
	VecModule		m_vecModule;
	BOOL			m_bIsLoaded;	//是否加载过
};

NAMESPACE_END(logic)
/******************************************************************************/
#endif// MODULEMANAGER_ED8A7F0B_61EE_43A5_86AD_8A5545678674_H__
