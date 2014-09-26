/******************************************************************************* 
 *  @file      IniParseTool.cpp 2014\7\16 14:13:45 $
 *  @author    大佛<dafo@mogujie.com>
 *  @brief   
 ******************************************************************************/

#include "stdafx.h"
#include "IniParseTool.h"
#include "Modules/IMiscModule.h"
/******************************************************************************/

// -----------------------------------------------------------------------------
//  IniParseTool: Public, Constructor

IniParseTool::IniParseTool()
{

}

// -----------------------------------------------------------------------------
//  IniParseTool: Public, Destructor

IniParseTool::~IniParseTool()
{
	
}

CString IniParseTool::getIDByName(LPCTSTR  str)
{
	CMapStringToString::CPair* pPair = m_mapKey2Value.PGetFirstAssoc();
	do
	{
		if (pPair&&pPair->value == str)
		{
			return pPair->key;
		}
		else
		{
			pPair = m_mapKey2Value.PGetNextAssoc(pPair);
		}
	} while (nullptr != pPair);

	return _T("");
}
CString IniParseTool::getNameByID(LPCTSTR strID)
{
	CString	strText;

	if (strID != NULL)
		m_mapKey2Value.Lookup(strID, strText);

	return strText;
}
BOOL IniParseTool::loadIniFile(LPCTSTR strFilePath)
{
	BOOL ret = FALSE;
	FILE* fp = NULL;
	CString	strVal;
	CStringList	strStringList;
	TCHAR buffer[4096];

	if (NULL == (fp = _tfopen(strFilePath, _T("rt,ccs=UTF-8"))))
	{
		goto leave;
	}

	while (_fgetts(buffer, sizeof(buffer) / sizeof(buffer[0]), fp))
	{
		strVal = buffer;
		strVal.Trim();
		if (!strVal.IsEmpty())
			strStringList.AddTail(strVal);
	}
	fclose(fp);

	_analyzeStringTable(strStringList);

	ret = TRUE;

leave:
	return ret;
}

BOOL IniParseTool::_analyzeStringTable(CStringList& list)
{
	int nStartPos = 0;
	int nComment;
	POSITION pos;

	for (pos = list.GetHeadPosition(); pos;)
	{
		CString& csLine = list.GetNext(pos);

		csLine.Trim();
		nComment = csLine.Find(_T(';'));
		if (nComment != -1)
			csLine = csLine.Left(nComment);

		nStartPos = 0;

		/* 取Key */
		CString& csKey = csLine.Tokenize(_T("="), nStartPos);
		csKey.Trim();
		if (!csKey.IsEmpty())
		{
			/* 取Value */
			CString& csValue = csLine.Tokenize(_T("="), nStartPos);
			csValue.Trim();
			if (!csValue.IsEmpty())
			{
				LPTSTR pch1, pch2, pch3;
				pch1 = (LPTSTR)(LPCTSTR)csValue;
				pch3 = pch1 + csValue.GetLength();

				/* 线性复杂度扫描，转义换行、回车 */
				for (pch2 = pch1; pch1 < pch3; pch1++)
				{
					if (*pch1 == _T('\\'))
					{
						if (*(pch1 + 1) == _T('r'))
						{
							pch1++;
							*(pch2++) = _T('\r');
						}
						else if (*(pch1 + 1) == _T('n'))
						{
							pch1++;
							*(pch2++) = _T('\n');
						}
						else if (*(pch1 + 1) == _T('t'))
						{
							pch1++;
							*(pch2++) = _T('\t');
						}
						else
						{
							*(pch2++) = _T('\\');
							*(pch2++) = *(++pch1);
						}
					}
					else
						*(pch2++) = *pch1;
				}

				*pch2 = 0;

				/* 向表格登记该二元组 */
				m_mapKey2Value.SetAt(csKey, csValue);
			}
		}
	}
	return TRUE;
}




/******************************************************************************/