/******************************************************************************* 
 *  @file      ReceiveMsgManage.cpp 2014\8\7 14:57:37 $
 *  @author    大佛<dafo@mogujie.com>
 *  @brief     
 ******************************************************************************/

#include "stdafx.h"
#include "ReceiveMsgManage.h"
#include "utility/utilStrCodeAPI.h"
#include "utility/utilCommonAPI.h"
#include "utility/Multilingual.h"
#include "utility/TTAutoLock.h"
#include "Modules/IUserListModule.h"
#include "Modules/IEmotionModule.h"
#include "Modules/ISysConfigModule.h"
#include "../Session/SessionManager.h"
#include "Modules/UI/SessionDialog.h"
#include "../Session/UI/UIIMEdit.h"
#include "cxImage/cxImage/ximage.h"
/******************************************************************************/
namespace
{
	const CString HTML_IMG_EMOTION_TAG = _T("<img src=\"%s\" width=\"22\" height=\"22\" />");
	const CString HTML_IMG_IMAGE_TAG = _T("<a href=\"%s\" target=\"_blank\"><img title=\"%s\" src=\"%s\" /></a>");
	const CString HTML_IMG_IMAGE_NOHEITHTAG
		= _T("<a href=\"%s\" target=\"_blank\"><img title=\"%s\" src=\"%s\" width=\"%d\" /></a>");
	const CString HTML_IMG_IMAGE_WITHHEITHTAG
		= _T("<a href=\"%s\" target=\"_blank\"><img title=\"%s\" src=\"%s\" width=\"%d\" height=\"%d\" /></a>");
	const CString HTML_A_TAG = _T("<a href=\"%s\" target=\"_blank\">%s</a>");
	const CString HTML_CHAT2_TAG = _T("<a class=\"blue_link\" href=\"moguim://moguim/:chat2?%s\" target=\"_blank\">@%s</a>");//替换成连接
	const UInt8 MAX_RECEIVEMSG_CNT = 10;	//消息去重，每个会话缓存10条信息


}
// -----------------------------------------------------------------------------
//  ReceiveMsgManage: Public, Constructor

ReceiveMsgManage::ReceiveMsgManage()
{

}

// -----------------------------------------------------------------------------
//  ReceiveMsgManage: Public, Destructor

ReceiveMsgManage::~ReceiveMsgManage()
{

}

ReceiveMsgManage* ReceiveMsgManage::getInstance()
{
	static ReceiveMsgManage manager;
	return &manager;
}

BOOL ReceiveMsgManage::pushMessageBySId(const std::string& sId, MessageEntity& msg)
{
	if (sId.empty())
		return FALSE;

	util::TTAutoLock lock(&m_lock);
	try
	{
		SessionMessage_List* listChatMsg = _getChatMsgListBySID(sId);
		if (listChatMsg)
		{
			listChatMsg->push_back(msg);
		}
		else
		{
			SessionMessage_List listChatMsg2;
			listChatMsg2.push_back(msg);
			m_mapSessionMsg[sId] = listChatMsg2;
		}
	}
	catch (...)
	{
		CString csSid = util::stringToCString(sId);
		CString csMsgCnt = util::stringToCString(msg.content, CP_UTF8);
		APP_LOG(LOG_ERROR, _T("pushMessageBySId failed,SId:%s,msg conteng:%s"), csSid, csMsgCnt);
		return FALSE;
	}

	return TRUE;
}
BOOL ReceiveMsgManage::popMessageBySId(const std::string& sId, MessageEntity& msg)
{
	util::TTAutoLock lock(&m_lock);
	SessionMessage_List* listChatMsg = _getChatMsgListBySID(sId);
	if (listChatMsg && !listChatMsg->empty())
	{
		msg = listChatMsg->front();
		if (MSG_TYPE_AUDIO_P2P == msg.msgType && !msg.isReaded())//如果是语音未读消息,先存起来
		{
			AudioMessageMananger::getInstance()->pushAudioMessageBySId(sId, msg);
		}
		listChatMsg->pop_front();
		return TRUE;
	}

	return FALSE;
}
BOOL ReceiveMsgManage::frontMessageBySId(const std::string& sId, MessageEntity& msg)
{
	util::TTAutoLock lock(&m_lock);
	SessionMessage_List* listChatMsg = _getChatMsgListBySID(sId);
	if (listChatMsg && !listChatMsg->empty())
	{
		msg = listChatMsg->back();
		return TRUE;
	}

	return FALSE;
}

BOOL ReceiveMsgManage::popAllMessageBySId(const std::string& sId, SessionMessage_List& msgList)
{
	util::TTAutoLock lock(&m_lock);
	SessionMessage_List* listChatMsg = _getChatMsgListBySID(sId);
	if (listChatMsg && !listChatMsg->empty())
	{
		msgList = *listChatMsg;
		for (SessionMessage_List::iterator it = msgList.begin();
			it != msgList.end(); ++it)
		{
			if (!it->isReaded())
			{
				AudioMessageMananger::getInstance()->pushAudioMessageBySId(sId, *it);
			}
		}
		listChatMsg->clear();
		return TRUE;
	}

	return FALSE;
}

UInt32 ReceiveMsgManage::getUnReadMsgCountBySId(const std::string& sId)
{
	util::TTAutoLock lock(&m_lock);
	SessionMessage_List* listChatMsg = _getChatMsgListBySID(sId);
	if (listChatMsg)
	{
		return listChatMsg->size();
	}
	return 0;
}
UInt32 ReceiveMsgManage::getTotalUnReadMsgCountByIds(std::vector<std::string>& vecIds)
{
	UInt32 totalCount = 0;
	std::vector<std::string>::iterator iter = vecIds.begin();
	for (; iter != vecIds.end(); ++iter)
	{
		totalCount += getUnReadMsgCountBySId(*iter);
	}

	return totalCount;
}
UInt32 ReceiveMsgManage::getTotalUnReadSysMsgCount()
{
	UInt32 totalCount = 0;
	totalCount += getUnReadMsgCountBySId("124wuoe");
	totalCount += getUnReadMsgCountBySId("123mhci");

	return totalCount;
}

void ReceiveMsgManage::removeAllMessage()
{
	util::TTAutoLock lock(&m_lock);
	//记录下程序退出时所有的消息列表，并且记录下来，这个会成为离线消息
	SessionMessageMap::iterator iterMap = m_mapSessionMsg.begin();
	APP_LOG(LOG_ERROR, _T("MessageMananger unread message trace begin："));
	for (; iterMap != m_mapSessionMsg.end(); ++iterMap)
	{
		SessionMessage_List msgList;
		if (popAllMessageBySId(iterMap->first, msgList))
		{
			SessionMessage_List::iterator iter = msgList.begin();
			for (; iter != msgList.end(); ++iter)
			{
				MessageEntity& msg = *iter;
				CString csUId = util::stringToCString(msg.talkerSid);
				CString csCnt = util::stringToCString(msg.content, CP_UTF8);
				CTime time(msg.msgTime);
				CString csTime = time.Format(_T("%Y-%m-%d %H:%M:%S"));
				APP_LOG(LOG_DEBUG, _T("time:%s id:%s,content:%s"), csTime, csUId, csCnt);
			}
		}
	}
	APP_LOG(LOG_DEBUG, _T("MessageMananger unread message trace end"));
	m_mapSessionMsg.clear();
}

void ReceiveMsgManage::removeMessageBySId(const std::string& sId)
{
	SessionMessage_List* listChatMsg = _getChatMsgListBySID(sId);
	if (listChatMsg && !listChatMsg->empty())
	{
		listChatMsg->clear();
	}
}
void ReceiveMsgManage::parseContent(CString& content, BOOL bFloatForm, Int32 chatWidth, BOOL isTo)
{
	_urlScan(content);
	if (!bFloatForm)
	{
		content.Replace(_T("<"), _T("&lt;"));
		content.Replace(_T(">"), _T("&gt;"));
		content.Replace(_T("\""), _T("&quot;"));
		content.Replace(_T("\'"), _T("&#039;"));
		//content.Replace(_T("\n"),_T("<br>"));
		for (int i = 0; i <= content.GetLength() - 1; i++)
		if (content.GetAt(i) == 0x0D)//如果是回车
		{
			CString strfront = content.Mid(0, i);
			CString strBack = content.Mid(i + 1, content.GetLength() - i - 1);
			content = strfront + _T("<br>") + strBack;
			i = i + 3;//替换的字符比原来多了3个，可以略过检查
		}
	}
	int hitStartIndex = content.Find(CS_SPLIT_CODE_START);
	if (hitStartIndex < 0 && !bFloatForm)
	{
		content.Replace(_T(" "), _T("&nbsp"));
	}

	//解析表情
	int startIndex = content.Find(_T("["));
	while (startIndex != -1)
	{
		int endIndex = content.Find(_T("]"), startIndex);
		if (-1 == endIndex)
			break;
		CString csEmotion = content.Mid(startIndex, endIndex - startIndex + 1);

		CString csPath;
		if (module::getEmotionModule()->getEmotionNameByID(csEmotion, csPath))
		{
			CString csHtml;
			if (bFloatForm)
			{
				csHtml = util::getMultilingual()->getStringViaID(_T("STRID_FLOATFORM_EMOTION"));
			}
			else
			{
				csHtml.Format(HTML_IMG_EMOTION_TAG, csPath);
			}
			content.Replace(csEmotion, csHtml);
		}
		startIndex = content.Find(_T("["), endIndex);
	}

	//替换图片
	BOOL bCanRelaceSpace = TRUE;
	startIndex = content.Find(CS_SPLIT_CODE_START);
	while (startIndex != -1)
	{
		int endIndex = content.Find(CS_SPLIT_CODE_END, startIndex);
		if (-1 == endIndex)
			break;
		CString csImgPath = content.Mid(startIndex + CS_SPLIT_CODE_START.GetLength(), endIndex
			- startIndex - CS_SPLIT_CODE_START.GetLength());
		CString csImgPathTag = content.Mid(startIndex, endIndex - startIndex + CS_SPLIT_CODE_END.GetLength());
		CString csHtml;
		if (bFloatForm)
		{
			csHtml = util::getMultilingual()->getStringViaID(_T("STRID_FLOATFORM_IMAGE"));
		}
		else
		{
			chatWidth -= 130;
			CString csTips = util::getMultilingual()->getStringViaID(_T("STRID_SESSIONMODULE_MESSAGE_SOURCEIMAGE"));
			if (isTo)
			{
				//通过字符串分析解析图片宽高
				CString csWidth, csHeight;
				int startIndex = csImgPath.ReverseFind('_');
				int endIndex = csImgPath.ReverseFind('x');
				if (startIndex >= 0 && endIndex >= 0)
				{
					csWidth = csImgPath.Mid(startIndex + 1, endIndex - startIndex - 1);
				}
				startIndex = endIndex;
				endIndex = csImgPath.ReverseFind('.');
				if (startIndex >= 0 && endIndex >= 0)
				{
					csHeight = csImgPath.Mid(startIndex + 1, endIndex - startIndex - 1);
				}
				int widImage = util::cstringToInt32(csWidth);
				int height = util::cstringToInt32(csHeight);
				int widSel = chatWidth;
				module::TTConfig* pCfg = module::getSysConfigModule()->getSystemConfig();
				CString csUrlPath = pCfg->fileSysAddr + csImgPath;
				if (widImage > 0 && widImage < chatWidth)
				{
					widSel = widImage;
					csHtml.Format(HTML_IMG_IMAGE_WITHHEITHTAG, csUrlPath, csTips, csUrlPath, widSel, height);
				}
				else
				{
					csHtml.Format(HTML_IMG_IMAGE_NOHEITHTAG, csUrlPath, csTips, csUrlPath, widSel);
				}
			}
			else
			{
				//获取本地图片的宽度
				CxImage cximage;
				if (cximage.Load(csImgPath))
				{
					//通过handle获取bitmap
					int widSel = chatWidth;
					int widImage = cximage.GetWidth();
					int height = cximage.GetHeight();
					if (widImage > 0 && widImage <= chatWidth)
					{
						widSel = widImage;
						csHtml.Format(HTML_IMG_IMAGE_WITHHEITHTAG, csImgPath, csTips, csImgPath, widSel, height);
					}
					else
					{
						csHtml.Format(HTML_IMG_IMAGE_NOHEITHTAG, csImgPath, csTips, csImgPath, widSel);
					}

				}
				else
				{
					APP_LOG(LOG_ERROR, _T("parseContent LoadImage failed,path:%s"), csImgPath);
					csHtml.Format(HTML_IMG_IMAGE_TAG, csImgPath, csTips, csImgPath);
				}
				bCanRelaceSpace = FALSE;
			}
		}
		//先注释掉，目前还没这段代码的用处。
		//todo... 若启用，会导致一条消息中有多张图片时，只能显示一张图
		//if(bCanRelaceSpace && !bFloatForm)
		//    content.Replace(_T(" "),_T("&nbsp"));
		content.Replace(csImgPathTag, csHtml);
		startIndex = content.Find(CS_SPLIT_CODE_START, endIndex);
	}
	_urlReplace(content);
	_Quickchat2Fromat(content);
}

SessionMessage_List* ReceiveMsgManage::_getChatMsgListBySID(const std::string& sId)
{
	util::TTAutoLock lock(&m_lock);
	SessionMessageMap::iterator iter = m_mapSessionMsg.find(sId);
	if (iter == m_mapSessionMsg.end())
		return 0;

	return &(iter->second);
}
int ReceiveMsgManage::_outputUrlCallback(unsigned pos, const UrlScanner::STRING& url)
{
	CString csUrl(url.c_str());
	//黑名单处理...

	std::vector<CString>::iterator iter = m_scanUrls.begin();
	for (; iter != m_scanUrls.end(); ++iter)
	{
		if (iter->CompareNoCase(csUrl) == 0)
			return 0;
	}
	m_scanUrls.push_back(csUrl);
	return 0;
}
void ReceiveMsgManage::_urlReplace(CString& content)
{
	std::vector<CString>::iterator iter = m_scanUrls.begin();
	for (; iter != m_scanUrls.end(); ++iter)
	{
		CString csHtml, csUrl(*iter);
		//对www开头的链接前面加http://
		if (csUrl.Find(_T("www.")) > -1 && csUrl.Find(_T("http://")) < 0)
		{
			csUrl = _T("http://") + csUrl;
		}
		csHtml.Format(HTML_A_TAG, csUrl, *iter);
		content.Replace(*iter, csHtml);
	}
}

void ReceiveMsgManage::_urlScan(CString& content)
{
	m_scanUrls.clear();
	UrlScanner scan;
	scan.init(this, 0);
	for (int i = 0; i < (int)content.GetLength(); ++i)
	{
		scan.feed(content[i]);
	}
	scan.flush();
}
//////////////////////////////////消息去重////////////////////////////////////////


BOOL ReceiveMsgManage::checkIsReduplicatedMsg(IN const MessageEntity& msg, IN const UInt32 seqNo)
{
	ReceiveMsgMap::iterator itMap = m_MsgMap.find(msg.sessionId);
	if (itMap != m_MsgMap.end())
	{
		ReceiveMsgList& MsgList = itMap->second;
		BOOL bFind = FALSE;
		for (ReceiveMsgList::reverse_iterator itList = MsgList.rbegin();
			itList != MsgList.rend(); ++itList)
		{
			if (itList->msg.content == msg.content
				&& itList->seqNo == seqNo)
			{
				bFind = TRUE;
				return TRUE;
			}
		}

		if (!bFind)
		{
			if (MAX_RECEIVEMSG_CNT == MsgList.size())
			{
				MsgList.pop_front();
			}
			ReceiveMsg rMsg;
			rMsg.msg = msg;
			rMsg.seqNo = seqNo;
			MsgList.push_back(rMsg);
		}
	}
	else
	{
		ReceiveMsgList rMsglist;
		ReceiveMsg rMsg;
		rMsg.msg = msg;
		rMsg.seqNo = seqNo;
		rMsglist.push_back(rMsg);
		m_MsgMap[msg.sessionId] = rMsglist;
	}
	return FALSE;
}

void ReceiveMsgManage::_Quickchat2Fromat(OUT CString& content)
{
	int nStart = 0;
	while (TRUE)
	{
		int nLen = content.GetLength();
		int nPos = content.Find('@', nStart);
		if (-1 != nPos && nPos + 2 < nLen)
		{
			CString sName = content.Mid(nPos + 1, 2);
			BOOL bFind = FALSE;
			CString csHTML;
			module::UserInfoEntityMap MapUsers;
			module::getUserListModule()->getAllUsersInfo(MapUsers);
			for (auto itMapItem:MapUsers)
			{
				module::UserInfoEntity&  userInfo = itMapItem.second;
				if (userInfo.csName == sName)
				{
					//do替换
					csHTML.Format(HTML_CHAT2_TAG, util::stringToCString(userInfo.sId), sName);
					CString strA = content.Mid(0, nPos);
					CString strB = content.Mid(nPos + 3);
					content = strA + csHTML + strB;
					bFind = TRUE;
					break;
				}
			}
			if (bFind)
			{
				nStart = nPos + csHTML.GetLength();
			}
			else
			{
				nStart = nPos + 1;
			}
		}
		else
		{
			break;
		}
	}
}

/******************************************************************************/

//////////////////////////////语音消息接受处理////////////////////////////////////////////
AudioMessageMananger::~AudioMessageMananger()
{
}

AudioMessageMananger* AudioMessageMananger::getInstance()
{
	static AudioMessageMananger manager;
	return &manager;
}

BOOL AudioMessageMananger::playAudioMsgByAudioSid(IN const std::string& sSessionID, IN const std::string& sAID)
{
	//若是未读语音消息，则置成已读
	SessionMessageMap::iterator itAudio = m_mapUnReadAudioMsg.find(sSessionID);
	if (itAudio != m_mapUnReadAudioMsg.end())
	{
		SessionMessage_List& listChatMsg = itAudio->second;
		for (SessionMessage_List::iterator it = listChatMsg.begin();
			it != listChatMsg.end(); ++it)
		{
			if (it->content == sAID)
			{
				it->msgAudioReaded = 1;
			}
		}
	}

	//播放声音
	CString csFilePath = module::getMiscModule()->GetAudioFileSavedPath();
	csFilePath += util::stringToCString(sAID, CP_UTF8);
	if (!util::isFileExist(csFilePath))
	{
		APP_LOG(LOG_ERROR, _T("AudioMessageMananger::playAudioMsgByAudioSid-File not exist:%s "), csFilePath);
		return FALSE;
	}
	csFilePath = _T("\"") + csFilePath + _T("\"");

	//先关闭原来的播放窗口 
	HWND hTTWnd = FindWindow(NULL, _T("teamtalkaudio-speexdecWindow"));
	if (hTTWnd)
	{
		COPYDATASTRUCT cpyData = { 0 };
		//cpyData.lpData = (PVOID)m_sPlayingAID.c_str();
		//cpyData.cbData = m_sPlayingAID.length();
		cpyData.dwData = 1; // 1 表示退出语音播放进程
		::SendMessage(hTTWnd, WM_COPYDATA, 0, (LPARAM)&cpyData);
		APP_LOG(LOG_DEBUG, _T("AudioMessageMananger::breakPlayingAnimate"));
	}

	//解析播放语音文件
	CString csPath = util::getAppPath() + _T("speexdec.exe");
	APP_LOG(LOG_DEBUG, _T("AudioMessageMananger::playAudioMsgByAudioSid FilePath:%s"), csFilePath);
	::ShellExecute(NULL, _T("open"), csPath, csFilePath, _T(""), SW_HIDE);

	m_sPlayingSessionID = sSessionID;
	m_sPlayingAID = sAID;
	return TRUE;
}


BOOL AudioMessageMananger::makeAppAudioSid(IN const UInt32 msgTime, IN const std::string sFromId, OUT std::string& sAID)
{
	CString strTime;
	CTime timeData(msgTime);
	strTime = timeData.Format(_T("-%Y%m%d-%H-%M-%S-"));

	CString strGuid;
	GUID guid;
	if (CoCreateGuid(&guid))
	{
		APP_LOG(LOG_ERROR, _T("create guid error"));
		//return FALSE;
	}
	strGuid.Format(_T("%08X%04X%04x%02X%02X%02X%02X%02X%02X%02X%02X"),
		guid.Data1, guid.Data2, guid.Data3,
		guid.Data4[0], guid.Data4[1], guid.Data4[2],
		guid.Data4[3], guid.Data4[4], guid.Data4[5],
		guid.Data4[6], guid.Data4[7]);

	module::UserInfoEntity info;
	module::getUserListModule()->getUserInfoBySId(sFromId, info);

	sAID = util::cStringToString(info.getRealName(), CP_UTF8)
		+ util::cStringToString(strTime, CP_UTF8)
		+ util::cStringToString(strGuid, CP_UTF8);//name+消息事件+GUID

	return TRUE;
}
BOOL AudioMessageMananger::saveAudioDataToFile(IN UCHAR* data, IN UINT32 lenth, IN std::string sFileName)
{
	if (lenth <= 4)
	{
		APP_LOG(LOG_ERROR, _T("AudioMessageMananger::saveAudioDataToFile-lenth<=4"));
		return FALSE;
	}
	CString sFilePath = module::getMiscModule()->GetAudioFileSavedPath();

	if (!::PathIsDirectory(sFilePath))
	{
		util::createAllDirectories(sFilePath);
		sFilePath += _T("\\");
	}

	sFilePath += util::stringToCString(sFileName, CP_UTF8);

	HANDLE hFile = CreateFile(sFilePath,
		GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL,
		CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		NULL);
	if (INVALID_HANDLE_VALUE == hFile)
	{
		DWORD res = GetLastError();
		APP_LOG(LOG_ERROR, _T("CreateFile error:GetLastError = %d"), res);
		return FALSE;
	}
	::SetFilePointer(hFile, 0, 0, FILE_END);

	DWORD lret = 0;
	BOOL  bRes = WriteFile(hFile, (LPCVOID)data, lenth, &lret, NULL);
	if (!bRes)
	{
		DWORD res = GetLastError();
		APP_LOG(LOG_ERROR, _T("WriteFile error:GetLastError = %d"), res);
		return FALSE;
	}

	if (NULL != hFile)
	{
		CloseHandle(hFile);
		hFile = NULL;
	}

	return TRUE;
}

BOOL AudioMessageMananger::getAudioMsgLenth(IN UCHAR* data, IN UINT32 lenth, OUT UInt8& AudioMsgLen)
{
	AudioMsgLen = 0;
	if (lenth < 4 || NULL == data)
	{
		//前4个字节为语音长度
		return FALSE;
	}
	AudioMsgLen = (data[0] << 24) + (data[1] << 16) + (data[2] << 8) + data[3];
	if (0 == AudioMsgLen)
	{
		APP_LOG(LOG_ERROR, _T("AudioMessageMananger::getAudioMsgLenth 语音时长为零"));
	}
	return TRUE;
}

BOOL AudioMessageMananger::pushAudioMessageBySId(const std::string& sId, MessageEntity& msg)
{
	SessionMessageMap::iterator it = m_mapUnReadAudioMsg.find(sId);
	if (it == m_mapUnReadAudioMsg.end())
	{
		SessionMessage_List listChatMsg;
		listChatMsg.push_back(msg);
		m_mapUnReadAudioMsg[sId] = listChatMsg;
	}
	else
	{
		SessionMessage_List& listChatMsg = it->second;
		listChatMsg.push_back(msg);
	}
	return TRUE;
}

BOOL AudioMessageMananger::autoplayNextUnReadAudioMsg()
{
	//停掉之前播放的动画
	stopPlayingAnimate();
	APP_LOG(LOG_ERROR, _T("AudioMessageMananger::stopPlayingAnimate"));

	SessionMessageMap::iterator itAudio = m_mapUnReadAudioMsg.find(m_sPlayingSessionID);
	if (itAudio == m_mapUnReadAudioMsg.end())
	{
		APP_LOG(LOG_DEBUG, _T("AudioMessageMananger::AutoplayNextUnReadAudioMsg: Not exist in the list!"));
		return FALSE;
	}

	SessionMessage_List& listChatMsg = itAudio->second;

	SessionMessage_List::iterator itNext = listChatMsg.end();
	SessionMessage_List::iterator it = listChatMsg.begin();
	for (; it != listChatMsg.end();)
	{
		if (it->isReaded() && it->content == m_sPlayingAID)
		{
			itNext = listChatMsg.erase(it);//删除已经播放过的
			break;
		}
		else
			++it;
	}

	if (itNext != listChatMsg.end())//说明是有效的ID
	{
		APP_LOG(LOG_DEBUG, _T("AudioMessageMananger::AutoplayNextUnReadAudioMsg"));
		startPlayingAnimate(itNext->content);//大补鸡支持
		playAudioMsgByAudioSid(m_sPlayingSessionID, itNext->content);
		return TRUE;
	}
	return FALSE;
}

BOOL AudioMessageMananger::popPlayingAudioMsg()
{
	stopPlayingAnimate();

	SessionMessageMap::iterator itAudio = m_mapUnReadAudioMsg.find(m_sPlayingSessionID);
	if (itAudio == m_mapUnReadAudioMsg.end())
	{
		APP_LOG(LOG_DEBUG, _T("AudioMessageMananger::popPlayingAudioMsg:Not exist in the list!"));
		return FALSE;
	}
	SessionMessage_List& listChatMsg = itAudio->second;
	SessionMessage_List::iterator it = listChatMsg.begin();
	for (; it != listChatMsg.end();)
	{
		if (it->isReaded() && it->content == m_sPlayingAID)
		{
			listChatMsg.erase(it);//删除已经播放过的
			m_sPlayingSessionID.clear();
			m_sPlayingAID.clear();
			return TRUE;
		}
		else
			++it;
	}
	return FALSE;
}

BOOL AudioMessageMananger::clearAudioMsgBySessionID(IN const std::string sSessionID)
{
	SessionMessageMap::iterator itAudio = m_mapUnReadAudioMsg.find(sSessionID);
	if (itAudio == m_mapUnReadAudioMsg.end())
	{
		APP_LOG(LOG_DEBUG, _T("AudioMessageMananger::playAudioMsgByAudioSid: Not exist in the list!"));
		return FALSE;
	}
	SessionMessage_List& listChatMsg = itAudio->second;
	listChatMsg.clear();
	return TRUE;
}

BOOL AudioMessageMananger::stopPlayingAnimate()
{
	SessionDialog* pSessionDialog = SessionDialogManager::getInstance()->findSessionDialogBySId(m_sPlayingSessionID);
	if (pSessionDialog)
	{
		pSessionDialog->StopPlayingAnimate(m_sPlayingAID);
		return TRUE;
	}
	return FALSE;
}

BOOL AudioMessageMananger::startPlayingAnimate(IN const std::string& sToPlayAID)
{
	SessionDialog* pSessionDialog = SessionDialogManager::getInstance()->findSessionDialogBySId(m_sPlayingSessionID);
	if (pSessionDialog)
	{
		pSessionDialog->StartPlayingAnimate(sToPlayAID);
		return TRUE;
	}
	return FALSE;
}

