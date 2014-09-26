/*******************************************************************************
 *  @file      ICaptureModule.h 2014\8\13 17:53:01 $
 *  @author    快刀<kuaidao@mogujie.com>
 *  @brief     截屏模块
 ******************************************************************************/

#ifndef ICAPTUREMODULE_7229AFB6_4897_43B8_AFB8_1FF7F0143390_H__
#define ICAPTUREMODULE_7229AFB6_4897_43B8_AFB8_1FF7F0143390_H__

#include "GlobalDefine.h"
#include "TTLogic/IModule.h"
#include "Modules/ModuleDll.h"
/******************************************************************************/
NAMESPACE_BEGIN(module)
//KEYID
enum
{
	KEY_CAPTURE_DOWNNOTIFY				= MODULE_ID_CAPTURE << 16 | 1,	//截图成功通知
};

/**
 * The class <code>截屏模块</code> 
 *
 */
class ICaptureModule : public logic::IModule
{
public:
    /** @name Constructors and Destructor*/

    //@{
    /**
     * Constructor 
     */
	ICaptureModule()
	{
		m_moduleId = MODULE_ID_CAPTURE;
	}
    //@}

public:
	virtual BOOL saveToFile(HBITMAP hBitmap, CString& csDstFileName) = 0;
};

MODULE_API ICaptureModule* getCaptureModule();

NAMESPACE_END(module)
/******************************************************************************/
#endif// ICAPTUREMODULE_7229AFB6_4897_43B8_AFB8_1FF7F0143390_H__
