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

#include "CImage32.h"

#include "AXImage.h"
#include "AXApp.h"


//----------------------

#define FIXF_BIT    14
#define FIXF_VAL    (1 << FIXF_BIT)

//----------------------

/*!
    @class CImage32
    @brief 32bitイメージ
*/



//***********************************
// キャンバスへの描画
//***********************************
/*
    [CANVASDRAW]

    nBaseX,Y   : イメージ座標での基準位置
    nScrollX,Y : 出力先座標での ( 基準位置 - スクロール位置 )
*/


//! キャンバスへの描画（回転なし・ニアレストネイバー）

void CImage32::drawCanvasNormal(AXImage *pimgDst,CANVASDRAW *pinfo)
{
    int w,h,bpp,pitchd,ix,iy,n;
    long addx,addy,stx,fx,fy;
    LPBYTE pDst;
    LPDWORD pSrcY;
    DWORD pixEx;

    w = pinfo->rcsDst.w;
    h = pinfo->rcsDst.h;

    pixEx = axapp->rgbToPix(pinfo->dwExCol);

    pDst   = pimgDst->getBufPt(pinfo->rcsDst.x, pinfo->rcsDst.y);
    bpp    = pimgDst->getBytes();
    pitchd = pimgDst->getPitch() - w * bpp;

    addx = addy = (long)(FIXF_VAL * pinfo->dScaleDiv);
    if(pinfo->bHRev) addx = -addx;

    stx = (pinfo->rcsDst.x - pinfo->nScrollX) * addx + (pinfo->nBaseX << FIXF_BIT);
    fy  = (pinfo->rcsDst.y - pinfo->nScrollY) * addy + (pinfo->nBaseY << FIXF_BIT);

    //

    for(iy = h; iy > 0; iy--, fy += addy)
    {
        n = fy >> FIXF_BIT;

        //Yが範囲外

        if(n < 0 || n >= m_nHeight)
        {
            pimgDst->lineHBuf(pDst, w, pinfo->dwExCol);
            pDst += pimgDst->getPitch();
            continue;
        }

        //X

        pSrcY = m_pBuf + n * m_nWidth;

        for(ix = w, fx = stx; ix > 0; ix--, fx += addx, pDst += bpp)
        {
            n = fx >> FIXF_BIT;

            if(n >= 0 && n < m_nWidth)
                pimgDst->setPixelBuf(pDst, *(pSrcY + n));
            else
                pimgDst->setPixelBufPx(pDst, pixEx);
        }

        pDst += pitchd;
    }
}

//! キャンバスへの描画（回転なし・縮小[4x4オーバーサンプリング]）

void CImage32::drawCanvasScaleDown(AXImage *pimgDst,CANVASDRAW *pinfo)
{
    int w,h,bpp,pitchd,ix,iy,jx,jy,srcx,srcy,r,g,b,tblX[4];
    long addx,addy,stx,fx,fy,addx2,addy2,jf;
    LPBYTE pDst;
    LPDWORD pSrcY[4];
    AXImage32::PIXEL *pSrc;
    DWORD pixEx;

    w = pinfo->rcsDst.w;
    h = pinfo->rcsDst.h;

    pixEx = axapp->rgbToPix(pinfo->dwExCol);

    pDst   = pimgDst->getBufPt(pinfo->rcsDst.x, pinfo->rcsDst.y);
    bpp    = pimgDst->getBytes();
    pitchd = pimgDst->getPitch() - w * bpp;

    addx = addy = (long)(FIXF_VAL * pinfo->dScaleDiv);
    if(pinfo->bHRev) addx = -addx;

    addx2 = addx >> 2;
    addy2 = addy >> 2;

    stx = (pinfo->rcsDst.x - pinfo->nScrollX) * addx + (pinfo->nBaseX << FIXF_BIT);
    fy  = (pinfo->rcsDst.y - pinfo->nScrollY) * addy + (pinfo->nBaseY << FIXF_BIT);

    //

    for(iy = h; iy > 0; iy--, fy += addy)
    {
        srcy = fy >> FIXF_BIT;

        //Yが範囲外

        if(srcy < 0 || srcy >= m_nHeight)
        {
            pimgDst->lineHBuf(pDst, w, pinfo->dwExCol);
            pDst += pimgDst->getPitch();
            continue;
        }

        //Y位置テーブル

        for(jy = 0, jf = fy; jy < 4; jy++, jf += addy2)
        {
            srcy = jf >> FIXF_BIT;
            if(srcy < 0) srcy = 0; else if(srcy >= m_nHeight) srcy = m_nHeight - 1;

            pSrcY[jy] = m_pBuf + srcy * m_nWidth;
        }

        //X

        for(ix = w, fx = stx; ix > 0; ix--, fx += addx, pDst += bpp)
        {
            srcx = fx >> FIXF_BIT;

            if(srcx < 0 || srcx >= m_nWidth)
                //Xが範囲外
                pimgDst->setPixelBufPx(pDst, pixEx);
            else
            {
                //X位置テーブル

                for(jx = 0, jf = fx; jx < 4; jx++, jf += addx2)
                {
                    srcx = jf >> FIXF_BIT;
                    if(srcx < 0) srcx = 0; else if(srcx >= m_nWidth) srcx = m_nWidth - 1;

                    tblX[jx] = srcx;
                }

                //4x4 平均値

                r = g = b = 0;

                for(jy = 0; jy < 4; jy++)
                {
                    for(jx = 0; jx < 4; jx++)
                    {
                        pSrc = (AXImage32::PIXEL *)(pSrcY[jy] + tblX[jx]);

                        r += pSrc->r;
                        g += pSrc->g;
                        b += pSrc->b;
                    }
                }

                pimgDst->setPixelBuf(pDst, r >> 4, g >> 4, b >> 4);
            }
        }

        pDst += pitchd;
    }
}

//! キャンバスへの描画（回転あり・補間なし）

void CImage32::drawCanvasRotNormal(AXImage *pimgDst,CANVASDRAW *pinfo)
{
    int w,h,bpp,pitchd,ix,iy,sx,sy;
    long stx,sty,fx,fy,dxx,dxy,dyx,dyy;
    LPBYTE pDst;
    DWORD pixEx;
    double scalex;

    w = pinfo->rcsDst.w;
    h = pinfo->rcsDst.h;

    pixEx = axapp->rgbToPix(pinfo->dwExCol);

    pDst   = pimgDst->getBufPt(pinfo->rcsDst.x, pinfo->rcsDst.y);
    bpp    = pimgDst->getBytes();
    pitchd = pimgDst->getPitch() - w * bpp;

    //

    fx = pinfo->rcsDst.x - pinfo->nScrollX;
    fy = pinfo->rcsDst.y - pinfo->nScrollY;

    scalex = pinfo->dScaleDiv;
    if(pinfo->bHRev) scalex = -scalex;

    stx = (long)(((fx * pinfo->dCos - fy * pinfo->dSin) * scalex + pinfo->nBaseX) * FIXF_VAL);
    sty = (long)(((fx * pinfo->dSin + fy * pinfo->dCos) * pinfo->dScaleDiv + pinfo->nBaseY) * FIXF_VAL);

    //xx,yy:cos, xy:sin, yx:-sin

    dxx = (long)(pinfo->dCos * pinfo->dScaleDiv * FIXF_VAL);
    dxy = (long)(pinfo->dSin * pinfo->dScaleDiv * FIXF_VAL);
    dyx = -dxy;
    dyy = dxx;

    if(pinfo->bHRev)
        dxx = -dxx, dyx = -dyx;

    //

    for(iy = h; iy > 0; iy--)
    {
        fx = stx, fy = sty;

        for(ix = w; ix > 0; ix--, pDst += bpp)
        {
            sx = fx >> FIXF_BIT;
            sy = fy >> FIXF_BIT;

            if(sx >= 0 && sx < m_nWidth && sy >= 0 && sy < m_nHeight)
                pimgDst->setPixelBuf(pDst, *(m_pBuf + sy * m_nWidth + sx));
            else
                pimgDst->setPixelBufPx(pDst, pixEx);

            fx += dxx;
            fy += dxy;
        }

        stx  += dyx;
        sty  += dyy;
        pDst += pitchd;
    }
}

//! キャンバスへの描画（回転あり・補間あり[2x2オーバーサンプリング]）

void CImage32::drawCanvasRotHiQuality(AXImage *pimgDst,CANVASDRAW *pinfo)
{
    int w,h,bpp,pitchd,ix,iy,jx,jy,sx,sy,r,g,b;
    long stx,sty,fx,fy,dxx,dxy,dyx,dyy,fxx,fyy,fxx2,fyy2,jdxx,jdxy,jdyx,jdyy;
    LPBYTE pDst;
    AXImage32::PIXEL *pSrc;
    DWORD pixEx;
    double scalex;

    w = pinfo->rcsDst.w;
    h = pinfo->rcsDst.h;

    pixEx = axapp->rgbToPix(pinfo->dwExCol);

    pDst   = pimgDst->getBufPt(pinfo->rcsDst.x, pinfo->rcsDst.y);
    bpp    = pimgDst->getBytes();
    pitchd = pimgDst->getPitch() - w * bpp;

    //

    fx = pinfo->rcsDst.x - pinfo->nScrollX;
    fy = pinfo->rcsDst.y - pinfo->nScrollY;

    scalex = pinfo->dScaleDiv;
    if(pinfo->bHRev) scalex = -scalex;

    stx = (long)(((fx * pinfo->dCos - fy * pinfo->dSin) * scalex + pinfo->nBaseX) * FIXF_VAL);
    sty = (long)(((fx * pinfo->dSin + fy * pinfo->dCos) * pinfo->dScaleDiv + pinfo->nBaseY) * FIXF_VAL);

    //xx,yy:cos, xy:sin, yx:-sin

    dxx = (long)(pinfo->dCos * pinfo->dScaleDiv * FIXF_VAL);
    dxy = (long)(pinfo->dSin * pinfo->dScaleDiv * FIXF_VAL);
    dyx = -dxy;
    dyy = dxx;

    if(pinfo->bHRev)
        dxx = -dxx, dyx = -dyx;

    //

    jdxx = dxx / 2;
    jdxy = dxy / 2;
    jdyx = dyx / 2;
    jdyy = dyy / 2;

    //

    for(iy = h; iy > 0; iy--)
    {
        fx = stx, fy = sty;

        for(ix = w; ix > 0; ix--, pDst += bpp)
        {
            sx = fx >> FIXF_BIT;
            sy = fy >> FIXF_BIT;

            if(sx >= 0 && sx < m_nWidth && sy >= 0 && sy < m_nHeight)
            {
                //範囲内

                r = g = b = 0;
                fxx = fx, fyy = fy;

                for(jy = 2; jy > 0; jy--, fxx += jdyx, fyy += jdyy)
                {
                    fxx2 = fxx, fyy2 = fyy;

                    for(jx = 2; jx > 0; jx--, fxx2 += jdxx, fyy2 += jdxy)
                    {
                        sx = fxx2 >> FIXF_BIT;
                        sy = fyy2 >> FIXF_BIT;

                        if(sx < 0) sx = 0; else if(sx >= m_nWidth) sx = m_nWidth - 1;
                        if(sy < 0) sy = 0; else if(sy >= m_nHeight) sy = m_nHeight - 1;

                        pSrc = (AXImage32::PIXEL *)(m_pBuf + sy * m_nWidth + sx);

                        r += pSrc->r;
                        g += pSrc->g;
                        b += pSrc->b;
                    }
                }

                pimgDst->setPixelBuf(pDst, r >> 2, g >> 2, b >> 2);
            }
            else
                //範囲外
                pimgDst->setPixelBufPx(pDst, pixEx);

            fx += dxx;
            fy += dxy;
        }

        stx  += dyx;
        sty  += dyy;
        pDst += pitchd;
    }
}


//==============================
//グリッド
//==============================


//! グリッド描画（回転なし時）
/*!
    pimgDst の現在イメージにグリッド色の線を合成する。
*/

void CImage32::drawCanvasGrid(AXImage *pimgDst,CANVASDRAW *pinfo,int gridw,int gridh,DWORD dwGridCol)
{
    int iRight,iBottom,itx,ity,v,i,cnt,imgpos,first,end,bpp,pitch;
    LPBYTE pTop,pd;
    AXImage32::PIXEL *pcol;
    double dd,dadd,dScaleX,dScaleXDiv;

    pcol  = (AXImage32::PIXEL *)&dwGridCol;
    bpp   = pimgDst->getBytes();
    pitch = pimgDst->getPitch();

    dScaleX     = pinfo->dScale;
    dScaleXDiv  = pinfo->dScaleDiv;

    if(pinfo->bHRev)
        dScaleX = -dScaleX, dScaleXDiv = -dScaleXDiv;

    iRight  = pinfo->rcsDst.x + pinfo->rcsDst.w;
    iBottom = pinfo->rcsDst.y + pinfo->rcsDst.h;

    //最初のグリッド位置（イメージ位置）
    //※位置は余裕を持たせる

    v = (pinfo->bHRev)? iRight: pinfo->rcsDst.x - 1;

    itx = (int)((v - pinfo->nScrollX) * dScaleXDiv) + pinfo->nBaseX;
    ity = (int)((pinfo->rcsDst.y - 1 - pinfo->nScrollY) * pinfo->dScaleDiv) + pinfo->nBaseY;

    if(itx < gridw) itx = gridw; else itx = itx / gridw * gridw;
    if(ity < gridh) ity = gridh; else ity = ity / gridh * gridh;

    //--------- 縦線

    dd   = (itx - pinfo->nBaseX) * dScaleX + pinfo->nScrollX;
    dadd = gridw * dScaleX;

    first = (int)(-pinfo->nBaseY * pinfo->dScale + pinfo->nScrollY);
    if(first < pinfo->rcsDst.y) first = pinfo->rcsDst.y;

    end = (int)((m_nHeight - pinfo->nBaseY) * pinfo->dScale + pinfo->nScrollY) + 1;
    if(end > iBottom) end = iBottom;

    cnt = end - first;

    //

    pTop = pimgDst->getBufPt(0, first);

    for(imgpos = itx; imgpos < m_nWidth; dd += dadd, imgpos += gridw)
    {
        v = (int)(dd + 0.5);

        if(pinfo->bHRev)
        {
            if(v < pinfo->rcsDst.x) break; else if(v >= iRight) continue;
        }
        else
        {
            if(v < pinfo->rcsDst.x) continue; else if(v >= iRight) break;
        }

        pd = pTop + v * bpp;

        for(i = cnt; i > 0; i--, pd += pitch)
            pimgDst->blendPixelBuf(pd, pcol->r, pcol->g, pcol->b, pcol->a);
    }

    //---------- 横線

    dd   = (ity - pinfo->nBaseY) * pinfo->dScale + pinfo->nScrollY;
    dadd = gridh * pinfo->dScale;

    if(pinfo->bHRev)
    {
        first = (int)((m_nWidth - pinfo->nBaseX) * dScaleX + pinfo->nScrollX);
        end   = (int)(-pinfo->nBaseX * dScaleX + pinfo->nScrollX) + 1;
    }
    else
    {
        first = (int)(-pinfo->nBaseX * dScaleX + pinfo->nScrollX);
        end   = (int)((m_nWidth - pinfo->nBaseX) * dScaleX + pinfo->nScrollX) + 1;
    }

    if(first < pinfo->rcsDst.x) first = pinfo->rcsDst.x;
    if(end > iRight) end = iRight;

    cnt = end - first;

    //

    pTop = pimgDst->getBufPt(first, 0);

    for(imgpos = ity; imgpos < m_nHeight; dd += dadd, imgpos += gridh)
    {
        v = (int)(dd + 0.5);
        if(v < pinfo->rcsDst.y) continue; else if(v >= iBottom) break;

        pd = pTop + v * pitch;

        for(i = cnt; i > 0; i--, pd += bpp)
            pimgDst->blendPixelBuf(pd, pcol->r, pcol->g, pcol->b, pcol->a);
    }
}

//! キャンバスへの描画（回転あり・補間なし・グリッドあり）

void CImage32::drawCanvasRotNormalGrid(AXImage *pimgDst,CANVASDRAW *pinfo)
{
    int w,h,bpp,pitchd,ix,iy,sx,sy;
    long stx,sty,fx,fy,dxx,dxy,dyx,dyy;
    LPBYTE pDst;
    AXImage32::PIXEL dst,*pcol,*pcols;
    DWORD pixEx;
    double scalex;

    w = pinfo->rcsDst.w;
    h = pinfo->rcsDst.h;

    pixEx = axapp->rgbToPix(pinfo->dwExCol);

    pcol  = (AXImage32::PIXEL *)&pinfo->dwGridCol;
    pcols = (AXImage32::PIXEL *)&pinfo->dwGridSplitCol;

    pDst   = pimgDst->getBufPt(pinfo->rcsDst.x, pinfo->rcsDst.y);
    bpp    = pimgDst->getBytes();
    pitchd = pimgDst->getPitch() - w * bpp;

    //

    fx = pinfo->rcsDst.x - pinfo->nScrollX;
    fy = pinfo->rcsDst.y - pinfo->nScrollY;

    scalex = pinfo->dScaleDiv;
    if(pinfo->bHRev) scalex = -scalex;

    stx = (long)(((fx * pinfo->dCos - fy * pinfo->dSin) * scalex + pinfo->nBaseX) * FIXF_VAL);
    sty = (long)(((fx * pinfo->dSin + fy * pinfo->dCos) * pinfo->dScaleDiv + pinfo->nBaseY) * FIXF_VAL);

    //xx,yy:cos, xy:sin, yx:-sin

    dxx = (long)(pinfo->dCos * pinfo->dScaleDiv * FIXF_VAL);
    dxy = (long)(pinfo->dSin * pinfo->dScaleDiv * FIXF_VAL);
    dyx = -dxy;
    dyy = dxx;

    if(pinfo->bHRev)
        dxx = -dxx, dyx = -dyx;

    //

    for(iy = h; iy > 0; iy--)
    {
        fx = stx, fy = sty;

        for(ix = w; ix > 0; ix--, pDst += bpp)
        {
            sx = fx >> FIXF_BIT;
            sy = fy >> FIXF_BIT;

            if(sx >= 0 && sx < m_nWidth && sy >= 0 && sy < m_nHeight)
            {
                dst.c = *(m_pBuf + sy * m_nWidth + sx);

                //グリッド

                if(pinfo->dwGridFlags & 1)
                {
                    if((sx && !(sx % pinfo->nGridW)) || (sy && !(sy % pinfo->nGridH)))
                    {
                        dst.r = ((pcol->r - dst.r) * pcol->a >> 8) + dst.r;
                        dst.g = ((pcol->g - dst.g) * pcol->a >> 8) + dst.g;
                        dst.b = ((pcol->b - dst.b) * pcol->a >> 8) + dst.b;
                    }
                }

                //分割線

                if(pinfo->dwGridFlags & 2)
                {
                    if((sx && !(sx % pinfo->nGridSplitW)) || (sy && !(sy % pinfo->nGridSplitH)))
                    {
                        dst.r = ((pcols->r - dst.r) * pcols->a >> 8) + dst.r;
                        dst.g = ((pcols->g - dst.g) * pcols->a >> 8) + dst.g;
                        dst.b = ((pcols->b - dst.b) * pcols->a >> 8) + dst.b;
                    }
                }

                pimgDst->setPixelBuf(pDst, dst.c);
            }
            else
                pimgDst->setPixelBufPx(pDst, pixEx);

            fx += dxx;
            fy += dxy;
        }

        stx  += dyx;
        sty  += dyy;
        pDst += pitchd;
    }
}

//! キャンバスへの描画（回転あり・補間あり[2x2オーバーサンプリング]・グリッドあり）

void CImage32::drawCanvasRotHiQualityGrid(AXImage *pimgDst,CANVASDRAW *pinfo)
{
    int w,h,bpp,pitchd,ix,iy,jx,jy,sx,sy,r,g,b,jsx,jsy;
    long stx,sty,fx,fy,dxx,dxy,dyx,dyy,fxx,fyy,fxx2,fyy2,jdxx,jdxy,jdyx,jdyy;
    LPBYTE pDst;
    AXImage32::PIXEL *pSrc,*pcol,*pcols;
    DWORD pixEx;
    double scalex;

    w = pinfo->rcsDst.w;
    h = pinfo->rcsDst.h;

    pixEx = axapp->rgbToPix(pinfo->dwExCol);

    pcol  = (AXImage32::PIXEL *)&pinfo->dwGridCol;
    pcols = (AXImage32::PIXEL *)&pinfo->dwGridSplitCol;

    pDst   = pimgDst->getBufPt(pinfo->rcsDst.x, pinfo->rcsDst.y);
    bpp    = pimgDst->getBytes();
    pitchd = pimgDst->getPitch() - w * bpp;

    //

    fx = pinfo->rcsDst.x - pinfo->nScrollX;
    fy = pinfo->rcsDst.y - pinfo->nScrollY;

    scalex = pinfo->dScaleDiv;
    if(pinfo->bHRev) scalex = -scalex;

    stx = (long)(((fx * pinfo->dCos - fy * pinfo->dSin) * scalex + pinfo->nBaseX) * FIXF_VAL);
    sty = (long)(((fx * pinfo->dSin + fy * pinfo->dCos) * pinfo->dScaleDiv + pinfo->nBaseY) * FIXF_VAL);

    //xx,yy:cos, xy:sin, yx:-sin

    dxx = (long)(pinfo->dCos * pinfo->dScaleDiv * FIXF_VAL);
    dxy = (long)(pinfo->dSin * pinfo->dScaleDiv * FIXF_VAL);
    dyx = -dxy;
    dyy = dxx;

    if(pinfo->bHRev)
        dxx = -dxx, dyx = -dyx;

    //

    jdxx = dxx / 2;
    jdxy = dxy / 2;
    jdyx = dyx / 2;
    jdyy = dyy / 2;

    //

    for(iy = h; iy > 0; iy--)
    {
        fx = stx, fy = sty;

        for(ix = w; ix > 0; ix--, pDst += bpp)
        {
            sx = fx >> FIXF_BIT;
            sy = fy >> FIXF_BIT;

            if(sx >= 0 && sx < m_nWidth && sy >= 0 && sy < m_nHeight)
            {
                //2x2

                r = g = b = 0;
                fxx = fx, fyy = fy;

                for(jy = 2; jy > 0; jy--, fxx += jdyx, fyy += jdyy)
                {
                    fxx2 = fxx, fyy2 = fyy;

                    for(jx = 2; jx > 0; jx--, fxx2 += jdxx, fyy2 += jdxy)
                    {
                        jsx = fxx2 >> FIXF_BIT;
                        jsy = fyy2 >> FIXF_BIT;

                        if(jsx < 0) jsx = 0; else if(jsx >= m_nWidth) jsx = m_nWidth - 1;
                        if(jsy < 0) jsy = 0; else if(jsy >= m_nHeight) jsy = m_nHeight - 1;

                        pSrc = (AXImage32::PIXEL *)(m_pBuf + jsy * m_nWidth + jsx);

                        r += pSrc->r;
                        g += pSrc->g;
                        b += pSrc->b;
                    }
                }

                r >>= 2;
                g >>= 2;
                b >>= 2;

                //グリッド

                if(pinfo->dwGridFlags & 1)
                {
                    if((sx && !(sx % pinfo->nGridW)) || (sy && !(sy % pinfo->nGridH)))
                    {
                        r = ((pcol->r - r) * pcol->a >> 8) + r;
                        g = ((pcol->g - g) * pcol->a >> 8) + g;
                        b = ((pcol->b - b) * pcol->a >> 8) + b;
                    }
                }

                //分割線

                if(pinfo->dwGridFlags & 2)
                {
                    if((sx && !(sx % pinfo->nGridSplitW)) || (sy && !(sy % pinfo->nGridSplitH)))
                    {
                        r = ((pcols->r - r) * pcols->a >> 8) + r;
                        g = ((pcols->g - g) * pcols->a >> 8) + g;
                        b = ((pcols->b - b) * pcols->a >> 8) + b;
                    }
                }

                pimgDst->setPixelBuf(pDst, r, g, b);
            }
            else
                pimgDst->setPixelBufPx(pDst, pixEx);

            fx += dxx;
            fy += dxy;
        }

        stx  += dyx;
        sty  += dyy;
        pDst += pitchd;
    }
}
