/*******************************************************************************
 *  @file      IGroupListModule.h 2014\8\6 15:29:01 $
 *  @author    快刀<kuaidao@mogujie.com>
 *  @brief     群、讨论组相关模块
 ******************************************************************************/

#ifndef IGROUPLISTMODULE_3AD36DFC_4041_486A_A437_948E152517E8_H__
#define IGROUPLISTMODULE_3AD36DFC_4041_486A_A437_948E152517E8_H__

#include "GlobalDefine.h"
#include "TTLogic/IModule.h"
#include "Modules/ModuleDll.h"
#include "Modules/IUserListModule.h"
#include <list>
#include <string>
/******************************************************************************/
NAMESPACE_BEGIN(module)

enum
{
	KEY_GROUPLIST_UPDATE_GROUPLIST			= MODULE_ID_GROUPLIST << 16 | 1,	//成功获取到固定群信息
	KEY_GROUPLIST_UPDATE_GROUPDISCUSSLIST	= MODULE_ID_GROUPLIST << 16 | 2,	//成功获取到讨论组信息
	KEY_GROUPLIST_UPDATE_RECENTGROUPLIST	= MODULE_ID_GROUPLIST << 16 | 3,	//成功获取到所有的群和讨论组，更新最近联系人群列表
	KEY_GROUPLIST_UPDATE_NEWGROUPADDED		= MODULE_ID_GROUPLIST << 16 | 4,	//收到一条陌生群消息返货
	KEY_GROUPLIST_UPDATE_CREATNEWGROUP		= MODULE_ID_GROUPLIST << 16 | 5,	//创建一个讨论组返回

};


class GroupInfoEntity
{
public:
	string				gId;					//群ID
	std::string			avatarUrl;
	std::string			avatarLocalPath;             //头像下载成功后的存储路径
	CString				desc;					//群公告
	CString				csName;
	UInt32				type = 0;				//群类型：1，固定群 2,讨论组
	UInt32				groupUpdated = 0;		//最后一次更新消息时间
	std::list<string>	groupMemeberList;

public:
	std::string getAvatarPath()
	{
		std::string path = avatarLocalPath;
		if (path.empty())
		{
			std::string sDataPath = util::cStringToString(module::getMiscModule()->getDefaultAvatar());
			if (1 == type)
			{
				path = sDataPath + "Groups.png";
			}
			else
			{
				path = sDataPath + "DiscussionGroups.png";
			}
		}
		else
		{
			std::string sDownPath = util::cStringToString(module::getMiscModule()->getDownloadDir());
			path = sDownPath + avatarLocalPath;
		}

		return path;
	}
};

typedef std::map<std::string, GroupInfoEntity>       GroupInfoMap;//群列表
typedef std::vector<std::string>     GroupVec;//群列表ID,最近联系群
/**
 * The class <code>群、讨论组相关模块</code> 
 *
 */
class MODULE_API IGroupListModule : public logic::IPduAsyncSocketModule
{
public:
    /** @name Constructors and Destructor*/

    //@{
    /**
     * Constructor 
     */
	IGroupListModule()
	{
		m_moduleId = MODULE_ID_GROUPLIST;
	}
    //@}

public:
	/**
	* 获取所有群的信息，用于插入群列表
	*
	* @param   OUT module::GroupMap & Groups
	* @return  void
	* @exception there is no any exception to throw.
	*/
	virtual void getAllGroupListInfo(OUT module::GroupInfoMap& Groups) = 0;
	/**
	* 查询群信息
	*
	* @param   IN const std::string & sID
	* @param   OUT module::GroupInfoEntity & groupInfo
	* @return  BOOL
	* @exception there is no any exception to throw.
	*/
	virtual BOOL getGroupInfoBySId(IN const std::string& sID, OUT module::GroupInfoEntity& groupInfo) = 0;
	/**
	* 获取某个群的所有成员
	*
	* @param   IN const std::string & sID
	* @param   OUT module::VecUserInfoEntity & groupUserVec
	* @return  BOOL
	* @exception there is no any exception to throw.
	*/
	virtual BOOL getGroupUserVecBySId(IN const std::string& sID, OUT module::UserInfoEntityVec& groupUserVec) = 0;

	/**
	* 获取群列表ID 用户插入最近联系人中的群项
	*
	* @param   OUT module::GroupVec & groups
	* @return  void
	* @exception there is no any exception to throw.
	*/
	virtual void getGroupListVec(module::GroupVec& groups) = 0;
	/**
	* 清除之前获取的最近联系群列表
	*
	* @return  void
	* @exception there is no any exception to throw.
	*/
	virtual void releaseGroupListInfoVec() = 0;
	/**
	* 获取群新增人员
	*
	* @param   IN const std::string & sID
	* @param   OUT module::VecUserInfoEntity & AddedMemberVec
	* @return  BOOL
	* @exception there is no any exception to throw.
	*/
	virtual BOOL popAddedMemberVecBySId(IN const std::string& sID, OUT module::UserInfoEntityVec& AddedMemberVec) = 0;
	/**
	* 获取群刚踢掉的人员
	*
	* @param   IN const std::string & sID
	* @param   OUT module::VecUserInfoEntity & RemovedMemberVec
	* @return  BOOL
	* @exception there is no any exception to throw.
	*/
	virtual BOOL popRemovedMemberVecBySId(IN const std::string& sID, OUT module::UserInfoEntityVec& RemovedMemberVec) = 0;

	/**
	* 获取群离线消息
	*
	* @return  BOOL
	* @exception there is no any exception to throw.
	*/
	virtual BOOL tcpGetGroupOfflineMsg() = 0;

	/**
	* 获取默认的群头像信息
	*
	* @return  CString
	* @exception there is no any exception to throw.
	*/
	virtual CString getDefaultAvatartPath() = 0;

	/**
	* 删除离线群消息的群
	*
	* @param   const std::string & sid
	* @return  BOOL
	* @exception there is no any exception to throw.
	*/
	virtual BOOL DeleteGroupIDFromVecOfflineMsgGroup(const std::string& sid) = 0;

	/**
	* 获取新的群成员
	*
	* @param   IN const std::string & groupId
	* @return  void
	* @exception there is no any exception to throw.
	*/
	virtual void tcpGetGroupInfo(IN const std::string& groupId) = 0;//获取新群的信息

	virtual void onCreateDiscussionGrpDialog(const std::string& currentSessionId) = 0;

	virtual void GetSearchGroupNameListByShortName(IN const CString& sShortName, OUT module::GroupVec & gidList) = 0;
};

MODULE_API IGroupListModule* getGroupListModule();

NAMESPACE_END(module)
/******************************************************************************/
#endif// IGROUPLISTMODULE_3AD36DFC_4041_486A_A437_948E152517E8_H__
