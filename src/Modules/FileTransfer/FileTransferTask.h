/*******************************************************************************
 *  @file      FileTransferTask.h 2014\4\10 13:26:40 $
 *  @author    快刀<kuaidao@mogujie.com>
 *  @summary   文件传输task
 ******************************************************************************/

#ifndef FILETRANSFERTASK_4BDBD48A_8369_4F3D_A2B8_2F7EED00124C_H__
#define FILETRANSFERTASK_4BDBD48A_8369_4F3D_A2B8_2F7EED00124C_H__

#include "TTLogic/IOperation.h"
#include <string>
/******************************************************************************/
class FileTransferSocket;
class FileTransTaskBase : public logic::IOperation
{
public:
    FileTransTaskBase(const std::string& sfId);
    virtual ~FileTransTaskBase();

public:
    virtual void release();

public:
    BOOL                        m_bContinue;
    std::string                 m_sTaskId;
    FileTransferSocket*         m_pFileTransSocket;
};

/**
 * The class <code>FileTransferTask</code> 
 *
 */
class FileReceiveTask : public FileTransTaskBase
{
public:
    /** @name Constructors and Destructor*/

    //@{
    /**
     * Constructor 
     */
    FileReceiveTask(const std::string& sfId);
    /**
     * Destructor
     */
    virtual ~FileReceiveTask();
    //@}

public:
	virtual void process();
    virtual void release();
};

class FileSendTask : public FileTransTaskBase
{
public:
    /** @name Constructors and Destructor*/

    //@{
    /**
     * Constructor 
     */
    FileSendTask(const std::string& sfId);
    /**
     * Destructor
     */
    virtual ~FileSendTask();
    //@}

public:
	virtual void process();
    virtual void release();
};

class OfflineFileReqTask: public FileTransTaskBase
{
public:
    /** @name Constructors and Destructor*/

    //@{
    /**
     * Constructor 
     */
    OfflineFileReqTask(const std::string& sfId);
    /**
     * Destructor
     */
    virtual ~OfflineFileReqTask();
    //@}

public:
	virtual void process();
    virtual void release();
};
/******************************************************************************/
#endif// FILETRANSFERTASK_4BDBD48A_8369_4F3D_A2B8_2F7EED00124C_H__
