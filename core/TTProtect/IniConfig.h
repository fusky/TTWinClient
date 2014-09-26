/*******************************************************************************
 *  @file      IniConfig.h 2014\9\1 13:15:52 $
 *  @author    ¿ìµ¶<kuaidao@mogujie.com>
 *  @brief     
 ******************************************************************************/

#ifndef INICONFIG_C0F81335_CB24_4C2D_BA6C_0E00FA00A17D_H__
#define INICONFIG_C0F81335_CB24_4C2D_BA6C_0E00FA00A17D_H__

/******************************************************************************/

class CIniConfig
{
private:
	CString m_sIniFile;
public:
	void SetString(CString sSection, CString sItem, CString sVal);
	CString GetString(CString sSection, CString sItem);
	void SetInt(CString sSection, CString sItem, int iVal);
	int GetInt(CString sSection, CString sItem);
	void SetFile(CString sFile);
	CIniConfig(CString sFile);
	CIniConfig(void);
	~CIniConfig(void);
};

/******************************************************************************/
#endif// INICONFIG_C0F81335_CB24_4C2D_BA6C_0E00FA00A17D_H__