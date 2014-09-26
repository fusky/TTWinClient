/******************************************************************************* 
 *  @file      FloatDialogManager.cpp 2014\8\27 17:45:43 $
 *  @author    ´ó·ð<dafo@mogujie.com>
 *  @brief     
 ******************************************************************************/

#include "stdafx.h"
#include "FloatDialogManager.h"

/******************************************************************************/

// -----------------------------------------------------------------------------
//  FloatDialogManager: Public, Constructor

FloatDialogManager::FloatDialogManager()
{

}

// -----------------------------------------------------------------------------
//  FloatDialogManager: Public, Destructor

FloatDialogManager::~FloatDialogManager()
{

}

FloatDialogManager* FloatDialogManager::getInstance()
{
	static FloatDialogManager manager;
	return &manager;
}

void FloatDialogManager::pushFloatWnd(IN FloatInfo info, IN HWND parentWnd)
{
	BOOL bFind = FALSE;
	for (FloatWndInfo infoItem : m_vecWndInfo)
	{
		if (infoItem.m_info.sId == info.sId)
		{
			bFind = TRUE;
			infoItem.m_pFloatDialog->UpdateContent(info);
			break;
		}
	}
	if (!bFind)
	{
		FloatDialog* pFloatWnd = new FloatDialog(info);
		if (pFloatWnd == NULL) return;
		pFloatWnd->Create(parentWnd, _T("FloatWnd")
			, UI_WNDSTYLE_FRAME | WS_THICKFRAME, WS_EX_TOOLWINDOW | WS_EX_TOPMOST | WS_EX_STATICEDGE
			, 0, 0, 0, 0);
		pFloatWnd->BringToTop();
		FloatWndInfo newWndInfo;
		newWndInfo.m_info = info;
		newWndInfo.m_pFloatDialog = pFloatWnd;
		util::TTAutoLock lock(&m_lock);
		m_vecWndInfo.push_back(newWndInfo);
	}
}

void FloatDialogManager::popFloatWnd(IN std::string sid)
{
	std::vector<FloatWndInfo>::iterator it = m_vecWndInfo.begin();
	for (; it != m_vecWndInfo.end(); ++it)
	{
		if (it->m_info.sId == sid)
		{
			util::TTAutoLock lock(&m_lock);
			m_vecWndInfo.erase(it);
			return;
		}
	}
}


/******************************************************************************/