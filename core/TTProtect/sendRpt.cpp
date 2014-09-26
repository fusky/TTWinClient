/******************************************************************************* 
 *  @file      sendRpt.cpp 2014\9\1 13:21:59 $
 *  @author    快刀<kuaidao@mogujie.com>
 *  @brief     
 ******************************************************************************/

#include "stdafx.h"
#include "sendRpt.h"
/******************************************************************************/

CSendRpt::CSendRpt(CCrashManager *pCrashmanager, HWND hWnd) :
m_pCrashmanager(pCrashmanager)
, m_hWnd(hWnd)
{
}

UINT CSendRpt::Run()
{
	//todo...暂时注释掉crash dump包的发送
	//if (m_pCrashmanager->HttpQueryCanUpload())
	//	m_pCrashmanager->ReportByHttp();

	::PostMessage(m_hWnd, WM_SENDRPT_FINISH, 0, 0);

	return 0;
}


void CSendRpt::Shutdown()
{
	if (!wait(2000))
		destory();
}

/******************************************************************************/