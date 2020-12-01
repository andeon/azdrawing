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

#include "CImage8.h"

#include "AXImage32.h"
#include "AXImage.h"


/*!
    @class CImage8
    @brief 8bitイメージ

    - Y1列のバイト境界はなし。
    - 確保サイズは 4Byte 単位。
*/



CImage8::CImage8()
{
    m_pBuf = NULL;
    m_nWidth = m_nHeight = 0;
}

CImage8::~CImage8()
{
    free();
}

//! 解放

void CImage8::free()
{
    if(m_pBuf)
    {
        ::free(m_pBuf);

        m_pBuf = NULL;
        m_nWidth = m_nHeight = 0;
    }
}

//! 作成

BOOL CImage8::create(int width,int height)
{
    free();

    //メモリ確保 (4Byte単位)

    m_dwAllocSize = (width * height + 3) & (~3);

    m_pBuf = (LPBYTE)::malloc(m_dwAllocSize);
    if(!m_pBuf) return FALSE;

    //

    m_nWidth  = width;
    m_nHeight = height;

    return TRUE;
}

//! 複製の作成

BOOL CImage8::clone(const CImage8 &src)
{
    if(!src.getBuf())
    {
        free();
        return TRUE;
    }

    //作成

    if(!create(src.getWidth(), src.getHeight())) return FALSE;

    //コピー

    copy(src);

    return TRUE;
}

//! テクスチャとして色取得

BYTE CImage8::getPixelTexture(int x,int y)
{
    while(x < 0) x += m_nWidth;
    while(y < 0) y += m_nHeight;

    return *(m_pBuf + (y % m_nHeight) * m_nWidth + (x % m_nWidth));
}

//! クリア

void CImage8::clear()
{
    LPDWORD p = (LPDWORD)m_pBuf;

    for(int i = m_dwAllocSize >> 2; i > 0; i--)
        *(p++) = 0;
}

//! コピー（２つのサイズは同じであること）

void CImage8::copy(const CImage8 &src)
{
    LPDWORD pd,ps;

    pd = (LPDWORD)m_pBuf;
    ps = (LPDWORD)src.getBuf();

    for(int i = m_dwAllocSize >> 2; i > 0; i--)
        *(pd++) = *(ps++);
}

//! 32bitイメージから変換して作成

BOOL CImage8::createFrom32bit(const AXImage32 &src,BOOL bAlpha)
{
    LPBYTE pd;
    AXImage32::PIXEL *ps;
    int i,w,h;

    w = src.getWidth();
    h = src.getHeight();

    if(!create(w, h)) return FALSE;

    //

    pd = m_pBuf;
    ps = (AXImage32::PIXEL *)src.getBuf();

    for(i = w * h; i > 0; i--, ps++)
        *(pd++) = (bAlpha)? ps->a: 255 - ps->r;

    return TRUE;
}

//! 32bitイメージから変換して作成（ブラシ画像用）
/*!
    イメージが正方形でない場合は、正方形になるように調整される。
*/

BOOL CImage8::createFrom32bit_brush(const AXImage32 &src,BOOL bAlpha)
{
    LPBYTE pd;
    AXImage32::PIXEL *ps;
    int sw,sh,size,ix,iy,pitch;

    sw = src.getWidth();
    sh = src.getHeight();

    if(sw < sh) size = sh;
    else size = sw;

    if(!create(size, size)) return FALSE;

    clear();

    //

    pd = getBufPt((size - sw) / 2, (size - sh) / 2);
    ps = (AXImage32::PIXEL *)src.getBuf();
    pitch = size - sw;

    for(iy = sh; iy > 0; iy--, pd += pitch)
    {
        for(ix = sw; ix > 0; ix--, ps++)
        {
            *(pd++) = (bAlpha)? ps->a: 255 - ps->r;
        }
    }

    return TRUE;
}

//! デフォルトブラシイメージ作成（アンチエイリアス付き円）

void CImage8::createBrushImg_fillCircle(int size)
{
    int ix,iy,jx,jy,c;
    int xx[5],yy[5],rr,ct,f;
    LPBYTE p;

    if(!create(size, size)) return;

    //5x5

    p = m_pBuf;

    ct = size * 5 / 2;
    rr = size * 5 / 2 - 2;
    rr = rr * rr;

    for(iy = 0; iy < size; iy++)
    {
        for(jy = 0, f = iy * 5 - ct; jy < 5; jy++, f++)
            yy[jy] = f * f;

        //

        for(ix = 0; ix < size; ix++)
        {
            for(jx = 0, f = ix * 5 - ct; jx < 5; jx++, f++)
                xx[jx] = f * f;

            //

            c = 0;

            for(jy = 0; jy < 5; jy++)
            {
                for(jx = 0; jx < 5; jx++)
                {
                    if(xx[jx] + yy[jy] < rr) c += 255;
                }
            }

            *(p++) = c / 25;
        }
    }
}

//! テクスチャプレビュー用描画（繰り返し）

void CImage8::drawTexturePrev(AXImage *pimgDst)
{
    int ix,iy,w,h,bpp,pitchd,v;
    LPBYTE psY,pd;

    w = pimgDst->getWidth();
    h = pimgDst->getHeight();

    pd  = pimgDst->getBuf();
    bpp = pimgDst->getBytes();
    pitchd = pimgDst->getPitch() - w * bpp;

    for(iy = 0; iy < h; iy++)
    {
        psY = m_pBuf + (iy % m_nHeight) * m_nWidth;

        for(ix = 0; ix < w; ix++, pd += bpp)
        {
            v = 255 - *(psY + (ix % m_nWidth));

            pimgDst->setPixelBuf(pd, v, v, v);
        }

        pd += pitchd;
    }
}

//! ブラシ画像プレビュー用描画
/*!
    矩形内に収まるように縮小。this 画像は正方形。
*/

void CImage8::drawBrushPrev(AXImage *pimgDst)
{
    int pitchd,dsize,dw,dh,bpp;
    int ix,iy,addf,fx,fy,x,y,c;
    LPBYTE pDst,pSrcY;

    dw = pimgDst->getWidth();
    dh = pimgDst->getHeight();
    dsize = (dw < dh)? dw: dh;

    pDst    = pimgDst->getBuf();
    bpp     = pimgDst->getBytes();
    pitchd  = pimgDst->getPitch() - dw * bpp;

    addf    = (int)((double)m_nWidth / dsize * (1 << 16));

    //

    for(iy = dh, fy = 0; iy > 0; iy--, fy += addf)
    {
        y     = fy >> 16;
        pSrcY = (y < m_nHeight)? m_pBuf + y * m_nWidth: NULL;

        for(ix = dw, fx = 0; ix > 0; ix--, pDst += bpp, fx += addf)
        {
            x = fx >> 16;

            if(!pSrcY || x >= m_nWidth)
                pimgDst->setPixelBuf(pDst, 0x808080);
            else
            {
                c = 255 - pSrcY[x];

                pimgDst->setPixelBuf(pDst, c, c, c);
            }
        }

        pDst += pitchd;
    }
}
