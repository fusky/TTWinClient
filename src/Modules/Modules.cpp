#include "stdafx.h"
#include "Modules.h"
#include "GlobalMacros.h"
#include "TTLogic/ILogic.h"
#include "Login/LoginModule_Impl.h"
#include "HttpPool/HttpPoolModule_Impl.h"
#include "Session/SessionModule_Impl.h"
#include "Database/DatabaseModule_Impl.h"
#include "Message/MessageModule_Impl.h"
#include "Misc/MiscModule_Impl.h"
#include "SysConfig/SysConfigModule_Impl.h"
#include "GroupList/GroupListModule_Impl.h"
#include "UserList/UserListModule_Impl.h"
#include "Emotion/EmotionModule_Impl.h"
#include "Capture/CaptureModule_Impl.h"
#include "P2PCmd/P2PCmdModule_Impl.h"
#include "FileTransfer/FileTransferModule_Impl.h"

__declspec(dllexport) BOOL loadModules(logic::ILogic* pLogic)
{
	logic::LogicErrorCode errCode = logic::LOGIC_OK;

	errCode = pLogic->registerModule(new MiscModule_Impl());
	CHECK_BOOL(logic::LOGIC_OK == errCode);

	errCode = pLogic->registerModule(new SysConfigModule_Impl());
	CHECK_BOOL(logic::LOGIC_OK == errCode);

	errCode = pLogic->registerModule(new LoginModule_Impl());
	CHECK_BOOL(logic::LOGIC_OK == errCode);

	errCode = pLogic->registerModule(new HttpPoolModule_Impl());
	CHECK_BOOL(logic::LOGIC_OK == errCode);

	errCode = pLogic->registerModule(new SessionModule_Impl());
	CHECK_BOOL(logic::LOGIC_OK == errCode);

	errCode = pLogic->registerModule(new DatabaseModule_Impl());
	CHECK_BOOL(logic::LOGIC_OK == errCode);

	errCode = pLogic->registerModule(new MessageModule_Impl());
	CHECK_BOOL(logic::LOGIC_OK == errCode);

	errCode = pLogic->registerModule(new UserListModule_Impl());
	CHECK_BOOL(logic::LOGIC_OK == errCode);

	errCode = pLogic->registerModule(new GroupListModule_Impl());
	CHECK_BOOL(logic::LOGIC_OK == errCode);

	errCode = pLogic->registerModule(new EmotionModule_Impl());
	CHECK_BOOL(logic::LOGIC_OK == errCode);

	errCode = pLogic->registerModule(new CaptureModule_Impl());
	CHECK_BOOL(logic::LOGIC_OK == errCode);

	errCode = pLogic->registerModule(new P2PCmdModule_Impl());
	CHECK_BOOL(logic::LOGIC_OK == errCode);

	errCode = pLogic->registerModule(new FileTransferModule_Impl());
	CHECK_BOOL(logic::LOGIC_OK == errCode);

	return TRUE;

END0:
	return FALSE;
}

BEGIN_MESSAGE_MAP(CModulesApp, CWinApp)
END_MESSAGE_MAP()

CModulesApp::CModulesApp()
{
}

CModulesApp theApp;

BOOL CModulesApp::InitInstance()
{
	CWinApp::InitInstance();

	return TRUE;
}
