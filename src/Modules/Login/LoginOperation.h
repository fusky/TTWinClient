/*******************************************************************************
 *  @file      LoginOperation.h 2014\7\30 15:32:25 $
 *  @author    快刀<kuaidao@mogujie.com>
 *  @brief     
 ******************************************************************************/

#ifndef LOGINOPERATION_9610A313_DC31_429E_B9E9_09A34ABA8063_H__
#define LOGINOPERATION_9610A313_DC31_429E_B9E9_09A34ABA8063_H__

#include "TTLogic/IOperation.h"
#include "src/base/ImPduClient.h"

/******************************************************************************/
enum
{
	LOGIN_FAIL = -1,            //登陆失败
	LOGIN_OK = 0,               //登陆成功
	LOGIN_LOGINSVR_FAIL,		//登录登录服务器失败
	LOGIN_MSGSVR_FAIL,			//登陆消息服务器失败
	LOGIN_DB_INVALID,			//DB验证失败
	LOGIN_VERSION_TOOOLD,       //协议版本太旧了，已经不兼容，提示用户去官网下载最新版本
};

struct LoginParam
{
public:
	Int8            result = LOGIN_FAIL;
	UInt8           mySelectedStatus = USER_STATUS_ONLINE;
	UInt32          serverTime = 0;
	CString			csUserName;
	std::string		password;
};

/**
 * The class <code>LoginOperation</code> 
 *
 */
class LoginOperation : public logic::ICallbackOpertaion
{
public:
    /** @name Constructors and Destructor*/

    //@{
    /**
     * Constructor 
     */
	LoginOperation(logic::ICallbackHandler& callback, LoginParam& param);
    /**
     * Destructor
     */
    virtual ~LoginOperation();
    //@}

public:
	virtual void process();
	virtual void release();

private:
	LoginParam			m_loginParam;
};
/******************************************************************************/
#endif// LOGINOPERATION_9610A313_DC31_429E_B9E9_09A34ABA8063_H__
