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

#include <string.h>

#include "CTileImg.h"

#include "CImage32.h"
#include "CImage8.h"


/*!
    @class CTileImg
    @brief レイヤイメージの１タイル分(64x64)のイメージ
*/


//! ゼロクリア

void CTileImg::clear()
{
#ifdef _AX_OPT_ASM_X86

    ASMV(
        "movl   $1024,%%ecx\n\t"
        "xorl   %%eax,%%eax\n\t"
        "rep    stosl\n\t"
        ::"D"(m_buf) :"%eax","%ecx"
    );

#else

    LPDWORD p = (LPDWORD)m_buf;

    for(int i = 1024; i > 0; i--)
        *(p++) = 0;

#endif
}

//! 値を指定してクリア

void CTileImg::clear(BYTE val)
{
    DWORD dw = ((DWORD)val << 24) | (val << 16) | (val << 8) | val;

#ifdef _AX_OPT_ASM_X86

    ASMV(
        "movl   $1024,%%ecx\n\t"
        "rep    stosl\n\t"
        ::"D"(m_buf),"a"(dw) :"%ecx"
    );

#else

    LPDWORD p = (LPDWORD)m_buf;

    for(int i = 1024; i > 0; i--)
        *(p++) = dw;

#endif
}

//! コピー

void CTileImg::copy(CTileImg *psrc)
{
#ifdef _AX_OPT_ASM_X86

    ASMV(
        "movl   $1024,%%ecx\n\t"
        "rep    movsl\n\t"
        ::"D"(m_buf),"S"(psrc->getBuf()) :"%ecx"
    );

#else

    LPDWORD ps,pd;

    ps = (LPDWORD)psrc->getBuf();
    pd = (LPDWORD)m_buf;

    for(int i = 1024; i > 0; i--)
        *(pd++) = *(ps++);

#endif
}

//! 全体が0か

BOOL CTileImg::isFullZero()
{
    register int i;
    register LPDWORD p = (LPDWORD)m_buf;

    for(i = 1024; i > 0; i--)
    {
        if(*(p++)) return FALSE;
    }

    return TRUE;
}


//===============================
// 編集
//===============================


//! 左右反転

void CTileImg::reverseHorz()
{
    int ix,iy;
    LPBYTE pd,ps;
    BYTE tmp;

    pd = m_buf;
    ps = m_buf + 63;

    for(iy = 64; iy > 0; iy--, pd += 32, ps += 96)
    {
        for(ix = 32; ix > 0; ix--, pd++, ps--)
        {
            tmp = *pd;
            *pd = *ps;
            *ps = tmp;
        }
    }
}


//===============================
// 合成
//===============================


//! 32BITイメージに合成（表示用）
/*!
    @param alpha 0-128
*/

void CTileImg::blend32Bit(CImage32 *pimgDst,int dx,int dy,DWORD col,int alpha,const AXRect &rcClip)
{
    int w,h,sx,sy,pitchs,pitchd;
    LPBYTE pSrc;

    sx = sy = 0;
    w = h = 64;

    //クリッピング

    if(dx < rcClip.left) w += dx - rcClip.left, sx += rcClip.left - dx, dx = rcClip.left;
    if(dy < rcClip.top)  h += dy - rcClip.top , sy += rcClip.top  - dy, dy = rcClip.top;
    if(dx + w > rcClip.right)  w = rcClip.right  - dx;
    if(dy + h > rcClip.bottom) h = rcClip.bottom - dy;

    if(w <= 0 || h <= 0) return;

    //---------------

    pSrc    = m_buf + (sy << 6) + sx;
    pitchs  = 64 - w;

#ifdef _AX_OPT_ASM_X86

    LPDWORD pDst;

    pDst    = pimgDst->getBufPt(dx, dy);
    pitchd  = (pimgDst->getWidth() - w) * 4;

    ASMV(
        "xorl       %%eax,%%eax\n\t"
        "pxor       %%mm7,%%mm7\n\t"
        "movd       %4,%%mm6\n\t"
        "movd       %5,%%mm5\n\t"
        "punpcklbw  %%mm7,%%mm6\n\t"
        "punpcklwd  %%mm5,%%mm5\n\t"
        "punpcklwd  %%mm5,%%mm5\n\t"

    "BLEND32_LOOPY:\n\t"
        "movl       %2,%%ecx\n\t"
    "BLEND32_LOOPX:\n\t"
        "movb       (%0),%%al\n\t"
        "movq       %%mm6,%%mm0\n\t"
        "movd       (%1),%%mm4\n\t"
        "movd       %%eax,%%mm2\n\t"
        "punpcklbw  %%mm7,%%mm4\n\t"
        "punpcklwd  %%mm2,%%mm2\n\t"
        "punpcklwd  %%mm2,%%mm2\n\t"
        "pmullw     %%mm5,%%mm2\n\t"
        "psrlw      $7,%%mm2\n\t"

        "psubw      %%mm4,%%mm0\n\t"    //src - dst
        "pmullw     %%mm2,%%mm0\n\t"    //* alpha
        "psrlw      $8,%%mm0\n\t"       //>>8
        "paddb      %%mm4,%%mm0\n\t"    //+dst

        "packuswb   %%mm7,%%mm0\n\t"
        "movd       %%mm0,(%1)\n\t"

        "incl   %0\n\t"
        "addl   $4,%1\n\t"
        "decl   %%ecx\n\t"
        "jne    BLEND32_LOOPX\n\t"

        "addl   %6,%0\n\t"
        "addl   %7,%1\n\t"
        "decl   %%edx\n\t"
        "jne    BLEND32_LOOPY\n\t"

        "emms\n\t"

        ::"S"(pSrc),"D"(pDst),"b"(w),"d"(h),"g"(col),"g"(alpha),"g"(pitchs),"g"(pitchd)
        :"%eax","%ecx"
    );

#else

    AXImage32::PIXEL *pDst,*pCol;
    int ix,iy,a;

    pDst    = (AXImage32::PIXEL *)pimgDst->getBufPt(dx, dy);
    pSrc    = m_buf + (sy << 6) + sx;
    pCol    = (AXImage32::PIXEL *)&col;
    pitchd  = pimgDst->getWidth() - w;

    for(iy = h; iy > 0; iy--, pSrc += pitchs, pDst += pitchd)
    {
        for(ix = w; ix > 0; ix--, pDst++)
        {
            a = *(pSrc++) * alpha >> 7;

            pDst->r = ((pCol->r - pDst->r) * a >> 8) + pDst->r;
            pDst->g = ((pCol->g - pDst->g) * a >> 8) + pDst->g;
            pDst->b = ((pCol->b - pDst->b) * a >> 8) + pDst->b;
        }
    }

#endif
}

//! 32BITイメージに合成(正確に)

void CTileImg::blend32Bit_real(CImage32 *pimgDst,int dx,int dy,DWORD col,int alpha,const AXRect &rcClip,BOOL bAlpha)
{
    int w,h,sx,sy,pitchs,pitchd,ix,iy,a;
    LPBYTE pSrc;
    AXImage32::PIXEL *pDst,*pCol;
    double sa,da,na;

    sx = sy = 0;
    w = h = 64;

    //クリッピング

    if(dx < rcClip.left) w += dx - rcClip.left, sx += rcClip.left - dx, dx = rcClip.left;
    if(dy < rcClip.top)  h += dy - rcClip.top , sy += rcClip.top  - dy, dy = rcClip.top;
    if(dx + w > rcClip.right)  w = rcClip.right  - dx;
    if(dy + h > rcClip.bottom) h = rcClip.bottom - dy;

    if(w <= 0 || h <= 0) return;

    //

    pSrc   = m_buf + sy * 64 + sx;
    pDst   = (AXImage32::PIXEL *)pimgDst->getBufPt(dx, dy);
    pCol   = (AXImage32::PIXEL *)&col;
    pitchs = 64 - w;
    pitchd = pimgDst->getWidth() - w;

    //

    if(!bAlpha)
    {
        //-------- RGB + RGBA -> RGB

        for(iy = h; iy > 0; iy--)
        {
            for(ix = w; ix > 0; ix--, pDst++)
            {
                a = *(pSrc++) * alpha >> 7;

                pDst->r = (pCol->r - pDst->r) * a / 255 + pDst->r;
                pDst->g = (pCol->g - pDst->g) * a / 255 + pDst->g;
                pDst->b = (pCol->b - pDst->b) * a / 255 + pDst->b;
            }

            pSrc += pitchs;
            pDst += pitchd;
        }
    }
    else
    {
        //------- RGBA + RGBA -> RGBA

        for(iy = h; iy > 0; iy--)
        {
            for(ix = w; ix > 0; ix--, pSrc++, pDst++)
            {
                sa = (*pSrc * alpha >> 7) / 255.0;
                da = pDst->a / 255.0;

                na = sa + da - sa * da;

                //A

                pDst->a = (BYTE)(na * 255 + 0.5);

                //RGB

                if(pDst->a)
                {
                    da *= (1.0 - sa);
                    na = 1.0 / na;

                    pDst->r = (BYTE)((pCol->r * sa + pDst->r * da) * na + 0.5);
                    pDst->g = (BYTE)((pCol->g * sa + pDst->g * da) * na + 0.5);
                    pDst->b = (BYTE)((pCol->b * sa + pDst->b * da) * na + 0.5);
                }
            }

            pSrc += pitchs;
            pDst += pitchd;
        }
    }
}


//============================
//イメージから変換
//============================


//! 32BITイメージから変換
/*!
    @param bAlpha  TRUEでアルファ値をコピー、FALSEでRGBをグレイスケール変換
*/

void CTileImg::convFrom32Bit(const CImage32 &src,int sx,int sy,BOOL bAlpha)
{
    int ix,iy,w,h,sw,sh;
    LPBYTE pd;
    AXImage32::PIXEL *ps;

    sw = src.getWidth();
    sh = src.getHeight();

    w = h = 64;

    if(sx + 64 > sw) w = sw - sx;
    if(sy + 64 > sh) h = sh - sy;

    //

    pd = m_buf;
    ps = (AXImage32::PIXEL *)src.getBufPt(sx, sy);

    for(iy = h; iy > 0; iy--, pd += 64 - w, ps += sw - w)
    {
        for(ix = w; ix > 0; ix--, pd++, ps++)
        {
            if(bAlpha)
                *pd = ps->a;
            else
                *pd = 255 - ((ps->r * 77 + ps->g * 150 + ps->b * 29) >> 8);
        }
    }
}

//! 8Bitイメージから変換

void CTileImg::convFrom8Bit(LPBYTE pSrc,int sx,int sy,int sw,int sh,int pitchs)
{
    int w,h,pitchd;
    register LPBYTE pd,ps;
    register int ix,iy;

    w = h = 64;

    if(sx + 64 > sw) w = sw - sx;
    if(sy + 64 > sh) h = sh - sy;

    if(w <= 0 || h <= 0) return;

    //

    pd = m_buf;
    ps = pSrc + sy * pitchs + sx;

    pitchd = 64 - w;
    pitchs -= w;

    for(iy = h; iy > 0; iy--)
    {
        for(ix = w; ix > 0; ix--)
            *(pd++) = *(ps++);

        pd += pitchd;
        ps += pitchs;
    }
}


//============================
// 転送
//============================


//! 8bitイメージに転送

void CTileImg::bltTo8Bit(CImage8 *pimg,int dx,int dy)
{
    int w,h,dw,dh,sx,sy,pitchs,pitchd;
    register int ix,iy;
    register LPBYTE pSrc,pDst;

    sx = sy = 0;
    w = h = 64;

    dw = pimg->getWidth();
    dh = pimg->getHeight();

    //クリッピング

    if(dx < 0) w += dx, sx -= dx, dx = 0;
    if(dy < 0) h += dy, sy -= dy, dy = 0;
    if(dx + w > dw) w = dw - dx;
    if(dy + h > dh) h = dh - dy;

    if(w <= 0 || h <= 0) return;

    //転送

    pSrc   = m_buf + sy * 64 + sx;
    pDst   = pimg->getBufPt(dx, dy);
    pitchs = 64 - w;
    pitchd = dw - w;

    for(iy = h; iy > 0; iy--)
    {
        for(ix = w; ix > 0; ix--)
        {
            *(pDst++) = *(pSrc++);
        }

        pSrc += pitchs;
        pDst += pitchd;
    }
}


//============================
// UNDO 時の圧縮
//============================


//! PackBits エンコード
/*!
    @return 圧縮後サイズ（4096で無圧縮）
*/

WORD CTileImg::encodePackBits(LPBYTE pDst)
{
    LPBYTE ps;
    BYTE dat[128],val;
    int i,cnt,type,size;

    ps    = m_buf;
    size  = 0;

    //最初は1個の連続状態

    dat[0]  = *(ps++);
    type    = 1;        //[0]非連続 [1]連続
    cnt     = 1;

    //2Byte目以降

    for(i = 1; i < 4096; i++)
    {
        val = *(ps++);

        if(type == 1)
        {
            //------ 連続状態時

            if(dat[cnt - 1] != val)
            {
                //前と違う値が来た

                if(cnt == 1)
                {
                    //1個の場合(A[B])、非連続の2個に切り替え(AB)

                    type = 0;
                }
                else
                {
                    //2個以上の場合出力(AA[B]) -> 連続の1個(B)

                    _encWriteRLE(pDst, &size, dat, cnt, 1);
                    cnt = 0;
                }

                dat[cnt++] = val;
            }
            else
            {
                //続ける

                if(cnt >= 128)  //128個が上限
                {
                    _encWriteRLE(pDst, &size, dat, cnt, 1);
                    cnt = 0;
                }

                dat[cnt++] = val;
            }
        }
        else
        {
            //------- 非連続状態時

            if(dat[cnt - 1] == val)
            {
                //前と同じ値が来たら cnt-1 出力 -> 2個の連続に切り替え
                //(AB[B]) -> (BB) または (A[B]) -> (AB)

                if(cnt >= 2)
                {
                    _encWriteRLE(pDst, &size, dat, cnt - 1, 0);
                    dat[0] = dat[cnt - 1];
                }

                dat[1]  = val;
                type    = 1;
                cnt     = 2;
            }
            else
            {
                //続ける

                if(cnt >= 128)
                {
                    _encWriteRLE(pDst, &size, dat, cnt, 0);
                    cnt = 0;
                }

                dat[cnt++] = val;
            }
        }
    }

    //残り出力

    _encWriteRLE(pDst, &size, dat, cnt, type);

    return size;
}

//! PackBits 書き込み

void CTileImg::_encWriteRLE(LPBYTE pDstBuf,LPINT pDstPos,LPBYTE pDat,int cnt,int type)
{
    char len;
    int datsize,i;
    LPBYTE pd,ps;

    if(*pDstPos >= 4096) return;

    //長さ

    len = cnt - 1;
    if(type == 1) len = -len;

    //データのサイズ

    datsize = (type == 0)? cnt: 1;

    //4KB以上の場合、無圧縮に

    if(*pDstPos + datsize + 1 >= 4096)
    {
        *pDstPos = 4096;
        ::memcpy(pDstBuf, m_buf, 4096);

        return;
    }

    //バッファに追加

    pd = pDstBuf + *pDstPos;
    ps = pDat;

    *((char *)pd) = len;

    for(i = datsize, pd++; i > 0; i--)
        *(pd++) = *(ps++);

    //

    *pDstPos += datsize + 1;
}

//! PackBits展開
/*!
    @param size 4096で無圧縮
*/

void CTileImg::decodePackBits(LPBYTE pSrcBuf,int size)
{
    LPBYTE ps,pd,pdEnd,psEnd;
    int len,i;
    BYTE val;

    //無圧縮

    if(size == 4096)
    {
        ::memcpy(m_buf, pSrcBuf, 4096);
        return;
    }

    //-----------

    pd      = m_buf;
    ps      = pSrcBuf;
    pdEnd   = pd + 4096;
    psEnd   = ps + size;

    while(pd < pdEnd && ps < psEnd)
    {
        len = *((char *)ps);
        ps++;

        if(len <= 0)
        {
            //連続

            val = *(ps++);

            for(i = -len + 1; i > 0; i--)
                *(pd++) = val;
        }
        else
        {
            //非連続

            for(i = len + 1; i > 0; i--)
                *(pd++) = *(ps++);
        }
    }
}
