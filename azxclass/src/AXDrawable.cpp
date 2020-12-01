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

#include "AXX11.h"

#include "AXDrawable.h"
#include "AXApp.h"
#include "AXAppRes.h"
#include "AXGC.h"


#define DISP        ((Display *)m_pDisp)
#define RESGC(no)   (GC)axres->colGC(no)
#define SetXSegment(seg,xx1,yy1,xx2,yy2)    (seg)->x1 = xx1, (seg)->y1 = yy1, (seg)->x2 = xx2, (seg)->y2 = yy2;


/*!
    @class AXDrawable
    @brief AXWindow/AXPixmap の基底クラス

    @ingroup core
*/

//-------------------------


AXDrawable::~AXDrawable()
{

}

AXDrawable::AXDrawable()
{
    m_id    = 0;
    m_pDisp = axapp->getDisp();
}

//! 直線描画

void AXDrawable::drawLine(int x1,int y1,int x2,int y2,int rescol)
{
    ::XDrawLine(DISP, m_id, RESGC(rescol), x1, y1, x2, y2);
}

void AXDrawable::drawLine(int x1,int y1,int x2,int y2,const AXGC &gc)
{
    ::XDrawLine(DISP, m_id, (GC)gc.getGC(), x1, y1, x2, y2);
}

//! 水平線描画

void AXDrawable::drawLineH(int x,int y,int w,int rescol)
{
    ::XDrawLine(DISP, m_id, RESGC(rescol), x, y, x + w - 1, y);
}

//! 垂直線描画

void AXDrawable::drawLineV(int x,int y,int h,int rescol)
{
    ::XDrawLine(DISP, m_id, RESGC(rescol), x, y, x, y + h - 1);
}

//! 連続直線描画

void AXDrawable::drawLines(const DRAWPOINT *ppt,int cnt,int rescol)
{
    ::XDrawLines(DISP, m_id, RESGC(rescol), (XPoint *)ppt, cnt, CoordModeOrigin);
}

//! 複数直線描画

void AXDrawable::drawLineSeg(const DRAWSEGMENT *pseg,int cnt,int rescol)
{
    ::XDrawSegments(DISP, m_id, RESGC(rescol), (XSegment *)pseg, cnt);
}

//! 四角枠描画

void AXDrawable::drawBox(int x,int y,int w,int h,int rescol)
{
    ::XDrawRectangle(DISP, m_id, RESGC(rescol), x, y, w - 1, h - 1);
}

//! 複数四角枠描画

void AXDrawable::drawBoxs(const DRAWRECT *prc,int cnt,int rescol)
{
    ::XDrawRectangles(DISP, m_id, RESGC(rescol), (XRectangle *)prc, cnt);
}

//! 四角塗りつぶし描画

void AXDrawable::drawFillBox(int x,int y,int w,int h,int rescol)
{
	::XFillRectangle(DISP, m_id, RESGC(rescol), x, y, w, h);
}

void AXDrawable::drawFillBox(int x,int y,int w,int h,const AXGC &gc)
{
	::XFillRectangle(DISP, m_id, (GC)gc.getGC(), x, y, w, h);
}

//! 複数四角塗りつぶし描画

void AXDrawable::drawFillBoxs(const DRAWRECT *prc,int cnt,int rescol)
{
    ::XFillRectangles(DISP, m_id, RESGC(rescol), (XRectangle *)prc, cnt);
}

//! 楕円描画

void AXDrawable::drawEllipse(int x,int y,int w,int h,int rescol)
{
    ::XDrawArc(DISP, m_id, RESGC(rescol), x, y, w, h, 0, 360 * 64);
}

//! 楕円塗りつぶし描画

void AXDrawable::drawFillEllipse(int x,int y,int w,int h,int rescol)
{
    ::XFillArc(DISP, m_id, RESGC(rescol), x, y, w, h, 0, 360 * 64);
}


//=========================
//
//=========================


//! くぼみ枠描画

void AXDrawable::drawFrameSunken(int x,int y,int w,int h)
{
    XSegment s[2];
    int x2,y2;

    x2 = x + w - 1;
    y2 = y + h - 1;

    //暗い

    SetXSegment(s, x, y, x2, y);
    SetXSegment(s + 1, x, y, x, y2);

    ::XDrawSegments(DISP, m_id, RESGC(AXAppRes::FRAMEDARK), s, 2);

    //明るい

    SetXSegment(s, x, y2, x2, y2);
    SetXSegment(s + 1, x2, y, x2, y2);

    ::XDrawSegments(DISP, m_id, RESGC(AXAppRes::FRAMELIGHT), s, 2);
}

//! 浮きだし枠描画

void AXDrawable::drawFrameRaise(int x,int y,int w,int h)
{
    XSegment s[2];
    int x2,y2;

    x2 = x + w - 1;
    y2 = y + h - 1;

    //明るい

    SetXSegment(s, x, y, x2, y);
    SetXSegment(s + 1, x, y, x, y2);

    ::XDrawSegments(DISP, m_id, RESGC(AXAppRes::FRAMELIGHT), s, 2);

    //暗い

    SetXSegment(s, x, y2, x2, y2);
    SetXSegment(s + 1, x2, y, x2, y2);

    ::XDrawSegments(DISP, m_id, RESGC(AXAppRes::FRAMEDARK), s, 2);
}

//! ボタン描画
//! @param flags AXDrawable::DRAWBUTTONFLAGS

void AXDrawable::drawButton(int x,int y,int w,int h,UINT flags)
{
    if(flags & DRAWBUTTON_NOOUTFRAME)
    {
        //------- 外枠なし

        //枠

        if(flags & DRAWBUTTON_DOWN)
            drawFrameSunken(x, y, w, h);
        else
            drawFrameRaise(x, y, w, h);

        //背景

        drawFillBox(x + 1, y + 1, w - 2, h - 2,
            (flags & DRAWBUTTON_FOCUSED)? AXAppRes::FACEFOCUS: AXAppRes::FACE);
    }
    else
    {
        //-------- 外枠あり

        int n;

        //外枠

        if(flags & DRAWBUTTON_DISABLE)
            n = AXAppRes::FRAMEDARK;
        else if(flags & DRAWBUTTON_DEFAULT)
            n = AXAppRes::FRAMEDEFBTT;
        else
            n = AXAppRes::FRAMEOUTSIDE;

        drawBox(x, y, w, h, n);

        //内枠

        if(flags & DRAWBUTTON_DOWN)
            drawFrameSunken(x + 1, y + 1, w - 2, h - 2);
        else
            drawFrameRaise(x + 1, y + 1, w - 2, h - 2);

        //背景

        drawFillBox(x + 2, y + 2, w - 4, h - 4,
            (flags & DRAWBUTTON_FOCUSED)? AXAppRes::FACEFOCUS: AXAppRes::FACE);
    }
}

//! チェック描画 (7x7)

void AXDrawable::drawCheck(int x,int y,int rescol)
{
    XSegment s[6];

    SetXSegment(s    , x    , y + 2, x + 2, y + 4);
    SetXSegment(s + 1, x    , y + 3, x + 2, y + 5);
    SetXSegment(s + 2, x    , y + 4, x + 2, y + 6);
    SetXSegment(s + 3, x + 2, y + 4, x + 6, y);
    SetXSegment(s + 4, x + 2, y + 5, x + 6, y + 1);
    SetXSegment(s + 5, x + 2, y + 6, x + 6, y + 2);

    ::XDrawSegments(DISP, m_id, RESGC(rescol), s, 6);
}

//! ラジオチェック描画 (4x4)

void AXDrawable::drawRadioCheck(int x,int y,int rescol)
{
    XSegment s[4];

    SetXSegment(s    , x + 1, y    , x + 2, y);
    SetXSegment(s + 1, x    , y + 1, x + 3, y + 1);
    SetXSegment(s + 2, x    , y + 2, x + 3, y + 2);
    SetXSegment(s + 3, x + 1, y + 3, x + 2, y + 3);

    ::XDrawSegments(DISP, m_id, RESGC(rescol), s, 4);
}

//! 左矢印描画 (3x5)

void AXDrawable::drawArrowLeft(int ctx,int cty,int rescol)
{
    XSegment s[3];

    SetXSegment(s    , ctx - 1, cty    , ctx - 1, cty);
    SetXSegment(s + 1, ctx    , cty - 1, ctx    , cty + 1);
    SetXSegment(s + 2, ctx + 1, cty - 2, ctx + 1, cty + 2);

    ::XDrawSegments(DISP, m_id, RESGC(rescol), s, 3);
}

//! 右矢印描画 (3x5)

void AXDrawable::drawArrowRight(int ctx,int cty,int rescol)
{
    XSegment s[3];

    SetXSegment(s    , ctx - 1, cty - 2, ctx - 1, cty + 2);
    SetXSegment(s + 1, ctx    , cty - 1, ctx    , cty + 1);
    SetXSegment(s + 2, ctx + 1, cty    , ctx + 1, cty);

    ::XDrawSegments(DISP, m_id, RESGC(rescol), s, 3);
}

//! 上矢印描画 (5x3)

void AXDrawable::drawArrowUp(int ctx,int cty,int rescol)
{
    XSegment s[3];

    SetXSegment(s    , ctx    , cty - 1, ctx    , cty - 1);
    SetXSegment(s + 1, ctx - 1, cty    , ctx + 1, cty);
    SetXSegment(s + 2, ctx - 2, cty + 1, ctx + 2, cty + 1);

    ::XDrawSegments(DISP, m_id, RESGC(rescol), s, 3);
}

//! 下矢印描画 (5x3)

void AXDrawable::drawArrowDown(int ctx,int cty,int rescol)
{
    XSegment s[3];

    SetXSegment(s    , ctx - 2, cty - 1, ctx + 2, cty - 1);
    SetXSegment(s + 1, ctx - 1, cty    , ctx + 1, cty);
    SetXSegment(s + 2, ctx    , cty + 1, ctx    , cty + 1);

    ::XDrawSegments(DISP, m_id, RESGC(rescol), s, 3);
}

//! 小さい上矢印描画 (3x2)

void AXDrawable::drawArrowUpSmall(int ctx,int topy,int rescol)
{
    XSegment s[2];

    SetXSegment(s, ctx, topy, ctx, topy);
    SetXSegment(s + 1, ctx - 1, topy + 1, ctx + 1, topy + 1);

    ::XDrawSegments(DISP, m_id, RESGC(rescol), s, 2);
}

//! 小さい下矢印描画 (3x2)

void AXDrawable::drawArrowDownSmall(int ctx,int bottomy,int rescol)
{
    XSegment s[2];

    SetXSegment(s, ctx, bottomy, ctx, bottomy);
    SetXSegment(s + 1, ctx - 1, bottomy - 1, ctx + 1, bottomy - 1);

    ::XDrawSegments(DISP, m_id, RESGC(rescol), s, 2);
}

//! 下矢印描画（7x4）

void AXDrawable::drawArrowDown7(int ctx,int topy,int rescol)
{
    XSegment s[4];
    int i;

    for(i = 0; i < 4; i++)
        SetXSegment(s + i, ctx + i - 3, topy + i, ctx + 3 - i, topy + i);

    ::XDrawSegments(DISP, m_id, RESGC(rescol), s, 4);
}

//! 右矢印描画（4x7）

void AXDrawable::drawArrowRight7(int topx,int cty,int rescol)
{
    XSegment s[4];
    int i;

    for(i = 0; i < 4; i++)
        SetXSegment(s + i, topx + i, cty + i - 3, topx + i, cty + 3 - i);

    ::XDrawSegments(DISP, m_id, RESGC(rescol), s, 4);
}
