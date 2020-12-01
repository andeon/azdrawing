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

#ifndef _AX_LISTBOXITEMMANAGER_H
#define _AX_LISTBOXITEMMANAGER_H

#include "AXList.h"

class AXListBoxItem;

class AXListBoxItemManager:public AXList
{
protected:
    AXListItem  *m_pSelItem;
    int         m_nSelNo;

protected:
    void _changeSel();
    void _setSelNone();

public:
    AXListBoxItemManager();

    int getSel() const { return m_nSelNo; }
    AXListBoxItem *getSelItem() const { return (AXListBoxItem *)m_pSelItem; }
    AXListBoxItem *getTopItem() const { return (AXListBoxItem *)m_pTop; }

    int addItem(LPCUSTR pstr,ULONG lParam);
    int addItem(AXListBoxItem *pItem);
    int insertItem(int pos,LPCUSTR pstr,ULONG lParam);
    void deleteAllItem();
    BOOL deleteItem(AXListBoxItem *pItem);
    BOOL deleteItemPos(int pos);

    AXListBoxItem *getItemFromPos(int pos);
    BOOL setSel(int pos);
    BOOL setSel(AXListBoxItem *pItem);
    BOOL moveSelUpDown(int dir);

    void getItemText(int pos,AXString *pstr);
    BOOL setItemText(int pos,LPCUSTR pstr);
    ULONG getItemParam(int pos);
    void setItemParam(int pos,ULONG lParam);
    int findItemParam(ULONG lParam);
    int findItemText(LPCUSTR pstr);

    BOOL moveItemUpDown(int pos,BOOL bDown);
};

#endif

