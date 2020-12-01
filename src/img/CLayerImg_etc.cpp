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
/*
    CLayerImg [etc]
*/

#include "CLayerImg.h"

#include "CTileImg.h"
#include "CImage32.h"
#include "CImage8.h"

#include "AXImage.h"
#include "AXPNGSave.h"


//=============================
// 結合
//=============================


//! 結合（2つのイメージ含むように）
/*!
    @param type [0]通常合成 [1]減算
*/

void CLayerImg::combine(const CLayerImg &src,int alpha,int type)
{
    int tx,ty,txcnt,tycnt;
    int px,py,pxx,pyy,nd,ns;
    LPBYTE pd,ps;
    CTileImg **ppTile;

    //2つのイメージを含むようリサイズ

    if(!resizeTile_double(src)) return;

    //--------- 結合（ソースのタイルを基準に、タイル単位で合成）

    ppTile  = src.getTileBuf();
    txcnt   = src.m_nTileXCnt;
    tycnt   = src.m_nTileYCnt;

    //

    pyy = src.m_nTileTopY * 64 + src.m_nOffY;

    for(ty = 0; ty < tycnt; ty++, pyy += 64)
    {
        pxx = src.m_nTileTopX * 64 + src.m_nOffX;

        for(tx = 0; tx < txcnt; tx++, pxx += 64, ppTile++)
        {
            if(!(*ppTile)) continue;

            //64x64 ピクセル合成

            ps = (*ppTile)->getBuf();

            for(py = 0; py < 64; py++)
            {
                for(px = 0; px < 64; px++, ps++)
                {
                    if(!(*ps)) continue;

                    pd = getPixelPoint_create(pxx + px, pyy + py);
                    if(!pd) continue;

                    ns = ((*ps) * alpha >> 7);
                    nd = *pd;

                    switch(type)
                    {
                        //通常合成
                        case 0:
                            *pd = (BYTE)((((ns + nd) << 8) - ((ns * nd << 8) / 255) + (1 << 7)) >> 8);
                            break;
                        //減算
                        case 1:
                            nd -= ns;
                            if(nd < 0) nd = 0;

                            *pd = nd;
                            break;
                    }
                }
            }
        }
    }
}


//=============================
// 合成
//=============================


//! 32BITイメージに合成（表示用）

void CLayerImg::blend32Bit(CImage32 *pimgDst,const AXRectSize &rcs,DWORD col,int alpha)
{
    int ix,iy,xx,yy,pitch;
    AXRect rc,rcClip;
    AXPoint ptTop;
    CTileImg **pp;

    if(!m_ppTile || alpha == 0) return;

    //各情報取得

    if(!getTileRect_inImageInfo(rcs, &rc, &rcClip, &ptTop)) return;

    //タイルごとに64x64合成

    pp     = getTilePt(rc.left, rc.top);
    pitch  = m_nTileXCnt - (rc.right - rc.left + 1);

    for(iy = rc.top, yy = ptTop.y; iy <= rc.bottom; iy++, yy += 64, pp += pitch)
    {
        for(ix = rc.left, xx = ptTop.x; ix <= rc.right; ix++, xx += 64, pp++)
        {
            if(*pp)
                (*pp)->blend32Bit(pimgDst, xx, yy, col, alpha, rcClip);
        }
    }
}

//! 32BITイメージに合成（保存用に正確に）
/*!
    @param bAlpha アルファ計算あり
*/

void CLayerImg::blend32Bit_real(CImage32 *pimgDst,const AXRectSize &rcs,DWORD col,int alpha,BOOL bAlpha)
{
    int ix,iy,xx,yy,pitch;
    AXRect rc,rcClip;
    AXPoint ptTop;
    CTileImg **pp;

    if(!m_ppTile || alpha == 0) return;

    //各情報取得

    if(!getTileRect_inImageInfo(rcs, &rc, &rcClip, &ptTop)) return;

    //タイルごとに64x64合成

    pp     = getTilePt(rc.left, rc.top);
    pitch  = m_nTileXCnt - (rc.right - rc.left + 1);

    for(iy = rc.top, yy = ptTop.y; iy <= rc.bottom; iy++, yy += 64, pp += pitch)
    {
        for(ix = rc.left, xx = ptTop.x; ix <= rc.right; ix++, xx += 64, pp++)
        {
            if(*pp)
                (*pp)->blend32Bit_real(pimgDst, xx, yy, col, alpha, rcClip, bAlpha);
        }
    }
}


//=============================
// 変換
//=============================


//! 32BITイメージから変換（画像からレイヤ作成）
/*!
    @param bAlpha 32BITのアルファ値を使う
*/

void CLayerImg::convFrom32Bit(const CImage32 &src,BOOL bAlpha)
{
    CTileImg **pp,*pTileTmp;
    int ix,iy;

    pTileTmp = new CTileImg;

    pp = m_ppTile;

    for(iy = 0; iy < m_nTileYCnt; iy++)
    {
        for(ix = 0; ix < m_nTileXCnt; ix++, pp++)
        {
            //作業用タイルに8bit変換

            pTileTmp->clear();
            pTileTmp->convFrom32Bit(src, ix * 64, iy * 64, bAlpha);

            //透明でなければ実際にタイル作成＆コピー

            if(!pTileTmp->isFullZero())
            {
                *pp = allocTile();

                if(*pp) (*pp)->copy(pTileTmp);
            }
        }
    }

    delete pTileTmp;
}

//! 8BITイメージから変換
/*!
    - タイル配列はあらかじめ確保しておくこと
    - 8bitイメージはトップダウン
    - pSrc の (0,0) が タイル (0,0) の位置になる

    @param bPadding 4バイト境界か
*/

void CLayerImg::convFrom8Bit(LPBYTE pSrc,int width,int height,BOOL bPadding)
{
    int ix,iy,sx,sy,pitchs;
    CTileImg **pp;
    CTileImg *pTileTmp;

    pp = m_ppTile;

    pitchs = width;
    if(bPadding) pitchs = (width + 3) & (~3);

    //

    pTileTmp = new CTileImg;

    for(iy = m_nTileYCnt, sy = 0; iy > 0; iy--, sy += 64)
    {
        for(ix = m_nTileXCnt, sx = 0; ix > 0; ix--, pp++, sx += 64)
        {
            //作業用タイルに変換

            pTileTmp->clear();
            pTileTmp->convFrom8Bit(pSrc, sx, sy, width, height, pitchs);

            //透明でなければタイル作成＆コピー

            if(!pTileTmp->isFullZero())
            {
                *pp = allocTile();

                if(*pp) (*pp)->copy(pTileTmp);
            }
        }
    }

    delete pTileTmp;
}

//! 8bitイメージに変換

BOOL CLayerImg::convTo8Bit(CImage8 *pimgDst,const AXRectSize &rcs,BOOL bCreate)
{
    int ix,iy,topx,topy;
    int dx,dy,pitcht;
    AXRect rc;
    CTileImg **pp;

    //8BITイメージ作成

    if(bCreate)
    {
        if(!pimgDst->create(rcs.w, rcs.h)) return FALSE;
    }

    pimgDst->clear();

    //範囲取得

    if(!getTileRect_inImage(&rc, rcs)) return TRUE;

    calcTileToPixel(&topx, &topy, rc.left, rc.top);
    topx -= rcs.x;
    topy -= rcs.y;

    //タイルごとに

    pp     = getTilePt(rc.left, rc.top);
    pitcht = m_nTileXCnt - (rc.right - rc.left + 1);

    for(iy = rc.top, dy = topy; iy <= rc.bottom; iy++, dy += 64, pp += pitcht)
    {
        for(ix = rc.left, dx = topx; ix <= rc.right; ix++, dx += 64, pp++)
        {
            if(*pp)
                (*pp)->bltTo8Bit(pimgDst, dx, dy);
        }
    }

    return TRUE;
}


//==========================
// 他イメージへ描画
//==========================


//! レイヤプレビュー用イメージ描画
/*!
    pimgDst のサイズに収まるように縮小描画。@n
    色はグレイスケール。

    @param srcw,srch イメージの全体サイズ
*/

void CLayerImg::drawLayerPrev(CImage32 *pimgDst,int srcw,int srch)
{
    int ix,iy,dw,dh,sx_f,sy_f,sx,sy,addf,stx;
    int val,sy2,sx2;
    AXImage32::PIXEL *pDst;
    double scale,scale1;

    dw = pimgDst->getWidth();
    dh = pimgDst->getHeight();

    //倍率（倍率の低い方。拡大はしない）

    scale   = (double)dw / srcw;
    scale1  = (double)dh / srch;

    if(scale1 < scale) scale = scale1;
    if(scale > 1.0) scale = 1.0;

    //

    pDst = (AXImage32::PIXEL *)pimgDst->getBuf();

    addf = (int)((1 << 12) / scale);

    stx  = (-dw / 2) * addf + ((srcw / 2) << 12);
    sy_f = (-dh / 2) * addf + ((srch / 2) << 12);

    //----------------

    for(iy = dh; iy > 0; iy--, sy_f += addf)
    {
        sy = sy_f >> 12;

        //Yが範囲外なら1列を範囲外色で塗る

        if(sy < 0 || sy >= srch)
        {
            for(ix = dw; ix > 0; ix--, pDst++)
                pDst->c = 0xb0b0b0;

            continue;
        }

        //------ Yが範囲内

        sy2 = (sy_f + (addf >> 1)) >> 12;

        //X

        for(ix = dw, sx_f = stx; ix > 0; ix--, sx_f += addf, pDst++)
        {
            sx = sx_f >> 12;

            if(sx < 0 || sx >= srcw)
                //Xが範囲外
                pDst->c = 0xb0b0b0;
            else
            {
                sx2 = (sx_f + (addf >> 1)) >> 12;

                val = getPixel(sx, sy);
                val += getPixel(sx2, sy);
                val += getPixel(sx, sy2);
                val += getPixel(sx2, sy2);

                val = 255 - (val >> 2);

                pDst->r = pDst->g = pDst->b = (BYTE)val;
            }
        }
    }
}

//! 色を黒として AXImage にセット（フィルタプレビュー用）
/*!
    rcs は CLayerImg の範囲。pimgDst へは (1,1) を始点として描画。
*/

void CLayerImg::drawFilterPrev(AXImage *pimgDst,const AXRectSize &rcs) const
{
    LPBYTE pd;
    int ix,iy,c,bytes,pitch;

    if(!m_ppTile || !pimgDst->isExist()) return;

    pd    = pimgDst->getBufPt(1, 1);
    bytes = pimgDst->getBytes();
    pitch = pimgDst->getPitch() - bytes * rcs.w;

    for(iy = 0; iy < rcs.h; iy++, pd += pitch)
    {
        for(ix = 0; ix < rcs.w; ix++, pd += bytes)
        {
            c = 255 - getPixel(rcs.x + ix, rcs.y + iy);

            pimgDst->setPixelBuf(pd, c, c, c);
        }
    }
}


//===========================
// ほか
//===========================


//! アルファ付きPNGに保存（レイヤ単体での保存）

BOOL CLayerImg::savePNG32(const AXString &filename,int w,int h,DWORD col)
{
    AXPNGSave png;
    AXPNGSave::INFO info;
    AXMem mem;
    LPBYTE pd;
    BYTE r,g,b;
    int ix,iy;

    info.nWidth     = w;
    info.nHeight    = h;
    info.nBits      = 32;
    info.nPalCnt    = 0;

    if(!png.openFile(filename, &info)) return FALSE;
    if(!png.startImg()) return FALSE;

    if(!mem.alloc(png.getPitch())) return FALSE;

    r = _GETR(col);
    g = _GETG(col);
    b = _GETB(col);

    for(iy = 0; iy < h; iy++)
    {
        pd = mem;

        *(pd++) = 0;   //フィルタタイプ

        for(ix = 0; ix < w; ix++, pd += 4)
        {
            pd[0] = r;
            pd[1] = g;
            pd[2] = b;
            pd[3] = getPixel(ix, iy);
        }

        png.putImg(mem);
    }

    png.endImg();
    png.close();

    return TRUE;
}

//! レベル補正時用、ヒストグラム取得

void CLayerImg::getHistogram(LPDWORD pBuf)
{
    CTileImg **pp = m_ppTile;
    LPBYTE ps;
    int i,j;

    for(i = m_nTileXCnt * m_nTileYCnt; i > 0; i--, pp++)
    {
        //タイルがない場合は透明

        if(!(*pp))
            pBuf[0] += 64 * 64;
        else
        {
            ps = (*pp)->getBuf();

            for(j = 64 * 64; j > 0; j--, ps++)
                pBuf[*ps]++;
        }
    }
}
