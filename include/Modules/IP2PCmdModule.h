/*******************************************************************************
 *  @file      IP2PCmdModule.h 2014\8\18 13:40:21 $
 *  @author    快刀<kuaidao@mogujie.com>
 *  @brief     客户端与客户端之间传输数据，如正在输入、抖动
 ******************************************************************************/

#ifndef IP2PCMDMODULE_A5B625AF_2FEF_4DDC_9A60_BC8E12EC25A1_H__
#define IP2PCMDMODULE_A5B625AF_2FEF_4DDC_9A60_BC8E12EC25A1_H__

#include "GlobalDefine.h"
#include "TTLogic/IModule.h"
#include "Modules/ModuleDll.h"
/******************************************************************************/
NAMESPACE_BEGIN(module)
//KEYID
enum
{
	KEY_P2PCMD_NOTIFY		= MODULE_ID_P2PCMD << 16 | 1,		//收到P2P消息通知
};
enum	//P2P消息类型枚举
{
	TAG_P2PCMD_SHAKEWINDOW	= 1, //震屏
	TAG_P2PCMD_WRITING		= 2, //正在输入
	TAG_P2PCMD_INNERMSG		= 3, //内网推送的消息
};
//TAGID
enum
{
	TAG_P2PCMD_SHAKEWINDOW_NOTIFY = TAG_P2PCMD_SHAKEWINDOW << 16 | 1,
};

enum
{
	TAG_P2PCMD_WRITING_NOTIFY = TAG_P2PCMD_WRITING << 16 | 1,
	TAG_P2PCMD_STOP_WRITING_NOTIFY = TAG_P2PCMD_WRITING << 16 | 2,

};
/**
 * The class <code>客户端与客户端之间传输数据，如正在输入、抖动</code> 
 *
 */
class IP2PCmdModule : public logic::IPduAsyncSocketModule
{
public:
    /** @name Constructors and Destructor*/

    //@{
    /**
     * Constructor 
     */
	IP2PCmdModule()
	{
		m_moduleId = MODULE_ID_P2PCMD;
	}
    //@}

public:
	virtual BOOL tcpSendShakeWindowCMD(IN std::string sToID) = 0;
	virtual void tcpSendWritingCMD(IN std::string sToID, IN const BOOL bWriting) = 0;
};

MODULE_API IP2PCmdModule* getP2PCmdModule();

NAMESPACE_END(module)
/******************************************************************************/
#endif// IP2PCMDMODULE_A5B625AF_2FEF_4DDC_9A60_BC8E12EC25A1_H__
