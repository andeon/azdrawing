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

#ifndef _AX_MULTIEDITAREA_H
#define _AX_MULTIEDITAREA_H

#include "AXScrollArea.h"
#include "AXPixmap.h"

class AXEditString;
class AXString;

class AXMultiEditArea:public AXScrollArea
{
    friend class AXMultiEdit;

public:
    enum MULTIEDITAREASTYLE
    {
        MEAS_READONLY   = SAS_EXTRA
    };

protected:
    AXEditString    *m_pString;
    AXPixmap        m_img;
    int     m_fBtt,
            m_nBkPos;

protected:
    virtual void getICPos(AXPoint *pPos);
    void _drawText();
    void _adjustScroll();
    void _drawimg();
    void setScrollInfo();

public:
    virtual ~AXMultiEditArea();
    AXMultiEditArea(AXWindow *pParent,UINT uStyle,AXEditString *pString);

    virtual BOOL isShowScroll(int size,BOOL bHorz);

    virtual BOOL onSize();
    virtual BOOL onPaint(AXHD_PAINT *phd);
    virtual BOOL onKeyDown(AXHD_KEY *phd);
    virtual BOOL onButtonDown(AXHD_MOUSE *phd);
    virtual BOOL onButtonUp(AXHD_MOUSE *phd);
    virtual BOOL onMouseMove(AXHD_MOUSE *phd);
    virtual BOOL onMouseWheel(AXHD_MOUSE *phd,BOOL bUp);
    virtual BOOL onNotify(AXWindow *pwin,UINT uNotify,ULONG lParam);
};

#endif
