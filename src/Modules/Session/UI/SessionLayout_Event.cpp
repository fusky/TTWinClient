/******************************************************************************* 
 *  @file      SessionLayout_Event.cpp 2014\8\15 13:03:04 $
 *  @author    大佛<dafo@mogujie.com>
 *  @brief     
 ******************************************************************************/

#include "stdafx.h"
#include "Modules/UI/SessionLayout.h"
#include "Modules/IEmotionModule.h"
#include "Modules/IMiscModule.h"
#include "Modules/IGroupListModule.h"
#include "Modules/ISessionModule.h"
#include "Modules/IP2PCmdModule.h"
#include "Modules/ISysConfigModule.h"
#include "Modules/IFileTransferModule.h"
#include "../SessionManager.h"
#include "../../Message/SendMsgManage.h"
#include "../../Message/ReceiveMsgManage.h"
#include "../Operation/SendImgHttpOperation.h"
#include "utility/Multilingual.h"
#include "utility/utilStrCodeAPI.h"
#include "UIIMEdit.h"

#define  TIMER_CHECK_WRITING 1

/******************************************************************************/

void SessionLayout::Notify(TNotifyUI& msg)
{
	if (_tcsicmp(msg.sType, DUI_MSGTYPE_WINDOWINIT) == 0 )
	{
		OnWindowInitialized(msg);
	}
	else if (msg.sType == DUI_MSGTYPE_CLICK)
	{
		if (msg.pSender == m_pBtnSendMsg)
		{
			SendMsg();
		}
		else if (m_pBtnClose == msg.pSender)
		{
		}
		else if (msg.pSender == m_pBtnEmotion)
		{
			//表情先不展示
			POINT pt = { 0 };
			CDuiRect rcEmotionBtn = msg.pSender->GetPos();
			CDuiRect rcWindow;
			GetWindowRect(m_pManager->GetPaintWindow(), &rcWindow);

			pt.y = rcWindow.top + rcEmotionBtn.top;
			pt.x = rcWindow.left + rcEmotionBtn.left;

			module::getEmotionModule()->showEmotionDialog(m_sId,pt);
		}
		else if (msg.pSender == m_pBtnSendImage)
		{
			std::list<CString> lstFile;
			util::GetOpenFilePath(AfxGetMainWnd(), lstFile, FALSE, _T("图片 文件|*.png;*.jpeg;*.jpg;*.gif;*.bmp||"));
			if (lstFile.empty())
				return;
			CString strFile = lstFile.front();
			if (strFile.IsEmpty())
			{
				return;
			}
			_SendImage(strFile);
		}
		else if (msg.pSender == m_pBtnbanGroupMsg)
		{
			_OnBanGroupMsg(true);
		}
		else if (msg.pSender == m_pBtndisplayGroupMsg)
		{
			_OnBanGroupMsg(false);
		}
		else if (msg.pSender == m_pBtnshock)
		{
			module::UserInfoEntity userInfo;
			if (module::getUserListModule()->getUserInfoBySId(m_sId, userInfo))
			{
				if (userInfo.onlineState != USER_STATUS_ONLINE)
				{
					_DisplaySysMsg(_T("STRID_SESSIONMODULE_SHAKEWINDOW_OFFLINE_TIP"));
					return;
				}
			}
			else
			{
				return;
			}
			DWORD CurTime = GetTickCount();
			if (CurTime - m_tShakeWindow < 5000)
			{
				_DisplaySysMsg(_T("STRID_SESSIONMODULE_SHAKEWINDOW_TOOMAMY_TIP"));
				return;
			}
			m_tShakeWindow = CurTime;
			_DisplaySysMsg(_T("STRID_SESSIONMODULE_SHAKEWINDOW_SEND_TIP"));
			logic::GetLogic()->asynNotifyObserver(module::KEY_SESSION_SHAKEWINDOW_MSG, m_sId);
			module::getP2PCmdModule()->tcpSendShakeWindowCMD(m_sId);
		}
		//文件传输
		else if (msg.pSender == m_pBtnsendfile)
		{
			module::UserInfoEntity userInfo;
			if (!module::getUserListModule()->getUserInfoBySId(m_sId, userInfo))
			{
				APP_LOG(LOG_ERROR, _T("SendFile can't find the sid"));
				return;
			}
			std::list<CString> lstFile;
			util::GetOpenFilePath(AfxGetMainWnd(), lstFile, FALSE, _T("文件|*.*||"));
			if (lstFile.empty())
				return;
			CString strFile = lstFile.front();
			if (strFile.IsEmpty())
			{
				return;
			}
			module::getFileTransferModule()->sendFile(strFile, m_sId, userInfo.isOnlne());
		}
		else if (msg.pSender == m_pBtnadduser)
		{
			module::getGroupListModule()->onCreateDiscussionGrpDialog(m_sId);
		}
	}
	else if (msg.sType == _T("return"))
	{
		if (msg.pSender == m_pInputRichEdit)
		{
			SendMsg();
		}
	}
	else if (0 == _tcsicmp(msg.sType, DUI_MSGTYPE_ITEMACTIVATE))
	{
		if (msg.pSender->GetName() == _T("ListGroupMembersItem"))
		{
			CListContainerElementUI* pListElement = static_cast<CListContainerElementUI*>(msg.pSender);
			if (!pListElement->GetUserData().IsEmpty())
			{
				std::string sid = util::cStringToString(CString(pListElement->GetUserData()));
				logic::GetLogic()->asynNotifyObserver(module::KEY_SESSION_OPENNEWSESSION, sid);//通知主窗口创建会话
			}
		}
	}
	else if (0 == _tcsicmp(msg.sType, UIIMEdit_MSGTYPE_TEXTCHANGED))
	{
		if (msg.pSender->GetName() == _T("input_richedit"))
		{
			if (!m_bGroupSession)
			{
				module::getP2PCmdModule()->tcpSendWritingCMD(m_sId, TRUE);
				m_pManager->KillTimer(this, TIMER_CHECK_WRITING);
				m_pManager->SetTimer(this, TIMER_CHECK_WRITING, 5000);
			}
		}
	}
	else if (0 == _tcsicmp(msg.sType, DUI_MSGTYPE_KILLFOCUS))
	{
		if (!m_bGroupSession)
		{
			m_pManager->KillTimer(this, TIMER_CHECK_WRITING);
			module::getP2PCmdModule()->tcpSendWritingCMD(m_sId, FALSE);
		}
	}
	else if (0 == _tcsicmp(msg.sType, DUI_MSGTYPE_TEXTCHANGED))
	{
		if (msg.pSender == m_pSearchEdit)
		{
			m_pSearchResultList->RemoveAll();
			CDuiString inputText = m_pSearchEdit->GetText();
			if (inputText.IsEmpty())
			{
				m_pSearchResultList->SetVisible(false);
				m_pGroupMemberList->SetVisible(true);
			}
			else
			{
				m_pGroupMemberList->SetVisible(false);
				m_pSearchResultList->SetVisible(true);

				module::UserInfoEntityVec userList;
				_GetGroupNameListByShortName(inputText.GetData(), userList);
				_UpdateSearchRsultList(userList);
			}
		}
	}
}
void SessionLayout::_OnBanGroupMsg(const bool bBan)
{
	m_pBtnbanGroupMsg->SetVisible(!bBan);
	m_pBtndisplayGroupMsg->SetVisible(bBan);
	SessionEntity* pSessionInfo = SessionEntityManager::getInstance()->getSessionEntityBySId(m_sId);
	if (!pSessionInfo)
	{
		APP_LOG(LOG_ERROR, _T("SessionLayout::_OnBanGroupMsg-can't find the sessionid"));
		return;
	}
	pSessionInfo->m_bBanGroupMsg = !bBan;
	SessionEntityManager::getInstance()->saveBanGroupMSGSetting(m_sId, pSessionInfo->m_bBanGroupMsg);
}
void SessionLayout::_GetGroupNameListByShortName(IN const CString& sShortName, OUT std::vector<string>& nameList)
{
	for (int n = 0; n < m_pGroupMemberList->GetCount();++n)
	{
		CListContainerElementUI* pListElement = static_cast<CListContainerElementUI*>(m_pGroupMemberList->GetItemAt(n));
		if (pListElement)
		{
			CLabelUI* pNameLable = static_cast<CLabelUI*>(pListElement->FindSubControl(_T("nickname")));
			if (!pNameLable)
			{
				continue;;
			}
			CString Name = pNameLable->GetText();
			std::string sid = util::cStringToString(CString(pListElement->GetUserData()));

			if (util::isIncludeChinese(util::cStringToString(sShortName, CP_ACP)))//检索中文
			{
				if (Name.Find(sShortName) != -1)
				{
					nameList.push_back(sid);
				}
			}
			else//检索字母
			{
				CString firstPY = util::HZ2FirstPY(util::cStringToString(Name, CP_ACP));
				if (firstPY.Find(sShortName) != -1)//先检索简拼
				{
					nameList.push_back(sid);
				}
				else
				{
					CString allPY = util::HZ2AllPY(Name);//再检索全拼
					if (allPY.Find(sShortName) != -1)
					{
						nameList.push_back(sid);
					}
				}
			}
		}
	}
}

void SessionLayout::DoEvent(TEventUI& event)
{
	if (event.Type == UIEVENT_TIMER  )
	{
		if (event.pSender == this && !m_bGroupSession && TIMER_CHECK_WRITING == event.wParam)
		{
			module::getP2PCmdModule()->tcpSendWritingCMD(m_sId, FALSE);
		}
	}
	else if (event.Type == UIEVENT_CONTEXTMENU)
	{
	}
}

void SessionLayout::DocmentComplete(IDispatch *pDisp, VARIANT *&url)
{
	logic::GetLogic()->asynFireUIEventWithLambda(
		[=]()
	{
		if (!_DisplayUnreadMsg())
		{
			_DisplayHistoryMsgToIE(20,TRUE);
		}
	}
	);
}

HRESULT STDMETHODCALLTYPE SessionLayout::TranslateUrl( /* [in] */ DWORD dwTranslate, /* [in] */ OLECHAR __RPC_FAR *pchURLIn, /* [out] */ OLECHAR __RPC_FAR *__RPC_FAR *ppchURLOut)
{
	m_csTobeTranslateUrl = pchURLIn;
	*ppchURLOut = 0;

	if (!IsWindow(m_pManager->GetPaintWindow()))
	{
		return S_OK;
	}
	//历史消息内容
	CString csUrl = pchURLIn;
	if (csUrl.Find(_T("moguim/:history")) > -1)//显示历史消息
	{
		_DisplayHistoryMsgToIE(20,FALSE);
	}
	else if (csUrl.Find(_T("moguim/:playvoice")) > -1)//播放语音文件
	{
		int npos = csUrl.Find(_T("?"));
		if (-1 != npos)
		{
			//先停掉前面一个的播放动画
			AudioMessageMananger::getInstance()->popPlayingAudioMsg();

			//播放当前选择的声音
			string sAudioID = util::cStringToString(csUrl.Mid(npos + 1, csUrl.GetLength() - npos));
			AudioMessageMananger::getInstance()->playAudioMsgByAudioSid(m_sId, sAudioID);

			//由于播放同一个文件两次，在第二次的时候，gif动画就不出来了，这个时候需要手动再调一下
			StartPlayingAnimate(sAudioID);
		}
	}
	else if (csUrl.Find(_T("moguim/:chat2")) > -1)//点击了联系某个人
	{
		int npos = csUrl.Find(_T("?"));
		if (-1 != npos)
		{
			string sUesrID = util::cStringToString(csUrl.Mid(npos + 1, csUrl.GetLength() - npos));
			if (!sUesrID.empty()
				&& sUesrID != module::getSysConfigModule()->userID())//不能和自己联系
			{
				//创建会话窗口
				logic::GetLogic()->asynNotifyObserver(module::KEY_SESSION_OPENNEWSESSION, sUesrID);//通知主窗口创建会话
			}
		}
	}
	return S_OK;
}

void SessionLayout::NewWindow2(VARIANT_BOOL *&Cancel, BSTR bstrUrl)
{
	*Cancel = VARIANT_TRUE;
	if (m_csTobeTranslateUrl.Find(_T("moguim/:history")) > -1
		|| m_csTobeTranslateUrl.Find(_T("moguim/:chat2")) > -1
		|| m_csTobeTranslateUrl.Find(_T("moguim/:playvoice"))>-1)
		return;
	module::getMiscModule()->asynOpenWebBrowser(m_csTobeTranslateUrl);
}

void SessionLayout::OnEmotionModuleEvent(UInt16 moduleId, UInt32 keyId, MKO_TUPLE_PARAM mkoParam)
{
	if (keyId == module::KEY_EMOTION_SELECTED)
	{
		std::shared_ptr<void> p = std::get<MKO_SHARED_VOIDPTR>(mkoParam);
		module::EmotionParam* pEmotionParam = (module::EmotionParam*) p.get();
		if (pEmotionParam && pEmotionParam->sid == m_sId)
		{
			CString strPath = pEmotionParam->strPath;
			if (m_pInputRichEdit && !strPath.IsEmpty())
			{
				SIZE size = { 0, 0 };
				m_pInputRichEdit->InsertImage(strPath.GetBuffer(),size,TRUE);
			}
		}
	}
}

BOOL SessionLayout::StopPlayingAnimate(std::string& sAudioPlayingID)
{
	if (sAudioPlayingID.empty())
	{
		return FALSE;
	}

	CString param;
	param = util::stringToCString(sAudioPlayingID);
	CComVariant result;
	BOOL bRet = m_pWebBrowser->CallJScript(_T("stopVoice"), param.GetBuffer(), &result);
	if (!bRet)
		APP_LOG(LOG_ERROR, _T("stopVoice CallJScript failed,%s"), util::stringToCString(sAudioPlayingID));

	return TRUE;
}
BOOL SessionLayout::StartPlayingAnimate(std::string sAudioPlayingID)
{
	if (sAudioPlayingID.empty())
	{
		return FALSE;
	}

	CString param;
	param = util::stringToCString(sAudioPlayingID);
	CComVariant result;
	BOOL bRet = m_pWebBrowser->CallJScript(_T("playVoice"), param.GetBuffer(), &result);
	if (!bRet)
		APP_LOG(LOG_ERROR, _T("playVoice CallJScript failed,%s"), util::stringToCString(sAudioPlayingID));

	return TRUE;
}

void SessionLayout::OnCallbackOperation(std::shared_ptr<void> param)
{
	SendImgParam* pParam = (SendImgParam*)param.get();
	if (pParam == nullptr)
	{
		return;
	}

	for (auto mixedMsgIt = m_SendingMixedMSGList.begin(); mixedMsgIt != m_SendingMixedMSGList.end(); mixedMsgIt++)
	{
		for (auto picdata : mixedMsgIt->m_picDataVec)
		{
			if (picdata.strLocalPicPath == pParam->csFilePath)
			{
				if (SendImgParam::SENDIMG_OK == pParam->m_result)
				{
					mixedMsgIt->SetNetWorkPicPath(pParam->csFilePath, util::stringToCString(pParam->m_pathUrl));
					if (mixedMsgIt->SucceedToGetAllNetWorkPic())
					{
						MessageEntity msg;
						msg.content = util::cStringToString(mixedMsgIt->MakeMixedNetWorkMSG());
						msg.sessionId = m_sId;
						msg.talkerSid = module::getSysConfigModule()->userID();
						msg.msgRenderType = MESSAGE_RENDERTYPE_TEXT;

						SessionEntity* pSessionInfo = SessionEntityManager::getInstance()->getSessionEntityBySId(m_sId);

						if (!pSessionInfo)
						{
							return;
						}
						if (pSessionInfo->m_sessionType == SESSION_USERTYPE)
						{
							msg.msgType = MSG_TYPE_TEXT_P2P;
						}
						else
						{
							msg.msgType = MSG_TYPE_TEXT_GROUP;
						}
						msg.msgFromType = pSessionInfo->m_sessionType;	//sessionType和FromType定义一致
						msg.msgTime = module::getSessionModule()->getTime();
						SendMsgManage::getInstance()->pushSendingMsg(msg);					
						m_SendingMixedMSGList.erase(mixedMsgIt);
						//更新会话时间
						SessionEntity*  pSessionEntity = SessionEntityManager::getInstance()->getSessionEntityBySId(msg.sessionId);
						if (pSessionEntity)
						{
							pSessionEntity->m_updatedTime = msg.msgTime;
						}
						//主界面 消息内容，时间更新
						logic::GetLogic()->asynNotifyObserver(module::TAG_SESSION_TRAY_NEWMSGSEND, msg.sessionId);
					}
				}
				else
				{
					APP_LOG(LOG_ERROR, _T("SessionLayout::OnCallbackOperation-STRID_SESSIONMODULE_MESSAGE_IMAGEFAILED:%d"), pParam->m_result);
					m_SendingMixedMSGList.erase(mixedMsgIt);
					MessageEntity msg;
					msg.content = util::cStringToString(util::getMultilingual()->getStringViaID(_T("STRID_SESSIONMODULE_MESSAGE_IMAGEFAILED")));
					msg.sessionId = m_sId;
					msg.talkerSid = module::getSysConfigModule()->userID();
					msg.msgRenderType = MESSAGE_RENDERTYPE_SYSTEMTIPS;
					ReceiveMsgManage::getInstance()->pushMessageBySId(m_sId, msg);
					logic::GetLogic()->asynNotifyObserver(module::KEY_SESSION_NEWMESSAGE, msg.sessionId);	//传图失败
				}
				return;
			}
		}
	}
}
/******************************************************************************/