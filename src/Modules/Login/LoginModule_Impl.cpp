/******************************************************************************* 
 *  @file      LoginModule_Impl.cpp 2014\7\17 19:51:47 $
 *  @author    快刀<kuaidao@mogujie.com>
 *  @brief   
 ******************************************************************************/

#include "stdafx.h"
#include "LoginModule_Impl.h"
#include "TTLogic/ILogic.h"
#include "TTLogic/ITcpClientModule.h"
#include "utility/utilCommonAPI.h"
#include "utility/Multilingual.h"
#include "delegate/FastDelegate.h"
#include "LoginDialog.h"
#include "src/base/ImPduGroup.h"
#include "src/base/ImPduClient.h"
#include "ReloginManager.h"

/******************************************************************************/

namespace module
{
	ILoginModule* getLoginModule()
	{
		return (ILoginModule*)logic::GetLogic()->getModule(MODULE_ID_LOGIN);
	}
}
// -----------------------------------------------------------------------------
//  LoginModule_Impl: Public, Constructor

LoginModule_Impl::LoginModule_Impl()
:m_pReloginManager(0)
,m_bIsOfflineByMyself(FALSE)
{

}

// -----------------------------------------------------------------------------
//  LoginModule_Impl: Public, Destructor

LoginModule_Impl::~LoginModule_Impl()
{
	delete m_pReloginManager;
	m_pReloginManager = 0;
}

void LoginModule_Impl::release()
{
	delete this;
}

BOOL LoginModule_Impl::showLoginDialog()
{
	BOOL bRet = FALSE;
	LoginDialog* pDialog = new LoginDialog();
	PTR_FALSE(pDialog);
	CString csTitle = util::getMultilingual()->getStringViaID(_T("STRID_LOGINDIALOG_BTN_LOGIN"));
	pDialog->Create(NULL, csTitle, UI_CLASSSTYLE_DIALOG, WS_EX_STATICEDGE | WS_EX_APPWINDOW, 0, 0, 0, 0);
	pDialog->CenterWindow();
	bRet = (IDOK == pDialog->ShowModal());

	return bRet;
}

void LoginModule_Impl::notifyLoginDone()
{
	logic::GetLogic()->pushBackOperationWithLambda(
		[]
	{
		//获取组织架构
		CImPduClientDepartmentRequest departmentListReq;
		logic::getTcpClientModule()->sendPacket(&departmentListReq);

		//获取所有用户列表信息
		CImPduClientAllUserRequest  allUsersInfoReq;
		logic::getTcpClientModule()->sendPacket(&allUsersInfoReq);

		//获取最近联系人
		CImPduClientBuddyListRequest recentUserInfoReq(0);
		logic::getTcpClientModule()->sendPacket(&recentUserInfoReq);

		//获取群列表
		CImPduClientGroupListRequest groupReq(CID_GROUP_LIST_REQUEST);
		logic::getTcpClientModule()->sendPacket(&groupReq);
		//获取讨论组
		CImPduClientGroupListRequest TempgroupReq(CID_GROUP_DIALOG_LIST_REQUEST);
		logic::getTcpClientModule()->sendPacket(&TempgroupReq);
	});
}

void LoginModule_Impl::onPacket(std::auto_ptr<CImPdu> pdu)
{
	CImPdu* pPdu = pdu.get();
	PTR_VOID(pPdu);
	switch (pdu->GetCommandId())
	{
	case CID_LOGIN_KICK_USER:
		_kickUserResponse(pPdu);
		break;
	default:
		return;
	}
}

void LoginModule_Impl::_kickUserResponse(CImPdu* pdu)
{
	CImPduKickUser* pKickOut = (CImPduKickUser*)pdu;
	if (KICK_REASON_DUPLICATE_USER == pKickOut->GetReason())
	{
		logic::GetLogic()->asynNotifyObserver(module::KEY_LOGIN_KICKOUT);
	}
}

void LoginModule_Impl::relogin(BOOL bForce)
{
	if (!m_pReloginManager)
	{
		m_pReloginManager = new ReloginManager;
	}
	if (bForce)
	{
		m_pReloginManager->forceRelogin();
	}
	else
	{
		m_pReloginManager->startReloginTimer(3);
	}
}

BOOL LoginModule_Impl::isOfflineByMyself()const
{
	return m_bIsOfflineByMyself;
}

void LoginModule_Impl::setOfflineByMyself(BOOL b)
{
	m_bIsOfflineByMyself = b;
}

/******************************************************************************/