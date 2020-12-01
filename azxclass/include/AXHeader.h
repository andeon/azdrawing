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

#ifndef _AX_HEADER_H
#define _AX_HEADER_H

#include "AXWindow.h"
#include "AXString.h"
#include "AXList.h"


class AXHeaderItem:public AXListItem
{
public:
    enum FLAGS
    {
        FLAG_RIGHT  = 1,
        FLAG_FIX    = 2,
        FLAG_EXPAND = 4 | FLAG_FIX,
        FLAG_COL_OWNERDRAW = 8
    };

public:
    AXString    m_strText;
    int         m_nWidth;
    UINT        m_uFlags;

    AXHeaderItem(LPCUSTR pText,int width,UINT uFlags);
    AXHeaderItem *next() { return (AXHeaderItem *)m_pNext; }
};


class AXHeader:public AXWindow
{
public:
    enum HEADERNOTIFY
    {
        HDN_RESIZE
    };

protected:
    AXList  m_dat;
    int     m_nScr,
            m_nDragLeft;
    AXHeaderItem    *m_pDragItem;

    enum HEADERFLAGS
    {
        FLAG_DRAG = FLAG_TEMP1
    };

protected:
    void _createHeader();

public:
    virtual ~AXHeader();

    AXHeader(AXWindow *pParent,UINT uStyle,UINT uLayoutFlags);
    AXHeader(AXWindow *pParent,UINT uStyle,UINT uLayoutFlags,UINT uItemID,DWORD dwPadding);

    int getScroll() const { return m_nScr; }
    int getItemCnt() const { return m_dat.getCnt(); }
    AXHeaderItem *getTopItem() { return (AXHeaderItem *)m_dat.getTop(); }

    void setScroll(int scr);
    int getAllWidth();
    AXHeaderItem *getCurPosItem(int x,LPINT pNo=NULL,LPINT pX=NULL);

    void addItem(LPCUSTR pText,int width,UINT uFlags);
    int getItemWidth(int no);
    void setItemWidth(int no,int width);

    virtual void calcDefSize();

    virtual BOOL onSize();
    virtual BOOL onPaint(AXHD_PAINT *phd);
    virtual BOOL onButtonDown(AXHD_MOUSE *phd);
    virtual BOOL onButtonUp(AXHD_MOUSE *phd);
    virtual BOOL onMouseMove(AXHD_MOUSE *phd);
    virtual BOOL onLeave(AXHD_ENTERLEAVE *phd);
};

#endif
