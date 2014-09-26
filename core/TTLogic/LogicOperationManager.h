/*******************************************************************************
 *  @file      LogicOperationManager.h 2012\9\11 14:55:32 $
 *  @author    ¿ìµ¶<kuaidao@mogujie.com>
 *  @brief   opertaion
 ******************************************************************************/

#ifndef LOGICOPERATIONMANAGER_803C98F5_0F85_48BA_BF1E_FD868A5E21B9_H__
#define LOGICOPERATIONMANAGER_803C98F5_0F85_48BA_BF1E_FD868A5E21B9_H__

#include "GlobalDefine.h"
#include "utility/TTThread.h"
#include "utility/TTAutoLock.h"
#include "TTLogic/ErrorCode.h"
#include <list>
#include <functional>

/******************************************************************************/
NAMESPACE_BEGIN(logic)
struct IOperation;
class LogicOperationManager;

class OperationThread final : public util::TTThread
{
	friend LogicOperationManager;
public:
    OperationThread();
    virtual ~OperationThread();

private:
    OperationThread(const OperationThread &);
    OperationThread& operator=(const OperationThread&);

public:
	LogicErrorCode  pushBackOperation(IOperation* const pOperation);
	LogicErrorCode  pushFrontOperation(IOperation* const pOperation);

protected:
	virtual UInt32 process();

private:
	void            _notifyExit();
	void            _removeAllOperations();

private:
    std::list<IOperation*>          m_vecOperations;
    BOOL                            m_bContinue;
    HANDLE                          m_semaphore;
    util::TTFastLock				m_lock;
};

class LogicOperationManager
{
public:
    LogicOperationManager();
    ~LogicOperationManager();

private:
    LogicOperationManager(LogicOperationManager&);
    LogicOperationManager* operator=(LogicOperationManager&);

public:
    LogicErrorCode startup();
    void shutdown(IN int seconds = 2000);
	LogicErrorCode pushBackOperation(IN IOperation* pOperation,Int32 delay);
	LogicErrorCode pushBackOperationWithLambda(std::function<void()> operationRun, Int32 delay);

private:
    OperationThread				*m_pOperationThread;
	std::list<IOperation*>      m_vecDelayOperations;
};

NAMESPACE_END(logic)
/******************************************************************************/
#endif// LOGICOPERATIONMANAGER_803C98F5_0F85_48BA_BF1E_FD868A5E21B9_H__