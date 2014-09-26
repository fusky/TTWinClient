/******************************************************************************* 
 *  @file      ServerConfigDialog.cpp 2014\9\16 13:27:30 $
 *  @author    ´ó·ð<dafo@mogujie.com>
 *  @brief     
 ******************************************************************************/

#include "stdafx.h"
#include "ServerConfigDialog.h"
#include "GlobalDefine.h"
#include "Modules/ISysConfigModule.h"
#include "Modules/IMiscModule.h"
#include "utility/utilStrCodeAPI.h"

DUI_BEGIN_MESSAGE_MAP(ServerConfigDialog, WindowImplBase)
	DUI_ON_MSGTYPE(DUI_MSGTYPE_WINDOWINIT, OnWindowInitialized)
	DUI_ON_MSGTYPE(DUI_MSGTYPE_CLICK, OnClick)
DUI_END_MESSAGE_MAP()

/******************************************************************************/

// -----------------------------------------------------------------------------
//  SettingDialog: Public, Constructor

ServerConfigDialog::ServerConfigDialog()
:m_pbtnApply(0)
{

}

// -----------------------------------------------------------------------------
//  SettingDialog: Public, Destructor

ServerConfigDialog::~ServerConfigDialog()
{

}

LPCTSTR ServerConfigDialog::GetWindowClassName() const
{
	return _T("TeamTalkServerConfigDialog");
}

DuiLib::CDuiString ServerConfigDialog::GetSkinFile()
{
	return  _T("SysConfigDialog\\ServerConfigDialog.xml");
}

CControlUI* ServerConfigDialog::CreateControl(LPCTSTR pstrClass)
{
	return nullptr;
}

void ServerConfigDialog::OnFinalMessage(HWND hWnd)
{
	WindowImplBase::OnFinalMessage(hWnd);
	delete this;
}

void ServerConfigDialog::OnWindowInitialized(TNotifyUI& msg)
{
	m_pbtnApply = (CButtonUI*)m_PaintManager.FindControl(_T("applybtn"));
	PTR_VOID(m_pbtnApply);

	m_pEditAddress = (CEditUI*)m_PaintManager.FindControl(_T("Address"));
	PTR_VOID(m_pEditAddress);

	_InitlizeSetting();
}

void ServerConfigDialog::OnClick(TNotifyUI& msg)
{
	PTR_VOID(msg.pSender);
	if (msg.pSender == m_pbtnApply)
	{
		if(_ApplySetting())
			Close(IDOK);
	}

	__super::OnClick(msg);
}

DuiLib::CDuiString ServerConfigDialog::GetSkinFolder()
{
	return _T("");
}

void ServerConfigDialog::_InitlizeSetting()
{
	APP_LOG(LOG_ERROR, _T("_InitlizeSetting,¶ÁÈ¡ÉèÖÃÊ§°Ü"));
	module::TTConfig* pTTConfig = module::getSysConfigModule()->getSystemConfig();
	if (!pTTConfig)
	{
		return;
	}

	CString& strPHPAdress = util::stringToCString(pTTConfig->phpServerAddr);
	if (!strPHPAdress.IsEmpty())
	{
		m_pEditAddress->SetText(strPHPAdress);
	}
}

BOOL ServerConfigDialog::_ApplySetting()
{
	module::TTConfig* pTTConfig = module::getSysConfigModule()->getSystemConfig();	
	CString strPHPAdress = m_pEditAddress->GetText();
	if (strPHPAdress.IsEmpty())
		return FALSE;
		
	pTTConfig->phpServerAddr = util::cStringToString(strPHPAdress);	
	return module::getSysConfigModule()->saveData();
}

/******************************************************************************/