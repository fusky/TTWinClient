#include "StdAfx.h"
#include "RecvFileReadyTask.h"
#include "im-server/src/base/ImPduFile.h"

RecvFileReadyTask::RecvFileReadyTask(IN const std::string& sFromID,
                                     IN  const std::string& sToID)
:m_sFromID(sFromID)
,m_sToID(sToID)
{
}

RecvFileReadyTask::~RecvFileReadyTask(void)
{
}

void RecvFileReadyTask::execute()
{
    CImPduClientFileRecvReady pduRecvFileReadyMsg(
        m_sFromID.c_str(),
        m_sToID.c_str());
    sendPacket(&pduRecvFileReadyMsg);
}

void RecvFileReadyTask::release()
{
    delete this;
}
