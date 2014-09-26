/*******************************************************************************
 *  @file      UIUserList.h 2014\7\17 13:07:13 $
 *  @author    大佛<dafo@mogujie.com>
 *  @brief   
 ******************************************************************************/

#ifndef UIUSERLIST_92BF7D1E_3298_41A6_93EF_14865AE0DBCC_H__
#define UIUSERLIST_92BF7D1E_3298_41A6_93EF_14865AE0DBCC_H__

#include "Modules/UI/UIIMList.h"
#include "GlobalDefine.h"
class SessionListItemInfo : public IMListItemInfo
{

public:
	UInt32 Time;
};

/******************************************************************************/

/**
	* The class <code>UIUserList</code>
	*
	*/
class CUIRecentSessionList : public UIIMList
{
public:
	CUIRecentSessionList(CPaintManagerUI& paint_manager);
	
	Node* AddNode(const SessionListItemInfo& item, Node* parent = NULL, int index = 0);

	BOOL AddNode(const std::string& sId);
	BOOL UpdateItemBySId(const std::string& sId);
	void ClearItemMsgCount(IN const std::string& sId);//清除显示的未读计数
	void sort();
};

/******************************************************************************/
#endif// UIUSERLIST_92BF7D1E_3298_41A6_93EF_14865AE0DBCC_H__
