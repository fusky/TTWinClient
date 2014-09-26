/******************************************************************************* 
 *  @file      ModuleManager.cpp 2014\7\16 17:56:22 $
 *  @author    快刀<kuaidao@mogujie.com>
 *  @brief   
 ******************************************************************************/

#include "stdafx.h"
#include "ModuleManager.h"
#include "TTLogic/IModule.h"
#include <algorithm>

/******************************************************************************/
NAMESPACE_BEGIN(logic)

namespace
{
	class FindModuleIDFunctor
		: public std::unary_function<IModule*, bool>
	{
	public:
		FindModuleIDFunctor(UInt16 sID)
			:m_moduleId(sID)
		{

		}

		bool operator()(const IModule* pModule)
		{
			return (m_moduleId == pModule->getModuleId());
		}

	private:
		UInt16      m_moduleId;
	};
}

// -----------------------------------------------------------------------------
//  ModuleManager: Public, Constructor

ModuleManager::ModuleManager()
:m_bIsLoaded(FALSE)
{

}

// -----------------------------------------------------------------------------
//  ModuleManager: Public, Destructor

ModuleManager::~ModuleManager()
{

}
// -----------------------------------------------------------------------------
// private   
LogicErrorCode ModuleManager::registerModule(IModule* pModule) throw()
{
	LogicErrorCode errCode = LOGIC_OK;
	assert(pModule);
	if (0 == pModule || MODULE_ID_NONE == pModule->getModuleId())
		return LOGIC_ARGUMENT_ERROR;

	auto iter = std::find_if(m_vecModule.begin()
		, m_vecModule.end()
		, FindModuleIDFunctor(pModule->getModuleId()));
	if (iter != m_vecModule.end())
		return LOGIC_MODULE_HASONE_ERROR;
	m_vecModule.push_back(pModule);

	//如果是延迟加载模块，则先不初始化模块资源，等到需要用到的时候再加载
	if (!pModule->isLazyLoadModule())
	{
		m_bIsLoaded = TRUE;
		pModule->onLoadModule();
	}

	return errCode;
}
// -----------------------------------------------------------------------------
// private   
logic::LogicErrorCode ModuleManager::unRegisterModule(IModule* pModule) throw()
{
	LogicErrorCode errCode = LOGIC_OK;
	assert(pModule);
	if (0 == pModule || MODULE_ID_NONE == pModule->getModuleId())
		return LOGIC_ARGUMENT_ERROR;

	auto iter = std::remove_if(m_vecModule.begin()
		, m_vecModule.end()
		, FindModuleIDFunctor(pModule->getModuleId()));
	if (iter == m_vecModule.end())
	{
		return LOGIC_MODULE_INEXISTENCE_ERROR;
	}
	m_vecModule.erase(iter,m_vecModule.end());

	//卸载模块后
	m_bIsLoaded = FALSE;
	pModule->onUnLoadModule();

	return errCode;
}
logic::LogicErrorCode ModuleManager::loadModule(IModule* pModule)
{
	if (!pModule)
		return LOGIC_MODULE_INEXISTENCE_ERROR;

	if (!m_bIsLoaded && pModule->isLazyLoadModule())
	{
		m_bIsLoaded = TRUE;
		return pModule->onLoadModule();
	}		

	return LOGIC_MODULE_LOAD_ERROR;
}
// -----------------------------------------------------------------------------
// private   
IModule* ModuleManager::getModule(Int16 moduleId)
{
	auto iter = std::find_if(m_vecModule.begin()
		, m_vecModule.end()
		, FindModuleIDFunctor(moduleId));
	if (iter != m_vecModule.end())
	{
		return *iter;
	}

	return 0;
}

// -----------------------------------------------------------------------------
// private   
void ModuleManager::_removeAllModules()
{
	for (IModule* pModule : m_vecModule)
	{
		pModule->onUnLoadModule();
		pModule->release();
	}
	m_vecModule.clear();
}

NAMESPACE_END(logic)
/******************************************************************************/