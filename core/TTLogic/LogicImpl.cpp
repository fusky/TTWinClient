/******************************************************************************* 
 *  @file      LogicImpl.cpp 2014\7\17 14:13:42 $
 *  @author    ¿ìµ¶<kuaidao@mogujie.com>
 *  @brief   
 ******************************************************************************/

#include "stdafx.h"
#include "LogicImpl.h"
#include "LogicOperationManager.h"
#include "ModuleManager.h"
#include "LogicEventManager.h"
#include "MKObserver.h"
#include "TcpClientModule_Impl.h"

/******************************************************************************/
NAMESPACE_BEGIN(logic)

namespace
{
	ILogic*     g_pILogic = 0;
}

LogicErrorCode CreateSingletonILogic()
{
	if (0 == g_pILogic)
	{
		g_pILogic = new LogicImpl();
		if (0 == g_pILogic)
		{
			return LOGIC_ALLOC_ERROR;
		}
	}
	return LOGIC_OK;
}

void DestroySingletonILogic()
{
	if (g_pILogic)
	{
		g_pILogic->release();
	}
	g_pILogic = 0;
}

// -----------------------------------------------------------------------------
// public   
ILogic* GetLogic()
{
	return g_pILogic;
}


// -----------------------------------------------------------------------------
//  LogicImpl: Public, Constructor

LogicImpl::LogicImpl()
:m_pOperationManager(0)
,m_pModuleManager(0)
,m_pEventManager(0)
,m_pMkoObserver(0)
,m_pTcpClientModule(0)
{
	m_pOperationManager = new LogicOperationManager;
	m_pModuleManager = new ModuleManager;
	m_pEventManager = new LogicEventManager;
	m_pMkoObserver = new MKObserver;
	m_pTcpClientModule = new TcpClientModule_Impl();
}

// -----------------------------------------------------------------------------
//  LogicImpl: Public, Destructor

LogicImpl::~LogicImpl()
{
	delete m_pModuleManager;
	m_pModuleManager = 0;

	delete m_pOperationManager;
	m_pOperationManager = 0;

	delete m_pEventManager;
	m_pEventManager = 0;

	delete m_pMkoObserver;
	m_pMkoObserver = 0;
}

logic::LogicErrorCode LogicImpl::startup()
{
	LogicErrorCode errCode = LOGIC_OK;

	errCode = m_pEventManager->startup();
	if (LOGIC_OK != errCode)
		return errCode;

	errCode = m_pOperationManager->startup();
	if (LOGIC_OK != errCode)
		return errCode;

	registerModule(m_pTcpClientModule);

	return errCode;
}

void LogicImpl::shutdownPhase1()
{
	m_pTcpClientModule->closeSocket();
	m_pTcpClientModule->shutdown();
	m_pOperationManager->shutdown();
	m_pEventManager->shutdown();
}

void LogicImpl::shutdownPhase2()
{
	m_pModuleManager->_removeAllModules();
}

logic::LogicErrorCode LogicImpl::registerModule(IModule* pModule) throw()
{
	return m_pModuleManager->registerModule(pModule);
}

logic::LogicErrorCode LogicImpl::unRegisterModule(IModule* pModule) throw()
{
	return m_pModuleManager->unRegisterModule(pModule);
}

logic::LogicErrorCode LogicImpl::loadModule(IModule* pModule)
{
	return m_pModuleManager->loadModule(pModule);
}

IModule* LogicImpl::getModule(Int16 ModuleId)
{
	return m_pModuleManager->getModule(ModuleId);
}

logic::LogicErrorCode LogicImpl::pushBackOperation(IN IOperation* pOpertaion,Int32 delay /*= 0*/)
{
	return m_pOperationManager->pushBackOperation(pOpertaion,delay);
}

logic::LogicErrorCode LogicImpl::pushBackOperationWithLambda(std::function<void()> operationRun, Int32 delay /*= 0*/)
{
	return m_pOperationManager->pushBackOperationWithLambda(operationRun,delay);
}

logic::LogicErrorCode LogicImpl::asynFireUIEvent(IN const IEvent* pEvent)
{
	return m_pEventManager->asynFireUIEvent(pEvent);
}

logic::LogicErrorCode LogicImpl::asynFireUIEventWithLambda(std::function<void()> eventRun)
{
	return m_pEventManager->asynFireUIEventWithLambda(eventRun);
}

logic::LogicErrorCode LogicImpl::scheduleTimerWithLambda(IN UInt32 delay, IN BOOL bRepeat
													 , IN std::function<void()> timerRun
													 , OUT ITimerEvent** ppTimer)
{
	return m_pEventManager->scheduleTimerWithLambda(delay, bRepeat,timerRun, ppTimer);
}

logic::LogicErrorCode LogicImpl::scheduleTimer(IN ITimerEvent* pTimerEvent
											 , IN UInt32 delay
											 , IN BOOL bRepeat)
{
	return m_pEventManager->scheduleTimer(pTimerEvent, delay, bRepeat);
}

logic::LogicErrorCode LogicImpl::killTimer(IN ITimerEvent* pTimerEvent)
{
	return m_pEventManager->killTimer(pTimerEvent);
}

void LogicImpl::addObserver(void* pObserObject,UInt16 moduleId, IObserverHandler& handle)
{
	m_pMkoObserver->addObserver(pObserObject,moduleId, handle);
}

void LogicImpl::asynNotifyObserver(UInt32 keyId)
{
	m_pMkoObserver->asynNotifyObserver(keyId);
}

void LogicImpl::asynNotifyObserver(UInt32 keyId, std::string& mkoString)
{
	m_pMkoObserver->asynNotifyObserver(keyId, mkoString);
}

void LogicImpl::asynNotifyObserver(UInt32 keyId, Int32 mkoInt)
{
	m_pMkoObserver->asynNotifyObserver(keyId, mkoInt);
}

void LogicImpl::asynNotifyObserver(UInt32 keyId, void* pmkoVoid)
{
	m_pMkoObserver->asynNotifyObserver(keyId, pmkoVoid);
}

void LogicImpl::asynNotifyObserver(UInt32 keyId, std::shared_ptr<void> pmkoShardVoid)
{
	m_pMkoObserver->asynNotifyObserver(keyId, pmkoShardVoid);
}

void LogicImpl::removeObserver(void* pObserObject)
{
	m_pMkoObserver->removeObserver(pObserObject);
}

NAMESPACE_END(logic)
/******************************************************************************/