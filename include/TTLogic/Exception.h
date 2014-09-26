/*******************************************************************************
 *  @file      Exception.h 2012\9\10 16:25:53 $
 *  @author    ¿ìµ¶<kuaidao@mogujie.com>
 *  @brief   Âß¼­ÒýÇæÒì³£
 ******************************************************************************/

#ifndef EXCEPTION_CCAF75A2_A865_4870_A2C4_B4BE30FEC9AF_H__
#define EXCEPTION_CCAF75A2_A865_4870_A2C4_B4BE30FEC9AF_H__

#include "GlobalDefine.h"

/******************************************************************************/
NAMESPACE_BEGIN(logic)

struct Exception : public CSimpleException
{
public:
    Exception(int errorCode,LPCTSTR lpszMsg)
   :m_ErrorCode(errorCode)
   ,m_lpszMsg(lpszMsg)
    {
    }

public:
    int         m_ErrorCode;
    LPCTSTR     m_lpszMsg;
};

NAMESPACE_END(logic)
/******************************************************************************/
#endif// EXCEPTION_CCAF75A2_A865_4870_A2C4_B4BE30FEC9AF_H__