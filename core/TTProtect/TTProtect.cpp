#include "stdafx.h"
#include "TTProtect.h"
#include "TTProtectDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace
{
	#define  DUMP_MUTEX_NAME  _T("{4DE254CE-A66D-4848-9F6F-040DFD62F944}")
	const CString TEAMTALK_CLASSNAME = _T("TeamTalk CrashDump");
}

BEGIN_MESSAGE_MAP(CTTProtectApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()

CTTProtectApp::CTTProtectApp()
{
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_RESTART;
}

CTTProtectApp theApp;

BOOL CTTProtectApp::InitInstance()
{
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);
	CWinApp::InitInstance();
	AfxEnableControlContainer();

	WNDCLASS wc;
	::GetClassInfo(AfxGetInstanceHandle(), _T("#32770"), &wc);
	wc.lpszClassName = TEAMTALK_CLASSNAME;
	AfxRegisterClass(&wc);

	if (IsHaveInstance())
	{
		HWND hwndMain = FindWindow(TEAMTALK_CLASSNAME, NULL);
		if (hwndMain)
		{
			::SendMessage(hwndMain, WM_START_CRASHDLG, NULL, NULL);
		}
		return FALSE;
	}

	CTTProtectDlg dlg;
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
	}
	else if (nResponse == IDCANCEL)
	{
	}
	else if (nResponse == -1)
	{
		TRACE(traceAppMsg, 0, "警告: 对话框创建失败，应用程序将意外终止。\n");
		TRACE(traceAppMsg, 0, "警告: 如果您在对话框上使用 MFC 控件，则无法 #define _AFX_NO_MFC_CONTROLS_IN_DIALOGS。\n");
	}

	return FALSE;
}

BOOL CTTProtectApp::IsHaveInstance()
{
	HANDLE hMutex = ::CreateMutex(NULL, TRUE, DUMP_MUTEX_NAME);
	if (hMutex != NULL && GetLastError() == ERROR_ALREADY_EXISTS)
	{
		CloseHandle(hMutex);
		return TRUE;
	}

	return FALSE;
}

