/*******************************************************************************
 *  @file      ITcpClientModule.h 2014\7\29 13:11:43 $
 *  @author    快刀<kuaidao@mogujie.com>
 *  @brief     客户端TCP网络长连接模块接口定义，对TcpClientScoket做一个适配
 ******************************************************************************/

#ifndef ITCPCLIENTMODULE_66DE1A78_4D88_4416_BFC6_F4F16832ECBB_H__
#define ITCPCLIENTMODULE_66DE1A78_4D88_4416_BFC6_F4F16832ECBB_H__

#include "TTLogic/IModule.h"
#include "GlobalDefine.h"
#include "TTLogicDll.h"
/******************************************************************************/
class   CImPdu;
NAMESPACE_BEGIN(logic)
//KEYID
enum
{
	KEY_TCPCLIENT_STATE		= MODULE_ID_TCPCLIENT << 16 | 1,	//TCP长连状态变更
};

/**
* The class <code>TcpClientSocket</code>
*
*/
struct ITcpClientSocketCallback
{
	virtual void onClose(int error) = 0;
	virtual void onReceiveData(const char* data, UInt32 size) = 0;
	virtual void onParseError() = 0;
	virtual void onConnectDone() = 0;
};

enum TCPCLIENT_STATE
{
	TCPCLIENT_STATE_OK = 0,
	TCPCLIENT_STATE_DISCONNECT
};

/**
 * The class <code>客户端TCP网络长连接模块，对TcpClientScoket做一个适配</code> 
 *
 */
class TTLOGIC_API ITcpClientModule : public logic::IModule
{
public:
    /** @name Constructors and Destructor*/

    //@{
    /**
     * Constructor 
     */
	ITcpClientModule()
	{
		m_moduleId = MODULE_ID_TCPCLIENT;
	}
    //@}

public:
	virtual BOOL create() = 0;
	virtual CImPdu* doLogin(CString &linkaddr, UInt16 port
		,CString& uName, std::string& pass) = 0;
	virtual void closeSocket() = 0;
	virtual void shutdown() = 0;

	/**
	* 发送协议包
	*
	* @return  void
	* @exception there is no any exception to throw.
	*/
	virtual void sendPacket(CImPdu* pdu) = 0;
	/**
	 * 客户端开启发送心跳包
	 *
	 * @return  void
	 * @exception there is no any exception to throw.
	 */
	
	virtual void startHeartbeat() = 0;
	virtual UInt8 getTcpClientNetState()const = 0;
};

TTLOGIC_API ITcpClientModule* getTcpClientModule();

NAMESPACE_END(logic)
/******************************************************************************/
#endif// ITCPCLIENTMODULE_66DE1A78_4D88_4416_BFC6_F4F16832ECBB_H__
