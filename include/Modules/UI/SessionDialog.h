#ifndef CHATDIALOG_HPP
#define CHATDIALOG_HPP

#include "GlobalDefine.h"
#include "UIEAUserTreelist.h"
#include "TTLogic/MKObserver.h"

class SessionLayout;
class SessionDialog : public WindowImplBase
{
public:

	SessionDialog(const std::string& sId);
	virtual ~SessionDialog();

public:
	LPCTSTR GetWindowClassName() const;	
	virtual void InitWindow();
	virtual LRESULT ResponseDefaultKeyEvent(WPARAM wParam);
	virtual CDuiString GetSkinFile();
	virtual CDuiString GetSkinFolder();
	virtual CControlUI* CreateControl(LPCTSTR pstrClass);
	void UpdateRunTimeMsg();
	virtual void OnFinalMessage(HWND hWnd);
	virtual LRESULT HandleCustomMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	void Notify(TNotifyUI& msg);
	void OnWindowInitialized(TNotifyUI& msg);
	virtual LRESULT OnSetFocus(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled);
	virtual LRESULT OnSysCommand(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	void OnSessionModuleEvent(UInt16 moduleId, UInt32 keyId, MKO_TUPLE_PARAM mkoParam);
	void OnSysConfigModuleEvent(UInt16 moduleId, UInt32 keyId, MKO_TUPLE_PARAM mkoParam);
	void OnTcpClientModuleEvent(UInt16 moduleId, UInt32 keyId, MKO_TUPLE_PARAM mkoParam);
	void OnLoginModuleEvent(UInt16 moduleId, UInt32 keyId, MKO_TUPLE_PARAM mkoParam);
	void OnUserListModuleEvent(UInt16 moduleId, UInt32 keyId, MKO_TUPLE_PARAM mkoParam);
	
	//语音播放动画操作
	BOOL StopPlayingAnimate(std::string& sAudioPlayingID);
	BOOL StartPlayingAnimate(std::string sAudioPlayingID);

private:
	void _ShakeWindow();
	void _FreshMyAvatar();

public:
	CButtonUI*			m_pBtnAvatar;
	CButtonUI*			m_pBtnMax;
	CButtonUI*			m_pBtnRestore;
	CTextUI*			m_pWritingStatus;
	CTextUI*			m_pTxtName;

	SessionLayout*		m_pSessionLayout;
	std::string			m_sId;		//会话Id
};

#endif // CHARTDIALOG_HPP