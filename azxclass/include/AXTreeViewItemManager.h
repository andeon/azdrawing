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

#ifndef _AX_TREEVIEWITEMMANAGER_H
#define _AX_TREEVIEWITEMMANAGER_H

#include "AXTree.h"

class AXTreeViewItem;

class AXTreeViewItemManager:public AXTree
{
protected:
    AXTreeViewItem  *m_pFocus;

public:
    AXTreeViewItemManager();

    AXTreeViewItem *getTopItem() const { return (AXTreeViewItem *)m_pTop; }
    AXTreeViewItem *getFocusItem() const { return m_pFocus; }

    BOOL setFocusItem(AXTreeViewItem *p);
    BOOL setFocusItemVisible(AXTreeViewItem *p);

    void deleteAllItem();
    void delItem(AXTreeViewItem *p);

    void calcReconfig(int nSpaceLevel,int nItemH,LPINT pMaxWidth,LPINT pMaxHeight);
    AXTreeViewItem *getCurItem(int x,int y,int itemh,LPINT pTopY=NULL);
    BOOL isVisibleItem(AXTreeViewItem *pItem);
    BOOL isFocusVisible();
    BOOL isFocusEnableExpand();
    BOOL isItemChild(AXTreeViewItem *pParent,AXTreeViewItem *pItem);
    BOOL moveFocusUpDown(BOOL bUp);
    BOOL moveFocusHomeEnd(BOOL bHome);
    int getFocusYPos(int itemh);
};

#endif
