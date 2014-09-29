/******************************************************************************* 
 *  @file      utilCommonAPI.cpp 2014\7\15 20:30:45 $
 *  @author    快刀<kuaidao@mogujie.com>
 *  @brief   
 ******************************************************************************/

#include "stdafx.h"
#include "utility/utilCommonAPI.h"
#include "md5.h"
#include <shlwapi.h>
#include <shellapi.h>

/******************************************************************************/
NAMESPACE_BEGIN(util)

CString bin2hex(const void * _src, size_t len)
{
	const TCHAR * Hex = (const TCHAR *)_T("0123456789abcdef");
	const unsigned char * src = (const unsigned char *)_src;

	tstring hex;
	hex.resize(len << 1);

	for (size_t i = 0; i < len; i++)
	{
		UINT b = (UINT)src[i];
		hex[i * 2] = Hex[b >> 4];
		hex[i * 2 + 1] = Hex[b & 0xF];
	}

	return hex.c_str();
}

void makeMD5Value(void* pSrc, size_t length, std::vector<unsigned char>& res)
{
	res.clear();
	res.resize(16);
	MD5_CTX ctx;
	MD5Init(&ctx);
	MD5Update(&ctx, (BYTE*)pSrc, length);
	MD5Final(&res[0], &ctx);
}

CString makeMD5Value(void* pSrc, size_t length)
{
	std::vector<unsigned char> bmd5;
	makeMD5Value(pSrc, length, bmd5);

	return bin2hex(&bmd5[0], 16);
}

CString reparsePath(const CString& strFilePath)
{
	CString strTmp = strFilePath;

	strTmp.Replace(_T("/"), _T("\\"));
	strTmp.Replace(_T("//"), _T("\\"));

	// 如果非本地路径
	int nLen = strTmp.GetLength();
	if ((nLen > 2 && strTmp.Left(2).CompareNoCase(_T("\\\\")) == 0) || (nLen > 5 && (strTmp.Left(5).CompareNoCase(_T("http:")) == 0 || strTmp.Left(5).CompareNoCase(_T("file:")) == 0)) || (nLen > 4 && strTmp.Left(4).CompareNoCase(_T("ftp:")) == 0))
		return strTmp;

	strTmp.Replace(_T("\\\\"), _T("\\"));
	return strTmp;
}

CString getFileDirFromPath(const CString& strFilePath)
{
	CString strTmp = reparsePath(strFilePath);
	int index = strTmp.ReverseFind('\\');
	if (index == -1)
	{
		index = strTmp.ReverseFind(':');
	}
	return strTmp.Left(index + 1);
}

CString getAppPath()
{
	static CString g_sDllPath = _T("");

	if (g_sDllPath.IsEmpty())
	{
		TCHAR	buffer[MAX_PATH];
		ZeroMemory(buffer, sizeof(TCHAR)* MAX_PATH);
		HMODULE h = GetModuleHandle(NULL);
		::GetModuleFileName(h, buffer, MAX_PATH);
		g_sDllPath = getFileDirFromPath(CString(buffer));
	}
	return g_sDllPath;
}

CString getParentAppPath()
{
	static CString g_csParentAppPath = _T("");
	if (g_csParentAppPath.IsEmpty())
	{
		g_csParentAppPath = getAppPath();
		LPTSTR lpszPath = g_csParentAppPath.GetBuffer();
		::PathRemoveBackslash(lpszPath);
		g_csParentAppPath = getFileDirFromPath(g_csParentAppPath);
	}
	return g_csParentAppPath;
}

BOOL isFileExist(LPCTSTR lpcsFileName)
{
	return PathFileExists(lpcsFileName);
}

BOOL createAllDirectories(CString & csDir)
{
	if (csDir.Right(1) == _T("\\"))
	{
		csDir = csDir.Left(csDir.GetLength() - 1);
	}

	if (::GetFileAttributes(csDir) != INVALID_FILE_ATTRIBUTES)
	{
		return TRUE;
	}

	int nFound = csDir.ReverseFind('\\');
	createAllDirectories(csDir.Left(nFound));

	if (!::CreateDirectory(csDir, NULL))
	{
		return FALSE;
	}

	return TRUE;
}

BOOL emptyFolder(const CString& csFolder, BOOL bDeleteSelf /*= FALSE*/)
{
	CFileFind tempFind;
	CString tmp;
	tmp.Format(_T("%s\\*.*"), csFolder);
	BOOL IsFinded = tempFind.FindFile(tmp);
	while (IsFinded)
	{
		IsFinded = tempFind.FindNextFile();

		if (!tempFind.IsDots())
		{
			CString& sFoundFileName = tempFind.GetFileName();

			if (tempFind.IsDirectory())
			{
				CString sTempDir;
				sTempDir.Format(_T("%s\\%s"), csFolder, sFoundFileName);
				emptyFolder(sTempDir, TRUE);
			}
			else
			{
				CString sTempFileName;
				sTempFileName.Format(_T("%s\\%s"), csFolder, sFoundFileName);
				DeleteFile(sTempFileName);
			}
		}
	}
	tempFind.Close();
	if (bDeleteSelf)
	{
		if (!RemoveDirectory(csFolder))
		{
			return FALSE;
		}
	}
	return TRUE;
}
CString getFileExtName(CString& strFile)
{
	CString strExt = _T("");
	LPTSTR pszExtName = (LPTSTR)_tcsrchr(strFile, _T('.'));
	if (pszExtName++)
	{
		strExt = pszExtName;
	}

	return strExt;
}

UInt32 hash_BKDR(const char* str)
{
	unsigned int seed = 131; // 31 131 1313 13131 131313 etc.
	unsigned int hash = 0;
	while (*str)
	{
		hash = hash * seed + (*str++);
	}

	return (hash & 0x7FFFFFFF);
}

BOOL saveBitmapToFile(HBITMAP hBitmap, LPCTSTR lpFileName)
{
	HDC hDC;
	int iBits;
	WORD wBitCount = 24;
	DWORD dwPaletteSize = 0,
		dwBmBitsSize,
		dwDIBSize, dwWritten;
	BITMAP Bitmap;
	BITMAPFILEHEADER bmfHdr;
	BITMAPINFOHEADER bi;
	LPBITMAPINFOHEADER lpbi;

	HANDLE fh = INVALID_HANDLE_VALUE;
	HGDIOBJ hPal = NULL, hOldPal = NULL;
	void* pData = NULL;
	BOOL bResult = FALSE;

	HDC hWndDC = CreateDC(L"DISPLAY", NULL, NULL, NULL);
	hDC = ::CreateCompatibleDC(hWndDC);
	if (hDC)
		iBits = GetDeviceCaps(hDC, BITSPIXEL) * GetDeviceCaps(hDC, PLANES);
	DeleteDC(hDC);
	DeleteDC(hWndDC);

	if (iBits <= 1)
		wBitCount = 1;
	else if (iBits <= 4)
		wBitCount = 4;
	else if (iBits <= 8)
		wBitCount = 8;
	else if (iBits <= 24)
		wBitCount = 24;
	else
		wBitCount = 24;

	if (wBitCount <= 8)
		dwPaletteSize = (1 << wBitCount) * sizeof(RGBQUAD);

	GetObject(hBitmap, sizeof(BITMAP), (LPSTR)&Bitmap);
	bi.biSize = sizeof(BITMAPINFOHEADER);
	bi.biWidth = Bitmap.bmWidth;
	bi.biHeight = Bitmap.bmHeight;
	bi.biPlanes = 1;
	bi.biBitCount = wBitCount;
	bi.biCompression = BI_RGB;
	bi.biSizeImage = 0;
	bi.biXPelsPerMeter = 0;
	bi.biYPelsPerMeter = 0;
	bi.biClrUsed = 0;
	bi.biClrImportant = 0;

	dwBmBitsSize = ((Bitmap.bmWidth * wBitCount + 31) / 32) * 4 * Bitmap.bmHeight;

	pData = malloc(dwBmBitsSize + dwPaletteSize + sizeof(BITMAPINFOHEADER));

	if (pData == NULL)
		goto leave;

	lpbi = (LPBITMAPINFOHEADER)pData;
	*lpbi = bi;

	hPal = GetStockObject(DEFAULT_PALETTE);
	if (hPal)
	{
		hDC = ::GetDC(NULL);
		hOldPal = ::SelectPalette(hDC, (HPALETTE)hPal, FALSE);
		RealizePalette(hDC);
	}

	GetDIBits(hDC, hBitmap, 0, (UINT)Bitmap.bmHeight,
		(LPSTR)lpbi + sizeof(BITMAPINFOHEADER)
		+dwPaletteSize,
		(LPBITMAPINFO)
		lpbi, DIB_RGB_COLORS);

	if (hOldPal)
	{
		SelectPalette(hDC, (HPALETTE)hOldPal, TRUE);
		RealizePalette(hDC);
		::ReleaseDC(NULL, hDC);
	}

	fh = CreateFile(lpFileName, GENERIC_WRITE,
		0, NULL, CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL, NULL);

	if (fh == INVALID_HANDLE_VALUE)
		goto leave;

	bmfHdr.bfType = 0x4D42; // "BM"
	dwDIBSize = sizeof(BITMAPFILEHEADER)
		+sizeof(BITMAPINFOHEADER)
		+dwPaletteSize + dwBmBitsSize;
	bmfHdr.bfSize = dwDIBSize;
	bmfHdr.bfReserved1 = 0;
	bmfHdr.bfReserved2 = 0;
	bmfHdr.bfOffBits = (DWORD)sizeof(BITMAPFILEHEADER)
		+(DWORD)sizeof(BITMAPINFOHEADER)
		+dwPaletteSize;

	WriteFile(fh, (LPSTR)&bmfHdr, sizeof(BITMAPFILEHEADER), &dwWritten, NULL);

	WriteFile(fh, (LPSTR)lpbi, dwDIBSize, &dwWritten, NULL);

	bResult = TRUE;

leave:
	if (pData != NULL)
	{
		free(pData);
	}

	if (fh != INVALID_HANDLE_VALUE)
		CloseHandle(fh);

	return bResult;

}

CString GetIEVersionStr()
{
	HKEY hKey = NULL;
	LONG lRet = ::RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("Software\\Microsoft\\Internet Explorer"), 0, KEY_ALL_ACCESS, &hKey);
	if (lRet != ERROR_SUCCESS)  //该键值不存在
	{
		::RegCloseKey(hKey);
		return _T("");
	}

	DWORD datasize = MAX_PATH;
	BYTE  szData[MAX_PATH];
	memset(szData, 0, datasize);
	DWORD dType = REG_SZ;

	lRet = ::RegQueryValueEx(hKey, _T("Version"), NULL, &dType, szData, &datasize);
	if (lRet != ERROR_SUCCESS)
	{
		::RegCloseKey(hKey);
		return _T("");
	}

	::RegCloseKey(hKey);

	szData[datasize] = 0;
	CString sVersion;
	sVersion.Format(_T("%s"), szData);

	return sVersion;
}

UTILITY_API void openWebBrowser(CString& url, Int32 nFlag /*= SW_SHOWMAXIMIZED*/)
{
	HINSTANCE	hRet = 0;
	hRet = ::ShellExecute(NULL, _T("open"), url, NULL, NULL, nFlag);
	if ((DWORD)hRet <= 32)
	{
		hRet = ::ShellExecute(NULL, _T("open"), _T("IEXPLORE.EXE"), url, NULL, nFlag);
		if ((DWORD)hRet <= 32)
		{
			APP_LOG(LOG_ERROR, _T("openWebBrowser open IEXPLORE.EXE failed"));
		}
	}
}

BOOL loadFile(const CString& sFilepath, std::string& sBuffer)
{
	try
	{
		CFile file;
		BOOL b = file.Open(sFilepath, CFile::modeRead | CFile::shareDenyNone | CFile::typeBinary);
		if (!b)
		{
			ASSERT(FALSE);
			return FALSE;
		}

		const int READ_BUFFER_SIZE = 4096;
		char buff[READ_BUFFER_SIZE];
		int  nReadSize = 0;
		do
		{
			nReadSize = file.Read(buff, READ_BUFFER_SIZE);
			if (nReadSize)
				sBuffer.append(buff, nReadSize);

		} while (nReadSize > 0);
		file.Close();

		return TRUE;
	}
	catch (CFileException* e)
	{
		e->Delete();
		return FALSE;
	}
}

#include "nb30.h"
//取得机器的mac地址
typedef   struct   _ASTAT
{
	ADAPTER_STATUS   adapt;
	NAME_BUFFER   NameBuffer[30];
}   ASTAT, *PASTAT;
CString getMacAddress()
{
	CString   sMacNumber;
	sMacNumber = _T("unknowuser00");
	ASTAT   Adapter;
	NCB   Ncb;
	UCHAR   uRetCode;
	//char   NetName[50];   
	LANA_ENUM       lenum;
	int             i;
	memset(&Ncb, 0, sizeof(Ncb));
	Ncb.ncb_command = NCBENUM;
	Ncb.ncb_buffer = (UCHAR   *)&lenum;
	Ncb.ncb_length = sizeof(lenum);
	uRetCode = Netbios(&Ncb);
	for (i = 0; i < lenum.length; i++)
	{
		memset(&Ncb, 0, sizeof(Ncb));
		Ncb.ncb_command = NCBRESET;
		Ncb.ncb_lana_num = lenum.lana[i];
		uRetCode = Netbios(&Ncb);
		memset(&Ncb, 0, sizeof   (Ncb));
		Ncb.ncb_command = NCBASTAT;
		Ncb.ncb_lana_num = lenum.lana[i];
		strcpy((char   *)Ncb.ncb_callname, "*                               ");
		Ncb.ncb_buffer = (unsigned   char   *)&Adapter;
		Ncb.ncb_length = sizeof(Adapter);
		uRetCode = Netbios(&Ncb);
		if (uRetCode == 0)
		{
			if (Adapter.adapt.adapter_address[0] +
				Adapter.adapt.adapter_address[1] +
				Adapter.adapt.adapter_address[2] +
				Adapter.adapt.adapter_address[3] +
				Adapter.adapt.adapter_address[4] +
				Adapter.adapt.adapter_address[5] != 0)
			{

				sMacNumber.Format(_T("%02x-%02x-%02x-%02x-%02x-%02x"),
					Adapter.adapt.adapter_address[0],
					Adapter.adapt.adapter_address[1],
					Adapter.adapt.adapter_address[2],
					Adapter.adapt.adapter_address[3],
					Adapter.adapt.adapter_address[4],
					Adapter.adapt.adapter_address[5]);
				break;
			}
		}
	}
	return sMacNumber;
}

BOOL directoryOperation(const CString & csSrc, const CString & csDes, int flag)
{
	ASSERT(flag == FO_COPY || flag == FO_DELETE);

	if (!::PathFileExists(csSrc))
	{
		return FALSE;
	}
	if (FO_COPY == flag)
	{
		if (!::PathFileExists(csDes))
		{
			return FALSE;
		}
	}

	CString csFrom = csSrc;
	csFrom.AppendChar('\0');
	CString csTo = csDes;
	csTo.AppendChar('\0');

	SHFILEOPSTRUCT FO;
	memset(&FO, 0, sizeof(FO));
	FO.hwnd = NULL;
	FO.wFunc = flag;
	FO.fFlags = FOF_MULTIDESTFILES | FOF_SILENT | FOF_NOCONFIRMATION;
	switch (flag)
	{
	case FO_COPY:
	{
		FO.pTo = csTo;
		FO.pFrom = csFrom;
	}
		break;
	case FO_DELETE:
	{
		// 去除斜杠
		LPTSTR lpszPath = csFrom.GetBuffer();
		::PathRemoveBackslash(lpszPath);
		csFrom.ReleaseBuffer();

		FO.pTo = NULL;
		FO.pFrom = csFrom;
	}
		break;
	}

	return !SHFileOperation(&FO);
}

void messagePump()
{
	MSG msg;
	while (::GetMessage(&msg, NULL, 0, 0) > 0)
	{
		::TranslateMessage(&msg);
		::DispatchMessage(&msg);
	}
}

UInt64 getFileSize(const CString& csFileName)
{
	struct __stat64 buffer;
#ifdef _UNICODE
	int ret = _wstat64(csFileName, &buffer);
#else
	int nLen = MultiByteToWideChar(CP_ACP, 0, csFileName, -1, NULL, NULL);
	LPWSTR lpszW = new WCHAR[nLen];
	MultiByteToWideChar(CP_ACP, 0,
		csFileName, -1, lpszW, nLen);
	int ret = _wstat64(lpszW, &buffer);
	delete[] lpszW;
#endif
	return buffer.st_size;
}

UTILITY_API BOOL GetOpenFilePath(CWnd* pParent, std::list<CString>& lstFiles, BOOL bAllowMultiSel /*= TRUE */, CString csFilter /*= _T("*.*|*.*||") */, CString initDir /*= _T("")*/)
{
	lstFiles.clear();

	// 选择文件，支持多选
	CFileDialog	dlg(TRUE, NULL, NULL, OFN_HIDEREADONLY | OFN_FILEMUSTEXIST | (bAllowMultiSel ? OFN_ALLOWMULTISELECT : 0), csFilter, pParent);

	dlg.m_ofn.lpstrInitialDir = initDir;
	dlg.m_ofn.Flags |= OFN_NOCHANGEDIR;

	dlg.DoModal();
	lstFiles.clear();

	POSITION nPos = dlg.GetStartPosition();
	while (nPos != NULL)
	{
		CString sPathName = dlg.GetNextPathName(nPos);
		lstFiles.push_back(sPathName);
	}

	return TRUE;
}

UTILITY_API CString getFormatSizeString(const UInt64 nSize)
{
	CString strText(_T(""));
	float   fSize = 0.0f;

	// 数值大于1G，以G为单位格式化字符串
	if (nSize > (1 << 30))
	{
		fSize = (float)nSize / (1024 * 1024 * 1024);
		strText.Format(_T("%0.1f GB"), fSize);
	}
	// 数值在1M和1G之间，以M为单位格式化字符串
	else if (nSize > (1 << 20))
	{
		fSize = (float)nSize / (1024 * 1024);
		strText.Format(_T("%0.1f MB"), fSize);
	}
	// 数值在1K与1M之间，以K为单位格式化字符串
	else if (nSize > (1 << 10))
	{
		fSize = (float)nSize / 1024;
		strText.Format(_T("%0.1f KB"), fSize);
	}
	// 数值在1K之内，以B为单位格式化字符串
	else
	{
		strText.Format(_T("%I64u B"), nSize);
	}

	// 去掉.0
	int nPos = strText.Find(_T(".0"));
	if (nPos != -1)
		strText.Delete(nPos, 2);
	return strText;
}

BOOL registerDll(const CString& sFilePath)
{
	if (!isFileExist(sFilePath))
		return FALSE;

	typedef LRESULT(*DllRegisterServerProc)(void);
	BOOL retVal = FALSE;
	HINSTANCE hDll = LoadLibrary(sFilePath);
	while (TRUE)
	{
		if (hDll == NULL)
			break;
		DllRegisterServerProc DllRegisterServer;
		DllRegisterServer = (DllRegisterServerProc)GetProcAddress(hDll, "DllRegisterServer");
		if (DllRegisterServer == NULL)
			break;

		int temp = DllRegisterServer();
		if (temp != S_OK)
			break;

		retVal = TRUE;
		break;
	}

	if (retVal == FALSE)
	{
		APP_LOG(LOG_ERROR, _T("register dll failed,%s"),sFilePath);
	}

	FreeLibrary(hDll);
	return retVal;
}

NAMESPACE_END(util)
/******************************************************************************/