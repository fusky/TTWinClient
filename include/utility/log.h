/*******************************************************************************
 *  @file      log.h 2014\7\16 10:56:56 $
 *  @author    快刀<kuaidao@mogujie.com>
 *  @brief   基础应用程序log功能
 ******************************************************************************/

#ifndef LOG_1E22546D_9B7E_46D1_966F_5288CBF10B72_H__
#define LOG_1E22546D_9B7E_46D1_966F_5288CBF10B72_H__

#include "utilityDll.h"
#include "GlobalDefine.h"
#include "TTAutoLock.h"
#include <string>
/******************************************************************************/
const UInt8 LOG_DETAIL	= 1;
const UInt8 LOG_DEBUG	= 2;
const UInt8 LOG_INFO	= 3;
const UInt8 LOG_ERROR	= 4;
const UInt8 LOG_FATAL	= 5;

NAMESPACE_BEGIN(util)

/**
 * The class <code>log</code> 
 *
 */
class UTILITY_CLASS Log
{
public:
    /** @name Constructors and Destructor*/

    //@{
    /**
     * Constructor 
     */
	Log(CString sLogName, Int8 level, DWORD nMaxFileSize);
    /**
     * Destructor
     */
    ~Log();
    //@}
	static Log* getInstance();

public:
	BOOL logging(int level, BOOL flush, const TCHAR * format, ...);
	BOOL logging(int level, const TCHAR * format, ...);

private:
	BOOL writeDequeData(CString &data, BOOL flush);

private:
	BOOL		m_bFileOpen;
	int			m_nLogLevel;
	DWORD		m_dwFileLength;
	CFile		m_file;
	CString		m_sFilePath;
	TTMutexLock	m_lock;
};

NAMESPACE_END(util)
/******************************************************************************/
#endif// LOG_1E22546D_9B7E_46D1_966F_5288CBF10B72_H__
