/*******************************************************************************
 *  @file      ISessionModule.h 2014\7\27 10:06:08 $
 *  @author    快刀<kuaidao@mogujie.com>
 *  @brief   
 ******************************************************************************/

#ifndef ISESSIONMODULE_070C0321_0708_4487_8028_C1D8934B709D_H__
#define ISESSIONMODULE_070C0321_0708_4487_8028_C1D8934B709D_H__

#include "GlobalDefine.h"
#include "TTLogic/IModule.h"
#include "Modules/ModuleDll.h"
/******************************************************************************/
namespace DuiLib
{
	class CControlUI;
	class CPaintManagerUI;
}
NAMESPACE_BEGIN(module)

enum
{
	KEY_SESSION_NEWMESSAGE				= MODULE_ID_SEESION << 16 | 1,      //接收到消息，包括运行时消息、离线消息
	KEY_SESSION_OPENNEWSESSION			= MODULE_ID_SEESION << 16 | 2,      //通知打开一个新的会话
	KEY_SESSION_SENDMSG_TOOFAST			= MODULE_ID_SEESION << 16 | 3,      //发送消息太快
	KEY_SESSION_SENDMSG_FAILED			= MODULE_ID_SEESION << 16 | 4,      //发送消息失败
	KEY_SESSION_SHAKEWINDOW_MSG			= MODULE_ID_SEESION << 16 | 5,		//抖动主窗口
	KEY_SESSION_WRITING_MSG				= MODULE_ID_SEESION << 16 | 6,		//正在输入
	KEY_SESSION_STOPWRITING_MSG			= MODULE_ID_SEESION << 16 | 7,		//停止了正在输入
	KEY_SESSION_SENDMSGTIP_KEY			= MODULE_ID_SEESION << 16 | 8,		//发送消息键值改变

	TAG_SESSION_TRAY_STARTEMOT			= MODULE_ID_SEESION << 16 | 9,		//开启托盘图标闪烁
	TAG_SESSION_TRAY_STOPEMOT			= MODULE_ID_SEESION << 16 | 10,		//关闭托盘图标闪烁
	TAG_SESSION_TRAY_NEWMSGSEND			= MODULE_ID_SEESION << 16 | 11,		//发送了一个消息，最近联系人更新
	TAG_SESSION_TRAY_COPYDATA			= MODULE_ID_SEESION << 16 | 12,		//语音播放模块发过来数据通知
};

/**
 * The class <code>ISessionModule</code> 
 *
 */
class MODULE_API ISessionModule : public logic::IPduAsyncSocketModule
{
public:
    /** @name Constructors and Destructor*/

    //@{
    /**
     * Constructor 
     */
	ISessionModule()
	{
		m_moduleId = MODULE_ID_SEESION;
	}
	//@}

public:
	virtual DuiLib::CControlUI* createMainDialogControl(
		LPCTSTR pstrClass,DuiLib::CPaintManagerUI& paintManager) = 0;
	virtual void OnGroupUnreadMsgRespone(IN CImPdu* pdu) = 0;	

	/**@name 同步服务器时间*/
	//@{
	virtual UInt32 getTime()const = 0;
	virtual void setTime(UInt32 time) = 0;
	virtual void startSyncTimeTimer() = 0;
	//@}
};

MODULE_API ISessionModule* getSessionModule();

NAMESPACE_END(module)
/******************************************************************************/
#endif// ISESSIONMODULE_070C0321_0708_4487_8028_C1D8934B709D_H__
