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
    CDraw [draw_xor] - XOR処理
*/

#include <math.h>
#include <stdlib.h>

#include "CDraw.h"

#include "CCanvasWin.h"
#include "CStatusBar.h"
#include "CPolygonPos.h"
#include "CImage1.h"

#include "AXImage.h"


//=============================
//XOR直線
//=============================
/*
    m_ptTmp[0] : 始点
    m_ptTmp[1] : 終点（マウス位置）
*/


//! 押し

BOOL CDraw::onDown_xorLine(int aftertype)
{
    m_nNowCtlNo     = NOWCTL_XORLINE;
    m_nAfterCtlType = aftertype;

    getNowPoint(&m_ptTmp[0]);

    m_ptTmp[1] = m_ptTmp[0];

    drawXorLine(m_ptTmp[0], m_ptTmp[1]);

    return TRUE;
}

//! 移動（※XorLine2共通）

void CDraw::onMove_xorLine(BOOL b45)
{
    AXPoint pt;
    BOOL flag;

    drawXorLine(m_ptTmp[0], m_ptTmp[1]);

    getNowPoint(&pt);
    if(b45) calcLine45(&pt, m_ptTmp[0]);

    m_ptTmp[1] = pt;

    drawXorLine(m_ptTmp[0], m_ptTmp[1]);

    //ものさし・直線描画時はステータスバーに情報表示

    flag = (m_nNowCtlNo == NOWCTL_XORLINE && m_nAfterCtlType == AFTERLINE_CALCRULE);    //ものさし
    flag |= (m_nNowCtlNo == NOWCTL_XORLINE && m_nAfterCtlType == AFTERLINE_DRAWLINE);   //直線

    if(flag)
        setMainStatus_line(m_ptTmp[0], m_ptTmp[1]);
}

//! 離し
/*!
    @return キャプチャ終了するか
*/

BOOL CDraw::onUp_xorLine()
{
    drawXorLine(m_ptTmp[0], m_ptTmp[1]);

    CANVASAREA->clearTimerUpdate(CCanvasWinArea::TIMERID_UPDATE);

    switch(m_nAfterCtlType)
    {
        //直線描画
        case AFTERLINE_DRAWLINE:
            STATUSBAR->setLabel_help();
            draw_line();
            break;
        //ベジェ曲線
        case AFTERLINE_BEZIER:
            return onUp_firstBezier();
        //グラデーション
        case AFTERLINE_GRAD:
            draw_grad();
            break;
        //ものさし
        case AFTERLINE_CALCRULE:
            STATUSBAR->setLabel_help();
            break;

        //定規、平行線/格子線
        case AFTERLINE_RULE_LINE:
        case AFTERLINE_RULE_GRID:
            double x1,y1,x2,y2,d;

            calcWindowToImage(&x1, &y1, m_ptTmp[0].x, m_ptTmp[0].y);
            calcWindowToImage(&x2, &y2, m_ptTmp[1].x, m_ptTmp[1].y);

            d = ::atan2(y2 - y1, x2 - x1);

            if(m_nAfterCtlType == AFTERLINE_RULE_LINE)
                m_rule.dAngleLine = d;
            else
                m_rule.dAngleGrid = d;
            break;
    }

	return TRUE;
}


//=============================
//XOR直線2
//（連続直線・集中線・多角形）
//=============================
/*
    m_ptTmp[0] : 始点
    m_ptTmp[1] : 終点（マウス位置）
    m_ptTmp[2] : 一番最初の点（保存）

    ※移動時は onDown_xorLine()
*/


//! 最初の押し

BOOL CDraw::onDown_xorLine2(int aftertype)
{
    getNowPoint(&m_ptTmp[0]);

    //各初期化

    switch(aftertype)
    {
        //連続直線・集中線
        case AFTERLINE2_SUCC:
        case AFTERLINE2_CONC:
            setBeforeDraw(m_nDrawToolNo);
            beginDrawUndo();
            break;
        //多角形
        case AFTERLINE2_FILLPOLY:
            if(!onDown_fillPoly()) return FALSE;
            break;
    }

    /* 多角形が初期化に失敗する場合があるので、ここで行う */

    m_nNowCtlNo     = NOWCTL_XORLINE2;
    m_nAfterCtlType = aftertype;

    m_ptTmp[1] = m_ptTmp[0];
    m_ptTmp[2] = m_ptTmp[0];

    drawXorLine(m_ptTmp[0], m_ptTmp[0]);

    return TRUE;
}

//! 2度目以降の押し時

void CDraw::onDown2_xorLine2()
{
    switch(m_nAfterCtlType)
    {
        //連続直線・集中線
        case AFTERLINE2_SUCC:
        case AFTERLINE2_CONC:
            onDown2_lineSuccConc((m_nAfterCtlType == AFTERLINE2_SUCC));
            break;
        //多角形
        case AFTERLINE2_FILLPOLY:
            onDown2_fillPoly();
            break;
    }
}

//! キャンセルor終了処理

BOOL CDraw::onCancel_xorLine2()
{
    switch(m_nAfterCtlType)
    {
        //連続直線・集中線
        case AFTERLINE2_SUCC:
        case AFTERLINE2_CONC:
            onCancel_lineSuccConc(FALSE);
            break;
        //多角形
        case AFTERLINE2_FILLPOLY:
            onCancel_fillPoly();
            break;
    }

    return TRUE;
}


//==================================
//XOR四角枠（キャンバスウィンドウに対する）
//==================================
/*
    m_ptTmp[0] : 左上（ウィンドウ）
    m_ptTmp[1] : 右下（ウィンドウ）
    m_ptTmp[2] : 開始点
*/


//! 押し

BOOL CDraw::onDown_xorBoxWin(int aftertype)
{
    m_nNowCtlNo     = NOWCTL_XORBOXWIN;
    m_nAfterCtlType = aftertype;

    getNowPoint(&m_ptTmp[2]);

    m_ptTmp[0] = m_ptTmp[1] = m_ptTmp[2];

    drawXorBoxWin(m_ptTmp[0], m_ptTmp[1]);

    return TRUE;
}

//! 移動時

void CDraw::onMove_xorBoxWin(BOOL bSquare)
{
    AXPoint pt;
    int w,h;

    drawXorBoxWin(m_ptTmp[0], m_ptTmp[1]);

    getNowPoint(&pt);

    //左上、左下

    if(pt.x < m_ptTmp[2].x)
        m_ptTmp[0].x = pt.x, m_ptTmp[1].x = m_ptTmp[2].x;
    else
        m_ptTmp[0].x = m_ptTmp[2].x, m_ptTmp[1].x = pt.x;

    if(pt.y < m_ptTmp[2].y)
        m_ptTmp[0].y = pt.y, m_ptTmp[1].y = m_ptTmp[2].y;
    else
        m_ptTmp[0].y = m_ptTmp[2].y, m_ptTmp[1].y = pt.y;

    //正方形調整

    if(bSquare)
    {
        w = m_ptTmp[1].x - m_ptTmp[0].x;
        h = m_ptTmp[1].y - m_ptTmp[0].y;

        if(w < h)
        {
            if(pt.x < m_ptTmp[2].x)
                m_ptTmp[0].x = m_ptTmp[1].x - h;
            else
                m_ptTmp[1].x = m_ptTmp[0].x + h;
        }
        else
        {
            if(pt.y < m_ptTmp[2].y)
                m_ptTmp[0].y = m_ptTmp[1].y - w;
            else
                m_ptTmp[1].y = m_ptTmp[0].y + w;
        }
    }

    //

    drawXorBoxWin(m_ptTmp[0], m_ptTmp[1]);
}

//! 離し時

void CDraw::onUp_xorBoxWin()
{
    drawXorBoxWin(m_ptTmp[0], m_ptTmp[1]);
    CANVASAREA->clearTimerUpdate(CCanvasWinArea::TIMERID_UPDATE);

    switch(m_nAfterCtlType)
    {
        //四角枠描画
        case AFTERBOXWIN_DRAWBOX:
            draw_box();
            break;
        //四角塗りつぶし描画
        case AFTERBOXWIN_DRAWFILL:
            draw_fillBox();
            break;
        //閉領域塗りつぶし
        case AFTERBOXWIN_PAINTCLOSE:
            draw_paintClose_box();
            break;
        //選択範囲
        case AFTERBOXWIN_DRAWSEL:
            draw_sel_box();
            break;
    }
}


//=============================
//XOR四角枠 (イメージに対する)
//=============================
/*
    m_ptTmp[0] : 開始点（イメージ位置）
    m_ptTmp[1] : 左上位置（イメージ位置）
    m_ptTmp[2] : 右下位置（イメージ位置）
*/


//! 押し

BOOL CDraw::onDown_xorBoxImg(int aftertype)
{
    m_nNowCtlNo     = NOWCTL_XORBOXIMG;
    m_nAfterCtlType = aftertype;

    calcWindowToImage(&m_ptTmp[0], m_ptDownWinPos.x, m_ptDownWinPos.y);

    m_ptTmp[1] = m_ptTmp[2] = m_ptTmp[0];

    drawXorBoxImg();

    return TRUE;
}

//! 移動時

void CDraw::onMove_xorBoxImg(BOOL bSquare)
{
    AXPoint pt;
    int w,h;

    calcWindowToImage(&pt, m_ptNowWinPos.x, m_ptNowWinPos.y);

    //消す

    drawXorBoxImg();

    //左上、左下

    if(pt.x < m_ptTmp[0].x)
        m_ptTmp[1].x = pt.x, m_ptTmp[2].x = m_ptTmp[0].x;
    else
        m_ptTmp[1].x = m_ptTmp[0].x, m_ptTmp[2].x = pt.x;

    if(pt.y < m_ptTmp[0].y)
        m_ptTmp[1].y = pt.y, m_ptTmp[2].y = m_ptTmp[0].y;
    else
        m_ptTmp[1].y = m_ptTmp[0].y, m_ptTmp[2].y = pt.y;

    //正方形調整

    if(bSquare)
    {
        w = m_ptTmp[2].x - m_ptTmp[1].x;
        h = m_ptTmp[2].y - m_ptTmp[1].y;

        if(w < h)
        {
            if(pt.x < m_ptTmp[0].x)
                m_ptTmp[1].x = m_ptTmp[2].x - h;
            else
                m_ptTmp[2].x = m_ptTmp[1].x + h;
        }
        else
        {
            if(pt.y < m_ptTmp[0].y)
                m_ptTmp[1].y = m_ptTmp[2].y - w;
            else
                m_ptTmp[2].y = m_ptTmp[1].y + w;
        }
    }

    //

    drawXorBoxImg();
}

//! 離し時

void CDraw::onUp_xorBoxImg()
{
    FLAGRECT rcf;

    drawXorBoxImg();
    CANVASAREA->clearTimerUpdate(CCanvasWinArea::TIMERID_UPDATE);

    //範囲をイメージ内に調整 -> m_rcsTmp[0]
    //FALSEで範囲外

    rcf.x1 = m_ptTmp[1].x;
    rcf.y1 = m_ptTmp[1].y;
    rcf.x2 = m_ptTmp[2].x;
    rcf.y2 = m_ptTmp[2].y;

    if(!calcImgRectInCanvas(&m_rcsTmp[0], rcf)) return;

    //

    switch(m_nAfterCtlType)
    {
        //左右反転
        case AFTERBOXIMG_HREV:
            draw_boxedit_revHV(TRUE);
            break;
        //上下反転
        case AFTERBOXIMG_VREV:
            draw_boxedit_revHV(FALSE);
            break;
        //拡大縮小＆回転
        case AFTERBOXIMG_SCALEROT:
            //後で実行
            CANVASWIN->sendCommand(CCanvasWin::CMDID_SCALEROTDLG, 0, 0);
            break;
        //トリミング
        case AFTERBOXIMG_TRIM:
            resizeCanvas(m_rcsTmp[0].w, m_rcsTmp[0].h, -m_rcsTmp[0].x, -m_rcsTmp[0].y);
            break;
    }
}


//====================================
//XOR円 （キャンバスウィンドウに対する）
//====================================
/*
    m_ptTmp[0] : 位置
    m_ptTmp[1] : 半径
*/


//! 押し

BOOL CDraw::onDown_xorCircle(int aftertype)
{
    m_nNowCtlNo     = NOWCTL_XORCIRCLE;
    m_nAfterCtlType = aftertype;

    getNowPoint(&m_ptTmp[0]);

    m_ptTmp[1].x = 0;
    m_ptTmp[1].y = 0;

    drawXorCircle(m_ptTmp[0], m_ptTmp[1]);

    return TRUE;
}

//! 移動

void CDraw::onMove_xorCircle(BOOL bSquare)
{
    AXPoint pt;
    int xr,yr;

    drawXorCircle(m_ptTmp[0], m_ptTmp[1]);

    getNowPoint(&pt);

    xr = ::abs(pt.x - m_ptTmp[0].x);
    yr = ::abs(pt.y - m_ptTmp[0].y);

    if(bSquare)
    {
        if(xr > yr) yr = xr;
        else xr = yr;
    }

    m_ptTmp[1].x = xr;
    m_ptTmp[1].y = yr;

    drawXorCircle(m_ptTmp[0], m_ptTmp[1]);
}

//! 離し

void CDraw::onUp_xorCircle()
{
    drawXorCircle(m_ptTmp[0], m_ptTmp[1]);

    CANVASAREA->clearTimerUpdate(CCanvasWinArea::TIMERID_UPDATE);

    switch(m_nAfterCtlType)
    {
        //円枠描画
        case AFTERCIR_DRAWCIRCLE:
            draw_circle();
            break;
        //円塗りつぶし描画
        case AFTERCIR_DRAWFILL:
            draw_fillCircle();
            break;
        //定規、楕円
        case AFTERCIR_RULE:
            double xx,yy;

            calcWindowToImage(&m_rule.ptCtEll, m_ptTmp[0].x, m_ptTmp[0].y);

            xx = m_ptTmp[1].x * m_dViewParam[VIEWPARAM_SCALEDIV];
            yy = m_ptTmp[1].y * m_dViewParam[VIEWPARAM_SCALEDIV];
            if(xx == 0) xx = 0.1;
            if(yy == 0) yy = 0.1;

            m_rule.dEllHV[0]    = xx / yy;
            m_rule.dEllHV[1]    = yy / xx;
            m_rule.dEllTmp[0]   = m_dViewParam[VIEWPARAM_COSREV];
            m_rule.dEllTmp[1]   = m_dViewParam[VIEWPARAM_SINREV];
            m_rule.dEllTmp[2]   = m_dViewParam[VIEWPARAM_COS];
            m_rule.dEllTmp[3]   = m_dViewParam[VIEWPARAM_SIN];
            m_rule.bEllRevH     = m_bCanvasHRev;
            break;
    }
}


//=============================
//XOR投げ縄
//=============================
/*
    m_ptTmp[0] : 前の位置
    m_ptTmp[1] : 現在の位置
*/


//! 押し時

BOOL CDraw::onDown_xorLasso(int aftertype)
{
    AXPoint pt;

    getNowPoint(&m_ptTmp[0]);

    //作業用

    if(!create1bitWork(m_szCanvas.w, m_szCanvas.h))
        return FALSE;

    //多角形位置

    if(!m_pPolyPos->alloc(200))
    {
        free1bitWork();
        return FALSE;
    }

    calcWindowToImage(&pt, m_ptTmp[0].x, m_ptTmp[0].y);
    m_pPolyPos->add(pt.x, pt.y);

    //

    m_nNowCtlNo     = NOWCTL_XORLASSO;
    m_nAfterCtlType = aftertype;

    m_ptTmp[1] = m_ptTmp[0];

    drawXorLasso(FALSE);

    return TRUE;
}

//! 移動時

void CDraw::onMove_xorLasso()
{
    AXPoint pt;

    m_ptTmp[0] = m_ptTmp[1];

    getNowPoint(&m_ptTmp[1]);

    drawXorLasso(FALSE);

    //点追加

    calcWindowToImage(&pt, m_ptTmp[1].x, m_ptTmp[1].y);
    m_pPolyPos->add(pt.x, pt.y);
}

//! 離し時

void CDraw::onUp_xorLasso()
{
    drawXorLasso(TRUE);
    CANVASAREA->clearTimerUpdate(CCanvasWinArea::TIMERID_UPDATE);

    free1bitWork();
    m_pPolyPos->endPos();

    switch(m_nAfterCtlType)
    {
        //塗りつぶし
        case AFTERLASSO_FILL:
            draw_fillPolygon_common();
            break;
        //閉領域
        case AFTERLASSO_PAINTCLOSE:
            draw_paintClose_common(TRUE);
            break;
        //選択範囲
        case AFTERLASSO_SEL:
            draw_sel_polygonCommon();
            break;
    }

    m_pPolyPos->free();
}


//=============================
//そのほか
//=============================


//! 定規:集中線・正円 の中心位置 押し時
/*
    m_ptTmp[0] : 位置
*/

BOOL CDraw::onDown_ruleCenterPos()
{
    AXPoint pt;

    m_nNowCtlNo = NOWCTL_RULEPOS;

    getNowPoint(&m_ptTmp[0]);

    //中央位置セット

    calcWindowToImage(&pt, m_ptTmp[0].x, m_ptTmp[0].y);

    if(m_ruleType == RULETYPE_CONCLINE)
        m_rule.ptCtConc = pt;
    else
        m_rule.ptCtCir = pt;

    //

    drawXorRulePos(m_ptTmp[0]);

    return TRUE;
}


//=============================
// XOR描画
//=============================


//! XOR直線

void CDraw::drawXorLine(const AXPoint &pt1,const AXPoint &pt2)
{
    AXRectSize rcs;

    m_pimgCanvas->line(pt1.x, pt1.y, pt2.x, pt2.y, AXImage::COL_XOR);

    //更新

    rcs.setFromPoint(pt1.x, pt1.y, pt2.x, pt2.y);

    CANVASAREA->redraw(rcs);
    CANVASAREA->setTimer_update();
}

//! XOR四角枠（ウィンドウに対する）

void CDraw::drawXorBoxWin(const AXPoint &pt1,const AXPoint &pt2)
{
    AXRectSize rcs;

    m_pimgCanvas->box(pt1.x, pt1.y, pt2.x - pt1.x + 1, pt2.y - pt1.y + 1, AXImage::COL_XOR);

    //更新

    rcs.setFromPoint(pt1.x, pt1.y, pt2.x, pt2.y);

    CANVASAREA->redraw(rcs);
    CANVASAREA->setTimer_update();
}

//! XOR四角枠描画（イメージに対する）

void CDraw::drawXorBoxImg()
{
    AXPoint pt[5];
    int x1,y1,x2,y2;

    x1 = m_ptTmp[1].x, y1 = m_ptTmp[1].y;
    x2 = m_ptTmp[2].x, y2 = m_ptTmp[2].y;

    if(m_nCanvasScale != 100) x2++, y2++;

    //イメージ -> ウィンドウ位置

    calcImageToWindow(pt    , x1, y1);
    calcImageToWindow(pt + 1, x2, y1);
    calcImageToWindow(pt + 2, x2, y2);
    calcImageToWindow(pt + 3, x1, y2);
    pt[4] = pt[0];

    //描画

    m_pimgCanvas->lines(pt, 5, AXImage::COL_XOR);

    //更新

    CANVASAREA->redraw();
    CANVASAREA->setTimer_update();
}

//! XOR円

void CDraw::drawXorCircle(const AXPoint &pt1,const AXPoint &pt2)
{
    m_pimgCanvas->ellipse(pt1.x - pt2.x, pt1.y - pt2.y, pt1.x + pt2.x, pt1.y + pt2.y, AXImage::COL_XOR);

    //更新

    CANVASAREA->redraw(pt1.x - pt2.x, pt1.y - pt2.y, pt2.x * 2 + 1, pt2.y * 2 + 1);
    CANVASAREA->setTimer_update();
}

//! XOR投げ縄

void CDraw::drawXorLasso(BOOL bErase)
{
    if(bErase)
    {
        m_pimg1Work->blendXor(m_pimgCanvas, 0, 0, m_szCanvas.w, m_szCanvas.h);

        CANVASAREA->redraw();
    }
    else
    {
        AXRect rc;
        AXRectSize rcs;

        rc.setFromPoint(m_ptTmp[0], m_ptTmp[1]);

        if(calcRectInCanvasWin(&rcs, rc))
        {
            m_pimg1Work->blendXor(m_pimgCanvas, rcs.x, rcs.y, rcs.w, rcs.h);
            m_pimg1Work->line(m_ptTmp[0].x, m_ptTmp[0].y, m_ptTmp[1].x, m_ptTmp[1].y, 1);
            m_pimg1Work->blendXor(m_pimgCanvas, rcs.x, rcs.y, rcs.w, rcs.h);

            CANVASAREA->redraw(rcs);
        }
    }

    CANVASAREA->setTimer_update();
}

//! XORベジェ曲線

void CDraw::drawXorBezier(BOOL bErase)
{
    if(!bErase)
    {
        m_pimg1Work->clear();
        m_pimg1Work->drawBezierTemp(m_ptTmp, &m_rcsTmp[0], m_nAfterCtlType);
    }

    m_pimg1Work->blendXor(m_pimgCanvas, m_rcsTmp[0].x, m_rcsTmp[0].y, m_rcsTmp[0].w, m_rcsTmp[0].h);

    CANVASAREA->redraw(m_rcsTmp[0]);
    CANVASAREA->setTimer_update();
}

//! スプライン曲線
/*!
    m_ptTmp[0] が 前の位置。

    @param ppt  追加位置。NULLで消去
*/

void CDraw::drawXorSpline(AXPoint *ppt)
{
    if(ppt)
    {
        m_pimg1Work->box(ppt->x - 2, ppt->y - 2, 5, 5, 1);
        m_pimg1Work->line(m_ptTmp[0].x, m_ptTmp[0].y, ppt->x, ppt->y, 1);
    }

    m_pimg1Work->blendXor(m_pimgCanvas, 0, 0, m_szCanvas.w, m_szCanvas.h);

    CANVASAREA->redrawUpdate();
}

//! 定規中央位置（十字線）

void CDraw::drawXorRulePos(const AXPoint &pt)
{
    //描画

    m_pimgCanvas->lineH(pt.x - 15, pt.y, 31, AXImage::COL_XOR);
    m_pimgCanvas->lineV(pt.x, pt.y - 15, 31, AXImage::COL_XOR);

    //更新

    CANVASAREA->redraw(pt.x - 15, pt.y - 15, 31, 31);
}

//! ブラシ円描画
/*!
    m_ptTmp[0] が中心ウィンドウ位置。m_nTmp[0] が直径pxサイズ。
*/

void CDraw::drawXorBrushCircle(BOOL bErase)
{
    AXRectSize rcs;

    if(bErase)
        rcs = m_rcsTmp[0];
    else
    {
        rcs.x = m_ptTmp[0].x - m_nTmp[0] / 2;
        rcs.y = m_ptTmp[0].y - m_nTmp[0] / 2;
        rcs.w = m_nTmp[0];
        rcs.h = m_nTmp[0];

        if(rcs.w == 0) rcs.w = 1;
        if(rcs.h == 0) rcs.h = 1;

        m_rcsTmp[0] = rcs;
    }

    //描画

    m_pimgCanvas->ellipse(rcs.x, rcs.y, rcs.x + rcs.w - 1, rcs.y + rcs.h - 1, AXImage::COL_XOR);

    //更新

    CANVASAREA->redraw(rcs);
}
