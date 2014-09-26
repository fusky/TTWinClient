/*******************************************************************************
 *  @file      IUserListModule.h 2014\8\6 15:25:06 $
 *  @author    快刀<kuaidao@mogujie.com>
 *  @brief     组织架构相关模块
 ******************************************************************************/

#ifndef IUSERLISTMODULE_FDBD79FF_2F63_4E0A_8265_A001E2EB5182_H__
#define IUSERLISTMODULE_FDBD79FF_2F63_4E0A_8265_A001E2EB5182_H__

#include "GlobalDefine.h"
#include "TTLogic/IModule.h"
#include "Modules/ModuleDll.h"
#include "Modules/IMiscModule.h"
#include "Modules/IUserListModule.h"
#include "utility/utilStrCodeAPI.h"
#include "src/base/ImPduClient.h"
#include <string>
#include <list>
/******************************************************************************/
#define		PREFIX_GRAY_AVATAR		_T("gray_")

NAMESPACE_BEGIN(module)
//KEYID
enum
{
	KEY_USERLIST_UPDATE_DEPARTMENTLIST		=	MODULE_ID_USERLIST << 16 | 1,			//成功获取到组织架构信息
	KEY_USERLIST_UPDATE_RECENTLISTLIST		=   MODULE_ID_USERLIST << 16 | 2,			//成功获取最近联系人
	KEY_USERLIST_UPDATE_NEWUSESADDED		=	MODULE_ID_USERLIST << 16 | 3,			//新用户更新
	KEY_USERLIST_DOWNAVATAR_SUCC			=	MODULE_ID_USERLIST << 16 | 4,			//头像下载成功通知
	KEY_USERLIST_ALLUSERLINESTATE			=	MODULE_ID_USERLIST << 16 | 5,			//列表所有用户在线状态通知
	KEY_USERLIST_USERLINESTATE				=	MODULE_ID_USERLIST << 16 | 5,			//单个用户在线状态通知
};

/**
* The class <code>部门信息定义</code>
*
*/
struct MODULE_API DepartmentEntity
{
	std::string				dId;				//部门ID
	std::string				parentDepartId;		//上级部门名称
	std::string				leaderId;			//直接领导的id
	CString					title;				//部门名称
	CString					description;		//部门描述
	UInt32					status;				//部门状态  0:正常 1:删除
	std::list<std::string>	members;			//成员列表
};
typedef std::vector<DepartmentEntity>	DepartmentVec;

class MODULE_CLASS UserInfoEntity
{
public:
	UserInfoEntity();

public:
	CString getRealName();
	/**
	* 根据在线状态获取头像
	*
	* @return  std::string 头像路径
	* @exception there is no any exception to throw.
	*/
	std::string getAvatarPath();
	/**
	 * 获取纯头像，没有在线状态的区分
	 *
	 * @return  std::string 头像路径
	 * @exception there is no any exception to throw.
	 */	
	std::string getAvatarPathWithoutOnlineState();

	BOOL isOnlne()const;
public:
	UInt8           onlineState;            //用户在线状态  	USER_STATUS_ONLINE 	= 1,USER_STATUS_OFFLINE	= 2,USER_STATUS_LEAVE	= 3,
	CString         csName;                 //用户名
	CString         csNickName;             //用户昵称
	std::string     avatarUrl;              //用户头像地址
	std::string     avatarLocalPath;        //头像下载成功后的存储路径
	std::string     avatarGrayLocalPath;    //灰度处理之后的头像存储路径
	std::string     sId;                    //公司员工id
	std::string		dId;					//部门id
	UInt32			updated;				//最后一次消息更新时间
};
typedef std::map<string, UserInfoEntity>    UserInfoEntityMap;
typedef std::vector<std::string>			UserInfoEntityVec;

/**
 * The class <code>组织架构相关模块</code> 
 *
 */
class MODULE_API IUserListModule : public logic::IPduAsyncSocketModule
{
public:
    /** @name Constructors and Destructor*/

    //@{
    /**
     * Constructor 
     */
	IUserListModule()
	{
		m_moduleId = MODULE_ID_USERLIST;
	}
    //@}

public:
	/**
	* 所有部门信息
	*
	* @return  const module::DepartmentVec&
	* @exception there is no any exception to throw.
	*/
	virtual const DepartmentVec& getAllDepartments() = 0;
	virtual BOOL getDepartmentByDId(const std::string& dId, DepartmentEntity& entity) = 0;
	/**
	* 获取所有人的信息
	*
	* @param   module::UserInfoEntityMap & MapUsers
	* @return  void
	* @exception there is no any exception to throw.
	*/
	virtual void getAllUsersInfo(module::UserInfoEntityMap& MapUsers)const = 0;
	/**
	* 根据用户ID获取个人信息
	*
	* @param   IN std::string sid
	* @param   OUT module::UserInfoEntity & userInfo
	* @return  BOOL
	* @exception there is no any exception to throw.
	*/
	virtual BOOL getUserInfoBySId(IN std::string sid, OUT module::UserInfoEntity& userInfo) = 0;
	/**
	* 获取最近联系人列表，有序的
	*
	* @param   OUT module::UserInfoEntityVec & VecUsers
	* @return  void
	* @exception there is no any exception to throw.
	*/
	virtual void getUserListInfoVec(OUT module::UserInfoEntityVec& VecUsers) = 0;				 //最近联系人列表
	/**
	* 清空获取的最近联系人列表
	*
	* @return  void
	* @exception there is no any exception to throw.
	*/
	virtual void releaseUserListInfoVec() = 0;

	/**
	* 清空所有人列表
	*
	* @return  void
	* @exception there is no any exception to throw.
	*/
	virtual void removeAllListInfo() = 0;
	/**
	* 获取自己的信息
	*
	* @return  UInt8
	* @exception there is no any exception to throw.
	*/
	virtual UInt8 getMyLineStatus()= 0;

	/**
	* 获取某个人的在线状态
	*
	* @param   IN const std::string & sId
	* @return  void
	* @exception there is no any exception to throw.
	*/
	virtual void tcpGetUserOnlieStatus(IN const std::string& sId) = 0;
	/**
	* 获取一批人的在线列表，如果列表人数过多，要分批获取
	*
	* @param   const module::VecUserInfoEntity & VecId
	* @return  void
	* @exception there is no any exception to throw.
	*/
	virtual void tcpGetUserOnlieStatus(const module::UserInfoEntityVec& VecId) = 0;
	/**
	* 获取所有人的在线状态
	*
	* @param   void
	* @return  void
	* @exception there is no any exception to throw.
	*/
	virtual void tcpGetAllUserOnlieStatus(void) = 0;
	/**
	* tcp获取个人信息
	*
	* @param   IN const std::string & sId
	* @return  void
	* @exception there is no any exception to throw.
	*/
	virtual void tcpGetUserInfo(IN const std::string& sId) = 0;
	/**
	* tcp获取某一批人的信息
	*
	* @param   IN const module::VecUserInfoEntity & VecUnKnowUserInfo
	* @return  void
	* @exception there is no any exception to throw.
	*/
	virtual void tcpGetUserInfoList(IN const module::UserInfoEntityVec& VecUnKnowUserInfo) = 0;
	/**
	* 获取缺省的个人头像
	*
	* @return  CString
	* @exception there is no any exception to throw.
	*/
	virtual CString getDefaultAvatartPath() = 0;
	/**
	* 获取自己的信息
	*
	* @return BOOL
	* @exception there is no any exception to throw.
	*/
	virtual BOOL getMyInfo(OUT module::UserInfoEntity& myInfo) = 0;

	/**
	 * 根据缩写或者全拼获取搜索的人的名称
	 *
	 * @param   IN const CString & sShortName
	 * @param   OUT std::vector<CString> & nameList
	 * @return  void
	 * @exception there is no any exception to throw.
	 */
	virtual void getSearchUserNameListByShortName(IN const CString& sShortName, OUT	module::UserInfoEntityVec& nameList) = 0;
	/**
	 * 这个接口目前就是给离线消息、创建登陆者信息用的,创建不考虑替换，如果存在sId了就失败处理
	 *
	 * @param   const std::string & sId
	 * @param   const module::UserInfoEntity & info
	 * @return  BOOL
	 * @exception there is no any exception to throw.
	 */	
	virtual BOOL createUserInfo(const std::string& sId, const UserInfoEntity& info) = 0;
};

MODULE_API IUserListModule* getUserListModule();

NAMESPACE_END(module)
/******************************************************************************/
#endif// IUSERLISTMODULE_FDBD79FF_2F63_4E0A_8265_A001E2EB5182_H__
