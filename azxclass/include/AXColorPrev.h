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

#ifndef _AX_COLORPREV_H
#define _AX_COLORPREV_H

#include "AXWindow.h"
#include "AXGC.h"

class AXColorPrev:public AXWindow
{
public:
    enum COLOPREVSTYLE
    {
        CPS_SUNKEN  = WS_EXTRA,
        CPS_FRAME   = WS_EXTRA << 1
    };

protected:
    AXGC    m_gc;
    DWORD   m_dwCol;

protected:
    void _createColorPrev();

public:
    virtual ~AXColorPrev();

    AXColorPrev(AXWindow *pParent,UINT uStyle,UINT uLayoutFlags);
    AXColorPrev(AXWindow *pParent,UINT uStyle,UINT uLayoutFlags,UINT uItemID,DWORD dwPadding);

    DWORD getColor() const { return m_dwCol; }
    void setColor(DWORD col);

    virtual BOOL onPaint(AXHD_PAINT *phd);
};

#endif
