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

#ifndef _AZDRAW_PRESSCURVEVIEW_H_
#define _AZDRAW_PRESSCURVEVIEW_H_

#include "AXWindow.h"
#include "AXImage.h"

class CPressCurveView:public AXWindow
{
protected:
    AXImage m_img;
    int     m_nVal;

protected:
    void _changePos(int x,int y);
    void _draw();

public:
    CPressCurveView(AXWindow *pParent,UINT uItemID,int size);

    int getVal() { return m_nVal; }
    void setVal(int val);

    virtual BOOL onPaint(AXHD_PAINT *phd);
    virtual BOOL onButtonDown(AXHD_MOUSE *phd);
    virtual BOOL onButtonUp(AXHD_MOUSE *phd);
    virtual BOOL onMouseMove(AXHD_MOUSE *phd);
};

#endif
