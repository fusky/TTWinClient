/******************************************************************************* 
 *  @file      CaptureModule_Impl.cpp 2014\8\13 17:57:25 $
 *  @author    ¿ìµ¶<kuaidao@mogujie.com>
 *  @brief     
 ******************************************************************************/

#include "stdafx.h"
#include "CaptureModule_Impl.h"
#include <gdiPlus.h>

/******************************************************************************/
using namespace Gdiplus;
namespace module
{
	ICaptureModule* getCaptureModule()
	{
		return (ICaptureModule*)logic::GetLogic()->getModule(MODULE_ID_CAPTURE);
	}
}

namespace
{
	HINSTANCE g_hInstgdiplus = 0;

	typedef Status(__stdcall *PROC1) (unsigned int *, unsigned int *);
	typedef Status(__stdcall *PROC2) (unsigned int, unsigned int, ImageCodecInfo *);
	typedef Status(__stdcall *PROC3) (GpImage*, const WCHAR*, const CLSID*, const EncoderParameters *);
	typedef VOID(__stdcall *PROC6) (struct HBITMAP__ *, struct HPALETTE__ *, GpBitmap**);
	typedef VOID(__stdcall *PROC7) (void*);
	typedef Status(__stdcall *PROC10) (ULONG_PTR*, GdiplusStartupInput*, GdiplusStartupOutput*);
	typedef Status(__stdcall *PROC11) (ULONG_PTR);

	int getEncoderClsid(const WCHAR* format, CLSID* pClsid)
	{
		UINT  num = 0;          // number of image encoders
		UINT  size = 0;         // size of the image encoder array in bytes

		ImageCodecInfo* pImageCodecInfo = NULL;

		PROC1 Gdi_GetImageEncodersSize = (PROC1)GetProcAddress(g_hInstgdiplus, "GdipGetImageEncodersSize");
		if (Gdi_GetImageEncodersSize)
		{
			(*Gdi_GetImageEncodersSize)(&num, &size);
		}

		if (size == 0)
			return -1;  // Failure

		pImageCodecInfo = (ImageCodecInfo*)(malloc(size));
		if (pImageCodecInfo == NULL)
			return -1;  // Failure

		PROC2 Gdi_GetImageEncoders = (PROC2)GetProcAddress(g_hInstgdiplus, "GdipGetImageEncoders");
		if (Gdi_GetImageEncoders)
		{
			(*Gdi_GetImageEncoders)(num, size, pImageCodecInfo);
		}

		for (UINT j = 0; j < num; ++j)
		{
			if (wcscmp(pImageCodecInfo[j].MimeType, format) == 0)
			{
				*pClsid = pImageCodecInfo[j].Clsid;
				free(pImageCodecInfo);
				return j;  // Success
			}
		}

		free(pImageCodecInfo);
		return -1;
	}

	Status gdiplusSaveToFile(GpBitmap *pBitmap, CString &csFilename)
	{
		CLSID imgClsid;
		Status status = AccessDenied;

		CString csExt = util::getFileExtName(csFilename);
		csExt.MakeLower();
		if (csExt == "bmp")
		{
			getEncoderClsid(L"image/bmp", &imgClsid);
		}
		else if (csExt == "jpg" || csExt == "jpeg")
		{
			getEncoderClsid(L"image/jpeg", &imgClsid);
		}
		else if (csExt == "gif")
		{
			getEncoderClsid(L"image/gif", &imgClsid);
		}
		else if (csExt == "tif")
		{
			getEncoderClsid(L"image/tiff", &imgClsid);
		}
		else if (csExt == "png")
		{
			getEncoderClsid(L"image/png", &imgClsid);
		}
		else
		{
			getEncoderClsid(L"image/jpeg", &imgClsid);
		}

		BSTR pSysString = csFilename.AllocSysString();

		PROC3 Gdi_SaveImageToFile = (PROC3)GetProcAddress(g_hInstgdiplus, "GdipSaveImageToFile");
		if (Gdi_SaveImageToFile)
		{
			//²Î¿¼ http://www.jose.it-berater.org/smfforum/index.php?topic=1861.0
			EncoderParameters encoderParameters;
			ULONG             quality;
			encoderParameters.Count = 1;
			encoderParameters.Parameter[0].Guid = EncoderQuality;
			encoderParameters.Parameter[0].Type = EncoderParameterValueTypeLong;
			encoderParameters.Parameter[0].NumberOfValues = 1;

			quality = 100;
			encoderParameters.Parameter[0].Value = &quality;
			status = (*Gdi_SaveImageToFile)(pBitmap, pSysString, &imgClsid, &encoderParameters);
		}

		::SysFreeString(pSysString);

		return status;
	}


	Status saveFile(HBITMAP bmp, CString& csDstFile)
	{
		Status status;
		GpBitmap *pBmp = NULL;

		PROC6 Gdi_CreateBitmapFromHBITMAP = (PROC6)GetProcAddress(g_hInstgdiplus, "GdipCreateBitmapFromHBITMAP");
		if (Gdi_CreateBitmapFromHBITMAP)
		{
			(*Gdi_CreateBitmapFromHBITMAP)(bmp, NULL, &pBmp);
		}

		status = AccessDenied;
		if (pBmp)
		{
			status = gdiplusSaveToFile(pBmp, csDstFile);
			PROC7 Gdi_GdipFree = (PROC7)GetProcAddress(g_hInstgdiplus, "GdipFree");
			if (Gdi_GdipFree)
			{
				(*Gdi_GdipFree)(pBmp);
			}
			pBmp = NULL;
		}
		return status;
	}
}

// -----------------------------------------------------------------------------
//  CaptureModule_Impl: Public, Constructor

CaptureModule_Impl::CaptureModule_Impl()
:m_token(0)
{

}

// -----------------------------------------------------------------------------
//  CaptureModule_Impl: Public, Destructor

CaptureModule_Impl::~CaptureModule_Impl()
{

}

void CaptureModule_Impl::release()
{
	delete this;
}

BOOL CaptureModule_Impl::saveToFile(HBITMAP hBitmap, CString& csDstFileName)
{
	CString csExtName = util::getFileExtName(csDstFileName);
	csExtName.MakeLower();
	BOOL bRet = FALSE;
	if (!_startupGdiPlus() || csExtName == _T(".bmp"))
	{
		CString csDstTemp = csDstFileName + _T(".bmp");
		bRet = util::saveBitmapToFile(hBitmap, csDstTemp);
	}
	else
	{
		Status stat = saveFile(hBitmap, csDstFileName);
		bRet = (Ok == stat);
	}

	return bRet;
}

BOOL CaptureModule_Impl::_startupGdiPlus()
{
	if (g_hInstgdiplus == NULL)
	{
		g_hInstgdiplus = LoadLibrary(_T("GdiPlus"));

		PROC10 Gdi_GdiplusStartup = (PROC10)GetProcAddress(g_hInstgdiplus, "GdiplusStartup");

		GdiplusStartupInput input;

		if (Gdi_GdiplusStartup)
		{
			Status status = (*Gdi_GdiplusStartup)(&m_token, &input, NULL);
			if (status == 0)
			{
				return TRUE;
			}
			else
			{
				FreeLibrary(g_hInstgdiplus);
				g_hInstgdiplus = NULL;
			}
		}
	}
	else
		return TRUE;

	return FALSE;
}

/******************************************************************************/