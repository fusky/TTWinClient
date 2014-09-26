/******************************************************************************* 
 *  @file      SyncTimerTimer.cpp 2013\9\6 14:49:09 $
 *  @author    ¿ìµ¶<kuaidao@mogujie.com>
 *  @brief   
 ******************************************************************************/

#include "stdafx.h"
#include "SyncTimerTimer.h"
#include "TTLogic/ILogic.h"
#include "TTLogic/ITcpClientModule.h"
#include "src/base/ImPduClient.h"

/******************************************************************************/

// -----------------------------------------------------------------------------
//  SyncTimerTimer: Public, Constructor

SyncTimeTimer::SyncTimeTimer()
:m_timeCount(0)
,m_serverTime(0)
{
    m_serverTime = (UInt32)time(0);
}

// -----------------------------------------------------------------------------
//  SyncTimerTimer: Public, Destructor

SyncTimeTimer::~SyncTimeTimer()
{

}

void SyncTimeTimer::process()
{
    ++m_serverTime;
    ++m_timeCount;
    if(m_timeCount >= 600)
    {
        m_timeCount = 0;
		logic::GetLogic()->pushBackOperationWithLambda(
			[]()
		{
			CImPduClientTimeRequest pduTime;
			logic::getTcpClientModule()->sendPacket(&pduTime);
		});
    }
}

void SyncTimeTimer::release()
{
    delete this;
}

/******************************************************************************/
