/*******************************************************************************
 *  @file      ProcessKiller.h 2014\9\1 13:19:53 $
 *  @author    ¿ìµ¶<kuaidao@mogujie.com>
 *  @brief     
 ******************************************************************************/

#ifndef PROCESSKILLER_D824A144_3413_4A65_B1C9_8E58A87268FB_H__
#define PROCESSKILLER_D824A144_3413_4A65_B1C9_8E58A87268FB_H__

#include <list>
/******************************************************************************/

class CProcessKiller
{
public:
	CProcessKiller(void);
	~CProcessKiller(void);

public:
	static void	KillTTProcesses();

	static CString	GetProcessName(DWORD processID);

	static BOOL	KillProcess(DWORD dwProcessId);
};

/******************************************************************************/
#endif// PROCESSKILLER_D824A144_3413_4A65_B1C9_8E58A87268FB_H__