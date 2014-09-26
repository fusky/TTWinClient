/*******************************************************************************
 *  @file      Parameters.h 2014\9\1 13:16:57 $
 *  @author    ¿ìµ¶<kuaidao@mogujie.com>
 *  @brief     
 ******************************************************************************/

#ifndef PARAMETERS_9EB51B1F_0166_4D46_8FEB_C6BA8C8D8BD6_H__
#define PARAMETERS_9EB51B1F_0166_4D46_8FEB_C6BA8C8D8BD6_H__

/******************************************************************************/

class CParameters
{
public:
	DWORD				m_dwPid;
	DWORD				m_dwTid;
	PEXCEPTION_POINTERS m_pClientExceptionPtr;
	CString				m_sParameter;
	CString				m_sRestartAppPath;

	CString				m_sCommandLine;

private:
	void ParseCommandLine();

public:
	static const CParameters& Instance()
	{
		static CParameters _instance;
		return _instance;
	}

	CParameters();
	~CParameters();
};
/******************************************************************************/
#endif// PARAMETERS_9EB51B1F_0166_4D46_8FEB_C6BA8C8D8BD6_H__