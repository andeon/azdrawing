/************************************************************************
*  Copyright (C) 2013-2015 Azel.
*
*  This file is part of AzDrawing.
*
*  AzDrawing is free software: you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation, either version 3 of the License, or
*  (at your option) any later version.
*
*  AzDrawing is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*************************************************************************/
//SubWidget.cpp

#ifndef _AZDRAW_IMGPREV_H_
#define _AZDRAW_IMGPREV_H_

#include "AXWindow.h"
#include "AXImage.h"

class CImgPrev:public AXWindow
{
public:
    enum
    {
        STYLE_RESIZE = WS_EXTRA,

        NOTIFY_RESIZE = 0
    };

protected:
    AXImage     m_img;

public:
    CImgPrev(AXWindow *pParent,UINT uStyle,UINT uLayoutFlags,DWORD dwPadding,int w,int h);

    AXImage *getImg() { return &m_img; }

    virtual void calcDefSize();
    virtual BOOL onSize();
    virtual BOOL onPaint(AXHD_PAINT *phd);
};

#endif
