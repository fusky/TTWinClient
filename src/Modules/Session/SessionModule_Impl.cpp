/******************************************************************************* 
 *  @file      SessionModule_Impl.cpp 2014\7\27 10:11:02 $
 *  @author    快刀<kuaidao@mogujie.com>
 *  @brief   
 ******************************************************************************/

#include "stdafx.h"
#include "SessionModule_Impl.h"
#include "Operation/SyncTimerTimer.h"
#include "src/base/ImPduClient.h"
#include "src/base/ImPduGroup.h"
#include "TTLogic/ITcpClientModule.h"
#include "Modules/UI/MainListLayout.h"
#include "Modules/UI/UIRecentSessionList.h"
#include "Modules/UI/UIGroupsTreelist.h"
#include "Modules/UI/UIEAUserTreelist.h"
#include "Modules/IUserListModule.h"
#include "Modules/IMessageModule.h"
#include "Modules/IGroupListModule.h"
#include "Modules/ISysConfigModule.h"
#include "Modules/UI//SearchLayout.h"
#include "Modules/MessageEntity.h"
#include "../Message/SendMsgManage.h"
#include "../Message/ReceiveMsgManage.h"
#include "Session/SessionManager.h"
/******************************************************************************/
namespace module
{
	module::ISessionModule* getSessionModule()
	{
		return (module::ISessionModule*)logic::GetLogic()->getModule(MODULE_ID_SEESION);
	}
}

// -----------------------------------------------------------------------------
//  SessionModule_Impl: Public, Constructor

SessionModule_Impl::SessionModule_Impl()
:m_pSyncTimer(0)
{

}

// -----------------------------------------------------------------------------
//  SessionModule_Impl: Public, Destructor

SessionModule_Impl::~SessionModule_Impl()
{

}

void SessionModule_Impl::release()
{
	delete this;
}

void SessionModule_Impl::onPacket(std::auto_ptr<CImPdu> pdu)
{
	CImPdu* pPdu = pdu.get();
	PTR_VOID(pPdu);
	switch (pdu->GetCommandId())
	{
	case CID_MSG_DATA:
		_sessionMsgResponse(pPdu);
		break;
	case CID_MSG_DATA_ACK:
		_sessionMsgACK(pPdu);
		break;
	case CID_MSG_TIME_RESPONSE:
		_sessionMsgTimeResponse(pPdu);
		break;
	case CID_MSG_UNREAD_CNT_RESPONSE:
		_sessionMsgUnreadCntResponse(pPdu);
		break;
	case CID_MSG_UNREAD_MSG_RESPONSE:
		_sessionMsgUnreadMsgResponse(pPdu);
		break;
	default:
		return;
	}
}

DuiLib::CControlUI* SessionModule_Impl::createMainDialogControl(
	LPCTSTR pstrClass, DuiLib::CPaintManagerUI& paintManager)
{
	if (_tcsicmp(pstrClass, _T("ListLayout")) == 0)
	{
		return new MainListLayout();
	}
	if (0 == _tcsicmp(pstrClass, _T("SearchLayout")))
	{
		return new SearchLayout();
	}
	if (_tcsicmp(pstrClass, _T("FriendList")) == 0)
	{
		return new CEAUserTreelistUI(paintManager);
	}
	else if (_tcsicmp(pstrClass, _T("GroupList")) == 0)
	{
		return new CGroupsTreelistUI(paintManager);
	}
	else if (_tcsicmp(pstrClass, _T("SessionList")) == 0)
	{
		return new CUIRecentSessionList(paintManager);
	}
	return nullptr;
}

void SessionModule_Impl::OnGroupUnreadMsgRespone(IN CImPdu* pdu)
{
	CImPduClientGroupMsgListResponse* pPduMsg = (CImPduClientGroupMsgListResponse*)pdu;
	client_group_msg_t* pMsgList = pPduMsg->GetMsgList();
	MessageEntity msg;
	msg.msgStatusType = MESSAGE_TYPE_OFFLINE;
	BOOL bNotify = TRUE;
	for (UInt32 i = pPduMsg->GetMsgCnt(); i > 0; --i)
	{
		client_group_msg_t pduMsg = pMsgList[i - 1];
		msg.content = string((char*)pduMsg.msg_content, pduMsg.msg_len);
		msg.msgTime = pduMsg.create_time;
		msg.talkerSid = string(pduMsg.from_user_id_url, pduMsg.from_user_id_len);
		msg.msgRenderType = MESSAGE_RENDERTYPE_TEXT;
		msg.msgFromType = MESSAGETYPE_FROM_GROUP;
		msg.sessionId = string(pPduMsg->GetGroupIdUrl(), pPduMsg->GetGroupIdLen());
		msg.makeGroupSessionId();
		if (i == pPduMsg->GetMsgCnt())
		{
			//第一次先清空存在客户端的未读消息，因为服务端又会将这个消息送过来，避免重复
			module::getMessageModule()->removeMessageBySId(msg.sessionId);
			bNotify = _checkMsgFromStranger(msg);
		}
		module::getGroupListModule()->DeleteGroupIDFromVecOfflineMsgGroup(msg.sessionId);

		if (!_banGroupMSG(msg))
		{
			module::getMessageModule()->pushMessageBySId(msg.sessionId, msg);
		}
	}
	if (bNotify)
		logic::GetLogic()->asynNotifyObserver(module::KEY_SESSION_NEWMESSAGE, msg.sessionId);
}

BOOL SessionModule_Impl::_banGroupMSG(IN MessageEntity msg)
{
	SessionEntity* psessionInfo = SessionEntityManager::getInstance()->getSessionEntityBySId(msg.sessionId);
	if (psessionInfo &&psessionInfo->m_bBanGroupMsg)
	{
		if (msg.msgRenderType != MESSAGE_RENDERTYPE_SYSTEMTIPS)
		{
			//发已读确认，通知服务器消息已读
			logic::GetLogic()->pushBackOperationWithLambda(
				[=]()
			{
				MessageEntity sendingmsgTemp = msg;
				std::string OriginSessionId = sendingmsgTemp.getOriginSessionId();
				CImPduClientGroupMsgReadAck pduMsgData(OriginSessionId.c_str());
				logic::getTcpClientModule()->sendPacket(&pduMsgData);
			}
			);

			return TRUE;
		}
	}
	return FALSE;
}

void SessionModule_Impl::_sessionMsgResponse(CImPdu* pdu)
{
	CImPduClientMsgData* pPduMsgData = (CImPduClientMsgData*)pdu;
	MessageEntity msg;
	msg.msgTime = pPduMsgData->GetCreateTime();
	msg.talkerSid = string(pPduMsgData->GetFromIdUrl(), pPduMsgData->GetFromIdLen());
	msg.msgStatusType = MESSAGE_TYPE_RUNTIME;
	msg.msgType = pPduMsgData->GetMsgType();
	if (MSG_TYPE_AUDIO_P2P == msg.msgType)
	{
		msg.msgRenderType = MESSAGE_RENDERTYPE_AUDIO;
		AudioMessageMananger::getInstance()->getAudioMsgLenth(pPduMsgData->GetMsgData(), pPduMsgData->GetMsgLen(), msg.msgAudioTime);
		AudioMessageMananger::getInstance()->makeAppAudioSid(msg.msgTime, msg.talkerSid, msg.content);
		AudioMessageMananger::getInstance()->saveAudioDataToFile(pPduMsgData->GetMsgData(), pPduMsgData->GetMsgLen(), msg.content);

		//语音消息为个人消息
		msg.msgFromType = MESSAGETYPE_FROM_FRIEND;
		if (string(pPduMsgData->GetFromIdUrl(), pPduMsgData->GetFromIdLen()) == module::getSysConfigModule()->userID())
			msg.sessionId = string(pPduMsgData->GetToIdUrl(), pPduMsgData->GetToIdLen());
		else
			msg.sessionId = string(pPduMsgData->GetFromIdUrl(), pPduMsgData->GetFromIdLen());
	}
	else if (MSG_TYPE_TEXT_P2P == msg.msgType)//个人消息
	{
		msg.msgFromType = MESSAGETYPE_FROM_FRIEND;
		if (string(pPduMsgData->GetFromIdUrl(), pPduMsgData->GetFromIdLen()) == module::getSysConfigModule()->userID())
			msg.sessionId = string(pPduMsgData->GetToIdUrl(), pPduMsgData->GetToIdLen());
		else
			msg.sessionId = string(pPduMsgData->GetFromIdUrl(), pPduMsgData->GetFromIdLen());

		msg.msgRenderType = MESSAGE_RENDERTYPE_TEXT;
		msg.content = string((char*)pPduMsgData->GetMsgData(), pPduMsgData->GetMsgLen());
	}
	else if (MSG_TYPE_TEXT_GROUP == msg.msgType)//群消息
	{
		//群功能屏蔽
		//return;
		msg.msgFromType = MESSAGETYPE_FROM_GROUP;//由于临时群增加了type，目前当初固定群处理
		msg.sessionId = string(pPduMsgData->GetToIdUrl(), pPduMsgData->GetToIdLen());//发送者的ID
		msg.makeGroupSessionId();
		msg.msgRenderType = MESSAGE_RENDERTYPE_TEXT;
		msg.content = string((char*)pPduMsgData->GetMsgData(), pPduMsgData->GetMsgLen());
	}
	else
	{
		APP_LOG(LOG_ERROR, _T("SessionModule_Impl::_sessionMsgResponse,CID_SESSION_MSG,消息类型不可知！！！"));
		return;
	}

	if (ReceiveMsgManage::getInstance()->checkIsReduplicatedMsg(msg, pPduMsgData->GetSeqNo()))
	{
		APP_LOG(LOG_ERROR, _T("SessionModule_Impl::_sessionMsgResponse,checkIsReduplicatedMsg,消息重复"));
		return;
	}

	if (ReceiveMsgManage::getInstance()->pushMessageBySId(msg.sessionId, msg))
	{
		if (_checkMsgFromStranger(msg))
		{
			if (_banGroupMSG(msg))
			{
				MessageEntity Popmsg;
				ReceiveMsgManage::getInstance()->popMessageBySId(msg.sessionId, Popmsg);
			}
			else
				logic::GetLogic()->asynNotifyObserver(module::KEY_SESSION_NEWMESSAGE, msg.sessionId);
		}

		//通知服务器收到该消息
		if (MESSAGETYPE_FROM_FRIEND == msg.msgFromType)//只有个人消息要发已读确认,群消息是不发已读确认的
		{
			logic::GetLogic()->pushBackOperationWithLambda(
				[=]()
			{
				MessageEntity sendingmsgTemp = msg;
				std::string OriginSessionId = sendingmsgTemp.getOriginSessionId();
				CImPduClientMsgDataAck pduMsgData(pPduMsgData->GetSeqNo()
					, OriginSessionId.length(), OriginSessionId.c_str());
				logic::getTcpClientModule()->sendPacket(&pduMsgData);
			});
		}
	}
	else
	{
		CString csUId = util::stringToCString(msg.talkerSid);
		APP_LOG(LOG_ERROR, _T("pushMessageBySId failed...,UId：%s"), csUId);
	}

}

void SessionModule_Impl::_sessionMsgACK(CImPdu* pdu)
{
	CImPduClientMsgDataAck* pPduMsgDataAck = (CImPduClientMsgDataAck*)pdu;
	SendMsgManage::getInstance()->popUpSendingMsgByAck(pPduMsgDataAck->GetSeqNo());
}

BOOL SessionModule_Impl::_checkMsgFromStranger(IN MessageEntity msg)
{
	if (MESSAGETYPE_FROM_FRIEND == msg.msgFromType)
	{
		module::UserInfoEntity info;
		if (!module::getUserListModule()->getUserInfoBySId(msg.sessionId, info))
		{
			//这个时候需要去请求下用户的好友信息
			CString csUId = util::stringToCString(msg.sessionId);
			APP_LOG(LOG_DEBUG, _T("SessionModule_Impl::_checkMsgFromStranger userinfo not exist,tcpGetUserFriendInfo now sId：%s"), csUId);
			module::getUserListModule()->tcpGetUserInfo(msg.sessionId);
			return FALSE;
		}
	}
	else if (MESSAGETYPE_FROM_GROUP == msg.msgFromType)
	{
		module::GroupInfoEntity info;
		if (!module::getGroupListModule()->getGroupInfoBySId(msg.sessionId, info))
		{
			//这个时候需要去请求下群信息
			CString csUId = util::stringToCString(msg.sessionId);
			APP_LOG(LOG_DEBUG, _T("SessionModule_Impl::_checkMsgFromStranger groupinfo not exist,tcpGetGroupInfo now sId：%s"), csUId);
			module::getGroupListModule()->tcpGetGroupInfo(msg.getOriginSessionId());
			return FALSE;
		}
	}
	return TRUE;
}

void SessionModule_Impl::_sessionMsgTimeResponse(CImPdu* pdu)
{
	CImPduClientTimeResponse* pduServerTime = (CImPduClientTimeResponse*)pdu;
	m_pSyncTimer->setTime(pduServerTime->GetServerTime());
}

void SessionModule_Impl::_sessionMsgUnreadCntResponse(CImPdu* pdu)
{
	CImPduClientUnreadMsgCntResponse* pduOfflineMsgCnt = (CImPduClientUnreadMsgCntResponse*)pdu;
	unread_info_t* pList = pduOfflineMsgCnt->GetUnreadList();
	for (UInt32 i = 0; i < pduOfflineMsgCnt->GetUnreadCnt(); ++i)
	{
		std::string sId(pList[i].id_url, pList[i].id_len);
		//直接开启获取具体离线消息的task
		logic::GetLogic()->pushBackOperationWithLambda(
			[=]()
		{
			CImPduClientUnreadMsgRequest pduMsgData(sId.c_str());
			logic::getTcpClientModule()->sendPacket(&pduMsgData);
		}
		);
	}
}

void SessionModule_Impl::_sessionMsgUnreadMsgResponse(CImPdu* pdu)
{
	CImPduClientMsgListResponse* pPduMsg = (CImPduClientMsgListResponse*)pdu;
	client_msg_t* pMsgList = pPduMsg->GetMsgList();
	MessageEntity msg;
	msg.msgStatusType = MESSAGE_TYPE_OFFLINE;
	BOOL bNotify = TRUE;
	UInt32 msgCnt = pPduMsg->GetMsgCnt();
	for (UInt32 i = msgCnt; i > 0; --i)
	{
		client_msg_t pduMsg = pMsgList[i - 1];
		msg.msgTime = pduMsg.create_time;
		msg.msgFromType = MESSAGETYPE_FROM_FRIEND;
		msg.talkerSid = string(pduMsg.from_id_url, pduMsg.from_id_len);
		msg.sessionId = msg.talkerSid;//个人消息中的sessionid就是说话的人的ID
		msg.msgType = pduMsg.msg_type;
		if (MSG_TYPE_AUDIO_P2P == msg.msgType)
		{
			msg.msgRenderType = MESSAGE_RENDERTYPE_AUDIO;
			AudioMessageMananger::getInstance()->getAudioMsgLenth(pduMsg.msg_content, pduMsg.msg_len, msg.msgAudioTime);
			AudioMessageMananger::getInstance()->makeAppAudioSid(msg.msgTime, msg.talkerSid, msg.content);
			AudioMessageMananger::getInstance()->saveAudioDataToFile(pduMsg.msg_content, pduMsg.msg_len, msg.content);
		}
		else
		{
			msg.msgRenderType = MESSAGE_RENDERTYPE_TEXT;
			msg.content = string((char*)pduMsg.msg_content, pduMsg.msg_len);
		}

		if (i == pPduMsg->GetMsgCnt())
		{
			//第一次先清空存在客户端的未读消息，因为服务端又会将这个消息送过来，避免重复
			ReceiveMsgManage::getInstance()->removeMessageBySId(msg.talkerSid);
			//第一次插入消息记录前，先去确认下是否要创建用户列表
			bNotify = _checkMsgFromStranger(msg);
		}
		ReceiveMsgManage::getInstance()->pushMessageBySId(msg.talkerSid, msg);
	}
	//通知UI显示
	if (bNotify)
		logic::GetLogic()->asynNotifyObserver(module::KEY_SESSION_NEWMESSAGE, msg.talkerSid);
}

UInt32 SessionModule_Impl::getTime() const
{
	return m_pSyncTimer->getTime();
}

void SessionModule_Impl::setTime(UInt32 time)
{
	m_pSyncTimer->setTime(time);
}

void SessionModule_Impl::startSyncTimeTimer()
{
	if (!m_pSyncTimer)
	{
		m_pSyncTimer = new SyncTimeTimer();
		logic::GetLogic()->scheduleTimer(m_pSyncTimer, 1, TRUE);
	}
}
/******************************************************************************/