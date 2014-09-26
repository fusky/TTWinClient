/******************************************************************************* 
 *  @file      HistoryMsgModule_Impl.cpp 2014\8\3 11:14:33 $
 *  @author    ¿ìµ¶<kuaidao@mogujie.com>
 *  @brief     
 ******************************************************************************/

#include "stdafx.h"
#include "MessageModule_Impl.h"
#include "utility/CppSQLite3.h"
#include "utility/utilStrCodeAPI.h"
#include "Modules/IMiscModule.h"

#include "ReceiveMsgManage.h"
#include "SendMsgManage.h"


/******************************************************************************/
namespace module
{
	IMessageModule* getMessageModule()
	{
		return (IMessageModule*)logic::GetLogic()->getModule(MODULE_ID_MESSAGE);
	}
}

// -----------------------------------------------------------------------------
//  HistoryMsgModule_Impl: Public, Constructor

MessageModule_Impl::MessageModule_Impl()
:m_pHistoryMSGDB(new CppSQLite3DB())
{

}

// -----------------------------------------------------------------------------
//  HistoryMsgModule_Impl: Public, Destructor

MessageModule_Impl::~MessageModule_Impl()
{
	delete m_pHistoryMSGDB;
}

void MessageModule_Impl::release()
{
	delete this;
}

logic::LogicErrorCode MessageModule_Impl::onUnLoadModule()
{
	_closeDB();

	return logic::LOGIC_OK;
}

void MessageModule_Impl::removeMessageBySId(const std::string& sId)
{
	ReceiveMsgManage::getInstance()->removeMessageBySId(sId);
}

BOOL MessageModule_Impl::pushMessageBySId(const std::string& sId, MessageEntity& msg)
{
	return ReceiveMsgManage::getInstance()->pushMessageBySId(sId, msg);
}

/******************************************************************************/