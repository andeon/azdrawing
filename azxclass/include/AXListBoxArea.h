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

#ifndef _AX_LISTBOXAREA_H
#define _AX_LISTBOXAREA_H

#include "AXScrollArea.h"

class AXListBoxItemManager;

class AXListBoxArea:public AXScrollArea
{
public:
    enum LISTBOXAREASTYLE
    {
        LBAS_OWNERDRAW  = SAS_EXTRA,
        LBAS_POPUP      = SAS_EXTRA << 1
    };

    enum LISTBOXAREANOTIFY
    {
        LBAN_SELCHANGE_MOUSE,
        LBAN_SELCHANGE_KEY,
        LBAN_DBLCLK,
        LBAN_POPUPEND
    };

protected:
    AXListBoxItemManager    *m_pDat;
    int     m_nItemH;

protected:
    void _sendNotify(UINT uNotify);
    int _getCurItemNo(int y);

public:
    virtual ~AXListBoxArea();
    AXListBoxArea(AXWindow *pParent,AXWindow *pOwner,UINT uStyle,AXListBoxItemManager *pDat);

    int getItemH() const { return m_nItemH; }
    void setItemH(int h) { m_nItemH = h; }

    void setScrVInfo();
    void adjustScrVSel(int dir);

    virtual BOOL isShowScroll(int size,BOOL bHorz);

    virtual BOOL onSize();
    virtual BOOL onNotify(AXWindow *pwin,UINT uNotify,ULONG uParam);
    virtual BOOL onDblClk(AXHD_MOUSE *phd);
    virtual BOOL onButtonDown(AXHD_MOUSE *phd);
    virtual BOOL onMouseWheel(AXHD_MOUSE *phd,BOOL bUp);
    virtual BOOL onMouseMove(AXHD_MOUSE *phd);
    virtual BOOL onKeyDown(AXHD_KEY *phd);
    virtual BOOL onPaint(AXHD_PAINT *phd);
};

#endif
