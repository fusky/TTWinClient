/******************************************************************************* 
 *  @file      Observer.cpp 2014\7\23 13:55:19 $
 *  @author    ¿ìµ¶<kuaidao@mogujie.com>
 *  @brief   
 ******************************************************************************/

#include "stdafx.h"
#include "TTLogic/Observer.h"
#include "MKObserver.h"
#include <algorithm>
/******************************************************************************/
NAMESPACE_BEGIN(logic)

MKOContext::MKOContext(UInt16 mId, IObserverHandler& hd, void* pObserObject)
: moduleId(mId)
, keyId(0)
, handler(hd)
, m_pObserverObject(pObserObject)
{

}


void ObserverEvent_Impl::process()
{
	std::vector<IObserverHandler> vecTempObserverHandler;
	for (IObserverHandler handler : m_lstObserverHandlers)
	{
		if (m_pMko->isObserverExist(handler))
		{
			handler(m_moduleId, m_keyId, std::make_tuple(m_mkoString, m_mkoInt, m_pmkoShardVoid, m_pmkoVoid));
		}
		else
		{
			vecTempObserverHandler.push_back(handler);
		}
	}
	for (IObserverHandler handler : vecTempObserverHandler)
	{
		auto iter = std::remove_if(m_lstObserverHandlers.begin(), m_lstObserverHandlers.end(),
			[=](IObserverHandler hd)
		{
			return (handler == hd);
		}
		);
		if (iter != m_lstObserverHandlers.end())
		{
			m_lstObserverHandlers.erase(iter, m_lstObserverHandlers.end());
		}
	}
}

void ObserverEvent_Impl::release()
{
	delete this;
}

ObserverEvent_Impl::ObserverEvent_Impl(MKObserver* p)
: m_pMko(p)
, m_mkoInt(0)
, m_pmkoVoid(0)
{

}

ObserverEvent_Impl::~ObserverEvent_Impl()
{
}

NAMESPACE_END(logic)
/******************************************************************************/