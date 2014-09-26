/*******************************************************************************
 *  @file      LogicImpl.h 2014\7\17 14:13:30 $
 *  @author    快刀<kuaidao@mogujie.com>
 *  @brief   
 ******************************************************************************/

#ifndef LOGICIMPL_A4F11648_E6D4_450E_8C98_B1B50AA8AB10_H__
#define LOGICIMPL_A4F11648_E6D4_450E_8C98_B1B50AA8AB10_H__

#include "TTLogic/ILogic.h"
/******************************************************************************/
NAMESPACE_BEGIN(logic)
class LogicOperationManager;
class ModuleManager;
class LogicEventManager;
class MKObserver;
class ITcpClientModule;

/**
 * The class <code>LogicImpl</code> 
 *
 */
class LogicImpl final : public ILogic
{
public:
	/**@name TTLogic框架启动，关闭*/
	//@{
	virtual LogicErrorCode  startup();
	virtual void            shutdownPhase1();
	virtual void            shutdownPhase2();
	//@}

	/**@name 业务模块相关*/
	//@{
	virtual LogicErrorCode registerModule(IModule* pModule)throw();
	virtual LogicErrorCode unRegisterModule(IModule* pModule)throw();
	virtual LogicErrorCode	loadModule(IModule* pModule);
	virtual IModule* getModule(Int16 ModuleId);
	//@}

	/**@name operation相关*/
	//@{
	virtual LogicErrorCode pushBackOperation(IN IOperation* pOpertaion, Int32 delay = 0);
	virtual LogicErrorCode pushBackOperationWithLambda(std::function<void()> operationRun, Int32 delay = 0);
	virtual LogicErrorCode asynFireUIEvent(IN const IEvent* pEvent);
	virtual LogicErrorCode asynFireUIEventWithLambda(std::function<void()> eventRun);
	virtual LogicErrorCode scheduleTimerWithLambda(IN UInt32 delay, IN BOOL bRepeat
												 , IN std::function<void()> timerRun
												 , OUT ITimerEvent** ppTimer);
	virtual LogicErrorCode scheduleTimer(IN ITimerEvent* pTimerEvent, IN UInt32 delay, IN BOOL bRepeat);
	virtual LogicErrorCode killTimer(IN ITimerEvent* pTimerEvent);
	//@}

	/**@name MKO相关*/
	//@{
	virtual void addObserver(void* pObserObject,UInt16 moduleId, IObserverHandler& handle);
	virtual void removeObserver(void* pObserObject);
	virtual void asynNotifyObserver(UInt32 keyId);
	virtual void asynNotifyObserver(UInt32 keyId, std::string& mkoString);
	virtual void asynNotifyObserver(UInt32 keyId, Int32 mkoInt);
	virtual void asynNotifyObserver(UInt32 keyId, void* pmkoVoid);
	virtual void asynNotifyObserver(UInt32 keyId, std::shared_ptr<void> pmkoShardVoid);
	//@}

private:
	LogicImpl();
	virtual ~LogicImpl();
	LogicImpl(const LogicImpl&);
	LogicImpl& operator=(const LogicImpl&);
	virtual void release(){ delete this; };
	friend LogicErrorCode CreateSingletonILogic();

private:
	LogicOperationManager*		m_pOperationManager;
	ModuleManager*				m_pModuleManager;
	LogicEventManager*			m_pEventManager;
	MKObserver*					m_pMkoObserver;
	ITcpClientModule*			m_pTcpClientModule;
};
NAMESPACE_END(logic)
/******************************************************************************/
#endif// LOGICIMPL_A4F11648_E6D4_450E_8C98_B1B50AA8AB10_H__
