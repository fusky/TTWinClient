/*******************************************************************************
 *  @file      IniParseTool.h 2014\7\16 14:13:08 $
 *  @author    ´ó·ð<dafo@mogujie.com>
 *  @brief     
 ******************************************************************************/

#ifndef INIPARSETOOL_FA758BBE_9BB9_4638_945A_C8393D72AE51_H__
#define INIPARSETOOL_FA758BBE_9BB9_4638_945A_C8393D72AE51_H__


#include "GlobalDefine.h"
/******************************************************************************/

/**
 * The class <code>Multilingual</code> 
 *
 */
class  IniParseTool
{
public:
    /** @name Constructors and Destructor*/

    //@{
    /**
     * Constructor 
     */
	IniParseTool();
    /**
     * Destructor
     */
	~IniParseTool();
    //@}

public:
	CString	   getIDByName(LPCTSTR strID);
	CString    getNameByID(LPCTSTR str);

	BOOL	   loadIniFile(LPCTSTR strFilePath);
private:
	BOOL	   _analyzeStringTable(CStringList& list);

private:
	CMapStringToString	m_mapKey2Value;
};

/******************************************************************************/
#endif// INIPARSETOOL_FA758BBE_9BB9_4638_945A_C8393D72AE51_H__
