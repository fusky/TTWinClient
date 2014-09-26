/*******************************************************************************
 *  @file      IOperation.h 2014\7\16 19:10:09 $
 *  @author    快刀<kuaidao@mogujie.com>
 *  @brief   异步操作执行单位
 ******************************************************************************/

#ifndef IOPERATION_0D9C9F48_BD45_4D45_AF64_197CF594BE95_H__
#define IOPERATION_0D9C9F48_BD45_4D45_AF64_197CF594BE95_H__

#include "TTLogic/ILogic.h"
#include "GlobalDefine.h"
#include "TTLogicDll.h"
#include "delegate/FastDelegate.h"
#include <memory>
/******************************************************************************/
NAMESPACE_BEGIN(logic)
typedef
fastdelegate::FastDelegate1<std::shared_ptr<void> >	ICallbackHandler;
struct Exception;

/**
 * The class <code>IOperation</code> 
 *
 */
struct TTLOGIC_API IOperation
{
	friend class OperationThread;

public:
	virtual void process() = 0;
	virtual void onException(logic::Exception* e)
	{
		assert(FALSE);
		assert(e);
	}

private:
	/**
	* 必须让容器来释放自己
	*
	* @return  void
	* @exception there is no any exception to throw.
	*/
	virtual void release() = 0;
};

/**
* The class <code>操作回调事件，主要用于在任务执行过程中通知回调</code>
*
*/
class CallbackOperationEvent : public IEvent
{
public:
	CallbackOperationEvent(ICallbackHandler& callback, std::shared_ptr<void> param)
		:m_callback(callback)
		, m_param(param)
	{

	}
	virtual ~CallbackOperationEvent()
	{}

	virtual void process()
	{
		m_callback(m_param);
	}
	virtual void release(){ delete this; }

private:
	ICallbackHandler				m_callback;
	std::shared_ptr<void>			m_param;
};

/**
* The class <code>具有通知回调的异步操作</code>
*
*/
class TTLOGIC_API ICallbackOpertaion : public IOperation
{
public:
	ICallbackOpertaion(ICallbackHandler& callback)
	:m_callback(callback)
	{
	}
	virtual ~ICallbackOpertaion() {}

protected:
	/**
	 * 同步回调
	 *
	 * @param   std::shared_ptr<void> param
	 * @return  void
	 * @exception there is no any exception to throw.
	 */

	void syncCallback(std::shared_ptr<void> param)
	{
		m_callback(param);
	}
	/**
	 * 异步回调，借助UIEvent
	 *
	 * @param   std::shared_ptr<void> param
	 * @return  void
	 * @exception there is no any exception to throw.
	 */
	
	void asyncCallback(std::shared_ptr<void> param)
	{
		CallbackOperationEvent* pEvent = new CallbackOperationEvent(m_callback, param);
		GetLogic()->asynFireUIEvent(pEvent);
	}

private:
	ICallbackHandler          m_callback;
};

NAMESPACE_END(logic)
/******************************************************************************/
#endif// IOPERATION_0D9C9F48_BD45_4D45_AF64_197CF594BE95_H__
