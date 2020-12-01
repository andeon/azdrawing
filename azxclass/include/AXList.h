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

#ifndef _AX_LIST_H
#define _AX_LIST_H

#include "AXDef.h"

class AXListItem
{
public:
    AXListItem	*m_pPrev,*m_pNext;

public:
    virtual ~AXListItem();
    AXListItem();
};

//-------------

class AXList
{
protected:
    AXListItem  *m_pTop,*m_pBottom;
    int         m_nCnt;

protected:
    void linkAdd(AXListItem *p);
    void linkInsert(AXListItem *pSrc,AXListItem *pIns);
    void linkRemove(AXListItem *p);

public:
    virtual ~AXList();
    AXList();

    int getCnt() const { return m_nCnt; }
    AXListItem *getTop() const { return m_pTop; }
    AXListItem *getBottom() const { return m_pBottom; }

    void deleteAll();

    AXListItem *add(AXListItem *p);
    AXListItem *insert(AXListItem *pItem,AXListItem *pIns);
    AXListItem *insert(AXListItem *pItem,int pos);

    void deleteItem(AXListItem *p);
    BOOL deleteItem(int pos);
    void remove(AXListItem *p);

    void move(AXListItem *pSrc,AXListItem *pDst);
    void moveTop(AXListItem *pItem);
    void swap(AXListItem *p1,AXListItem *p2);

    AXListItem *getItem(int pos) const;
    AXListItem *getItemBottom(int pos) const;
    int getPos(AXListItem *pItem) const;
    int getPosBottom(AXListItem *pItem) const;
    int getDir(AXListItem *pItem1,AXListItem *pItem2) const;

    void sort(int (*func)(AXListItem*,AXListItem*,ULONG),ULONG lParam);
};

#endif
