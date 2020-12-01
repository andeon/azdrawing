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

#ifndef _AZDRAW_VALBAR_H_
#define _AZDRAW_VALBAR_H_

#include "AXWindow.h"

class CValBar:public AXWindow
{
protected:
    int   m_nPos,m_nMin,m_nMax;

    enum { FLAG_DRAG = FLAG_TEMP1 };

    void _changePos(int x);

public:
    CValBar(AXWindow *pParent,UINT uStyle,UINT uLayoutFlags,UINT uID,DWORD dwPadding);
    CValBar(AXWindow *pParent,UINT uStyle,UINT uLayoutFlags,UINT uID,DWORD dwPadding,int min,int max,int pos);

    BOOL isDrag() { return m_uFlags & FLAG_DRAG; }
    int getPos() { return m_nPos; }
    void setPos(int pos);
    void setRange(int min,int max);

    virtual void calcDefSize();

    virtual BOOL onPaint(AXHD_PAINT *phd);
    virtual BOOL onButtonDown(AXHD_MOUSE *phd);
    virtual BOOL onButtonUp(AXHD_MOUSE *phd);
    virtual BOOL onMouseMove(AXHD_MOUSE *phd);
};

#endif
