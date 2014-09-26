/*******************************************************************************
 *  @file      Observer.h 2014\7\23 13:52:44 $
 *  @author    快刀<kuaidao@mogujie.com>
 *  @brief   MKO Observer事件定义，参数定义等
 ******************************************************************************/

#ifndef OBSERVER_6444A026_CF7D_4675_BD86_A112B52B992D_H__
#define OBSERVER_6444A026_CF7D_4675_BD86_A112B52B992D_H__

#include "GlobalDefine.h"
#include "TTLogic/IEvent.h"
#include "delegate/FastDelegate.h"
#include <list>
#include <tuple> 
#include <memory>
/******************************************************************************/
typedef std::tuple<std::string, Int32, std::shared_ptr<void>, void*> MKO_TUPLE_PARAM;
enum
{
	MKO_STRING = 0,
	MKO_INT,
	MKO_SHARED_VOIDPTR,
	MKO_VOIDPTR,
};

NAMESPACE_BEGIN(logic)
class MKObserver;

typedef fastdelegate::FastDelegate3<UInt16, UInt32,MKO_TUPLE_PARAM>	IObserverHandler;

struct IObserverEvent : IEvent
{
};

class ObserverEvent_Impl final : public IObserverEvent
{
public:
	ObserverEvent_Impl(MKObserver* p);
	virtual ~ObserverEvent_Impl();
public:
	virtual void process();
	virtual void release();

public:
	UInt16							m_moduleId;
	UInt32							m_keyId;
	Int32							m_mkoInt;
	void*							m_pmkoVoid;
	std::string						m_mkoString;
	std::shared_ptr<void>			m_pmkoShardVoid;
	std::list<IObserverHandler>     m_lstObserverHandlers;

	MKObserver*						m_pMko;        //为了调用MKObserver的接口，这里放一个映射指针
};

class MKOContext
{
public:
	MKOContext(UInt16 mId, IObserverHandler& hd, void* pObserObject);

public:
	UInt16				moduleId;
	UInt32				keyId;
	void*				m_pObserverObject;
	IObserverHandler	handler;
};

NAMESPACE_END(logic)
/******************************************************************************/
#endif// OBSERVER_6444a026-cf7d-4675-bd86-a112b52b992d_H__
