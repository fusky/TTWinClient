/******************************************************************************* 
 *  @file      MKObserver.cpp 2014\7\23 11:04:42 $
 *  @author    ¿ìµ¶<kuaidao@mogujie.com>
 *  @brief   
 ******************************************************************************/

#include "stdafx.h"
#include "MKObserver.h"
#include "TTLogic/Observer.h"
#include "TTLogic/ILogic.h"
#include "utility/utilCommonAPI.h"
#include <algorithm>

/******************************************************************************/
NAMESPACE_BEGIN(logic)

// -----------------------------------------------------------------------------
//  MKObserver: Public, Constructor

MKObserver::MKObserver()
{

}

// -----------------------------------------------------------------------------
//  MKObserver: Public, Destructor

MKObserver::~MKObserver()
{
	try
	{
		_removeAllObservers();
	}
	catch (...)
	{
		APP_LOG(LOG_ERROR, _T("MKObserver: throw unknown exception"));
		assert(FALSE);
	}
}

void MKObserver::addObserver(void* pObserObject, UInt16 moduleId, IObserverHandler& handle)
{
	if (isObserverExist(handle))
		return;
	MKOContext* pMkoCtx = new MKOContext(moduleId, handle, pObserObject);
	{
		util::TTAutoLock lock(&m_lockObserver);
		m_vecObservers.push_back(pMkoCtx);
	}
}

void MKObserver::removeObserver(void* pObserObject)
{
	std::vector<MKOContext*> vecRemove;
	util::TTAutoLock lock(&m_lockObserver);
	auto iter = std::remove_if(m_vecObservers.begin(), m_vecObservers.end(), 
		[=](MKOContext* pCtxItem)
	{
		bool b = (pObserObject == pCtxItem->m_pObserverObject);
		if (b)
		{
			delete pCtxItem;
			pCtxItem = 0;
		}
		return b;
	}
	);
	if (iter != m_vecObservers.end())
	{
		m_vecObservers.erase(iter,m_vecObservers.end());
	}
}

void MKObserver::_removeAllObservers()
{
	util::TTAutoLock lock(&m_lockObserver);
	for (MKOContext* pCtx : m_vecObservers)
	{
		delete pCtx;
		pCtx = 0;
	}
	m_vecObservers.clear();
}

void MKObserver::asynNotifyObserver(UInt32 keyId)
{
	ObserverEvent_Impl* pEvent = new ObserverEvent_Impl(this);
	_asynNotifyObserver(keyId, pEvent);
}

void MKObserver::asynNotifyObserver(UInt32 keyId, std::string& mkoString)
{
	ObserverEvent_Impl* pEvent = new ObserverEvent_Impl(this);
	pEvent->m_mkoString = mkoString;
	_asynNotifyObserver(keyId, pEvent);
}

void MKObserver::asynNotifyObserver(UInt32 keyId, Int32 mkoInt)
{
	ObserverEvent_Impl* pEvent = new ObserverEvent_Impl(this);
	pEvent->m_mkoInt = mkoInt;
	_asynNotifyObserver(keyId, pEvent);
}

void MKObserver::asynNotifyObserver(UInt32 keyId, void* pmkoVoid)
{
	ObserverEvent_Impl* pEvent = new ObserverEvent_Impl(this);
	pEvent->m_pmkoVoid = pmkoVoid;
	_asynNotifyObserver(keyId, pEvent);
}

void MKObserver::asynNotifyObserver(UInt32 keyId, std::shared_ptr<void> pmkoShardVoid)
{
	ObserverEvent_Impl* pEvent = new ObserverEvent_Impl(this);
	pEvent->m_pmkoShardVoid = pmkoShardVoid;
	_asynNotifyObserver(keyId, pEvent);
}

BOOL MKObserver::isObserverExist(const IObserverHandler& handle)
{
	util::TTAutoLock lock(&m_lockObserver);
	auto iterObserver = std::find_if(m_vecObservers.begin(), m_vecObservers.end(),
		[=](MKOContext* pMKOCtx)
	{
		return (handle == pMKOCtx->handler);
	}
	);
	return (iterObserver != m_vecObservers.end());
}

void MKObserver::_assembleObservers(ObserverEvent_Impl* pEvent)
{
	util::TTAutoLock lock(&m_lockObserver);
	for (MKOContext* pCtx : m_vecObservers)
	{
		if (pCtx->moduleId == pEvent->m_moduleId)
		{
			pEvent->m_lstObserverHandlers.push_back(pCtx->handler);
		}
	}
}

void MKObserver::_asynNotifyObserver(UInt32 keyId, ObserverEvent_Impl* pEvent)
{
	pEvent->m_moduleId = keyId >> 16;
	pEvent->m_keyId = keyId;
	_assembleObservers(pEvent);

	logic::GetLogic()->asynFireUIEvent(pEvent);
}

NAMESPACE_END(logic)
/******************************************************************************/