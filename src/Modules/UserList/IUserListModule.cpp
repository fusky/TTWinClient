/******************************************************************************* 
 *  @file      IUserListModule.cpp 2014\8\26 16:36:25 $
 *  @author    快刀<kuaidao@mogujie.com>
 *  @brief     
 ******************************************************************************/

#include "stdafx.h"
#include "Modules/IUserListModule.h"
#include "TTLogic/ITcpClientModule.h"

/******************************************************************************/
NAMESPACE_BEGIN(module)
UserInfoEntity::UserInfoEntity() 
:onlineState(USER_STATUS_OFFLINE)
{

}

CString UserInfoEntity::getRealName()
{
	if (!csNickName.IsEmpty())
	{
		return csNickName;
	}
	else if (!csName.IsEmpty())
	{
		return csName;
	}
	else
	{
		return util::stringToCString(sId);
	}
}

std::string UserInfoEntity::getAvatarPath()
{
	//获取当前登录状态
	UInt8 netState = logic::getTcpClientModule()->getTcpClientNetState();
	std::string path;
	if (logic::TCPCLIENT_STATE_OK == netState && USER_STATUS_OFFLINE != onlineState)
	{
		path = avatarLocalPath;
		if (path.empty())
		{
			std::string sDataPath = util::cStringToString(module::getMiscModule()->getDefaultAvatar());
			path = sDataPath + "default.png";
		}
	}
	else
	{
		path = avatarGrayLocalPath;
		if (path.empty())
		{
			std::string sDataPath = util::cStringToString(module::getMiscModule()->getDefaultAvatar());
			path = sDataPath + "gray_default.png";
		}
	}

	return path;
}

std::string UserInfoEntity::getAvatarPathWithoutOnlineState()
{
	std::string path = avatarLocalPath;
	if (path.empty())
	{
		std::string sDataPath = util::cStringToString(module::getMiscModule()->getDefaultAvatar());
		path = sDataPath + "default.png";
	}

	return path;
}

BOOL UserInfoEntity::isOnlne() const
{
	return USER_STATUS_ONLINE == onlineState ? TRUE : FALSE;
}

NAMESPACE_END(module)
/******************************************************************************/