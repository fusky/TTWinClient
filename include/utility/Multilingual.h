/*******************************************************************************
 *  @file      Multilingual.h 2014\7\16 14:13:08 $
 *  @author    快刀<kuaidao@mogujie.com>
 *  @brief   多语言支持工具类
 ******************************************************************************/

#ifndef MULTILINGUAL_FA758BBE_9BB9_4638_945A_C8393D72AE51_H__
#define MULTILINGUAL_FA758BBE_9BB9_4638_945A_C8393D72AE51_H__

#include "utilityDll.h"
#include "GlobalDefine.h"
/******************************************************************************/
NAMESPACE_BEGIN(util)

/**
 * The class <code>Multilingual</code> 
 *
 */
class UTILITY_CLASS Multilingual
{
public:
    /** @name Constructors and Destructor*/

    //@{
    /**
     * Constructor 
     */
    Multilingual();
    /**
     * Destructor
     */
    ~Multilingual();
    //@}

public:
	CString getStringViaID(LPCTSTR strID);
	BOOL loadStringTable(LPCTSTR strFilePath);

private:
	BOOL _analyzeStringTable(CStringList& list);

private:
	CMapStringToString	m_mapKey2Value;
};
extern UTILITY_API Multilingual* getMultilingual();

NAMESPACE_END(util)
/******************************************************************************/
#endif// MULTILINGUAL_FA758BBE_9BB9_4638_945A_C8393D72AE51_H__
