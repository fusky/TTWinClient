/******************************************************************************* 
 *  @file      SessionChatMsg.cpp 2014\7\25 22:59:44 $
 *  @author    ´ó·ð<dafo@mogujie.com>
 *  @brief   
 ******************************************************************************/

#include "stdafx.h"
#include "Modules/MessageEntity.h"
#include "Modules/ISysConfigModule.h"
/******************************************************************************/

// -----------------------------------------------------------------------------
//  SessionChatMsg: Public, Constructor

MessageEntity::MessageEntity()
: msgType(0)
, msgStatusType(0)
, msgRenderType(0)
, msgAudioTime(0)
, msgTime(0)
, msgAudioReaded(0)
, msgFromType(MESSAGETYPE_FROM_ERROR)
{

}

// -----------------------------------------------------------------------------
//  SessionChatMsg: Public, Destructor

MessageEntity::~MessageEntity()
{

}

BOOL MessageEntity::getSenderInfo(OUT CString& senderName, OUT std::string& senderAvatartPath)
{
	return FALSE;
}

BOOL MessageEntity::isFromGroupMsg() const
{
	return MESSAGETYPE_FROM_GROUP == msgFromType;
}

BOOL MessageEntity::isMySendMsg() const
{
	return (talkerSid == module::getSysConfigModule()->userID());
}

std::string MessageEntity::getOriginSessionId()
{
	if (MESSAGETYPE_FROM_FRIEND == msgFromType)
	{
		return sessionId;
	}
	else if (MESSAGETYPE_FROM_GROUP == msgFromType)
	{
		std::string::size_type len = sessionId.size();
		if (len > 6)
		{
			std::string groupMark = sessionId.substr(0, 6);
			if (groupMark == string("group_"))
			{
				std::string OriginalSID = sessionId.substr(6, len);
				return OriginalSID;
			}
			else
			{//error

			}
		}
		else
		{//error

		}
	}
	else
	{//error

	}
	return sessionId;
}

BOOL MessageEntity::isReaded() const
{
	return msgAudioReaded == 1;
}

BOOL MessageEntity::makeGroupSessionId()
{
	if (MESSAGETYPE_FROM_GROUP == msgFromType)
	{
		sessionId = "group_" + sessionId;
		return TRUE;
	}
	return FALSE;
}

/******************************************************************************/