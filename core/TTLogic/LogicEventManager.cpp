/******************************************************************************* 
 *  @file      LogicEventManager.cpp 2014\7\18 15:09:03 $
 *  @author    快刀<kuaidao@mogujie.com>
 *  @brief   
 ******************************************************************************/

#include "stdafx.h"
#include "LogicEventManager.h"
#include "TTLogic/IEvent.h"
#include "TTLogic/ILogic.h"
#include "TTLogic/Exception.h"
#include "utility/utilCommonAPI.h"
#include <algorithm>

/******************************************************************************/
NAMESPACE_BEGIN(logic)

namespace
{
	#define LOGIC_EVNET_MSG             _T("___LogicEventDisptacherMessage")
	#define LOGIC_WINDOW_CLASSNAME      _T("___LogicEventDisptacherWndClass")
	#define LOGIC_WINDOW_NAME           _T("")
	#define LOGIC_TIMER_ID              8888

	UINT g_msg = (UINT)-1;
}

namespace
{
	template<class base>
	class LambdaEvent : public base
	{
	public:
		LambdaEvent(std::function<void()> eventRun)
			:m_eventRun(eventRun)
		{
		}

		virtual void process()
		{
			m_eventRun();
		}

		virtual void release()
		{
			delete this;
		}

	private:
		std::function<void()> m_eventRun;
	};
}

// -----------------------------------------------------------------------------
//  LogicEventManager: Public, Constructor

LogicEventManager::LogicEventManager()
:m_hWnd(0)
{
	g_msg = ::RegisterWindowMessage(LOGIC_EVNET_MSG);
}

// -----------------------------------------------------------------------------
//  LogicEventManager: Public, Destructor

LogicEventManager::~LogicEventManager()
{
	//捕捉可能抛出的未知异常
	try
	{
		shutdown();
	}
	catch (...)
	{
		APP_LOG(LOG_ERROR, _T("LogicEventManager: shutdown throw unknown exception"));
		assert(FALSE);
	}
}

logic::LogicErrorCode LogicEventManager::startup()
{
	LogicErrorCode errCode = LOGIC_OK;

	if (0 != m_hWnd)
		return LOGIC_OK;
	else
		m_hWnd = _createWnd();

	if (FALSE == ::IsWindow(m_hWnd))
		errCode = LOGIC_INVALID_HWND_ERROR;

	return errCode;
}

void LogicEventManager::shutdown()
{
	if (0 != m_hWnd)
	{
		util::TTAutoLock lock(&m_lock);
		_removeEvents();
		::KillTimer(m_hWnd, LOGIC_TIMER_ID);
		::DestroyWindow(m_hWnd);
		m_hWnd = 0;
	}
}

HWND LogicEventManager::_createWnd()
{
	HWND hwnd = 0;

	WNDCLASS wc;
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = _WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = ::AfxGetInstanceHandle();
	wc.hIcon = 0;
	wc.hCursor = 0;
	wc.hbrBackground = 0;
	wc.lpszMenuName = 0;
	wc.lpszClassName = LOGIC_WINDOW_CLASSNAME;

	if (!::RegisterClass(&wc))
		return 0;
	hwnd = ::CreateWindowEx(0, LOGIC_WINDOW_CLASSNAME, LOGIC_WINDOW_NAME,
		0, 0, 0, 1, 1, HWND_MESSAGE, 0, 0, 0);
	if (hwnd)
	{
		::SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)this);
		::SetTimer(hwnd, LOGIC_TIMER_ID, 1000, NULL);
	}

	return hwnd;
}

LRESULT _stdcall LogicEventManager::_WndProc(HWND hWnd
											, UINT message
											, WPARAM wparam
											, LPARAM lparam)
{
	if (message == g_msg)
	{
		_processEvent((IEvent*)lparam, TRUE);
	}
	else if (message == WM_TIMER)
	{
		if (wparam == LOGIC_TIMER_ID)
		{
			LogicEventManager* pServer
				= (LogicEventManager*)::GetWindowLongPtr(hWnd, GWLP_USERDATA);
			assert(0 != pServer);
			pServer->_processTimer();
		}
	}

	return ::DefWindowProc(hWnd, message, wparam, lparam);
}

void LogicEventManager::_removeEvents()
{
	//消息队列中的消息
	MSG msg;
	while (::PeekMessage(&msg, m_hWnd, g_msg, g_msg, PM_REMOVE))
	{
		if (msg.lParam != 0)
		{
			((IEvent*)msg.lParam)->release();
		}
	}

	{
		util::TTAutoLock lock(&m_lock);
		for (auto &pTimer : m_lstTimers)
		{
			if (pTimer.pTimerEvent)
			{
				pTimer.pTimerEvent->release();
			}
		}
		m_lstTimers.clear();
	}
}

void LogicEventManager::_processEvent(IEvent* pEvent, BOOL bRelease)
{
	assert(pEvent);
	if (0 == pEvent)
		return;

	try
	{
		pEvent->process();
		if (bRelease)
			pEvent->release();
	}
	catch (logic::Exception *e)
	{
		APP_LOG(LOG_ERROR, _T("LogicEventManager: event run exception"));
		pEvent->onException(e);
		if (bRelease)
			pEvent->release();
		if (e)
		{
			APP_LOG(LOG_ERROR, _T("LogicEventManager: event run exception:%s"), e->m_lpszMsg);
			assert(FALSE);
			e->Delete();
		}
	}
	catch (...)
	{
		APP_LOG(LOG_ERROR, _T("LogicOperationThread: operation run exception,unknown reason"));
		if (bRelease)
			pEvent->release();
		assert(FALSE);
	}
}

logic::LogicErrorCode LogicEventManager::asynFireUIEvent(IN const IEvent* const pEvent)
{
	assert(m_hWnd);
	assert(pEvent);
	if (0 == m_hWnd || 0 == pEvent)
		return LOGIC_ARGUMENT_ERROR;

	if (FALSE == ::PostMessage(m_hWnd, g_msg, 0, (LPARAM)pEvent))
		return LOGIC_WORK_POSTMESSAGE_ERROR;

	return LOGIC_OK;
}

logic::LogicErrorCode LogicEventManager::asynFireUIEventWithLambda(std::function<void()> eventRun)
{
	assert(m_hWnd);
	if (0 == m_hWnd)
		return LOGIC_ARGUMENT_ERROR;

	//注：todo...这里再应用退出时,可能会引起内存泄露，因为释放是靠windows消息队列来的
	LambdaEvent<IEvent>* pLambdaEvent = new LambdaEvent<IEvent>(eventRun);
	return asynFireUIEvent(pLambdaEvent);
}

logic::LogicErrorCode LogicEventManager::scheduleTimerWithLambda(IN UInt32 delay
																,IN BOOL bRepeat
																,IN std::function<void()> timerRun
																,OUT ITimerEvent** ppTimer)
{
	LambdaEvent<ITimerEvent>* pLambdaEvent = new LambdaEvent<ITimerEvent>(timerRun);
	logic::LogicErrorCode errCode = scheduleTimer(pLambdaEvent, delay, bRepeat);
	*ppTimer = pLambdaEvent;
	if (LOGIC_OK != errCode)
	{
		delete pLambdaEvent;
		pLambdaEvent = nullptr;
		*ppTimer = 0;
	}
	return errCode;
}

logic::LogicErrorCode LogicEventManager::scheduleTimer(IN ITimerEvent* pTimerEvent
													 , IN UInt32 delay
													 , IN BOOL bRepeat)
{
	assert(pTimerEvent);
	if (0 == pTimerEvent)
	{
		return LOGIC_ARGUMENT_ERROR;
	}
	//需要立即执行的
	if (0 == delay)
	{
		asynFireUIEvent(pTimerEvent);
	}
	{
		util::TTAutoLock lock(&m_lock);
		TTTimer ctx;
		ctx.bRepeat = bRepeat;
		ctx.nDelay = delay;
		ctx.pTimerEvent = pTimerEvent;

		std::list<TTTimer>::iterator iter = m_lstTimers.begin();
		for (; iter != m_lstTimers.end(); ++iter)
		{
			if (pTimerEvent == iter->pTimerEvent)
			{
				*iter = ctx;
				return LOGIC_OK;
			}
		}
		m_lstTimers.push_back(ctx);
	}

	return LOGIC_OK;
}

void LogicEventManager::_processTimer()
{
	util::TTAutoLock lock(&m_lock);
	for (auto iter = m_lstTimers.begin(); iter != m_lstTimers.end();)
	{
		TTTimer& ctx = *iter;
		if (++ctx.nElapse < ctx.nDelay)
		{
			++iter;
			continue;
		}
		if (!ctx.bRepeat)
		{
			TTTimer ctxBak = *iter;
			iter = m_lstTimers.erase(iter);
			_processEvent(ctxBak.pTimerEvent, TRUE);
		}
		else
		{
			if (0 == (ctx.nElapse % ctx.nDelay))
			{
				_processEvent(ctx.pTimerEvent, FALSE);
				++iter;
				ctx.nElapse = 0;
				continue;
			}
		}
	}
}

logic::LogicErrorCode LogicEventManager::killTimer(IN ITimerEvent* pTimerEvent)
{
	util::TTAutoLock lock(&m_lock);
	auto iter = std::remove_if(m_lstTimers.begin()
		, m_lstTimers.end()
		, [=](TTTimer& ttime)
	{
		return (pTimerEvent == ttime.pTimerEvent);
	}
	);
	if (iter != m_lstTimers.end())
	{
		m_lstTimers.erase(iter,m_lstTimers.end());
		delete pTimerEvent;
		pTimerEvent = 0;
		return LOGIC_OK;
	}

	return LOGIC_WORK_TIMER_INEXISTENCE_ERROR;
}

TTTimer::TTTimer() 
:nElapse(0)
,nDelay(0)
,bRepeat(TRUE)
,pTimerEvent(0)
{

}

NAMESPACE_END(logic)
/******************************************************************************/