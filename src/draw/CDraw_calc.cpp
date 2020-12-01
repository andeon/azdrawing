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
    CDraw [calc] - 計算
*/

#include <math.h>
#include <stdlib.h>

#include "CDraw.h"

#include "CLayerList.h"


//! 回転・拡大用値計算

void CDraw::calcViewParam()
{
    double r,s;

    r = m_nCanvasRot * M_PI / 18000.0;
    s = m_nCanvasScale * 0.01;

    m_dViewParam[VIEWPARAM_SCALE]    = s;
    m_dViewParam[VIEWPARAM_SCALEDIV] = 1.0 / s;
    m_dViewParam[VIEWPARAM_COSREV]   = ::cos(-r);
    m_dViewParam[VIEWPARAM_SINREV]   = ::sin(-r);
    m_dViewParam[VIEWPARAM_COS]      = ::cos(r);
    m_dViewParam[VIEWPARAM_SIN]      = ::sin(r);
}


//=============================
//座標変換
//=============================


//! ウィンドウ位置 -> イメージ位置 (int)

void CDraw::calcWindowToImage(AXPoint *pDst,double x,double y)
{
    double xx,yy,x1,y1;

    xx = x + m_ptScr.x - m_szCanvas.w / 2;
    yy = y + m_ptScr.y - m_szCanvas.h / 2;

    if(m_nCanvasRot)
    {
        x1 = xx * m_dViewParam[VIEWPARAM_COSREV] - yy * m_dViewParam[VIEWPARAM_SINREV];
        y1 = xx * m_dViewParam[VIEWPARAM_SINREV] + yy * m_dViewParam[VIEWPARAM_COSREV];

        xx = x1, yy = y1;
    }

    if(m_bCanvasHRev) xx = -xx;

    pDst->x = (int)(xx * m_dViewParam[VIEWPARAM_SCALEDIV] + m_ptBaseImg.x);
    pDst->y = (int)(yy * m_dViewParam[VIEWPARAM_SCALEDIV] + m_ptBaseImg.y);
}

//! ウィンドウ位置 -> イメージ位置 (double)

void CDraw::calcWindowToImage(double *pX,double *pY,double x,double y)
{
    double xx,yy,x1,y1;

    xx = x + m_ptScr.x - m_szCanvas.w / 2;
    yy = y + m_ptScr.y - m_szCanvas.h / 2;

    if(m_nCanvasRot)
    {
        x1 = xx * m_dViewParam[VIEWPARAM_COSREV] - yy * m_dViewParam[VIEWPARAM_SINREV];
        y1 = xx * m_dViewParam[VIEWPARAM_SINREV] + yy * m_dViewParam[VIEWPARAM_COSREV];

        xx = x1, yy = y1;
    }

    if(m_bCanvasHRev) xx = -xx;

    *pX = xx * m_dViewParam[VIEWPARAM_SCALEDIV] + m_ptBaseImg.x;
    *pY = yy * m_dViewParam[VIEWPARAM_SCALEDIV] + m_ptBaseImg.y;
}

//! イメージ位置(int) -> ウィンドウ位置

void CDraw::calcImageToWindow(AXPoint *pDst,int x,int y)
{
    int xx,yy;
    double fx,fy;

    fx = (x - m_ptBaseImg.x) * m_dViewParam[VIEWPARAM_SCALE];
    fy = (y - m_ptBaseImg.y) * m_dViewParam[VIEWPARAM_SCALE];

    if(m_bCanvasHRev) fx = -fx;

    if(m_nCanvasRot == 0)
    {
        xx = (int)fx;
        yy = (int)fy;
    }
    else
    {
        xx = (int)(fx * m_dViewParam[VIEWPARAM_COS] - fy * m_dViewParam[VIEWPARAM_SIN]);
        yy = (int)(fx * m_dViewParam[VIEWPARAM_SIN] + fy * m_dViewParam[VIEWPARAM_COS]);
    }

    xx += m_szCanvas.w / 2 - m_ptScr.x;
    yy += m_szCanvas.h / 2 - m_ptScr.y;

    pDst->x = xx;
    pDst->y = yy;
}

//! イメージ範囲 -> キャンバスウィンドウ範囲(描画時の更新範囲)
/*!
    @return 範囲外でFALSE
*/

BOOL CDraw::calcImageToWindowRect(AXRectSize *pdst,const AXRectSize &src)
{
    AXPoint pt[4];
    int x1,y1,x2,y2,i;

    //※範囲は大きめに見積もる

    x1 = src.x - 1, y1 = src.y - 1;
    x2 = src.x + src.w, y2 = src.y + src.h;

    //x,y 最小・最大

    if(m_nCanvasRot == 0)
    {
        calcImageToWindow(pt    , x1, y1);
        calcImageToWindow(pt + 1, x2, y2);

        if(pt[0].x < pt[1].x)
            x1 = pt[0].x, x2 = pt[1].x;
        else
            x1 = pt[1].x, x2 = pt[0].x;

        if(pt[0].y < pt[1].y)
            y1 = pt[0].y, y2 = pt[1].y;
        else
            y1 = pt[1].y, y2 = pt[0].y;
    }
    else
    {
        calcImageToWindow(pt    , x1, y1);
        calcImageToWindow(pt + 1, x2, y1);
        calcImageToWindow(pt + 2, x1, y2);
        calcImageToWindow(pt + 3, x2, y2);

        x1 = x2 = pt[0].x;
        y1 = y2 = pt[0].y;

        for(i = 1; i < 4; i++)
        {
            if(x1 > pt[i].x) x1 = pt[i].x;
            if(y1 > pt[i].y) y1 = pt[i].y;
            if(x2 < pt[i].x) x2 = pt[i].x;
            if(y2 < pt[i].y) y2 = pt[i].y;
        }
    }

    //念のため範囲拡張

    x1--, y1--;
    x2++, y2++;

    //範囲内判定

    if(x2 < 0 || y2 < 0) return FALSE;
    if(x1 >= m_szCanvas.w || y1 >= m_szCanvas.h) return FALSE;

    //調整

    if(x1 < 0) x1 = 0;
    if(y1 < 0) y1 = 0;
    if(x2 >= m_szCanvas.w) x2 = m_szCanvas.w - 1;
    if(y2 >= m_szCanvas.h) y2 = m_szCanvas.h - 1;

    //

    pdst->set(x1, y1, x2 - x1 + 1, y2 - y1 + 1);

    return TRUE;
}


//=============================
// AXRect/AXRectSize/FLAGRECT
//=============================


//! pdst に psrc の範囲追加（x == -1 で範囲なし）
/*!
    @return 範囲があるか（両方共範囲なしで FALSE）
*/

BOOL CDraw::calcUnionRectSize(AXRectSize *pdst,const AXRectSize &src)
{
    //両方とも範囲なし

    if(pdst->x < 0 && src.x < 0) return FALSE;

    //srcが範囲なし（そのまま）

    if(pdst->x >= 0 && src.x < 0) return TRUE;

    //pdstが範囲なし（srcをコピー）

    if(pdst->x < 0 && src.x >= 0)
    {
        *pdst = src;
        return TRUE;
    }

    //両方範囲あり

    pdst->combine(src);

    return TRUE;
}

//! FLAGRECT に FLAGRECT 範囲を追加（自由線時などの全体更新範囲追加）

void CDraw::calcUnionFlagRect(FLAGRECT *pd,const FLAGRECT &src)
{
    if(!pd->flag)
    {
        //セット先に範囲がなければそのままコピー

        *pd = src;
    }
    else
    {
        if(!src.flag) return;

        if(src.x1 < pd->x1) pd->x1 = src.x1;
        if(src.y1 < pd->y1) pd->y1 = src.y1;
        if(src.x2 > pd->x2) pd->x2 = src.x2;
        if(src.y2 > pd->y2) pd->y2 = src.y2;
    }
}

//! FLAGRECT 範囲(イメージ位置)をキャンバスイメージの範囲内に調整して AXRectSize へ
/*!
    @return FALSEで範囲外
*/

BOOL CDraw::calcImgRectInCanvas(AXRectSize *prcs,const FLAGRECT &rcf)
{
    int x1,x2,y1,y2;

    x1 = rcf.x1, y1 = rcf.y1;
    x2 = rcf.x2, y2 = rcf.y2;

    //範囲外

    if(x1 >= m_nImgW || y1 >= m_nImgH) return FALSE;
    if(x2 < 0 || y2 < 0) return FALSE;

    //調整

    if(x1 < 0) x1 = 0;
    if(y1 < 0) y1 = 0;
    if(x2 >= m_nImgW) x2 = m_nImgW - 1;
    if(y2 >= m_nImgH) y2 = m_nImgH - 1;

    //セット

    prcs->x	= x1;
    prcs->y = y1;
    prcs->w = x2 - x1 + 1;
    prcs->h = y2 - y1 + 1;

    return TRUE;
}

//! FLAGRECTの範囲に指定値加算

void CDraw::calcAddFlagRect(FLAGRECT *pd,int addx,int addy)
{
    pd->x1 += addx;
    pd->x2 += addx;
    pd->y1 += addy;
    pd->y2 += addy;
}

//! AXRect の範囲をキャンバスウィンドウ範囲内に調整
/*!
    @return FALSEで範囲外
*/

BOOL CDraw::calcRectInCanvasWin(AXRectSize *prcs,const AXRect &rcSrc)
{
    AXRect rc = rcSrc;

    //範囲外

    if(rc.left >= m_szCanvas.w || rc.top >= m_szCanvas.h) return FALSE;
    if(rc.right < 0 || rc.bottom < 0) return FALSE;

    //調整

    if(rc.left < 0) rc.left = 0;
    if(rc.top  < 0) rc.top = 0;
    if(rc.right  >= m_szCanvas.w) rc.right  = m_szCanvas.w - 1;
    if(rc.bottom >= m_szCanvas.w) rc.bottom = m_szCanvas.h - 1;

    //セット

    prcs->set(rc);

    return TRUE;
}


//=============================
//ツール・描画関連
//=============================


//! イメージ移動・転送先位置計算
/*!
    @param pDst 転送先位置
    @param src  総移動数(px)
*/

void CDraw::calcMoveDstPos(AXPoint *pDst,const AXPoint &src)
{
    pDst->x = (int)((src.x * m_dViewParam[VIEWPARAM_COSREV] - src.y * m_dViewParam[VIEWPARAM_SINREV]) * m_dViewParam[VIEWPARAM_SCALEDIV]);
    pDst->y = (int)((src.x * m_dViewParam[VIEWPARAM_SINREV] + src.y * m_dViewParam[VIEWPARAM_COSREV]) * m_dViewParam[VIEWPARAM_SCALEDIV]);

    if(m_bCanvasHRev) pDst->x = -(pDst->x);
}

//! [直線/連続直線/集中線] 描画位置取得
/*!
    ※m_ptTmp[0],[1] にウィンドウ位置

    @param prc  描画イメージ位置(double)
*/

void CDraw::calcDrawLineRect(DRECT *prc)
{
    calcWindowToImage(&prc->x1, &prc->y1, m_ptTmp[0].x, m_ptTmp[0].y);
    calcWindowToImage(&prc->x2, &prc->y2, m_ptTmp[1].x, m_ptTmp[1].y);
}

//! 四角枠描画の位置（double[4][2]）取得
//※m_ptTmp[0],[1]にウィンドウ位置

void CDraw::calcDrawBoxPoint(double *pDst)
{
    calcWindowToImage(pDst    , pDst + 1, m_ptTmp[0].x, m_ptTmp[0].y);
    calcWindowToImage(pDst + 2, pDst + 3, m_ptTmp[1].x, m_ptTmp[0].y);
    calcWindowToImage(pDst + 4, pDst + 5, m_ptTmp[1].x, m_ptTmp[1].y);
    calcWindowToImage(pDst + 6, pDst + 7, m_ptTmp[0].x, m_ptTmp[1].y);
}

//! 四角枠描画の4点の位置取得(int)

void CDraw::calcDrawBoxPointInt(AXPoint *pDst)
{
    calcWindowToImage(pDst    , m_ptTmp[0].x, m_ptTmp[0].y);
    calcWindowToImage(pDst + 1, m_ptTmp[1].x, m_ptTmp[0].y);
    calcWindowToImage(pDst + 2, m_ptTmp[1].x, m_ptTmp[1].y);
    calcWindowToImage(pDst + 3, m_ptTmp[0].x, m_ptTmp[1].y);
}

//! 直線を45度単位に調整
/*!
    @param pdst  調整したい位置
    @param st    始点
*/

void CDraw::calcLine45(AXPoint *pdst,const AXPoint &st)
{
    int dx,dy,adx,ady,n;

    dx = pdst->x - st.x;
    dy = pdst->y - st.y;

    adx = ::abs(dx);
    ady = ::abs(dy);

    n = (adx > ady)? adx: ady;

    if(::abs(adx - ady) < n / 2)
    {
        //45度線

        if(adx < ady)
            pdst->y = st.y + ((dy < 0)? -adx: adx);
        else
            pdst->x = st.x + ((dx < 0)? -ady: ady);
    }
    else if(adx > ady)
        //水平
        pdst->y = st.y;
    else
        //垂直
        pdst->x = st.x;
}

//! 選択範囲コピー時のイメージpx範囲取得

void CDraw::calcSelCopyRect(AXRect *prc)
{
    //タイルの範囲

    m_pCurLayer->m_img.calcTileRectToPixel(prc);

    //選択範囲の分縮める

    if(prc->left   < m_rcfSel.x1) prc->left   = m_rcfSel.x1;
    if(prc->top    < m_rcfSel.y1) prc->top    = m_rcfSel.y1;
    if(prc->right  > m_rcfSel.x2) prc->right  = m_rcfSel.x2;
    if(prc->bottom > m_rcfSel.y2) prc->bottom = m_rcfSel.y2;
}


//=============================
//そのほか
//=============================


//! キャンバスウィンドウのスクロールバーの最大値取得
/*!
    四隅をイメージ座標 -> ウィンドウ座標変換し、一番大きい半径の値＋ウィンドウサイズ
*/

void CDraw::calcCanvasScrollMax(AXSize *psize)
{
    AXPoint pt[4];
    int i,n,max = 1;
    double x,y;

    //イメージ座標 -> ウィンドウ座標

    pt[0].x = pt[0].y = 0;
    pt[1].x = m_nImgW, pt[1].y = 0;
    pt[2].x = 0,       pt[2].y = m_nImgH;
    pt[3].x = m_nImgW, pt[3].y = m_nImgH;

    for(i = 0; i < 4; i++)
    {
        calcImageToWindow(pt + i, pt[i].x, pt[i].y);

        x = pt[i].x - m_szCanvas.w / 2;
        y = pt[i].y - m_szCanvas.h / 2;

        n = (int)(::sqrt(x * x + y * y) + 0.5);

        if(n > max) max = n;
    }

    //

    psize->w = max * 2 + m_szCanvas.w;
    psize->h = max * 2 + m_szCanvas.h;
}

//! イメージがキャンバスに収まるように表示倍率計算

int CDraw::calcCanvasScaleFromSize()
{
    AXRectSize rcs;
    int w,h,s;

    if(m_szCanvas.w < 20 || m_szCanvas.h < 20)
        w = h = 20;
    else
    {
        w = m_szCanvas.w - 20;
        h = m_szCanvas.h - 20;
    }

    rcs.set(0, 0, m_nImgW, m_nImgH);
    rcs.inBoxKeepAspect(w, h, TRUE);

    //

    s = rcs.w * 100 / m_nImgW;

    if(s < SCALE_MIN) s = SCALE_MIN;
    else if(s > SCALE_MAX) s = SCALE_MAX;

    return s;
}
