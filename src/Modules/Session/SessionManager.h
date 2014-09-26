/*******************************************************************************
 *  @file      SessionManager.h 2014\8\11 16:58:48 $
 *  @author    快刀<kuaidao@mogujie.com>
 *  @brief     会话信息管理
 ******************************************************************************/

#ifndef SESSIONMANAGER_863FBDB8_F00A_4A46_8F57_1AECBC9D277E_H__
#define SESSIONMANAGER_863FBDB8_F00A_4A46_8F57_1AECBC9D277E_H__

#include "GlobalDefine.h"
#include "utility/TTAutoLock.h"
#include <string>
#include <map>
#include <list>
/******************************************************************************/
class SessionDialog;
enum
{
	SESSION_ERRORTYPE = 0,
	SESSION_USERTYPE,		//个人会话
	SESSION_GROUPTYPE,		//群会话
};
class SessionEntity
{
public:
	SessionEntity();
	std::string getAvatarPath();
	UInt8 getOnlineState();
	CString getName();
	void SetUpdatedTimeByServerTime();

public:
	BOOL                                m_bBanGroupMsg;		//屏蔽群消息
	UInt8								m_sessionType;		//SESSION_USERTYPE / SESSION_GROUPTYPE
	time_t                              m_updatedTime;      //消息最后更新时间
	UInt32                              m_unReadMsgCount;   //消息数
	CString								m_csDesc;			//
	std::string                         m_sId;				//会话ID
};

/**
 * The class <code>会话信息管理</code> 
 *
 */
class SessionEntityManager
{
public:
	~SessionEntityManager();
	static SessionEntityManager* getInstance();

private:
	SessionEntityManager();

public:
	SessionEntity* createSessionEntity(const std::string& sId);
	SessionEntity* getSessionEntityBySId(IN const std::string& sId);
	BOOL removeSessionEntity(const std::string& sId);
	BOOL saveBanGroupMSGSetting(IN const std::string& sId, IN BOOL bBanMsg);//设置群消息屏蔽
private:
	UInt8 _getSessionType(IN const std::string& sID);
	BOOL _getBanGroupMSGSetting(IN const std::string& sId);
	void _removeAllSessionEntity();

private:
	util::TTFastLock							m_lock;
	std::map<std::string,SessionEntity*>		m_mapSessionEntity;
};

//已经打开的会话
class SessionDialogManager
{
public:
	SessionDialog* openSessionDialog(const std::string& sId);
	SessionDialog* findSessionDialogBySId(const std::string& sId);
	void closeSessionDialog(const std::string& sId);
	static SessionDialogManager* getInstance();

private:
	SessionDialogManager();

private:
	std::list<SessionDialog*>					m_lstSessionDialog;
};

/******************************************************************************/
#endif// SESSIONMANAGER_863FBDB8_F00A_4A46_8F57_1AECBC9D277E_H__
