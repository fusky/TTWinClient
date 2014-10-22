/******************************************************************************* 
 *  @file      UserDetailInfoDialog.cpp 2014\10\22 11:15:09 $
 *  @author    ´ó·ð<dafo@mogujie.com>
 *  @brief     
 ******************************************************************************/

#include "stdafx.h"
#include "Modules/UI/UserDetailInfoDialog.h"

/******************************************************************************/
DUI_BEGIN_MESSAGE_MAP(UserDetailInfoDialog, WindowImplBase)
//DUI_ON_MSGTYPE(DUI_MSGTYPE_WINDOWINIT, OnWindowInitialized)
//DUI_ON_MSGTYPE(DUI_MSGTYPE_CLICK, OnClick)
DUI_END_MESSAGE_MAP()
// -----------------------------------------------------------------------------
//  UserDetailInfoDialog: Public, Constructor

UserDetailInfoDialog::UserDetailInfoDialog(IN std::string sid)
:m_sid(sid)
{

}

// -----------------------------------------------------------------------------
//  UserDetailInfoDialog: Public, Destructor

UserDetailInfoDialog::~UserDetailInfoDialog()
{

}

LPCTSTR UserDetailInfoDialog::GetWindowClassName() const
{
	return _T("UserDetailInfoDialog");
}

DuiLib::CDuiString UserDetailInfoDialog::GetSkinFile()
{
	return  _T("MainDialog\\UserDetailInfoDialog.xml");
}

DuiLib::CDuiString UserDetailInfoDialog::GetSkinFolder()
{
	return _T("");
}

void UserDetailInfoDialog::OnFinalMessage(HWND hWnd)
{
	__super::OnFinalMessage(hWnd);
	delete this;
}

/******************************************************************************/