/*******************************************************************************
 *  @file      FloatDialog.h 2014\7\30 14:54:00 $
 *  @author    大佛<dafo@mogujie.com>
 *  @brief     
 ******************************************************************************/

#ifndef FLOATWND_050F52B6_9D64_4E05_8E01_984E3EB0A3FE_H__
#define FLOATWND_050F52B6_9D64_4E05_8E01_984E3EB0A3FE_H__

#include "DuiLib/UIlib.h"
/******************************************************************************/
using namespace DuiLib;

// 飘窗信息结构
class  FloatInfo
{
public:
	std::string     sId;                        //用户Id
	std::string     sAvatarPath;                //头像下载成功后的存储路径
	CString         csUserName;                 //用户名
	CString         csMsgContent;               //消息内容
};

/**
 * The class <code>FloatWnd</code> 
 *
 */
class FloatDialog : public WindowImplBase
{
public:
    /** @name Constructors and Destructor*/

    //@{
    /**
     * Constructor 
     */
	FloatDialog(FloatInfo floatInfo);
    /**
     * Destructor
     */
    virtual ~FloatDialog();
    //@}
	DUI_DECLARE_MESSAGE_MAP()
public:
	LPCTSTR GetWindowClassName() const;
	virtual CDuiString GetSkinFile();
	virtual CDuiString GetSkinFolder();
	virtual void InitWindow();
	virtual void OnFinalMessage(HWND hWnd);
	virtual LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
public:
	void BringToTop(void);
	void UpdateContent(FloatInfo floatInfo);
protected:
	void OnPrepare(TNotifyUI& msg);
	void OnTimer(TNotifyUI& msg);
	void OnClick(TNotifyUI& msg);
private:
	void _BringToTop();
	void _GetSysPromptFormPos(DuiLib::CPoint& pt, int w, int h);
private:
	CRect						m_rcWnd;
	int							m_nOffset;
	static DuiLib::CPoint       m_LastPos;
	int							m_iInterval;
	static bool					m_Created;
	FloatInfo					m_floatInfo;

	CButtonUI*					m_pBtLogo;
	CTextUI*					m_pTextName;
	CTextUI*					m_pTextContent;
	CHorizontalLayoutUI*		m_pContentLayout;
};
/******************************************************************************/
#endif// FLOATWND_050F52B6_9D64_4E05_8E01_984E3EB0A3FE_H__

