/*******************************************************************************
 *  @file      TTAutoLock.h 2014\7\11 11:19:46 $
 *  @author    快刀<kuaidao@mogujie.com>
 *  @brief   自动锁工具类
 ******************************************************************************/

#ifndef TTAUTOLOCK_6154B031_97E0_4698_93BD_709F44EAB249_H__
#define TTAUTOLOCK_6154B031_97E0_4698_93BD_709F44EAB249_H__

#include "GlobalDefine.h"
#include <afxmt.h>

/******************************************************************************/
NAMESPACE_BEGIN(util)

typedef CSyncObject			TTLockBase;
typedef CCriticalSection	TTFastLock;
typedef CMutex				TTMutexLock;

/**
 * The class <code>TTAutoLock</code> 
 *
 */
class TTAutoLock
{
public:
    /** @name Constructors and Destructor*/

    //@{
    /**
     * Constructor 
     */
	TTAutoLock(TTLockBase* pLock)
		:m_pLock(pLock)
	{
		assert(pLock);
		if (0 != (m_pLock = pLock))
			m_pLock->Lock();
	}
    /**
     * Destructor
     */
	~TTAutoLock()
	{
		if (m_pLock)
			m_pLock->Unlock();
	}
    //@}

private:
	TTAutoLock(const TTAutoLock&);
	TTAutoLock& operator=(const TTAutoLock&);

private:
	TTLockBase* m_pLock;
};

NAMESPACE_END(util)
/******************************************************************************/
#endif// TTAUTOLOCK_6154B031_97E0_4698_93BD_709F44EAB249_H__
