/*******************************************************************************
 *  @file      SysConfigModule_Impl.h 2014\8\4 10:56:38 $
 *  @author    快刀<kuaidao@mogujie.com>
 *  @brief     系统配置信息，包括系统设置和全局配置信息
 ******************************************************************************/

#ifndef SYSCONFIGMODULE_IMPL_9E63D68E_676C_49DB_A936_7F97A626D551_H__
#define SYSCONFIGMODULE_IMPL_9E63D68E_676C_49DB_A936_7F97A626D551_H__

#include "Modules/ISysConfigModule.h"
/******************************************************************************/


/**
 * The class <code>系统配置信息，包括系统设置和全局配置信息</code> 
 *
 */
class SysConfigModule_Impl final : public module::ISysConfigModule
{
public:
    /** @name Constructors and Destructor*/

    //@{
    /**
     * Constructor 
     */
    SysConfigModule_Impl();
    /**
     * Destructor
     */
    virtual ~SysConfigModule_Impl();
    //@}
	virtual void release();
	virtual logic::LogicErrorCode onLoadModule();
	virtual logic::LogicErrorCode onUnLoadModule();

public:
	virtual module::TTConfig* getSystemConfig();
	virtual BOOL saveData();
	virtual std::string userID()const;
	virtual CString UserID()const;
	virtual void showSysConfigDialog(HWND hParentWnd);
	virtual BOOL showServerConfigDialog(HWND hParentWnd);
	virtual void SetSysConfigDialogFlag(BOOL bIsExist);
private:
	/**
	 * 加载序列化的数据
	 *
	 * @return  BOOL
	 * @exception there is no any exception to throw.
	 */	
	BOOL _loadData();
	/**
	* 保存序列化的数据
	*
	* @return  BOOL
	* @exception there is no any exception to throw.
	*/
	BOOL _saveData();
	/**
	 * 反序列化
	 *
	 * @param   CArchive & ar
	 * @return  void
	 * @exception there is no any exception to throw.
	 */
	void _unMarshal(CArchive& ar);
	/**
	* 序列化
	*
	* @param   CArchive & ar
	* @return  void
	* @exception there is no any exception to throw.
	*/
	void _marshal(CArchive& ar);

private:
	module::TTConfig			m_pConfig;
	BOOL						m_bSysConfigDialogFlag;//确保单个窗口实例
};
/******************************************************************************/
#endif// SYSCONFIGMODULE_IMPL_9E63D68E_676C_49DB_A936_7F97A626D551_H__
