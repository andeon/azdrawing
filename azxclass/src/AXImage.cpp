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

#define _AX_X11_UTIL
#define _AX_X11_XSHM
#include "AXX11.h"

#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#include "AXImage.h"

#include "AXImage8.h"
#include "AXImage32.h"
#include "AXGC.h"
#include "AXApp.h"
#include "AXAppRes.h"
#include "AXRect.h"
#include "AXFont.h"
#include "AXPixmap.h"


//-------------------

#define XIMAGE      ((XImage *)m_pImg)
#define SEGINFO     ((XShmSegmentInfo *)m_pShmInfo)

//-------------------


/*!
    @class AXImage
    @brief 直接書き込み可能なイメージ

    - ピクセル値を直接メモリに書き込み可能で、AXWindow や AXPixmap に転送できる。
    - イメージデータはシステムと同じエンディアンで、ピクセル値(VRAMと同じフォーマット)が並んでいる。
    - 16/24/32bit のみ。
    - 描画時の色は上位バイトから (A)-R-G-B の順。_RGB() マクロでも指定可。

    @ingroup draw
*/


AXImage::~AXImage()
{
    free();
}

AXImage::AXImage()
{
    m_pImg      = NULL;
    m_pShmInfo  = NULL;
    m_nWidth = m_nHeight = 0;
}

//! 解放

void AXImage::free()
{
    if(m_pImg)
    {
        if(m_pShmInfo)
            ::XShmDetach(axdisp(), SEGINFO);

        XDestroyImage(XIMAGE);

        if(m_pShmInfo)
        {
            shmdt(SEGINFO->shmaddr);

            delete SEGINFO;
            m_pShmInfo = NULL;
        }

        m_pImg = NULL;
        m_nWidth = m_nHeight = 0;
    }
}

//! 作成

BOOL AXImage::create(int w,int h)
{
    XImage *pimg = NULL;
    XShmSegmentInfo *pinfo;
    int depth;

    free();

    //深さ

    depth = axapp->getDepth();
    if(depth <= 8) return FALSE;

    //-------- XImage

    if(axapp->isSupport_XSHM())
    {
        //XSHM

        pinfo = new XShmSegmentInfo;

        pimg = ::XShmCreateImage(axdisp(), CopyFromParent, depth, ZPixmap, NULL, pinfo, w, h);
        if(!pimg) { delete pinfo; goto NORMAL; }

        pinfo->shmid = shmget(IPC_PRIVATE, pimg->bytes_per_line * h, IPC_CREAT | 0777);

        if(pinfo->shmid < 0)
        {
            //失敗

            XDestroyImage(pimg);
            delete pinfo;
            pimg = NULL;
        }
        else
        {
            //成功

            pinfo->shmaddr = pimg->data = (char *)shmat(pinfo->shmid, 0, 0);
            pinfo->readOnly = FALSE;

            ::XShmAttach(axdisp(), pinfo);

            m_pShmInfo = (LPVOID)pinfo;
        }
    }

    //通常のXImage

NORMAL:

    if(!pimg)
    {
        pimg = ::XCreateImage(axdisp(), CopyFromParent, depth, ZPixmap, 0, NULL, w, h, 32, 0);
        if(!pimg) return FALSE;

        //メモリ確保

        pimg->data = (char *)::malloc(pimg->bytes_per_line * h);

        if(!pimg->data)
        {
            XDestroyImage(pimg);
            return FALSE;
        }
    }

    //----------

    m_pImg      = (LPVOID)pimg;
    m_nWidth    = w;
    m_nHeight   = h;
    m_nPitch    = pimg->bytes_per_line;
    m_nBytes    = (pimg->bits_per_pixel + 7) >> 3;

    return TRUE;
}

//! 再作成
/*!
    水平・垂直のブロックサイズを単位として作成（1以下でブロック計算なし）@n
    サイズが同じ場合はそのまま。
*/

BOOL AXImage::recreate(int w,int h,int hblock,int vblock)
{
    if(hblock > 1)
        w = (w <= hblock)? hblock: (w + hblock - 1) / hblock * hblock;

    if(vblock > 1)
        h = (h <= vblock)? vblock: (h + vblock - 1) / vblock * vblock;

    if(w != m_nWidth || h != m_nHeight)
        return create(w, h);

    return TRUE;
}


//================================
//転送
//================================


//! 転送

void AXImage::put(UINT dstID) const
{
    if(!m_pImg) return;

    if(m_pShmInfo)
    {
        ::XShmPutImage(axdisp(), dstID, (GC)axres->colGC(0), XIMAGE,
                0, 0, 0, 0, m_nWidth, m_nHeight, FALSE);
    }
    else
    {
        ::XPutImage(axdisp(), dstID, (GC)axres->colGC(0), XIMAGE,
                0, 0, 0, 0, m_nWidth, m_nHeight);
    }
}

void AXImage::put(UINT dstID,int dx,int dy) const
{
    put(dstID, dx, dy, 0, 0, m_nWidth, m_nHeight);
}

void AXImage::put(UINT dstID,int dx,int dy,int sx,int sy,int w,int h) const
{
    if(!m_pImg) return;

    if(m_pShmInfo)
    {
        ::XShmPutImage(axdisp(), dstID, (GC)axres->colGC(0), XIMAGE,
                        sx, sy, dx, dy, w, h, FALSE);
    }
    else
    {
        ::XPutImage(axdisp(), dstID, (GC)axres->colGC(0), XIMAGE,
                sx, sy, dx, dy, w, h);
    }
}

void AXImage::put(UINT dstID,const AXGC &gc,int dx,int dy,int sx,int sy,int w,int h) const
{
    if(!m_pImg) return;

    if(m_pShmInfo)
    {
        ::XShmPutImage(axdisp(), dstID, (GC)gc.getGC(), XIMAGE,
                        sx, sy, dx, dy, w, h, FALSE);
    }
    else
    {
        ::XPutImage(axdisp(), dstID, (GC)gc.getGC(), XIMAGE,
                sx, sy, dx, dy, w, h);
    }
}


//=============================
//変換
//=============================


//! AXImage32 から変換して作成

BOOL AXImage::createFromImage32(const AXImage32 &imgSrc)
{
    //作成

    if(!create(imgSrc.getWidth(), imgSrc.getHeight()))
        return FALSE;

    //変換

    setFromImage32(imgSrc);

    return TRUE;
}

//! AXImage32 から変換してセット ※双方のサイズは同じであること

void AXImage::setFromImage32(const AXImage32 &imgSrc)
{
    switch(m_nBytes)
    {
        case 4:
            _to32bit(imgSrc);
            break;
        case 3:
            _to24bit(imgSrc);
            break;
        case 2:
            _to16bit(imgSrc);
            break;
    }
}

//! アイコンなどの無効時のイメージ作成

BOOL AXImage::createDisableImg(const AXImage32 &imgSrc)
{
    LPBYTE pDst;
    LPDWORD pSrc;
    AXImage32::PIXEL *ps;
    int i,v;

    if(!create(imgSrc.getWidth(), imgSrc.getHeight()))
        return FALSE;

    //

    pDst = getBuf();
    pSrc = imgSrc.getBuf();

    for(i = m_nWidth * m_nHeight; i > 0; i--, pSrc++, pDst += m_nBytes)
    {
        ps = (AXImage32::PIXEL *)pSrc;

        v = ((ps->r * 77 + ps->g * 150 + ps->b * 29) >> 8);
        v = v - (v - 128) / 2 + 32;     //コントラストを落とす

        if(v < 0) v = 0; else if(v > 255) v = 255;

        setPixelBuf(pDst, v, v, v);
    }

    return TRUE;
}


//=============================
//取得
//=============================


//! (0,0)の位置のバッファ取得

LPBYTE AXImage::getBuf() const
{
    return (LPBYTE)XIMAGE->data;
}

//! バッファ位置取得

LPBYTE AXImage::getBufPt(int x,int y) const
{
    if(x >= 0 && x < m_nWidth && y >= 0 && y < m_nHeight)
        return (LPBYTE)XIMAGE->data + y * m_nPitch + x * m_nBytes;
    else
        return NULL;
}

//! バッファ位置からRGB値取得

void AXImage::getRGBFromBuf(LPBYTE pbuf,LPINT pr,LPINT pg,LPINT pb) const
{
    DWORD c;

    switch(m_nBytes)
    {
        case 4:
            c = *((LPDWORD)pbuf);
            break;
        case 3:
        #if defined(__BIG_ENDIAN__)
            c = (pbuf[0] << 16) | (pbuf[1] << 8) | pbuf[2];
        #else
            c = (pbuf[2] << 16) | (pbuf[1] << 8) | pbuf[0];
        #endif
            break;
        case 2:
            c = *((LPWORD)pbuf);
            break;
    }

    *pr = axapp->getPixRed(c);
    *pg = axapp->getPixGreen(c);
    *pb = axapp->getPixBlue(c);
}


//=============================
//点セット
//=============================


//! 点描画（RGB値）
/*!
    @param col RGB値。AXImage::COL_XOR(-1) で XOR
*/

void AXImage::setPixel(int x,int y,DWORD col)
{
    LPBYTE pd;

    pd = getBufPt(x, y);
    if(!pd) return;

    if(col == AXImage::COL_XOR)
    {
        //XOR

        switch(m_nBytes)
        {
            case 4:
                *((LPDWORD)pd) ^= 0xffffffff;
                break;
            case 3:
                pd[0] ^= 0xff;
                pd[1] ^= 0xff;
                pd[2] ^= 0xff;
                break;
            case 2:
                *((LPWORD)pd) ^= 0xffff;
                break;
        }
    }
    else
    {
        //通常

        col = axapp->rgbToPix(col);

        switch(m_nBytes)
        {
            case 4:
                *((LPDWORD)pd) = col;
                break;
            case 3:
            #if defined(__BIG_ENDIAN__)
                pd[0] = (BYTE)(col >> 16);
                pd[1] = (BYTE)(col >> 8);
                pd[2] = (BYTE)col;
            #else
                pd[0] = (BYTE)col;
                pd[1] = (BYTE)(col >> 8);
                pd[2] = (BYTE)(col >> 16);
            #endif
                break;
            case 2:
                *((LPWORD)pd) = col;
                break;
        }
    }
}

//! 点描画（ピクセル値）
/*!
    @param colPix RGBから変換済みのピクセル値。-1 でXOR。
*/

void AXImage::setPixelPx(int x,int y,DWORD colPix)
{
    LPBYTE pd;

    pd = getBufPt(x, y);
    if(!pd) return;

    if(colPix == (DWORD)-1)
    {
        //XOR

        switch(m_nBytes)
        {
            case 4:
                *((LPDWORD)pd) ^= 0xffffffff;
                break;
            case 3:
                pd[0] ^= 0xff;
                pd[1] ^= 0xff;
                pd[2] ^= 0xff;
                break;
            case 2:
                *((LPWORD)pd) ^= 0xffff;
                break;
        }
    }
    else
    {
        //通常

        switch(m_nBytes)
        {
            case 4:
                *((LPDWORD)pd) = colPix;
                break;
            case 3:
            #if defined(__BIG_ENDIAN__)
                pd[0] = (BYTE)(colPix >> 16);
                pd[1] = (BYTE)(colPix >> 8);
                pd[2] = (BYTE)colPix;
            #else
                pd[0] = (BYTE)colPix;
                pd[1] = (BYTE)(colPix >> 8);
                pd[2] = (BYTE)(colPix >> 16);
            #endif
                break;
            case 2:
                *((LPWORD)pd) = colPix;
                break;
        }
    }
}

//! バッファ位置に色セット（RGB）

void AXImage::setPixelBuf(LPBYTE pBuf,DWORD col)
{
    col = axapp->rgbToPix(col);

    switch(m_nBytes)
    {
        case 4:
            *((LPDWORD)pBuf) = col;
            break;
        case 3:
        #if defined(__BIG_ENDIAN__)
            pBuf[0] = (BYTE)(col >> 16);
            pBuf[1] = (BYTE)(col >> 8);
            pBuf[2] = (BYTE)col;
        #else
            pBuf[0] = (BYTE)col;
            pBuf[1] = (BYTE)(col >> 8);
            pBuf[2] = (BYTE)(col >> 16);
        #endif
            break;
        case 2:
            *((LPWORD)pBuf) = col;
            break;
    }
}

//! バッファ位置に色セット（R,G,B 各指定）

void AXImage::setPixelBuf(LPBYTE pBuf,int r,int g,int b)
{
    DWORD col = axapp->rgbToPix(r, g, b);

    switch(m_nBytes)
    {
        case 4:
            *((LPDWORD)pBuf) = col;
            break;
        case 3:
        #if defined(__BIG_ENDIAN__)
            pBuf[0] = (BYTE)(col >> 16);
            pBuf[1] = (BYTE)(col >> 8);
            pBuf[2] = (BYTE)col;
        #else
            pBuf[0] = (BYTE)col;
            pBuf[1] = (BYTE)(col >> 8);
            pBuf[2] = (BYTE)(col >> 16);
        #endif
            break;
        case 2:
            *((LPWORD)pBuf) = col;
            break;
    }
}

//! バッファ位置をXOR反転

void AXImage::setPixelBufXor(LPBYTE pBuf)
{
    switch(m_nBytes)
    {
        case 4:
            *((LPDWORD)pBuf) ^= 0xffffffff;
            break;
        case 3:
            pBuf[0] ^= 0xff;
            pBuf[1] ^= 0xff;
            pBuf[2] ^= 0xff;
            break;
        case 2:
            *((LPWORD)pBuf) ^= 0xffff;
            break;
    }
}

//! バッファ位置に色セット（ピクセル値）

void AXImage::setPixelBufPx(LPBYTE pBuf,DWORD colPix)
{
    switch(m_nBytes)
    {
        case 4:
            *((LPDWORD)pBuf) = colPix;
            break;
        case 3:
        #if defined(__BIG_ENDIAN__)
            pBuf[0] = (BYTE)(colPix >> 16);
            pBuf[1] = (BYTE)(colPix >> 8);
            pBuf[2] = (BYTE)colPix;
        #else
            pBuf[0] = (BYTE)colPix;
            pBuf[1] = (BYTE)(colPix >> 8);
            pBuf[2] = (BYTE)(colPix >> 16);
        #endif
            break;
        case 2:
            *((LPWORD)pBuf) = colPix;
            break;
    }
}

//! col（ARGB）のアルファ値で RGB+RGBA->RGB 合成

void AXImage::blendPixel(int x,int y,DWORD col)
{
    LPBYTE pbuf;
    int r,g,b,a;

    pbuf = getBufPt(x, y);
    if(!pbuf) return;

    getRGBFromBuf(pbuf, &r, &g, &b);

    a = _GETA(col) + 1;

    r = ((_GETR(col) - r) * a >> 8) + r;
    g = ((_GETG(col) - g) * a >> 8) + g;
    b = ((_GETB(col) - b) * a >> 8) + b;

    setPixelBuf(pbuf, r, g, b);
}

//! アルファ合成（バッファ位置）

void AXImage::blendPixelBuf(LPBYTE pBuf,DWORD col)
{
    int r,g,b,a;

    getRGBFromBuf(pBuf, &r, &g, &b);

    a = _GETA(col) + 1;

    r = ((_GETR(col) - r) * a >> 8) + r;
    g = ((_GETG(col) - g) * a >> 8) + g;
    b = ((_GETB(col) - b) * a >> 8) + b;

    setPixelBuf(pBuf, r, g, b);
}

//! アルファ合成（バッファ位置、各値指定）

void AXImage::blendPixelBuf(LPBYTE pBuf,int r,int g,int b,int a)
{
    int rr,gg,bb;

    getRGBFromBuf(pBuf, &rr, &gg, &bb);

    a++;

    rr = ((r - rr) * a >> 8) + rr;
    gg = ((g - gg) * a >> 8) + gg;
    bb = ((b - bb) * a >> 8) + bb;

    setPixelBuf(pBuf, rr, gg, bb);
}


//================================
//描画
//================================


//! クリア

void AXImage::clear(DWORD col)
{
    int i;

    col = axapp->rgbToPix(col);
    i   = m_nWidth * m_nHeight;

    switch(m_nBytes)
    {
        case 4:
            LPDWORD pdw;

            for(pdw = (LPDWORD)XIMAGE->data; i > 0; i--)
                *(pdw++) = col;
            break;
        case 3:
            LPBYTE pbt;
            BYTE c1,c2,c3;

        #if defined(__BIG_ENDIAN__)
            c1 = (BYTE)(col >> 16);
            c2 = (BYTE)(col >> 8);
            c3 = (BYTE)col;
        #else
            c1 = (BYTE)col;
            c2 = (BYTE)(col >> 8);
            c3 = (BYTE)(col >> 16);
        #endif

            for(pbt = (LPBYTE)XIMAGE->data; i > 0; i--, pbt += 3)
            {
                pbt[0] = c1; pbt[1] = c2; pbt[2] = c3;
            }
            break;
        case 2:
            LPWORD pw;

            for(pw = (LPWORD)XIMAGE->data; i > 0; i--)
                *(pw++) = (WORD)col;
            break;
    }
}

//! 直線描画

void AXImage::line(int x1,int y1,int x2,int y2,DWORD col)
{
    int sx,sy,dx,dy,a,a1,e;

    col = axapp->rgbToPix(col);

    if(x1 == x2 && y1 == y2)
    {
        setPixelPx(x1, y1, col);
        return;
    }

    //

    dx = (x1 < x2)? x2 - x1: x1 - x2;
    dy = (y1 < y2)? y2 - y1: y1 - y2;
    sx = (x1 <= x2)? 1: -1;
    sy = (y1 <= y2)? 1: -1;

    if(dx >= dy)
    {
        a  = 2 * dy;
        a1 = a - 2 * dx;
        e  = a - dx;

        while(x1 != x2)
        {
            setPixelPx(x1, y1, col);

            if(e >= 0) y1 += sy, e += a1;
            else e += a;
            x1 += sx;
        }
    }
    else
    {
        a  = 2 * dx;
        a1 = a - 2 * dy;
        e  = a - dy;

        while(y1 != y2)
        {
            setPixelPx(x1, y1, col);

            if(e >= 0) x1 += sx, e += a1;
            else e += a;
            y1 += sy;
        }
    }

    setPixelPx(x1, y1, col);
}

//! 直線描画（終点を描画しない）

void AXImage::lineNoEnd(int x1,int y1,int x2,int y2,DWORD col)
{
    int sx,sy,dx,dy,a,a1,e;

    col = axapp->rgbToPix(col);

    if(x1 == x2 && y1 == y2) return;

    //

    dx = (x1 < x2)? x2 - x1: x1 - x2;
    dy = (y1 < y2)? y2 - y1: y1 - y2;
    sx = (x1 <= x2)? 1: -1;
    sy = (y1 <= y2)? 1: -1;

    if(dx >= dy)
    {
        a  = 2 * dy;
        a1 = a - 2 * dx;
        e  = a - dx;

        while(x1 != x2)
        {
            setPixelPx(x1, y1, col);

            if(e >= 0) y1 += sy, e += a1;
            else e += a;
            x1 += sx;
        }
    }
    else
    {
        a  = 2 * dx;
        a1 = a - 2 * dy;
        e  = a - dy;

        while(y1 != y2)
        {
            setPixelPx(x1, y1, col);

            if(e >= 0) x1 += sx, e += a1;
            else e += a;
            y1 += sy;
        }
    }
}

//! 水平線描画

void AXImage::lineH(int x,int y,int w,DWORD col)
{
    col = axapp->rgbToPix(col);

    for(; w > 0; w--, x++)
        setPixelPx(x, y, col);
}

//! 垂直線描画

void AXImage::lineV(int x,int y,int h,DWORD col)
{
    col = axapp->rgbToPix(col);

    for(; h > 0; h--, y++)
        setPixelPx(x, y, col);
}

//! 水平線描画（バッファ位置指定）

void AXImage::lineHBuf(LPBYTE pbuf,int w,DWORD col)
{
    col = axapp->rgbToPix(col);

    switch(m_nBytes)
    {
        case 4:
            LPDWORD pdw;

            for(pdw = (LPDWORD)pbuf; w > 0; w--)
                *(pdw++) = col;
            break;
        case 3:
            BYTE c1,c2,c3;

        #if defined(__BIG_ENDIAN__)
            c1 = (BYTE)(col >> 16);
            c2 = (BYTE)(col >> 8);
            c3 = (BYTE)col;
        #else
            c1 = (BYTE)col;
            c2 = (BYTE)(col >> 8);
            c3 = (BYTE)(col >> 16);
        #endif

            for(; w > 0; w--, pbuf += 3)
            {
                pbuf[0] = c1;
                pbuf[1] = c2;
                pbuf[2] = c3;
            }
            break;
        case 2:
            LPWORD pw;

            for(pw = (LPWORD)pbuf; w > 0; w--)
                *(pw++) = (WORD)col;
            break;
    }
}

//! 四角枠描画

void AXImage::box(int x,int y,int w,int h,DWORD col)
{
    int i,n;

    if(h == 1)
    {
        lineH(x, y, w, col);
        return;
    }
    else if(w == 1)
    {
        lineV(x, y, h, col);
        return;
    }

    //---------

    col = axapp->rgbToPix(col);

    //横

    for(i = x, n = y + h - 1; i < x + w; i++)
    {
        setPixelPx(i, y, col);
        setPixelPx(i, n, col);
    }

    //縦

    for(i = y + 1, n = x + w - 1; i < y + h - 1; i++)
    {
        setPixelPx(x, i, col);
        setPixelPx(n, i, col);
    }
}

//! クリッピング付き四角枠描画

void AXImage::boxClip(int x,int y,int w,int h,DWORD col)
{
    int x2,y2,n1,n2,i;

    if(w < 0 || h < 0) return;

    x2 = x + w - 1, y2 = y + h - 1;

    //全体が範囲外

    if(x >= m_nWidth || y >= m_nHeight || x2 < 0 || y2 < 0)
        return;

    //

    col = axapp->rgbToPix(col);

    //上下

    n1 = (x < 0)? 0: x;
    n2 = (x2 >= m_nWidth)? m_nWidth - 1: x2;

    if(y >= 0)
    {
        for(i = n1; i <= n2; i++)
            setPixelPx(i, y, col);
    }

    if(y2 < m_nHeight && y != y2)
    {
        for(i = n1; i <= n2; i++)
            setPixelPx(i, y2, col);
    }

    //左右

    n1 = (y < 0)? 0: y + 1;
    n2 = (y2 >= m_nHeight)? m_nHeight - 1: y2 - 1;

    if(n1 <= n2)
    {
        if(x >= 0)
        {
            for(i = n1; i <= n2; i++)
                setPixelPx(x, i, col);
        }

        if(x2 < m_nWidth && x != x2)
        {
            for(i = n1; i <= n2; i++)
                setPixelPx(x2, i, col);
        }
    }
}

//! つながっている連続した直線を引く（終点は描画されない）

void AXImage::lines(const AXPoint *ppt,int cnt,DWORD col)
{
    const AXPoint *p = ppt;

    for(; cnt > 1; cnt--, p++)
        lineNoEnd(p->x, p->y, p[1].x, p[1].y, col);
}

//! 四角塗りつぶし描画

void AXImage::fillBox(int x,int y,int w,int h,DWORD col)
{
    int ix,iy;

    col = axapp->rgbToPix(col);

    for(iy = 0; iy < h; iy++)
    {
        for(ix = 0; ix < w; ix++)
            setPixelPx(x + ix, y + iy, col);
    }
}

//! チェック柄(8x8)で塗りつぶし

void AXImage::fillCheck(int x,int y,int w,int h,DWORD col1,DWORD col2)
{
    int ix,iy,yf;
    DWORD c[2];

    c[0] = axapp->rgbToPix(col1);
    c[1] = axapp->rgbToPix(col2);

    for(iy = 0; iy < h; iy++)
    {
        yf = ((y + iy) & 15) >> 3;

        for(ix = 0; ix < w; ix++)
            setPixelPx(x + ix, y + iy, c[yf ^ (((x + ix) & 15) >> 3)]);
    }
}

//! 楕円

void AXImage::ellipse(int x1,int y1,int x2,int y2,DWORD col)
{
    LONGLONG a,b,b1,dx,dy,e,e2;

    if(x1 == x2 && y1 == y2)
    {
        setPixel(x1, y1, col);
        return;
    }

    col = axapp->rgbToPix(col);

    a = (x1 < x2)? x2 - x1: x1 - x2;
    b = (y1 < y2)? y2 - y1: y1 - y2;

    if(a >= b)
    {
        //横長

        b1 = b & 1;
        dx = 4 * (1 - a) * b * b;
        dy = 4 * (b1 + 1) * a * a;
        e = dx + dy + b1 * a * a;

        if(x1 > x2) { x1 = x2; x2 += a; }
        if(y1 > y2) y1 = y2;

        y1 += (b + 1) / 2;
        y2 = y1 - b1;

        a *= 8 * a;
        b1 = 8 * b * b;

        do
        {
            setPixelPx(x2, y1, col);
            if(x1 != x2) setPixelPx(x1, y1, col);
            if(y1 != y2) setPixelPx(x1, y2, col);
            if(x1 != x2 && y1 != y2) setPixelPx(x2, y2, col);

            e2 = 2 * e;
            if(e2 <= dy) { y1++; y2--; e += dy += a; }
            if(e2 >= dx || 2 * e > dy) { x1++; x2--; e += dx += b1; }
        }while(x1 <= x2);
    }
    else
    {
        //縦長

        b1 = a & 1;
        dy = 4 * (1 - b) * a * a;
        dx = 4 * (b1 + 1) * b * b;
        e = dx + dy + b1 * b * b;

        if(y1 > y2) { y1 = y2; y2 += b; }
        if(x1 > x2) x1 = x2;

        x1 += (a + 1) / 2;
        x2 = x1 - b1;

        b *= 8 * b;
        b1 = 8 * a * a;

        do
        {
            setPixelPx(x2, y1, col);
            if(x1 != x2) setPixelPx(x1, y1, col);
            if(y1 != y2) setPixelPx(x1, y2, col);
            if(x1 != x2 && y1 != y2) setPixelPx(x2, y2, col);

            e2 = 2 * e;
            if(e2 <= dx) { x1++; x2--; e += dx += b; }
            if(e2 >= dy || 2 * e > dx) { y1++; y2--; e += dy += b1; }
        }while(y1 <= y2);
    }
}

//! 1bitパターンデータから描画
/*!
    1BYTE で、横 8px 分。@n
    次のY行に行く場合は、次のバイトへ。
*/

void AXImage::drawPattern(int x,int y,DWORD col,const BYTE *pPattern,int ptw,int pth)
{
    const BYTE *pSrc;
    int ix,iy;
    BYTE bit = 0x80;

    col = axapp->rgbToPix(col);

    pSrc = pPattern;

    for(iy = 0; iy < pth; iy++)
    {
        for(ix = 0; ix < ptw; ix++)
        {
            if(*pSrc & bit)
                setPixelPx(x + ix, y + iy, col);

            if(bit == 1)
                pSrc++, bit = 0x80;
            else
                bit >>= 1;
        }

        if(bit != 0x80) pSrc++, bit = 0x80;
    }
}

//! 1bitパターンデータから描画（背景色も塗る）

void AXImage::drawPattern(int x,int y,DWORD colFg,DWORD colBg,const BYTE *pPattern,int ptw,int pth)
{
    const BYTE *pSrc;
    int ix,iy;
    BYTE bit = 0x80;

    colFg = axapp->rgbToPix(colFg);
    colBg = axapp->rgbToPix(colBg);
    pSrc  = pPattern;

    for(iy = 0; iy < pth; iy++)
    {
        for(ix = 0; ix < ptw; ix++)
        {
            setPixelPx(x + ix, y + iy, (*pSrc & bit)? colFg: colBg);

            if(bit == 1)
                pSrc++, bit = 0x80;
            else
                bit >>= 1;
        }

        if(bit != 0x80) pSrc++, bit = 0x80;
    }
}

//! 数値描画
/*!
    数字は5x7。ドットは3x7。

    @param pText 半角数字とドットのみ
*/

void AXImage::drawNumber(int x,int y,LPCSTR pText,DWORD col)
{
    BYTE bits[11][4] = {
        {0xf9,0x99,0x99,0xf0},   //0 (4x7分。左1pxの余白は含まない）
        {0x62,0x22,0x22,0x70},   //1
        {0xf1,0x1f,0x88,0xf0},   //2
        {0xf1,0x1f,0x11,0xf0},   //3
        {0x99,0x9f,0x11,0x10},   //4
        {0xf8,0x8f,0x11,0xf0},   //5
        {0xf8,0x8f,0x99,0xf0},   //6
        {0xf9,0x91,0x11,0x10},   //7
        {0xf9,0x9f,0x99,0xf0},   //8
        {0xf9,0x9f,0x11,0x10},   //9
        {0x00,0x00,0x0c,0xc0}    //.
    };
    int ix,iy,f,w,left;
    LPBYTE ps;

    col = axapp->rgbToPix(col);

    for(left = x + 1; *pText; pText++)
    {
        if(*pText == '.')
        {
            ps = bits[10];
            w  = 2;
        }
        else if(*pText >= '0' && *pText <= '9')
        {
            ps = bits[*pText - '0'];
            w  = 4;
        }
        else
            continue;

        //

        for(iy = 0, f = 0x80; iy < 7; iy++)
        {
            for(ix = 0; ix < w; ix++)
            {
                if(*ps & f)
                    setPixelPx(left + ix, y + iy, col);

                f >>= 1;
            }

            //2行目終了時次のバイトへ
            if(iy & 1)
            {
                ps++;
                f = 0x80;
            }
        }

        left += w + 1;
    }
}


//=============================
//イメージ変換 各ビット毎
//=============================


//! 32BIT

void AXImage::_to32bit(const AXImage32 &imgSrc)
{
    LPDWORD ps,pd;
    int x,y,add;
    AXImage32::PIXEL *px;

    pd  = (LPDWORD)XIMAGE->data;
    ps  = imgSrc.getBuf();
    add = m_nPitch - m_nWidth * 4;

    for(y = m_nHeight; y > 0; y--)
    {
        for(x = m_nWidth; x > 0; x--, ps++)
        {
            px = (AXImage32::PIXEL *)ps;

            *(pd++) = axapp->rgbToPix(px->r, px->g, px->b);
        }

        pd += add;
    }
}

//! 24BIT

void AXImage::_to24bit(const AXImage32 &imgSrc)
{
    LPDWORD ps;
    LPBYTE pd;
    int x,y,add;
    DWORD col;
    AXImage32::PIXEL *px;

    pd  = (LPBYTE)XIMAGE->data;
    ps  = imgSrc.getBuf();
    add = m_nPitch - m_nWidth * 3;

    for(y = m_nHeight; y > 0; y--)
    {
        for(x = m_nWidth; x > 0; x--, pd += 3, ps++)
        {
            px = (AXImage32::PIXEL *)ps;

            col = axapp->rgbToPix(px->r, px->g, px->b);

        #if defined(__BIG_ENDIAN__)
            pd[0] = (BYTE)(col >> 16);
            pd[1] = (BYTE)(col >> 8);
            pd[2] = (BYTE)col;
        #else
            pd[0] = (BYTE)col;
            pd[1] = (BYTE)(col >> 8);
            pd[2] = (BYTE)(col >> 16);
        #endif
        }

        pd += add;
    }
}

//! 16BIT

void AXImage::_to16bit(const AXImage32 &imgSrc)
{
    LPDWORD ps;
    LPBYTE pd;
    int x,y,add;
    DWORD col;
    AXImage32::PIXEL *px;

    pd  = (LPBYTE)XIMAGE->data;
    ps  = imgSrc.getBuf();
    add = m_nPitch - m_nWidth * 2;

    for(y = m_nHeight; y > 0; y--)
    {
        for(x = m_nWidth; x > 0; x--, ps++, pd += 2)
        {
            px = (AXImage32::PIXEL *)ps;

            col = axapp->rgbToPix(px->r, px->g, px->b);

        #if defined(__BIG_ENDIAN__)
            pd[0] = (BYTE)(col >> 8);
            pd[1] = (BYTE)col;
        #else
            pd[0] = (BYTE)col;
            pd[1] = (BYTE)(col >> 8);
        #endif
        }

        pd += add;
    }
}


//=============================
//テキスト
//=============================


//! テキスト描画

void AXImage::drawText(const AXFont &font,int x,int y,int w,int h,LPCUSTR pText,int len,DWORD col)
{
    AXPixmap img;

    if(x + w > m_nWidth)  w = m_nWidth - x;
    if(y + h > m_nHeight) h = m_nHeight - y;

    if(w <= 0 || h <= 0 || len <= 0) return;

    //Pixmapに取得 -> テキスト描画 -> Pixmapから転送

    if(!img.create(w, h)) return;

    put(img.getid(), 0, 0, x, y, w, h);

    AXDrawText dt(img.getid());
    dt.drawRGB(font, 0, 0, pText, len, col);
    dt.end();

    ::XGetSubImage(axdisp(), img.getid(), 0, 0, w, h, 0xffffffff, ZPixmap, XIMAGE, x, y);
}

//! テキスト描画（ASCII）

void AXImage::drawText(const AXFont &font,int x,int y,int w,int h,LPCSTR pText,int len,DWORD col)
{
    AXPixmap img;

    if(x + w > m_nWidth)  w = m_nWidth - x;
    if(y + h > m_nHeight) h = m_nHeight - y;

    if(w <= 0 || h <= 0 || len <= 0) return;

    //Pixmapに取得 -> テキスト描画 -> Pixmapから転送

    if(!img.create(w, h)) return;

    put(img.getid(), 0, 0, x, y, w, h);

    AXDrawText dt(img.getid());
    dt.drawRGB(font, 0, 0, pText, len, col);
    dt.end();

    ::XGetSubImage(axdisp(), img.getid(), 0, 0, w, h, 0xffffffff, ZPixmap, XIMAGE, x, y);
}

//! テキスト描画（折り返しあり）

void AXImage::drawTextWrap(const AXFont &font,int x,int y,int w,int h,LPCUSTR pText,int len,DWORD col)
{
    AXPixmap img;

    if(x + w > m_nWidth)  w = m_nWidth - x;
    if(y + h > m_nHeight) h = m_nHeight - y;

    if(w <= 0 || h <= 0 || len <= 0) return;

    //

    if(!img.create(w, h)) return;

    put(img.getid(), 0, 0, x, y, w, h);

    //

    AXDrawText dt(img.getid());
    dt.drawWrapRGB(font, 0, 0, w, h, pText, len, col);
    dt.end();

    //

    ::XGetSubImage(axdisp(), img.getid(), 0, 0, w, h, 0xffffffff, ZPixmap, XIMAGE, x, y);
}


//=============================
//他イメージから転送
//=============================


//! AXImage8 から通常転送

void AXImage::blt(int dx,int dy,const AXImage8 &imgSrc,int sx,int sy,int w,int h)
{
    LPBYTE pSrc,pDst;
    int addSrc,addDst,x,y;

    if(dx + w >= m_nWidth)  w = m_nWidth - dx;
    if(dy + h >= m_nHeight) h = m_nHeight - dy;
    if(dx < 0) w += dx, sx -= dx, dx = 0;
    if(dy < 0) h += dy, sy -= dy, dy = 0;

    if(w <= 0 || h <= 0) return;

    //

    pDst   = getBufPt(dx, dy);
    pSrc   = imgSrc.getButPt(sx, sy);
    addDst = m_nPitch - m_nBytes * w;
    addSrc = imgSrc.getPitch() - w;

    for(y = h; y > 0; y--)
    {
        for(x = w; x > 0; x--, pSrc++, pDst += m_nBytes)
        {
            setPixelBuf(pDst, imgSrc.getPalCol(*pSrc));
        }

        pDst += addDst;
        pSrc += addSrc;
    }
}

//! AXImage32 から通常転送

void AXImage::blt(int dx,int dy,const AXImage32 &imgSrc,int sx,int sy,int w,int h)
{
    LPDWORD pSrc;
    LPBYTE pDst;
    int addSrc,addDst,x,y;

    if(dx + w >= m_nWidth)  w = m_nWidth - dx;
    if(dy + h >= m_nHeight) h = m_nHeight - dy;
    if(dx < 0) w += dx, sx -= dx, dx = 0;
    if(dy < 0) h += dy, sy -= dy, dy = 0;

    if(w <= 0 || h <= 0) return;

    //

    pDst   = getBufPt(dx, dy);
    pSrc   = imgSrc.getBufPt(sx, sy);
    addDst = m_nPitch - m_nBytes * w;
    addSrc = imgSrc.getWidth() - w;

    for(y = h; y > 0; y--)
    {
        for(x = w; x > 0; x--, pSrc++, pDst += m_nBytes)
        {
            setPixelBuf(pDst, *pSrc);
        }

        pDst += addDst;
        pSrc += addSrc;
    }
}

//! AXImage8 から透過色付き転送

void AXImage::bltTP(int dx,int dy,const AXImage8 &imgSrc,int sx,int sy,int w,int h,DWORD col)
{
    LPBYTE pSrc,pDst;
    int addSrc,addDst,x,y;
    DWORD c;

    if(dx + w >= m_nWidth)  w = m_nWidth - dx;
    if(dy + h >= m_nHeight) h = m_nHeight - dy;
    if(dx < 0) w += dx, sx -= dx, dx = 0;
    if(dy < 0) h += dy, sy -= dy, dy = 0;

    if(w <= 0 || h <= 0) return;

    //

    pDst   = getBufPt(dx, dy);
    pSrc   = imgSrc.getButPt(sx, sy);
    addDst = m_nPitch - m_nBytes * w;
    addSrc = imgSrc.getPitch() - w;

    for(y = h; y > 0; y--)
    {
        for(x = w; x > 0; x--, pSrc++, pDst += m_nBytes)
        {
            c = imgSrc.getPalCol(*pSrc);

            if(c != col)
                setPixelBuf(pDst, c);
        }

        pDst += addDst;
        pSrc += addSrc;
    }
}

//! AXImage32 から透過色付き転送

void AXImage::bltTP(int dx,int dy,const AXImage32 &imgSrc,int sx,int sy,int w,int h,DWORD col)
{
    LPDWORD pSrc;
    LPBYTE pDst;
    int addSrc,addDst,x,y;

    if(dx + w >= m_nWidth)  w = m_nWidth - dx;
    if(dy + h >= m_nHeight) h = m_nHeight - dy;
    if(dx < 0) w += dx, sx -= dx, dx = 0;
    if(dy < 0) h += dy, sy -= dy, dy = 0;

    if(w <= 0 || h <= 0) return;

    //

    pDst   = getBufPt(dx, dy);
    pSrc   = imgSrc.getBufPt(sx, sy);
    addDst = m_nPitch - m_nBytes * w;
    addSrc = imgSrc.getWidth() - w;

    for(y = h; y > 0; y--)
    {
        for(x = w; x > 0; x--, pSrc++, pDst += m_nBytes)
        {
            if((*pSrc & 0xffffff) != col)
                setPixelBuf(pDst, *pSrc);
        }

        pDst += addDst;
        pSrc += addSrc;
    }
}

//! AXImage32 からアルファ合成転送

void AXImage::bltAlpha(int dx,int dy,const AXImage32 &imgSrc,int sx,int sy,int w,int h)
{
    LPDWORD pSrc;
    LPBYTE pDst;
    int addSrc,addDst,x,y;

    if(dx + w >= m_nWidth)  w = m_nWidth - dx;
    if(dy + h >= m_nHeight) h = m_nHeight - dy;
    if(dx < 0) w += dx, sx -= dx, dx = 0;
    if(dy < 0) h += dy, sy -= dy, dy = 0;

    if(w <= 0 || h <= 0) return;

    //

    pDst   = getBufPt(dx, dy);
    pSrc   = imgSrc.getBufPt(sx, sy);
    addDst = m_nPitch - m_nBytes * w;
    addSrc = imgSrc.getWidth() - w;

    for(y = h; y > 0; y--)
    {
        for(x = w; x > 0; x--, pSrc++, pDst += m_nBytes)
        {
            blendPixelBuf(pDst, *pSrc);
        }

        pDst += addDst;
        pSrc += addSrc;
    }
}


//==============================
//拡大縮小
//==============================


//! プレビュー用縮小イメージセット
/*!
    ※あらかじめ create() で作成しておく。@n
    背景をチェック柄にし、pimgSrc を現在のサイズに収まるようにリサイズしてアルファ合成する。

    @param pimgSrc NULL でチェック柄塗りつぶしのみ
    @param dwBkCol1,dwBkCol2 チェック柄の色
*/

void AXImage::setPreviewImg(const AXImage32 *pimgSrc,DWORD dwBkCol1,DWORD dwBkCol2)
{
    LPDWORD pSrcBuf,pSrcY[4];
    LPBYTE pDst,pCol;
    AXImage32::PIXEL *ps;
    int sw,sh,r,g,b,a;
    int x,y,ix,iy,ox[4];
    int fx,fy,addfx,addfy,ff,bky,fxtop;
    BYTE bkrgb[2][3];
    AXRectSize rcs;

    if(!isExist()) return;

    if(!pimgSrc)
    {
        fillCheck(0, 0, m_nWidth, m_nHeight, dwBkCol1, dwBkCol2);
        return;
    }

    if(!pimgSrc->isExist()) return;

    //

    sw = pimgSrc->getWidth();
    sh = pimgSrc->getHeight();

    //位置とサイズ

    rcs.w = sw, rcs.h = sh;
    rcs.inBoxKeepAspect(m_nWidth, m_nHeight, TRUE);

    //縮小なしの場合、通常転送

    if(rcs.w == sw && rcs.h == sh)
    {
        fillCheck(0, 0, m_nWidth, m_nHeight, dwBkCol1, dwBkCol2);
        bltAlpha(rcs.x, rcs.y, *pimgSrc, 0, 0, sw, sh);
        return;
    }

    //==================

    pSrcBuf = pimgSrc->getBuf();
    pDst    = getBuf();

    addfx   = _DTOI((double)sw / rcs.w * (1 << 16));
    addfy   = _DTOI((double)sh / rcs.h * (1 << 16));
    fxtop   = -rcs.x * addfx;

    bkrgb[0][0] = _GETR(dwBkCol1);
    bkrgb[0][1] = _GETG(dwBkCol1);
    bkrgb[0][2] = _GETB(dwBkCol1);
    bkrgb[1][0] = _GETR(dwBkCol2);
    bkrgb[1][1] = _GETG(dwBkCol2);
    bkrgb[1][2] = _GETB(dwBkCol2);

    //--------- Y

    fy = -rcs.y * addfy;

    for(y = 0; y < m_nHeight; y++, fy += addfy)
    {
        fx  = fxtop;
        bky = (y & 15) >> 3;

        //オーバーサンプリングのY位置

        for(iy = 0, ff = fy; iy < 4; iy++, ff += addfy >> 2)
        {
            r = ff >> 16;

            if(r < 0 || r >= sh)
                pSrcY[iy] = NULL;
            else
                pSrcY[iy] = pSrcBuf + r * sw;
        }

        //---------- X

        for(x = 0; x < m_nWidth; x++, fx += addfx, pDst += m_nBytes)
        {
            //オーバーサンプリングのX位置

            for(ix = 0, ff = fx; ix < 4; ix++, ff += addfx >> 2)
            {
                r = ff >> 16;
                ox[ix] = (r < 0 || r >= sw)? -1: r;
            }

            //4x4 オーバーサンプリング

            r = g = b = a = 0;

            for(iy = 0; iy < 4; iy++)
            {
                if(!pSrcY[iy]) continue;

                for(ix = 0; ix < 4; ix++)
                {
                    if(ox[ix] == -1) continue;

                    ps = (AXImage32::PIXEL *)(pSrcY[iy] + ox[ix]);

                    r += ps->r * ps->a;
                    g += ps->g * ps->a;
                    b += ps->b * ps->a;
                    a += ps->a;
                }
            }

            //背景と合成

            pCol = bkrgb[bky ^ ((x & 15) >> 3)];

            if((a >> 4) == 0)
                setPixelBuf(pDst, pCol[0], pCol[1], pCol[2]);
            else
            {
                r /= a; if(r > 255) r = 255;
                g /= a; if(g > 255) g = 255;
                b /= a; if(b > 255) b = 255;
                a >>= 4;

                r = ((r - pCol[0]) * a >> 8) + pCol[0];
                g = ((g - pCol[1]) * a >> 8) + pCol[1];
                b = ((b - pCol[2]) * a >> 8) + pCol[2];

                setPixelBuf(pDst, r, g, b);
            }
        }
    }
}

//! 等倍または拡大イメージセット
/*!
    ※あらかじめ create() で作成しておく。

    @param rcs          描画する範囲
    @param topx,topy    左上の位置（スクロール。拡大状態での位置）
    @param scale        拡大率。100(%)〜
    @param dwExCol      範囲外部分の色
*/

void AXImage::setScaleUpImg(const AXImage32 *pimgSrc,const AXRectSize &rcs,int topx,int topy,int scale,DWORD dwExCol)
{
    int sw,sh,fadd;
    int x,y,stx,fx,fy,n,pitch;
    LPBYTE pDst;
    LPDWORD pSrc,pSrcY;

    sw = pimgSrc->getWidth();
    sh = pimgSrc->getHeight();

    pDst = getBufPt(rcs.x, rcs.y);
    pSrc = pimgSrc->getBuf();

    if(!pSrc) return;

    dwExCol = axapp->rgbToPix(dwExCol);

    fadd  = _DTOI((double)(1 << 18) / scale * 100.0);
    stx   = topx * fadd;
    fy    = topy * fadd;
    pitch = (m_nWidth - rcs.w) * m_nBytes;

    //

    for(y = rcs.h; y > 0; y--, fy += fadd, pDst += pitch)
    {
        n = fy >> 18;

        //範囲外の場合、Xすべて範囲外色

        if(n < 0 || n >= sh)
        {
            for(x = rcs.w; x > 0; x--, pDst += m_nBytes)
                setPixelBufPx(pDst, dwExCol);

            continue;
        }

        //

        pSrcY = pSrc + n * sw;

        for(x = rcs.w, fx = stx; x > 0; x--, pDst += m_nBytes, fx += fadd)
        {
            n = fx >> 18;

            if(n < 0 || n >= sw)
                setPixelBufPx(pDst, dwExCol);
            else
                setPixelBuf(pDst, *(pSrcY + n));
        }
    }
}
