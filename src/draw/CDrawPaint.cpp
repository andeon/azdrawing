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

#include "CDrawPaint.h"

#include "CTileImg.h"
#include "defStruct.h"


/*!
    @class CDrawPaint
    @brief 塗りつぶし描画クラス
*/
/*
    [m_imgRef]
        判定元レイヤをすべて合成したイメージ。
        塗りつぶし途中の作業用としても使う。
        最初は空の状態だが、塗りつぶしでの走査時にタイルが確保されていなければ、
        その都度合成したイメージを作成していく。

    [m_imgDraw]
        塗りつぶし部分を描画していくイメージ。
        直接描画先レイヤに描画する場合は不要。
*/


CDrawPaint::CDrawPaint()
{
    m_pBuf = NULL;
}

CDrawPaint::~CDrawPaint()
{
    free();
}

//! 解放

void CDrawPaint::free()
{
    AXFree((void **)&m_pBuf);

    m_imgRef.free();
    m_imgDraw.free();
}

//! 実行前の初期化
/*!
    @return エラー値
*/

int CDrawPaint::init(CLayerImg *pDst,CLayerImg *pRefSt,const AXPoint &ptStart,
                          int nVal,int nCompVal,int type)
{
    m_pimgDst   = pDst;
    m_pimgRefSt = pRefSt;
    m_ptStart   = ptStart;

    m_nType     = type;
    m_nVal      = nVal;
    m_nCompVal  = nCompVal;

    //判定元と描画のpx範囲

    _getAreaRect();

    //開始の点が範囲内か

    if(m_ptStart.x < m_rcArea.left  || m_ptStart.y < m_rcArea.top) return RET_NODRAW;
    if(m_ptStart.x > m_rcArea.right || m_ptStart.y > m_rcArea.bottom) return RET_NODRAW;

    //開始の点が描画に値するか

    m_nStartVal = _getRefPixel(m_ptStart.x, m_ptStart.y);

    switch(type)
    {
        //アンチエイリアス自動
        case TYPE_ANTIAUTO:
            if(m_nStartVal >= m_nVal) return RET_NODRAW;
            break;
        //指定濃度未満
        case TYPE_LESS:
            if(m_nStartVal >= m_nCompVal) return RET_NODRAW;
            break;
        //不透明消し
        case TYPE_ERASE:
            if(m_nStartVal == 0) return RET_NODRAW;
            break;
    }

    //バッファ確保

    m_pBuf = (PAINTBUF *)AXMalloc(sizeof(PAINTBUF) * BUFSIZE);
    if(!m_pBuf) return RET_ERR_MEMORY;

    //作業用イメージタイル配列

    if(!m_imgRef.allocTileArrayFromPx(m_rcArea)) return RET_ERR_MEMORY;

    if(type != TYPE_ERASE)
    {
        if(!m_imgDraw.allocTileArrayFromPx(m_rcDrawArea)) return RET_ERR_MEMORY;
    }

    return RET_SUCCESS;
}

//! 閉領域用の判定元イメージ作成
/*!
    pimgArea の処理範囲を1px拡張したサイズで作成し、判定元レイヤをすべて合成。@n
    それを塗りつぶし時の判定元イメージとして使う。

    @param pimgDst  作成先
    @param pimgArea 処理範囲のイメージ。処理する部分には点がある。
    @param prcf     pimgAreaの最小範囲
    @param pRefSt   塗りつぶし判定元最初のイメージ
*/

int CDrawPaint::setCloseRefImg(CLayerImg *pimgDst,CLayerImg *pimgArea,const FLAGRECT &rcf,CLayerImg *pRefSt)
{
    AXRect rc;
    CTileImg **ppTile;
    int tx,ty,xcnt,ycnt,xtop,ytop,xx,yy;

    //判定元用イメージ作成（1px拡張）

    rc.left     = rcf.x1 - 1;
    rc.top      = rcf.y1 - 1;
    rc.right    = rcf.x2 + 1;
    rc.bottom   = rcf.y2 + 1;

    if(!pimgDst->allocTileArrayFromPx(rc)) return RET_ERR_MEMORY;

    pimgDst->setLink(NULL);

    //pimgArea で点がある部分を合成

    ppTile  = pimgDst->getTileBuf();
    xcnt    = pimgDst->getTileXCnt();
    ycnt    = pimgDst->getTileYCnt();

    pimgDst->calcTileToPixel(&xtop, &ytop, 0, 0);

    for(ty = 0, yy = ytop; ty < ycnt; ty++, yy += 64)
    {
        for(tx = 0, xx = xtop; tx < xcnt; tx++, ppTile++, xx += 64)
        {
            //タイル確保

            *ppTile = pimgDst->allocTileClear();
            if(!(*ppTile)) return RET_ERR_MEMORY;

            //合成

            _close_blendRefTile(*ppTile, xx, yy, pimgArea, pRefSt);
        }
    }

    //拡張1pxの分を透明に（タイルを確保する）

    for(tx = rc.left; tx <= rc.right; tx++)
    {
        pimgDst->setPixel_create(tx, rc.top, 0);
        pimgDst->setPixel_create(tx, rc.bottom, 0);
    }

    for(ty = rc.top; ty <= rc.bottom; ty++)
    {
        pimgDst->setPixel_create(rc.left, ty, 0);
        pimgDst->setPixel_create(rc.right, ty, 0);
    }

    return RET_SUCCESS;
}

//! 実行

void CDrawPaint::run()
{
    switch(m_nType)
    {
        case TYPE_ANTIAUTO:
            _runAuto();
            _drawDst();
            break;
        case TYPE_LESS:
            _runLess();
            _drawDst();
            break;
        case TYPE_ERASE:
            _runErase();
            break;
        case TYPE_CLOSE:
            _runAuto();
            _drawClose();
            break;
    }

    free();
}


//===========================
//最終描画処理
//===========================


//! 描画用イメージから描画先に描画

void CDrawPaint::_drawDst()
{
    int xtop,ytop,xcnt,ycnt,tx,ty,x,y,xx,yy;
    CTileImg **ppSrc;
    LPBYTE ps;

    m_imgDraw.calcTileToPixel(&xtop, &ytop, 0, 0);

    ppSrc = m_imgDraw.getTileBuf();
    xcnt  = m_imgDraw.getTileXCnt();
    ycnt  = m_imgDraw.getTileYCnt();

    for(ty = 0, yy = ytop; ty < ycnt; ty++, yy += 64)
    {
        for(tx = 0, xx = xtop; tx < xcnt; tx++, xx += 64, ppSrc++)
        {
            if(*ppSrc)
            {
                ps = (*ppSrc)->getBuf();

                for(y = 0; y < 64; y++)
                {
                    for(x = 0; x < 64; x++, ps++)
                    {
                        if(*ps)
                            m_pimgDst->setPixelDraw(xx + x, yy + y, m_nVal);
                    }
                }
            }
        }
    }
}

//! 閉領域描画
/*!
    imgDraw で塗られていない部分を m_pimgDst に塗る
*/

void CDrawPaint::_drawClose()
{
    int xtop,ytop,xcnt,ycnt,tx,ty,x,y,xx,yy;
    CTileImg **ppSrc;
    LPBYTE ps;

    m_imgDraw.calcTileToPixel(&xtop, &ytop, 0, 0);

    ppSrc = m_imgDraw.getTileBuf();
    xcnt  = m_imgDraw.getTileXCnt();
    ycnt  = m_imgDraw.getTileYCnt();

    for(ty = 0, yy = ytop; ty < ycnt; ty++, yy += 64)
    {
        for(tx = 0, xx = xtop; tx < xcnt; tx++, xx += 64, ppSrc++)
        {
            if(*ppSrc)
            {
                //-------- タイルが確保されている

                ps = (*ppSrc)->getBuf();

                for(y = 0; y < 64; y++)
                {
                    for(x = 0; x < 64; x++, ps++)
                    {
                        if(*ps == 0)
                            m_pimgDst->setPixelDraw(xx + x, yy + y, m_nVal);
                    }
                }
            }
            else
            {
                //---------- タイルが空（64x64すべて塗る）

                for(y = 0; y < 64; y++)
                    for(x = 0; x < 64; x++)
                        m_pimgDst->setPixelDraw(xx + x, yy + y, m_nVal);
            }

        }
    }
}


//==========================
//途中描画
//==========================


//! 判定元用イメージに水平線描画
/*!
    タイルはすでに確保されているとみなす。

    @return FALSE でタイルが確保されていない
*/

BOOL CDrawPaint::_drawRefLineH(int x1,int x2,int y,BYTE val)
{
    int tx,ty,ix,subx,suby;
    CTileImg **pp;
    LPBYTE pd;

    m_imgRef.calcPixelToTile(&tx, &ty, x1, y);

    pp = m_imgRef.getTilePt(tx, ty);
    if(!(*pp)) return FALSE;

    //

    subx = (x1 - m_imgRef.getOffX()) & 63;
    suby = (y  - m_imgRef.getOffY()) & 63;

    pd = (*pp)->getBufPt(subx, suby);

    //

    for(ix = x1; ix <= x2; ix++)
    {
        *pd = val;

        subx = (subx + 1) & 63;

        if(subx)
            pd++;
        else
        {
            //次のタイルへ

            if(ix == x2) break;

            pp++;
            if(!(*pp)) return FALSE;

            pd = (*pp)->getBufPt(0, suby);
        }
    }

    return TRUE;
}

//! 判定元用に垂直線描画

BOOL CDrawPaint::_drawRefLineV(int y1,int y2,int x,BYTE val)
{
    int tx,ty,iy,subx,suby;
    CTileImg **pp;
    LPBYTE pd;

    m_imgRef.calcPixelToTile(&tx, &ty, x, y1);

    pp = m_imgRef.getTilePt(tx, ty);
    if(!(*pp)) return FALSE;

    subx = (x  - m_imgRef.getOffX()) & 63;
    suby = (y1 - m_imgRef.getOffY()) & 63;

    pd   = (*pp)->getBufPt(subx, suby);

    //

    for(iy = y1; iy <= y2; iy++)
    {
        *pd = val;

        suby = (suby + 1) & 63;

        if(suby)
            pd += 64;
        else
        {
            //次のタイルへ

            if(iy == y2) break;

            pp += m_imgRef.getTileXCnt();
            if(!(*pp)) return FALSE;

            pd = (*pp)->getBufPt(subx, 0);
        }
    }

    return TRUE;
}

//! 描画用に水平線描画

void CDrawPaint::_drawDrawLineH(int x1,int x2,int y,BYTE val)
{
    int tx,ty,ix,subx,suby;
    CTileImg **pp;
    LPBYTE pd;

    //描画範囲外

    if(y < m_rcDrawArea.top || y > m_rcDrawArea.bottom) return;
    if(x1 > m_rcDrawArea.right || x2 < m_rcDrawArea.left) return;

    //描画範囲調整

    if(x1 < m_rcDrawArea.left ) x1 = m_rcDrawArea.left;
    if(x2 > m_rcDrawArea.right) x2 = m_rcDrawArea.right;

    //

    m_imgDraw.calcPixelToTile(&tx, &ty, x1, y);

    pp   = m_imgDraw.getTilePt(tx, ty);

    subx = (x1 - m_imgDraw.getOffX()) & 63;
    suby = (y  - m_imgDraw.getOffY()) & 63;

    if(!(*pp))
    {
        *pp = m_imgDraw.allocTileClear();
        if(!(*pp)) return;
    }

    pd = (*pp)->getBufPt(subx, suby);

    //

    for(ix = x1; ix <= x2; ix++)
    {
        *pd = val;

        subx = (subx + 1) & 63;

        if(subx)
            pd++;
        else
        {
            //次のタイルへ

            if(ix == x2) break;

            pp++;

            if(!(*pp))
            {
                *pp = m_imgDraw.allocTileClear();
                if(!(*pp)) return;
            }

            pd = (*pp)->getBufPt(0, suby);
        }
    }
}

//! 描画用に垂直線描画

void CDrawPaint::_drawDrawLineV(int y1,int y2,int x,BYTE val)
{
    int tx,ty,iy,subx,suby;
    CTileImg **pp;
    LPBYTE pd;

    //描画範囲外

    if(x < m_rcDrawArea.left || x > m_rcDrawArea.right) return;
    if(y2 < m_rcDrawArea.top || y1 > m_rcDrawArea.bottom) return;

    //描画範囲調整

    if(y1 < m_rcDrawArea.top   ) y1 = m_rcDrawArea.top;
    if(y2 > m_rcDrawArea.bottom) y2 = m_rcDrawArea.bottom;

    //

    m_imgDraw.calcPixelToTile(&tx, &ty, x, y1);

    pp   = m_imgDraw.getTilePt(tx, ty);

    subx = (x  - m_imgDraw.getOffX()) & 63;
    suby = (y1 - m_imgDraw.getOffY()) & 63;

    if(!(*pp))
    {
        *pp = m_imgDraw.allocTileClear();
        if(!(*pp)) return;
    }

    pd = (*pp)->getBufPt(subx, suby);

    //

    for(iy = y1; iy <= y2; iy++)
    {
        *pd = val;

        suby = (suby + 1) & 63;

        if(suby)
            pd += 64;
        else
        {
            //次のタイルへ

            if(iy == y2) break;

            pp += m_imgDraw.getTileXCnt();

            if(!(*pp))
            {
                *pp = m_imgDraw.allocTileClear();
                if(!(*pp)) return;
            }

            pd = (*pp)->getBufPt(subx, 0);
        }
    }
}


//===========================
//サブ
//===========================


//! 閉領域、指定タイルに判定元イメージをすべて合成（pArea で透明な部分は除く）

void CDrawPaint::_close_blendRefTile(CTileImg *ptile,int xtop,int ytop,CLayerImg *pArea,CLayerImg *pRefSt)
{
    CLayerImg *pimg;
    LPBYTE pd;
    int x,y,xx,yy,ns,nd;

    pd = ptile->getBuf();

    for(y = 0, yy = ytop; y < 64; y++, yy++)
    {
        for(x = 0, xx = xtop; x < 64; x++, xx++, pd++)
        {
            //マスク

            if(pArea->getPixel(xx, yy) == 0) continue;

            //最初のレイヤはそのまま

            nd = pRefSt->getPixel(xx, yy);

            //以降、合成

            for(pimg = pRefSt->getLink(); pimg; pimg = pimg->getLink())
            {
                ns = pimg->getPixel(xx, yy);
                nd = (((nd + ns) << 4) - (nd * ns << 4) / 255 + 8) >> 4;
            }

            //セット

            *pd = nd;
        }
    }
}

//! 判定の範囲と描画範囲セット

void CDrawPaint::_getAreaRect()
{
    int w,h;

    switch(m_nType)
    {
        //自動/指定濃度未満
        //（判定はキャンバス内）
        /*
            キャンバスの右/下端に余白が出来ている場合や、位置移動で少し移動している場合などは、
            端を意図的に塗りつぶしておかないと
            余白に繋がっている部分が塗りつぶされてしまうため
        */
        case TYPE_ANTIAUTO:
        case TYPE_LESS:
            CLayerImg::getCanvasImgSize(&w, &h);

            m_rcArea.set(0, 0, w - 1, h - 1);
            m_rcDrawArea = m_rcArea;
            break;
        //消しゴム
        case TYPE_ERASE:
            _getAllRefRect(&m_rcArea);
            m_pimgDst->getEnableDrawRectPixel(&m_rcDrawArea);
            break;
        //閉領域
        //（判定元の範囲で）
        //※判定元はキャンバスの範囲
        case TYPE_CLOSE:
            m_pimgRefSt->calcTileRectToPixel(&m_rcArea);
            m_rcDrawArea = m_rcArea;
            break;
    }
}

//! 判定元すべてのpx範囲＋キャンバス範囲取得

void CDrawPaint::_getAllRefRect(AXRect *prc)
{
    AXRect rc,rc2;
    CLayerImg *p;

    //最初の範囲(＋キャンバス範囲)

    m_pimgRefSt->getEnableDrawRectPixel(&rc);

    //以降

    for(p = m_pimgRefSt->getLink(); p; p = p->getLink())
    {
        p->calcTileRectToPixel(&rc2);

        rc.combine(rc2);
    }

    *prc = rc;
}

//! 判定元をすべて合成した点を取得

int CDrawPaint::_getRefPixel(int x,int y)
{
    CLayerImg *p;
    int val,ns;

    //最初の点はそのまま

    val = m_pimgRefSt->getPixel(x, y);

    //以降、合成

    for(p = m_pimgRefSt->getLink(); p; p = p->getLink())
    {
        ns = p->getPixel(x, y);

        val = (((val + ns) << 4) - (val * ns << 4) / 255 + 8) >> 4;
    }

    return val;
}

//! 判定元のタイルセット

BOOL CDrawPaint::_setRefTile(int tx,int ty,CTileImg **ppDstTile)
{
    CTileImg **pptile;
    CLayerImg *pimg;
    int x,y,xtop,ytop,ns,nd;
    LPBYTE pd;

    //タイル範囲外

    if(tx < 0 || ty < 0 ||
        tx >= m_imgRef.getTileXCnt() || ty >= m_imgRef.getTileYCnt())
    {
        *ppDstTile = NULL;
        return FALSE;
    }

    //

    pptile = m_imgRef.getTilePt(tx, ty);

    //確保済み

    if(*pptile)
    {
        *ppDstTile = *pptile;
        return TRUE;
    }

    //タイル確保

    *pptile = m_imgRef.allocTile();
    if(!(*pptile))
    {
        *ppDstTile = NULL;
        return FALSE;
    }

    //タイルの左上px位置

    m_imgRef.calcTileToPixel(&xtop, &ytop, tx, ty);

    //最初のレイヤはそのまま

    pd = (*pptile)->getBuf();

    for(y = 0; y < 64; y++)
        for(x = 0; x < 64; x++, pd++)
            *pd = m_pimgRefSt->getPixel(xtop + x, ytop + y);

    //以降、合成

    for(pimg = m_pimgRefSt->getLink(); pimg; pimg = pimg->getLink())
    {
        pd = (*pptile)->getBuf();

        for(y = 0; y < 64; y++)
        {
            for(x = 0; x < 64; x++, pd++)
            {
                nd = *pd;
                ns = pimg->getPixel(xtop + x, ytop + y);

                *pd = (((nd + ns) << 4) - (nd * ns << 4) / 255 + 8) >> 4;
            }
        }
    }

    //タイルポインタ

    *ppDstTile = *pptile;

    return TRUE;
}

//! 判定元カレント位置セット

void CDrawPaint::_setCurrent(int px,int py)
{
    m_nCurPX    = px;
    m_nCurPY    = py;

    m_nCurSubX  = (px - m_imgRef.getOffX()) & 63;
    m_nCurSubY  = (py - m_imgRef.getOffY()) & 63;

    m_imgRef.calcPixelToTile(&m_nCurTileX, &m_nCurTileY, px, py);

    //タイル確保

    _setRefTile(m_nCurTileX, m_nCurTileY, &m_pCurTile);
}

//! カレント位置の色取得

int CDrawPaint::_getCurPixel()
{
    if(!m_pCurTile)
        return _getRefPixel(m_nCurPX, m_nCurPY);
    else
        return m_pCurTile->getPixel(m_nCurSubX, m_nCurSubY);
}

//! カレント位置に色セット

BOOL CDrawPaint::_setCurPixel(BYTE val)
{
    if(!m_pCurTile) return FALSE;

    *(m_pCurTile->getBufPt(m_nCurSubX, m_nCurSubY)) = val;

    return TRUE;
}

//! カレント位置を右に+1px

void CDrawPaint::_movCurRight()
{
    if(m_nCurSubX == 63)
    {
        m_nCurTileX++;

        _setRefTile(m_nCurTileX, m_nCurTileY, &m_pCurTile);
    }

    m_nCurPX++;
    m_nCurSubX = (m_nCurSubX + 1) & 63;
}

//! カレント位置を左に-1px

void CDrawPaint::_movCurLeft()
{
    if(m_nCurSubX == 0)
    {
        m_nCurTileX--;

        _setRefTile(m_nCurTileX, m_nCurTileY, &m_pCurTile);
    }

    m_nCurPX--;
    m_nCurSubX = (m_nCurSubX - 1) & 63;
}

//! カレント位置を下に+1px

void CDrawPaint::_movCurDown()
{
    if(m_nCurSubY == 63)
    {
        m_nCurTileY++;

        _setRefTile(m_nCurTileX, m_nCurTileY, &m_pCurTile);
    }

    m_nCurPY++;
    m_nCurSubY = (m_nCurSubY + 1) & 63;
}

//! カレント位置を上に-1px

void CDrawPaint::_movCurUp()
{
    if(m_nCurSubY == 0)
    {
        m_nCurTileY--;

        _setRefTile(m_nCurTileX, m_nCurTileY, &m_pCurTile);
    }

    m_nCurPY--;
    m_nCurSubY = (m_nCurSubY - 1) & 63;
}
