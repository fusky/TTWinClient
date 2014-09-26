/******************************************************************************* 
 *  @file      LogicOperationManager.cpp 2012\9\11 14:56:01 $
 *  @author    快刀<kuaidao@mogujie.com>
 *  @brief   
 ******************************************************************************/
#include "stdafx.h"
#include "LogicOperationManager.h"
#include "LogicEventManager.h"
#include "TTLogic/IOperation.h"
#include "TTLogic/Exception.h"
#include "TTLogic/ILogic.h"
#include "utility/utilCommonAPI.h"
#include <algorithm>

/******************************************************************************/
NAMESPACE_BEGIN(logic)

namespace
{
	//todo... 这个可以用template
	class LambdaOperation : public IOperation
	{
	public:
		LambdaOperation(std::function<void()> operationRun)
			:m_operationRun(operationRun)
		{
		}

		virtual void process()
		{
			m_operationRun();
		}
		virtual void release()
		{
			delete this;
		}

	private:
		std::function<void()> m_operationRun;
	};
}

// -----------------------------------------------------------------------------
//  OperationManager: Public, Constructor

LogicOperationManager::LogicOperationManager()
:m_pOperationThread(0)
{
    
}

// -----------------------------------------------------------------------------
//  OperationManager: Public, Destructor

LogicOperationManager::~LogicOperationManager()
{
    //捕捉可能抛出的未知异常
    try
    {
        shutdown();
    }
    catch(...)
    {
        APP_LOG(LOG_ERROR,_T("LogicOperationManager: shutdown throw unknown exception"));
        assert(FALSE);
    }
}

// -----------------------------------------------------------------------------
// private   
LogicErrorCode LogicOperationManager::startup()
{
    if(0 != m_pOperationThread)
        return LOGIC_ALLOC_ERROR;

    m_pOperationThread = new OperationThread;
    if(0 != m_pOperationThread)
    {
        m_pOperationThread->create();
    }

    LogicErrorCode errCode = LOGIC_OK;
    if( 0 == m_pOperationThread)
        return LOGIC_ALLOC_ERROR;

    return errCode;
}

// -----------------------------------------------------------------------------
// private   
void LogicOperationManager::shutdown(IN int seconds/* = 2000*/)
{
    if(m_pOperationThread)
    {
        m_pOperationThread->_notifyExit();
        m_pOperationThread->wait(seconds);
        m_pOperationThread->destory();
        delete m_pOperationThread;
        m_pOperationThread = 0;
    }
	for (IOperation* pOper : m_vecDelayOperations)
	{
		delete pOper;
		pOper = 0;
	}
	m_vecDelayOperations.clear();
}

// -----------------------------------------------------------------------------
// private   
LogicErrorCode LogicOperationManager::pushBackOperation(IN IOperation* pOperation, Int32 delay)
{
	assert(pOperation);
    assert(m_pOperationThread);
	if (0 == m_pOperationThread || 0 == pOperation)
    {
        return LOGIC_ARGUMENT_ERROR;
    }

	if (delay > 0)
	{
		m_vecDelayOperations.push_back(pOperation);
		ITimerEvent* pTimer = 0;
		return GetLogic()->scheduleTimerWithLambda(delay, FALSE,
			[=]()
		{
			m_pOperationThread->pushFrontOperation(pOperation);
			auto delayIter = std::remove_if(m_vecDelayOperations.begin()
											,m_vecDelayOperations.end()
											,[=](IOperation* pOper)
			{
				return (pOperation == pOper);
			}
			);
			if (delayIter != m_vecDelayOperations.end())
			{
				m_vecDelayOperations.erase(delayIter,m_vecDelayOperations.end());
			}
		}
		, &pTimer);
	}
	else
	{
		return m_pOperationThread->pushBackOperation(pOperation);
	}
}

logic::LogicErrorCode LogicOperationManager::pushBackOperationWithLambda(std::function<void()> operationRun,int delay)
{
	assert(m_pOperationThread);
	if (0 == m_pOperationThread)
	{
		return LOGIC_ARGUMENT_ERROR;
	}

	//todo 这里会有内存泄露的，需要处理
	LambdaOperation* pLambdaOper = new LambdaOperation(operationRun);
	return pushBackOperation(pLambdaOper, delay);
}

// -----------------------------------------------------------------------------
// private   virtual 
OperationThread::~OperationThread()
{
}

// -----------------------------------------------------------------------------
// private   virtual 
OperationThread::OperationThread()
:m_bContinue(TRUE)
,m_semaphore(0)
{
    m_semaphore = ::CreateSemaphore(0,0,LONG_MAX,0);
}
logic::LogicErrorCode OperationThread::pushFrontOperation(IOperation* const pOperation)
{
	if (0 == pOperation)
		return LOGIC_ARGUMENT_ERROR;

	{
		util::TTAutoLock lock(&m_lock);
		m_vecOperations.push_front(pOperation);

		if (FALSE == ::ReleaseSemaphore(m_semaphore, 1, 0))
			return LOGIC_WORK_PUSHOPERTION_ERROR;
	}

	return LOGIC_OK;
}
LogicErrorCode OperationThread::pushBackOperation(IOperation* const pOperation)
{
	if (0 == pOperation)
        return LOGIC_ARGUMENT_ERROR;

    {
        util::TTAutoLock lock(&m_lock);
		m_vecOperations.push_back(pOperation);

        if(FALSE == ::ReleaseSemaphore(m_semaphore,1,0))
            return LOGIC_WORK_PUSHOPERTION_ERROR;
    }

    return LOGIC_OK;
}

// -----------------------------------------------------------------------------
// private   
void OperationThread::_notifyExit()
{
    m_bContinue = FALSE;
    ::ReleaseSemaphore(m_semaphore,1,0);
    _removeAllOperations();
}

// -----------------------------------------------------------------------------
// private   
void OperationThread::_removeAllOperations()
{
    util::TTAutoLock lock(&m_lock);
	for (IOperation* pOper : m_vecOperations)
	{
		try
		{
			pOper->release();
		}
		catch (...)
		{

		}
	}
    m_vecOperations.clear();
}

// -----------------------------------------------------------------------------
// private   virtual 
UInt32  OperationThread::process()
{
    IOperation* pOperation = 0;

    while (m_bContinue)
    {
        if (WAIT_OBJECT_0 != ::WaitForSingleObject(m_semaphore, INFINITE))
        {
            APP_LOG(LOG_ERROR,_T("CoreOperationThread: Wait for Operations failed\n"));
            break;
        }

        if (!m_bContinue)
        {
            break;
        }

        {
			util::TTAutoLock lock(&m_lock);
            if (m_vecOperations.empty())
                pOperation = 0;
            else
            {
                pOperation = m_vecOperations.front();
                m_vecOperations.pop_front();
            }
        }

        try 
        {
            if (m_bContinue && pOperation)
                pOperation->process();

            if (pOperation)
                pOperation->release();
        }
        catch(logic::Exception* e)
        {
            if(pOperation)
            {
                pOperation->onException(e);
                pOperation->release();
            }
            if(e)
            {
                assert(FALSE);
                APP_LOG(LOG_ERROR,_T("LogicOperationThread: Operation run exception:%s"),e->m_lpszMsg);
                e->Delete();
            }
        }
        catch(std::runtime_error &e)
        {
            APP_LOG(LOG_ERROR,_T("LogicOperationThread: Operation run error:%s"),e.what());
            assert(FALSE);
        }
#ifndef DEBUG
        catch (...) 
        {
            APP_LOG(LOG_ERROR,_T("LogicOperationThread: Operation run exception,unknown reason"));
            assert(FALSE);
        }
#endif
    }
    return 0;
}

NAMESPACE_END(logic)
/******************************************************************************/