#include "stdafx.h"
#include "TTProtect.h"
#include "TTProtectDlg.h"
#include "afxdialogex.h"
#include "HttpPoster.h"
#include "ProcessKiller.h"
#include "GlobalDefine.h"
#include "sendRpt.h"
#include "utility/utilCommonAPI.h"
#include <Shlwapi.h>
#include <WinInet.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


CTTProtectDlg::CTTProtectDlg(CWnd* pParent /*=NULL*/)
: CDialogEx(CTTProtectDlg::IDD, pParent)
, m_bSendFinish(false)
, m_bClickOK(false)
, m_pSendRpt(0)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CTTProtectDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CHECK_RESTARTAPP, m_ctrRestartAppCheck);
	DDX_Control(pDX, IDC_CHECK_REPORT, m_ctrReportCheck);
}

BEGIN_MESSAGE_MAP(CTTProtectDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDOK, &CTTProtectDlg::OnBnClickedOk)
	ON_WM_CTLCOLOR()
	ON_MESSAGE(WM_SENDRPT_FINISH, &CTTProtectDlg::OnSendRptFinish)
	ON_WM_CLOSE()
	ON_MESSAGE(WM_START_CRASHDLG, OnStartCrash)
END_MESSAGE_MAP()

BOOL CTTProtectDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	InitUI();
	if (!m_crashMgr.CreateDumpFile())
	{
		OnCancel();
		return TRUE;
	}

	//先开始上传dump信息
	m_pSendRpt = new CSendRpt(&m_crashMgr, GetSafeHwnd());
	m_pSendRpt->create();

	return TRUE;
}

void CTTProtectDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

HCURSOR CTTProtectDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}
void CTTProtectDlg::OnBnClickedOk()
{
	CProcessKiller::KillTTProcesses();
	if (m_ctrRestartAppCheck.GetCheck())
	{
		m_crashMgr.RestartApp();
	}

	if (m_bSendFinish)
		Destory();
	else
	{
		m_bClickOK = TRUE;
		ShowWindow(SW_HIDE);
	}
}

HRESULT CTTProtectDlg::OnSendRptFinish(WPARAM wparam, LPARAM lparam)
{
	if (m_pSendRpt)
	{
		m_pSendRpt->Shutdown();
		m_pSendRpt->Release();
	}
	if (m_bClickOK)
		Destory();
	else
		m_bSendFinish = TRUE;

	return S_OK;
}

void CTTProtectDlg::InitUI()
{
	HMODULE hModule = (HMODULE)AfxGetApp()->m_hInstance;
	TCHAR lpFn[255];
	::GetModuleFileName(hModule, lpFn, 255);
	CString sIniFileName = lpFn;
	int iPos = sIniFileName.ReverseFind('\\');
	sIniFileName = sIniFileName.Left(iPos + 1);
	sIniFileName += _T("crashdumper.ini");

	CString sQuerryUrl, sPostUrl;
	if (util::isFileExist(sIniFileName))
	{
		CString sValue;
		m_iniConfig.SetFile(sIniFileName);
		sValue = m_iniConfig.GetString(_T("UI"), _T("title"));
		SetWindowText(sValue);
		sValue = m_iniConfig.GetString(_T("UI"), _T("CrashMsg2"));
		SetDlgItemText(IDC_STATIC_CRASHMSG2, sValue);
		sValue = m_iniConfig.GetString(_T("UI"), _T("RestartAppMsg"));
		SetDlgItemText(IDC_CHECK_RESTARTAPP, sValue);
		sValue = m_iniConfig.GetString(_T("UI"), _T("ReportBtnText"));
		SetDlgItemText(IDC_CHECK_REPORT, sValue);
		sValue = m_iniConfig.GetString(_T("UI"), _T("OkBtnText"));
		SetDlgItemText(IDOK, sValue);

		sQuerryUrl = m_iniConfig.GetString(_T("Network"), _T("HttpQueryUrl"));
		sPostUrl = m_iniConfig.GetString(_T("Network"), _T("HttpPostUrl"));
	}
	else
	{
		SetWindowText(_T("错误报告"));
		SetDlgItemText(IDC_STATIC_CRASHMSG2, _T("TeamTalk遇到未知错误，我们对此引起的不便表示歉意。我们已经产生了一个关于此错误的报告，我们希望您选择发送此报告给我们以帮助改善TeamTalk的质量。"));
		SetDlgItemText(IDC_CHECK_REPORT, _T("发送错误报告"));
		SetDlgItemText(IDC_CHECK_RESTARTAPP, _T("重启程序"));
		SetDlgItemText(IDOK, _T("确定"));

		sQuerryUrl = _T("");
		sPostUrl = _T("");
	}

	m_ctrRestartAppCheck.SetCheck(TRUE);
	m_ctrReportCheck.SetCheck(TRUE);

	m_crashMgr.Init(sQuerryUrl, sPostUrl);
}

void CTTProtectDlg::OnClose()
{
	OnBnClickedOk();
}

HBRUSH CTTProtectDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	/*if (nCtlColor == CTLCOLOR_DLG || nCtlColor == CTLCOLOR_STATIC)
	{
	hbr = (HBRUSH)::GetStockObject(WHITE_BRUSH);
	pDC->SetBkMode(TRANSPARENT);
	}*/

	return hbr;
}

void CTTProtectDlg::Destory()
{
	CString csDumps = m_crashMgr.GetDumpsDirName();
#ifdef REMOVE_DUMPS_FOLDER
	util::directoryOperation(csDumps, CString(), FO_DELETE);
#endif

	OnOK();
	return;
}

LRESULT CTTProtectDlg::OnStartCrash(WPARAM wparm, LPARAM lparam)
{
	::SetForegroundWindow(GetSafeHwnd());
	::ShowWindow(m_hWnd, SW_SHOW);

	return 0;
}