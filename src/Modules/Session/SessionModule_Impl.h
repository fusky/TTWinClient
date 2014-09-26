/*******************************************************************************
 *  @file      SessionModule_Impl.h 2014\7\27 10:10:59 $
 *  @author    快刀<kuaidao@mogujie.com>
 *  @brief   
 ******************************************************************************/

#ifndef SESSIONMODULE_IMPL_414A6CFB_C817_43C4_9C73_7E965E7317C7_H__
#define SESSIONMODULE_IMPL_414A6CFB_C817_43C4_9C73_7E965E7317C7_H__

#include "Modules/ISessionModule.h"
/******************************************************************************/
class SyncTimeTimer;

/**
 * The class <code>SessionModule_Impl</code> 
 *
 */
class MessageEntity;
class SessionModule_Impl final : public module::ISessionModule
{
public:
    /** @name Constructors and Destructor*/

    //@{
    /**
     * Constructor 
     */
	SessionModule_Impl();
    /**
     * Destructor
     */
    virtual ~SessionModule_Impl();
    //@}
	virtual void release();
	virtual void onPacket(std::auto_ptr<CImPdu> pdu);

public:
	virtual DuiLib::CControlUI* createMainDialogControl(
		LPCTSTR pstrClass, DuiLib::CPaintManagerUI& paintManager);
	virtual void OnGroupUnreadMsgRespone(IN CImPdu* pdu);

	/**@name 同步服务器时间*/
	//@{
	virtual UInt32 getTime()const;
	virtual void setTime(UInt32 time);
	virtual void startSyncTimeTimer();
	//@}

private:
	/**@name 服务器端拆包*/
	//@{
	void _sessionMsgResponse(CImPdu* pdu);
	void _sessionMsgACK(CImPdu* pdu);
	void _sessionMsgTimeResponse(CImPdu* pdu);
	void _sessionMsgUnreadCntResponse(CImPdu* pdu);
	void _sessionMsgUnreadMsgResponse(CImPdu* pdu);
	//@}
	BOOL _checkMsgFromStranger(IN MessageEntity msg);//消息来源的ID是存在当前会话ID列表中，不存在，则要去获取
	BOOL _banGroupMSG(IN MessageEntity msg);//群消息屏蔽

private:
	SyncTimeTimer*              m_pSyncTimer;
};
/******************************************************************************/
#endif// SESSIONMODULE_IMPL_414A6CFB_C817_43C4_9C73_7E965E7317C7_H__
