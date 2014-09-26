/*******************************************************************************
 *  @file      UIGroupsTreelist.h 2014\8\7 15:44:04 $
 *  @author    快刀<kuaidao@mogujie.com>
 *  @brief     群、讨论组会话树形控件
 ******************************************************************************/

#ifndef UIGROUPSTREELIST_A6A09709_9687_4B3D_8924_25CA7CBC4A8F_H__
#define UIGROUPSTREELIST_A6A09709_9687_4B3D_8924_25CA7CBC4A8F_H__

#include "Modules/UI/UIIMList.h"

const CDuiString MY_DISCUSSGROUP_ITEMID = _T("myDiscussGroup_id");
const CDuiString MY_GROUP_ITEMID = _T("mygroup_id");

class GroupsListItemInfo :public IMListItemInfo
{

};

/**
* The class <code>群、讨论组会话树形控件</code>
*
*/
class CGroupsTreelistUI : public UIIMList
{
public:
	
	CGroupsTreelistUI(CPaintManagerUI& paint_manager);

	Node* AddNode(const GroupsListItemInfo& item, Node* parent = NULL);
	BOOL UpdateItemBySId(const std::string& sId);
	void ClearItemMsgCount(IN const std::string& sId);//清除显示的未读计数
};
/******************************************************************************/
#endif// UIGROUPSTREELIST_a6a09709-9687-4b3d-8924-25ca7cbc4a8f_H__