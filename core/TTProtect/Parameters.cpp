/******************************************************************************* 
 *  @file      Parameters.cpp 2014\9\1 13:17:30 $
 *  @author    ¿ìµ¶<kuaidao@mogujie.com>
 *  @brief     
 ******************************************************************************/

#include "stdafx.h"
#include "Parameters.h"

/******************************************************************************/

CParameters::CParameters()
{
	m_sCommandLine = ::GetCommandLine();
	ParseCommandLine();
}

CParameters::~CParameters()
{

}

void CParameters::ParseCommandLine()
{
	if (m_sCommandLine.IsEmpty())
		return;

	int iPos = 0;
	int iPosEnd = 0;
	CString sValue;
	iPos = m_sCommandLine.Find(_T("pid="));
	if (iPos != -1)
	{
		iPosEnd = m_sCommandLine.Find(_T(";"), iPos);
		if (iPosEnd == -1)
			iPosEnd = m_sCommandLine.GetLength();
		sValue = m_sCommandLine.Mid(iPos + 4, iPosEnd - iPos - 4);
		m_dwPid = _tcstoul(sValue, NULL, 0);
	}


	iPos = m_sCommandLine.Find(_T("tid="));
	if (iPos != -1)
	{
		iPosEnd = m_sCommandLine.Find(_T(";"), iPos);
		if (iPosEnd == -1)
			iPosEnd = m_sCommandLine.GetLength();
		sValue = m_sCommandLine.Mid(iPos + 4, iPosEnd - iPos - 4);
		m_dwTid = _tcstoul(sValue, NULL, 0);
	}


	iPos = m_sCommandLine.Find(_T("excaddr="));
	if (iPos != -1)
	{
		iPosEnd = m_sCommandLine.Find(_T(";"), iPos);
		if (iPosEnd == -1)
			iPosEnd = m_sCommandLine.GetLength();
		sValue = m_sCommandLine.Mid(iPos + 8, iPosEnd - iPos - 8);
		m_pClientExceptionPtr = (PEXCEPTION_POINTERS)_tcstoul(sValue, NULL, 0);
	}

	iPos = m_sCommandLine.Find(_T("param="));
	if (iPos != -1)
	{
		iPosEnd = m_sCommandLine.Find(_T(";"), iPos);
		if (iPosEnd == -1)
			iPosEnd = m_sCommandLine.GetLength();
		m_sParameter = m_sCommandLine.Mid(iPos + 6, iPosEnd - iPos - 6);
	}

	iPos = m_sCommandLine.Find(_T("apppath="));
	if (iPos != -1)
	{
		iPosEnd = m_sCommandLine.Find(_T(";"), iPos);
		if (iPosEnd == -1)
			iPosEnd = m_sCommandLine.GetLength();
		m_sRestartAppPath = m_sCommandLine.Mid(iPos + 8, iPosEnd - iPos - 8);
	}
}

/******************************************************************************/