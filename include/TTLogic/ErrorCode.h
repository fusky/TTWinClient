/*******************************************************************************
 *  @file      ErrorCode.h 2012\8\16 22:21:34 $
 *  @author    快刀<kuaidao@mogujie.com>
 *  @brief   Logic本地错误码的定义,通信协议的错误码定义在ProtErrorCode.h
 ******************************************************************************/

#ifndef ERRORCODE_016A9735_BF5C_425C_822A_C4E4680D3449_H__
#define ERRORCODE_016A9735_BF5C_425C_822A_C4E4680D3449_H__

#include "GlobalDefine.h"
/******************************************************************************/
NAMESPACE_BEGIN(logic)

typedef UInt32  LogicErrorCode;
//错误掩码，error code flag
const LogicErrorCode LOGIC_FLAG                 = 0x000000U; // 此码代表常规错误
const LogicErrorCode LOGIC_MODULE_FLAG         = 0x010000U; // 此码代表module产生的错误
const LogicErrorCode LOGIC_WORK_FLAG            = 0x020000U; // 此码代表后台任务opertaion,event产生的错误
const LogicErrorCode LOGIC_DOCUMENT_FLAH        = 0x040000U; // 此码代表文档机制产生的错误

//常规错误
const LogicErrorCode LOGIC_OK                               = LOGIC_FLAG | 0x00U;   //一切OK
const LogicErrorCode LOGIC_ALLOC_ERROR                      = LOGIC_FLAG | 0x01U;   //内存分配错误
const LogicErrorCode LOGIC_INVALID_HWND_ERROR               = LOGIC_FLAG | 0x02U;   //无效的窗口句柄
const LogicErrorCode LOGIC_ARGUMENT_ERROR                   = LOGIC_FLAG | 0x03U;   //逻辑参数错误
const LogicErrorCode LOGIC_FILE_OPEN_ERROR                  = LOGIC_FLAG | 0x04U;   //文件打开失败
const LogicErrorCode LOGIC_FILE_READ_ERROR                  = LOGIC_FLAG | 0x05U;   //文件读取失败
const LogicErrorCode LOGIC_FILE_WRITE_ERROR                 = LOGIC_FLAG | 0x06U;   //文件读取失败
const LogicErrorCode LOGIC_FILE_SYSTEM_ERROR                = LOGIC_FLAG | 0x07U;   //文件未知异常

//module 错误
const LogicErrorCode LOGIC_MODULE_LOAD_ERROR               = LOGIC_MODULE_FLAG | 0x01; //加载模块失败
const LogicErrorCode LOGIC_MODULE_HASONE_ERROR             = LOGIC_MODULE_FLAG | 0x02; //模块已经存在
const LogicErrorCode LOGIC_MODULE_INEXISTENCE_ERROR        = LOGIC_MODULE_FLAG | 0x03; //模块不存在

//opertaion event错误
const LogicErrorCode LOGIC_WORK_INTERNEL_ERROR              = LOGIC_WORK_FLAG | 0x01;   //worker内部错误
const LogicErrorCode LOGIC_WORK_POSTMESSAGE_ERROR           = LOGIC_WORK_FLAG | 0x02;   //event post消息失败
const LogicErrorCode LOGIC_WORK_PUSHOPERTION_ERROR          = LOGIC_WORK_FLAG | 0x03;   //opertaion push失败
const LogicErrorCode LOGIC_WORK_TIMER_INEXISTENCE_ERROR     = LOGIC_WORK_FLAG | 0x04;   //Timer不存在

//document错误


NAMESPACE_END(logic)
/******************************************************************************/
#endif// ERRORCODE_016A9735_BF5C_425C_822A_C4E4680D3449_H__