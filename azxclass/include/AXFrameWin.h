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

#ifndef _AX_FRAMEWIN_H
#define _AX_FRAMEWIN_H

#include "AXWindow.h"

class AXFrameWin:public AXWindow
{
public:
    enum FRAMEWINSTYLE
    {
        FS_SUNKEN   = 0,
        FS_FRAME    = WS_EXTRA
    };

public:
    virtual ~AXFrameWin();
    AXFrameWin(AXWindow *pParent,UINT uStyle,UINT uLayoutFlags);
    AXFrameWin(AXWindow *pParent,UINT uStyle,UINT uLayoutFlags,UINT uItemID,DWORD dwPadding);

    virtual void getClientRect(AXRect *prc);
    virtual BOOL onPaint(AXHD_PAINT *phd);
};

#endif
