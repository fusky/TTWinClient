/******************************************************************************* 
 *  @file      LoginWnd.cpp 2014\7\31 12:23:51 $
 *  @author    大佛<dafo@mogujie.com>
 *  @brief     
 ******************************************************************************/

#include "stdafx.h"
#include "LoginDialog.h"
#include "GlobalDefine.h"
#include "LoginOperation.h"
#include "GetServConfigHttpOperation.h"
#include "Modules/ISysConfigModule.h"
#include "Modules/ILoginModule.h"
#include "Modules/IMiscModule.h"
#include "Modules/ISessionModule.h"
#include "Modules/IMessageModule.h"
#include "Modules/IHttpPoolModule.h"
#include "DoLoginServer.h"
#include "utility/Multilingual.h"
#include "utility/utilCommonAPI.h"
#include "utility/utilStrCodeAPI.h"
#include "TTLogic/ITcpClientModule.h"

DUI_BEGIN_MESSAGE_MAP(LoginDialog, WindowImplBase)
	DUI_ON_MSGTYPE(DUI_MSGTYPE_WINDOWINIT, OnWindowInitialized)
	DUI_ON_MSGTYPE(DUI_MSGTYPE_CLICK,OnClick)
	DUI_ON_MSGTYPE(DUI_MSGTYPE_TEXTCHANGED, OnTextChanged)
DUI_END_MESSAGE_MAP()

/******************************************************************************/

// -----------------------------------------------------------------------------
//  LoginWnd: Public, Constructor

LoginDialog::LoginDialog()
:m_ptxtTip(0)
,m_pedtUserName(0)
,m_pedtPassword(0)
,m_pBtnSysSetting(0)
,m_pChkRememberPWD(0)
,m_bPassChanged(FALSE)
{

}

// -----------------------------------------------------------------------------
//  LoginWnd: Public, Destructor

LoginDialog::~LoginDialog()
{
}

LPCTSTR LoginDialog::GetWindowClassName() const
{
	return _T("LoginDialog");
}

DuiLib::CDuiString LoginDialog::GetSkinFile()
{
	return  _T("LoginDialog\\LoginDialog.xml");
}

DuiLib::CDuiString LoginDialog::GetSkinFolder()
{
	return _T("");
}

CControlUI* LoginDialog::CreateControl(LPCTSTR pstrClass)
{
	return nullptr;
}
void LoginDialog::OnFinalMessage(HWND hWnd)
{
	DoLoginServer::getInstance()->shutdown();
	DoLoginServer::releaseInstance();
	
	__super::OnFinalMessage(hWnd);

	delete this;
}

LRESULT LoginDialog::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (WM_NCLBUTTONDBLCLK == uMsg)//禁用双击标题栏最大化
	{
		return 0;
	}
	return WindowImplBase::HandleMessage(uMsg, wParam, lParam);;
}

void LoginDialog::OnWindowInitialized(TNotifyUI& msg)
{
	m_ptxtTip = (CTextUI*)m_PaintManager.FindControl(_T("tipText"));
	PTR_VOID(m_ptxtTip);
	m_pedtUserName = (CEditUI*)m_PaintManager.FindControl(_T("edit_name"));
	PTR_VOID(m_pedtUserName);
	m_pedtPassword = (CEditUI*)m_PaintManager.FindControl(_T("edit_pwd"));
	PTR_VOID(m_pedtPassword);
	m_pBtnLogin = (CButtonUI*)m_PaintManager.FindControl(_T("loginBtn"));
	PTR_VOID(m_pBtnLogin);
	m_pBtnSysSetting = (CButtonUI*)m_PaintManager.FindControl(_T("setting"));
	PTR_VOID(m_pBtnSysSetting);
	m_pChkRememberPWD = (CCheckBoxUI*)m_PaintManager.FindControl(_T("isRememberPWD"));
	PTR_VOID(m_pChkRememberPWD);

	_Initlize();

	m_pBtnLogin->SetFocus();
}

void LoginDialog::OnTextChanged(TNotifyUI& msg)
{
	PTR_VOID(msg.pSender);
	if (msg.pSender == m_pedtPassword)
	{
		m_bPassChanged = TRUE;
	}
}

void LoginDialog::OnClick(TNotifyUI& msg)
{
	PTR_VOID(msg.pSender);
	if (msg.pSender == m_pBtnLogin)
	{
		_DoLogin();
		return;
	}
	else if (msg.pSender == m_pBtnSysSetting)
	{
		module::getSysConfigModule()->showServerConfigDialog(m_PaintManager.GetPaintWindow());
		return;
	}
	__super::OnClick(msg);
}

void LoginDialog::_DoLogin()
{
	APP_LOG(LOG_DEBUG, TRUE, _T("LoginDialog DoLogin now"));

	m_ptxtTip->SetText(_T(""));
	CDuiString userName = m_pedtUserName->GetText();
	CDuiString password = m_pedtPassword->GetText();
	if (userName.IsEmpty())
	{
		CString csTip = util::getMultilingual()->getStringViaID(_T("STRID_LOGINDIALOG_USERNAME_EMPTY"));
		m_ptxtTip->SetText(csTip);
		return;
	}
	if (password.IsEmpty())
	{
		CString csTip = util::getMultilingual()->getStringViaID(_T("STRID_LOGINDIALOG_PASSWORD_EMPTY"));
		m_ptxtTip->SetText(csTip);
		return;
	}
	module::TTConfig* pCfg = module::getSysConfigModule()->getSystemConfig();
	pCfg->userName = userName;
	if (m_bPassChanged)
	{
		std::string sPass = util::cStringToString(CString(password));
		pCfg->password = util::cStringToString(util::makeMD5Value((void*)sPass.c_str(), sPass.size()));
	}

	pCfg->isRememberPWD = m_pChkRememberPWD->GetCheck();
	module::getSysConfigModule()->saveData();

	CString csTxt = util::getMultilingual()->getStringViaID(_T("STRID_LOGINDIALOG_BTN_DOLOGIN"));
	m_pBtnLogin->SetText(csTxt);
	m_pBtnLogin->SetEnabled(false);

	//连接PHP服务器，获取地址
	GetServConfigParam param;
	GetServConfigHttpOperation* pOper = new GetServConfigHttpOperation(
		fastdelegate::MakeDelegate(this, &LoginDialog::OnHttpCallbackOperation),param);
	module::getHttpPoolModule()->pushHttpOperation(pOper);

}
void LoginDialog::OnHttpCallbackOperation(std::shared_ptr<void> param)
{
	GetServConfigParam* pParam = (GetServConfigParam*)param.get();
	if (GETSERV_CONFIT_SUCC == pParam->result)
	{
		//登陆登陆服务器
		if (!DoLoginServer::getInstance()->createLink())
		{
			APP_LOG(LOG_ERROR, _T("Create login link service error"));
			return;
		}
		if (!logic::getTcpClientModule()->create())
		{
			APP_LOG(LOG_ERROR, _T("Create logic link service error"));
			return;
		}
		module::TTConfig* pCfg = module::getSysConfigModule()->getSystemConfig();
		PTR_VOID(pCfg);
		LoginParam loginparam;
		loginparam.csUserName = pCfg->userName;
		loginparam.password = pCfg->password;
		loginparam.csUserName.Trim();
		LoginOperation* pOperation = new LoginOperation(
			fastdelegate::MakeDelegate(this, &LoginDialog::OnOperationCallback), loginparam);
		logic::GetLogic()->pushBackOperation(pOperation);
	}
	else
	{
		CString csTip = util::getMultilingual()->getStringViaID(_T("STRID_LOGINDIALOG_GETSERVER_CONFIG_FAIL"));
		m_ptxtTip->SetText(csTip);
		module::TTConfig* pCfg = module::getSysConfigModule()->getSystemConfig();
		APP_LOG(LOG_ERROR, _T("LoginDialog get server config faild,php addres:%s"), util::stringToCString(pCfg->phpServerAddr));

		CString csTxt = util::getMultilingual()->getStringViaID(_T("STRID_LOGINDIALOG_BTN_LOGIN"));
		m_pBtnLogin->SetText(csTxt);
		m_pBtnLogin->SetEnabled(true);
	}
}
void LoginDialog::OnOperationCallback(std::shared_ptr<void> param)
{
	DoLoginServer::getInstance()->shutdown();
	LoginParam* pLoginParam = (LoginParam*)param.get();
	if (LOGIN_OK == pLoginParam->result)	//登陆成功
	{
		Close(IDOK);

		//创建用户目录
		_CreateUsersFolder();

		//开启同步消息时间timer
		module::getSessionModule()->startSyncTimeTimer();
		module::getSessionModule()->setTime(pLoginParam->serverTime);

		//通知服务器客户端初始化完毕,获取组织架构信息和群列表
		module::getLoginModule()->notifyLoginDone();

		//打开历史记录数据库
		module::getMessageModule()->openDB();
	}
	else	//登陆失败处理
	{
		logic::getTcpClientModule()->closeSocket();
		logic::getTcpClientModule()->shutdown();

		if (LOGIN_VERSION_TOOOLD == pLoginParam->result)
		{			
			CString csTip = util::getMultilingual()->getStringViaID(_T("STRID_LOGINDIALOG_LOGIN_TOOOLD"));
			m_ptxtTip->SetText(csTip);
			APP_LOG(LOG_ERROR, _T("uname:%s,LoginDialog login for loging server faild！"), pLoginParam->csUserName);
		}
		else if (LOGIN_LOGINSVR_FAIL == pLoginParam->result)
		{
			CString csTip = util::getMultilingual()->getStringViaID(_T("STRID_LOGINDIALOG_LOGIN_LOGINSVR_FAIL"));
			m_ptxtTip->SetText(csTip);
			APP_LOG(LOG_ERROR, _T("uname:%s,LoginDialog login for loging server faild！"), pLoginParam->csUserName);
		}
		else if (LOGIN_MSGSVR_FAIL == pLoginParam->result)
		{
			CString csTip = util::getMultilingual()->getStringViaID(_T("STRID_LOGINDIALOG_LOGIN_MSGSVR_FAIL"));
			m_ptxtTip->SetText(csTip);
			APP_LOG(LOG_ERROR, _T("uname:%s,LoginDialog login msg server faild！"), pLoginParam->csUserName);
		}
		else
		{
			CString csTip = util::getMultilingual()->getStringViaID(_T("STRID_LOGINDIALOG_LOGIN_FAIL"));
			m_ptxtTip->SetText(csTip);
			APP_LOG(LOG_ERROR, _T("uname:%s,LoginDialog login msg server faild！"), pLoginParam->csUserName);
		}
	}

	CString csTxt = util::getMultilingual()->getStringViaID(_T("STRID_LOGINDIALOG_BTN_LOGIN"));
	m_pBtnLogin->SetText(csTxt);
	m_pBtnLogin->SetEnabled(true);
}

void LoginDialog::_Initlize()
{
	module::TTConfig* pCfg = module::getSysConfigModule()->getSystemConfig();
	if (!pCfg->userName.IsEmpty())
		m_pedtUserName->SetText(pCfg->userName);


	if (pCfg->isRememberPWD)
	{
		m_pChkRememberPWD->SetCheck(true);
		if (!pCfg->password.empty())
			m_pedtPassword->SetText(_T("*********"));
	}
	else
		m_pChkRememberPWD->SetCheck(false);
}

BOOL LoginDialog::_CreateUsersFolder()
{
	module::IMiscModule* pModule = module::getMiscModule();

	//账号目录
	if (!util::createAllDirectories(pModule->getCurAccountDir()))
	{
		APP_LOG(LOG_ERROR, TRUE, _T("_CreateUsersFolder current Account direcotry failed!"));
		return FALSE;
	}

	//清理临时目录数据
	util::emptyFolder(pModule->getUserTempDir());
	//当前用户的临时文件目录
	if (!util::createAllDirectories(pModule->getUserTempDir()))
	{
		APP_LOG(LOG_ERROR, TRUE, _T("_CreateUsersFolder users temp direcotry failed!"));
		return FALSE;
	}

	return TRUE;
}

LRESULT LoginDialog::ResponseDefaultKeyEvent(WPARAM wParam)
{
	if (wParam == VK_RETURN)
	{
		_DoLogin();	
	}
	return FALSE;
}



/******************************************************************************/