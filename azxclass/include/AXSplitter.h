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

#ifndef _AX_SPLITTER_H
#define _AX_SPLITTER_H

#include "AXWindow.h"

class AXSplitter:public AXWindow
{
public:
    enum SPLITTERSTYLE
    {
        SPLS_VERT   = 0,
        SPLS_HORZ   = WS_EXTRA << 1
    };

protected:
    enum
    {
        FLAG_DOWN = FLAG_TEMP1
    };

    int     m_nStartPos;
    AXRect  m_rcPrev,m_rcNext;

    void _createSplitter();

public:
    virtual ~AXSplitter();

    AXSplitter(AXWindow *pParent,UINT uStyle);

    virtual void calcDefSize();

    virtual BOOL onPaint(AXHD_PAINT *phd);
    virtual BOOL onButtonDown(AXHD_MOUSE *phd);
    virtual BOOL onButtonUp(AXHD_MOUSE *phd);
    virtual BOOL onMouseMove(AXHD_MOUSE *phd);
};

#endif
