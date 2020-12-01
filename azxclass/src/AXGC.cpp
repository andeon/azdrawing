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

#include "AXGC.h"

#include "AXApp.h"


#define GCPT     (GC)m_gc

/*!
    @class AXGC
    @brief グラフィックコンテキスト

    @ingroup draw
*/


AXGC::AXGC()
{
    m_gc = NULL;
}

AXGC::~AXGC()
{
    free();
}

//! 解放

void AXGC::free()
{
    if(m_gc)
    {
        ::XFreeGC(axdisp(), GCPT);
        m_gc = NULL;
    }
}

//! 内部作成処理

BOOL AXGC::_create(ULONG mask,LPVOID pVal)
{
    free();

    m_gc = (LPVOID)::XCreateGC(axdisp(), axapp->getRootID(), mask, (XGCValues *)pVal);

    return (m_gc != NULL);
}

//! デフォルトで作成

BOOL AXGC::createDefault()
{
    return _create(0, NULL);
}

//! 色指定作成

BOOL AXGC::createColor(DWORD col)
{
    XGCValues gv;

    gv.fill_style           = FillSolid;
    gv.graphics_exposures   = FALSE;
    gv.foreground           = axapp->rgbToPix(col);
    gv.background           = gv.foreground;

    return _create(GCForeground | GCBackground | GCFillStyle | GCGraphicsExposures, &gv);
}

//! XOR作成

BOOL AXGC::createXor()
{
    XGCValues gv;

    gv.function             = GXxor;
    gv.graphics_exposures   = FALSE;
    gv.foreground           = axapp->rgbToPix(0xffffff);

    return _create(GCForeground | GCFunction | GCGraphicsExposures, &gv);
}

//! クリッピングマスク指定作成

BOOL AXGC::createClipMask(ULONG pixmapID)
{
    XGCValues gv;

    gv.graphics_exposures  = FALSE;
    gv.clip_mask           = pixmapID;

    return _create(GCGraphicsExposures | GCClipMask, &gv);
}


//==========================
//各属性
//==========================


//! 前景色変更（ピクセル値）

void AXGC::setColorPix(ULONG pix)
{
    ::XSetForeground(axdisp(), GCPT, pix);
}

//! 前景色変更（RGB）

void AXGC::setColorRGB(DWORD col)
{
    ::XSetForeground(axdisp(), GCPT, axapp->rgbToPix(col));
}

//! 背景色変更

void AXGC::setBkColorPix(ULONG pix)
{
    ::XSetBackground(axdisp(), GCPT, pix);
}

//! GraphicsExpose/NoExposeイベントを送るか

void AXGC::setExposureFlag(BOOL flag)
{
    ::XSetGraphicsExposures(axdisp(), GCPT, flag);
}

//! クリップマスクなしにする

void AXGC::setClipNone()
{
    ::XSetClipMask(axdisp(), GCPT, None);
}

//! 矩形のクリップマスクセット

void AXGC::setClipRect(int x,int y,int w,int h)
{
    XRectangle rc;

    rc.x = x, rc.y = y, rc.width = w, rc.height = h;

    ::XSetClipRectangles(axdisp(), GCPT, 0, 0, &rc, 1, Unsorted);
}

//! クリッピング原点セット

void AXGC::setClipOrigin(int x,int y)
{
    ::XSetClipOrigin(axdisp(), GCPT, x, y);
}
