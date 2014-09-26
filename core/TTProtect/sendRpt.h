/*******************************************************************************
 *  @file      sendRpt.h 2014\9\1 13:21:16 $
 *  @author    ¿ìµ¶<kuaidao@mogujie.com>
 *  @brief     
 ******************************************************************************/

#ifndef SENDRPT_9092B430_4D16_4699_9E2C_A5BB6B3370E1_H__
#define SENDRPT_9092B430_4D16_4699_9E2C_A5BB6B3370E1_H__

#include <list>
#include "utility/TTThread.h"
#include "utility/TTAutoLock.h"
#include "CrashManager.h"
/******************************************************************************/

#define		WM_SENDRPT_FINISH		WM_USER + 100

class CSendRpt : public util::TTThread
{
public:
	CSendRpt(CCrashManager *pCrashmanager, HWND hWnd);
	virtual ~CSendRpt(){}
	void Shutdown();

protected:
	virtual UINT Run();

private:
	CCrashManager	*m_pCrashmanager;
	HWND	m_hWnd;
};

/******************************************************************************/
#endif// SENDRPT_9092B430_4D16_4699_9E2C_A5BB6B3370E1_H__