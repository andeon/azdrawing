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

#ifndef _AX_SCROLLVIEW_H
#define _AX_SCROLLVIEW_H

#include "AXWindow.h"

class AXScrollBar;
class AXScrollArea;
class AXEdgeBox;

class AXScrollView:public AXWindow
{
public:
    enum SCROLLVIEWSTYLE
    {
        SVS_HORZ        = WS_EXTRA,
        SVS_VERT        = WS_EXTRA << 1,
        SVS_HORZ_FIX    = WS_EXTRA << 2,
        SVS_VERT_FIX    = WS_EXTRA << 3,
        SVS_FRAME       = WS_EXTRA << 4,
        SVS_SUNKEN      = WS_EXTRA << 5,

        SVS_EXTRA       = WS_EXTRA << 6,

        SVS_HORZVERT        = SVS_HORZ|SVS_VERT,
        SVS_HORZVERT_FIX    = SVS_HORZ|SVS_VERT|SVS_HORZ_FIX|SVS_VERT_FIX,
        SVS_HORZVERTSUNKEN  = SVS_HORZ|SVS_VERT|SVS_SUNKEN
    };

protected:
    AXScrollArea    *m_pScrArea;
    AXScrollBar     *m_pScrH,
                    *m_pScrV;
    AXEdgeBox       *m_pEdge;
    BOOL            m_bSetLayout;

protected:
    void _createScrollView();
    void _setLayout();

public:
    virtual ~AXScrollView();

    AXScrollView(AXWindow *pParent,UINT uStyle,UINT uLayoutFlags);
    AXScrollView(AXWindow *pParent,UINT uStyle,UINT uLayoutFlags,UINT uItemID,DWORD dwPadding);

    AXScrollBar *scrH() const { return m_pScrH; }
    AXScrollBar *scrV() const { return m_pScrV; }
    void setScrollArea(AXScrollArea *p) { m_pScrArea = p; }

    virtual void layout();
    virtual void calcDefSize();
    virtual void getClientRect(AXRect *prc);

    virtual void reconfig();
    virtual BOOL onSize();
    virtual BOOL onNotify(AXWindow *pwin,UINT uNotify,ULONG uParam);
    virtual BOOL onPaint(AXHD_PAINT *phd);
};

#endif
