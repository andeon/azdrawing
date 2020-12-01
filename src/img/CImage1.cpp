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

#include <stdlib.h>

#include "CImage1.h"

#include "AXRect.h"
#include "AXImage.h"


/*!
    @class CImage1
    @brief 1bitイメージクラス
*/


CImage1::CImage1()
{
    m_pBuf = NULL;
    m_nWidth = m_nHeight = 0;
}

CImage1::~CImage1()
{
    free();
}

//! 解放

void CImage1::free()
{
    if(m_pBuf)
    {
        ::free(m_pBuf);

        m_pBuf = NULL;
        m_nWidth = m_nHeight = 0;
    }
}

//! 作成

BOOL CImage1::create(int width,int height)
{
    free();

    //1行は4バイト単位

    m_nPitch = ((width + 7) / 8 + 3) & (~3);

    //メモリ確保

    m_pBuf = (LPBYTE)::malloc(m_nPitch * height);
    if(!m_pBuf) return FALSE;

    //情報

    m_nWidth  = width;
    m_nHeight = height;

    //クリア

    clear();

    return TRUE;
}

//! ゼロクリア

void CImage1::clear()
{
    LPDWORD p = (LPDWORD)m_pBuf;
    int i;

    for(i = (m_nPitch >> 2) * m_nHeight; i > 0; i--)
        *(p++) = 0;
}

//! 点セット

void CImage1::setPixel(int x,int y,int val)
{
    LPBYTE p;
    BYTE f;

    if(x >= 0 && x < m_nWidth && y >= 0 && y < m_nHeight)
    {
        p = m_pBuf + y * m_nPitch + (x >> 3);

        f = 1 << (7 - (x & 7));

        if(val)
            *p |= f;
        else
            *p &= ~f;
    }
}

//! 色取得

int CImage1::getPixel(int x,int y)
{
    if(x >= 0 && x < m_nWidth && y >= 0 && y < m_nHeight)
    {
        LPBYTE p = m_pBuf + y * m_nPitch + (x >> 3);

        return (*p >> (7 - (x & 7))) & 1;
    }
    else
        return 0;
}

//! 直線

void CImage1::line(int x1,int y1,int x2,int y2,int val)
{
    int f,d = 0,add = 1;

    if(::abs(x1 - x2) > ::abs(y1 - y2))
    {
        f = y1 << 16;
        if(x2 != x1) d = ((y2 - y1) << 16) / (x2 - x1);
        if(x1 > x2) add = -1, d = -d;

        while(x1 != x2)
        {
            setPixel(x1, (f + (1 << 15)) >> 16, val);
            f += d;
            x1 += add;
        }
    }
    else
    {
        f = x1 << 16;
        if(y2 != y1) d = ((x2 - x1) << 16) / (y2 - y1);
        if(y1 > y2) add = -1, d = -d;

        while(y1 != y2)
        {
            setPixel((f + (1 << 15)) >> 16, y1, val);
            f += d;
            y1 += add;
        }
    }

    setPixel(x2, y2, val);
}

//! 四角枠

void CImage1::box(int x,int y,int w,int h,int val)
{
    int i;

    for(i = 0; i < w; i++)
    {
        setPixel(x + i, y        , val);
        setPixel(x + i, y + h - 1, val);
    }

    for(i = 1; i < h - 1; i++)
    {
        setPixel(x        , y + i, val);
        setPixel(x + w - 1, y + i, val);
    }
}

//! ベジェ線描画（XOR描画用）
/*!
    @param prcs 描画範囲が入る
    @param type 0 で補助線2を描画しない
*/

void CImage1::drawBezierTemp(AXPoint *pPos,AXRectSize *prcs,int type)
{
    double x[4],y[4];
    double t,tt,t1,t2,t3,t4;
    int i,nx,ny,sx,sy;
    AXPoint min,max;

    sx = pPos[0].x, sy = pPos[0].y;

    //補助線

    line(sx, sy, pPos[2].x, pPos[2].y, 1);
    box(pPos[2].x - 2, pPos[2].y - 2, 5, 5, 1);

    if(type)
    {
        line(pPos[3].x, pPos[3].y, pPos[1].x, pPos[1].y, 1);
        box(pPos[3].x - 2, pPos[3].y - 2, 5, 5, 1);
    }

    //最小・最大

    min.x = sx, min.y = sy;
    max = min;

    _setMinMax(&min, &max, pPos[1].x, pPos[1].y);
    _setMinMax(&min, &max, pPos[2].x - 2, pPos[2].y - 2);
    _setMinMax(&min, &max, pPos[2].x + 2, pPos[2].y + 2);
    _setMinMax(&min, &max, pPos[3].x - 2, pPos[3].y - 2);
    _setMinMax(&min, &max, pPos[3].x + 2, pPos[3].y + 2);

    //曲線

    x[0] = pPos[0].x, y[0] = pPos[0].y;
    x[1] = pPos[2].x, y[1] = pPos[2].y;
    x[2] = pPos[3].x, y[2] = pPos[3].y;
    x[3] = pPos[1].x, y[3] = pPos[1].y;

    for(i = 1, t = 0.01; i <= 100; i++, t += 0.01)
    {
        tt = 1 - t;

        t1 = tt * tt * tt;
        t2 = 3 * t * tt * tt;
        t3 = 3 * t * t * tt;
        t4 = t * t * t;

        nx = (int)(x[0] * t1 + x[1] * t2 + x[2] * t3 + x[3] * t4 + 0.5);
        ny = (int)(y[0] * t1 + y[1] * t2 + y[2] * t3 + y[3] * t4 + 0.5);

        //描画

        line(sx, sy, nx, ny, 1);

        //

        if(nx < min.x) min.x = nx; if(nx > max.x) max.x = nx;
        if(ny < min.y) min.y = ny; if(ny > max.y) max.y = ny;

        sx = nx, sy = ny;
    }

    //更新範囲

    if(min.x < 0) min.x = 0;
    if(min.y < 0) min.y = 0;
    if(max.x >= m_nWidth)  max.x = m_nWidth - 1;
    if(max.y >= m_nHeight) max.y = m_nHeight - 1;

    prcs->set(min.x, min.y, max.x - min.x + 1, max.y - min.y + 1);
}

//! 最小値・最大値のセット

void CImage1::_setMinMax(AXPoint *pMin,AXPoint *pMax,int x,int y)
{
    if(x < pMin->x) pMin->x = x;
    if(x > pMax->x) pMax->x = x;

    if(y < pMin->y) pMin->y = y;
    if(y > pMax->y) pMax->y = y;
}

//! AXImageにXOR合成

void CImage1::blendXor(AXImage *pimgDst,int x,int y,int w,int h)
{
    LPBYTE pSrc,ps,pDst;
    int ix,iy,pitchd,bpp;
    BYTE mask;

    pSrc = getBufPt(x, y);
    pDst = pimgDst->getBufPt(x, y);

    bpp    = pimgDst->getBytes();
    pitchd = pimgDst->getPitch() - w * bpp;

    for(iy = h; iy > 0; iy--)
    {
        ps   = pSrc;
        mask = 1 << (7 - (x & 7));

        for(ix = w; ix > 0; ix--, pDst += bpp)
        {
            if(*ps & mask)
                pimgDst->setPixelBufXor(pDst);

            mask >>= 1;

            if(mask == 0)
            {
                ps++;
                mask = 0x80;
            }
        }

        pSrc += m_nPitch;
        pDst += pitchd;
    }
}
