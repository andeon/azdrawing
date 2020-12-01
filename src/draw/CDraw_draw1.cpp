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
    CDraw [draw1]

    描画処理

    ボタン離し後に実行、スポイト、イメージ位置移動、
    キャンバス移動、キャンバス回転、上下ドラッグでの表示倍率変更、左右ドラッグでのブラシサイズ変更
*/


#include <math.h>

#include "CDraw.h"

#include "CConfig.h"
#include "CCanvasWin.h"
#include "CBrushWin.h"
#include "CScaleRotDlg.h"

#include "CLayerList.h"
#include "CUndo.h"
#include "CDrawPaint.h"
#include "CPolygonPos.h"
#include "CPosBuf.h"
#include "CImage1.h"
#include "CImage8.h"
#include "CGradList.h"
#include "CBrushManager.h"
#include "CBrushTreeItem.h"
#include "CApp.h"

#include "defGlobal.h"

#include "AXMem.h"



//=============================
// 描画処理
//=============================


//! 多角形塗りつぶし 共通

void CDraw::draw_fillPolygon_common()
{
    setCursorWait();
    beginDrawUndo();

    m_pCurLayer->m_img.drawFillPolygon(m_pPolyPos,
        getDrawValBrush(m_optToolTmp & TOOLPOLY_VAL_MASK, m_optToolTmp & TOOLPOLY_F_BRUSHVAL),
        m_optToolTmp & TOOLPOLY_F_ANTI);

    commonAfterDraw();
    restoreCursor();
}

//! 四角塗りつぶし描画

void CDraw::draw_fillBox()
{
    AXPoint pt[4];
    int i;

    //描画位置

    calcDrawBoxPointInt(pt);

    //点セット

    if(!m_pPolyPos->alloc(5)) return;

    for(i = 0; i < 4; i++)
        m_pPolyPos->add(pt[i].x, pt[i].y);

    m_pPolyPos->endPos();

    //描画

    draw_fillPolygon_common();

    //終了

    m_pPolyPos->free();
}

//! 円塗りつぶし描画

void CDraw::draw_fillCircle()
{
    double cx,cy,xr,yr;

    calcWindowToImage(&cx, &cy, m_ptTmp[0].x, m_ptTmp[0].y);

    xr = m_ptTmp[1].x * m_dViewParam[VIEWPARAM_SCALEDIV];
    yr = m_ptTmp[1].y * m_dViewParam[VIEWPARAM_SCALEDIV];

    //描画
    /* m_optToolTmp は setBeforeDraw() でセットされている */

    setCursorWait();
    beginDrawUndo();

    m_pCurLayer->m_img.drawFillCircle(cx, cy, xr, yr, m_dViewParam, m_bCanvasHRev,
        getDrawValBrush(m_optToolTmp & TOOLPOLY_VAL_MASK, m_optToolTmp & TOOLPOLY_F_BRUSHVAL),
        m_optToolTmp & TOOLPOLY_F_ANTI);

    commonAfterDraw();
    restoreCursor();
}

//! 塗りつぶし（共通）
/*!
    @param type [0]アンチエイリアス自動 [1]指定濃度未満の範囲
    @param nVal 塗る濃度
    @param nCompareVal 比較濃度
    @param bDisableRef 判定元無効
*/

void CDraw::draw_paintCommon(int type,int nVal,int nCompareVal,BOOL bDisableRef)
{
    CDrawPaint paint;
    CLayerImg *pStart;
    AXPoint pt;
    int ret,tool;

    //判定元、+Shift で無効

    if(isDownKeyShift())
        bDisableRef = TRUE;

    //判定元リンクセット

    pStart = m_player->setPaintRefLink(&m_pCurLayer->m_img, bDisableRef);

    //開始位置

    calcWindowToImage(&pt, m_ptDownWinPos.x, m_ptDownWinPos.y);

    //初期化

    ret = paint.init(&m_pCurLayer->m_img, pStart, pt, nVal, nCompareVal, type);

    if(ret != CDrawPaint::RET_SUCCESS)
        return;

    //ツール初期化

    if(type == CDrawPaint::TYPE_ERASE)
        tool = TOOL_PAINTERASE;
    else
        tool = TOOL_PAINT;

    setBeforeDraw(tool);

    //描画

    setCursorWait();
    beginDrawUndo();

    paint.run();

    commonAfterDraw();
    restoreCursor();
}

//! グラデーション描画

void CDraw::draw_grad()
{
    AXMem memTbl;
    AXPoint pt[2];
    int flag;
    AXRect rc;

    //テーブル確保

    if(!memTbl.alloc(1024)) return;

    //濃度テーブルセット

    m_pGrad->setGradTable(memTbl, 1024,
                          m_optToolGrad & TOOLGRAD_VAL_MASK,
                          m_optToolGrad & TOOLGRAD_F_REV);

    //描画位置

    calcWindowToImage(&pt[0], m_ptTmp[0].x, m_ptTmp[0].y);
    calcWindowToImage(&pt[1], m_ptTmp[1].x, m_ptTmp[1].y);

    //フラグ

    flag = 0;
    if(m_optToolGrad & TOOLGRAD_F_LOOP) flag |= 1;  //繰り返し

    //描画する矩形範囲

    if(m_rcfSel.flag)
        m_rcfSel.toRect(&rc);
    else
        m_pCurLayer->m_img.getEnableDrawRectPixel(&rc);

    //描画

    setCursorWait();
    setBeforeDraw(TOOL_GRAD);
    beginDrawUndo();

    switch(m_toolSubNo[TOOL_GRAD])
    {
        //線形
        case 0:
            m_pCurLayer->m_img.drawGradient_line(pt[0].x, pt[0].y, pt[1].x, pt[1].y, rc, memTbl, flag);
            break;
        //円
        case 1:
            m_pCurLayer->m_img.drawGradient_circle(pt[0].x, pt[0].y, pt[1].x, pt[1].y, rc, memTbl, flag);
            break;
        //矩形
        case 2:
            m_pCurLayer->m_img.drawGradient_box(pt[0].x, pt[0].y, pt[1].x, pt[1].y, rc, memTbl, flag);
            break;
    }

    commonAfterDraw();
    restoreCursor();
}


//=============================
//閉領域
//=============================
/*
    m_pimgTmp に処理する範囲の図形を描画する。
    また、m_datDraw.rcfDraw にその範囲をセットする。
*/


//! 閉領域、四角形

void CDraw::draw_paintClose_box()
{
    AXPoint pt[4];
    int i;

    //描画位置

    calcDrawBoxPointInt(pt);

    //点セット

    if(!m_pPolyPos->alloc(5)) return;

    for(i = 0; i < 4; i++)
        m_pPolyPos->add(pt[i].x, pt[i].y);

    m_pPolyPos->endPos();

    //

    draw_paintClose_common(TRUE);
}

//! 閉領域、共通描画処理
/*!
    @param bPolygon TRUE で m_pPolyPos から多角形描画。FALSE で m_pimgTmp の内容をそのまま使う。
*/

void CDraw::draw_paintClose_common(BOOL bPolygon)
{
    CDrawPaint paint;
    CLayerImg *pStart,imgRef;
    AXPoint pt;
    int ret;

    setCursorWait();

    //m_pPolyPos から、m_pimgTmp に多角形描画（キャンバスの範囲）

    if(bPolygon)
    {
        if(!m_pimgTmp->create(m_nImgW, m_nImgH)) goto END;

        m_datDraw.rcfDraw.flag = FALSE;
        if(!m_pimgTmp->drawFillPolygonTmp(m_pPolyPos, 255, &CLayerImg::setPixel_calcRect)) goto END;
    }

    //処理範囲がない

    if(!m_datDraw.rcfDraw.flag) goto END;

    //塗りつぶし判定元リンクセット

    pStart = m_player->setPaintRefLink(&m_pCurLayer->m_img, m_optToolPaintClose & TOOLPAINTC_F_NOREF);

    //m_pimgTmp の最小範囲を1px拡張したサイズで imgRef を作成し、判定元レイヤをすべて合成

    ret = paint.setCloseRefImg(&imgRef, m_pimgTmp, m_datDraw.rcfDraw, pStart);
    if(ret != CDrawPaint::RET_SUCCESS) goto END;

    m_pimgTmp->free();

    //塗りつぶし初期化
    //imgRef を判定元として使う

    pt.x = m_datDraw.rcfDraw.x1 - 1;
    pt.y = m_datDraw.rcfDraw.y1 - 1;

    ret = paint.init(&m_pCurLayer->m_img, &imgRef, pt,
            getDrawValBrush(m_optToolPaintClose & TOOLPAINTC_VAL_MASK, m_optToolPaintClose & TOOLPAINTC_F_USEBRUSH),
            0, CDrawPaint::TYPE_CLOSE);

    if(ret != CDrawPaint::RET_SUCCESS) goto END;

    //塗りつぶし実行

    setBeforeDraw(TOOL_PAINTCLOSE);
    beginDrawUndo();

    paint.run();

    commonAfterDraw();

    //------- 終了

END:
    m_pimgTmp->free();
    m_pPolyPos->free();

    restoreCursor();
}


//=============================
//矩形編集
//=============================
/*
    m_rcsTmp[0] に矩形範囲（イメージ位置）
    ※範囲はキャンバス内に調整されている
*/


//! 左右・上下反転

void CDraw::draw_boxedit_revHV(BOOL bHorz)
{
    setCursorWait();
    setBeforeDraw(TOOL_BOXEDIT);
    beginDrawUndo();

    if(bHorz)
        m_pCurLayer->m_img.reverseHorz(m_rcsTmp[0]);
    else
        m_pCurLayer->m_img.reverseVert(m_rcsTmp[0]);

    commonAfterDraw();
    restoreCursor();
}

//! 拡大縮小＆回転
/*
    カレントレイヤの矩形範囲を CImage8 イメージに変換し、それを元に描画する。
    プレビュー中はカレントレイヤの矩形範囲は消去する。

    m_pimgTmp   : プレビュー描画用(updateRect 時に直接描画)
    m_datDraw.rcfDraw : プレビューの描画範囲
*/

void CDraw::draw_boxedit_scaleRot()
{
    CScaleRotDlg *pdlg;
    CImage8 img8;
    AXRect rc;
    FLAGRECT rcf;
    BOOL ret;
    double val[2];

    //範囲をFLAGRECTに

    rcf.x1 = m_rcsTmp[0].x;
    rcf.y1 = m_rcsTmp[0].y;
    rcf.x2 = m_rcsTmp[0].x + m_rcsTmp[0].w - 1;
    rcf.y2 = m_rcsTmp[0].y + m_rcsTmp[0].h - 1;
    rcf.flag = TRUE;

    rcf.toRect(&rc);

    //範囲のイメージを8BITにコピー

    if(!m_pCurLayer->m_img.convTo8Bit(&img8, m_rcsTmp[0], TRUE))
        return;

    //プレビュー用

    if(!m_pimgTmp->create(m_nImgW, m_nImgH)) return;

    //----------

    m_nNowCtlNo = NOWCTL_BOXEDIT_SCALEROT;

    m_datDraw.rcfDraw.flag = FALSE; //プレビューの描画範囲

    //元範囲をクリアする

    m_pCurLayer->m_img.fillBox(rc, 0, &CLayerImg::setPixel_clear);

    //プレビュー（OFF 時は、消去した部分を更新）

    if(m_optBEScaleRot & BESCALEROT_F_PREV)
        draw_boxedit_scaleRotPrev(img8, rc, 1.0, 0);
    else
        updateCanvasAndPrevWin(rcf, FALSE);

    //--------- ダイアログ

    pdlg = new CScaleRotDlg(&img8, &rc, val);

    ret = pdlg->runDialog();

    //---------

    m_nNowCtlNo = NOWCTL_NONE;

    m_pimgTmp->free();

    //クリアした部分のイメージを戻す

    m_pCurLayer->m_img.restoreImageFrom8bit(rc, img8);

    //矩形範囲＋プレビュー範囲を戻す

    calcUnionFlagRect(&rcf, m_datDraw.rcfDraw);
    updateCanvasAndPrevWin(rcf, FALSE);

    //描画

    if(ret)
    {
        setCursorWait();
        setBeforeDraw(TOOL_BOXEDIT);
        beginDrawUndo();

        //消去

        m_datDraw.nColSetFunc = CLayerImg::COLSETF_OVERWRITE;

        m_pCurLayer->m_img.fillBox(rc, 0, &CLayerImg::setPixelDraw);

        //描画

        m_datDraw.nColSetFunc = CLayerImg::COLSETF_BLEND;

        m_pCurLayer->m_img.scaleAndRotate(img8, rc, val[0], val[1], TRUE, &CLayerImg::setPixelDraw);

        commonAfterDraw();
        restoreCursor();
    }
}

//! 拡大縮小＆回転 プレビュー

void CDraw::draw_boxedit_scaleRotPrev(const CImage8 &img8,const AXRect &rc,double scale,double angle)
{
    FLAGRECT rcf = m_datDraw.rcfDraw;

    m_pimgTmp->freeAllTile();
    m_datDraw.rcfDraw.flag = FALSE;

    //

    if(m_optBEScaleRot & BESCALEROT_F_PREV)
    {
        //描画

        m_datDraw.rcfDraw.flag = FALSE;

        m_pimgTmp->scaleAndRotate(img8, rc, scale, angle,
            m_optBEScaleRot & BESCALEROT_F_HQPREV,
            &CLayerImg::setPixel_calcRect);

        //範囲

        calcUnionFlagRect(&rcf, m_datDraw.rcfDraw);
    }

    updateCanvasAndPrevWin(rcf, FALSE);
}


//*********************************
//*********************************


//=============================
//閉領域 ブラシ描画
//=============================
/*
    m_pimgTmp(CLayerImg) に処理範囲を描画していく
*/


//! 押し時

BOOL CDraw::onDown_paintclose_brush()
{
    //tmpイメージ作成

    if(!m_pimgTmp->create(m_nImgW, m_nImgH)) return FALSE;

    //描画準備

    setBeforeDraw_paintCloseBrush();

    m_rcfDrawUp.flag = FALSE;

    //

    m_nNowCtlNo = NOWCTL_PAINTCLOSE_BRUSH;

    return TRUE;
}

//! 移動時

void CDraw::onMove_paintclose_brush()
{
    DRAWPOINT pt;
    CPosBuf::POSDAT *p;

    //位置 （定規無効）

    getDrawPoint(&pt, FALSE);

    p = m_pPosBuf->addPos(pt.x, pt.y, pt.press, 0, 0);

    //描画

    m_datDraw.rcfDraw.flag = FALSE;

    m_pimgTmp->drawBrush_free(p->x, p->y, p->press);

    commonFreeDraw();
}

//! 離し時

void CDraw::onUp_paintclose_brush()
{
    //ブラシ跡を消す
    /* updateRect() 時に m_pimgTmp の範囲を描画しないようにするため
       m_nNowCtlNo をクリアしておく */

    m_nNowCtlNo = NOWCTL_NONE;

    updateCanvasAndPrevWin(m_rcfDrawUp, FALSE);

    //閉領域実行

    m_datDraw.rcfDraw = m_rcfDrawUp;

    draw_paintClose_common(FALSE);
}


//=============================
//多角形
//=============================
/*
    m_nAfterCtlType2 に動作タイプ。
    m_pimg1Work に確定された直線を描画。
    現在の線は直接キャンバスに描画。
*/


//! 最初の押し時
/*!
    @return FALSEで初期化失敗
*/

BOOL CDraw::onDown_fillPoly()
{
    AXPoint pt;

    if(!create1bitWork(m_szCanvas.w, m_szCanvas.h))
        return FALSE;

    if(!m_pPolyPos->alloc(50))
    {
        free1bitWork();
        return FALSE;
    }

    calcWindowToImage(&pt, m_ptTmp[0].x, m_ptTmp[0].y);
    m_pPolyPos->add(pt.x, pt.y);

    return TRUE;
}

//! 2度目以降の押し時

void CDraw::onDown2_fillPoly()
{
    AXPoint pt;

    //点追加

    calcWindowToImage(&pt, m_ptTmp[1].x, m_ptTmp[1].y);

    m_pPolyPos->add(pt.x, pt.y);

    //XOR

    drawXorLine(m_ptTmp[0], m_ptTmp[1]);

    m_pimg1Work->blendXor(m_pimgCanvas, 0, 0, m_szCanvas.w, m_szCanvas.h);

    m_pimg1Work->line(m_ptTmp[0].x, m_ptTmp[0].y, m_ptTmp[1].x, m_ptTmp[1].y, 1);

    m_pimg1Work->blendXor(m_pimgCanvas, 0, 0, m_szCanvas.w, m_szCanvas.h);

    m_ptTmp[0] = m_ptTmp[1];

    drawXorLine(m_ptTmp[0], m_ptTmp[1]);

    CANVASAREA->redraw();
}

//! 終了/キャンセル
/*!
    @return TRUEで操作終了
*/

BOOL CDraw::onCancel_fillPoly(BOOL bDraw)
{
	//XOR消去

    drawXorLine(m_ptTmp[0], m_ptTmp[1]);
    m_pimg1Work->blendXor(m_pimgCanvas, 0, 0, m_szCanvas.w, m_szCanvas.h);

    CANVASAREA->redraw();
    CANVASAREA->clearTimerUpdate(CCanvasWinArea::TIMERID_UPDATE);

    //描画

    if(bDraw)
    {
        //始点追加

        m_pPolyPos->endPos();

        //描画

        switch(m_nAfterCtlType2)
        {
            //多角形塗りつぶし
            case AFTER2FILLPOLY_FILL:
                draw_fillPolygon_common();
                break;
            //閉領域
            case AFTER2FILLPOLY_PAINTCLOSE:
                draw_paintClose_common(TRUE);
                break;
            //選択範囲
            case AFTER2FILLPOLY_SEL:
                draw_sel_polygonCommon();
                break;
        }
    }

	//削除

	m_pPolyPos->free();
	free1bitWork();

	return TRUE;
}


//=============================
//ボタン離し後に実行
//=============================


//! 押し時

BOOL CDraw::onDown_funcUp(int aftertype)
{
    m_nNowCtlNo     = NOWCTL_FUNCUP;
    m_nAfterCtlType = aftertype;

    return TRUE;
}

//! 離し時

void CDraw::onUp_funcUp()
{
    switch(m_nAfterCtlType)
    {
        //塗りつぶし
        case AFTERUP_PAINT:
            draw_paintCommon(
                (m_optToolPaint >> TOOLPAINT_TYPE_SHIFT) & TOOLPAINT_TYPE_MASK,
                getDrawValBrush(m_optToolPaint & TOOLPAINT_VAL_MASK, m_optToolPaint & TOOLPAINT_F_USEBRUSH),
                (m_optToolPaint >> TOOLPAINT_VALCMP_SHIFT) & 255,
                m_optToolPaint & TOOLPAINT_F_NOREF);
            break;
        //不透明範囲消去
        case AFTERUP_PAINTERASE:
            draw_paintCommon(CDrawPaint::TYPE_ERASE, 0, 0, m_optToolPaintErase & TOOLPAINTE_NOREF);
            break;
    }
}


//=============================
//スポイト
//=============================


BOOL CDraw::onDown_spoit()
{
    CLayerItem *p;
    AXPoint pt;
    int col = 0,c;

    m_nNowCtlNo = NOWCTL_TEMP;

    //色取得

    calcWindowToImage(&pt, m_ptDownWinPos.x, m_ptDownWinPos.y);

    if(isDownKeyShift())
    {
        //+Shift : 全レイヤ合成後

        for(p = m_player->getTopItem(); p; p = p->next())
        {
            if(p->isView())
            {
                c = p->m_img.getPixel(pt.x, pt.y);
                c = c * p->m_btAlpha >> 7;

                col = ((((c + col) << 5) - ((c * col << 5) / 255) + (1 << 4)) >> 5);
            }
        }
    }
    else
        //カレントレイヤ
        col = m_pCurLayer->m_img.getPixel(pt.x, pt.y);

    //ブラシに濃度セット（透明の場合はセットしない）

    if(col)
        BRUSHWIN->setBrushVal(col);

    return TRUE;
}


//=============================
//イメージ位置移動
//=============================
/*
    m_dptTmp[0] : 総移動px数（ウィンドウ）
    m_ptTmp[1]  : 押し時の元オフセット位置
    m_ptTmp[2]  : オフセット位置総移動px数

    押し時に各レイヤの作業フラグをクリアする。移動されたレイヤはフラグON
*/


//! 押し

BOOL CDraw::onDown_imgMove()
{
    m_nNowCtlNo = NOWCTL_IMGMOVE;

    //

    m_dptTmp[0].x = m_dptTmp[0].y = 0;

    m_pCurLayer->m_img.getOffset(&m_ptTmp[1]);

    m_ptTmp[2].zero();

    //作業用フラグクリア

    m_player->clearWorkFlag();

    return TRUE;
}

//! 移動

void CDraw::onMove_imgMove(BOOL bHorz,BOOL bVert)
{
    AXPoint pt,pt2,ptMov;
    int type;
    double x,y;

    //ウィンドウ位置移動数

    x = m_ptNowWinPos.x - m_ptBkWinPos.x;
    y = m_ptNowWinPos.y - m_ptBkWinPos.y;

    if(bVert) x = 0;
    if(bHorz) y = 0;

    //--------- 移動数計算

    //総移動数加算

    m_dptTmp[0].x += x;
    m_dptTmp[0].y += y;

    //総移動数から変換

    pt2.x = (int)m_dptTmp[0].x;
    pt2.y = (int)m_dptTmp[0].y;

    calcMoveDstPos(&pt, pt2);

    pt += m_ptTmp[1];

    //------ オフセット位置、前回からの移動数

    m_pCurLayer->m_img.getOffset(&ptMov);

    ptMov.x = pt.x - ptMov.x;
    ptMov.y = pt.y - ptMov.y;

    if(ptMov.x == 0 && ptMov.y == 0) return;

    //----------- 移動

    m_ptTmp[2] += ptMov;

    //-------- オフセット変更
    //（+Alt でカレントレイヤのみ）

    //各レイヤ

    type = m_optToolMove;
    if(isDownKeyAlt()) type = 1;

    m_player->addOffset_group(m_pCurLayer, ptMov.x, ptMov.y, type);

    //選択範囲がある場合は範囲も移動

    if(m_rcfSel.flag)
    {
        m_pimgSel->setOffset(ptMov.x, ptMov.y, TRUE);
        calcAddFlagRect(&m_rcfSel, ptMov.x, ptMov.y);
    }

    //-------- 更新

    CANVASAREA->setTimer_updateMove();
}

//! 離し

void CDraw::onUp_imgMove()
{
    if(m_ptTmp[2].x || m_ptTmp[2].y)
    {
        //アンドゥ

        m_pundo->add_layerOffset(m_ptTmp[2].x, m_ptTmp[2].y);

        //※低品質で表示しているので元に戻す

        CANVASAREA->delTimer(CCanvasWinArea::TIMERID_UPDATE_MOVE);

        updateAll();
    }
}


//============================
// キャンバス移動
//============================
/*
    m_ptTmp[0]  : 開始時のスクロール位置
*/


//! 押し時

BOOL CDraw::onDown_canvasMove()
{
    m_nNowCtlNo = NOWCTL_CANVASMOVE;

    m_ptTmp[0] = m_ptScr;

    setCursorDrag(CURSOR_MOVE);

    return TRUE;
}

//! 移動時

void CDraw::onMove_canvasMove()
{
    AXPoint pt;

    pt.x = m_ptTmp[0].x + (int)(m_ptDownWinPos.x - m_ptNowWinPos.x);
    pt.y = m_ptTmp[0].y + (int)(m_ptDownWinPos.y - m_ptNowWinPos.y);

    if(pt != m_ptScr)
    {
        m_ptScr = pt;

        CANVASWIN->setScrollPos();
        CANVASAREA->setTimer_updateCanvas();
    }
}


//========================
// キャンバス回転
//========================
/*
    m_nTmp[0] : 現在の角度
    m_nTmp[1] : 前の角度
*/


//! 押し

BOOL CDraw::onDown_canvasRotate()
{
    m_nNowCtlNo = NOWCTL_CANVASROTATE;

    m_nTmp[0] = m_nCanvasRot;

    setScroll_reset();

    onMove_canvasRotate(TRUE, FALSE);

    setCursorDrag(CURSOR_ROTATE);

    return TRUE;
}

//! 移動
/*!
    ※キャンバス中央を中心に回転

    @param bCalcOnly 角度のみ計算し、実際の回転はしない（押し時はTRUE）
    @param b45       45度単位
*/

void CDraw::onMove_canvasRotate(BOOL bCalcOnly,BOOL b45)
{
    double x,y;
    int n,rot;

    x = m_ptNowWinPos.x - m_szCanvas.w * 0.5;
    y = m_ptNowWinPos.y - m_szCanvas.h * 0.5;

    //角度

    n = (int)(::atan2(y, x) * 18000.0 / M_PI);

    if(!bCalcOnly)
    {
        m_nTmp[0] += n - m_nTmp[1];

        if(m_nTmp[0] < -18000) m_nTmp[0] += 36000;
        else if(m_nTmp[0] > 18000) m_nTmp[0] -= 36000;

        rot = m_nTmp[0];
        if(b45) rot = rot / 4500 * 4500;

        if(rot != m_nCanvasRot)
        {
            changeRotate(rot, FALSE, FALSE, FALSE);

            CANVASAREA->setTimer_updateCanvas();
        }
    }

    m_nTmp[1] = n;
}


//==========================
// 上下ドラッグでの表示倍率変更
//==========================
/*
    m_nTmp[0]   : 開始時の倍率
*/


//! 押し時

BOOL CDraw::onDown_scaleDrag()
{
    m_nNowCtlNo = NOWCTL_SCALEDRAG;

    m_nTmp[0] = m_nCanvasScale;

    setScroll_reset();

    return TRUE;
}

//! 移動時

void CDraw::onMove_scaleDrag()
{
    int scale;

    scale = m_nTmp[0] + (int)(m_ptDownWinPos.y - m_ptNowWinPos.y);

    if(scale != m_nCanvasScale)
    {
        changeScale(scale, FALSE, FALSE, FALSE);

        CANVASAREA->setTimer_updateCanvas();
    }
}


//=============================
//左右ドラッグでのブラシサイズ変更
//=============================
/*
    m_ptTmp[0]   : 押し時の位置
    m_nTmp[0]    : ブラシサイズ(px)
    m_nTmp[1]    : 押し時のブラシサイズ
    m_rcsTmp[0]  : XOR描画範囲
    m_pTmp       : 対象ブラシアイテム
*/


//! 押し時
/*!
    @param bRegBrush1 登録ブラシ[1]のサイズを変更
*/

BOOL CDraw::onDown_dragBrushSize(BOOL bRegBrush1)
{
    CBrushTreeItem *p;

    m_nNowCtlNo = NOWCTL_DRAGBRUSHSIZE;

    getNowPoint(&m_ptTmp[0]);

    //対象ブラシ
    /* 登録ブラシ[1]が対象で、登録ブラシ[1]が現在選択されている場合は編集用データ */

    if(bRegBrush1)
    {
        p = BRUSHMAN->getRegBrush(0);

        if(!p || p == BRUSHMAN->getSelBrush())
            p = BRUSHMANEDIT;
    }
    else
        p = BRUSHMANEDIT;

    //

    m_pTmp = p;

    m_nTmp[0] = (int)(p->m_wSize * 0.1 * m_dViewParam[VIEWPARAM_SCALE] + 0.5);
    m_nTmp[1] = p->m_wSize;

    drawXorBrushCircle(FALSE);

    return TRUE;
}

//! 移動時

void CDraw::onMove_dragBrushSize()
{
    CBrushTreeItem *p = (CBrushTreeItem *)m_pTmp;
    int n;

    //サイズ

    n = m_nTmp[1] + (int)((m_ptNowWinPos.x - m_ptDownWinPos.x) * g_pconf->sDragBrushSizeW);

    if(n < CBrushTreeItem::BRUSHSIZE_MIN)
        n = CBrushTreeItem::BRUSHSIZE_MIN;
    else if(n > p->m_wSizeCtlMax)
        n = p->m_wSizeCtlMax;

    //

    if(n != p->m_wSize)
    {
        drawXorBrushCircle(TRUE);

        m_nTmp[0] = (int)(n * 0.1 * m_dViewParam[VIEWPARAM_SCALE] + 0.5);

        drawXorBrushCircle(FALSE);

        //登録ブラシ[1]が対象で、現在選択されていない場合は直接値を変更

        if(p == BRUSHMANEDIT)
            BRUSHWIN->setBrushSize(n);
        else
            p->m_wSize = n;
    }
}
