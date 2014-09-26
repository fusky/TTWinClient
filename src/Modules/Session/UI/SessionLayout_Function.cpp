/******************************************************************************* 
 *  @file      SessionLayout_Function.cpp 2014\8\15 13:26:01 $
 *  @author    快刀<kuaidao@mogujie.com>
 *  @brief     
 ******************************************************************************/

#include "stdafx.h"
#include "Modules/MessageEntity.h"
#include "Modules/UI/SessionLayout.h"
#include "Modules/ISysConfigModule.h"
#include "Modules/ISessionModule.h"
#include "Modules/IUserListModule.h"
#include "Modules/IUserListModule.h"
#include "Modules/IMessageModule.h"
#include "utility/Multilingual.h"
#include "utility/utilStrCodeAPI.h"
#include "TTLogic/ITcpClientModule.h"
#include "../../Message/ReceiveMsgManage.h"
#include "../../Message/SendMsgManage.h"
#include "../Operation/SendImgHttpOperation.h"
#include "../SessionManager.h"
#include "json/reader.h"
#include "json/writer.h"
#include "UIIMEdit.h"
/******************************************************************************/
void SessionLayout::_SendSessionMsg(IN MixedMsg mixMsg)
{
	if (mixMsg.IsPureTextMsg())
	{
		MessageEntity msg;
		msg.content = util::cStringToString(mixMsg.m_strTextData);
		msg.sessionId = m_sId;
		msg.talkerSid = module::getSysConfigModule()->userID();
		msg.msgRenderType = MESSAGE_RENDERTYPE_TEXT;
		SessionEntity* pSessionInfo = SessionEntityManager::getInstance()->getSessionEntityBySId(m_sId);
		PTR_VOID(pSessionInfo);
		msg.msgType = (pSessionInfo->m_sessionType == SESSION_USERTYPE) ? MSG_TYPE_TEXT_P2P : MSG_TYPE_TEXT_GROUP;
		msg.msgFromType = pSessionInfo->m_sessionType;	
		msg.msgTime = module::getSessionModule()->getTime();
		SendMsgManage::getInstance()->pushSendingMsg(msg);

		//更新会话时间
		SessionEntity*  pSessionEntity = SessionEntityManager::getInstance()->getSessionEntityBySId(msg.sessionId);
		if (pSessionEntity)
		{
			pSessionEntity->m_updatedTime = msg.msgTime;
		}
		//主界面 消息内容，时间更新
		logic::GetLogic()->asynNotifyObserver(module::TAG_SESSION_TRAY_NEWMSGSEND, msg.sessionId);
	}
	else
	{
		for (ST_picData& picData : mixMsg.m_picDataVec)
		{
			//图片需要上传
			SendImgParam param;
			param.csFilePath = picData.strLocalPicPath;
			SendImgHttpOperation* pOper = new SendImgHttpOperation(param
				, fastdelegate::MakeDelegate(this, &SessionLayout::OnCallbackOperation));
			module::getHttpPoolModule()->pushHttpOperation(pOper,TRUE);
		}
		m_SendingMixedMSGList.push_back(mixMsg);
	}
}
void SessionLayout::SendMsg()
{
	MessageEntity msg;
	module::UserInfoEntity myInfo;
	module::getUserListModule()->getMyInfo(myInfo);
	UInt8 netState = logic::getTcpClientModule()->getTcpClientNetState();
	if (logic::TCPCLIENT_STATE_OK == netState && USER_STATUS_OFFLINE != myInfo.onlineState)
	{
		MixedMsg mixMsg;
		if (!m_pInputRichEdit->GetContent(mixMsg))
		{
			return;
		}
		//将消息投递给对方
		_SendSessionMsg(mixMsg);
		//本地消息展现
		msg.msgType = MSG_TYPE_TEXT_P2P;
		msg.talkerSid = module::getSysConfigModule()->userID();
		msg.sessionId = m_sId;
		msg.msgRenderType = MESSAGE_RENDERTYPE_TEXT;
		msg.msgStatusType = MESSAGE_TYPE_RUNTIME;
		msg.content = util::cStringToString(mixMsg.MakeMixedLocalMSG());
		msg.msgTime = module::getSessionModule()->getTime();
		_DisplayMsgToIE(msg, _T("sendMessage"));
	}
	else
	{
		//发送消息太快
		_DisplaySysMsg(_T("STRID_SESSIONMODULE_OFFLINE_SENDMSG_TIP"));
	}
	
}

void SessionLayout::_DisplaySysMsg(IN CString strID)
{
	MessageEntity msg;
	CString csTip = util::getMultilingual()->getStringViaID(strID);
	msg.content = util::cStringToString(csTip);
	msg.sessionId = m_sId;
	msg.talkerSid = module::getSysConfigModule()->userID();
	msg.msgRenderType = MESSAGE_RENDERTYPE_SYSTEMTIPS;
	ReceiveMsgManage::getInstance()->pushMessageBySId(msg.sessionId, msg);
	logic::GetLogic()->asynNotifyObserver(module::KEY_SESSION_NEWMESSAGE, msg.sessionId);	//收到屏幕抖动消息提示

}

BOOL SessionLayout::_DisplayMsgToIE(IN MessageEntity msg ,IN CString jsInterface)
{
	module::UserInfoEntity userInfo;
	if (!module::getUserListModule()->getUserInfoBySId(msg.talkerSid, userInfo))
	{
		return FALSE;
	}

	Json::Value root;
	root["name"] = util::cStringToString(userInfo.getRealName());
	root["avatar"] = userInfo.getAvatarPathWithoutOnlineState();
	root["msgtype"] = msg.msgRenderType;
	root["uuid"] = msg.talkerSid;
	root["mtype"] = msg.isMySendMsg() ? "me" : "other";
	CTime timeData(msg.msgTime);
	root["time"] = util::cStringToString(timeData.Format(_T("%Y-%m-%d %H:%M:%S")));

	//语音内容特殊处理
	if (MESSAGE_RENDERTYPE_AUDIO == msg.msgRenderType)
	{
		root["voiceid"] = msg.content;
		CString sVoicetime;
		sVoicetime.Format(_T("%d秒"), msg.msgAudioTime);
		root["voicetime"] = util::cStringToString(sVoicetime);
		root["voiceisread"] = msg.msgAudioReaded ? std::string("true") : string("false");
	}
	else
	{
		CString csContent = util::stringToCString(msg.content);
		ReceiveMsgManage::getInstance()->parseContent(csContent, FALSE, GetWidth(), !msg.isMySendMsg());
		root["content"] = util::cStringToString(csContent);
	}
	Json::StyledWriter styleWrite;
	std::string record = styleWrite.write(root);
	Json::Reader jsonRead;
	Json::Value rootRead;
	CString jsData = _T("[]");
	if (!jsonRead.parse(record, rootRead) || rootRead.isNull())
	{
		CString csError = util::stringToCString(record, CP_UTF8);
		APP_LOG(LOG_INFO, TRUE, _T("json parse error:%s"), csError);
		jsData = _T("[]");
		return FALSE;
	}
	else
		jsData = util::stringToCString(record, CP_UTF8);
	//调用页面的JS代码
	if (m_pWebBrowser)
	{
		VARIANT VarResult;
		m_pWebBrowser->CallJScript(jsInterface.GetBuffer(), jsData.GetBuffer(), &VarResult);
		jsData.ReleaseBuffer();
	}
	return TRUE;
}


void SessionLayout::_DisplayHistoryMsgToIE(IN UInt32 nMsgCount,BOOL scrollBottom)
{
	std::vector<MessageEntity> msgList;
	if (module::getMessageModule()->sqlGetHistoryMsg(m_sId, nMsgCount, msgList))
	{
		//给接收到的消息增加offset计数
		module::getMessageModule()->countMsgOffset(m_sId, msgList.size());
	}

	if (msgList.empty())
		return;

	Json::Value root;
	for (std::vector<MessageEntity>::reverse_iterator itMsg = msgList.rbegin();
		itMsg != msgList.rend(); ++itMsg)
	{
		module::UserInfoEntity userInfo;
		if (!module::getUserListModule()->getUserInfoBySId(itMsg->talkerSid, userInfo))
			continue;

		//组装json data
		Json::Value msgItem;
		msgItem["name"] = util::cStringToString(userInfo.getRealName());
		msgItem["avatar"] = userInfo.getAvatarPathWithoutOnlineState();
		msgItem["mtype"] = itMsg->isMySendMsg() ? "me" : "other";
		CTime time(itMsg->msgTime);
		msgItem["time"] = util::cStringToString(time.Format(_T("%Y-%m-%d %H:%M:%S")));
		msgItem["uuid"] = itMsg->talkerSid;
		msgItem["msgtype"] = itMsg->msgRenderType;

		if (MESSAGE_RENDERTYPE_AUDIO == itMsg->msgRenderType)
		{
			msgItem["voiceid"] = itMsg->content;
			CString sVoicetime;
			sVoicetime.Format(_T("%d秒"), itMsg->msgAudioTime);
			msgItem["voicetime"] = util::cStringToString(sVoicetime, CP_UTF8);
			msgItem["voiceisread"] = itMsg->isReaded() ? std::string("true") : string("false");
		}
		else
		{
			CString& csContent = util::stringToCString(itMsg->content);
			ReceiveMsgManage::getInstance()->parseContent(csContent, FALSE, GetWidth(),TRUE);
			std::string content = util::cStringToString(csContent, CP_UTF8);
			msgItem["content"] = content;
		}

		root.append(msgItem);
	}

	Json::StyledWriter styleWrite;
	std::string record = styleWrite.write(root);
	CString jsData = _T("[]");
	Json::Reader jsonRead;
	Json::Value rootRead;
	if (!jsonRead.parse(record, rootRead) || rootRead.isNull())
	{
		CString csError = util::stringToCString(record, CP_UTF8);
		APP_LOG(LOG_ERROR, _T("history is null or json parse error:%s"), csError);
		jsData = _T("[]");
	}
	else
	{
		jsData = util::stringToCString(record);
	}

	//调用js
	CComVariant result;
	BOOL bRet = m_pWebBrowser->CallJScript(_T("historyMessage"), jsData.GetBuffer(), &result);
	if (!bRet)
		APP_LOG(LOG_ERROR, _T("_DisplayHistoryMsgToIE CallJScript failed,%s"), jsData);
	if (scrollBottom)
	{
		logic::GetLogic()->asynFireUIEventWithLambda(
			[=]()
		{
			CComVariant result;
			m_pWebBrowser->CallJScript(_T("scrollBottom"), _T(""), &result);
		});
	}
}

void SessionLayout::UpdateSendMsgKey()
{
	if (!m_pInputRichEdit)
	{
		return;
	}
	module::TTConfig* pTTConfig = module::getSysConfigModule()->getSystemConfig();
	BOOL bWantCtrlEnter = (pTTConfig->sysBaseFlag & module::BASE_FLAG_SENDIMG_BY_CTRLENTER);
	if (bWantCtrlEnter)
	{
		m_pSendDescription->SetText(util::getMultilingual()->getStringViaID(_T("STRID_SESSIONMODULE_CTRLENTERSEND")));
	}
	else
	{
		m_pSendDescription->SetText(util::getMultilingual()->getStringViaID(_T("STRID_SESSIONMODULE_ENTERSEND")));
	}
	m_pInputRichEdit->SetWantReturn(!bWantCtrlEnter);
	m_bottomLayout->NeedUpdate();
}

BOOL SessionLayout::_DisplayUnreadMsg()
{
	SessionMessage_List msgList;
	if (!ReceiveMsgManage::getInstance()->popAllMessageBySId(m_sId, msgList))
	{
		//没有未读消息
		return FALSE;
	}

	for (auto MessageInfo : msgList)
	{
		_DisplayMsgToIE(MessageInfo, _T("sendMessage"));
		//给接收到的消息增加offset计数
		module::getMessageModule()->countMsgOffset(MessageInfo.talkerSid, 1);
	}

	//保存到历史消息中
	module::getMessageModule()->sqlBatchInsertHistoryMsg(msgList);

	//发送已读确认 todo...
	auto msg = msgList.front();
	_AsynSendReadAck(msg);
	return TRUE;
}
/******************************************************************************/