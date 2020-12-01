/************************************************************************
*  Copyright (C) 2013-2015 Azel.
*
*  This file is part of AzDrawing.
*
*  AzDrawing is free software: you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation, either version 3 of the License, or
*  (at your option) any later version.
*
*  AzDrawing is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*************************************************************************/

#ifndef _AZDRAW_GRADLIST_H_
#define _AZDRAW_GRADLIST_H_

#include "AXList.h"
#include "AXString.h"
#include "AXMem.h"

class AXImage;

class CGradListItem:public AXListItem
{
public:
    enum
    {
        POINT_MAX   = 20,
        POS_BIT     = 10,
        POS_MAX     = 1<<POS_BIT,
        POS_MASK    = 0xffff,
        VAL_SHIFT   = 16
    };

public:
    AXString    m_strName;
    AXMem       m_memDat;

public:
    CGradListItem *prev() { return (CGradListItem *)m_pPrev; }
    CGradListItem *next() { return (CGradListItem *)m_pNext; }

    void setDefault();
    void setNewDat();
    void setDat(LPINT pDat,int ptcnt);
    void drawPrev(AXImage *pimg,int x,int y,int w,int h);

    static void drawPrevDat(AXImage *pimg,int x,int y,int w,int h,LPINT pDat);
};

//

class CGradList:public AXList
{
protected:
    CGradListItem   *m_pSelItem;
    BOOL    m_bUpdate;

    enum { MAXCNT = 100 };

public:
    CGradList();

    CGradListItem *getTopItem() const { return (CGradListItem *)m_pTop; }
    CGradListItem *getSelItem() const { return m_pSelItem; }
    int getSelNo() { return AXList::getPos(m_pSelItem); }

    void updateOn() { m_bUpdate = TRUE; }
    void setSelItem(CGradListItem *p) { m_pSelItem = p; }

    BOOL addGrad();
    BOOL delGrad();
    void setGradTable(LPBYTE pBuf,int cnt,int val,BOOL bRev);

    void setSelItemFromNo(int no);
    void saveFile(const AXString &filename);
    void readFile(const AXString &filename);
};

#endif
