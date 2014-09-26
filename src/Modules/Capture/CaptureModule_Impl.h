/*******************************************************************************
 *  @file      CaptureModule_Impl.h 2014\8\13 17:56:33 $
 *  @author    ¿ìµ¶<kuaidao@mogujie.com>
 *  @brief     
 ******************************************************************************/

#ifndef CAPTUREMODULE_IMPL_5998A1E9_EC2A_4870_A49F_532C597CB16E_H__
#define CAPTUREMODULE_IMPL_5998A1E9_EC2A_4870_A49F_532C597CB16E_H__

#include "Modules/ICaptureModule.h"
/******************************************************************************/

/**
 * The class <code>CaptureModule_Impl</code> 
 *
 */
class CaptureModule_Impl : public module::ICaptureModule
{
public:
    /** @name Constructors and Destructor*/

    //@{
    /**
     * Constructor 
     */
    CaptureModule_Impl();
    /**
     * Destructor
     */
    virtual ~CaptureModule_Impl();
    //@}
	virtual void release();

public:
	virtual BOOL saveToFile(HBITMAP hBitmap, CString& csDstFileName);

private:
	BOOL _startupGdiPlus();

private:
	ULONG_PTR           m_token;
};
/******************************************************************************/
#endif// CAPTUREMODULE_IMPL_5998A1E9_EC2A_4870_A49F_532C597CB16E_H__
