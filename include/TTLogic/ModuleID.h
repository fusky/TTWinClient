/*******************************************************************************
 *  @file      ModuleID.h 2014\7\16 17:56:55 $
 *  @author    快刀<kuaidao@mogujie.com>
 *  @brief   
 ******************************************************************************/

#ifndef MODULEID_D64A8C53_C3BB_4041_A371_CD65CFE7DA37_H__
#define MODULEID_D64A8C53_C3BB_4041_A371_CD65CFE7DA37_H__

#include "GlobalDefine.h"
/******************************************************************************/

const UInt16 MODULE_REMOTE_APPBASE = 0x0000U;    //与远程交互的模块基础ID
const UInt16 MODULE_LOCAL_BASE = 0x4000U;        //客户端本地模块基础ID

enum
{
	MODULE_ID_NONE = 0,

	//与远程交互的模块基础I	
	MODULE_ID_LOGIN			= MODULE_REMOTE_APPBASE | 0x0001,      //登陆模块
	MODULE_ID_USERLIST		= MODULE_REMOTE_APPBASE | 0x0002,      //成员列表模块
	MODULE_ID_SEESION		= MODULE_REMOTE_APPBASE | 0x0003,      //会话模块
	MODULE_ID_P2PCMD		= MODULE_REMOTE_APPBASE | 0x0004,      //自定义P2P协议消息
	MODULE_ID_GROUPLIST		= MODULE_REMOTE_APPBASE | 0x0005,      //群模块
	MODULE_ID_FILETRANSFER	= MODULE_REMOTE_APPBASE | 0x0006,      //文件传输
	MODULE_ID_HTTPPOOL		= MODULE_REMOTE_APPBASE | 0x0007,      //HTTP线程池
	MODULE_ID_TCPCLIENT		= MODULE_REMOTE_APPBASE | 0x0008,      //连接模块

	//客户端本地模块基础ID
	MODULE_ID_DATABASE		= MODULE_LOCAL_BASE | 0x0001,      //本地数据存储模块
	MODULE_ID_MISC			= MODULE_LOCAL_BASE | 0x0002,      //一些比较杂的功能
	MODULE_ID_CAPTURE		= MODULE_LOCAL_BASE | 0x0003,      //截屏模块
	MODULE_ID_SYSCONFIG		= MODULE_LOCAL_BASE | 0x0004,      //系统配置信息
	MODULE_ID_MESSAGE		= MODULE_LOCAL_BASE | 0x0005,      //历史消息
	MODULE_ID_EMOTION		= MODULE_LOCAL_BASE | 0x0006,      //表情
	MODULE_ID_MENU			= MODULE_LOCAL_BASE | 0x0007,      //菜单
};

/******************************************************************************/
#endif// MODULEID_D64A8C53_C3BB_4041_A371_CD65CFE7DA37_H__
