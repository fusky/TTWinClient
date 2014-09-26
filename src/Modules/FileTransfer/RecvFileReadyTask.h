#pragma once


#include "LogicEngine/LinkService/NetworkTask.h"

class RecvFileReadyTask : public logic::IAsynSocketTask
{
public:
    RecvFileReadyTask(IN const std::string& sFromID,
        IN  const std::string& sToID);
    ~RecvFileReadyTask(void);
public:
    virtual void execute();
    virtual void release();
private:
    std::string  m_sFromID;
    std::string  m_sToID;
};
