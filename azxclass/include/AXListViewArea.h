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

#ifndef _AX_LISTVIEWAREA_H
#define _AX_LISTVIEWAREA_H

#include "AXScrollArea.h"

class AXListView;
class AXHeader;
class AXListViewItemManager;
class AXListViewItem;
class AXPixmap;

class AXListViewArea:public AXScrollArea
{
    friend class AXListView;

protected:
    AXListViewItemManager   *m_pDat;
    AXHeader    *m_pHeader;
    AXPixmap    *m_pImg;
    int         m_nLastClkColumn;

protected:
    void _draw();
    void _notify(UINT notify);
    void _notify(UINT notify,AXListViewItem *pItem);
    void _adjustScroll(int dir);
    int _getTopY();
    int _getAreaH();
    AXListViewItem *_getCurPosItem(int x,int y);
    void _setLastClkColumn(int x);
    BOOL _checkbox(AXListViewItem *pi,int x,int y);
    void _pageupdown(BOOL bUp);

public:
    virtual ~AXListViewArea();
    AXListViewArea(AXWindow *pParent,UINT uStyle,AXListViewItemManager *pDat);

    AXHeader *getHeader() const { return m_pHeader; }

    virtual BOOL isShowScroll(int size,BOOL bHorz);

    virtual BOOL onSize();
    virtual BOOL onPaint(AXHD_PAINT *phd);
    virtual BOOL onKeyDown(AXHD_KEY *phd);
    virtual BOOL onButtonDown(AXHD_MOUSE *phd);
    virtual BOOL onNotify(AXWindow *pwin,UINT uNotify,ULONG lParam);
    virtual BOOL onMouseWheel(AXHD_MOUSE *phd,BOOL bUp);
    virtual BOOL onDblClk(AXHD_MOUSE *phd);
};

#endif
