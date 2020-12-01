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
    CLayerImg [pixel] - ピクセル関連
*/

#include "CLayerImg.h"

#include "CTileImg.h"
#include "CImage8.h"
#include "defStruct.h"



//=============================
// ツールなどでの描画時用、色セット
//=============================


//! 色セット（描画時用。アンドゥ用コピーも同時に行う）
/*!
    タイルがない場合は作成される。@n
    タイル配列外の場合は、キャンバス範囲内なら配列がリサイズされる。
*/

void CLayerImg::setPixelDraw(int x,int y,BYTE val)
{
    int tx,ty,dstval,bEmpty = FALSE;
    CTileImg **pp,**ppUndo;
    FLAGRECT *prc;

    //-------- 描画後の色＆タイル位置取得（描画後と描画前の色が同じならそのまま）

    dstval = _setPixelDraw_getCol(x, y, val, &pp, &tx, &ty);

    if(dstval == -1) return;

    //-------- タイル配列範囲外

    if(!pp)
    {
        //キャンバス範囲外なら描画しない

        if(x < 0 || y < 0 || x >= m_pinfo->nImgW || y >= m_pinfo->nImgH)
            return;

        //配列リサイズ＆タイル位置再取得

        if(!resizeTile_incImage()) return;

        calcPixelToTile(&tx, &ty, x, y);

        pp = m_ppTile + ty * m_nTileXCnt + tx;

        //UNDO用配列もリサイズ
        //※失敗したらアンドゥは機能させない

        if(!(m_pinfo->pimgUndo)->resizeTile(m_nTileTopX, m_nTileTopY, m_nTileXCnt, m_nTileYCnt))
        {
            m_pinfo->fUndoErr |= 2;
            (m_pinfo->pimgUndo)->free();
        }
    }

    //---------- タイルがない場合確保

    if(!(*pp))
    {
        *pp = allocTileClear();

        if(!(*pp))
        {
            m_pinfo->fUndoErr |= 1;
            return;
        }

        bEmpty = TRUE;
    }

    //----------- UNDO用に元イメージコピー

    ppUndo = (m_pinfo->pimgUndo)->getTileBuf();

    if(ppUndo)
    {
        ppUndo += ty * m_nTileXCnt + tx;

        if(bEmpty)
            //元が空の場合1をセット
            *ppUndo = (CTileImg *)TILEPT_EMPTY;
        else if(!(*ppUndo))
        {
            //すでにタイルがある場合はコピー済み、なければ作成&コピー

            *ppUndo = allocTile();

            if(!(*ppUndo))
                m_pinfo->fUndoErr |= 2;
            else
                (*ppUndo)->copy(*pp);
        }
    }

    //--------- 描画範囲

    prc = &m_pinfo->rcfDraw;

    if(prc->flag)
    {
        if(x < prc->x1) prc->x1 = x;
        if(y < prc->y1) prc->y1 = y;
        if(x > prc->x2) prc->x2 = x;
        if(y > prc->y2) prc->y2 = y;
    }
    else
    {
        prc->x1 = prc->x2 = x;
        prc->y1 = prc->y2 = y;
        prc->flag = TRUE;
    }

    //--------- 色セット

    (*pp)->setPixel((x - m_nOffX) & 63, (y - m_nOffY) & 63, dstval);
}

//! 描画後の色を取得（＆タイル位置取得）
/*!
    @param ns セットしたい色
    @param pppRetTile NULLでタイル範囲外
    @return 描画後の色（-1で変化なし）
*/

int CLayerImg::_setPixelDraw_getCol(int x,int y,int ns,CTileImg ***pppRetTile,LPINT pTX,LPINT pTY)
{
    CTileImg **ppTile;
    LPBYTE pDst = NULL;
    int nd = 0,result,maskval = 0xffff,tx,ty;

    //選択範囲外なら処理なし

    if(m_pinfo->pimgSel)
    {
        if((m_pinfo->pimgSel)->getPixel(x, y) == 0)
            return -1;
    }

    //レイヤマスク値取得（透明なら処理なし）

    if(m_pinfo->pimgMask)
    {
        maskval = (m_pinfo->pimgMask)->getPixel(x, y);
        if(maskval == 0) return -1;
    }

    //描画先の色取得＆タイル位置セット

    if(!calcPixelToTile(&tx, &ty, x, y))
        *pppRetTile = NULL;
    else
    {
        ppTile = m_ppTile + ty * m_nTileXCnt + tx;

        if(*ppTile)
        {
            pDst = (*ppTile)->getBufPt((x - m_nOffX) & 63, (y - m_nOffY) & 63);
            nd   = *pDst;
        }

        *pppRetTile = ppTile;
        *pTX        = tx;
        *pTY        = ty;
    }

    //テクスチャ適用
    /* 位置はオフセットを考慮させるとイメージ移動しても画像がつながるが、
        保存->開くなどでオフセット位置が変わるとずれてしまうため、(0,0) を基準とする */

    if(m_pinfo->pTexture)
        ns = (ns * (m_pinfo->pTexture)->getPixelTexture(x, y) + 127) / 255;

    //各タイプ (result に値セット)

    switch(m_pinfo->nColSetFunc)
    {
        //合成
        case COLSETF_BLEND:
            result = (BYTE)((((ns + nd) << 5) - ((ns * nd << 5) / 255) + (1 << 4)) >> 5);
            break;
        //比較上書き
        case COLSETF_COMP:
            if(ns > nd)
                result = ns;
            else
                result = nd;
            break;
        //消しゴム
        case COLSETF_ERASE:
            result = nd - ns;
            if(result < 0) result = 0;
            break;
        //上書き
        case COLSETF_OVERWRITE:
        case COLSETF_OVERWRITE2:
            result = ns;
            break;
        //ぼかし
        case COLSETF_BLUR:
            if(ns)
                result = _setPixelDraw_getBlur(pDst, x, y);
            else
                result = nd;
            break;
    }

    //レイヤマスク適用

    if(result > maskval) result = maskval;

    //描画前と描画後が同じなら色をセットしない

    if(result == nd) return -1;

    return result;
}

//! ぼかし後の色取得

int CLayerImg::_setPixelDraw_getBlur(LPBYTE pd,int x,int y)
{
    int px,py,ix,iy,c = 0;
    LPBYTE p;

    px = (x - m_nOffX) & 63;
    py = (y - m_nOffY) & 63;

    if(pd && px >= 1 && px <= 62 && py >= 1 && py <= 62)
    {
        //確保されているタイルかつ3x3がタイル内の場合

        for(iy = 0, p = pd - 64; iy < 3; iy++, p += 64)
            for(ix = -1; ix <= 1; ix++)
                c += *(p + ix);
    }
    else
    {
        //タイルがまたがる場合、またはタイル未確保の場合

        for(iy = -1; iy <= 1; iy++)
            for(ix = -1; ix <= 1; ix++)
                c += getPixel(x + ix, y + iy);
    }

    //色

    return ((c << 4) / 9 + 8) >> 4;
}


//===========================
// 色・バッファ位置取得
//===========================


//! 指定px位置の色取得（範囲外は0）

BYTE CLayerImg::getPixel(int x,int y) const
{
    int tx,ty;
    CTileImg *pt;

    if(calcPixelToTile(&tx, &ty, x, y))
    {
        pt = getTile(tx, ty);

        if(pt)
            return pt->getPixel((x - m_nOffX) & 63, (y - m_nOffY) & 63);
    }

    return 0;
}

//! クリッピング有無指定付き色取得

BYTE CLayerImg::getPixel(int x,int y,BOOL bClip) const
{
    int tx,ty;
    CTileImg *pt;

    if(bClip)
    {
        if(x < 0) x = 0; else if(x >= m_pinfo->nImgW) x = m_pinfo->nImgW - 1;
        if(y < 0) y = 0; else if(y >= m_pinfo->nImgH) y = m_pinfo->nImgH - 1;
    }

    if(calcPixelToTile(&tx, &ty, x, y))
    {
        pt = getTile(tx, ty);

        if(pt)
            return pt->getPixel((x - m_nOffX) & 63, (y - m_nOffY) & 63);
    }

    return 0;
}

//! クリッピング付き色取得

BYTE CLayerImg::getPixelClip(int x,int y)
{
    int tx,ty;
    CTileImg *pt;

    if(x < 0) x = 0; else if(x >= m_pinfo->nImgW) x = m_pinfo->nImgW - 1;
    if(y < 0) y = 0; else if(y >= m_pinfo->nImgH) y = m_pinfo->nImgH - 1;

    if(calcPixelToTile(&tx, &ty, x, y))
    {
        pt = getTile(tx, ty);

        if(pt)
            return pt->getPixel((x - m_nOffX) & 63, (y - m_nOffY) & 63);
    }

    return 0;
}

//! ポインタ位置取得

LPBYTE CLayerImg::getPixelPoint(int x,int y)
{
    int tx,ty;
    CTileImg *pt;

    if(calcPixelToTile(&tx, &ty, x, y))
    {
        pt = getTile(tx, ty);

        if(pt)
            return pt->getBufPt((x - m_nOffX) & 63, (y - m_nOffY) & 63);
    }

    return NULL;
}

//! ポインタ位置取得
/*!
    タイルがない場合は作成する。ただし、タイル配列外は除く。
*/

LPBYTE CLayerImg::getPixelPoint_create(int x,int y)
{
    int tx,ty;
    CTileImg **pp;

    //タイル

    if(!calcPixelToTile(&tx, &ty, x, y)) return NULL;

    //タイルがない場合確保

    pp = m_ppTile + ty * m_nTileXCnt + tx;

    if(!(*pp))
        *pp = allocTileClear();

    //タイルからpx位置

    return (*pp)->getBufPt((x - m_nOffX) & 63, (y - m_nOffY) & 63);
}


//=============================
// 色セット
//=============================


//! 色セット（タイルを作成してセット。値が 0 の場合も含む）

void CLayerImg::setPixel_create(int x,int y,BYTE val)
{
    LPBYTE p;

    p = getPixelPoint_create(x, y);
    if(p) *p = val;
}

//! 色セット（タイルを作成してセット。値が 0 の場合はなし）

void CLayerImg::setPixel_create2(int x,int y,BYTE val)
{
    LPBYTE p;

    if(val)
    {
        p = getPixelPoint_create(x, y);
        if(p) *p = val;
    }
}

//! 色セット（色を 0 にする）

void CLayerImg::setPixel_clear(int x,int y,BYTE val)
{
    LPBYTE p;

    p = getPixelPoint(x, y);
    if(p) *p = 0;
}

//! 色セット（消しゴム・減算）

void CLayerImg::setPixel_erase(int x,int y,BYTE val)
{
    LPBYTE p;
    int c;

    p = getPixelPoint(x, y);
    if(p)
    {
        c = *p - val;
        if(c < 0) c = 0;

        *p = c;
    }
}

//! 色セット（範囲計算付き）

void CLayerImg::setPixel_calcRect(int x,int y,BYTE val)
{
    LPBYTE p;
    FLAGRECT *prc;

    p = getPixelPoint_create(x, y);
    if(!p) return;

    //

    if(*p == val) return;

    *p = val;

    //範囲

    prc = &m_pinfo->rcfDraw;

    if(prc->flag)
    {
        if(x < prc->x1) prc->x1 = x;
        if(y < prc->y1) prc->y1 = y;
        if(x > prc->x2) prc->x2 = x;
        if(y > prc->y2) prc->y2 = y;
    }
    else
    {
        prc->x1 = prc->x2 = x;
        prc->y1 = prc->y2 = y;
        prc->flag = TRUE;
    }
}

//! 色セット（合成＆範囲計算付き）

void CLayerImg::setPixel_blendCalcRect(int x,int y,BYTE val)
{
    LPBYTE p;
    FLAGRECT *prc;
    int ns,nd;

    if(val == 0) return;

    //ポインタ

    p = getPixelPoint_create(x, y);
    if(!p) return;

    //合成

    ns = val;
    nd = *p;

    nd = (((ns + nd) << 5) - ((ns * nd << 5) / 255) + (1 << 4)) >> 5;

    if(*p == nd) return;

    *p = nd;

    //範囲

    prc = &m_pinfo->rcfDraw;

    if(prc->flag)
    {
        if(x < prc->x1) prc->x1 = x;
        if(y < prc->y1) prc->y1 = y;
        if(x > prc->x2) prc->x2 = x;
        if(y > prc->y2) prc->y2 = y;
    }
    else
    {
        prc->x1 = prc->x2 = x;
        prc->y1 = prc->y2 = y;
        prc->flag = TRUE;
    }
}

//! 色セット（合成&配列拡張あり）

void CLayerImg::setPixel_blendEx(int x,int y,BYTE val)
{
    int tx,ty,n;
    CTileImg **pp;
    LPBYTE pd;

    //タイル位置 （キャンバス範囲内なら配列拡張）

    if(!calcPixelToTile(&tx, &ty, x, y))
    {
        //キャンバス範囲外なら描画しない

        if(x < 0 || y < 0 || x >= m_pinfo->nImgW || y >= m_pinfo->nImgH)
            return;

        //配列リサイズ＆タイル位置再取得

        if(!resizeTile_incImage()) return;

        calcPixelToTile(&tx, &ty, x, y);
    }

    pp = m_ppTile + ty * m_nTileXCnt + tx;

    //タイル確保

    if(!(*pp))
        *pp = allocTileClear();

    //色セット

    pd = (*pp)->getBufPt((x - m_nOffX) & 63, (y - m_nOffY) & 63);

    n = *pd;
    n = (((val + n) << 5) - ((val * n << 5) / 255) + (1 << 4)) >> 5;

    *pd = n;
}

//! 色セット（範囲計算＆配列拡張あり）

void CLayerImg::setPixel_subDraw(int x,int y,BYTE val)
{
    int tx,ty,dstval,srcval;
    CTileImg **pp;
    FLAGRECT *prc;

    //セットする色と元の色比較

    srcval = getPixel(x, y);

    switch(m_pinfo->nColSetFunc)
    {
        case COLSETSUB_SET:
            dstval = val;
            break;
        case COLSETSUB_CLEAR:
            dstval = 0;
            break;
        case COLSETSUB_REV:
            dstval = 255 - srcval;
            break;
    }

    if(dstval == srcval) return;

    //タイル位置 （キャンバス範囲内なら配列拡張）

    if(!calcPixelToTile(&tx, &ty, x, y))
    {
        //キャンバス範囲外なら描画しない

        if(x < 0 || y < 0 || x >= m_pinfo->nImgW || y >= m_pinfo->nImgH)
            return;

        //配列リサイズ＆タイル位置再取得

        if(!resizeTile_incImage()) return;

        calcPixelToTile(&tx, &ty, x, y);
    }

    pp = m_ppTile + ty * m_nTileXCnt + tx;

    //タイル確保

    if(!(*pp))
        *pp = allocTileClear();

    //色セット

    (*pp)->setPixel((x - m_nOffX) & 63, (y - m_nOffY) & 63, dstval);

    //範囲

    prc = &m_pinfo->rcfDraw;

    if(prc->flag)
    {
        if(x < prc->x1) prc->x1 = x;
        if(y < prc->y1) prc->y1 = y;
        if(x > prc->x2) prc->x2 = x;
        if(y > prc->y2) prc->y2 = y;
    }
    else
    {
        prc->x1 = prc->x2 = x;
        prc->y1 = prc->y2 = y;
        prc->flag = TRUE;
    }
}


//=============================
// 色セット（ドット用）
//=============================


//! m_pinfo->funcDrawPixel で点をセット

void CLayerImg::setPixel_drawfunc(int x,int y,BYTE val)
{
    (this->*(m_pinfo->funcDrawPixel))(x, y, val);
}

//! 2pxドット

void CLayerImg::setPixel_dot2px(int x,int y,BYTE val)
{
    setPixelDraw(x, y, val);
    setPixelDraw(x + 1, y, val);
    setPixelDraw(x, y + 1, val);
    setPixelDraw(x + 1, y + 1, val);
}

//! 3pxドット

void CLayerImg::setPixel_dot3px(int x,int y,BYTE val)
{
    setPixelDraw(x, y, val);
    setPixelDraw(x - 1, y, val);
    setPixelDraw(x, y - 1, val);
    setPixelDraw(x + 1, y, val);
    setPixelDraw(x, y + 1, val);
}
