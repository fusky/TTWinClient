/******************************************************************************* 
 *  @file      TestButton.cpp 2014\8\4 10:15:46 $
 *  @author    大佛<dafo@mogujie.com>
 *  @brief     
 ******************************************************************************/

#include "stdafx.h"
#include "Modules/UI/UIEAUserTreelist.h"
#include "Modules/UI/UIGroupsTreelist.h"
#include "Modules/UI/UIRecentSessionList.h"
#include "Modules/UI/MainListLayout.h"
#include "Modules/UI/SessionDialog.h"
#include "Modules/IUserListModule.h"
#include "Modules/IGroupListModule.h"
#include "Modules/ISysConfigModule.h"
#include "Modules/ISessionModule.h"
#include "Modules/IMiscModule.h"
#include "Modules/IFileTransferModule.h"
#include "TTLogic/ILogic.h"
#include "../SessionManager.h"
#include "../../Message/ReceiveMsgManage.h"
#include "../../Misc/FloatDialog.h"
#include "utility/Multilingual.h"
/******************************************************************************/

// -----------------------------------------------------------------------------
//  TestButton: Public, Constructor



 
 
 MainListLayout::MainListLayout()
: m_Tab(nullptr)
,m_EAuserTreelist(nullptr)
,m_GroupList(nullptr)
,m_UIRecentConnectedList(nullptr)
,m_groupRootParent(nullptr)
,m_DiscussGroupRootParent(nullptr)
{
	SetContextMenuUsed(true);
}

// -----------------------------------------------------------------------------
//  TestButton: Public, Destructor

MainListLayout::~MainListLayout()
{
	m_pManager->RemoveNotifier(this);
}

LPVOID MainListLayout::GetInterface(LPCTSTR pstrName)
{
	if (_tcscmp(pstrName, _T("ListLayout")) == 0) return static_cast<MainListLayout*>(this);
	return __super::GetInterface(pstrName);
}

LPCTSTR MainListLayout::GetClass() const
{
	return _T("ListLayout");
}

void MainListLayout::DoInit()
{
	m_pManager->AddNotifier(this);

	m_Tab = static_cast<CTabLayoutUI*>(m_pManager->FindSubControlByName(this, _T("tabs")));
	m_EAuserTreelist = static_cast<CEAUserTreelistUI*>(m_pManager->FindSubControlByName(this, _T("friends")));
	m_GroupList = static_cast<CGroupsTreelistUI*>(m_pManager->FindSubControlByName(this, _T("groupsList")));
	m_UIRecentConnectedList = static_cast<CUIRecentSessionList*>(m_pManager->FindSubControlByName(this, _T("recentlyList")));

	//默认选中最近联系人列表
	m_Tab->SelectItem(2);

	//MKO
	logic::GetLogic()->addObserver(this, MODULE_ID_USERLIST
		, fastdelegate::MakeDelegate(this, &MainListLayout::OnUserlistModuleEvent));

	logic::GetLogic()->addObserver(this, MODULE_ID_GROUPLIST
		, fastdelegate::MakeDelegate(this, &MainListLayout::OnGrouplistModuleEvent));

	logic::GetLogic()->addObserver(this, MODULE_ID_SEESION
		, fastdelegate::MakeDelegate(this, &MainListLayout::OnSessionModuleEvent));

	logic::GetLogic()->addObserver(this, MODULE_ID_SYSCONFIG
		, fastdelegate::MakeDelegate(this, &MainListLayout::OnSysConfigModuleEvent));

	logic::GetLogic()->addObserver(this, MODULE_ID_FILETRANSFER
		, fastdelegate::MakeDelegate(this, &MainListLayout::OnFileTransferModuleEvent));
}



void MainListLayout::_LoadAllDepartment()
{
	const module::DepartmentVec vecDeparments
		= module::getUserListModule()->getAllDepartments();
	for (module::DepartmentEntity depart : vecDeparments)
	{
		EAUserTreeListItemInfo item;
		item.id = util::stringToCString(depart.dId);
		item.folder = true;
		item.empty = false;
		item.nickName = depart.title;
		Node* root_parent = m_EAuserTreelist->GetItemBySId(depart.dId);
		if (!root_parent)
			root_parent = m_EAuserTreelist->AddNode(item, NULL);

		for (std::string uId : depart.members)
		{
			module::UserInfoEntity user;			
			if (!module::getUserListModule()->getUserInfoBySId(uId, user))
				continue;
			if (!m_EAuserTreelist->IsExistSId(uId))
			{
				item.id = util::stringToCString(uId);
				item.folder = false;
				item.avatarPath = util::stringToCString(user.getAvatarPath());
				item.nickName = user.getRealName();
				item.description = _T("description...");
				m_EAuserTreelist->AddNode(item, root_parent);
			}
		}
	}
}

void MainListLayout::_AddGroupList()
{
	module::GroupInfoMap Groups;
	module::getGroupListModule()->getAllGroupListInfo(Groups);

	GroupsListItemInfo item;
	Node* rootParent = nullptr;
	item.folder = true;
	item.empty = false;

	UInt32 GroupCnt = 0;
	for (auto GroupInfoData : Groups)
	{
		module::GroupInfoEntity& GroupInfo = GroupInfoData.second;
		if (1 == GroupInfo.type)
			GroupCnt++;
	}
	item.id = MY_GROUP_ITEMID;
	item.nickName.Format(util::getMultilingual()->getStringViaID(_T("STRID_SESSIONMODULE_MAINLISTLAYOUT_GROUP_TIP")), GroupCnt);
	rootParent = m_GroupList->GetItemBySId(util::cStringToString(CString(item.id)));
	if (!rootParent)
		rootParent =m_GroupList->AddNode(item, NULL);

	for (auto GroupInfoData : Groups)
	{
		module::GroupInfoEntity& GroupInfo = GroupInfoData.second;
		if (1 == GroupInfo.type)
		{
			if (!m_GroupList->IsExistSId(GroupInfo.gId))
			{
				item.id = util::stringToCString(GroupInfo.gId);
				item.folder = false;
				item.avatarPath = util::stringToCString(GroupInfo.getAvatarPath());
				item.nickName = GroupInfo.csName;
				item.description = GroupInfo.desc;
				m_GroupList->AddNode(item, rootParent);
			}
		}
	}
}

void MainListLayout::_AddDiscussGroupList()
{
	module::GroupInfoMap Groups;
	module::getGroupListModule()->getAllGroupListInfo(Groups);

	GroupsListItemInfo item;
	Node* rootParent = nullptr;
	item.folder = true;
	item.empty = false;
	UInt32 GroupCnt = 0;

	for (auto GroupInfoData : Groups)
	{
		module::GroupInfoEntity& GroupInfo = GroupInfoData.second;
		if (2 == GroupInfo.type)
			GroupCnt++;
	}
	item.id = MY_DISCUSSGROUP_ITEMID;
	item.nickName.Format(util::getMultilingual()->getStringViaID(_T("STRID_SESSIONMODULE_MAINLISTLAYOUT_DISCUSS_TIP")), GroupCnt);
	rootParent = m_GroupList->GetItemBySId(util::cStringToString(CString(item.id)));
	if (!rootParent)
		rootParent = m_GroupList->AddNode(item, NULL);
	
	for (auto GroupInfoData : Groups)
	{
		module::GroupInfoEntity& GroupInfo = GroupInfoData.second;
		if (2 == GroupInfo.type)
		{
			if (!m_GroupList->IsExistSId(GroupInfo.gId))
			{
				item.id = util::stringToCString(GroupInfo.gId);
				item.folder = false;
				item.avatarPath = util::stringToCString(GroupInfo.getAvatarPath());
				item.nickName.Format(_T("%s(%d)"), GroupInfo.csName, GroupInfo.groupMemeberList.size());
				item.description = GroupInfo.desc;
				m_GroupList->AddNode(item, rootParent);
			}
		}
	}
}

void MainListLayout::_AddRecentUserListToUI()
{
	module::UserInfoEntityVec VecUsers;
	module::getUserListModule()->getUserListInfoVec(VecUsers);
	for (std::string sid: VecUsers)
	{
		module::UserInfoEntity userInfo;
		if (!module::getUserListModule()->getUserInfoBySId(sid, userInfo))
			continue;

		if (!m_UIRecentConnectedList->IsExistSId(userInfo.sId))
		{
			SessionListItemInfo item;
			item.folder = false;
			item.empty = false;
			item.id = util::stringToCString(userInfo.sId);
			item.avatarPath = util::stringToCString(userInfo.getAvatarPath());
			item.nickName = userInfo.getRealName();
			item.description = _T("最近一条的信息为空");	
			item.Time = userInfo.updated;
			m_UIRecentConnectedList->AddNode(item, NULL);
		}
	}
	m_UIRecentConnectedList->sort();
}

void MainListLayout::_AddRecentGroupListToUI()
{
	module::GroupInfoMap Groups;
	module::getGroupListModule()->getAllGroupListInfo(Groups);
	for (auto GroupInfoData : Groups)
	{
		module::GroupInfoEntity& GroupInfo = GroupInfoData.second;

		if (!m_UIRecentConnectedList->IsExistSId(GroupInfo.gId))
		{
			SessionListItemInfo item;
			item.folder = false;
			item.empty = false;
			item.id = util::stringToCString(GroupInfo.gId);
			item.avatarPath = util::stringToCString(GroupInfo.getAvatarPath());
			item.nickName = GroupInfo.csName;
			item.description = _T("最近一条的信息");
			item.Time = GroupInfo.groupUpdated;
			m_UIRecentConnectedList->AddNode(item, NULL);
		}
	}
	m_UIRecentConnectedList->sort();
}

void MainListLayout::_NewMsgUpdate(std::string& sId)
{
	SessionEntity* psessionInfo = SessionEntityManager::getInstance()->getSessionEntityBySId(sId);
	if (!psessionInfo)
	{
		APP_LOG(LOG_ERROR, _T("MainListLayout::_NewMsgUpdate，can't find the SessionEntity"));
		return;
	}
	MessageEntity lastMsg;
	if (ReceiveMsgManage::getInstance()->frontMessageBySId(sId, lastMsg))
	{
		psessionInfo->m_updatedTime = lastMsg.msgTime;
	}
	
	if (lastMsg.msgRenderType != MESSAGE_RENDERTYPE_SYSTEMTIPS)
	{
		if (!m_UIRecentConnectedList->IsExistSId(sId))
			m_UIRecentConnectedList->AddNode(sId);
		m_UIRecentConnectedList->UpdateItemBySId(sId);
		m_GroupList->UpdateItemBySId(sId);
		m_EAuserTreelist->UpdateItemBySId(sId);
	}

	//会话窗口已经存在，则即时显示消息(离线消息返回的时候，肯定没有窗口)
	SessionDialog* pDialog = SessionDialogManager::getInstance()->findSessionDialogBySId(sId);
	if (pDialog)
	{
		pDialog->UpdateRunTimeMsg();
	}
	else
	{
		MessageEntity msg;
		ReceiveMsgManage::getInstance()->frontMessageBySId(sId, msg);
		if (msg.msgStatusType == MESSAGE_TYPE_RUNTIME)
		{
			//托盘图标闪烁
			logic::GetLogic()->asynNotifyObserver(module::TAG_SESSION_TRAY_STARTEMOT);
			//飘窗提示
			module::TTConfig* pTTConfig = module::getSysConfigModule()->getSystemConfig();
			if (pTTConfig && pTTConfig->sysBaseFlag & module::BASE_FLAG_NOTIPWHENNEWMSG)
			{
				FloatInfo floatInfo;
				floatInfo.sId = sId;
				if (msg.msgFromType == MESSAGETYPE_FROM_FRIEND)
				{
					module::UserInfoEntity userInfo;
					module::getUserListModule()->getUserInfoBySId(sId, userInfo);
					floatInfo.csUserName = userInfo.getRealName();
					floatInfo.sAvatarPath = userInfo.getAvatarPath();
				}
				else if (MESSAGETYPE_FROM_GROUP == msg.msgFromType)
				{
					module::GroupInfoEntity groupInfo;
					module::getGroupListModule()->getGroupInfoBySId(sId, groupInfo);
					floatInfo.csUserName = groupInfo.csName;
					floatInfo.sAvatarPath = groupInfo.getAvatarPath();
				}

				if (MESSAGE_RENDERTYPE_AUDIO == msg.msgRenderType)
				{
					floatInfo.csMsgContent = util::getMultilingual()->getStringViaID(_T("STRID_SESSIONMODULE_RENDERTYPE_AUDIO"));
				}
				else
				{
					CString strContent = util::stringToCString(msg.content);
					floatInfo.csMsgContent = strContent;
					ReceiveMsgManage::getInstance()->parseContent(floatInfo.csMsgContent, TRUE, 400, FALSE);
				}
				module::getMiscModule()->floatForm(m_pManager->GetPaintWindow(), floatInfo);
			}
		}
	}
	//播放声音
	module::getMiscModule()->playSysConfigSound();
}

void MainListLayout::_NewGroupAdded(std::string& gId)
{
	//相当于收到了该群的消息
	_NewMsgUpdate(gId);
}

void MainListLayout::_CreatNewDiscussGroupRes(std::string& sId)
{
	module::GroupInfoEntity GroupInfo;
	if (module::getGroupListModule()->getGroupInfoBySId(sId, GroupInfo))
	{
		SessionEntityManager::getInstance()->createSessionEntity(GroupInfo.gId);//创建会话管理，显示的为系统时间

		SessionListItemInfo item;
		item.folder = false;
		item.empty = false;
		item.id = util::stringToCString(GroupInfo.gId);
		item.avatarPath = util::stringToCString(GroupInfo.getAvatarPath());
		item.nickName = GroupInfo.csName;
		item.Time = GroupInfo.groupUpdated;
		m_UIRecentConnectedList->AddNode(item, NULL);
		m_UIRecentConnectedList->sort();

		GroupsListItemInfo groupItem;
		groupItem.folder = false;
		groupItem.empty = false;
		groupItem.id = util::stringToCString(GroupInfo.gId);
		groupItem.avatarPath = util::stringToCString(GroupInfo.getAvatarPath());
		groupItem.nickName = GroupInfo.csName;

		Node* parent = m_GroupList->GetParentIdForAddNode(MY_DISCUSSGROUP_ITEMID);
		if (parent)
		{
			m_GroupList->AddNode(groupItem, parent);
		}
	}
}









/******************************************************************************/