/******************************************************************************* 
 *  @file      IUserListModule_Impl.cpp 2014\8\6 15:28:35 $
 *  @author    大佛<dafo@mogujie.com>
 *  @brief     
 ******************************************************************************/

#include "stdafx.h"
#include "UserListModule_Impl.h"
#include "src/base/ImPduClient.h"
#include "utility/utilStrCodeAPI.h"
#include "TTLogic/ITcpClientModule.h"
#include "Modules/ISysConfigModule.h"
#include "Modules/IDatabaseModule.h"
#include "Modules/ICaptureModule.h"
#include "cxImage/cxImage/ximage.h"
#include "../Session/Operation/DownloadImgHttpOperation.h"
#include <algorithm>
/******************************************************************************/
namespace module
{
	module::IUserListModule* getUserListModule()
	{
		return (module::IUserListModule*)logic::GetLogic()->getModule(MODULE_ID_USERLIST);
	}
}

// -----------------------------------------------------------------------------
//  IUserListModule_Impl: Public, Constructor

UserListModule_Impl::UserListModule_Impl()
{

}

// -----------------------------------------------------------------------------
//  IUserListModule_Impl: Public, Destructor

UserListModule_Impl::~UserListModule_Impl()
{

}

void UserListModule_Impl::release()
{
	delete this;
}

void UserListModule_Impl::onPacket(std::auto_ptr<CImPdu> pdu)
{
	CImPdu* pPdu = pdu.get();
	PTR_VOID(pPdu);
	switch (pdu->GetCommandId())
	{
	case CID_BUDDY_LIST_DEPARTMENT_RESPONSE:
		_departmentResponse(pPdu);
		break;
	case CID_BUDDY_LIST_ALL_USER_RESPONSE:
		_allUserlistResponse(pPdu);
		break;
	case CID_BUDDY_LIST_FRIEND_LIST:
		_recentlistResponse(pPdu);
		break;
	case CID_BUDDY_LIST_USER_INFO_RESPONSE:
		_newUserListInfoResponse(pPdu);
		break;
	case CID_BUDDY_LIST_ONLINE_FRIEND_LIST:
		_allUserlistLineStatusResponse(pPdu);
		break;
	case  CID_BUDDY_LIST_STATUS_NOTIFY:
		_userLineStatusResponse(pPdu);
		break;
	default:
		APP_LOG(LOG_DEBUG, _T("UserListModule_Impl::onPacket-Unknow commandID:%d"), pdu->GetCommandId());
		return;
	}
}

void UserListModule_Impl::_departmentResponse(CImPdu* pdu)
{
	CImPduClientDepartmentResponse* pResp = (CImPduClientDepartmentResponse*)pdu;
	client_department_info_t* pList = pResp->GetDepartmentList();
	m_vecDepartment.reserve(pResp->GetDepartCnt());
	for (UInt32 i = 0; i < pResp->GetDepartCnt(); i++)
	{
		module::DepartmentEntity depart;
		depart.dId = std::string(pList[i].depart_id_url, pList[i].depart_id_url_len);
		depart.dId = _makeDepartmentId(depart.dId);
		depart.parentDepartId = std::string(pList[i].parent_depart_id_url, pList[i].parent_depart_id_url_len);
		std::string sTitle(pList[i].title, pList[i].title_len);
		depart.title = util::stringToCString(sTitle);
		std::string sDescription(pList[i].description, pList[i].description_len);
		depart.description = util::stringToCString(sDescription);
		depart.status = pList[i].status;
		{
			util::TTAutoLock lock(&m_lock);
			m_vecDepartment.push_back(depart);
		}
	}
}

void UserListModule_Impl::_allUserlistResponse(CImPdu* pdu)
{
	CImPduClientUserInfoResponse* pResp = (CImPduClientUserInfoResponse*)pdu;
	client_user_info_t* pUserInfos = pResp->GetUserInfoList();
	for (UInt32 i = 0; i < pResp->GetUserCnt(); ++i)
	{
		std::string sId(pUserInfos[i].id_url, pUserInfos[i].id_len);

		module::UserInfoEntity userInfo;
		userInfo.sId = sId;
		if (pUserInfos[i].name_len)
		{
			std::string sTemp(pUserInfos[i].name, pUserInfos[i].name_len);
			userInfo.csName = util::stringToCString(sTemp);
		}
		else
		{
			APP_LOG(LOG_DEBUG, _T("pUserInfos[i].name_len = 0"));
		}
		if (pUserInfos[i].nick_name_len)
		{
			std::string sTemp(pUserInfos[i].nick_name, pUserInfos[i].nick_name_len);
			userInfo.csNickName = util::stringToCString(sTemp);
		}
		else
		{
			APP_LOG(LOG_DEBUG,_T("pUserInfos[i].nick_name_len = 0"));
		}
		userInfo.avatarUrl = string(pUserInfos[i].avatar_url, pUserInfos[i].avatar_len);
		userInfo.dId = string(pUserInfos[i].depart_id_url, pUserInfos[i].depart_id_len);
		userInfo.dId = _makeDepartmentId(userInfo.dId);
		//对号入座部门信息
		{
			util::TTAutoLock lock(&m_lock);
			auto iterUserInfo = m_mapUsers.find(sId);
			if (iterUserInfo == m_mapUsers.end())
			{
				m_mapUsers[sId] = userInfo;
			}
			else
			{
				//不能覆盖时间
				iterUserInfo->second.csName = userInfo.csName;
				iterUserInfo->second.csNickName = userInfo.csNickName;
				iterUserInfo->second.avatarUrl = userInfo.avatarUrl;
				iterUserInfo->second.dId = userInfo.dId;
			}

		}
	}

	//延迟加载组织架构部门与公司成员的关系
	logic::GetLogic()->pushBackOperationWithLambda(
		[=]()
	{
		for (auto& kvp : m_mapUsers)
		{
			module::UserInfoEntity& userInfo = kvp.second;
			_pushUserIdToDepartment(userInfo.sId, userInfo.dId);
		}
		logic::GetLogic()->asynNotifyObserver(module::KEY_USERLIST_UPDATE_DEPARTMENTLIST);
	}
	);

	//下载所有用户的头像
	_downloadAllUserAvatarImg();

	_updateRecentUserList();
}

void UserListModule_Impl::_recentlistResponse(CImPdu* pdu)//最近联系人信息
{
	CImPduClientFriendList* pResp = (CImPduClientFriendList*)pdu;
	client_user_info_t* pUserInfos = pResp->GetFriendList();
	std::string myUserId = module::getSysConfigModule()->userID();

	for (UInt32 i = 0; i < pResp->GetFriendCnt(); ++i)
	{
		std::string sId(pUserInfos[i].id_url, pUserInfos[i].id_len);
		if (sId != myUserId)
		{
			m_vecUsers.push_back(sId);
		}
		{
			util::TTAutoLock lock(&m_lock);
			auto iterUserInfo = m_mapUsers.find(sId);
			if (iterUserInfo != m_mapUsers.end())
			{
				//已经在获取了该人员的信息，更新下时间
				iterUserInfo->second.updated = pUserInfos[i].user_updated;
			}
			else
			{
				module::UserInfoEntity userInfo;
				userInfo.sId = sId;
				userInfo.updated = pUserInfos[i].user_updated;
				{
					util::TTAutoLock lock(&m_lock);
					m_mapUsers[sId] = userInfo;
				}
			}
		}
	}

	_updateRecentUserList();
}
void UserListModule_Impl::_newUserListInfoResponse(CImPdu* pdu)
{
	CImPduClientUserInfoResponse* pUserInfoResp = (CImPduClientUserInfoResponse*)pdu;
	client_user_info_t* pUserInfoList = pUserInfoResp->GetUserInfoList();
	for (UInt32 i = 0; i < pUserInfoResp->GetUserCnt(); ++i)
	{
		client_user_info_t& info = pUserInfoList[i];
		std::string sId(info.id_url, info.id_len);
		module::UserInfoEntity userInfo;
		BOOL bAddNewUser = FALSE;
		if (!getUserInfoBySId(sId, userInfo))
			bAddNewUser = TRUE;
		if (info.name_len)
		{
			std::string sTemp(info.name, info.name_len);
			userInfo.csName = util::stringToCString(sTemp);
		}
		if (info.nick_name_len)
		{
			std::string sTemp(info.nick_name, info.nick_name_len);
			userInfo.csNickName = util::stringToCString(sTemp);
		}
		userInfo.avatarUrl = string(info.avatar_url, info.avatar_len);
		userInfo.sId = sId;
		userInfo.dId = string(info.depart_id_url, info.depart_id_len);
		if (bAddNewUser)
		{
			tcpGetUserOnlieStatus(sId);//请求该用户的在线状态
			//对号入座部门信息
			_pushUserIdToDepartment(sId, userInfo.dId);
			util::TTAutoLock lock(&m_lock);
			m_mapUsers[sId] = userInfo;
			logic::GetLogic()->asynNotifyObserver(module::KEY_USERLIST_UPDATE_NEWUSESADDED);
		}
	}
}

const module::DepartmentVec& UserListModule_Impl::getAllDepartments()
{
	util::TTAutoLock lock(&m_lock);
	return m_vecDepartment;
}
void UserListModule_Impl::getAllUsersInfo(module::UserInfoEntityMap& MapUsers)const
{
	MapUsers = m_mapUsers;
}
BOOL UserListModule_Impl::getUserInfoBySId(IN std::string sid, OUT module::UserInfoEntity& userInfo)
{
	util::TTAutoLock lock(&m_lock);
	//据说map在find之前需要判空
	if (m_mapUsers.empty())
		return FALSE;
	module::UserInfoEntityMap::iterator iter = m_mapUsers.find(sid);
	if (iter == m_mapUsers.end())
		return FALSE;

	userInfo = iter->second;
	return TRUE;
}

void UserListModule_Impl::getUserListInfoVec(OUT module::UserInfoEntityVec& VecUsers)
{
	VecUsers = m_vecUsers;
}

void UserListModule_Impl::releaseUserListInfoVec()
{
	m_vecUsers.clear();
}

void UserListModule_Impl::removeAllListInfo()
{
	m_mapUsers.clear();
}

UInt8 UserListModule_Impl::getMyLineStatus()
{
	module::UserInfoEntity myInfo;
	getMyInfo(myInfo);
	return myInfo.onlineState;
}

void UserListModule_Impl::tcpGetUserOnlieStatus(IN const std::string& sId)
{
	logic::GetLogic()->pushBackOperationWithLambda(
		[=]()
	{
		CImPduClientUserStatusRequest pduRequest(sId.c_str());
		logic::getTcpClientModule()->sendPacket(&pduRequest);
	}
	);
}

void UserListModule_Impl::tcpGetUserOnlieStatus(const module::UserInfoEntityVec& VecId)
{
	if (VecId.empty())
	{
		return;
	}

	const UInt32 nMax = 300;		//大佛：分批获取,一次性获取太多会失败
	int nTime = static_cast<int>(VecId.size() / nMax);
	if (nTime > 0)
	{
		std::vector<std::string> MainList = static_cast<std::vector<std::string>>(VecId);
		std::vector<std::string>::iterator it = MainList.begin();
		for (int n = 0; n < nTime; ++n)
		{
			std::vector<std::string> VecId(nMax);
			std::vector<std::string>::iterator itSend = VecId.begin();
			std::copy(it + n*nMax, it + (n + 1)*nMax, itSend);
			_tcpGetUserOnlieStatus(VecId);
		}
		UInt32 nLeft = VecId.size() % nMax;
		if (nLeft)
		{
			std::vector<std::string> VecId(nLeft);
			std::vector<std::string>::iterator itSend = VecId.begin();
			std::copy(it + nTime*nMax, it + nTime*nMax + nLeft, itSend);
			_tcpGetUserOnlieStatus(VecId);
		}
	}
	else
	{
		std::vector<std::string> VecId;
		VecId = VecId;
		_tcpGetUserOnlieStatus(VecId);
	}
}

void UserListModule_Impl::_tcpGetUserOnlieStatus(const module::UserInfoEntityVec& VecId)
{
	logic::GetLogic()->pushBackOperationWithLambda(
		[=]()
	{
		list<string> usersList;
		std::copy(VecId.begin(), VecId.end(), std::back_inserter(usersList));
		CImPduClientUsersStatusRequest pduRequest(usersList);
		logic::getTcpClientModule()->sendPacket(&pduRequest);
	}
	);
}

void UserListModule_Impl::tcpGetAllUserOnlieStatus(void)
{
	if (m_mapUsers.empty())
	{
		return;
	}
	module::UserInfoEntityVec allUserVec;
	for (auto& kvp : m_mapUsers)
	{
		allUserVec.push_back(kvp.second.sId);
	}
	tcpGetUserOnlieStatus(allUserVec);
}

void UserListModule_Impl::tcpGetUserInfo(IN const std::string& sId)
{
	if (sId.empty())
	{
		return;
	}
	module::UserInfoEntityVec VecUserInfo;
	VecUserInfo.push_back(sId);
	_tcpGetUserInfoList(VecUserInfo);
}

void UserListModule_Impl::tcpGetUserInfoList(IN const module::UserInfoEntityVec& VecUnKnowUserInfo)
{
	//一次最多取200，多了取不到了，亲。 ---- 大佛
	const UInt32 nMax = 200;
	m_tcpGetUserFriendInfoListBackTime = 0;
	m_tcpGetUserFriendInfoListTime = static_cast<int>(VecUnKnowUserInfo.size() / nMax);
	if (m_tcpGetUserFriendInfoListTime > 0)
	{
		std::vector<std::string> MainList = static_cast<std::vector<std::string>>(VecUnKnowUserInfo);
		std::vector<std::string>::iterator it = MainList.begin();
		for (UInt32 n = 0; n < m_tcpGetUserFriendInfoListTime; ++n)
		{
			std::vector<std::string> VecId(nMax);
			std::vector<std::string>::iterator itSend = VecId.begin();
			std::copy(it + n*nMax, it + (n + 1)*nMax, itSend);
			_tcpGetUserInfoList(VecId);
		}
		UInt32 nLeft = VecUnKnowUserInfo.size() % nMax;
		if (nLeft)
		{
			std::vector<std::string> VecId(nLeft);
			std::vector<std::string>::iterator itSend = VecId.begin();
			std::copy(it + m_tcpGetUserFriendInfoListTime*nMax, it + m_tcpGetUserFriendInfoListTime*nMax + nLeft, itSend);
			_tcpGetUserInfoList(VecId);
		}
	}
	else
	{
		std::vector<std::string> VecId;
		VecId = VecUnKnowUserInfo;
		_tcpGetUserInfoList(VecId);
	}
}

void UserListModule_Impl::_tcpGetUserInfoList(IN module::UserInfoEntityVec VecUnKnowUserInfo)
{
	if (VecUnKnowUserInfo.empty())
		return;

	logic::GetLogic()->pushBackOperationWithLambda(
		[=]()
	{
		client_id_t* pClientIdList = new client_id_t[VecUnKnowUserInfo.size()];
		int nCount = 0;
		module::UserInfoEntityVec::const_iterator it = VecUnKnowUserInfo.begin();
		for (int nCount = 0; it != VecUnKnowUserInfo.end(); ++it, ++nCount)
		{
			pClientIdList[nCount].id_url = (char*)it->c_str();
			pClientIdList[nCount].id_len = (UInt32)it->size();
		}

		CImPduClientUserInfoRequest pduUserInfo(VecUnKnowUserInfo.size(), pClientIdList);
		logic::getTcpClientModule()->sendPacket(&pduUserInfo);

		delete[] pClientIdList;
		pClientIdList = NULL;
	}
	);
}

CString UserListModule_Impl::getDefaultAvatartPath()
{
	return module::getMiscModule()->getDataDir() + _T("default.jpg");
}

BOOL UserListModule_Impl::getMyInfo(OUT module::UserInfoEntity& myInfo)
{
	return getUserInfoBySId(module::getSysConfigModule()->userID(), myInfo);
}

BOOL UserListModule_Impl::getDepartmentByDId(const std::string& dId, module::DepartmentEntity& entity)
{
	util::TTAutoLock lock(&m_lock);
	auto iterDepartment = std::find_if(m_vecDepartment.begin(),m_vecDepartment.end(),
		[=](module::DepartmentEntity entity)
	{
		return (dId == entity.dId);
	});

	if (iterDepartment != m_vecDepartment.end())
	{
		entity = *iterDepartment;
		return TRUE;
	}
	return FALSE;
}

BOOL UserListModule_Impl::_pushUserIdToDepartment(const string& sId, const string& dId)
{
	util::TTAutoLock lock(&m_lock);
	auto iterDepartment = std::find_if(m_vecDepartment.begin(), m_vecDepartment.end(),
		[=](module::DepartmentEntity entity)
	{
		return (dId == entity.dId);
	});
	if (iterDepartment != m_vecDepartment.end())
	{
		module::DepartmentEntity& entity = *iterDepartment;
		entity.members.push_back(sId);
		return TRUE;
	}

	return FALSE;
}

void UserListModule_Impl::getSearchUserNameListByShortName(IN const CString& sShortName, OUT module::UserInfoEntityVec& nameList)
{
	for (auto& kvp : m_mapUsers)
	{
		module::UserInfoEntity& userInfo = kvp.second;

		CString RealName = userInfo.getRealName();

		if (util::isIncludeChinese(util::cStringToString(sShortName, CP_ACP)))//检索中文
		{
			if (RealName.Find(sShortName) != -1)
			{
				nameList.push_back(userInfo.sId);
			}
		}
		else//检索字母
		{
			CString firstPY = util::HZ2FirstPY(util::cStringToString(RealName, CP_ACP));
			if (firstPY.Find(sShortName) != -1)//先检索简拼
			{
				nameList.push_back(userInfo.sId);
			}
			else
			{
				CString allPY = util::HZ2AllPY(RealName);//再检索全拼
				if (allPY.Find(sShortName) != -1)
				{
					nameList.push_back(userInfo.sId);
				}
			}
		}

	}
}

std::string UserListModule_Impl::_makeDepartmentId(IN const string& dId)
{
	return std::string("department_") + dId;
}

void UserListModule_Impl::_downloadAllUserAvatarImg()
{
	for (auto& kv : m_mapUsers)
	{
		module::UserInfoEntity& user = kv.second;
		if (!user.avatarUrl.empty() && user.avatarUrl.find("avatar_default.jpg") == std::string::npos)
		{
			UInt32 hashcode = util::hash_BKDR(user.avatarUrl.c_str());
			module::ImImageEntity imageEntity;
			module::getDatabaseModule()->sqlGetImImageEntityByHashcode(hashcode, imageEntity);
			CString csLocalPath = module::getMiscModule()->getDownloadDir() + util::stringToCString(imageEntity.filename);
			if (!imageEntity.filename.empty() && util::isFileExist(csLocalPath))
			{
				//本地磁盘存在
				user.avatarLocalPath = util::cStringToString(csLocalPath);
				user.avatarGrayLocalPath = _getGrayLocalPathFromFilename(imageEntity.filename);
			}
			else
			{
				//不存在则去服务器下载
				DownloadImgHttpOperation* pOper = new DownloadImgHttpOperation(user.sId,user.avatarUrl,TRUE
					, fastdelegate::MakeDelegate(this, &UserListModule_Impl::onCallbackOperation));
				module::getHttpPoolModule()->pushHttpOperation(pOper);
			}
		}
	}
}

std::string UserListModule_Impl::_getGrayLocalPathFromFilename(std::string& finename)
{
	CString csFileName = util::stringToCString(finename);
	CString csGrayAvatarPath = module::getMiscModule()->getDownloadDir() + PREFIX_GRAY_AVATAR + csFileName;
	//本地磁盘存在
	if (util::isFileExist(csGrayAvatarPath))
	{
		return util::cStringToString(csGrayAvatarPath);
	}
	else
	{
		//不存在则对图片做灰度处理并且保存到本地
		CxImage cximage;
		CString csAvatarPath = module::getMiscModule()->getDownloadDir() + csFileName;
		bool bSucc = cximage.Load(csAvatarPath);
		if (bSucc)
		{
			cximage.GrayScale();
			module::getCaptureModule()->saveToFile(cximage.MakeBitmap(), csAvatarPath);
		}
	}

	return "";
}

void UserListModule_Impl::onCallbackOperation(std::shared_ptr<void> param)
{
	DownloadImgParam* pDownParam = (DownloadImgParam*)param.get();
	if (DownloadImgParam::DOWNLOADIMG_OK == pDownParam->m_result)
	{
		module::UserInfoEntityMap::iterator userIter = m_mapUsers.find(pDownParam->m_sId);
		if (userIter != m_mapUsers.end())
		{
			CString csLocalPath = module::getMiscModule()->getDownloadDir() + util::stringToCString(pDownParam->m_imgEntity.filename);
			userIter->second.avatarLocalPath = util::cStringToString(csLocalPath);
			userIter->second.avatarGrayLocalPath = _getGrayLocalPathFromFilename(pDownParam->m_imgEntity.filename);
		}

		logic::GetLogic()->asynNotifyObserver(module::KEY_USERLIST_DOWNAVATAR_SUCC
			, pDownParam->m_sId);
	}
}

BOOL UserListModule_Impl::createUserInfo(const std::string& sId, const module::UserInfoEntity& info)
{
	util::TTAutoLock lock(&m_lock);
	module::UserInfoEntity infoTemp;
	if (getUserInfoBySId(sId, infoTemp))
		return FALSE;
	m_mapUsers[sId] = info;

	//下载头像
	_downloadAllUserAvatarImg();

	return TRUE;
}

void UserListModule_Impl::_allUserlistLineStatusResponse(CImPdu* pdu)
{
	util::TTAutoLock lock(&m_lock);
	CImPduClientOnlineFriendList* pList = (CImPduClientOnlineFriendList*)pdu;
	client_stat_t* pSidList = pList->GetFriendStatList();
	for (UInt32 i = 0; i < pList->GetFriendCnt(); ++i)
	{
		std::string sId(pSidList[i].id_url, pSidList[i].id_len);
		//更新在线状态
		module::UserInfoEntityMap::iterator iter = m_mapUsers.find(sId);
		if (iter != m_mapUsers.end())
		{
			iter->second.onlineState= pSidList[i].status;
		}
	}

	logic::GetLogic()->asynNotifyObserver(module::KEY_USERLIST_ALLUSERLINESTATE);
}

void UserListModule_Impl::_userLineStatusResponse(CImPdu* pdu)
{
	util::TTAutoLock lock(&m_lock);
	CImPduClientFriendNotify* plineStatusNotify = (CImPduClientFriendNotify*)pdu;
	std::string sId(plineStatusNotify->GetIdUrl(), plineStatusNotify->GetIdLen());
	module::UserInfoEntityMap::iterator iter = m_mapUsers.find(sId);
	if (iter == m_mapUsers.end())
		return;
	iter->second.onlineState = plineStatusNotify->GetStatus();
	logic::GetLogic()->asynNotifyObserver(module::KEY_USERLIST_USERLINESTATE,sId);
}

BOOL UserListModule_Impl::_updateRecentUserList(void)
{
	m_UserListGetTime++;
	if (2 == m_UserListGetTime)
	{
		m_UserListGetTime = 0;
		logic::GetLogic()->asynNotifyObserver(module::KEY_USERLIST_UPDATE_RECENTLISTLIST);
		return TRUE;
	}
	return FALSE;
}

/******************************************************************************/