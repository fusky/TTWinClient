/*******************************************************************************
 *  @file      IEvent.h 2014\7\18 15:04:19 $
 *  @author    快刀<kuaidao@mogujie.com>
 *  @brief   
 ******************************************************************************/

#ifndef IEVENT_C22447FE_9B3D_47E6_B729_2638BE8D264A_H__
#define IEVENT_C22447FE_9B3D_47E6_B729_2638BE8D264A_H__

#include "GlobalDefine.h"
#include "TTLogicDll.h"
#include "utility/utilCommonAPI.h"
/******************************************************************************/
NAMESPACE_BEGIN(logic)

struct ILogic;
struct Exception;

/**
 * The class <code>IEvent</code> 
 *
 */
struct TTLOGIC_API IEvent
{
public:
	virtual void process() = 0;
	virtual void onException(logic::Exception* e)
	{
		APP_LOG(LOG_ERROR, _T("IEvent exception,%d"));
		assert(FALSE);
	}
	virtual void release() = 0;
};

/**
* The class <code>Timer的Event，目前只是当做tag</code>
*
*/
struct TTLOGIC_API ITimerEvent : public IEvent
{};

NAMESPACE_END(logic)
/******************************************************************************/
#endif// IEVENT_C22447FE_9B3D_47E6_B729_2638BE8D264A_H__
