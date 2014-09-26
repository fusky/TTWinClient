/******************************************************************************* 
 *  @file      EmotionModule_Impl.cpp 2014\8\6 20:07:17 $
 *  @author    ¿ìµ¶<kuaidao@mogujie.com>
 *  @brief     
 ******************************************************************************/

#include "stdafx.h"
#include "EmotionModule_Impl.h"
#include "EmotionDialog.h"
#include "Modules/IMiscModule.h"
/******************************************************************************/
namespace module
{
	module::IEmotionModule* getEmotionModule()
	{
		return (module::IEmotionModule*)logic::GetLogic()->getModule(MODULE_ID_EMOTION);
	}
}

// -----------------------------------------------------------------------------
//  EmotionModule_Impl: Public, Constructor

EmotionModule_Impl::EmotionModule_Impl()
:m_emotionWindow(0)
,m_pEmotionKey(nullptr)
{
	if (nullptr == m_pEmotionKey)
	{
		m_pEmotionKey = new IniParseTool();
		m_pEmotionKey->loadIniFile(module::getMiscModule()->getDataDir() + _T("Emotion\\sysemots.ini"));
	}
}

// -----------------------------------------------------------------------------
//  EmotionModule_Impl: Public, Destructor

EmotionModule_Impl::~EmotionModule_Impl()
{

}

void EmotionModule_Impl::showEmotionDialog(IN std::string sid, IN POINT pt)
{
	m_sid = sid;
	if (!m_emotionWindow)
	{
		m_emotionWindow = new EmotionDialog;
		m_emotionWindow->Create(NULL, _T("EmotionWnd")
			, UI_WNDSTYLE_FRAME | WS_POPUP, WS_EX_TOOLWINDOW | WS_EX_TOPMOST | WS_EX_STATICEDGE
			, 0, 0, 0, 0);
		m_emotionWindow->ShowWindow(pt, TRUE);
		return;
	}
	if (::IsWindowVisible(m_emotionWindow->GetHWND()))
		m_emotionWindow->ShowWindow(pt, FALSE);
	else
		m_emotionWindow->ShowWindow(pt, TRUE);


}

void EmotionModule_Impl::release()
{
	if (m_pEmotionKey)
	{
		delete m_pEmotionKey;
		m_pEmotionKey = nullptr;
	}
	delete this;
}

BOOL EmotionModule_Impl::getEmotionNameByID(IN CString ID, OUT CString& csPath)
{
	if (m_pEmotionKey)
	{
		CString csName = m_pEmotionKey->getNameByID(ID);
		if (!csName.IsEmpty())
		{
			csPath = module::getMiscModule()->getEmotionFilesDir() + csName;
			return TRUE;
		}		
	}
	return FALSE;
}

std::string EmotionModule_Impl::getCurEmotionWndSessionId(void)
{
	return m_sid;
}

BOOL EmotionModule_Impl::getEmotionIDByName(IN CString csName, OUT CString& csID)
{
	if (m_pEmotionKey)
	{
		csID = m_pEmotionKey->getIDByName(csName);
		if (!csID.IsEmpty())
			return TRUE;
	}
	return FALSE;
}

/******************************************************************************/