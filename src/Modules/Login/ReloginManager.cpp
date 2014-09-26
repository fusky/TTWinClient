/******************************************************************************* 
 *  @file      ReloginManager.cpp 2013\9\4 16:44:21 $
 *  @author    快刀<kuaidao@mogujie.com>
 *  @brief   
 ******************************************************************************/

#include "stdafx.h"
#include "ReloginManager.h"
#include "LoginOperation.h"
#include "DoLoginServer.h"
#include "Modules/IUserListModule.h"
#include "Modules/ISysConfigModule.h"
#include "Modules/ILoginModule.h"
#include "Modules/ISessionModule.h"
#include "Modules/IMiscModule.h"
#include "utility/Multilingual.h"
#include "TTLogic/ITcpClientModule.h"
/******************************************************************************/
// -----------------------------------------------------------------------------
//  ReloginManager: Public, Constructor

ReloginManager::ReloginManager()
:m_secondCount(3)
,m_bDoReloginNow(FALSE)
{

}

// -----------------------------------------------------------------------------
//  ReloginManager: Public, Destructor

ReloginManager::~ReloginManager()
{
}
// -----------------------------------------------------------------------------
// public   
void ReloginManager::startReloginTimer(UInt32 second)
{
    if(second > 15)
        second = 15;
	logic::ITimerEvent* pTimer = 0;
	logic::GetLogic()->scheduleTimerWithLambda(second, FALSE,
		[=]()
	{
		doRelogin();
	}
	, &pTimer);
    m_secondCount = second;
}
/******************************************************************************/
// -----------------------------------------------------------------------------
// public   virtual 
void ReloginTimer::execute(logic::ILogic* pLogic)
{
    m_pManager->doRelogin();
}
// -----------------------------------------------------------------------------
// public   virtual 
void ReloginTimer::release()
{
    delete this;
}
// -----------------------------------------------------------------------------
// public   
ReloginTimer::ReloginTimer(ReloginManager* pManager)
:m_pManager(pManager)
{

}
// -----------------------------------------------------------------------------
// public   
void ReloginManager::forceRelogin()
{
    doRelogin();
}

// -----------------------------------------------------------------------------
// private   
void ReloginManager::doRelogin()
{
    try
    {
        if(m_bDoReloginNow)
        {
            APP_LOG(LOG_DEBUG,TRUE,_T("is doing Relogin now..."));
            return;
        }

        if(!DoLoginServer::getInstance()->createLink())
        {
            APP_LOG(LOG_ERROR, 1, _T("Create relogin link service error"));
            if(USER_STATUS_OFFLINE == module::getUserListModule()->getMyLineStatus())
            {
                throw std::runtime_error( "Create Link Service Error" );
            }
            else
            {
                DoLoginServer::getInstance()->shutdown();
            }
            m_bDoReloginNow = FALSE;
            return;
        }

		if (!logic::getTcpClientModule()->create())
        {
            APP_LOG(LOG_ERROR, 1, _T("Create relogin logic engine link service error"));
			if (USER_STATUS_OFFLINE == module::getUserListModule()->getMyLineStatus())
            {
                throw std::runtime_error( "Create Link Service Error" );
            }
            else
            {
                DoLoginServer::getInstance()->shutdown();
            }
            m_bDoReloginNow = FALSE;
            return;
        }

        logic::ILogic* pLogic = logic::GetLogic();
        LoginParam param;
		module::TTConfig* pCfg = module::getSysConfigModule()->getSystemConfig();
		param.mySelectedStatus = pCfg->myselectStatus;
		param.csUserName = pCfg->userName;
		param.password = pCfg->password;
		param.csUserName.Trim();
		LoginOperation* pOperation = new LoginOperation(
			fastdelegate::MakeDelegate(this, &ReloginManager::OnOperationCallback), param);
		logic::GetLogic()->pushBackOperation(pOperation);
        m_bDoReloginNow = TRUE;
    }
    catch (...)
    {
		logic::getTcpClientModule()->closeSocket();
		logic::getTcpClientModule()->shutdown();
        startReloginTimer(++m_secondCount);
        DoLoginServer::getInstance()->shutdown();
        APP_LOG(LOG_ERROR, _T("relogin unknown exception"));
        m_bDoReloginNow = FALSE;
    }
}

void ReloginManager::OnOperationCallback(std::shared_ptr<void> param)
{
	m_bDoReloginNow = FALSE;
	LoginParam* pLoginParam = (LoginParam*)param.get();
	if (LOGIN_OK == pLoginParam->result)
	{
		APP_LOG(LOG_ERROR, TRUE, _T("ReloginManager regloin success!!!"));

		module::getSessionModule()->setTime(pLoginParam->serverTime);

		//通知服务器客户端初始化完毕,获取组织架构信息和群列表
		module::getLoginModule()->notifyLoginDone();

		//通知网络已经恢复正常，可以进行各种操作了
		logic::GetLogic()->asynNotifyObserver(module::KEY_LOGIN_RELOGINOK, pLoginParam->mySelectedStatus);
	}
	else
	{
		APP_LOG(LOG_ERROR, TRUE, _T("ReloginManager regloin failed!!!"));
		logic::getTcpClientModule()->closeSocket();
		logic::getTcpClientModule()->shutdown();

		//TCP\IP验证token失效,开启重新获取token的task
		//if (LOGIN_TOKEN_FAILED == pLoginParam->result)
		{
			//开启定时获取token的定时器
		}
		if (LOGIN_VERSION_TOOOLD == pLoginParam->result)
		{
			CString csTip = util::getMultilingual()->getStringViaID(_T("STRID_WEBLOGINFORM_TIP_VERSION_TOOOLD"));
			CString csTitle = module::getMiscModule()->getAppTitle();
			::MessageBox(0, csTip, csTitle, MB_OK | MB_ICONINFORMATION);
			module::getMiscModule()->quitTheApplication();
		}
		else
		{
			startReloginTimer(++m_secondCount);
		}
	}

	DoLoginServer::getInstance()->shutdown();
}
