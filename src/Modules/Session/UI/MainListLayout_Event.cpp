/******************************************************************************* 
 *  @file      MainListLayout_Event.cpp 2014\8\11 14:16:22 $
 *  @author    快刀<kuaidao@mogujie.com>
 *  @brief     
 ******************************************************************************/

#include "stdafx.h"
#include "Modules/IUserListModule.h"
#include "Modules/IGroupListModule.h"
#include "Modules/ISessionModule.h"
#include "Modules/ISysConfigModule.h"
#include "Modules/IFileTransferModule.h"
#include "Modules/UI/MainListLayout.h"
#include "Modules/UI/UIEAUserTreelist.h"
#include "Modules/UI/UIGroupsTreelist.h"
#include "Modules/UI/UIRecentSessionList.h"
#include "Modules/UI/SessionDialog.h"
#include "Modules/UI/UserDetailInfoDialog.h"
#include "utility/Multilingual.h"
#include "utility/utilStrCodeAPI.h"
#include "TTLogic/ITcpClientModule.h"
#include "../SessionManager.h"
#include "../../Message/ReceiveMsgManage.h"
#include "../../Message/SendMsgManage.h"
#include "../../FileTransfer/TransferManager.h"
#include "Modules/MessageEntity.h"
#include "src/base/ImPduGroup.h"
#include "src/base/ImPduFile.h"
/******************************************************************************/


void MainListLayout::OnUserlistModuleEvent(UInt16 moduleId, UInt32 keyId, MKO_TUPLE_PARAM mkoParam)
{
	if (module::KEY_USERLIST_UPDATE_DEPARTMENTLIST == keyId)
	{
		_LoadAllDepartment();
	}
	else if (module::KEY_USERLIST_UPDATE_RECENTLISTLIST == keyId)
	{
		//给所有的用户创建会话信息
		module::UserInfoEntityMap mapUserInfos;
		module::getUserListModule()->getAllUsersInfo(mapUserInfos);
		for (auto kv: mapUserInfos)
		{
			module::UserInfoEntity& user = kv.second;
			SessionEntityManager::getInstance()->createSessionEntity(user.sId);
		}
		_AddRecentUserListToUI();

		//获取个人会话离线消息
		logic::GetLogic()->pushBackOperationWithLambda(
			[]()
		{
			CImPduClientUnreadMsgCntRequest pduFriendMsgCount;//获取个人会话离线消息
			logic::getTcpClientModule()->sendPacket(&pduFriendMsgCount);
		});

		//获取离线文件
		logic::GetLogic()->pushBackOperationWithLambda(
			[]()
		{
			CImPduClientFileHasOfflineReq pduOfflineFileReq;//获取个人会话离线文件
			logic::getTcpClientModule()->sendPacket(&pduOfflineFileReq);
		});
	}
	else if (module::KEY_USERLIST_UPDATE_NEWUSESADDED == keyId)
	{
		//TODO:新的用户，或者一堆用户更新
	}
	//else if (module::KEY_USERLIST_USERLINESTATE == keyId)
	//{
	//	m_EAuserTreelist->sortByOnlineState();
	//}
}

void MainListLayout::OnGrouplistModuleEvent(UInt16 moduleId, UInt32 keyId, MKO_TUPLE_PARAM mkoParam)
{
	if (module::KEY_GROUPLIST_UPDATE_GROUPLIST == keyId)
	{
		_AddGroupList();
	}
	else if (module::KEY_GROUPLIST_UPDATE_GROUPDISCUSSLIST == keyId)
	{
		_AddDiscussGroupList();
	}
	else if (module::KEY_GROUPLIST_UPDATE_RECENTGROUPLIST == keyId)
	{
		//给所有的群/讨论组创建会话元信息
		module::GroupInfoMap mapGroupInfo;
		module::getGroupListModule()->getAllGroupListInfo(mapGroupInfo);
		for (auto kv : mapGroupInfo)
		{
			module::GroupInfoEntity& groupInfo = kv.second;
			SessionEntityManager::getInstance()->createSessionEntity(groupInfo.gId);
		}

		_AddRecentGroupListToUI();

		//获取群会话离线消息
		logic::GetLogic()->pushBackOperationWithLambda(
			[]()
		{
			CImPduClientGroupUnreadMsgCntRequest pduGroupMsgCount;//获取群会话离线消息
			logic::getTcpClientModule()->sendPacket(&pduGroupMsgCount);
		});
	}
	else if (module::KEY_GROUPLIST_UPDATE_NEWGROUPADDED == keyId)
	{		
		_NewGroupAdded(std::get<MKO_STRING>(mkoParam));
	}
	else if (module::KEY_GROUPLIST_UPDATE_CREATNEWGROUP == keyId)//新讨论组创建
	{
		_CreatNewDiscussGroupRes(std::get<MKO_STRING>(mkoParam));
	}
}
void MainListLayout::OnSessionModuleEvent(UInt16 moduleId, UInt32 keyId, MKO_TUPLE_PARAM mkoParam)
{
	if (module::KEY_SESSION_NEWMESSAGE == keyId)
	{
		_NewMsgUpdate(std::get<MKO_STRING>(mkoParam));
	}
	else if (module::KEY_SESSION_OPENNEWSESSION == keyId)
	{
		std::string sId = std::get<MKO_STRING>(mkoParam);
		if (module::getSysConfigModule()->userID() != sId)
		{
			SessionEntityManager::getInstance()->createSessionEntity(sId);
			SessionDialogManager::getInstance()->openSessionDialog(sId);
			m_UIRecentConnectedList->ClearItemMsgCount(sId);//清除显示的未读计数
			m_GroupList->ClearItemMsgCount(sId);
			m_EAuserTreelist->ClearItemMsgCount(sId);
		}
		else
		{
			APP_LOG(LOG_DEBUG, _T("MainListLayout::OnSessionModuleEvent-click myself!"));
		}
	}
	else if (module::KEY_SESSION_SENDMSG_TOOFAST == keyId)
	{
		std::string& sId = std::get<MKO_STRING>(mkoParam);
		SessionDialog* pSessionDialog = SessionDialogManager::getInstance()->findSessionDialogBySId(sId);
		if (pSessionDialog)
		{
			MessageEntity msg;
			msg.content = util::cStringToString(util::getMultilingual()->getStringViaID(_T("STRID_SESSIONMODULE_SENDMSG_TOOFAST")));
			msg.sessionId = sId;
			msg.talkerSid = module::getSysConfigModule()->userID(); 
			msg.msgRenderType = MESSAGE_RENDERTYPE_SYSTEMTIPS;
			ReceiveMsgManage::getInstance()->pushMessageBySId(sId, msg);
			logic::GetLogic()->asynNotifyObserver(module::KEY_SESSION_NEWMESSAGE, msg.sessionId);	//发送消息太快
		}
	}
	else if (module::KEY_SESSION_SENDMSG_FAILED == keyId)
	{
		SendingMsgList FailedMsgList;
		SendMsgManage::getInstance()->getSendFailedMsgs(FailedMsgList);
		CString csErrorTipFormat = util::getMultilingual()->getStringViaID(_T("STRID_SESSIONMODULE_SENDMSG_FAIL"));
		for (SendingMsg failedmsg:FailedMsgList)
		{
			CString csContent = util::stringToCString(failedmsg.msg.content);
			CString csErrorTip;
			MessageEntity msg;
			csErrorTip.Format(csErrorTipFormat, csContent);
			msg.content = util::cStringToString(csErrorTip);
			msg.sessionId = failedmsg.msg.sessionId;
			msg.talkerSid = module::getSysConfigModule()->userID();
			msg.msgRenderType = MESSAGE_RENDERTYPE_SYSTEMTIPS;
			ReceiveMsgManage::getInstance()->pushMessageBySId(msg.sessionId, msg);
			logic::GetLogic()->asynNotifyObserver(module::KEY_SESSION_NEWMESSAGE, msg.sessionId);	//发送消息失败
		}
	}
	else if (module::TAG_SESSION_TRAY_NEWMSGSEND == keyId)
	{
		std::string& sId = std::get<MKO_STRING>(mkoParam);
		if (!m_UIRecentConnectedList->IsExistSId(sId))
			m_UIRecentConnectedList->AddNode(sId);
		m_UIRecentConnectedList->UpdateItemBySId(sId);
		m_GroupList->UpdateItemBySId(sId);
		m_EAuserTreelist->UpdateItemBySId(sId);
		m_UIRecentConnectedList->sort();
	}
	else if (module::TAG_SESSION_TRAY_COPYDATA == keyId)
	{
		int	nParam = std::get<MKO_INT>(mkoParam);
		if (0 == nParam)    //表示结束语音播放
		{
			AudioMessageMananger::getInstance()->autoplayNextUnReadAudioMsg();
		}
		else if (2 == nParam)
		{
			AudioMessageMananger::getInstance()->stopPlayingAnimate();
		}
	}
}
void MainListLayout::OnFileTransferModuleEvent(UInt16 moduleId, UInt32 keyId, MKO_TUPLE_PARAM mkoParam)
{
	std::string& sFileId = std::get<MKO_STRING>(mkoParam);
	TransferFileEntity FileInfo;
	if (!TransferFileEntityManager::getInstance()->getFileInfoByTaskId(sFileId, FileInfo))
	{
		APP_LOG(LOG_ERROR, _T("MainListLayout::OnFileTransferModuleEvent:can't get the fileInfo:%s"),util::stringToCString(sFileId));
		return;
	}

	if (module::KEY_FILETRANSFER_REQUEST == keyId)
	{
		module::UserInfoEntity userInfo;
		if (!module::getUserListModule()->getUserInfoBySId(FileInfo.sFromID, userInfo))
		{
			APP_LOG(LOG_ERROR, _T("MainListLayout::OnFileTransferModuleEvent:can't get the userInfo:%s"),util::stringToCString(FileInfo.sFromID));
			return;
		}
		MessageEntity msg;
		CString csTipFormat = util::getMultilingual()->getStringViaID(_T("STRID_FILETRANSFERDIALOG_SEND_TIP"));
		CString csContent;
		csContent.Format(csTipFormat, userInfo.getRealName(), FileInfo.getRealFileName());
		msg.content = util::cStringToString(csContent);
		msg.sessionId = FileInfo.sFromID;
		msg.talkerSid = module::getSysConfigModule()->userID();
		msg.msgRenderType = MESSAGE_RENDERTYPE_SYSTEMTIPS;
		ReceiveMsgManage::getInstance()->pushMessageBySId(FileInfo.sFromID, msg);
		logic::GetLogic()->asynNotifyObserver(module::KEY_SESSION_NEWMESSAGE, msg.sessionId);	//给你发送了文件
	}
}
void MainListLayout::OnSysConfigModuleEvent(UInt16 moduleId, UInt32 keyId, MKO_TUPLE_PARAM mkoParam)
{
	if (module::KEY_SYSCONFIG_UPDATED == keyId)
	{
		module::TTConfig* pTTConfig = module::getSysConfigModule()->getSystemConfig();
		if (pTTConfig)
		{
			return;
		}
		if (pTTConfig->sysBaseFlag & module::BASE_FLAG_TOPMOST)
			::SetWindowPos(m_pManager->GetPaintWindow(), HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
		else
			::SetWindowPos(m_pManager->GetPaintWindow(), HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	}
	else if (module::KEY_SYSCONFIG_SHOW_USERDETAILDIALOG == keyId)
	{
		std::string& sId = std::get<MKO_STRING>(mkoParam);
		UserDetailInfoDialog* pFloatWnd = new UserDetailInfoDialog(sId);
		if (pFloatWnd == NULL) return;
		pFloatWnd->Create(m_pManager->GetPaintWindow(), _T("详细信息")
			,UI_CLASSSTYLE_DIALOG, WS_EX_STATICEDGE | WS_EX_APPWINDOW
			//, UI_WNDSTYLE_FRAME | WS_THICKFRAME, WS_EX_TOOLWINDOW | WS_EX_TOPMOST | WS_EX_STATICEDGE
			, 0, 0, 0, 0);
		pFloatWnd->CenterWindow();
		pFloatWnd->ShowWindow(true);
	}
}

void MainListLayout::_creatSessionDialog(IN UIIMList* pList, IN CControlUI* pMsgSender)
{
	if (nullptr == pList || pMsgSender == nullptr)
	{
		return;
	}
	if (-1 != pList->GetItemIndex(pMsgSender)
		&& 0 == _tcsicmp(pMsgSender->GetClass(), _T("ListContainerElementUI")))
	{
		Node* node = (Node*)pMsgSender->GetTag();
		if (!pList->CanExpand(node))
		{
			CString csId = node->data().sId;
			if (csId.IsEmpty())
				return;
			std::string sId = util::cStringToString(csId);
			SessionEntityManager::getInstance()->createSessionEntity(sId);
			SessionDialogManager::getInstance()->openSessionDialog(sId);

			m_UIRecentConnectedList->ClearItemMsgCount(sId);//清除显示的未读计数
			m_GroupList->ClearItemMsgCount(sId);
			m_EAuserTreelist->ClearItemMsgCount(sId);
			//停止托盘闪烁
			logic::GetLogic()->asynNotifyObserver(module::TAG_SESSION_TRAY_STOPEMOT);
		}
	}
}

void MainListLayout::Notify(TNotifyUI& msg)
{
	if (0 == _tcsicmp(msg.sType, DUI_MSGTYPE_ITEMCLICK))
	{
		if (_tcsicmp(msg.pSender->GetClass(), _T("ListContainerElementUI")) == 0)
		{
			if (m_Tab)
			{
				if (0 == m_Tab->GetCurSel() && m_EAuserTreelist)
				{
					Node* node = (Node*)msg.pSender->GetTag();

					if (m_EAuserTreelist->CanExpand(node))
					{
						m_EAuserTreelist->SetChildVisible(node, !node->data().child_visible_);
					}
				}
				else if (1 == m_Tab->GetCurSel() && m_GroupList)
				{
					Node* node = (Node*)msg.pSender->GetTag();

					if (m_GroupList->CanExpand(node))
					{
						m_GroupList->SetChildVisible(node, !node->data().child_visible_);
					}
				}

			}
		}
	}
	else if (0 == _tcsicmp(msg.sType, DUI_MSGTYPE_SELECTCHANGED))
	{
		if (_tcsicmp(msg.pSender->GetName(), _T("friendbtn")) == 0)
		{
			if (m_Tab && m_Tab->GetCurSel() != 0)
			{
				m_Tab->SelectItem(0);		
			}
		}
		else if (_tcsicmp(msg.pSender->GetName(), _T("groupbtn")) == 0)
		{
			if (m_Tab && m_Tab->GetCurSel() != 1)
			{
				m_Tab->SelectItem(1);
			}
		}
		else if (_tcsicmp(msg.pSender->GetName(), _T("RecentlyListbtn")) == 0)
		{
			if (m_Tab && m_Tab->GetCurSel() != 2)
			{
				m_Tab->SelectItem(2);
			}
		}
		if (m_UIRecentConnectedList)
		{
			m_UIRecentConnectedList->sort();
		}
	}
	else if (0 == _tcsicmp(msg.sType, DUI_MSGTYPE_ITEMACTIVATE))
	{
		if (m_Tab->GetCurSel() == 0)
		{
			_creatSessionDialog(m_EAuserTreelist, msg.pSender);
		}
		else if (1 == m_Tab->GetCurSel())
		{
			_creatSessionDialog(m_GroupList, msg.pSender);
		}
		else if (m_Tab->GetCurSel() == 2)
		{
			_creatSessionDialog(m_UIRecentConnectedList, msg.pSender);
		}
	}
	else if (0 == _tcsicmp(msg.sType, DUI_MSGTYPE_CLICK))
	{
		if (_tcsicmp(msg.pSender->GetName(), _T("searchbtn")) == 0)
		{

		}
	}
	else if (0 == _tcsicmp(msg.sType, DUI_MSGTYPE_MENU))
	{
		//CMenuWnd* pMenu = new CMenuWnd(NULL);
		//DuiLib::CPoint point = msg.ptMouse;
		//ClientToScreen(m_pManager->GetPaintWindow(), &point);
		//STRINGorID xml(_T("menu\\lineStatus.xml"));
		//pMenu->Init(reinterpret_cast<CMenuElementUI*>(this), xml, _T("xml"), point);
	}
}
void MainListLayout::DoEvent(TEventUI& event)
{
	if (event.Type == UIEVENT_TIMER)
	{
	}
}


/******************************************************************************/