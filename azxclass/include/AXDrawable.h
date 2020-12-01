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

#ifndef _AX_DRAWABLE_H
#define _AX_DRAWABLE_H

#include "AXDef.h"

class AXGC;

class AXDrawable
{
public:
    typedef struct
    {
        short x,y;
    }DRAWPOINT;

    typedef struct
    {
        short x1,y1,x2,y2;
    }DRAWSEGMENT;

    typedef struct
    {
        short   x,y;
        unsigned short w,h;
    }DRAWRECT;

public:
    enum DRAWBUTTONFLAGS
    {
        DRAWBUTTON_DOWN    = 1,
        DRAWBUTTON_FOCUSED = 2,
        DRAWBUTTON_DEFAULT = 4,
        DRAWBUTTON_DISABLE = 8,
        DRAWBUTTON_NOOUTFRAME = 16
    };

protected:
    ULONG   m_id;
    LPVOID  m_pDisp;

public:
    virtual ~AXDrawable();
    AXDrawable();

    ULONG getid() const { return m_id; }
    BOOL isExist() const { return (m_id != 0); }

    void setDrawRect(DRAWRECT *p,int x,int y,int w,int h)
    {
        p->x = x, p->y = y, p->w = w, p->h = h;
    }
    void setDrawSeg(DRAWSEGMENT *p,int x1,int y1,int x2,int y2)
    {
        p->x1 = x1, p->y1 = y1, p->x2 = x2, p->y2 = y2;
    }

    void drawLine(int x1,int y1,int x2,int y2,int rescol);
    void drawLine(int x1,int y1,int x2,int y2,const AXGC &gc);
    void drawLineH(int x,int y,int w,int rescol);
    void drawLineV(int x,int y,int h,int rescol);
    void drawLines(const DRAWPOINT *ppt,int cnt,int rescol);
    void drawLineSeg(const DRAWSEGMENT *pseg,int cnt,int rescol);
    void drawBox(int x,int y,int w,int h,int rescol);
    void drawBoxs(const DRAWRECT *prc,int cnt,int rescol);
    void drawFillBox(int x,int y,int w,int h,int rescol);
    void drawFillBox(int x,int y,int w,int h,const AXGC &gc);
    void drawFillBoxs(const DRAWRECT *prc,int cnt,int rescol);
    void drawEllipse(int x,int y,int w,int h,int rescol);
    void drawFillEllipse(int x,int y,int w,int h,int rescol);

    void drawFrameSunken(int x,int y,int w,int h);
    void drawFrameRaise(int x,int y,int w,int h);
    void drawButton(int x,int y,int w,int h,UINT flags);
    void drawCheck(int x,int y,int rescol);
    void drawRadioCheck(int x,int y,int rescol);

    void drawArrowLeft(int ctx,int cty,int rescol);
    void drawArrowRight(int ctx,int cty,int rescol);
    void drawArrowUp(int ctx,int cty,int rescol);
    void drawArrowDown(int ctx,int cty,int rescol);
    void drawArrowUpSmall(int ctx,int topy,int rescol);
    void drawArrowDownSmall(int ctx,int bottomy,int rescol);
    void drawArrowDown7(int ctx,int topy,int rescol);
    void drawArrowRight7(int topx,int cty,int rescol);
};

#endif
