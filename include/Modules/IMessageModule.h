/*******************************************************************************
 *  @file      IHitoryMsgModule.h 2014\8\3 11:10:16 $
 *  @author    快刀<kuaidao@mogujie.com>
 *  @brief     历史消息DB相关接口
 ******************************************************************************/

#ifndef IHITORYMSGMODULE_D042F2F2_05B0_45E6_9746_344A76279AE8_H__
#define IHITORYMSGMODULE_D042F2F2_05B0_45E6_9746_344A76279AE8_H__

#include "GlobalDefine.h"
#include "TTLogic/IModule.h"
#include "Modules/ModuleDll.h"
#include "MessageEntity.h"
/******************************************************************************/
class MessageEntity;
class TransferFileEntity;
NAMESPACE_BEGIN(module)

/**
 * The class <code>历史消息DB相关接口</code> 
 *
 */
class MODULE_API IMessageModule : public logic::IModule
{
public:
    /** @name Constructors and Destructor*/

    //@{
    /**
     * Constructor 
     */
	IMessageModule()
	{
		m_moduleId = MODULE_ID_MESSAGE;
	}
    //@}

public:
	/**@name 历史消息相关*/
	//@{
	/**
	 * 打开消息数据库
	 *
	 * @return  BOOL
	 * @exception there is no any exception to throw.
	 */
	virtual BOOL openDB() = 0;
	/**
	 * 给会话累加历史消息计数器
	 *
	 * @param   const std::string & sId
	 * @param   UInt8 v
	 * @return  void
	 * @exception there is no any exception to throw.
	 */
	virtual void countMsgOffset(const std::string& sId, Int32 v) = 0;
	/**
	 * 清除会话历史消息计数器
	 *
	 * @param   const std::string & sId
	 * @return  void
	 * @exception there is no any exception to throw.
	 */
	virtual void clearMsgOffset(const std::string& sId) = 0;
	virtual BOOL sqlInsertHistoryMsg(IN MessageEntity& msg) = 0;
	virtual BOOL sqlGetHistoryMsg(IN std::string sId, IN UInt32 nMsgCount, OUT std::vector<MessageEntity>& msgList) = 0;
	virtual BOOL sqlBatchInsertHistoryMsg(IN std::list<MessageEntity>& msgList) = 0;
	//@}

	/**@name 文件传输相关*/
	//@{
	virtual BOOL sqlInsertFileTransferHistory(IN TransferFileEntity& fileInfo) = 0;
	virtual BOOL sqlGetFileTransferHistory(OUT std::vector<TransferFileEntity>& fileList) = 0;
	//@}

	/**@name 运行时消息相关*/
	//@{
	/**
	 * 断线重连的情况下要清理消息 
	 *
	 * @param   const std::string & sId
	 * @return  void
	 * @exception there is no any exception to throw.
	 */
	virtual void removeMessageBySId(const std::string& sId) = 0;
	virtual BOOL pushMessageBySId(const std::string& sId, MessageEntity& msg) = 0;
	//@}
};

MODULE_API IMessageModule* getMessageModule();

NAMESPACE_END(module)
/******************************************************************************/
#endif// IHITORYMSGMODULE_D042F2F2_05B0_45E6_9746_344A76279AE8_H__
