/******************************************************************************* 
 *  @file      DownloadImgHttpOperation.cpp 2014\8\14 10:19:07 $
 *  @author    快刀<kuaidao@mogujie.com>
 *  @brief     
 ******************************************************************************/

#include "stdafx.h"
#include "DownloadImgHttpOperation.h"
#include "http/httpclient/http.h"
#include "Modules/IMiscModule.h"
#include "Modules/IDatabaseModule.h"
#include "Modules/ISysConfigModule.h"
#include "Modules/ICaptureModule.h"
#include "Modules/IUserListModule.h"
#include "utility/utilStrCodeAPI.h"
#include "cxImage/cxImage/ximage.h"
/******************************************************************************/

// -----------------------------------------------------------------------------
//  DownloadImgHttpOperation: Public, Constructor

DownloadImgHttpOperation::DownloadImgHttpOperation(std::string sId, std::string& downUrl, BOOL bGrayScale
	, logic::ICallbackHandler& callback)
:IHttpOperation(callback)
,m_downUrl(downUrl)
,m_sId(sId)
,m_bGrayScale(bGrayScale)
{

}

// -----------------------------------------------------------------------------
//  DownloadImgHttpOperation: Public, Destructor

DownloadImgHttpOperation::~DownloadImgHttpOperation()
{

}

void DownloadImgHttpOperation::process()
{
	UInt32 hashcode = util::hash_BKDR(m_downUrl.c_str());
	module::TTConfig* pCfg = module::getSysConfigModule()->getSystemConfig();
	std::string& fileSysAddr = util::cStringToString(pCfg->fileSysAddr);
	std::string url = fileSysAddr + m_downUrl;
	Http::HttpResponse	response;
	Http::HttpClient	client;
	Http::HttpRequest	request("GET", url);

	//下载资源
	CString extName = util::getFileExtName(util::stringToCString(m_downUrl));
	CString csFileName = util::int32ToCString(hashcode) + _T(".") + extName;
	CString csLocalPath = module::getMiscModule()->getDownloadDir() + csFileName;
	std::wstring cs = csLocalPath;
	request.saveToFile(cs);
	if (!client.execute(&request, &response))
	{
		CString csTemp = util::stringToCString(url, CP_UTF8);
		APP_LOG(LOG_ERROR, 1, _T("DownloadImgHttpOperation failed %s"), csTemp);
		client.killSelf();
		return;
	}
	client.killSelf();

	if (util::isFileExist(csLocalPath))
	{
		DownloadImgParam* pParam = new DownloadImgParam;
		pParam->m_sId = m_sId;
		pParam->m_result = DownloadImgParam::DOWNLOADIMG_OK;

		//存入ImImage表
		std::string localPath = util::cStringToString(csFileName);
		module::ImImageEntity imgTemp;
		module::ImImageEntity imgEntity = { hashcode, localPath, m_downUrl };
		if (module::getDatabaseModule()->sqlGetImImageEntityByHashcode(hashcode, imgTemp))
		{
			module::getDatabaseModule()->sqlUpdateImImageEntity(hashcode, imgEntity);
		}
		else
		{
			module::getDatabaseModule()->sqlInsertImImageEntity(imgEntity);
		}

		//会头像做灰度处理，并且保存到本地
		if (m_bGrayScale)
		{
			CxImage cximage;
			bool bSucc = cximage.Load(csLocalPath);
			if (bSucc)
			{
				cximage.GrayScale();
				CString csGrayPath = module::getMiscModule()->getDownloadDir()
					+ PREFIX_GRAY_AVATAR + csFileName;
				module::getCaptureModule()->saveToFile(cximage.MakeBitmap(), csGrayPath);
			}
		}

		//回调
		pParam->m_imgEntity = imgEntity;
		asyncCallback(std::shared_ptr<void>(pParam));
	}
}

void DownloadImgHttpOperation::release()
{
	delete this;
}

/******************************************************************************/