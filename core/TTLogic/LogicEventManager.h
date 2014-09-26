/*******************************************************************************
 *  @file      LogicEventManager.h 2014\7\18 15:09:00 $
 *  @author    ¿ìµ¶<kuaidao@mogujie.com>
 *  @brief   
 ******************************************************************************/

#ifndef LOGICEVENTMANAGER_717EB46C_8674_4EBC_9DD6_D5EA760A5868_H__
#define LOGICEVENTMANAGER_717EB46C_8674_4EBC_9DD6_D5EA760A5868_H__

#include "GlobalDefine.h"
#include "TTLogic/ErrorCode.h"
#include "utility/TTAutoLock.h"
#include <list>
#include <functional>
/******************************************************************************/
NAMESPACE_BEGIN(logic)

struct IEvent;
struct ITimerEvent;

struct TTTimer
{
	UInt64 nElapse;
	UInt32 nDelay;
	BOOL   bRepeat;
	ITimerEvent* pTimerEvent;

	TTTimer();
};

/**
 * The class <code>LogicEventManager</code> 
 *
 */
class LogicEventManager final
{
public:
    /** @name Constructors and Destructor*/

    //@{
    /**
     * Constructor 
     */
    LogicEventManager();
    /**
     * Destructor
     */
    ~LogicEventManager();
    //@}

public:
	LogicErrorCode  startup();
	void            shutdown();
	LogicErrorCode  asynFireUIEvent(IN const IEvent* const pEvent);
	LogicErrorCode	asynFireUIEventWithLambda(std::function<void()> eventRun);
	LogicErrorCode  scheduleTimer(IN ITimerEvent* pTimerEvent, IN UInt32 delay, IN BOOL bRepeat);
	LogicErrorCode  scheduleTimerWithLambda(IN UInt32 delay, IN BOOL bRepeat
										  , IN std::function<void()> timerRun
										  , OUT ITimerEvent** ppTimer);
	LogicErrorCode  killTimer(IN ITimerEvent* pTimerEvent);

private:
	HWND _createWnd();
	void _removeEvents();
	void _processTimer();
	static LRESULT _stdcall _WndProc(HWND hWnd
		, UINT message
		, WPARAM wparam
		, LPARAM lparam);
	static void _processEvent(IEvent* pEvent, BOOL bRelease);

private:
	LogicEventManager& operator=(LogicEventManager&);
	LogicEventManager(const LogicEventManager&);

private:
	HWND                    m_hWnd;
	util::TTFastLock		m_lock;
	std::list<TTTimer>		m_lstTimers;
};

NAMESPACE_END(logic)
/******************************************************************************/
#endif// LOGICEVENTMANAGER_717EB46C_8674_4EBC_9DD6_D5EA760A5868_H__
