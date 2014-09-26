/******************************************************************************* 
 *  @file      LoginOperation.cpp 2014\7\30 15:32:28 $
 *  @author    快刀<kuaidao@mogujie.com>
 *  @brief     
 ******************************************************************************/

#include "stdafx.h"
#include "LoginOperation.h"
#include "DoLoginServer.h"
#include "utility/utilStrCodeAPI.h"
#include "Modules/ISysConfigModule.h"
#include "Modules/IUserListModule.h"

/******************************************************************************/

// -----------------------------------------------------------------------------
//  LoginOperation: Public, Constructor

LoginOperation::LoginOperation(logic::ICallbackHandler& callback, LoginParam& param)
:ICallbackOpertaion(callback)
,m_loginParam(param)
{

}

// -----------------------------------------------------------------------------
//  LoginOperation: Public, Destructor

LoginOperation::~LoginOperation()
{

}

void LoginOperation::process()
{
	APP_LOG(LOG_INFO, 1, _T("uname:%s,LoginOperation begin,status:%d"), m_loginParam.csUserName
		, m_loginParam.mySelectedStatus);

	LoginParam* pParam = new LoginParam;
	pParam->csUserName = m_loginParam.csUserName;
	pParam->mySelectedStatus = m_loginParam.mySelectedStatus;

	//连接登录服务器
	CImPduMsgServResponse* pduMsgServResp = (CImPduMsgServResponse*)DoLoginServer::getInstance()->doLogin();
	if (0 == pduMsgServResp || pduMsgServResp->GetResult() != 0)
	{
		APP_LOG(LOG_ERROR, 1, _T("uname:%s,LoginOperation login for loging server failed"), m_loginParam.csUserName);
		if (pduMsgServResp)
			APP_LOG(LOG_ERROR, 1, _T("error code :%d"), pduMsgServResp->GetResult());
		if (pduMsgServResp && RESUSE_REASON_VERSION_TOO_OLD == pduMsgServResp->GetResult())
		{
			//提示用户去下载新版本，本协议太老了不兼容了
			pParam->result = LOGIN_VERSION_TOOOLD;
			APP_LOG(LOG_ERROR, 1, _T("uname:%s,LoginOperation login for loging server failed for version too old"),m_loginParam.csUserName);
		}
		pParam->result = LOGIN_LOGINSVR_FAIL;
		asyncCallback(std::shared_ptr<void>(pParam));
		return;
	}
	APP_LOG(LOG_ERROR, 1, _T("uname:%s,LoginOperation login for loging server done"), m_loginParam.csUserName);

	//连接消息服务器
	logic::ILogic* pLogic = logic::GetLogic();
	std::string msgServer = string(pduMsgServResp->GetIP2Addr(), pduMsgServResp->GetIP2Len());
	CString server = util::stringToCString(msgServer);
	CImPduLoginResponse* pduLoginResp = (CImPduLoginResponse*)logic::getTcpClientModule()
		->doLogin(server, pduMsgServResp->GetPort(),m_loginParam.csUserName,m_loginParam.password);
	if (0 == pduLoginResp || pduLoginResp->GetResult() != 0)
	{
		APP_LOG(LOG_ERROR, 1, _T("uname:%s,LoginOperation login for msg server failed"), m_loginParam.csUserName);
		if (pduLoginResp)
		{
			APP_LOG(LOG_ERROR, 1, _T("error code :%d"), pduLoginResp->GetResult());
			if (REFUSE_REASON_DB_VALIDATE_FAILED == pduLoginResp->GetResult())		//验证失败
			{
				APP_LOG(LOG_ERROR, 1, _T("uname:%s,LoginOperation login failed for DB invalid")
					, m_loginParam.csUserName);
				pParam->result = LOGIN_DB_INVALID;
			}
		}
		else
			pParam->result = LOGIN_MSGSVR_FAIL;
		asyncCallback(std::shared_ptr<void>(pParam));
		return;
	}
	pParam->result = LOGIN_OK;
	pParam->serverTime = pduLoginResp->GetServerTime();
	pParam->mySelectedStatus = pduLoginResp->GetOnlineStatus();

	//存储服务器端返回的userId
	module::TTConfig* pCfg = module::getSysConfigModule()->getSystemConfig();
	std::string sUserId(pduLoginResp->GetUserIdUrl(),pduLoginResp->GetUserIdUrllen());
	pCfg->userId = sUserId;
	pCfg->csUserId = util::stringToCString(sUserId);
	std::string sToken(pduLoginResp->GetToken(), pduLoginResp->GetTokenLen());
	pCfg->token = util::stringToCString(sToken);

	APP_LOG(LOG_INFO, TRUE, _T("LoginOperation login succeed! Name = %s Nickname = %s avatarUrl = %s sId = %s")
		, m_loginParam.csUserName
		, util::stringToCString(pduLoginResp->GetNickname(), CP_UTF8)
		, util::stringToCString(string(pduLoginResp->GetAvatarUrl(), pduLoginResp->GetAvatarLen()), CP_UTF8)
		, module::getSysConfigModule()->UserID());

	//登陆成功，创建自己的信息
	module::UserInfoEntity myInfo;
	myInfo.sId = sUserId;
	myInfo.csName = m_loginParam.csUserName;
	myInfo.onlineState = USER_STATUS_ONLINE;
	if (pduLoginResp->GetNickname())
	{
		std::string sTemp(pduLoginResp->GetNickname(), pduLoginResp->GetNicknameLen());
		myInfo.csNickName = util::stringToCString(sTemp);
	}
	myInfo.avatarUrl = string(pduLoginResp->GetAvatarUrl(), pduLoginResp->GetAvatarLen());
	myInfo.dId = std::string("department_") +string(pduLoginResp->GetDepartIdUrl(), pduLoginResp->GetDepartIdUrlLen());
	module::getUserListModule()->createUserInfo(sUserId,myInfo);

	asyncCallback(std::shared_ptr<void>(pParam));
	APP_LOG(LOG_INFO, 1, _T("uname:%s,LoginOperation login done,status:%d"), m_loginParam.csUserName
		, m_loginParam.mySelectedStatus);

	//开始发送心跳包
	logic::getTcpClientModule()->startHeartbeat();
}

void LoginOperation::release()
{
	delete this;
}

/******************************************************************************/