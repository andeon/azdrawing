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

#include <stdlib.h>
#include <string.h>

#include "AXImage32.h"

#include "AXMem.h"
#include "AXRect.h"


/*!
    @class AXImage32
    @brief 独自32ビットイメージ

    - 色は DWORD (ARGB)。エンディアンはシステムによる。
    - <b>高速</b> とある描画関数はクリッピングなどを行なっていないので、
      イメージ外にはみ出す可能性がある場合は使わないこと。

    @ingroup draw
*/


AXImage32::~AXImage32()
{
    free();
}

AXImage32::AXImage32()
{
    m_pBuf = NULL;
    m_nWidth = m_nHeight = 0;
}

AXImage32::AXImage32(int w,int h)
{
    create(w, h);
}

//! 解放

void AXImage32::free()
{
    if(m_pBuf)
    {
        ::free(m_pBuf);

        m_pBuf = NULL;
        m_nWidth = m_nHeight = 0;
    }
}

//! 作成

BOOL AXImage32::create(int w,int h)
{
    free();

    m_pBuf = (LPDWORD)::malloc(w * h * 4);
    if(!m_pBuf) return FALSE;

    m_nWidth    = w;
    m_nHeight   = h;

    return TRUE;
}

//! コピー作成

BOOL AXImage32::createCopy(const AXImage32 &imgSrc)
{
    if(!imgSrc.isExist())
        return FALSE;
    else
    {
        if(!create(imgSrc.getWidth(), imgSrc.getHeight())) return FALSE;

        ::memcpy(m_pBuf, imgSrc.getBuf(), m_nWidth * m_nHeight * 4);

        return TRUE;
    }
}


//==============================
//
//==============================


//! 色取得
/*!
    @return 範囲外の場合は0
*/

DWORD AXImage32::getPixel(int x,int y) const
{
    if(x >= 0 && x < m_nWidth && y >= 0 && y < m_nHeight)
        return *(m_pBuf + y * m_nWidth + x);
    else
        return 0;
}

//! 色セット

void AXImage32::setPixel(int x,int y,DWORD col)
{
    if(x >= 0 && x < m_nWidth && y >= 0 && y < m_nHeight)
        *(m_pBuf + y * m_nWidth + x) = col;
}

//! colのアルファ値を元に合成して色セット (RGB+ARGB -> RGB)

void AXImage32::setPixelBlend(int x,int y,DWORD col)
{
    PIXEL *px;
    int a;

    if(x >= 0 && x < m_nWidth && y >= 0 && y < m_nHeight)
    {
        px = (PIXEL *)(m_pBuf + y * m_nWidth + x);

        a = px->a;

        px->r = (_GETR(col) - px->r) * a / 255 + px->r;
        px->g = (_GETG(col) - px->g) * a / 255 + px->g;
        px->b = (_GETB(col) - px->b) * a / 255 + px->b;
    }
}

//! クリア

void AXImage32::clear(DWORD col)
{
    int cnt = m_nWidth * m_nHeight;

#ifdef _AX_OPT_ASM_X86
    ASMV(
        "rep stosl\n\t"
        ::"D"(m_pBuf),"a"(col),"c"(cnt)
    );
#else
    LPDWORD p = m_pBuf;

    for(; cnt > 0; cnt--)
        *(p++) = col;
#endif
}

//! クリア（範囲指定）

void AXImage32::clear(const AXRectSize &rcs,DWORD col)
{
    LPDWORD pDst = getBufPt(rcs.x, rcs.y);
    int pitch;

#ifdef _AX_OPT_ASM_X86
    pitch = (m_nWidth - rcs.w) * 4;

    ASMV(
    "CLEARRECT_LOOPY:\n\t"
        "movl   %2,%%ecx\n\t"
        "rep    stosl\n\t"
        "addl   %4,%0\n\t"
        "decl   %3\n\t"
        "jne    CLEARRECT_LOOPY\n\t"

        ::"D"(pDst),"a"(col),"b"(rcs.w),"d"(rcs.h),"S"(pitch)
        :"%ecx"
    );
#else
    int iy,ix;

    pitch = m_nWidth - rcs.w;

    for(iy = rcs.h; iy > 0; iy--, pDst += pitch)
    {
        for(ix = rcs.w; ix > 0; ix--)
            *(pDst++) = col;
    }
#endif
}

//! イメージ全体をコピーする（2つとも同じサイズであること）

void AXImage32::copy(const AXImage32 &imgSrc)
{
    ::memcpy(m_pBuf, imgSrc.getBuf(), m_nWidth * m_nHeight * 4);
}

//! イメージを範囲コピー（全体は同じサイズであること）

void AXImage32::copy(const AXImage32 &imgSrc,int x,int y,int w,int h)
{
    LPDWORD pd,ps;
    int pitch;

    pd = getBufPt(x, y);
    ps = imgSrc.getBufPt(x, y);

#ifdef _AX_OPT_ASM_X86

    pitch = (m_nWidth - w) * 4;

    ASMV(
    "COPYR_LOOPY:\n\t"
        "movl %2,%%ecx\n\t"
        "rep movsl\n\t"
        "addl %4,%0\n\t"
        "addl %4,%1\n\t"
        "decl %3\n\t"
        "jne COPYR_LOOPY\n\t"

        ::"D"(pd),"S"(ps),"a"(w),"b"(h),"d"(pitch)
        :"%ecx"
    );
#else

    int ix,iy;

    pitch = m_nWidth - w;

    for(iy = h; iy > 0; iy--)
    {
        for(ix = w; ix > 0; ix--)
            *(pd++) = *(ps++);

        ps += pitch;
        pd += pitch;
    }

#endif
}


//==============================
//図形描画
//==============================


//! 水平線（高速）

void AXImage32::lineH(int x,int y,int w,DWORD col)
{
    LPDWORD p = getBufPt(x, y);

#ifdef _AX_OPT_ASM_X86
    ASMV(
         "rep stosl\n\t"
         ::"D"(p),"a"(col),"c"(w)
    );
#else

    for(; w > 0; w--)
        *(p++) = col;

#endif
}

//! 四角枠（高速）

void AXImage32::box(int x,int y,int w,int h,DWORD col)
{
    int i;
    LPDWORD p1,p2;

    //横線

    p1 = getBufPt(x, y);
    p2 = getBufPt(x, y + h - 1);

    for(i = w; i > 0; i--)
    {
        *(p1++) = col;
        *(p2++) = col;
    }

    //縦線

    p1 = getBufPt(x, y + 1);
    p2 = getBufPt(x + w - 1, y + 1);

    for(i = h - 2; i > 0; i--)
    {
        *p1 = col; p1 += m_nWidth;
        *p2 = col; p2 += m_nWidth;
    }
}

//! 四角形塗りつぶし（高速）

void AXImage32::fillBox(int x,int y,int w,int h,DWORD col)
{
    LPDWORD pd = getBufPt(x, y);
    int pitch;

#ifdef _AX_OPT_ASM_X86

    pitch = (m_nWidth - w) * 4;

    ASMV(
    "FILLBOX_LOOPY:\n\t"
        "movl %2,%%ecx\n\t"
        "rep stosl\n\t"
        "addl %4,%0\n\t"
        "decl %3\n\t"
        "jne FILLBOX_LOOPY\n\t"

        ::"D"(pd),"a"(col),"b"(w),"d"(h),"S"(pitch)
        :"%ecx"
    );
#else

    int ix,iy;

    pitch = m_nWidth - w;

    for(iy = h; iy > 0; iy--, pd += pitch)
    {
        for(ix = w; ix > 0; ix--)
            *(pd++) = col;
    }

#endif
}

//! チェック柄(8x8)で塗りつぶし（高速）

void AXImage32::fillCheck(int x,int y,int w,int h,DWORD col1,DWORD col2)
{
    LPDWORD p = getBufPt(x, y);
    int xx,yy,pitch,yf;
    DWORD col[2] = {col1,col2};

    pitch = m_nWidth - w;

    for(yy = 0; yy < h; yy++, p += pitch)
    {
        yf = ((y + yy) & 15) >> 3;

        for(xx = 0; xx < w; xx++)
            *(p++) = col[yf ^ (((x + xx) & 15) >> 3)];
    }
}


//============================
//編集
//============================


//! colExt 以外の色を colDst に置き換える (RGBのみ判定)

void AXImage32::replaceExtract(int x,int y,int w,int h,DWORD colDst,DWORD colExt)
{
    LPDWORD p = getBufPt(x, y);
    int ix,iy;

    colExt &= 0xffffff;

    for(iy = h; iy > 0; iy--, p += m_nWidth - w)
    {
        for(ix = w; ix > 0; ix--, p++)
        {
            if((*p & 0xffffff) != colExt)
                *p = colDst;
        }
    }
}


//==============================
//転送
//==============================


//! 通常転送（クリッピングなし）

void AXImage32::blt(int dx,int dy,const AXImage32 &imgSrc,int sx,int sy,int w,int h)
{
    LPDWORD pSrc,pDst;
    int pitchSrc,pitchDst;

    pSrc        = imgSrc.getBufPt(sx, sy);
    pDst        = getBufPt(dx, dy);
    pitchSrc    = imgSrc.getWidth() - w;
    pitchDst    = m_nWidth - w;

#ifdef _AX_OPT_ASM_X86

    pitchSrc <<= 2;
    pitchDst <<= 2;

    ASMV(
    "BLT_LOOPY:\n\t"
        "movl %5,%%ecx\n\t"
        "rep  movsl\n\t"
        "addl %2,%0\n\t"
        "addl %3,%1\n\t"
        "decl %4\n\t"
        "jne  BLT_LOOPY"

        ::"S"(pSrc),"D"(pDst),"a"(pitchSrc),"b"(pitchDst),"d"(h),"g"(w)
        :"%ecx"
    );
#else

    int x,y;

    for(y = h; y > 0; y--)
    {
        for(x = w; x > 0; x--)
            *(pDst++) = *(pSrc++);

        pSrc += pitchSrc;
        pDst += pitchDst;
    }

#endif
}

//! クリッピング付き転送

void AXImage32::bltClip(int dx,int dy,const AXImage32 &imgSrc,int sx,int sy,int w,int h)
{
    int srcw,srch;

    srcw = imgSrc.getWidth();
    srch = imgSrc.getHeight();

    //調整

    if(sx + w >= srcw) w = srcw - sx;
    if(sy + h >= srch) h = srch - sy;
    if(dx + w >= m_nWidth)  w = m_nWidth - dx;
    if(dy + h >= m_nHeight) h = m_nHeight - dy;

    if(dx < 0) w += dx, sx -= dx, dx = 0;
    if(dy < 0) h += dy, sy -= dy, dy = 0;

    if(w <= 0 || h <= 0) return;

    //転送

    blt(dx, dy, imgSrc, sx, sy, w, h);
}


//===============================
//
//===============================


//! 画像からパレットデータ取得（最大256色）
/*!
    @param pmem メモリが確保され、データがセットされる。DWORD×256個。
    @return 色数。[-1] エラー [0] 256色以上ある
*/

int AXImage32::getPalette(AXMem *pmem) const
{
    LPDWORD pPal,pSrc,pp;
    UINT i,j,pcnt,f,col;

    if(!pmem->allocClear(256 * sizeof(DWORD))) return -1;

    //

    pPal = *pmem;
    pSrc = m_pBuf;
    pcnt = 0;

    for(i = m_nWidth * m_nHeight; i > 0; i--)
    {
        col = *(pSrc++) & 0xffffff;

        //すでにパレットにあるか

        for(j = pcnt, f = 0, pp = pPal; j > 0; j--, pp++)
        {
            if(*pp == col) { f = 1; break; }
        }

        //パレット追加

        if(!f)
        {
            if(pcnt == 256) return 0;

            pPal[pcnt++] = col;
        }
    }

    return pcnt;
}
