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

#ifndef _AX_SLIDERBAR_H
#define _AX_SLIDERBAR_H

#include "AXWindow.h"

class AXSliderBar:public AXWindow
{
public:
    enum SLIDERBARSTYLE
    {
        SLBS_SMALL  = WS_EXTRA
    };

    enum SLIDERBARNOTIFY
    {
        SLBN_POS_KEY,
        SLBN_BUTTON_DOWN,
        SLBN_BUTTON_DRAG,
        SLBN_BUTTON_UP
    };

protected:
    enum SLIDERBARFLAGS
    {
        FLAG_DOWN_MOUSE = FLAG_TEMP1
    };

protected:
    int     m_nMin,m_nMax,m_nPos;

protected:
    void _createSliderBar();
    void _moveCurPos(int x,UINT uNotify,BOOL bSend);

public:
    virtual ~AXSliderBar();

    AXSliderBar(AXWindow *pParent,UINT uStyle,UINT uLayoutFlags);
    AXSliderBar(AXWindow *pParent,UINT uStyle,UINT uLayoutFlags,UINT uItemID,DWORD dwPadding);

    virtual void calcDefSize();

    int getPos() const { return m_nPos; }
    int getMin() const { return m_nMin; }
    int getMax() const { return m_nMax; }

    void setStatus(int min,int max,int pos);
    void setPos(int pos);

    virtual BOOL onScroll(UINT uNotify);
    virtual BOOL onPaint(AXHD_PAINT *phd);
    virtual BOOL onKeyDown(AXHD_KEY *phd);
    virtual BOOL onButtonDown(AXHD_MOUSE *phd);
    virtual BOOL onButtonUp(AXHD_MOUSE *phd);
    virtual BOOL onMouseMove(AXHD_MOUSE *phd);
};

#endif
