 /*******************************************************************************
 *  @file      UserDetailInfoDialog.h 2014\10\22 11:14:34 $
 *  @author    ´ó·ð<dafo@mogujie.com>
 *  @brief     
 ******************************************************************************/

#ifndef USERDETAILINFODIALOG_9D88F1FB_337B_4EF3_8282_4B28D272199E_H__
#define USERDETAILINFODIALOG_9D88F1FB_337B_4EF3_8282_4B28D272199E_H__

#include "DuiLib/UIlib.h"
#include "GlobalDefine.h"
using namespace DuiLib;

/******************************************************************************/

/**
 * The class <code>UserDetailInfoDialog</code> 
 *
 */
class UserDetailInfoDialog : public WindowImplBase
{
public:
    /** @name Constructors and Destructor*/

    //@{
    /**
     * Constructor 
     */
    UserDetailInfoDialog(IN std::string sid);
    /**
     * Destructor
     */
    ~UserDetailInfoDialog();
    //@}

	LPCTSTR GetWindowClassName() const;
	virtual CDuiString GetSkinFile();
	virtual CDuiString GetSkinFolder();
	virtual void OnFinalMessage(HWND hWnd);
	DUI_DECLARE_MESSAGE_MAP()
private:
	std::string m_sid;
};
/******************************************************************************/
#endif// USERDETAILINFODIALOG_9D88F1FB_337B_4EF3_8282_4B28D272199E_H__
