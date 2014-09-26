/******************************************************************************* 
 *  @file      log.cpp 2014\7\16 10:57:01 $
 *  @author    ¿ìµ¶<kuaidao@mogujie.com>
 *  @brief   
 ******************************************************************************/

#include "stdafx.h"
#include "utility/log.h"
#include "utility/utilCommonAPI.h"
#include "utility/utilStrCodeAPI.h"
#include "GlobalConfig.h"

/******************************************************************************/
NAMESPACE_BEGIN(util)
namespace
{
#ifdef _DEBUG
	#define LOG_LEVEL LOG_DETAIL
#else
	#define LOG_LEVEL LOG_INFO
#endif // _DEBUG
}

Log* Log::getInstance()
{
	CString cfgPath = util::getAppPath() + _T("log.ini");
	Int8 settingLevel = ::GetPrivateProfileInt(_T("LOG"), _T("level"), LOG_LEVEL, cfgPath);

	static Log logInst(UTIL_LOG_APPFILE, settingLevel, 1024 * 1024);
	return &logInst;
}

// -----------------------------------------------------------------------------
//  log: Public, Constructor

Log::Log(CString sLogName, Int8 level, DWORD nMaxFileSize)
:m_nLogLevel(level),
m_dwFileLength(nMaxFileSize)
{
	char* chFileData = NULL;
	CString m_sFilePath = util::getAppPath() + sLogName;

	try
	{
		m_bFileOpen = m_file.Open(m_sFilePath, CFile::modeReadWrite |
			CFile::modeNoTruncate | CFile::modeCreate | CFile::shareDenyNone);

		DWORD dwFileLen = (DWORD)m_file.GetLength();
		if (dwFileLen >= m_dwFileLength)
		{
			chFileData = new char[m_dwFileLength / 2];
			if (chFileData)
			{
				m_file.Seek(dwFileLen - m_dwFileLength / 2, CFile::begin);
				m_file.Read(chFileData, m_dwFileLength / 2);
				m_file.Close();
				m_bFileOpen = FALSE;

				CFile::Remove(m_sFilePath);

				m_bFileOpen = m_file.Open(m_sFilePath, CFile::modeReadWrite |
					CFile::modeNoTruncate | CFile::modeCreate | CFile::shareDenyNone);

				if (m_bFileOpen)
				{
					m_file.Write(chFileData, m_dwFileLength / 2);
					m_file.Flush();
				}
				delete[] chFileData;
				chFileData = NULL;
			}
		}
	}
	catch (CException* e) {
		e->Delete();
	}
	catch (...) {

	}
}

// -----------------------------------------------------------------------------
//  log: Public, Destructor

Log::~Log()
{
	if (m_bFileOpen)
	{
		try
		{
			m_file.Close();
		}
		catch (CException* e) {
			e->Delete();
		}
		catch (...) {

		}
	}
}

BOOL Log::logging(int level, const TCHAR * format, ...)
{
	if (level < m_nLogLevel)
		return TRUE;

	CTime tNow = CTime::GetCurrentTime();
	TCHAR buffer[64];
	CString csOutText;

	va_list args;
	va_start(args, format);
	csOutText.FormatV(format, args);
	va_end(args);

	_stprintf_s(buffer, _countof(buffer), _T(" P:%u T:%u] "), ::GetCurrentProcessId(), ::GetCurrentThreadId());

	csOutText = tNow.Format(_T("[%Y-%m-%d %H:%M:%S")) + buffer + csOutText + _T("\r\n");

#ifdef _DEBUG
	TRACE(csOutText);
#endif

	try
	{
		writeDequeData(csOutText, TRUE);
	}
	catch (CException* e) {
		e->Delete();
	}
	catch (...) {

	}

	return TRUE;
}

BOOL Log::logging(int level, BOOL flush, const TCHAR * format, ...)
{
	if (level < m_nLogLevel)
		return TRUE;

	CTime tNow = CTime::GetCurrentTime();
	TCHAR buffer[64];
	CString csOutText;

	va_list args;
	va_start(args, format);
	csOutText.FormatV(format, args);
	va_end(args);

	_stprintf_s(buffer, _countof(buffer), _T(" P:%u T:%u] "), ::GetCurrentProcessId(), ::GetCurrentThreadId());

	csOutText = tNow.Format(_T("[%Y-%m-%d %H:%M:%S")) + buffer + csOutText + _T("\r\n");

#ifdef _DEBUG
	TRACE(csOutText);
#endif

	try
	{
		writeDequeData(csOutText, flush);
	}
	catch (CException* e) {
		e->Delete();
	}
	catch (...) {

	}

	return TRUE;
}

BOOL Log::writeDequeData(CString &data, BOOL flush)
{
	TTAutoLock lock(&m_lock);
	BOOL res = false;
	std::string sdata;

	sdata = util::cStringToUtf8(data);

	if (m_bFileOpen)
	{
		try
		{
			m_file.SeekToEnd();
			m_file.Write(sdata.c_str(), (UINT)sdata.length());
			if (flush)
				m_file.Flush();
			res = TRUE;
		}
		catch (CException* e) {
			e->Delete();
		}
		catch (...) {

		}
	}

	return res;

}

NAMESPACE_END(util)
/******************************************************************************/