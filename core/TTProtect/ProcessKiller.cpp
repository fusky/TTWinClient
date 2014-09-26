/******************************************************************************* 
 *  @file      ProcessKiller.cpp 2014\9\1 13:20:32 $
 *  @author    ¿ìµ¶<kuaidao@mogujie.com>
 *  @brief     
 ******************************************************************************/

#include "StdAfx.h"
#include "psapi.h"
#include "ProcessKiller.h"
#include "config.h"
/******************************************************************************/

CProcessKiller::CProcessKiller()
{
}

CProcessKiller::~CProcessKiller()
{

}

CString	CProcessKiller::GetProcessName(DWORD processID)
{
	TCHAR szProcessName[MAX_PATH];
	HMODULE hMod;
	DWORD cbNeeded;
	CString csProcessName = _T("");

	HANDLE hProcess = ::OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processID);

	if (hProcess != NULL)
	{
		if (::EnumProcessModules(hProcess, &hMod, sizeof(hMod), &cbNeeded))
		{
			if (GetModuleBaseName(hProcess, hMod, szProcessName, sizeof(TCHAR)* MAX_PATH) > 0)
				csProcessName = CString(szProcessName);
		}
		CloseHandle(hProcess);
	}
	return csProcessName;
}

BOOL CProcessKiller::KillProcess(DWORD dwProcessId)
{
	HANDLE hProcess = ::OpenProcess(PROCESS_TERMINATE, FALSE, dwProcessId);
	if (hProcess == NULL)
		return false;
	BOOL bRet = ::TerminateProcess(hProcess, 0);
	CloseHandle(hProcess);
	if (bRet == FALSE)
		return false;
	else
		return TRUE;
}

void CProcessKiller::KillTTProcesses()
{
	DWORD dwProcessId[4096];
	DWORD dwProcessCount;
	::EnumProcesses(dwProcessId, 4096, &dwProcessCount);
	dwProcessCount /= sizeof(DWORD);
	for (size_t i = 0; i < dwProcessCount; i++)
	{
		CString csProcessImage = GetProcessName(dwProcessId[i]);
		{
			if (csProcessImage == TEAMTALK_EXE_NAME)
			{
				KillProcess(dwProcessId[i]);
			}

		}
	}
}
/******************************************************************************/