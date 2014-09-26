/******************************************************************************* 
 *  @file      FloatWnd.cpp 2014\7\30 14:54:58 $
 *  @author    大佛<dafo@mogujie.com>
 *  @brief     
 ******************************************************************************/

#include "stdafx.h"
#include "FloatDialog.h"
#include "GlobalDefine.h"
#include "FloatDialogManager.h"
#include "utility/utilStrCodeAPI.h"
#include "Modules/ISessionModule.h"
#include "TTLogic/ITcpClientModule.h"

const UInt8 TIMER_BEGIN = 3;
DuiLib::CPoint FloatDialog::m_LastPos = DuiLib::CPoint(0, 0);
bool FloatDialog::m_Created = false;

DUI_BEGIN_MESSAGE_MAP(FloatDialog, WindowImplBase)
	DUI_ON_MSGTYPE(DUI_MSGTYPE_WINDOWINIT, OnPrepare)
	DUI_ON_MSGTYPE(DUI_MSGTYPE_CLICK,OnClick)
DUI_END_MESSAGE_MAP()

/******************************************************************************/

// -----------------------------------------------------------------------------
//  FloatWnd: Public, Constructor

FloatDialog::FloatDialog(FloatInfo floatInfo)
:m_iInterval(5000)
, m_pContentLayout(nullptr)
{
	m_floatInfo = floatInfo;
}

// -----------------------------------------------------------------------------
//  FloatWnd: Public, Destructor

FloatDialog::~FloatDialog()
{

}

LPCTSTR FloatDialog::GetWindowClassName() const
{
	return _T("FloatDialog");
}

DuiLib::CDuiString FloatDialog::GetSkinFile()
{
	return  _T("FloatDialog\\FloatDialog.xml");
}

DuiLib::CDuiString FloatDialog::GetSkinFolder()
{
	return _T("");
}

void FloatDialog::OnFinalMessage(HWND hWnd)
{
	FloatDialogManager::getInstance()->popFloatWnd(m_floatInfo.sId);
	WindowImplBase::OnFinalMessage(hWnd);
	m_Created = false;
	delete this;
}


LRESULT FloatDialog::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	//禁用双击标题栏最大化
	if (WM_NCLBUTTONDBLCLK == uMsg)
	{
		return 0;
	}
	//else if (WM_LBUTTONUP == uMsg)
	//{
	//	//通知主窗口创建会话
	//	logic::GetLogic()->asynNotifyObserver(module::KEY_SESSION_OPENNEWSESSION, m_floatInfo.sId);
	//}
	else if (WM_TIMER == uMsg)
	{
		TNotifyUI msg;
		msg.wParam = wParam;
		msg.lParam = lParam;
		OnTimer(msg);
		return 0;
	}
	return WindowImplBase::HandleMessage(uMsg, wParam, lParam);;
}

void FloatDialog::OnPrepare(TNotifyUI& msg)
{
	m_pBtLogo = dynamic_cast<CButtonUI*>(m_PaintManager.FindControl(_T("Avatar")));
	if (m_pBtLogo)
	{
		m_pBtLogo->SetBkImage(util::stringToCString(m_floatInfo.sAvatarPath));
	}
	m_pTextName = dynamic_cast<CTextUI*>(m_PaintManager.FindControl(_T("username")));
	if (m_pTextName)
	{
		m_pTextName->SetText(m_floatInfo.csUserName);
	}
	m_pTextContent = dynamic_cast<CTextUI*>(m_PaintManager.FindControl(_T("content")));
	if (m_pTextContent)
	{
		m_pTextContent->SetText(m_floatInfo.csMsgContent);
	}
	m_pContentLayout = dynamic_cast<CHorizontalLayoutUI*>(m_PaintManager.FindControl(_T("ContentLayout")));
}
void   FloatDialog::_GetSysPromptFormPos(DuiLib::CPoint& pt, int w, int h)
{
	//获取上一次的位置
	DuiLib::CPoint m_ptSysPromptForm = m_LastPos;
	int count = 0;
	int w1 = GetSystemMetrics(SM_CXFULLSCREEN);
	int h1 = GetSystemMetrics(SM_CYFULLSCREEN);
	pt.x = 0;
	pt.x = 0;


	RECT rWorkArea;
	BOOL bResult = SystemParametersInfo(SPI_GETWORKAREA, sizeof(RECT), &rWorkArea, 0);

	if (!bResult)
	{//如果调用不成功就利用GetSystemMetrics获取屏幕面积 
		rWorkArea.top = 0;
		rWorkArea.left = 0;
		rWorkArea.right = w1;
		rWorkArea.bottom = h1;
	}
	//如果有存在，则拿最后一个的数据
	if (m_Created)
	{
		//如果已经超过了上边界,则放到最下面
		if (m_ptSysPromptForm.y - h - 10 < rWorkArea.top)
		{
			pt.y = rWorkArea.bottom;
		}
		else{
			pt.y = m_ptSysPromptForm.y - 2;

		}

	}
	else{//如果没有，则从底下开始
		pt.y = rWorkArea.bottom;
	}
	pt.x = rWorkArea.right - w;
	//记住下个窗口应该的起始点
	m_ptSysPromptForm.x = pt.x;
	m_ptSysPromptForm.y = pt.y - h;
	m_LastPos = m_ptSysPromptForm;
	//
	if (pt.x < 0)
	{
		pt.x = 0;
	}
	if (pt.y < 0)
	{
		pt.y = 0;
	}

	
}

/**
 * 激活窗口
 *
 * @param   void
 * @return  void
 * @exception there is no any exception to throw.
 */
void FloatDialog::BringToTop(void)
{
	//CPaintManagerUI::SetTimer(this, TIMER_BEGIN, 10);
	SetTimer(m_hWnd, TIMER_BEGIN, 10, nullptr);
}

void FloatDialog::OnTimer(TNotifyUI& msg)
{
	if (msg.wParam == TIMER_BEGIN)
	{
		KillTimer(m_hWnd, TIMER_BEGIN);
		_BringToTop();
	}
	else if (msg.wParam == 2)
	{
		CRect rect;
		GetClientRect(m_hWnd,rect);
		DuiLib::CPoint curPoint;
		GetCursorPos(&curPoint);
		ScreenToClient(m_hWnd,&curPoint);
		if (rect.PtInRect(curPoint))return;
		KillTimer(m_hWnd,2);
		if (m_nOffset <= 0)
		{
			PostMessage(WM_CLOSE);
			return;
		}
		else
		{
			m_nOffset -= 20;
			::MoveWindow(m_hWnd, m_rcWnd.left, m_rcWnd.top - m_nOffset, m_rcWnd.Width(),m_rcWnd.Height(),TRUE);
//			CPaintManagerUI::SetTimer(this, 2, 30);
			SetTimer(m_hWnd, 2, 30, nullptr);
		}
	}
	else if (1 == msg.wParam)
	{
		KillTimer(m_hWnd,1);
		if (m_nOffset >= m_rcWnd.Height())
		{
			if (m_iInterval > 0)
				//				CPaintManagerUI::SetTimer(this, 2, m_iInterval);
				SetTimer(m_hWnd, 2, m_iInterval, nullptr);
			return;
		}
		else
		{
			if (m_nOffset + 20 > m_rcWnd.Height())
			{
				m_nOffset = m_rcWnd.Height();
			}
			else
			{
				m_nOffset += 20;
			}
			::MoveWindow(m_hWnd, m_rcWnd.left, m_rcWnd.top - m_nOffset,m_rcWnd.Width(),m_rcWnd.Height(),TRUE);
			//SetFormPosition(CRect(m_rcWnd.left, m_rcWnd.top - m_nOffset, m_rcWnd.right, m_rcWnd.bottom - m_nOffset));

			if (m_nOffset < 20 * 2)
			{
		//TODO		SetWindowPos(&wndTopMost, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_SHOWWINDOW | SWP_NOOWNERZORDER);
			}

			//Refresh();
//			CPaintManagerUI::SetTimer(this, 1, 30);
			SetTimer(m_hWnd, 1, 20, nullptr);
		}
	}
}

void FloatDialog::_BringToTop()
{
//	CPaintManagerUI::SetTimer(this, 1, 10);
	SetTimer(m_hWnd, 1, 10, nullptr);
}

void FloatDialog::InitWindow()
{
	DuiLib::CPoint pt;
	int screenWidth = GetSystemMetrics(SM_CXSCREEN);
	int screenHeight = GetSystemMetrics(SM_CYSCREEN);
	int posindex = 0;
	CRect rc;
	::GetClientRect(m_hWnd, &rc);
	int w = rc.Width();
	int h = rc.Height();
	_GetSysPromptFormPos(pt, w, h);
	CRect rcMainFrm(pt.x, pt.y, pt.x + w, pt.y + h);

	m_rcWnd = CRect(rcMainFrm.left, rcMainFrm.top, rcMainFrm.right, rcMainFrm.bottom);
	m_nOffset = 0;
	m_Created = true;
}

void FloatDialog::UpdateContent(FloatInfo floatInfo)
{
	if (!IsWindow(m_hWnd))
	{
		return;
	}
	if (m_pBtLogo)
	{
		m_pBtLogo->SetBkImage(util::stringToCString(floatInfo.sAvatarPath));
	}
	if (m_pTextName)
	{
		m_pTextName->SetText(floatInfo.csUserName);
	}
	if (m_pTextContent)
	{
		m_pTextContent->SetText(floatInfo.csMsgContent);
	}
	m_PaintManager.NeedUpdate();
}

void FloatDialog::OnClick(TNotifyUI& msg)
{
	if (msg.pSender == m_pBtLogo
		|| m_pTextName == msg.pSender
		|| m_pTextContent == msg.pSender)
	{
		//通知主窗口创建会话
		logic::GetLogic()->asynNotifyObserver(module::KEY_SESSION_OPENNEWSESSION, m_floatInfo.sId);
		//停止托盘闪烁
		logic::GetLogic()->asynNotifyObserver(module::TAG_SESSION_TRAY_STOPEMOT);
		return;
	}
	__super::OnClick(msg);
}




/******************************************************************************/