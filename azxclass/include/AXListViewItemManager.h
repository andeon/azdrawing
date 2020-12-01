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

#ifndef _AX_LISTVIEWITEMMANAGER_H
#define _AX_LISTVIEWITEMMANAGER_H

#include "AXList.h"

class AXListViewItem;

class AXListViewItemManager:public AXList
{
public:
    enum SELECTTYPE
    {
        SELECT_SINGLE,
        SELECT_MULTI,
        SELECT_MULTI_CTRL,
        SELECT_MULTI_SHIFT
    };

protected:
    AXListViewItem  *m_pFocus;

public:
    AXListViewItemManager();

    AXListViewItem *getTopItem() const { return (AXListViewItem *)m_pTop; }
    AXListViewItem *getFocusItem() const { return m_pFocus; }

    void deleteAllItem();
    void delItem(AXListViewItem *pItem);
    AXListViewItem *getSelNext(AXListViewItem *pTop);
    AXListViewItem *getItemPos(int pos);
    int getFocusPos();

    void selectAll();
    void unselectAll();
    BOOL updownFocus(BOOL bDown);
    BOOL moveFocusTopEnd(BOOL bTop);
    BOOL moveFocusFromPos(int pos);
    BOOL selectItem(int type,AXListViewItem *pItem);
};

#endif
