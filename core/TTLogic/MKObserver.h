/*******************************************************************************
 *  @file      MKObserver.h 2014\7\23 11:01:03 $
 *  @author    快刀<kuaidao@mogujie.com>
 *  @brief   基于[moduleId,keyId]的observer,和框架的module拆分结合起来
 ******************************************************************************/

#ifndef MKOBSERVER_3E97AAE6_0F4C_4129_AD79_FAEDAFEA3BB6_H__
#define MKOBSERVER_3E97AAE6_0F4C_4129_AD79_FAEDAFEA3BB6_H__

#include "GlobalDefine.h"
#include "utility/TTAutoLock.h"
#include "TTLogic/Observer.h"
#include <vector>
/******************************************************************************/
NAMESPACE_BEGIN(logic)
/**
 * The class <code>MKObserver</code> 
 *
 */
class MKObserver final
{
public:
    /** @name Constructors and Destructor*/

    //@{
    /**
     * Constructor 
     */
    MKObserver();
    /**
     * Destructor
     */
    ~MKObserver();
    //@}
public:
	void addObserver(void* pObserObject,UInt16 moduleId, IObserverHandler& handle);
	void removeObserver(void* pObserObject);
	void asynNotifyObserver(UInt32 keyId);
	void asynNotifyObserver(UInt32 keyId,std::string& mkoString);
	void asynNotifyObserver(UInt32 keyId, Int32 mkoInt);
	void asynNotifyObserver(UInt32 keyId, void* pmkoVoid);
	void asynNotifyObserver(UInt32 keyId, std::shared_ptr<void> pmkoShardVoid);
	BOOL isObserverExist(const IObserverHandler& handle);

private:
	void _removeAllObservers();
	void _assembleObservers(ObserverEvent_Impl* pEvent);
	void _asynNotifyObserver(UInt32 keyId,ObserverEvent_Impl* pEvent);

private:
	std::vector<MKOContext*>	m_vecObservers;
	util::TTFastLock			m_lockObserver;
};
NAMESPACE_END(logic)
/******************************************************************************/
#endif// MKOBSERVER_3e97aae6-0f4c-4129-ad79-faedafea3bb6_H__
