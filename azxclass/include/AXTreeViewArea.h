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

#ifndef _AX_TREEVIEWAREA_H
#define _AX_TREEVIEWAREA_H

#include "AXScrollArea.h"

class AXPixmap;
class AXTreeViewItem;
class AXTreeViewItemManager;

class AXTreeViewArea:public AXScrollArea
{
public:
    enum ITEMONTYPE
    {
        ONTYPE_NONE,
        ONTYPE_EXPAND,
        ONTYPE_CHECKBOX
    };

    enum TREEVIEWAREAFLAGS
    {
        FLAG_TIMER  = FLAG_TEMP1
    };

protected:
    AXTreeViewItemManager   *m_pDat;
    AXPixmap        *m_pImg;

    int     m_fBtt,
            m_nDragTopY;
    AXTreeViewItem  *m_pDragItem,
                    *m_pDragDstItem;

protected:
    void _drawimg();
    AXTreeViewItem *_getCurItem(int x,int y,LPINT pType,LPINT pTopY=NULL);
    void _notify(UINT notify,ULONG param);
    void _expandItem(AXTreeViewItem *p);
    void _adjustScroll(int dir);

public:
    virtual ~AXTreeViewArea();
    AXTreeViewArea(AXWindow *pParent,UINT uStyle,AXTreeViewItemManager *pDat);

    AXTreeViewItem *getDragSrcItem() { return m_pDragItem; }

    BOOL isShowScroll(int size,BOOL bHorz);

    virtual BOOL onSize();
    virtual BOOL onNotify(AXWindow *pwin,UINT uNotify,ULONG lParam);
    virtual BOOL onPaint(AXHD_PAINT *phd);
    virtual BOOL onKeyDown(AXHD_KEY *phd);
    virtual BOOL onButtonDown(AXHD_MOUSE *phd);
    virtual BOOL onButtonUp(AXHD_MOUSE *phd);
    virtual BOOL onMouseMove(AXHD_MOUSE *phd);
    virtual BOOL onDblClk(AXHD_MOUSE *phd);
    virtual BOOL onMouseWheel(AXHD_MOUSE *phd,BOOL bUp);
    virtual BOOL onTimer(UINT uTimerID,ULONG lParam);
};

#endif
