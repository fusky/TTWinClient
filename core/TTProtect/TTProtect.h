#pragma once

#ifndef __AFXWIN_H__
	#error "在包含此文件之前包含“stdafx.h”以生成 PCH 文件"
#endif

#include "resource.h"		// 主符号


// CTTProtectApp: 
// 有关此类的实现，请参阅 TTProtect.cpp
//

class CTTProtectApp : public CWinApp
{
public:
	CTTProtectApp();

// 重写
public:
	virtual BOOL InitInstance();
	/**
	 * 单实例运行
	 *
	 * @return  BOOL
	 * @exception there is no any exception to throw.
	 */	
	BOOL IsHaveInstance();

	DECLARE_MESSAGE_MAP()
};

extern CTTProtectApp theApp;