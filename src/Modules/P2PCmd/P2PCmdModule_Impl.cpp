/******************************************************************************* 
 *  @file      P2PCmdModule_Impl.cpp 2014\8\18 13:45:09 $
 *  @author    快刀<kuaidao@mogujie.com>
 *  @brief     
 ******************************************************************************/

#include "stdafx.h"
#include "P2PCmdModule_Impl.h"
#include "json/reader.h"
#include "json/writer.h"
#include "src/base/ImPduClient.h"
#include "Modules/IUserListModule.h"
#include "Modules/ISysConfigModule.h"
#include "Modules/ISessionModule.h"
#include "TTLogic/ITcpClientModule.h"
#include "utility/Multilingual.h"
#include "utility/utilStrCodeAPI.h"
#include "../Message/ReceiveMsgManage.h"
/******************************************************************************/
namespace module
{
	module::IP2PCmdModule* getP2PCmdModule()
	{
		return (module::IP2PCmdModule*)logic::GetLogic()->getModule(MODULE_ID_P2PCMD);
	}
}

// -----------------------------------------------------------------------------
//  P2PCmdModule_Impl: Public, Constructor

P2PCmdModule_Impl::P2PCmdModule_Impl()
{

}

// -----------------------------------------------------------------------------
//  P2PCmdModule_Impl: Public, Destructor

P2PCmdModule_Impl::~P2PCmdModule_Impl()
{

}

void P2PCmdModule_Impl::release()
{
	delete this;
}

void P2PCmdModule_Impl::onPacket(std::auto_ptr<CImPdu> pdu)
{
	CImPdu* pPdu = pdu.get();
	PTR_VOID(pPdu);
	switch (pdu->GetCommandId())
	{
	case CID_SWITCH_P2P_CMD:
		_p2pCmdNotifyResponse(pPdu);
		break;
	default:
		return;
	}
}

void P2PCmdModule_Impl::_p2pCmdNotifyResponse(CImPdu* pdu)
{
	CImPduClientP2PCmdMsg* pPduP2pMsg = (CImPduClientP2PCmdMsg*)pdu;
	std::string jsonMsgData((char*)pPduP2pMsg->GetCmdMsgData(), pPduP2pMsg->GetCmdMsgLen());
	std::string sFromId(pPduP2pMsg->GetFromId(), pPduP2pMsg->GetFromIdLen());
	UINT32 nServiceID = 0;
	UINT32 nCmdID = 0;
	_parseMsgJsonData(jsonMsgData, nServiceID, nCmdID);

	if (module::TAG_P2PCMD_SHAKEWINDOW == nServiceID && module::TAG_P2PCMD_SHAKEWINDOW_NOTIFY == nCmdID)
	{
		//窗口抖动生成一条系统消息，提示用户
		module::UserInfoEntity userInfo;
		if (module::getUserListModule()->getUserInfoBySId(sFromId, userInfo))
		{
			CString csTipFormat = util::getMultilingual()->getStringViaID(_T("STRID_SESSIONMODULE_SHAKEWINDOW_TIP"));
			CString csTip;
			MessageEntity msg;
			csTip.Format(csTipFormat, userInfo.getRealName());
			msg.content = util::cStringToString(csTip);
			msg.sessionId = sFromId;
			msg.talkerSid = module::getSysConfigModule()->userID();
			msg.msgRenderType = MESSAGE_RENDERTYPE_SYSTEMTIPS;
			msg.msgFromType = MESSAGETYPE_FROM_FRIEND;
			msg.msgTime = (UInt32)time(0);
			ReceiveMsgManage::getInstance()->pushMessageBySId(msg.sessionId, msg);
			logic::GetLogic()->asynNotifyObserver(module::KEY_SESSION_NEWMESSAGE, sFromId);	//收到屏幕抖动消息提示
		}

		logic::GetLogic()->asynNotifyObserver(module::KEY_SESSION_SHAKEWINDOW_MSG,sFromId);//抖动窗口
	}
	else if (module::TAG_P2PCMD_WRITING == nServiceID)
	{
		if (module::TAG_P2PCMD_WRITING_NOTIFY == nCmdID)
		{
			logic::GetLogic()->asynNotifyObserver(module::KEY_SESSION_WRITING_MSG, sFromId);
		}
		else if (module::TAG_P2PCMD_STOP_WRITING_NOTIFY == nCmdID)
		{
			logic::GetLogic()->asynNotifyObserver(module::KEY_SESSION_STOPWRITING_MSG, sFromId);
		}
	}
}
void P2PCmdModule_Impl::_parseMsgJsonData(IN std::string sJsonData, OUT UInt32& nServiceID, OUT UInt32& nCmdID)
{
	Json::Reader reader;
	Json::Value root;

	if (!reader.parse(sJsonData, root))
		return;
	nServiceID = (root.get("ServiceID", "")).asUInt();
	nCmdID = (root.get("CmdID", "")).asUInt();
}

BOOL P2PCmdModule_Impl::tcpSendShakeWindowCMD(IN std::string sToID)
{
	std::string fromId = module::getSysConfigModule()->userID();
	logic::GetLogic()->asynNotifyObserver(module::KEY_SESSION_SHAKEWINDOW_MSG, fromId);
	std::string cmdMsgData;
	_makeJsonData(module::TAG_P2PCMD_SHAKEWINDOW, module::TAG_P2PCMD_SHAKEWINDOW_NOTIFY, "shakewindow", cmdMsgData);

	logic::GetLogic()->pushBackOperationWithLambda(
		[=]()
	{
		CImPduClientP2PCmdMsg pduP2PMsg(0, fromId.c_str()
			, sToID.c_str(), cmdMsgData.size(), (uchar_t*)cmdMsgData.c_str());
		logic::getTcpClientModule()->sendPacket(&pduP2PMsg);
	}
		);

	return TRUE;
}

void P2PCmdModule_Impl::tcpSendWritingCMD(IN std::string sToID, IN const BOOL bWriting)
{
	std::string fromId = module::getSysConfigModule()->userID();
	std::string cmdMsgData;
	if (bWriting)
	{
		_makeJsonData(module::TAG_P2PCMD_WRITING, module::TAG_P2PCMD_WRITING_NOTIFY, "writing", cmdMsgData);
	}
	else
	{
		_makeJsonData(module::TAG_P2PCMD_WRITING, module::TAG_P2PCMD_STOP_WRITING_NOTIFY, "stop writing", cmdMsgData);
	}

	logic::GetLogic()->pushBackOperationWithLambda(
		[=]()
	{
		CImPduClientP2PCmdMsg pduP2PMsg(0, fromId.c_str()
			, sToID.c_str(), cmdMsgData.size(), (uchar_t*)cmdMsgData.c_str());
		logic::getTcpClientModule()->sendPacket(&pduP2PMsg);
	}
	);
}

void P2PCmdModule_Impl::_makeJsonData(IN UINT32 nServiceID, IN UINT32 nCmdID, IN std::string sContent, OUT std::string& sJsonData)
{
	Json::Value root;
	root["ServiceID"] = nServiceID;
	root["CmdID"] = nCmdID;
	root["Content"] = sContent;

	Json::FastWriter fstWrite;
	sJsonData = fstWrite.write(root);
}

/******************************************************************************/