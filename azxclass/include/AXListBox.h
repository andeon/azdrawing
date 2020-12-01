/*****************************************************************************
 *  Copyright (C) 2012-2015 Azel.
 *
 *  This file is part of AzXClass.
 *
 *  This library is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this library.  If not, see <http://www.gnu.org/licenses/>.
******************************************************************************/

#ifndef _AX_LISTBOX_H
#define _AX_LISTBOX_H

#include "AXScrollView.h"

class AXListBoxItemManager;
class AXListBoxItem;
class AXListItem;
class AXString;

class AXListBox:public AXScrollView
{
public:
    enum LISTBOXSTYLE
    {
        LBS_OWNERDRAW       = SVS_EXTRA,
        LBS_FOCUSOUT_SELOUT = SVS_EXTRA << 1
    };

    enum LISTBOXNOTIFY
    {
        LBN_SELCHANGE,
        LBN_DBLCLK
    };

protected:
    AXListBoxItemManager   *m_pDat;

protected:
    void _createListBox();

public:
    virtual ~AXListBox();

    AXListBox(AXWindow *pParent,UINT uStyle,UINT uLayoutFlags);
    AXListBox(AXWindow *pParent,UINT uStyle,UINT uLayoutFlags,UINT uItemID,DWORD dwPadding);

    void setScrollWidth(int width);
    void setItemHeight(int h);

    int getItemCnt();
    AXListBoxItem *getTopItem();
    AXListBoxItem *getBottomItem();
    AXListBoxItem *getSelItem();

    int addItem(LPCUSTR pstr);
    int addItem(LPCUSTR pstr,ULONG lParam);
    int addItem(LPCSTR pstr,ULONG lParam=0);
    int addItem(AXListBoxItem *pItem);
    void addItemTrMul(int startID,int cnt);
    int insertItem(int pos,LPCUSTR pstr,ULONG lParam=0);
    void deleteItemAll();
    void deleteItem(int pos);

    int getCurSel();
    void setCurSel(int pos);
    void setCurSel(AXListBoxItem *pItem);

    void getItemText(int pos,AXString *pstr);
    ULONG getItemParam(int pos);
    void setItemText(int pos,LPCUSTR pstr);
    void setItemParam(int pos,ULONG lParam);

    int findItemParam(ULONG lParam);
    BOOL moveItemUpDown(int pos,BOOL bDown);
    void sortItem(int (*func)(AXListItem*,AXListItem*,ULONG),ULONG lParam);
    void setAutoWidth(BOOL bScr=FALSE);
    void setAutoHeight();
    void setScrollEnd();
    void setScrollItem(AXListItem *pItem,int align);
    void updateArea();

    virtual void reconfig();
    virtual BOOL onFocusIn(int detail);
    virtual BOOL onFocusOut(int detail);
    virtual BOOL onNotify(AXWindow *pwin,UINT uNotify,ULONG uParam);
    virtual BOOL onKeyDown(AXHD_KEY *phd);
};

#endif
