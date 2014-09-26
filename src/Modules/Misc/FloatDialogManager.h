 /*******************************************************************************
 *  @file      FloatDialogManager.h 2014\8\27 17:45:39 $
 *  @author    ´ó·ð<dafo@mogujie.com>
 *  @brief     
 ******************************************************************************/

#ifndef FLOATDIALOGMANAGER_49794663_3D48_4531_9042_F78FC7897E6A_H__
#define FLOATDIALOGMANAGER_49794663_3D48_4531_9042_F78FC7897E6A_H__

#include "FloatDialog.h"
#include "utility/TTAutoLock.h"
#include <string>
#include <vector>

/******************************************************************************/
/**
 * The class <code>FloatDialogManager</code> 
 *
 */

class FloatWndInfo
{
public:
	FloatInfo		m_info;
	FloatDialog*	m_pFloatDialog;
};
class FloatDialogManager
{
public:
    ~FloatDialogManager();
	static FloatDialogManager* getInstance();
private:
	FloatDialogManager();

public:
	void pushFloatWnd(IN FloatInfo info, IN HWND parentWnd);
	void popFloatWnd(IN std::string sid);
private:
	std::vector<FloatWndInfo>		m_vecWndInfo;
	util::TTFastLock				m_lock;
};
/******************************************************************************/
#endif// FLOATDIALOGMANAGER_49794663_3D48_4531_9042_F78FC7897E6A_H__
