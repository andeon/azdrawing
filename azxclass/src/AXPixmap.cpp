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

#include "AXPixmap.h"

#include "AXImage32.h"
#include "AXGC.h"
#include "AXApp.h"
#include "AXAppRes.h"

#define DISP        ((Display *)m_pDisp)


/*!
    @class AXPixmap
    @brief ピクスマップ

    - Drawable に転送できるが、直接バッファに書き込めない。

    @ingroup draw
*/


AXPixmap::AXPixmap()
    : AXDrawable()
{
    m_nWidth = m_nHeight = 0;
}

AXPixmap::~AXPixmap()
{
    free();
}

//! 解放

void AXPixmap::free()
{
    if(m_id)
    {
        ::XFreePixmap(DISP, m_id);

        m_id = 0;
        m_nWidth = m_nHeight = 0;
    }
}

//! 作成

BOOL AXPixmap::create(int w,int h)
{
    free();

    m_id = ::XCreatePixmap(DISP, axapp->getRootID(), w, h, axapp->getDepth());
    if(!m_id) return FALSE;

    m_nWidth  = w;
    m_nHeight = h;

    return TRUE;
}

//! 再作成
/*!
    水平・垂直のブロックサイズを単位として作成（1以下でブロック計算なし）@n
    サイズが同じ場合はそのまま。
*/

BOOL AXPixmap::recreate(int w,int h,int hblock,int vblock)
{
    if(hblock > 1)
        w = (w <= hblock)? hblock: (w + hblock - 1) / hblock * hblock;

    if(vblock > 1)
        h = (h <= vblock)? vblock: (h + vblock - 1) / vblock * vblock;

    if(w != m_nWidth || h != m_nHeight)
        return create(w, h);

    return TRUE;
}

//! AXImage32 の指定した色の部分をマスクとして 1bit イメージ作成

BOOL AXPixmap::createMask(const AXImage32 &imgSrc,DWORD dwCol)
{
    int w,h,x,y,bit,pitch;
    LPBYTE pBuf,pDst,pDstY;
    LPDWORD pSrc;

    free();

    w = imgSrc.getWidth();
    h = imgSrc.getHeight();

    //----- 作業用イメージ確保

    pitch = (w + 7) / 8;

    pBuf = (LPBYTE)AXMalloc(pitch * h);
    if(!pBuf) return FALSE;

    AXMemZero(pBuf, pitch * h);

    //----- マスク部分を0、それ以外を1に

    pDstY = pBuf;
    pSrc  = imgSrc.getBuf();

    for(y = h; y > 0; y--)
    {
        pDst = pDstY;

        for(x = w, bit = 1; x > 0; x--, pSrc++)
        {
            if((*pSrc & 0xffffff) != dwCol)
                *pDst |= bit;

            bit <<= 1;
            if(bit == 256) { bit = 1; pDst++; }
        }

        pDstY += pitch;
    }

    //----- 作成

    m_id = ::XCreateBitmapFromData(DISP, axapp->getRootID(), (char *)pBuf, w, h);

    AXFreeNormal(pBuf);

    return (m_id != None);
}


//======================


//! 転送（転送先/元の位置は同じ）

void AXPixmap::put(AXDrawable *pdst,int x,int y,int w,int h) const
{
    if(m_id)
        ::XCopyArea(DISP, m_id, pdst->getid(), (GC)axres->colGC(0), x, y, w, h, x, y);
}

//! 転送

void AXPixmap::put(AXDrawable *pdst,int dx,int dy,int sx,int sy,int w,int h) const
{
    if(m_id)
        ::XCopyArea(DISP, m_id, pdst->getid(), (GC)axres->colGC(0), sx, sy, w, h, dx, dy);
}

//! 転送

void AXPixmap::put(AXDrawable *pdst,const AXGC &gc,int dx,int dy,int sx,int sy,int w,int h) const
{
    if(m_id)
        ::XCopyArea(DISP, m_id, pdst->getid(), (GC)gc.getGC(), sx, sy, w, h, dx, dy);
}
