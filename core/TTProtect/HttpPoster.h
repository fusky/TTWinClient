/*******************************************************************************
 *  @file      HttpPoster.h 2014\9\1 12:01:48 $
 *  @author    ¿ìµ¶<kuaidao@mogujie.com>
 *  @brief     
 ******************************************************************************/

#ifndef HTTPPOSTER_B826C96E_63D6_49F2_8BF8_CF0F3A835F5E_H__
#define HTTPPOSTER_B826C96E_63D6_49F2_8BF8_CF0F3A835F5E_H__

#include <map>
#include <WinInet.h>

/******************************************************************************/
namespace _internal{
	class CInetHandle
	{
	public:
		CInetHandle(HINTERNET src = NULL)
			: m_hInet(src)
		{
		}
		virtual ~CInetHandle()
		{
			Close();
		}
		inline void Close()
		{
			if (NULL != m_hInet)
			{
				::InternetCloseHandle(m_hInet);
				m_hInet = NULL;
			}
		}
		inline operator HINTERNET() const
		{
			return m_hInet;
		}
		void Attach(HINTERNET src)
		{
			if (src != m_hInet)
			{
				Close();
				m_hInet = src;
			}
		}
		HINTERNET Dettach()
		{
			HINTERNET _hInet = m_hInet;
			m_hInet = NULL;
			return _hInet;
		}
	protected:
		HINTERNET m_hInet;
	private:
		CInetHandle(const CInetHandle& src);
		CInetHandle& operator= (const CInetHandle rhs);
	};
};

class CHttpPoster
{
public:
	CHttpPoster();
	~CHttpPoster();
	BOOL PostFile(LPCTSTR lpszUrl, LPCTSTR lpszFileName, LPCTSTR lpszUserName = NULL, LPCTSTR lpszPassword = NULL);
	int QueryUrlStatusCode(CString sUrl);
	void Cancel();
private:
	enum { MAX_BUF_SIZE = 1024 * 1024, DEF_BUF_SIZE = 4096, MIN_BUF_SIZE = 512 };
	_internal::CInetHandle m_InetSession;
	_internal::CInetHandle m_hConnect;
	_internal::CInetHandle m_hRequest;
	CString m_FileName;
	HANDLE m_hFile;
	CString m_Url;
	INTERNET_SCHEME m_UrlScheme;
	CString m_HostName;
	UINT m_Port;
	CString m_HostPath;
	CString m_UserName;
	CString m_Password;
	CString m_ExtraInfo;
	volatile BOOL m_bCancelled;

	void CrackUrl(LPCTSTR lpszUserName, LPCTSTR lpszPassword);
	void OpenFile();
	void BeginRequest();
	void SetHeaders();
	void SendRequestExWithCA(DWORD nDataSize);
	void TestRequest();
	void SendFile();

	DWORD CalcSentBufferSize(DWORD nTickSpan, DWORD nSent, DWORD nOldSize);
	void CheckError(BOOL bValue, LPCTSTR lpszFmt, ...);

};

/******************************************************************************/
#endif// HTTPPOSTER_B826C96E_63D6_49F2_8BF8_CF0F3A835F5E_H__