/******************************************************************************* 
 *  @file      DumpReporter.cpp 2014\9\1 14:18:35 $
 *  @author    快刀<kuaidao@mogujie.com>
 *  @brief     
 ******************************************************************************/

#include "StdAfx.h"
#include "DumpReporter.h"
#include <exception>
/******************************************************************************/

TCHAR CDumpReporter::m_ProgramPath[MAX_PATH] = { 0 };
BOOL CDumpReporter::m_bExceptionFilterInstalled = false;
CDumpReporter::CReportDetail CDumpReporter::m_ExceptionReportDetails;
LPTOP_LEVEL_EXCEPTION_FILTER CDumpReporter::m_OldFilter = NULL;
PRE_EXCEPTION_PROC CDumpReporter::m_preExceptionProc = NULL;

void CDumpReporter::CReportDetail::SetProcessId(DWORD dwPid)
{
	m_dwPid = dwPid;
}

void CDumpReporter::CReportDetail::SetThreadId(DWORD dwTid)
{
	m_dwTid = dwTid;
}

void CDumpReporter::CReportDetail::SetExceptionAddr(PEXCEPTION_POINTERS pException)
{
	m_dwExceptionAddr = DWORD(unsigned __int64(pException));
}

void CDumpReporter::CReportDetail::SetParameter(CString sParam)
{
	m_sParameter = sParam;
}

void CDumpReporter::CReportDetail::SetAppPath(CString sPath)
{
	m_sAppPath = sPath;
}
CString CDumpReporter::CReportDetail::ToCommandLine()
{
	CString sCmdLine;
	sCmdLine.Format(_T("pid=%d;tid=%d;excaddr=%d;param=%s;apppath=%s"),
		m_dwPid, m_dwTid, m_dwExceptionAddr, m_sParameter, m_sAppPath);
	return sCmdLine;
}

CDumpReporter::CInstaller::CInstaller(PRE_EXCEPTION_PROC preExceptionProc, LPCTSTR ProgramPath)
{
	CDumpReporter::Initialize(preExceptionProc, ProgramPath);
	CDumpReporter::InstallExceptionFilter();
}

CDumpReporter::CInstaller::~CInstaller()
{
	CDumpReporter::UninstallExceptionFilter();
}

void CDumpReporter::CInstaller::SetParameter(CString sParam)
{
	CDumpReporter::m_ExceptionReportDetails.SetParameter(sParam);
}

CDumpReporter::CDumpReporter()
{
}

CDumpReporter::~CDumpReporter()
{
}

BOOL CDumpReporter::Initialize(PRE_EXCEPTION_PROC preExceptionProc, LPCTSTR ProgramPath)
{
	m_preExceptionProc = preExceptionProc;
	if (NULL == ProgramPath)
	{
		::GetModuleFileName(NULL, m_ProgramPath, MAX_PATH);
		_tcsrchr(m_ProgramPath, _T('\\'))[1] = 0;
#ifdef _DEBUG
		_tcscat_s(m_ProgramPath, _T("TTProtect.exe"));
#else
		_tcscat_s(m_ProgramPath, _T("TTProtect.exe"));
#endif
	}
	else
		_tcscpy_s(m_ProgramPath, ProgramPath);
	WIN32_FIND_DATA fd;
	HANDLE hFind = ::FindFirstFile(m_ProgramPath, &fd);
	if (INVALID_HANDLE_VALUE == hFind)
	{
		m_ProgramPath[0] = 0;
		return FALSE;
	}
	else
	{
		::FindClose(hFind);
		return TRUE;
	}
}

void CDumpReporter::InstallExceptionFilter()
{
	if (!m_bExceptionFilterInstalled)
	{
		SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX);
		m_OldFilter = SetUnhandledExceptionFilter(TopLevelExceptionFilter);
		m_bExceptionFilterInstalled = TRUE;
	}
}

void CDumpReporter::UninstallExceptionFilter()
{
	if (m_bExceptionFilterInstalled)
	{
		SetUnhandledExceptionFilter(m_OldFilter);
		m_bExceptionFilterInstalled = false;
	}
}

LONG WINAPI CDumpReporter::TopLevelExceptionFilter(EXCEPTION_POINTERS* pExceptionInfo)
{
	::ShowWindow(AfxGetMainWnd()->GetSafeHwnd(), SW_HIDE);
	m_ExceptionReportDetails.SetProcessId(GetCurrentProcessId());
	m_ExceptionReportDetails.SetThreadId(GetCurrentThreadId());
	m_ExceptionReportDetails.SetExceptionAddr(pExceptionInfo);

	TCHAR sAppPath[MAX_PATH];
	::GetModuleFileName(NULL, sAppPath, MAX_PATH);
	m_ExceptionReportDetails.SetAppPath(sAppPath);

	DWORD dwResult = Report(m_ExceptionReportDetails, INFINITE);

	LONG ret = EXCEPTION_EXECUTE_HANDLER;
	if (dwResult == REPORT_RESULT_DEBUG)
		ret = EXCEPTION_CONTINUE_SEARCH;
	else if (dwResult == REPORT_RESULT_NOTFOUND)
		MessageBox(GetActiveWindow(), _T("Unknown Error! ttprotect not found!"), _T("ERROR"), MB_OK);

	return ret;
}

DWORD CDumpReporter::Report(CReportDetail& Details, DWORD Timeout)
{
	// 执行预处理。该预处理函数由这个类的使用者提供
	if (m_preExceptionProc)
		m_preExceptionProc();

	if (m_ProgramPath[0] == 0)
		return 0;
	CString sCommandLine = Details.ToCommandLine();
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	memset(&si, 0, sizeof(si));
	si.cb = sizeof(si);
	si.wShowWindow = SW_SHOW;
	BOOL bCreate = ::CreateProcess(m_ProgramPath, sCommandLine.GetBuffer(), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
	if (!bCreate)
		return REPORT_RESULT_NOTFOUND;
	::WaitForSingleObject(pi.hProcess, Timeout);
	DWORD dwExitCode;
	::GetExitCodeProcess(pi.hProcess, &dwExitCode);
	::CloseHandle(pi.hProcess);
	::CloseHandle(pi.hThread);
	return dwExitCode;
}
/******************************************************************************/