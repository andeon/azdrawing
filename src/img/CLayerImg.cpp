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

#include <iostream>
#include <stdlib.h>

#include "CLayerImg.h"

#include "CTileImg.h"
#include "CSinTable.h"
#include "defStruct.h"

#include "AXRect.h"
#include "AXRand.h"


/*!
    @class CLayerImg
    @brief レイヤ1枚のイメージ

    - CTileImg を２次元配列で管理する。
    - 配列のポインタが TILEPT_EMPTY(1) の場合、UNDO用で元データが空という意味
*/


//--------------------

CSinTable g_pSinTable;
AXRand    g_rand;

DRAWLAYERINFO *CLayerImg::m_pinfo = NULL;
CSinTable *CLayerImg::m_pSinTbl = &g_pSinTable;
AXRand *CLayerImg::m_pRand = &g_rand;

//--------------------


//! キャンバスのイメージサイズ取得

void CLayerImg::getCanvasImgSize(int *pW,int *pH)
{
    *pW = m_pinfo->nImgW;
    *pH = m_pinfo->nImgH;
}


//=========================
//
//=========================


CLayerImg::CLayerImg()
{
    m_ppTile = NULL;
    m_nTileXCnt = m_nTileYCnt = 0;
}

CLayerImg::~CLayerImg()
{
    free();
}


//=========================
// 解放
//=========================


//! すべて解放

void CLayerImg::free()
{
    if(m_ppTile)
    {
        //タイル解放

        freeAllTile();

        //配列解放

        ::free(m_ppTile);

        m_ppTile = NULL;
        m_nTileXCnt = m_nTileYCnt = 0;
    }
}

//! タイルをすべて解放

void CLayerImg::freeAllTile()
{
    CTileImg **pp = m_ppTile;
    int i;

    if(pp)
    {
        for(i = m_nTileXCnt * m_nTileYCnt; i > 0; i--, pp++)
            freeTile(pp);
    }
}

//! 一つのタイルを解放

void CLayerImg::freeTile(CTileImg **ppTile)
{
    if(*ppTile)
    {
        //値が1の場合はUNDO用で元データが空という意味

        if(*ppTile != (CTileImg *)TILEPT_EMPTY)
            delete (*ppTile);

        *ppTile = NULL;
    }
}

//! 指定位置のタイル削除

void CLayerImg::freeTilePos(int tx,int ty)
{
    CTileImg **pp;

    pp = getTilePt(tx, ty);
    if(*pp) freeTile(pp);
}

//! 空のタイルを解放（アンドゥ側でタイルが確保されている部分のみ）
/*!
    アンドゥが確保されているということはイメージが変更された部分。@n
    イメージが変更された部分のうち、すべて透明なタイルは解放してメモリ負担を軽減させる。
*/

void CLayerImg::freeEmptyTileFromUndo()
{
    CTileImg **ppUndo,**ppDst;
    int i;

    ppUndo = (m_pinfo->pimgUndo)->getTileBuf();
    ppDst  = m_ppTile;

    for(i = m_nTileXCnt * m_nTileYCnt; i > 0; i--, ppUndo++, ppDst++)
    {
        //※元が空でも消しゴム描画で空にする場合もある

        if(*ppUndo && *ppDst)
        {
            if((*ppDst)->isFullZero())
                freeTile(ppDst);
        }
    }
}

//! 透明なタイルを解放
/*!
    @return TRUEで全て透明だった
*/

BOOL CLayerImg::freeTileTransparent()
{
    int i;
    CTileImg **pp = m_ppTile;
    BOOL bAll = TRUE;

    for(i = m_nTileXCnt * m_nTileYCnt; i > 0; i--, pp++)
    {
        if(*pp)
        {
            if((*pp)->isFullZero())
                freeTile(pp);
            else
                bAll = FALSE;
        }
    }

    return bAll;
}


//===========================
// 確保
//===========================


//! タイルを確保（失敗した場合 NULL）

CTileImg *CLayerImg::allocTile()
{
    return new(std::nothrow) CTileImg;
}

//! タイルを確保してクリア（失敗した場合 NULL）

CTileImg *CLayerImg::allocTileClear()
{
    CTileImg *p = new(std::nothrow) CTileImg;

    if(p) p->clear();

    return p;
}

//! タイル配列を確保

BOOL CLayerImg::allocTileArray()
{
    DWORD size;

    size = m_nTileXCnt * m_nTileYCnt * sizeof(CTileImg**);

    //確保

    m_ppTile = (CTileImg **)::malloc(size);
    if(!m_ppTile) return FALSE;

    //ゼロクリア

    AXMemZero(m_ppTile, size);

    return TRUE;
}

//! 指定px範囲に合うようタイル配列確保
/*!
    オフセットは左上pxに合わせる
*/

BOOL CLayerImg::allocTileArrayFromPx(const AXRect &rc)
{
    free();

    //タイル数

    m_nTileXCnt = (rc.right - rc.left + 1 + 63) / 64;
    m_nTileYCnt = (rc.bottom - rc.top + 1 + 63) / 64;

    //オフセット

    m_nOffX = rc.left;
    m_nOffY = rc.top;

    m_nTileTopX = m_nTileTopY = 0;

    //配列確保

    return allocTileArray();
}

//! 指定レイヤのタイル範囲とキャンバス範囲を含むタイル配列確保

BOOL CLayerImg::allocTileArray_incImgAndCanvas(CLayerImg *pSrc)
{
    AXRect rc;

    pSrc->getEnableDrawRectPixel(&rc);
    return allocTileArrayFromPx(rc);
}


//===========================
// タイル配列リサイズ
//===========================


//! タイル配列リサイズ

BOOL CLayerImg::resizeTile(int topx,int topy,int xcnt,int ycnt)
{
    CTileImg **ppNew,**ppd;
    int size,x,y,tx,ty;

    //変更なし

    if(topx == m_nTileTopX && topy == m_nTileTopY &&
       xcnt == m_nTileXCnt && ycnt == m_nTileYCnt)
        return TRUE;

    //新規確保

    size = xcnt * ycnt * sizeof(CTileImg**);

    ppNew = (CTileImg **)::malloc(size);
    if(!ppNew) return FALSE;

    AXMemZero(ppNew, size);

    //元データコピー

    ppd = ppNew;

    for(y = 0; y < ycnt; y++)
    {
        ty = y - m_nTileTopY + topy;

        for(x = 0; x < xcnt; x++, ppd++)
        {
            tx = x - m_nTileTopX + topx;

            if(tx >= 0 && tx < m_nTileXCnt && ty >= 0 && ty < m_nTileYCnt)
                *ppd = *(m_ppTile + ty * m_nTileXCnt + tx);
        }
    }

    //入れ替え

    ::free(m_ppTile);

    m_ppTile    = ppNew;
    m_nTileTopX = topx;
    m_nTileTopY = topy;
    m_nTileXCnt = xcnt;
    m_nTileYCnt = ycnt;

    return TRUE;
}

//! 画像サイズ全体に合うようタイル配列リサイズ

BOOL CLayerImg::resizeTile_incImage()
{
    AXRect rc;
    int x1,y1,x2,y2;

    //現在描画可能な範囲(px)

    getEnableDrawRectPixel(&rc);

    //px -> タイル

    calcPixelToTile(&x1, &y1, rc.left, rc.top);
    calcPixelToTile(&x2, &y2, rc.right, rc.bottom);

    //リサイズ

    return resizeTile(m_nTileTopX + x1, m_nTileTopY + y1, x2 - x1 + 1, y2 - y1 + 1);
}

//! 2つのイメージ全体を含めるようタイル配列リサイズ

BOOL CLayerImg::resizeTile_double(const CLayerImg &src)
{
    AXRect rc,rc2;
    int x1,y1,x2,y2;

    //タイルの四隅 -> px

    calcTileRectToPixel(&rc);
    src.calcTileRectToPixel(&rc2);

    //2つのpx範囲を合わせる

    rc.combine(rc2);

    //px -> [this]タイル

    calcPixelToTile(&x1, &y1, rc.left, rc.top);
    calcPixelToTile(&x2, &y2, rc.right, rc.bottom);

    //リサイズ
    /* x1,y1 は this の現在の基準を元にした位置なので、タイルトップ位置は元のトップ位置に加算する。
       （x1,y1 は 0 以下の値である。左・上側に拡張されることはあっても、反対側に縮小することはないため） */

    return resizeTile(m_nTileTopX + x1, m_nTileTopY + y1, x2 - x1 + 1, y2 - y1 + 1);
}


//===========================
// 作成
//===========================


//! 新規作成

BOOL CLayerImg::create(int w,int h)
{
    free();

    //

    m_nTileXCnt = (w + 63) / 64;
    m_nTileYCnt = (h + 63) / 64;

    m_nTileTopX	= m_nTileTopY = 0;
    m_nOffX = m_nOffY = 0;

    //タイル配列確保

    return allocTileArray();
}

//! コピー作成

BOOL CLayerImg::copy(const CLayerImg &src)
{
    CTileImg **ppSrc,**ppDst;
    int i;

    free();

    //情報コピー

    m_nTileXCnt = src.m_nTileXCnt;
    m_nTileYCnt = src.m_nTileYCnt;
    m_nTileTopX = src.m_nTileTopX;
    m_nTileTopY = src.m_nTileTopY;
    m_nOffX     = src.m_nOffX;
    m_nOffY     = src.m_nOffY;

    //タイル配列確保

    if(!allocTileArray()) return FALSE;

    //タイルコピー

    ppSrc = src.getTileBuf();
    ppDst = m_ppTile;

    for(i = m_nTileXCnt * m_nTileYCnt; i > 0; i--)
    {
        if(*ppSrc)
        {
            *ppDst = allocTile();
            if(!(*ppDst)) return FALSE;

            (*ppDst)->copy(*ppSrc);
        }

        ppSrc++;
        ppDst++;
    }

    return TRUE;
}

//! LAYERINFO の情報から作成

BOOL CLayerImg::createFromInfo(const LAYERINFO &info)
{
    free();

    //

    m_nTileXCnt = info.nTileXCnt;
    m_nTileYCnt = info.nTileYCnt;
    m_nTileTopX = info.nTileTopX;
    m_nTileTopY = info.nTileTopY;
    m_nOffX     = info.nOffX;
    m_nOffY     = info.nOffY;

    //タイル配列確保

    return allocTileArray();
}

//! 指定イメージと同じになるようタイル配列を作成

BOOL CLayerImg::createSame(const CLayerImg &src)
{
    BOOL flag = FALSE;

    //情報コピー

    m_nOffX     = src.m_nOffX;
    m_nOffY     = src.m_nOffY;
    m_nTileTopX	= src.m_nTileTopX;
    m_nTileTopY = src.m_nTileTopY;

    //作成するか

    if(!m_ppTile)
        flag = TRUE;
    else
        flag = (m_nTileXCnt != src.m_nTileXCnt || m_nTileYCnt != src.m_nTileYCnt);

    if(!flag) return TRUE;

    //再作成（確保できなかった場合はNULLとなる）

    free();

    m_nTileXCnt	= src.m_nTileXCnt;
    m_nTileYCnt = src.m_nTileYCnt;

    return allocTileArray();
}


//============================
// アンドゥ関連
//============================


//! UNDOデータからタイルセット

void CLayerImg::setTileFromUndo(int tx,int ty,LPBYTE pSrcBuf,int size)
{
    CTileImg **pp;

    pp = getTilePt(tx, ty);

    if(!(*pp))
    {
        *pp = allocTile();
        if(!(*pp)) return;
    }

    (*pp)->decodePackBits(pSrcBuf, size);
}

//! UNDOイメージ用、書き込むデータサイズとタイル数取得
/*!
    ポインタ値=TILEPT_EMPTY の場合を考慮する。
    @param pCnt [0]空でないタイル数 [1]空のタイル数
*/

DWORD CLayerImg::getUndoWriteSize(LPDWORD pCnt)
{
    CTileImg **pp = m_ppTile;
    DWORD size = 0,cnt = 0,cntemp = 0;
    int i;

    for(i = m_nTileXCnt * m_nTileYCnt; i > 0; i--, pp++)
    {
        if(*pp)
        {
            if(*pp == (CTileImg *)TILEPT_EMPTY)
            {
                size += 6;
                cntemp++;
            }
            else
            {
                size += 6 + 4096;
                cnt++;
            }
        }
    }

    pCnt[0] = cnt;
    pCnt[1] = cntemp;

    return size;
}


//============================
// 情報関連
//============================


//! タイルなどのレイヤ情報取得

void CLayerImg::getInfo(LAYERINFO *pd)
{
    pd->nTileXCnt   = m_nTileXCnt;
    pd->nTileYCnt   = m_nTileYCnt;
    pd->nOffX       = m_nOffX;
    pd->nOffY       = m_nOffY;
    pd->nTileTopX   = m_nTileTopX;
    pd->nTileTopY   = m_nTileTopY;
}

//! オフセット位置変更

void CLayerImg::setOffset(int x,int y,BOOL bAdd)
{
    if(bAdd)
    {
        m_nOffX += x;
        m_nOffY += y;
    }
    else
    {
        m_nOffX = x;
        m_nOffY = y;
    }
}


//============================
// 計算
//============================


//! px位置からタイル位置取得
/*!
    @param TRUEで範囲内、FALSEで範囲外
*/

BOOL CLayerImg::calcPixelToTile(int *pTX,int *pTY,int px,int py) const
{
    int tx,ty;

    tx = px - m_nOffX;
    ty = py - m_nOffY;

    if(tx < 0) tx -= 63;    //※マイナスの場合、/64 で正しい位置が取得できるように
    if(ty < 0) ty -= 63;

    tx = tx / 64 - m_nTileTopX;
    ty = ty / 64 - m_nTileTopY;

    *pTX = tx;
    *pTY = ty;

    return (tx >= 0 && tx < m_nTileXCnt && ty >= 0 && ty < m_nTileYCnt);
}

//! タイル位置から左上のpx位置取得

void CLayerImg::calcTileToPixel(int *pPX,int *pPY,int tx,int ty) const
{
    *pPX = (tx + m_nTileTopX) * 64 + m_nOffX;
    *pPY = (ty + m_nTileTopY) * 64 + m_nOffY;
}

//! タイル全体の四隅をpxで取得

void CLayerImg::calcTileRectToPixel(AXRect *prc) const
{
    int x1,y1,x2,y2;

    calcTileToPixel(&x1, &y1, 0, 0);
    calcTileToPixel(&x2, &y2, m_nTileXCnt - 1, m_nTileYCnt - 1);

    prc->left   = x1;
    prc->top    = y1;
    prc->right  = x2 + 63;
    prc->bottom = y2 + 63;
}

//! 指定px範囲内のタイル範囲を取得（32BITへの合成時）
/*!
    @param rcs イメージのpx範囲
    @param FALSEでタイル範囲外
*/

BOOL CLayerImg::getTileRect_inImage(AXRect *prc,const AXRectSize &rcs)
{
    int sx,sy,ex,ey,flag;

    //四隅のタイル位置

    calcPixelToTile(&sx, &sy, rcs.x, rcs.y);
    calcPixelToTile(&ex, &ey, rcs.x + rcs.w - 1, rcs.y + rcs.h - 1);

    //範囲外かどうかのフラグ

    flag  = (sx < 0) | ((ex < 0) << 1);
    flag |= ((sx >= m_nTileXCnt) << 2) | ((ex >= m_nTileXCnt) << 3);

    flag |= ((sy < 0) << 4) | ((ey < 0) << 5);
    flag |= ((sy >= m_nTileYCnt) << 6) | ((ey >= m_nTileYCnt) << 7);

    //範囲外か

    if((flag & 0x03) == 0x03 || (flag & 0x30) == 0x30) return FALSE;
    if((flag & 0x0c) == 0x0c || (flag & 0xc0) == 0xc0) return FALSE;

    //調整

    if(flag & 1) sx = 0;
    if(flag & 8) ex = m_nTileXCnt - 1;

    if(flag & 0x10) sy = 0;
    if(flag & 0x80) ey = m_nTileYCnt - 1;

    //セット

    prc->left   = sx;
    prc->right  = ex;
    prc->top    = sy;
    prc->bottom	= ey;

    return TRUE;
}

//! イメージの範囲内のタイルを、タイルごとに処理する際の各データ取得
/*!
    @param rcs     イメージの範囲(px)
    @param prcTile タイルの範囲が入る
    @param prcImg  rcs を AXRect に変換したものが入る（右・下は +1）
    @param pptTop  左上タイルのpx位置
*/

BOOL CLayerImg::getTileRect_inImageInfo(const AXRectSize &rcs,AXRect *prcTile,AXRect *prcImg,AXPoint *pptTop)
{
    int x,y;

    //イメージの範囲内のタイルの範囲

    if(!getTileRect_inImage(prcTile, rcs)) return FALSE;

    //左上タイルのpx位置

    calcTileToPixel(&x, &y, prcTile->left, prcTile->top);

    pptTop->x = x;
    pptTop->y = y;

    //rcs を AXRect へ

    prcImg->left    = rcs.x;
    prcImg->top     = rcs.y;
    prcImg->right   = rcs.x + rcs.w;
    prcImg->bottom  = rcs.y + rcs.h;

    return TRUE;
}

//! タイル全体とイメージ範囲を含むpx範囲取得（現在描画可能なpx範囲）

void CLayerImg::getEnableDrawRectPixel(AXRect *prc)
{
    AXRect rc1,rc2;

    //タイルの四隅 -> px

    calcTileRectToPixel(&rc1);

    //キャンバス範囲

    rc2.left = rc2.top = 0;
    rc2.right  = m_pinfo->nImgW - 1;
    rc2.bottom = m_pinfo->nImgH - 1;

    //2つのpx範囲を合わせる

    rc1.combine(rc2);

    *prc = rc1;
}

//! AXRect 範囲(px)を描画可能な範囲内に調整
/*!
    @return FALSE で範囲外
*/

BOOL CLayerImg::clipRectInEnableDraw(AXRect *prc)
{
    AXRect rc1;

    getEnableDrawRectPixel(&rc1);

    //範囲外

    if(prc->left > rc1.right || prc->top > rc1.bottom) return FALSE;
    if(prc->right < rc1.left || prc->bottom < rc1.top) return FALSE;

    //調整

    if(prc->left   < rc1.left)   prc->left = rc1.left;
    if(prc->top    < rc1.top)    prc->top = rc1.top;
    if(prc->right  > rc1.right)  prc->right = rc1.right;
    if(prc->bottom > rc1.bottom) prc->bottom = rc1.bottom;

    return TRUE;
}

//! キャンバスの範囲内に調整
/*!
    @return キャンバス範囲外なら FALSE
*/

BOOL CLayerImg::clipRectInCanvas(RECTANDSIZE *prs)
{
    //範囲外

    if((prs->x1 < 0 && prs->x2 < 0) || (prs->x1 >= m_pinfo->nImgW && prs->x2 >= m_pinfo->nImgW))
       return FALSE;

    if((prs->y1 < 0 && prs->y2 < 0) || (prs->y1 >= m_pinfo->nImgH && prs->y2 >= m_pinfo->nImgH))
       return FALSE;

    //

    if(prs->x1 < 0) prs->x1 = 0;
    if(prs->y1 < 0) prs->y1 = 0;
    if(prs->x2 >= m_pinfo->nImgW) prs->x2 = m_pinfo->nImgW - 1;
    if(prs->y2 >= m_pinfo->nImgH) prs->y2 = m_pinfo->nImgH - 1;

    prs->w = prs->x2 - prs->x1 + 1;
    prs->h = prs->y2 - prs->y1 + 1;

    return TRUE;
}

//! 確保されているタイル数取得

int CLayerImg::getAllocTileCnt()
{
    CTileImg **pp = m_ppTile;
    int i,cnt = 0;

    if(!pp) return 0;

    for(i = m_nTileXCnt * m_nTileYCnt; i > 0; i--, pp++)
    {
        if(*pp) cnt++;
    }

    return cnt;
}

//! 確保されているタイルのpx範囲取得（ADW/PSD保存用）

void CLayerImg::getExistTileRectPx(AXRect *prc)
{
    int x,y,minx,maxx,miny,maxy;
    CTileImg **pp = m_ppTile;

    //タイル最小・最大位置

    minx = m_nTileXCnt;
    miny = m_nTileYCnt;
    maxx = -1;
    maxy = -1;

    for(y = 0; y < m_nTileYCnt; y++)
    {
        for(x = 0; x < m_nTileXCnt; x++, pp++)
        {
            if(*pp)
            {
                if(x < minx) minx = x;
                if(y < miny) miny = y;
                if(x > maxx) maxx = x;
                if(y > maxy) maxy = y;
            }
        }
    }

    //タイル -> px変換

    if(maxx == -1)
        //一つもない
        prc->set(0, 0, 0, 0);
    else
    {
        calcTileToPixel(&x, &y, 0, 0);
        prc->set(x + minx * 64, y + miny * 64, x + maxx * 64 + 64, y + maxy * 64 + 64);
    }
}
