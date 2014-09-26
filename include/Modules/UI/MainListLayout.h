 /*******************************************************************************
 *  @file      TestButton.h 2014\8\4 10:15:42 $
 *  @author    大佛<dafo@mogujie.com>
 *  @brief     
 ******************************************************************************/

#ifndef LISTPANEL_00907A2F_35FC_48EE_B385_4C5DE6018C6A_H__
#define LISTPANEL_00907A2F_35FC_48EE_B385_4C5DE6018C6A_H__

#include "GlobalDefine.h"
#include "DuiLib/UIlib.h"
#include "TTLogic/Observer.h"
/******************************************************************************/
using namespace DuiLib;
class CEAUserTreelistUI;
class CGroupsTreelistUI;
class CUIRecentSessionList;
class UIIMList;
class Node;

/**
 * The class <code>TestButton</code> 
 *
 */
class MainListLayout :public CHorizontalLayoutUI,public INotifyUI
{
public:
    /** @name Constructors and Destructor*/

    //@{
    /**
     * Constructor 
     */
    MainListLayout();
    /**
     * Destructor
     */
    ~MainListLayout();
    //@}
public:
	virtual LPVOID GetInterface(LPCTSTR pstrName);
	virtual LPCTSTR GetClass() const;

	virtual void DoInit();
	virtual void DoEvent(TEventUI& event);
	virtual void Notify(TNotifyUI& msg);

private:
	void OnUserlistModuleEvent(UInt16 moduleId, UInt32 keyId, MKO_TUPLE_PARAM mkoParam);
	void OnGrouplistModuleEvent(UInt16 moduleId, UInt32 keyId, MKO_TUPLE_PARAM mkoParam);
	void OnSessionModuleEvent(UInt16 moduleId, UInt32 keyId, MKO_TUPLE_PARAM mkoParam);
	void OnSysConfigModuleEvent(UInt16 moduleId, UInt32 keyId, MKO_TUPLE_PARAM mkoParam);
	void OnFileTransferModuleEvent(UInt16 moduleId, UInt32 keyId, MKO_TUPLE_PARAM mkoParam);
private:
	void _creatSessionDialog(IN UIIMList* pList,IN CControlUI* pMsgSender);
	void _LoadAllDepartment();

	void _AddGroupList();
	void _AddDiscussGroupList();
	void _AddRecentUserListToUI();
	void _AddRecentGroupListToUI();
	void _NewGroupAdded(std::string& gId);
	void _NewMsgUpdate(std::string& sId);			//收到新消息更新
	void _CreatNewDiscussGroupRes(std::string& sId);//创建新的讨论组

private:
	CTabLayoutUI*			m_Tab;
	CEAUserTreelistUI*		m_EAuserTreelist;
	CGroupsTreelistUI*		m_GroupList;
	CUIRecentSessionList*	m_UIRecentConnectedList;

	Node*					m_groupRootParent;
	Node*					m_DiscussGroupRootParent;
};
/******************************************************************************/
#endif// LISTPANEL_00907A2F_35FC_48EE_B385_4C5DE6018C6A_H__
