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

#ifndef _AX_TREE_H
#define _AX_TREE_H

#include "AXDef.h"

class AXTreeItem
{
public:
    AXTreeItem  *m_pPrev,
                *m_pNext,
                *m_pFirst,
                *m_pLast,
                *m_pParent;

public:
    AXTreeItem();
    virtual ~AXTreeItem();

    AXTreeItem *nextTreeItem();
    AXTreeItem *nextTreeItem(AXTreeItem *pParent);
    AXTreeItem *nextTreeItemPass();
    AXTreeItem *nextTreeItemPass(AXTreeItem *pParent);
    AXTreeItem *prevTreeItem();
    AXTreeItem *prevTreeItem(AXTreeItem *pParent);
    AXTreeItem *prevTreeItemStop();
    AXTreeItem *lastTreeItem();
    BOOL isChild(AXTreeItem *pParent);
};


class AXTree
{
protected:
    AXTreeItem  *m_pTop,
                *m_pBottom;

protected:
    void _linkAdd(AXTreeItem *pParent,AXTreeItem *p);
    void _linkInsert(AXTreeItem *pIns,AXTreeItem *p);
    void _linkRemove(AXTreeItem *p);
    void _deleteChild(AXTreeItem *pItem);

public:
    AXTree();
    virtual ~AXTree();

    AXTreeItem *getTop() const { return m_pTop; }
    AXTreeItem *getBottom() const { return m_pBottom; }

    void deleteAll();

    AXTreeItem *add(AXTreeItem *pParent,AXTreeItem *pItem);
    AXTreeItem *insert(AXTreeItem *pIns,AXTreeItem *pItem);
    void deleteItem(AXTreeItem *pItem);
    void remove(AXTreeItem *pItem);
    void move(AXTreeItem *pSrc,AXTreeItem *pDst);
    void moveLast(AXTreeItem *pSrc,AXTreeItem *pParent);
    int getItemCnt();
    AXTreeItem *getTreeLastItem();
};

#endif
