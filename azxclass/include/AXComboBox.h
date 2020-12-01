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

#ifndef _AX_COMBOBOX_H
#define _AX_COMBOBOX_H

#include "AXWindow.h"

class AXListBoxItemManager;
class AXListBoxItem;
class AXString;

class AXComboBox:public AXWindow
{
public:
    enum COMBOBOXSTYLE
    {
        CBS_OWNERDRAW   = WS_EXTRA
    };

    enum COMBOBOXNOTIFY
    {
        CBN_SELCHANGE
    };

protected:
    AXListBoxItemManager    *m_pDat;
    int     m_nItemH;

protected:
    void _createComboBox();
    void _popupList();
    void _notifySel();

public:
    virtual ~AXComboBox();

    AXComboBox(AXWindow *pParent,UINT uStyle,UINT uLayoutFlags);
    AXComboBox(AXWindow *pParent,UINT uStyle,UINT uLayoutFlags,UINT uItemID,DWORD dwPadding);

    virtual void calcDefSize();

    AXListBoxItem *getTopItem();
    AXListBoxItem *getSelItem();

    int getItemH() const { return m_nItemH; }
    void setItemH(int h);

    int addItem(LPCUSTR pstr);
    int addItem(LPCUSTR pstr,ULONG lParam);
    int addItem(LPCSTR pText,ULONG lParam);
    int addItem(AXListBoxItem *pItem);
    int addItemTr(WORD wStrID,ULONG lParam);
    void addItemMulTr(int startID,int cnt);
    int addItemSort(LPCUSTR pstr,ULONG lParam=0);
    int insertItem(int pos,LPCUSTR pstr,ULONG lParam);
    void deleteItemAll();
    void deleteItem(int pos);
    void deleteItem(AXListBoxItem *pItem);

    int getItemCnt();
    int getCurSel();
    void setCurSel(int pos);
    void setCurSel_findParam(ULONG lParam);

    void getItemText(int pos,AXString *pstr);
    ULONG getItemParam(int pos);
    void setItemText(int pos,LPCUSTR pstr);
    void setItemParam(int pos,ULONG lParam);

    int findItemParam(ULONG lParam);
    int findItemText(LPCUSTR pstr);
    void setAutoWidth();

    virtual BOOL onPaint(AXHD_PAINT *phd);
    virtual BOOL onButtonDown(AXHD_MOUSE *phd);
    virtual BOOL onKeyDown(AXHD_KEY *phd);
    virtual BOOL onMouseWheel(AXHD_MOUSE *phd,BOOL bUp);
};

#endif
