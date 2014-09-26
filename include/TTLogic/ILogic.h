/*******************************************************************************
 *  @file      ILogic.h 2014\7\16 18:29:18 $
 *  @author    快刀<kuaidao@mogujie.com>
 *  @brief   
 ******************************************************************************/

#ifndef ILOGIC_D21393E4_63FB_419A_A4DF_758A7067FE3D_H__
#define ILOGIC_D21393E4_63FB_419A_A4DF_758A7067FE3D_H__

#include "GlobalDefine.h"
#include "TTLogic/ErrorCode.h"
#include "TTLogic/TTLogicDll.h"
#include "TTLogic/Observer.h"
#include "TTLogic/ModuleID.h"
#include <functional>
/******************************************************************************/
NAMESPACE_BEGIN(logic)

class  IModule;
struct IEvent;
struct ITimerEvent;
struct IOperation;

namespace
{
	struct ILogicMKO
	{
		virtual void addObserver(void* pObserObject,UInt16 moduleId, IObserverHandler& handle) = 0;
		virtual void removeObserver(void* pObserObject) = 0;
		virtual void asynNotifyObserver(UInt32 keyId) = 0;
		virtual void asynNotifyObserver(UInt32 keyId, std::string& mkoString) = 0;
		virtual void asynNotifyObserver(UInt32 keyId, Int32 mkoInt) = 0;
		virtual void asynNotifyObserver(UInt32 keyId, void* pmkoVoid) = 0;
		virtual void asynNotifyObserver(UInt32 keyId, std::shared_ptr<void> pmkoShardVoid) = 0;
	};
	/**
	* The class <code>Module接口定义</code>
	*
	*/
	struct ILogicModule
	{
		/**
		* 注册模块(注册进去的Module对象实例不需要自己释放，
		*         除非自行调用了unRegisterModule)
		*
		* @param   IModule * pModule     模块器接口指针
		* @return  LogicErrorCode
		* @exception there is no any exception to throw.
		*/
		virtual LogicErrorCode registerModule(IModule* pModule)throw() = 0;

		/**
		* 反注册模块（调用该接口之后，需自行调用pModule->release()，否则会有内存泄露）
		*
		* @param   IModule * pModule     模块器接口指针
		* @return  LogicErrorCode
		* @exception there is no any exception to throw.
		*/
		virtual LogicErrorCode unRegisterModule(IModule* pModule)throw() = 0;

		/**
		* 对延迟加载模块，手动进行加载
		*
		* @param   IModule * pModule     模块器接口指针
		* @return  LogicErrorCode
		* @exception there is no any exception to throw.
		*/
		virtual LogicErrorCode	loadModule(IModule* pModule) = 0;

		/**
		* 通过Module ID获取Module接口指针
		*
		* @param   Int16 ModuleId     Module ID(ID号定义见于Moduleid.h)
		* @return  IModule*           Module接口指针
		* @exception there is no any exception to throw.
		*/
		virtual IModule* getModule(Int16 ModuleId) = 0;
	};
}

/**
* The class <code>后台任务相关接口定义</code>
*
*/
struct ILogicWorker
{
	/**
	* 发起一个Operation到任务队列尾部(放入容器的Operation对象实例不需要自己释放)
	*
	* @param   IOperation * pOperation
	* @param	int delay = 0
	* @return  void
	*/
	virtual LogicErrorCode pushBackOperation(IN IOperation* pOpertaion,Int32 delay = 0) = 0;

	/**
	* 将Operation 以lambda表达式方式放入到任务队列中
	*
	* @param   std::function<void()> operationRun
	* @return  void
	*/
	virtual LogicErrorCode pushBackOperationWithLambda(std::function<void()> operationRun,Int32 delay = 0) = 0;

	/**
	* 异步发送一个UI事件到主线程(event对象实例不需要自己释放)
	*
	* @param   IN const IEvent * const pEvent
	* @return  void
	*/
	virtual LogicErrorCode asynFireUIEvent(IN const IEvent* pEvent) = 0;
	
	/**
	* UIEvent以lambda方式推送
	*
	* @param   std::function<void()> eventRun
	* @return  void
	*/
	virtual LogicErrorCode asynFireUIEventWithLambda(std::function<void()> eventRun) = 0;
	/**
	* lambda方式设置TT定时器
	*
	* @param   std::function<void()> timerRun timer定时器执行lambda表达式
	* @param   UInt32 delay				延时（以秒为单位）
	* @param   BOOL bRepeat				是否重复
	* @param   ITimerEvent** ppTimer	lambad依赖执行的TimerEvent对象指针
	* @return  logic::LogicErrorCode
	* @exception there is no any exception to throw.
	*/
	virtual LogicErrorCode scheduleTimerWithLambda(IN UInt32 delay, IN BOOL bRepeat
												 , IN std::function<void()> timerRun
												 , OUT ITimerEvent** ppTimer) = 0;
	/**
	* 设置TT定时器(TimerEvent不需要自己释放)
	*
	* @param   IN ITimerEvent * pEvent timer定时器执行接口
	* @param   UInt32 delay            延时（以秒为单位）
	* @param   BOOL bRepeat            是否重复
	* @return  logic::LogicErrorCode
	* @exception there is no any exception to throw.
	*/
	virtual LogicErrorCode scheduleTimer(IN ITimerEvent* pTimerEvent, IN UInt32 delay, IN BOOL bRepeat) = 0;

	/**
	* 删除定时器
	*
	* @param   IN ITimerEvent * pEvent
	* @return  logic::LogicErrorCode
	* @exception there is no any exception to throw.
	*/
	virtual LogicErrorCode killTimer(IN ITimerEvent* pTimerEvent) = 0;
};

/**
 * The class <code>ILogic</code> 
 *
 */
struct TTLOGIC_API ILogic : public ILogicModule
						  , public ILogicWorker
						  , public ILogicMKO
{
	//逻辑引擎启动与关闭
	/**
	* 启动逻辑引擎
	*
	* @return  LogicErrorCode
	* @exception there is no any exception to throw.
	*/
	virtual LogicErrorCode startup() = 0;
	/**
	* 关闭逻辑引擎,分为阶段1、2
	*
	* @return  void
	* @exception there is no any exception to throw.
	*/
	virtual void shutdownPhase1() = 0;
	virtual void shutdownPhase2() = 0;

	/**
	* 释放自己
	*
	* @return  void
	* @exception there is no any exception to throw.
	*/
	virtual void release() = 0;
};
TTLOGIC_API LogicErrorCode CreateSingletonILogic();
TTLOGIC_API void DestroySingletonILogic();
TTLOGIC_API ILogic* GetLogic();

NAMESPACE_END(logic)
/******************************************************************************/
#endif// ILOGIC_D21393E4_63FB_419A_A4DF_758A7067FE3D_H__
