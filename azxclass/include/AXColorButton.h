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

#ifndef _AX_COLORBUTTON_H
#define _AX_COLORBUTTON_H

#include "AXButton.h"
#include "AXGC.h"

class AXGC;

class AXColorButton:public AXButton
{
public:
    enum COLORBUTTONSTYLE
    {
        CBTS_CHOOSE = BS_EXTRA
    };

    enum COLORBUTTONNOTIFY
    {
        CBTN_PRESS
    };

protected:
    DWORD   m_dwCol;
    AXGC    m_gc;

protected:
    void _createColorButton(DWORD col);

public:
    virtual ~AXColorButton();

    AXColorButton(AXWindow *pParent,UINT uStyle,UINT uLayoutFlags);
    AXColorButton(AXWindow *pParent,UINT uStyle,UINT uLayoutFlags,UINT uItemID,DWORD dwPadding);
    AXColorButton(AXWindow *pParent,UINT uStyle,UINT uLayoutFlags,UINT uItemID,DWORD dwPadding,DWORD col);

    DWORD getColor() const { return m_dwCol; }
    void setColor(DWORD col);

    virtual void calcDefSize();

    virtual BOOL onPress();
    virtual BOOL onPaint(AXHD_PAINT *phd);
};

#endif
