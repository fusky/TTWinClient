#include "stdafx.h"
#include "../SessionManager.h"
#include "Modules/UI/SessionLayout.h"
#include "Modules/UI/SessionDialog.h"
#include "Modules/UI/UIRecentSessionList.h"
#include "Modules/ISysConfigModule.h"
#include "Modules/ISessionModule.h"
#include "Modules/ILoginModule.h"
#include "Modules/IMessageModule.h"
#include "Modules/IUserListModule.h"
#include "UIIMEdit.h"
#include "TTLogic/ILogic.h"
#include "TTLogic/ITcpClientModule.h"
#include "utility/utilStrCodeAPI.h"

const int kEmotionRefreshTimerId = 1001;
const int kEmotionRefreshInterval = 150;

#define  TIMER_CHECK_RECEIVE_WRITING_STATUS 2

DWORD WINAPI  WindowShake(LPVOID lpParam)//窗口抖动
{
	HWND hwnd = (HWND)lpParam;
	RECT rect;
	memset(&rect, 0, sizeof(rect));
	GetWindowRect(hwnd, &rect);
	for (int i = 0; i < 10; i++)
	{
		MoveWindow(hwnd, rect.left - 6, rect.top, rect.right - rect.left, rect.bottom - rect.top, true);
		Sleep(30);
		MoveWindow(hwnd, rect.left, rect.top - 5, rect.right - rect.left, rect.bottom - rect.top, true);
		Sleep(30);
		MoveWindow(hwnd, rect.left + 5, rect.top, rect.right - rect.left, rect.bottom - rect.top, true);
		Sleep(30);
		MoveWindow(hwnd, rect.left, rect.top + 5, rect.right - rect.left, rect.bottom - rect.top, true);
		Sleep(30);
		MoveWindow(hwnd, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, true);
	}
	return 0;
}


SessionDialog::SessionDialog(const std::string& sId)
: m_pSessionLayout(nullptr)
, m_pWritingStatus(nullptr)
, m_sId(sId)
, m_pBtnAvatar(nullptr)
, m_pTxtName(nullptr)
, m_pBtnMax(nullptr)
, m_pBtnRestore(nullptr)
{

}

SessionDialog::~SessionDialog()
{
}

LPCTSTR SessionDialog::GetWindowClassName() const
{
	return _T("SessionDialog");
}

CControlUI* SessionDialog::CreateControl(LPCTSTR pstrClass)
{
	if (_tcsicmp(pstrClass, _T("SessionLayout")) == 0)
	{
		m_pSessionLayout = new SessionLayout(m_sId,m_PaintManager);
		return m_pSessionLayout;
	}

	if (_tcsicmp(pstrClass, _T("GroupMemberList")) == 0)
	{
		return new CUIRecentSessionList(m_PaintManager);
	}
	if (_tcsicmp(pstrClass, _T("UIIMEdit")) == 0)
	{
		return new UIIMEdit();
	}
	return NULL;
}

void SessionDialog::OnFinalMessage(HWND hWnd)
{
	SessionDialogManager::getInstance()->closeSessionDialog(m_sId);
	logic::GetLogic()->removeObserver(this);
	module::getMessageModule()->clearMsgOffset(m_sId);
	WindowImplBase::OnFinalMessage(hWnd);
	delete this;
}

CDuiString SessionDialog::GetSkinFile()
{
	return _T("SessionDialog\\SessionDialog.xml");
}

CDuiString SessionDialog::GetSkinFolder()
{
	return _T("");
}

LRESULT SessionDialog::ResponseDefaultKeyEvent(WPARAM wParam)
{
	if (wParam == VK_RETURN)
	{
		return FALSE;
	}
	else if (wParam == VK_ESCAPE)
	{
		Close(IDCANCEL);
		return TRUE;
	}
	return FALSE;
}

void SessionDialog::InitWindow()
{
	SessionEntity* pSessionInfo = SessionEntityManager::getInstance()->getSessionEntityBySId(m_sId);
	if (!pSessionInfo)
		return;
	//群的话，从新计算大小
	if (SESSION_GROUPTYPE == pSessionInfo->m_sessionType)
	{
		ResizeClient(720,540);
	}
}

void SessionDialog::OnWindowInitialized(TNotifyUI& msg)
{
	SessionEntity* pSessionInfo = SessionEntityManager::getInstance()->getSessionEntityBySId(m_sId);
	if (!pSessionInfo)
		return;

	m_pBtnAvatar = (CButtonUI*)m_PaintManager.FindControl(_T("UserAvatar"));
	PTR_VOID(m_pBtnAvatar);
	m_pBtnAvatar->SetBkImage(util::stringToCString(pSessionInfo->getAvatarPath()));

	m_pBtnMax = (CButtonUI*)m_PaintManager.FindControl(_T("maxbtn"));
	PTR_VOID(m_pBtnMax);
	m_pBtnRestore = (CButtonUI*)m_PaintManager.FindControl(_T("restorebtn"));
	PTR_VOID(m_pBtnRestore);

	m_pTxtName = (CTextUI*)m_PaintManager.FindControl(_T("username"));
	PTR_VOID(m_pTxtName);
	m_pTxtName->SetText(pSessionInfo->getName());

	m_pWritingStatus = (CTextUI*)m_PaintManager.FindControl(_T("writingStatus"));
	PTR_VOID(m_pWritingStatus);

	logic::GetLogic()->addObserver(this, MODULE_ID_SEESION
		, fastdelegate::MakeDelegate(this, &SessionDialog::OnSessionModuleEvent));
	logic::GetLogic()->addObserver(this, MODULE_ID_SYSCONFIG
		, fastdelegate::MakeDelegate(this, &SessionDialog::OnSysConfigModuleEvent));
	logic::GetLogic()->addObserver(this, MODULE_ID_TCPCLIENT
		, fastdelegate::MakeDelegate(this, &SessionDialog::OnTcpClientModuleEvent));
	logic::GetLogic()->addObserver(this, MODULE_ID_LOGIN
		, fastdelegate::MakeDelegate(this, &SessionDialog::OnLoginModuleEvent));
	logic::GetLogic()->addObserver(this, MODULE_ID_USERLIST
		, fastdelegate::MakeDelegate(this, &SessionDialog::OnUserListModuleEvent));
}

void SessionDialog::Notify(TNotifyUI& msg)
{
	if (msg.sType == DUI_MSGTYPE_WINDOWINIT)
	{
		OnWindowInitialized(msg);
	}
	else if (msg.sType == DUI_MSGTYPE_CLICK)
	{
		if (msg.pSender == m_pBtnAvatar)
		{
		}
	}
	__super::Notify(msg);
}

void SessionDialog::UpdateRunTimeMsg()
{
	if (m_pSessionLayout)
	{
		m_pSessionLayout->UpdateRunTimeMsg();
	}
}

void SessionDialog::OnSessionModuleEvent(UInt16 moduleId, UInt32 keyId, MKO_TUPLE_PARAM mkoParam)
{
	if (module::KEY_SESSION_NEWMESSAGE == keyId)
	{
		std::string& sId = std::get<MKO_STRING>(mkoParam);

		//任务栏消息提示动画
		if (m_sId == sId && ::GetForegroundWindow() != m_hWnd)
		{
			FLASHWINFO flashInfo;
			flashInfo.cbSize = sizeof(FLASHWINFO);
			flashInfo.dwFlags = FLASHW_TRAY;
			flashInfo.dwTimeout = 0;
			flashInfo.uCount = 2;
			flashInfo.hwnd = m_hWnd;
			::FlashWindowEx(&flashInfo);
		}
	}
	else if (module::KEY_SESSION_WRITING_MSG == keyId)	//对方正在输入
	{
		std::string& sId = std::get<MKO_STRING>(mkoParam);
		if (m_sId == sId)
		{
			m_pWritingStatus->SetVisible(true);
			KillTimer(m_hWnd, TIMER_CHECK_RECEIVE_WRITING_STATUS);
			SetTimer(m_hWnd, TIMER_CHECK_RECEIVE_WRITING_STATUS, 6000, NULL);
		}
	}
	else if (module::KEY_SESSION_STOPWRITING_MSG == keyId)//对方停止输入
	{
		std::string& sId = std::get<MKO_STRING>(mkoParam);
		if (m_sId == sId)
		{
			m_pWritingStatus->SetVisible(false);
			KillTimer(m_hWnd, TIMER_CHECK_RECEIVE_WRITING_STATUS);
		}
	}
	else if (module::KEY_SESSION_SHAKEWINDOW_MSG == keyId)
	{
		std::string& sId = std::get<MKO_STRING>(mkoParam);
		if (sId == m_sId)
		{
			_ShakeWindow();
		}
	}

}
void SessionDialog::OnSysConfigModuleEvent(UInt16 moduleId, UInt32 keyId, MKO_TUPLE_PARAM mkoParam)
{
	if (module::KEY_SYSCONFIG_UPDATED == keyId)
	{
		if (m_pSessionLayout)
		{
			m_pSessionLayout->UpdateSendMsgKey();
		}
	}
}
void SessionDialog::_ShakeWindow()
{
	BringToTop();
	if (IsWindow(m_hWnd))
	{
		HANDLE hThread = CreateThread(NULL, 0, WindowShake, m_hWnd, 0, NULL);//窗口抖动
		CloseHandle(hThread);
	}
}

LRESULT SessionDialog::HandleCustomMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if (WM_TIMER == uMsg)
	{
		if (wParam == TIMER_CHECK_RECEIVE_WRITING_STATUS)
		{
			m_pWritingStatus->SetVisible(false);
			KillTimer(m_hWnd, TIMER_CHECK_RECEIVE_WRITING_STATUS);
		}
	}
	return __super::HandleCustomMessage(uMsg, wParam, lParam, bHandled);
}

void SessionDialog::OnTcpClientModuleEvent(UInt16 moduleId, UInt32 keyId, MKO_TUPLE_PARAM mkoParam)
{
	if (logic::KEY_TCPCLIENT_STATE == keyId)
	{
		_FreshMyAvatar();
		m_pSessionLayout->FreshAllGroupMemberAvatar();
	}
}

void SessionDialog::OnLoginModuleEvent(UInt16 moduleId, UInt32 keyId, MKO_TUPLE_PARAM mkoParam)
{
	if (module::KEY_LOGIN_RELOGINOK == keyId)
	{
		_FreshMyAvatar();
		m_pSessionLayout->FreshAllGroupMemberAvatar();
	}
}

void SessionDialog::_FreshMyAvatar()
{
	//刷新个人在线状态，群不刷新
	SessionEntity* pSession = SessionEntityManager::getInstance()->getSessionEntityBySId(m_sId);
	if (pSession && SESSION_USERTYPE == pSession->m_sessionType)
	{
		m_pBtnAvatar->SetBkImage(util::stringToCString(pSession->getAvatarPath()));
	}
}

void SessionDialog::OnUserListModuleEvent(UInt16 moduleId, UInt32 keyId, MKO_TUPLE_PARAM mkoParam)
{
	if (module::KEY_USERLIST_USERLINESTATE == keyId)
	{
		//刷新个人在线状态，群不刷新
		std::string& sId = std::get<MKO_STRING>(mkoParam);
		if (sId == m_sId)
		{
			_FreshMyAvatar();
		}
		else
		{
			m_pSessionLayout->FreshGroupMemberAvatar(sId);
		}
	}
}

BOOL SessionDialog::StopPlayingAnimate(std::string& sAudioPlayingID)
{
	if (m_pSessionLayout)
	{
		 return m_pSessionLayout->StopPlayingAnimate(sAudioPlayingID);
	}
	return FALSE;
}

BOOL SessionDialog::StartPlayingAnimate(std::string sAudioPlayingID)
{
	if (m_pSessionLayout)
	{
		return m_pSessionLayout->StartPlayingAnimate(sAudioPlayingID);
	}
	return FALSE;
}

LRESULT SessionDialog::OnSetFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	m_pSessionLayout->m_pInputRichEdit->SetFocus();//bug，切换的时候，第一次不能获得焦点
	return __super::OnSetFocus(uMsg, wParam, lParam, bHandled);
}

LRESULT SessionDialog::OnSysCommand(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	BOOL bZoomed = ::IsZoomed(m_hWnd);
	LRESULT lRes = CWindowWnd::HandleMessage(uMsg, wParam, lParam);
	if (::IsZoomed(m_hWnd) != bZoomed)
	{
		if (!bZoomed)
		{
			if (m_pBtnMax) m_pBtnMax->SetVisible(false);
			if (m_pBtnRestore) m_pBtnRestore->SetVisible(true);
		}
		else
		{
			if (m_pBtnMax) m_pBtnMax->SetVisible(true);
			if (m_pBtnRestore) m_pBtnRestore->SetVisible(false);
		}
	}
	return __super::OnSysCommand(uMsg, wParam, lParam, bHandled);
}



