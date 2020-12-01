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
    CDraw [draw_brushdot] - ブラシ/ドットペン描画
*/

#include <stdlib.h>

#include "CDraw.h"

#include "CCanvasWin.h"
#include "CLayerList.h"
#include "CSplinePos.h"
#include "CPosBuf.h"
#include "CImage1.h"



//===========================
//ブラシ描画・自由線
//===========================
/*
    m_optToolTmp : [0bit]筆圧最大
*/


//! 押し時
/*!
    @param nBrushNo -1で選択ブラシ、0〜で登録ブラシ
*/

void CDraw::onDown_brush_free(int nBrushNo,BOOL bPressMax)
{
    m_nNowCtlNo = NOWCTL_BRUSH_FREE;

    //描画準備

    m_optToolTmp = (bPressMax)? 1: 0;

    setBeforeDraw(TOOL_BRUSH, nBrushNo, TRUE);

    //UNDO用準備

    beginDrawUndo();
}

//! 移動時

void CDraw::onMove_brush_free()
{
    DRAWPOINT pt;
    CPosBuf::POSDAT *p;

    //位置

    getDrawPoint(&pt, TRUE);

    p = m_pPosBuf->addPos(pt.x, pt.y, pt.press, m_datDraw.bparam.nHoseiType, m_datDraw.bparam.nHoseiStr);

    //描画

    m_datDraw.rcfDraw.flag = FALSE;

    m_pCurLayer->m_img.drawBrush_free(p->x, p->y, p->press);

    commonFreeDraw();
}

//! 離し時

void CDraw::onUp_brush_free()
{
    DRAWPOINT pt;
    CPosBuf::POSDAT *p;

    //位置

    getDrawPoint(&pt, TRUE);

    p = m_pPosBuf->addPos(pt.x, pt.y, 0, m_datDraw.bparam.nHoseiType, m_datDraw.bparam.nHoseiStr);

    //描画

    m_datDraw.rcfDraw.flag = FALSE;

    m_pCurLayer->m_img.drawBrush_free(p->x, p->y, p->press);

    commonFreeDraw();

    //

    commonDrawEnd();
}


//=============================
// ドットペン・自由線
//=============================
/*
    +Shift で1px消しゴム

    m_nTmp[0]  : 細線か (setBeforeDraw でセット)
    m_ptTmp[0] : 前の位置
    m_ptTmp[1] : 2つ前の位置
*/


//! 押し
/*!
    @param bErase 1px消しゴムか
*/

BOOL CDraw::onDown_dot_free(BOOL bErase)
{
    AXPoint pt;

    m_nNowCtlNo = NOWCTL_DOT_FREE;

    //描画位置

    getDrawPointInt(&pt, FALSE);

    //作業用

    setBeforeDraw(TOOL_DOT, -1, TRUE);

    m_ptTmp[0] = m_ptTmp[1] = pt;

    //1px消しゴム（細線時も通常の線で）

    if(bErase)
    {
        m_datDraw.funcDrawPixel = &CLayerImg::setPixelDraw;
        m_datDraw.nColSetFunc   = CLayerImg::COLSETF_ERASE;
        m_nTmp[0] = FALSE;
    }

    //UNDO用準備

    beginDrawUndo();

    //点を打つ（細線時も）

    getCurLayerImg().setPixel_drawfunc(pt.x, pt.y, 255);

    //更新

    commonFreeDraw();

    return TRUE;
}

//! 移動時

void CDraw::onMove_dot_free()
{
    AXPoint pt;
    BOOL bDraw = TRUE;

    getDrawPointInt(&pt, TRUE);

    //前回と同じ位置なら描画しない

    if(pt.x == m_ptTmp[0].x && pt.y == m_ptTmp[0].y) return;

    //細線時、2つ前の位置と斜めの場合は描画しない

    if(m_nTmp[0])
    {
        if(::abs(pt.x - m_ptTmp[1].x) == 1 && ::abs(pt.y - m_ptTmp[1].y) == 1 &&
           (pt.x == m_ptTmp[0].x || pt.y == m_ptTmp[0].y))
            bDraw = FALSE;
    }

    //描画

    if(bDraw)
    {
        m_datDraw.rcfDraw.flag = FALSE;

        //描画

        if(m_nTmp[0])
            getCurLayerImg().drawLineF(m_ptTmp[0].x, m_ptTmp[0].y, pt.x, pt.y, 255);
        else
            getCurLayerImg().drawLineB(m_ptTmp[0].x, m_ptTmp[0].y, pt.x, pt.y, 255, TRUE);

        //更新

        commonFreeDraw();
    }

    //前の位置

    if(bDraw) m_ptTmp[1] = m_ptTmp[0];
    m_ptTmp[0] = pt;
}

//! 離し時

void CDraw::onUp_dot_free()
{
    commonDrawEnd();
}


//=============================
//描画処理
//=============================


//! 直線描画

void CDraw::draw_line()
{
    DRECT rc;

    setBeforeDraw(m_nDrawToolNo);

    //描画位置取得

    calcDrawLineRect(&rc);

    //描画

    setCursorWait();
    beginDrawUndo();

    if(m_nDrawToolNo == TOOL_BRUSH)
        m_pCurLayer->m_img.drawBrush_lineHeadTail(rc.x1, rc.y1, rc.x2, rc.y2, m_wHeadTailLine);
    else
        m_pCurLayer->m_img.drawLineB((int)rc.x1, (int)rc.y1, (int)rc.x2, (int)rc.y2, 255, FALSE);

    commonAfterDraw();
    restoreCursor();
}

//! 四角枠描画

void CDraw::draw_box()
{
    double pt[8];

    setBeforeDraw(m_nDrawToolNo);

    //描画位置

    calcDrawBoxPoint(pt);

    //描画

    setCursorWait();
    beginDrawUndo();

    if(m_nDrawToolNo == TOOL_BRUSH)
    {
        m_datDraw.bparam.dT = 0; m_pCurLayer->m_img.drawBrush_line(pt[0], pt[1], pt[2], pt[3]);
        m_datDraw.bparam.dT = 0; m_pCurLayer->m_img.drawBrush_line(pt[2], pt[3], pt[4], pt[5]);
        m_datDraw.bparam.dT = 0; m_pCurLayer->m_img.drawBrush_line(pt[4], pt[5], pt[6], pt[7]);
        m_datDraw.bparam.dT = 0; m_pCurLayer->m_img.drawBrush_line(pt[6], pt[7], pt[0], pt[1]);
    }
    else
    {
        int i,ptn[8];

        for(i = 0; i < 8; i++)
            ptn[i] = (int)pt[i];

        m_pCurLayer->m_img.drawLineB(ptn[0], ptn[1], ptn[2], ptn[3], 255, FALSE);
        m_pCurLayer->m_img.drawLineB(ptn[2], ptn[3], ptn[4], ptn[5], 255, TRUE);
        m_pCurLayer->m_img.drawLineB(ptn[4], ptn[5], ptn[6], ptn[7], 255, TRUE);
        m_pCurLayer->m_img.drawLineB(ptn[6], ptn[7], ptn[0], ptn[1], 255, TRUE);
    }

    commonAfterDraw();
    restoreCursor();
}

//! 円枠描画

void CDraw::draw_circle()
{
    double cx,cy,xr,yr;

    setBeforeDraw(m_nDrawToolNo);

    //イメージ位置に変換

    calcWindowToImage(&cx, &cy, m_ptTmp[0].x, m_ptTmp[0].y);

    xr = m_ptTmp[1].x * m_dViewParam[VIEWPARAM_SCALEDIV];
    yr = m_ptTmp[1].y * m_dViewParam[VIEWPARAM_SCALEDIV];

    //描画

    setCursorWait();
    beginDrawUndo();

    m_pCurLayer->m_img.drawCircle(cx, cy, xr, yr, m_dViewParam, m_bCanvasHRev, (m_nDrawToolNo == TOOL_BRUSH));

    commonAfterDraw();
    restoreCursor();
}


//===========================
//連続直線/集中線
//===========================
/*
    押し時は onDown_xorLine2() で処理
*/

//! 連続直線/集中線（描画）

void CDraw::onDown2_lineSuccConc(BOOL bSucc)
{
    DRECT rc;

    drawXorLine(m_ptTmp[0], m_ptTmp[1]);

    //線描画

    calcDrawLineRect(&rc);

    m_datDraw.rcfDraw.flag = FALSE;

    if(m_nDrawToolNo == TOOL_BRUSH)
    {
        m_datDraw.bparam.dT = 0;

        m_pCurLayer->m_img.drawBrush_lineHeadTail(rc.x1, rc.y1, rc.x2, rc.y2, m_wHeadTailLine);
    }
    else
        m_pCurLayer->m_img.drawLineB((int)rc.x1, (int)rc.y1, (int)rc.x2, (int)rc.y2, 255, FALSE);

    commonFreeDraw(FALSE);  //直接更新

    //次の線

    if(bSucc)
        m_ptTmp[0] = m_ptTmp[1];

    drawXorLine(m_ptTmp[0], m_ptTmp[1]);
}

//! 連続直線/集中線の終了処理
/*!
    @param bStartTo 連続直線時、最後に始点と結ぶ（BACKSPACEキー押し時）
*/

BOOL CDraw::onCancel_lineSuccConc(BOOL bStartTo)
{
    DRECT rc;

    drawXorLine(m_ptTmp[0], m_ptTmp[1]);

    //始点と結ぶ

    if(bStartTo)
    {
        m_ptTmp[1] = m_ptTmp[2];
        calcDrawLineRect(&rc);

        m_datDraw.rcfDraw.flag = FALSE;

        if(m_nDrawToolNo == TOOL_BRUSH)
        {
            m_datDraw.bparam.dT = 0;

            m_pCurLayer->m_img.drawBrush_lineHeadTail(rc.x1, rc.y1, rc.x2, rc.y2, m_wHeadTailLine);
        }
        else
            m_pCurLayer->m_img.drawLineB((int)rc.x1, (int)rc.y1, (int)rc.x2, (int)rc.y2, 255, FALSE);

        commonFreeDraw(FALSE);
    }

    commonDrawEnd();

    return TRUE;
}


//===========================
//ベジェ曲線
//===========================
/*
    m_ptTmp[0-4] : 各点
    m_rcsTmp[0]  : 更新範囲

    xorBox で直線指定 -> 制御点2つ指定(m_nAfterCtlTypeに操作進行)
*/


//! 直線指定後、制御点指定開始

BOOL CDraw::onUp_firstBezier()
{
    m_nNowCtlNo     = NOWCTL_BEZIER;
    m_nAfterCtlType = 0;

    m_ptTmp[2] = m_ptTmp[1];
    m_ptTmp[3] = m_ptTmp[1];

    drawXorBezier(FALSE);

    return FALSE;
}

//! 制御点操作時、移動

void CDraw::onMove_bezier()
{
    AXPoint pt;

    drawXorBezier(TRUE);

    getNowPoint(&pt);

    m_ptTmp[2 + m_nAfterCtlType] = pt;

    if(m_nAfterCtlType == 0)
        m_ptTmp[3] = pt;

    drawXorBezier(FALSE);
}

//! 制御点操作時、離し
/*!
    @return 操作終了するか
*/

BOOL CDraw::onUp_bezier()
{
    int i;
    double pt[8];

    drawXorBezier(TRUE);

    if(m_nAfterCtlType == 0)
    {
        //------ 次の制御点へ

        m_ptTmp[3] = m_ptTmp[2];
        m_nAfterCtlType = 1;

        drawXorBezier(FALSE);

        return FALSE;
    }
    else
    {
        //------- 終了

        free1bitWork();

        //イメージ位置

        for(i = 0; i < 4; i++)
            calcWindowToImage(pt + i * 2, pt + i * 2 + 1, m_ptTmp[i].x, m_ptTmp[i].y);

        //描画

        setCursorWait();
        setBeforeDraw(m_nDrawToolNo);
        beginDrawUndo();

        m_pCurLayer->m_img.drawBezier(pt, m_wHeadTailBezir, (m_nDrawToolNo == TOOL_BRUSH));

        commonAfterDraw();
        restoreCursor();

        return TRUE;
    }
}

//! キャンセル

BOOL CDraw::onCancel_bezier()
{
    drawXorBezier(TRUE);

    free1bitWork();

    return TRUE;
}


//===========================
//スプライン曲線
//===========================
/*
    +Ctrl点追加で筆圧0

    m_nAfterCtlType : 一度 UP が来たら1にする（最初のUPは無効にさせる）
    m_ptTmp[0]      : 前のクライアント位置
*/


//! 最初の押し時

BOOL CDraw::onDown_spline()
{
    double x,y;

    m_nNowCtlNo     = NOWCTL_SPLINE;
    m_nAfterCtlType = 0;

    getNowPoint(&m_ptTmp[0]);

    //位置追加

    calcWindowToImage(&x, &y, m_ptTmp[0].x, m_ptTmp[0].y);

    m_pSplinePos->add(m_ptTmp[0].x, m_ptTmp[0].y, x, y, (isDownKeyCtrl())? 0: 1.0);

    //描画

    m_pimg1Work->clear();
    drawXorSpline(&m_ptTmp[0]);

    return TRUE;
}

//! 離し時（点追加）

BOOL CDraw::onUp_spline()
{
    AXPoint pt;
    double x,y;

    if(m_nAfterCtlType == 0)
        //最初のクリック時の離しは無効
        m_nAfterCtlType = 1;
    else
    {
        getNowPoint(&pt);

        calcWindowToImage(&x, &y, pt.x, pt.y);

        if(m_pSplinePos->add(pt.x, pt.y, x, y, (isDownKeyCtrl())? 0: 1.0))
        {
            drawXorSpline(NULL);
            drawXorSpline(&pt);

            m_ptTmp[0] = pt;
        }
    }

    return FALSE;
}

//! 描画（右クリック、キー押し時など）

BOOL CDraw::onCancel_spline()
{
    drawXorSpline(NULL);

    //描画

    setCursorWait();
    setBeforeDraw(m_nDrawToolNo);
    beginDrawUndo();

    m_pCurLayer->m_img.drawSpline(m_pSplinePos, (m_nDrawToolNo == TOOL_BRUSH));

    commonAfterDraw();
    restoreCursor();

    //作業用解放

    m_pSplinePos->free();
    free1bitWork();

    return TRUE;
}

//! 一つ戻る

BOOL CDraw::onKeyBack_spline()
{
    CSplinePos::POSDAT *p;
    int i,cnt,bx,by;

    cnt = m_pSplinePos->getCnt() - 1;

    //点が一つなら操作をキャンセル

    if(cnt <= 0)
    {
        drawXorSpline(NULL);

        m_pSplinePos->free();
        free1bitWork();

        return TRUE;
    }

    //点削除

    m_pSplinePos->delLast();

    //消去

    m_pimg1Work->blendXor(m_pimgCanvas, 0, 0, m_szCanvas.w, m_szCanvas.h);

    //点再描画

    m_pimg1Work->clear();

    p = m_pSplinePos->getBuf();

    for(i = 0; i < cnt; i++, p++)
    {
        m_pimg1Work->box(p->cx - 2, p->cy - 2, 5, 5, 1);

        if(i) m_pimg1Work->line(bx, by, p->cx, p->cy, 1);

        bx = p->cx;
        by = p->cy;
    }

    m_ptTmp[0].x = bx;
    m_ptTmp[0].y = by;

    m_pimg1Work->blendXor(m_pimgCanvas, 0, 0, m_szCanvas.w, m_szCanvas.h);
    CANVASAREA->redrawUpdate();

    return FALSE;
}
