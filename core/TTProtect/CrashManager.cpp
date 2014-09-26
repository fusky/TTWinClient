/******************************************************************************* 
 *  @file      CrashManager.cpp 2014\9\1 12:00:55 $
 *  @author    快刀<kuaidao@mogujie.com>
 *  @brief     
 ******************************************************************************/

#include "StdAfx.h"
#include "CrashManager.h"
#include "Shlwapi.h"
#include "HttpPoster.h"
#include "utility/XZip.h"
#include "utility/utilCommonAPI.h"
#include "utility/utilStrCodeAPI.h"
#include <atlpath.h>
#include <DbgHelp.h>

/******************************************************************************/
CCrashManager::CCrashManager()
: m_Params(CParameters::Instance())

{
}

CCrashManager::~CCrashManager()
{

}

BOOL CCrashManager::CreateDumpFile()
{
	HANDLE hProcess = NULL;
	HANDLE hDumpFile = INVALID_HANDLE_VALUE;

	if (!m_Params.m_dwPid)
		return FALSE;
	hProcess = OpenProcess(PROCESS_VM_READ | PROCESS_QUERY_INFORMATION | PROCESS_DUP_HANDLE, NULL, m_Params.m_dwPid);
	if (hProcess == NULL)
		return FALSE;

	// 生成dump目录
	CString sModule;
	CString strExceptionFile;
	CString sDirName;
	CString sDmpFileName;
	CTime timeData = CTime::GetCurrentTime();
	CString sTime;
	sTime = timeData.Format(_T("%Y%m%d%H%M%S"));
	HMODULE hModule = (HMODULE)AfxGetApp()->m_hInstance;
	TCHAR lpFn[255];
	::GetModuleFileName(hModule, lpFn, 255);
	sDirName = lpFn;
	int iPos = sDirName.ReverseFind('\\');
	sDirName = sDirName.Left(iPos + 1);
	sModule = sDirName;
	sDirName = sDirName + _T("dumps\\");
	// 判断目录是否存在
	if (!::PathFileExists(sDirName.GetBuffer()))
		::CreateDirectory(sDirName.GetBuffer(), NULL);
	//::SetFileAttributes(sDirName, FILE_ATTRIBUTE_HIDDEN);

	// 获取版本号
	CString csVersion = _T("0");
	if (::PathFileExists(sModule + _T("version.dat")))
	{
		std::string strVersion;
		util::loadFile(sModule + _T("version.dat"), strVersion);
		csVersion = util::stringToCString(strVersion);
		int n = util::cstringToInt32(csVersion);
		csVersion = util::int32ToCString(n);
	}

	strExceptionFile = sDirName;
	sDirName = sDirName + _T("mtalk_") + util::getMacAddress() + _T("_") + csVersion + _T("_") + sTime + _T("\\");
	m_sDirName = sDirName;
	sDmpFileName = sDirName + _T("dump.dmp");
	// 判断目录是否存在
	if (!PathFileExists(sDirName.GetBuffer()))
		CreateDirectory(sDirName.GetBuffer(), NULL);
	::SetFileAttributes(sDirName, FILE_ATTRIBUTE_HIDDEN);

	// 生成dump文件
	hDumpFile = ::CreateFile(
		sDmpFileName,
		GENERIC_WRITE,
		FILE_SHARE_READ,
		NULL,
		CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		NULL);
	if (hDumpFile == INVALID_HANDLE_VALUE)
	{
		CloseHandle(hProcess);
		return FALSE;
	}

	MINIDUMP_EXCEPTION_INFORMATION mei = { m_Params.m_dwTid, m_Params.m_pClientExceptionPtr, TRUE };
	PMINIDUMP_EXCEPTION_INFORMATION pmei = 0 == m_Params.m_dwTid || NULL == m_Params.m_pClientExceptionPtr ? NULL : &mei;
	if (!MiniDumpWriteDump(hProcess, m_Params.m_dwPid, hDumpFile, MiniDumpNormal, pmei, NULL, NULL))
	{
		CloseHandle(hDumpFile);
		CloseHandle(hProcess);
		return FALSE;
	}

	CloseHandle(hDumpFile);
	CloseHandle(hProcess);

	// 拷贝日志文件
	CString csSrcLog = sModule + _T("applog.txt");
	CString csDstLog = sDirName + _T("applog.txt");
	::CopyFile(csSrcLog, csDstLog, FALSE);

	// 拷贝errorcode.exception文件
	strExceptionFile += _T("Errorcode.exception");
	if (PathFileExists(strExceptionFile.GetBuffer()))
	{
		CString strDes = m_sDirName + _T("Errorcode.exception");
		// copy 
		CopyFile(strExceptionFile, strDes, FALSE);
	}
	//
	Compress();

	return TRUE;
}

BOOL CCrashManager::Compress()
{
	HZIP hZip = NULL;
	CPath ZipFileName(m_sDirName);
	ZipFileName.RemoveBackslash();
	m_sZipFileName = CString(ZipFileName);
	m_sZipFileName += _T(".zip");
	CString csZipFileName = m_sZipFileName;
	hZip = CreateZip(csZipFileName.GetBuffer(), 0, ZIP_FILENAME);
	if (NULL == hZip)
		return FALSE;
	std::vector<CString> Dirs;
	std::vector<CString> Files;
	ListDir(m_sDirName, Dirs, Files);
	for (size_t i = 0; i < Dirs.size(); i++)
	{
		if (!Dirs[i].IsEmpty())
		{
			ZRESULT ZResult = ZipAdd(hZip, Dirs[i], NULL, 0, ZIP_FOLDER);
			if (ZResult != ZR_OK)
			{
				CloseZip(hZip);
				ASSERT(0);
				return FALSE;
			}
		}
	}
	for (size_t i = 0; i < Files.size(); i++)
	{
		if (!Files[i].IsEmpty())
		{
			ZRESULT ZResult = ZipAdd(hZip, Files[i], (m_sDirName + Files[i]).GetBuffer(), 0, ZIP_FILENAME);
			if (ZResult != ZR_OK)
			{
				CloseZip(hZip);
				ASSERT(0);
				return FALSE;
			}
		}
	}

	if (NULL != hZip)
		CloseZip(hZip);

	return TRUE;
}

void CCrashManager::ListDir(LPCTSTR lpszDir, vector<CString>& Dirs, vector<CString>& Files)
{
	CPath RootDir = lpszDir;
	RootDir.AddBackslash();
	static const int RESERVE_DIR_COUNT = 16;
	Dirs.clear();
	Files.clear();
	Dirs.reserve(RESERVE_DIR_COUNT);
	Files.reserve(RESERVE_DIR_COUNT);
	Dirs.push_back(_T(""));
	for (unsigned int i = 0; i < Dirs.size(); i++)
	{
		CPath Path(CString(RootDir) + Dirs[i]);
		Path += _T("*.*");
		WIN32_FIND_DATA FindData;
		HANDLE hFind = INVALID_HANDLE_VALUE;
		hFind = FindFirstFile(Path, &FindData);
		for (BOOL bFound = INVALID_HANDLE_VALUE != hFind; bFound; bFound = FindNextFile(hFind, &FindData))
		{
			CString;
			CPath NewFile(Dirs[i]);
			NewFile += FindData.cFileName;
			if (FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				if (FindData.cFileName[0] != _T('.'))
				{
					Dirs.push_back(NewFile);
				}
			}
			else
				Files.push_back(NewFile);
		}
		if (INVALID_HANDLE_VALUE != hFind)
			FindClose(hFind);
	}
}

void CCrashManager::ReportByHttp()
{
	try
	{
		CHttpPoster Poster;
		CString csUserName;
		CString csPassword;;
		LPCTSTR lpszUserName = csUserName.IsEmpty() ? NULL : LPCTSTR(csUserName);
		LPCTSTR lpszPassword = csUserName.IsEmpty() ? NULL : LPCTSTR(csPassword);
		Poster.PostFile(m_sPostUrl, m_sZipFileName, lpszUserName, lpszPassword);
	}
	catch (const CString& str)
	{
		::MessageBox(GetActiveWindow(), str, NULL, MB_OK | MB_ICONERROR);
	}
	catch (...)
	{
		::MessageBox(GetActiveWindow(), _T("send file unkown error."), NULL, MB_OK | MB_ICONERROR);
	}
}

BOOL CCrashManager::HttpQueryCanUpload()
{
	CHttpPoster httpPoster;
	int iCode = httpPoster.QueryUrlStatusCode(m_sQueryUrl);
	return iCode == 200;
}

void CCrashManager::Init(CString sQueryUrl, CString sPostUrl)
{
	m_sQueryUrl = sQueryUrl + m_Params.m_sParameter;
	m_sPostUrl = sPostUrl;
}

void CCrashManager::RestartApp()
{
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	memset(&si, 0, sizeof(si));
	si.cb = sizeof(si);
	si.wShowWindow = SW_SHOW;
	::CreateProcess(m_Params.m_sRestartAppPath, NULL, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
}

CString CCrashManager::GetDumpsDirName()
{
	CString csDumpsDir = m_sDirName;

	LPTSTR lpszPath = csDumpsDir.GetBuffer();
	::PathRemoveBackslash(lpszPath);
	csDumpsDir.ReleaseBuffer();
	csDumpsDir = util::getFileDirFromPath(csDumpsDir);
	lpszPath = csDumpsDir.GetBuffer();
	::PathRemoveBackslash(lpszPath);
	csDumpsDir.ReleaseBuffer();

	return csDumpsDir;
}
/******************************************************************************/