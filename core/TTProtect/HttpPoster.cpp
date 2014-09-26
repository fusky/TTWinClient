/******************************************************************************* 
 *  @file      HttpPoster.cpp 2014\9\1 12:02:52 $
 *  @author    快刀<kuaidao@mogujie.com>
 *  @brief     
 ******************************************************************************/

#include "StdAfx.h"
#include "Parameters.h"
#include "HttpPoster.h"
#include "utility/utilStrCodeAPI.h"
#include <atlutil.h>
/******************************************************************************/

using namespace _internal;

#pragma comment (lib, "WinInet.lib")

static const TCHAR* MULTI_PART_BOUNDARY = _T("----79FC950525714d54928DECF95B7B4255");

CHttpPoster::CHttpPoster()
: m_InetSession(::InternetOpen(_T("HttpPoster"), INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0))
, m_hConnect(NULL)
, m_hRequest(NULL)
, m_hFile(INVALID_HANDLE_VALUE)
{

}

CHttpPoster::~CHttpPoster()
{
	if (INVALID_HANDLE_VALUE != m_hFile)
		::CloseHandle(m_hFile);
}

BOOL CHttpPoster::PostFile(LPCTSTR lpszUrl, LPCTSTR lpszFileName, LPCTSTR lpszUserName, LPCTSTR lpszPassword)
{
	m_bCancelled = false;
	m_Url = lpszUrl;
	m_FileName = lpszFileName;
	m_hFile = INVALID_HANDLE_VALUE;

	CrackUrl(lpszUserName, lpszPassword);
	OpenFile();
	BeginRequest();
	SetHeaders();
	TestRequest();
	SendFile();

	::CloseHandle(m_hFile);
	m_hFile = INVALID_HANDLE_VALUE;
	m_hRequest.Close();
	m_hConnect.Close();
	return TRUE;
}

void CHttpPoster::Cancel()
{
	m_bCancelled = TRUE;
	m_hRequest.Close();
	m_hConnect.Close();
}

void CHttpPoster::CrackUrl(LPCTSTR lpszUserName, LPCTSTR lpszPassword)
{
	if (m_bCancelled)
		return;
	URL_COMPONENTS UrlComps;
	memset(&UrlComps, 0, sizeof(UrlComps));
	UrlComps.dwStructSize = sizeof(UrlComps);
	static const int MAX_STRING_SIZE = 1024;
	UrlComps.dwHostNameLength = MAX_STRING_SIZE;
	UrlComps.dwUrlPathLength = MAX_STRING_SIZE;
	UrlComps.dwUserNameLength = MAX_STRING_SIZE;
	UrlComps.dwPasswordLength = MAX_STRING_SIZE;
	UrlComps.dwExtraInfoLength = MAX_STRING_SIZE;
	UrlComps.lpszHostName = m_HostName.GetBuffer(UrlComps.dwHostNameLength);
	UrlComps.lpszUrlPath = m_HostPath.GetBuffer(UrlComps.dwUrlPathLength);
	UrlComps.lpszUserName = m_UserName.GetBuffer(UrlComps.dwUserNameLength);
	UrlComps.lpszPassword = m_Password.GetBuffer(UrlComps.dwPasswordLength);
	UrlComps.lpszExtraInfo = m_ExtraInfo.GetBuffer(UrlComps.dwExtraInfoLength);
	BOOL bCracked = ::InternetCrackUrl(m_Url, m_Url.GetLength(), 0, &UrlComps);
	CheckError(TRUE == bCracked, _T("crashurl error"), GetLastError());

	m_UrlScheme = UrlComps.nScheme;
	m_Port = UrlComps.nPort;
	m_HostName.ReleaseBuffer();
	m_HostPath.ReleaseBuffer();
	m_UserName.ReleaseBuffer();
	m_Password.ReleaseBuffer();
	m_ExtraInfo.ReleaseBuffer();

	if (NULL != lpszUserName && 0 != lpszUserName[0])
		m_UserName = lpszUserName;
	if (NULL != lpszPassword && 0 != lpszPassword[0])
		m_Password = lpszPassword;
}

void CHttpPoster::OpenFile()
{
	if (m_bCancelled)
		return;
	m_hFile = ::CreateFile(m_FileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	CheckError(INVALID_HANDLE_VALUE != m_hFile, _T("CreateFile failed. Error: %d."), GetLastError());
}

void CHttpPoster::BeginRequest()
{
	if (m_bCancelled)
		return;
	HINTERNET hConn = ::InternetConnect(m_InetSession, m_HostName, m_Port, m_UserName, m_Password, INTERNET_SERVICE_HTTP, 0, (DWORD)this);
	CheckError(NULL != hConn, _T("InternetConnect failed. Error: %d."), GetLastError());
	m_hConnect.Attach(hConn);

	DWORD dwOpenRequestFlag = INTERNET_SCHEME_HTTPS == m_UrlScheme ? INTERNET_FLAG_SECURE : 0;
	HINTERNET hReq = ::HttpOpenRequest(m_hConnect, _T("POST"), m_HostPath + m_ExtraInfo, NULL, NULL, NULL, dwOpenRequestFlag, (DWORD)this);
	CheckError(NULL != hReq, _T("HttpOpenRequest failed. Error: %d."), GetLastError());
	m_hRequest.Attach(hReq);
}

void CHttpPoster::SetHeaders()
{
	CString ContentType;
	ContentType.Format(_T("Content-Type: multipart/form-data; boundary=%s"), MULTI_PART_BOUNDARY);
	::HttpAddRequestHeaders(m_hRequest, ContentType, ContentType.GetLength(),
		HTTP_ADDREQ_FLAG_ADD | HTTP_ADDREQ_FLAG_REPLACE);

}

void CHttpPoster::SendRequestExWithCA(DWORD nDataSize)
{
	if (m_bCancelled)
		return;
	INTERNET_BUFFERS BuffersIn;
	memset(&BuffersIn, 0, sizeof(BuffersIn));
	BuffersIn.dwStructSize = sizeof(BuffersIn);
	BuffersIn.dwBufferTotal = nDataSize;

	BOOL bReqSent = ::HttpSendRequestEx(m_hRequest, &BuffersIn, NULL, 0, (DWORD)this);
	DWORD dwErrorCode = GetLastError();
	if (ERROR_INTERNET_INVALID_CA == dwErrorCode ||
		ERROR_INTERNET_SEC_CERT_CN_INVALID == dwErrorCode ||
		ERROR_INTERNET_SEC_CERT_DATE_INVALID == dwErrorCode)
	{
		DWORD dwResult = ::InternetErrorDlg(GetDesktopWindow(),
			m_hRequest,
			dwErrorCode,
			FLAGS_ERROR_UI_FILTER_FOR_ERRORS |
			FLAGS_ERROR_UI_FLAGS_GENERATE_DATA |
			FLAGS_ERROR_UI_FLAGS_CHANGE_OPTIONS,
			NULL);
		if (ERROR_INTERNET_FORCE_RETRY == dwResult)
		{
			bReqSent = ::HttpSendRequestEx(m_hRequest, &BuffersIn, NULL, 0, (DWORD)this);
			dwErrorCode = GetLastError();
		}
	}

	CheckError(TRUE == bReqSent, _T("HttpSendRequestEx failed. Error: %d."), dwErrorCode);
}

void CHttpPoster::TestRequest()
{
	if (m_bCancelled)
		return;
	if (!m_UserName.IsEmpty())
	{
		SendRequestExWithCA(0);
		BOOL bEndReq = ::HttpEndRequest(m_hRequest, NULL, 0, (DWORD)this);
		DWORD dwErrorCode = GetLastError();
		CheckError(TRUE == bEndReq || dwErrorCode == ERROR_INTERNET_FORCE_RETRY, _T("HttpEndRequest failed. Error code: %d."), dwErrorCode);
		InternetSetOption(m_hRequest, INTERNET_OPTION_USERNAME, (LPVOID)LPCTSTR(m_UserName), m_UserName.GetLength());
		InternetSetOption(m_hRequest, INTERNET_OPTION_PASSWORD, (LPVOID)LPCTSTR(m_Password), m_Password.GetLength());
	}
}

void CHttpPoster::SendFile()
{
	if (m_bCancelled)
		return;

	//这里必须是非unicode
	CString StartBoundary;
	CString EndBoundary;

	CPath FileNameOnly(m_FileName);
	FileNameOnly.StripPath();
	StartBoundary.Format(
		//		_T("--%s\r\nContent-Disposition: form-data; name=\"action\"\r\n\r\nupfile\r\n")
		//		_T("--%s\r\nContent-Disposition: form-data; name=\"file\"; filename=\"%s\"\r\nContent-Type: application/x-zip-compressed\r\n\r\n"),
		//_T("--%s\r\nContent-Disposition: form-data; name=\"action\"\r\n\r\nupfile\r\n")
		_T("--%s\r\nContent-Disposition: form-data; name=\"dumpFile\"; filename=\"%s\"\r\nContent-Type: application/octet-stream\r\n\r\n"),
		/*MULTI_PART_BOUNDARY,*/ MULTI_PART_BOUNDARY, FileNameOnly
		);
	EndBoundary.Format(_T("\r\n--%s--\r\n"), MULTI_PART_BOUNDARY);

	DWORD dwFileSize = GetFileSize(m_hFile, NULL);
	std::string sStBoudary = util::cStringToString(StartBoundary);
	std::string sEndBoudary = util::cStringToString(EndBoundary);
	DWORD dwTotalDataSize = dwFileSize + sStBoudary.size() + sEndBoudary.size();
	SendRequestExWithCA(dwTotalDataSize);

	CString sBuffer;
	LPBYTE lpBuffer = (LPBYTE)sBuffer.GetBuffer(MAX_BUF_SIZE / sizeof(TCHAR));
	DWORD nWriteBufferSize = DEF_BUF_SIZE;

	for (DWORD nPos = 0; nPos < (DWORD)sStBoudary.size();)
	{
		DWORD nWritten = 0;
		BOOL bWritten = ::InternetWriteFile(m_hRequest, sStBoudary.c_str(), (DWORD)sStBoudary.size() - nPos, &nWritten);
		CheckError(TRUE == bWritten, _T("InternetWriteFile failed. Error: %d."), GetLastError());
		nPos += nWritten;
	}

	for (DWORD nPos = 0; nPos < dwFileSize;)
	{
		DWORD nRead;
		BOOL bRead = ::ReadFile(m_hFile, lpBuffer, MAX_BUF_SIZE, &nRead, NULL);
		CheckError(TRUE == bRead, _T("ReadFile failed. Error: %d."), GetLastError());
		nPos += nRead;
		for (DWORD nPosInBuffer = 0; nPosInBuffer < nRead;)
		{
			if (m_bCancelled)
				return;
			DWORD nStartTick = GetTickCount();
			DWORD nWritten;
			DWORD nNeedWrite = min(nWriteBufferSize, nRead - nPosInBuffer);
			BOOL bWritten = ::InternetWriteFile(m_hRequest, lpBuffer + nPosInBuffer, nNeedWrite, &nWritten);
			CheckError(TRUE == bWritten, _T("InternetWriteFile failed. Error: %d."), GetLastError());
			DWORD nEndTick = GetTickCount();
			nPosInBuffer += nWritten;
			nWriteBufferSize = CalcSentBufferSize(nEndTick - nStartTick, nWritten, nWriteBufferSize);
		}

	}

	for (DWORD nPos = 0; nPos < (DWORD)sEndBoudary.size();)
	{
		DWORD nWritten = 0;
		BOOL bWritten = ::InternetWriteFile(m_hRequest, sEndBoudary.c_str(), (DWORD)sEndBoudary.size() - nPos, &nWritten);
		CheckError(TRUE == bWritten, _T("InternetWriteFile failed. Error: %d."), GetLastError());
		nPos += nWritten;
	}

	BOOL bEndReq = ::HttpEndRequest(m_hRequest, NULL, 0, 0);	//(DWORD)this);
	CheckError(TRUE == bEndReq, _T("HttpEndRequest failed. Error: %d."), GetLastError());
}

DWORD CHttpPoster::CalcSentBufferSize(DWORD nTickSpan, DWORD nSent, DWORD nOldSize)
{
	if (nTickSpan < 500)
		return min(MAX_BUF_SIZE, max(nSent << 1, nOldSize));
	if (nTickSpan > 1000)
		return max(MIN_BUF_SIZE, nSent >> 1);
	return nOldSize;
}

void CHttpPoster::CheckError(BOOL bValue, LPCTSTR lpszFmt, ...)
{
	if (!bValue)
	{
		CString str;
		va_list argList;
		va_start(argList, lpszFmt);
		str.FormatV(lpszFmt, argList);
		va_end(argList);
		throw str;
	}
}

int CHttpPoster::QueryUrlStatusCode(CString sUrl)
{
	TCHAR szBuffer[1024];
	HINTERNET internetopen;
	internetopen = ::InternetOpen(_T("QueryUrlStatusCode"), INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
	if (internetopen == NULL)
		return 0;
	HINTERNET internetopenurl;
	internetopenurl = ::InternetOpenUrl(internetopen, sUrl.GetBuffer(), NULL, 0, INTERNET_FLAG_RELOAD, 0);
	if (internetopenurl == NULL)
	{
		::InternetCloseHandle(internetopenurl);
		return 0;
	}
	int iRet = 0;
	int nLength = 512;
	BOOL bRet = ::HttpQueryInfo(internetopenurl, HTTP_QUERY_STATUS_CODE, szBuffer, (unsigned long *)&nLength, NULL);
	if (!bRet)
		iRet = 0;
	else
	{
		iRet = _tstoi(szBuffer);
	}
	::InternetCloseHandle(internetopenurl);
	::InternetCloseHandle(internetopen);
	return iRet;
}

/******************************************************************************/