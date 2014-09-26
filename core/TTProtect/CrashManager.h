/*******************************************************************************
 *  @file      CrashManager.h 2014\9\1 11:59:11 $
 *  @author    快刀<kuaidao@mogujie.com>
 *  @brief     
 ******************************************************************************/

#ifndef CRASHMANAGER_12CE91C1_2027_46F8_A371_039C6846EB9D_H__
#define CRASHMANAGER_12CE91C1_2027_46F8_A371_039C6846EB9D_H__

#include "Parameters.h"
#pragma comment(lib, "Dbghelp")
#include <vector>
using namespace std;
/******************************************************************************/

class CCrashManager
{
	const CParameters& m_Params;
	CString m_sZipFileName;
	CString m_sDirName;
	CString m_sQueryUrl;				// 校验的url
	CString m_sPostUrl;					// 发dump的url

private:
	void ListDir(LPCTSTR lpszDir, vector<CString>& Dirs, vector<CString>& Files);
	BOOL Compress();

public:
	CCrashManager();
	~CCrashManager();
	BOOL CreateDumpFile();
	void ReportByHttp();
	BOOL HttpQueryCanUpload();
	void Init(CString sQueryUrl, CString sPostUrl);
	void RestartApp();
	CString GetDumpsDirName();
};

/******************************************************************************/
#endif// CRASHMANAGER_12CE91C1_2027_46F8_A371_039C6846EB9D_H__