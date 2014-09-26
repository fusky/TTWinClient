/******************************************************************************* 
 *  @file      SessionManager.cpp 2014\8\11 16:58:51 $
 *  @author    快刀<kuaidao@mogujie.com>
 *  @brief     
 ******************************************************************************/

#include "stdafx.h"
#include "SessionManager.h"
#include "Modules/IUserListModule.h"
#include "Modules/IGroupListModule.h"
#include "Modules/UI/SessionDialog.h"
#include <algorithm>
/******************************************************************************/

// -----------------------------------------------------------------------------
//  SessionManager: Public, Constructor

SessionEntityManager::SessionEntityManager()
{

}

// -----------------------------------------------------------------------------
//  SessionManager: Public, Destructor

SessionEntityManager::~SessionEntityManager()
{
	_removeAllSessionEntity();
}

SessionEntityManager* SessionEntityManager::getInstance()
{
	static SessionEntityManager manager;
	return &manager;
}

SessionEntity* SessionEntityManager::createSessionEntity(const std::string& sId)
{
	if (sId.empty())
		return 0;
	auto iter = m_mapSessionEntity.find(sId);
	if (iter != m_mapSessionEntity.end())
		return iter->second;

	SessionEntity* pSessionEntity = new SessionEntity;
	pSessionEntity->m_sessionType = _getSessionType(sId);
	if (SESSION_GROUPTYPE == pSessionEntity->m_sessionType)
	{
		pSessionEntity->m_bBanGroupMsg = _getBanGroupMSGSetting(sId);
	}
	pSessionEntity->m_sId = sId;
	pSessionEntity->SetUpdatedTimeByServerTime();
	{
		util::TTAutoLock lock(&m_lock);
		m_mapSessionEntity[sId] = pSessionEntity;
	}

	return pSessionEntity;
}

SessionEntity* SessionEntityManager::getSessionEntityBySId(IN const std::string& sId)
{
	util::TTAutoLock lock(&m_lock);
	auto iter = m_mapSessionEntity.find(sId);
	if (iter != m_mapSessionEntity.end())
	{
		return iter->second;
	}
	return nullptr;
}

BOOL SessionEntityManager::removeSessionEntity(const std::string& sId)
{
	util::TTAutoLock lock(&m_lock);
	auto iterMap = m_mapSessionEntity.find(sId);
	if (iterMap == m_mapSessionEntity.end())
		return FALSE;
	SessionEntity* pSessionEntity = iterMap->second;
	delete  pSessionEntity;
	pSessionEntity = 0;
	m_mapSessionEntity.erase(iterMap);

	return TRUE;
}

void SessionEntityManager::_removeAllSessionEntity()
{
	util::TTAutoLock lock(&m_lock);
	auto iter = m_mapSessionEntity.begin();
	for (; iter != m_mapSessionEntity.end(); ++iter)
	{
		delete iter->second;
		iter->second = 0;
	}
	m_mapSessionEntity.clear();
}

UInt8 SessionEntityManager::_getSessionType(IN const std::string& sID)
{
	if (sID.empty())
	{
		return SESSION_ERRORTYPE;
	}
	else if (std::string::npos == sID.find("group_"))
	{
		return SESSION_USERTYPE;
	}
	else
	{
		return SESSION_GROUPTYPE;
	}
}

BOOL SessionEntityManager::_getBanGroupMSGSetting(IN const std::string& sId)
{
	CString szUserDir = module::getMiscModule()->getCurAccountDir() + _T("BanGroupMsg.ini");
	return ::GetPrivateProfileInt(_T("BanGroupMSG"), util::stringToCString(sId), 0, szUserDir);
}

BOOL SessionEntityManager::saveBanGroupMSGSetting(IN const std::string& sId, IN BOOL bBanMsg)
{
	CString szUserDir = module::getMiscModule()->getCurAccountDir() + _T("BanGroupMsg.ini");
	if (bBanMsg)
	{
		::WritePrivateProfileString(_T("BanGroupMSG"), util::stringToCString(sId), util::int32ToCString(bBanMsg), szUserDir);
	}
	else
	{
		::WritePrivateProfileString(_T("BanGroupMSG"), util::stringToCString(sId), NULL, szUserDir);
	}
	return TRUE;
}

/******************************************************************************/

std::string SessionEntity::getAvatarPath()
{
	if (SESSION_USERTYPE == m_sessionType)
	{
		module::UserInfoEntity user;
		if (module::getUserListModule()->getUserInfoBySId(m_sId, user))
			return user.getAvatarPath();

	}
	else if (SESSION_GROUPTYPE == m_sessionType)
	{
		module::GroupInfoEntity group;
		if (module::getGroupListModule()->getGroupInfoBySId(m_sId, group))
		{
			return group.getAvatarPath();
		}
	}
	return std::string("");
}

UInt8 SessionEntity::getOnlineState()
{
	if (SESSION_USERTYPE == m_sessionType)
	{
		module::UserInfoEntity user;
		if (module::getUserListModule()->getUserInfoBySId(m_sId, user))
			return user.onlineState;

	}
	else if (SESSION_GROUPTYPE == m_sessionType)
	{
		return USER_STATUS_ONLINE;//群永远在线的
	}

	return 0;
}

CString SessionEntity::getName()
{
	if (SESSION_USERTYPE == m_sessionType)
	{
		module::UserInfoEntity user;
		if (module::getUserListModule()->getUserInfoBySId(m_sId, user))
		{
			return user.getRealName();
		}
	}
	else if (SESSION_GROUPTYPE == m_sessionType)
	{
		module::GroupInfoEntity group;
		if (module::getGroupListModule()->getGroupInfoBySId(m_sId, group))
		{
			return group.csName;
		}

	}
	return _T("");
}

SessionEntity::SessionEntity()
:m_unReadMsgCount(0)
,m_updatedTime(0)
,m_sessionType(SESSION_ERRORTYPE)
,m_bBanGroupMsg(FALSE)
{

}

void SessionEntity::SetUpdatedTimeByServerTime()
{
	if (SESSION_USERTYPE == m_sessionType)
	{
		module::UserInfoEntity userInfo;
		if (module::getUserListModule()->getUserInfoBySId(m_sId, userInfo))
		{
			m_updatedTime = userInfo.updated;
		}
	}
	else if (SESSION_GROUPTYPE == m_sessionType)
	{
		module::GroupInfoEntity groupInfo;
		if (module::getGroupListModule()->getGroupInfoBySId(m_sId, groupInfo))
		{
			m_updatedTime = groupInfo.groupUpdated;
		}
	}
	return;
}

SessionDialog* SessionDialogManager::openSessionDialog(const std::string& sId)
{
	if (sId.empty())
		return 0;
	auto iterDialog = std::find_if(m_lstSessionDialog.begin(),m_lstSessionDialog.end(),
		[=](SessionDialog* pDialog)
	{
		return (sId == pDialog->m_sId);
	});

	SessionDialog* pChatDialog = 0;
	if (iterDialog == m_lstSessionDialog.end())
	{
		pChatDialog = new SessionDialog(sId);
		if (pChatDialog == NULL)
			return 0;
		SessionEntity* pSessionInfo = SessionEntityManager::getInstance()->createSessionEntity(sId);
		pChatDialog->Create(NULL, pSessionInfo->getName(), UI_WNDSTYLE_FRAME | WS_POPUP, NULL, 0, 0, 0, 0);
		pChatDialog->CenterWindow();
		m_lstSessionDialog.push_back(pChatDialog);
	}
	else
	{
		pChatDialog = *iterDialog;
	}
	pChatDialog->BringToTop();
	//pChatDialog->CenterWindow();
	return pChatDialog;
}

void SessionDialogManager::closeSessionDialog(const std::string& sId)
{
	auto iterDialog = std::remove_if(m_lstSessionDialog.begin(), m_lstSessionDialog.end(),
		[=](SessionDialog* pDialog)
	{
		return (sId == pDialog->m_sId);
	}
	);
	if (iterDialog != m_lstSessionDialog.end())
	{
		m_lstSessionDialog.erase(iterDialog, m_lstSessionDialog.end());
	}
}

SessionDialogManager* SessionDialogManager::getInstance()
{
	static SessionDialogManager inst;
	return &inst;
}

SessionDialogManager::SessionDialogManager()
{

}

SessionDialog* SessionDialogManager::findSessionDialogBySId(const std::string& sId)
{
	if (sId.empty())
		return nullptr;
	auto iterDialog = std::find_if(m_lstSessionDialog.begin(), m_lstSessionDialog.end(),
		[=](SessionDialog* pDialog)
	{
		return (sId == pDialog->m_sId);
	});

	SessionDialog* pChatDialog = 0;
	if (iterDialog == m_lstSessionDialog.end())
	{
		//不创建
		return nullptr;
	}
	else
	{
		pChatDialog = *iterDialog;
	}
	return pChatDialog;
}
