#include "stdafx.h"
#include "DumpReporter.h"
#include "teamtalk.h"
#include "UI/MainDialog.h"
#include "GlobalConfig.h"
#include "utility/utilCommonAPI.h"
#include "utility/Multilingual.h"
#include "TTLogic/ILogic.h"
#include "Modules/IHttpPoolModule.h"
#include "Modules/ILoginModule.h"
#include "Modules/IMiscModule.h"
#include "Modules/ISysConfigModule.h"
void CALLBACK PreExceptionProc();
#ifndef _DEBUG
	CDumpReporter::CInstaller g_dumpInstall(PreExceptionProc);
#endif

//	在这里做一些异常的预处理
#ifndef _DEBUG
void CALLBACK PreExceptionProc()
{
}
#endif

BEGIN_MESSAGE_MAP(CteamtalkApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()

CteamtalkApp::CteamtalkApp()
:m_pMainDialog(0)
{
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_RESTART;
}

CteamtalkApp theApp;

BOOL CteamtalkApp::InitInstance()
{
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	APP_LOG(LOG_INFO, _T("==============================================================================="));
	if (!__super::InitInstance())
	{
		APP_LOG(LOG_ERROR,_T("App: __super::InitInstance failed."));
		return FALSE;
	}
	AfxEnableControlContainer();
	WSADATA wsa_data;
	WSAStartup(MAKEWORD(2, 2), &wsa_data);


	if (IsHaveInstance())
	{
		APP_LOG(LOG_ERROR,_T("App: Had one instance,this will exit"));
		HWND hwndMain = FindWindow(_T("TeamTalkMainDialog"), NULL);
		if (hwndMain)
		{
			::SendMessage(hwndMain, WM_START_MOGUTALKINSTANCE, NULL, NULL);
		}
		return FALSE;
	}
	//多语言
	if (!util::getMultilingual()->loadStringTable(util::getParentAppPath() + UTIL_MULTILIGNUAL))
	{
		APP_LOG(LOG_ERROR, _T("App: Multilingual init failed"));
		return FALSE;
	}
	APP_LOG(LOG_INFO, _T("App: Multilingual init done"));

	//TTLogic框架启动
	if (logic::LOGIC_OK != logic::CreateSingletonILogic()
		|| logic::LOGIC_OK != logic::GetLogic()->startup())
	{
		APP_LOG(LOG_ERROR, _T("App: TTLogic init failed"));
		return FALSE;
	}
	APP_LOG(LOG_INFO, _T("App: TTLogic init done"));

	//注册modules
	__declspec(dllimport) BOOL  loadModules(logic::ILogic* pLogic);
	if (!loadModules(logic::GetLogic()))
	{
		APP_LOG(LOG_ERROR, 1, _T("App: load modules failed"));
		return FALSE;
	}
	APP_LOG(LOG_INFO, 1, _T("App: load modules done"));
	//创建用户目录
	_CreateUsersFolder();
	
	//duilib初始化
	CPaintManagerUI::SetInstance(AfxGetInstanceHandle());
	CPaintManagerUI::SetResourcePath(CPaintManagerUI::GetInstancePath() + _T("..\\gui\\"));//track这个设置了路径，会导致base里设置的无效。
	::CoInitialize(NULL);
	::OleInitialize(NULL);

	module::TTConfig* pCfg = module::getSysConfigModule()->getSystemConfig();
	if (pCfg && pCfg->phpServerAddr.empty())
	{
		if (!module::getSysConfigModule()->showServerConfigDialog(NULL))
		{
			APP_LOG(LOG_ERROR, _T("App: server config canceled"));
			return FALSE;
		}
	}

	if (!module::getLoginModule()->showLoginDialog())
	{
		APP_LOG(LOG_ERROR, _T("App: login canceled"));
		return FALSE;
	}
	APP_LOG(LOG_INFO, 1, _T("App: login success"));

	//创建主窗口
	if (!_CreateMainDialog())
	{
		APP_LOG(LOG_ERROR, _T("App: Create MianDialog failed"));
		return FALSE;
	}
	APP_LOG(LOG_INFO, _T("App: Create MianDialog done"));

	CPaintManagerUI::MessageLoop();
	CPaintManagerUI::Term();

	::OleUninitialize();
	::CoUninitialize();

	return TRUE;
}

BOOL CteamtalkApp::_DestroyMainDialog()
{
	delete m_pMainDialog;
	m_pMainDialog = 0;
	return TRUE;
}

BOOL CteamtalkApp::_CreateMainDialog()
{
	m_pMainDialog = new MainDialog();
	PTR_FALSE(m_pMainDialog);
	CString csTitle = util::getMultilingual()->getStringViaID(_T("STRID_GLOBAL_CAPTION_NAME"));
	if (!m_pMainDialog->Create(NULL, csTitle
		, UI_CLASSSTYLE_DIALOG, WS_EX_STATICEDGE | WS_EX_APPWINDOW, 0, 0, 600, 800))
		return FALSE;
	m_pMainDialog->BringToTop();

	return TRUE;
}

BOOL CteamtalkApp::ExitInstance()
{
	APP_LOG(LOG_INFO, _T("App:Exit Instance"));
	logic::ILogic* pLogic = logic::GetLogic();
	PTR_FALSE(pLogic);
	//关闭http线程池
	module::getHttpPoolModule()->shutdown();

	//逻辑引擎关闭阶段1
	logic::GetLogic()->shutdownPhase1();
	APP_LOG(LOG_INFO, _T("App:TTLogic shutdown phase1 done"));

	_DestroyMainDialog();
	APP_LOG(LOG_INFO, 1, _T("App:MainDialog Destory done"));

	//逻辑引擎关闭阶段2
	pLogic->shutdownPhase2();
	logic::DestroySingletonILogic();
	APP_LOG(LOG_INFO, _T("App:TTLogic shutdown phase2 done"));

	WSACleanup();

	APP_LOG(LOG_INFO, _T("Exit OK"));
	return __super::ExitInstance();
}

BOOL CteamtalkApp::_CreateUsersFolder()
{
	module::IMiscModule* pModule = module::getMiscModule();
	//users目录
	if (!util::createAllDirectories(pModule->getUsersDir()))
	{
		APP_LOG(LOG_ERROR, _T("_CreateUsersFolder users direcotry failed!"));
		return FALSE;
	}
	//下载目录
	if (!util::createAllDirectories(pModule->getDownloadDir()))
	{
		APP_LOG(LOG_ERROR, _T("_CreateUsersFolder download direcotry failed!"));
		return FALSE;
	}

	return TRUE;
}
#ifdef _DEBUG
	#define  AppSingletonMutex _T("{7A666640-EDB3-44CC-954B-0C43F35A2E17}")
#else
	#define  AppSingletonMutex _T("{5676532A-6F70-460D-A1F0-81D6E68F046A}")
#endif
BOOL CteamtalkApp::IsHaveInstance()
{
	// 单实例运行
	HANDLE hMutex = ::CreateMutex(NULL, TRUE, AppSingletonMutex);
	if (hMutex != NULL && GetLastError() == ERROR_ALREADY_EXISTS)
	{
		return TRUE;
	}

	return FALSE;
}
