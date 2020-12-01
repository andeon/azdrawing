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

#ifndef _AX_SCROLLBAR_H
#define _AX_SCROLLBAR_H

#include "AXWindow.h"

class AXScrollBar:public AXWindow
{
public:
    enum SCROLLBARSTYLE
    {
        SBS_HORZ    = 0,
        SBS_VERT    = WS_EXTRA
    };

    enum SCROLLBARNOTIFY
    {
        SBN_SCROLL
    };

protected:
    int     m_nMin,
            m_nMax,
            m_nPage,
            m_nPos,
            m_nInc,
            m_nRange,
            m_fDown,
            m_nDragAdj;

protected:
    void _createScrollBar();

    int _buttonDown(int x,int y);
    void _barDrag(int x,int y);
    int _getCurArea(int x,int y,int bw,int bpos);
    void _getBarStat(LPINT pBarW,LPINT pBarPos);

    void _drawHorzScr();
    void _drawVertScr();

public:
    virtual ~AXScrollBar();

    AXScrollBar(AXWindow *pParent,UINT uStyle,UINT uLayoutFlags);
    AXScrollBar(AXWindow *pParent,UINT uStyle,UINT uLayoutFlags,UINT uItemID,DWORD dwPadding);

    virtual void calcDefSize();

    int getPos() const { return m_nPos; }
    int getMin() const { return m_nMin; }
    int getMax() const { return m_nMax; }
    int getPage() const { return m_nPage; }
    int getScrollBoxWidth();

    void setStatus(int min,int max,int page,int inc=1);
    void setPage(int page);
    BOOL setPos(int pos);
    BOOL setPosEnd();
    BOOL movePos(int dir);

    virtual BOOL onScroll();
    virtual BOOL onPaint(AXHD_PAINT *phd);
    virtual BOOL onButtonDown(AXHD_MOUSE *phd);
    virtual BOOL onButtonUp(AXHD_MOUSE *phd);
    virtual BOOL onMouseMove(AXHD_MOUSE *phd);
    virtual BOOL onTimer(UINT uTimerID,ULONG lParam);
};

#endif
