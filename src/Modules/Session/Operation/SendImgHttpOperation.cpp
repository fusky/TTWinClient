/******************************************************************************* 
 *  @file      SendImgOperation.cpp 2014\8\8 9:39:46 $
 *  @author    ´ó·ð<dafo@mogujie.com>
 *  @brief     
 ******************************************************************************/

#include "stdafx.h"
#include "SendImgHttpOperation.h"
#include "http/httpclient/http.h"
#include "utility/utilStrCodeAPI.h"
#include "cxImage/cxImage/ximage.h"
#include "Modules/ISysConfigModule.h"
#include "json/reader.h"
#include <Shlwapi.h>
/******************************************************************************/

// -----------------------------------------------------------------------------
//  SendImgOperation: Public, Constructor

SendImgHttpOperation::SendImgHttpOperation(SendImgParam& sendImgParam, logic::ICallbackHandler& callback)
:IHttpOperation(callback)
,m_sendImgParam(sendImgParam)
{

}

// -----------------------------------------------------------------------------
//  SendImgOperation: Public, Destructor

SendImgHttpOperation::~SendImgHttpOperation()
{

}

void SendImgHttpOperation::process()
{
	if (m_sendImgParam.csFilePath.IsEmpty())
	{
		APP_LOG(LOG_ERROR, _T("SendImgHttpOperation::process()Â·¾¶Îª¿Õ"));
		return;
	}
	module::TTConfig* pCfg = module::getSysConfigModule()->getSystemConfig();
	std::string& fileSysAddr = util::cStringToString(pCfg->fileSysAddr);
	std::string url = fileSysAddr + "upload/";
	Http::HttpResponse	response;
	Http::HttpClient	client;
	Http::HttpRequest	request("post", url);

	CString strExt = util::getFileExtName(m_sendImgParam.csFilePath);
	SendImgParam* pPamram = new SendImgParam();
	pPamram->m_result = SendImgParam::SENDIMG_OK;
	pPamram->csFilePath = m_sendImgParam.csFilePath;

	std::string sExtName = util::cStringToString(strExt);
	std::string sContentType = "image/" + sExtName;
	UInt32 width = 0, height = 0;
	_getImgSize(width,height);
	CString csSize;
	csSize.Format(_T("_%dx%d.%s"), width, height, strExt);
	std::string fileName = util::cStringToString(PathFindFileName(m_sendImgParam.csFilePath) + csSize);
	Http::HttpFileStream file(std::wstring(m_sendImgParam.csFilePath), fileName, sContentType);
	request.addFile("image", &file);

	client.execute(&request, &response);
	std::string header = response.getHeader();
	std::string body = response.getBody();
	std::string pathUrl;
	if (200 == response.getHttpCode() && _parseResponse(body, pathUrl))
	{
		pPamram->m_pathUrl = pathUrl;
	}
	else
	{
		CString csHeader = util::stringToCString(header, CP_UTF8);
		APP_LOG(LOG_ERROR, 1, _T("SendImgHttpOperation failed %s"), csHeader);
		pPamram->m_result = SendImgParam::SENDIMG_ERROR_UP;
	}

	asyncCallback(std::shared_ptr<void>(pPamram));
	client.killSelf();
}

void SendImgHttpOperation::release()
{
	delete this;
}

BOOL SendImgHttpOperation::_parseResponse(IN const std::string& body, OUT std::string& pathUrl)
{
	Json::Reader reader;
	Json::Value root;
	if (!reader.parse(body, root))
		return FALSE;

	UInt32 errCode = root.get("error_code", 0).asUInt();
	if (errCode != 0)
	{
		std::string errMsg = root.get("error_msg", "").asString();
		CString csErrMsg = util::stringToCString(errMsg);
		APP_LOG(LOG_ERROR, _T("SendImgHttpOperation sendImg errmsg:%s"),csErrMsg);
		return FALSE;
	}
	pathUrl = root.get("path", "").asString();
	if (pathUrl.empty())
		return FALSE;

	return TRUE;
}

void SendImgHttpOperation::_getImgSize(UInt32& width, UInt32& height)
{
	if (!util::isFileExist(m_sendImgParam.csFilePath))
	{
		APP_LOG(LOG_ERROR, _T("SendImgHttpOperation::_getImgSize,file not exist"));
		return;
	}
	CxImage cximage;
	bool bSucc = cximage.Load(m_sendImgParam.csFilePath);
	if (bSucc)
	{
		width = cximage.GetWidth();
		height = cximage.GetHeight();
	}
}

/******************************************************************************/