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
    CDraw [draw_sub] - 描画関連サブ処理
*/

#include <math.h>

#include "CDraw.h"

#include "CBrushManager.h"
#include "CBrushTreeItem.h"
#include "CCanvasWin.h"
#include "CUndo.h"
#include "CImage1.h"
#include "CImage8.h"
#include "CDevList.h"
#include "CLayerList.h"
#include "CPosBuf.h"
#include "CStatusBar.h"



//=============================
//
//=============================


//! ボタン押し時に Ctrl キーが押されていたか

BOOL CDraw::isDownKeyCtrl()
{
    return m_uDownBtt & CDevItem::FLAG_CTRL;
}

//! ボタン押し時に Shift キーが押されていたか

BOOL CDraw::isDownKeyShift()
{
    return m_uDownBtt & CDevItem::FLAG_SHIFT;
}

//! ボタン押し時に Alt キーが押されていたか

BOOL CDraw::isDownKeyAlt()
{
    return m_uDownBtt & CDevItem::FLAG_ALT;
}

//! ボタン押し時に Ctrl キーのみが押されていたか

BOOL CDraw::isDownKeyCtrlOnly()
{
    return ((m_uDownBtt & CDevItem::MASK_MOD) == CDevItem::FLAG_CTRL);
}

//! ボタン押し時に Shift キーのみが押されていたか

BOOL CDraw::isDownKeyShiftOnly()
{
    return ((m_uDownBtt & CDevItem::MASK_MOD) == CDevItem::FLAG_SHIFT);
}

//! ボタン押し時に Ctrl+Shift キーのみが押されていたか

BOOL CDraw::isDownKeyCtrlShiftOnly()
{
    return ((m_uDownBtt & CDevItem::MASK_MOD) == (CDevItem::FLAG_CTRL|CDevItem::FLAG_SHIFT));
}

//! 指定ボタンの動作がメイン操作か

BOOL CDraw::isBttActionMain(UINT btt)
{
    return ((btt >> CDevItem::SHIFT_ACTION) == CDevItem::ACTION_MAIN);
}

//! 塗りつぶし関連の描画時、ブラシの濃度を使う場合に描画濃度取得

int CDraw::getDrawValBrush(int val,BOOL bUseBrush)
{
    if(!bUseBrush)
        return val;
    else
    {
        //選択ブラシがない場合はツールの濃度を使う

        if(BRUSHMAN->getSelBrush())
            return BRUSHMANEDIT->m_btVal;
        else
            return val;
    }
}

//! AXPoint に現在のウィンドウ位置取得

void CDraw::getNowPoint(AXPoint *ppt)
{
    ppt->x = (int)m_ptNowWinPos.x;
    ppt->y = (int)m_ptNowWinPos.y;
}

//! 描画範囲取得（選択範囲があればその範囲。なければカレントレイヤの範囲）

void CDraw::getDrawRectSel(AXRect *prc)
{
    if(isSelExist())
        m_rcfSel.toRect(prc);
    else
        m_pCurLayer->m_img.getEnableDrawRectPixel(prc);
}

//! ステータスバーに2点間の情報セット

void CDraw::setMainStatus_line(const AXPoint &pt1,const AXPoint &pt2)
{
    double x1,y1,x2,y2;
    int angle;

    calcWindowToImage(&x1, &y1, pt1.x, pt1.y);
    calcWindowToImage(&x2, &y2, pt2.x, pt2.y);

    x1 = x2 - x1;
    y1 = y2 - y1;

    //角度(1.0=100)

    angle = (int)(::atan2(y1, x1) * -18000 / M_PI);
    if(angle < 0) angle += 36000;

    STATUSBAR->setLabel_rule(::sqrt(x1 * x1 + y1 * y1), angle);
}

//! 1bitイメージ作成

BOOL CDraw::create1bitWork(int w,int h)
{
    free1bitWork();

    m_pimg1Work = new CImage1;

    if(!m_pimg1Work->create(w, h))
    {
        free1bitWork();
        return FALSE;
    }

    return TRUE;
}

//! 1bitイメージ解放

void CDraw::free1bitWork()
{
    if(m_pimg1Work)
    {
        delete m_pimg1Work;
        m_pimg1Work = NULL;
    }
}


//=============================
// アンドゥ用
//=============================


//! 描画用、アンドゥ準備

void CDraw::beginDrawUndo()
{
    m_datDraw.fUndoErr      = 0;
    m_datDraw.rcfDraw.flag  = FALSE;

    //※配列を同じに出来なかった場合はアンドゥなし

    m_pimgUndo->createSame(getCurLayerImg());

    getCurLayerImg().getInfo(&m_infoUndo);
}

//! 描画用、アンドゥ終了
/*!
    @param rcs 更新イメージ範囲（キャンバス範囲内調整済み）
*/

void CDraw::endDrawUndo(const AXRectSize &rcs)
{
    //エラー

    if(m_datDraw.fUndoErr)
        AXError("! undo err - not enough memory\n");

    if(m_datDraw.fUndoErr & 2)
        //アンドゥが確保できなかった場合はクリア
        m_pundo->deleteAllDat();

    //アンドゥ成功時

    if(m_datDraw.fUndoErr == 0)
    {
        //空タイルを解放
        getCurLayerImg().freeEmptyTileFromUndo();

        //UNDO追加
        m_pundo->add_curUndoImage(m_infoUndo, rcs);
    }

    //タイル削除（配列は残す）

    m_pimgUndo->freeAllTile();
}


//=============================
// アンドゥ・更新の共通処理
//=============================


//! [自由線/連続直線/集中線] 描画時の共通処理（描画した後）
/*!
    @param bTimer TRUEでタイマー更新セット。FALSEで直接更新。
*/
/*
    m_datDraw.rcfDraw : 描画された範囲
    m_rcfDrawUp       : 全体の描画された範囲
*/

void CDraw::commonFreeDraw(BOOL bTimer)
{
    AXRectSize rcsUp;

    if(m_datDraw.rcfDraw.flag)
    {
        //描画範囲追加

        calcUnionFlagRect(&m_rcfDrawUp, m_datDraw.rcfDraw);

        //更新（キャンバス範囲内）

        if(calcImgRectInCanvas(&rcsUp, m_datDraw.rcfDraw))
        {
            if(bTimer)
                CANVASAREA->setTimer_updateRect(rcsUp);
            else
                updateRect(rcsUp);
        }
    }
}

//! 自由線描画終了時など、複数描画後の共通処理
/*!
    m_rcfDrawUp に全体の更新範囲を入れておく。
*/

void CDraw::commonDrawEnd()
{
    AXRectSize rcs;

    CANVASAREA->clearTimerUpdate(CCanvasWinArea::TIMERID_UPDATERECT);

    if(m_rcfDrawUp.flag)
    {
        //全体の描画範囲をキャンバス範囲内に

        if(!calcImgRectInCanvas(&rcs, m_rcfDrawUp))
            rcs.x = -1;

        //UNDO

        endDrawUndo(rcs);

        //更新

        updateAfterDraw(rcs);
    }
}

//! 直線描画時などの描画後共通処理

void CDraw::commonAfterDraw()
{
    AXRectSize rcs;

    if(m_datDraw.rcfDraw.flag)
    {
        //キャンバス範囲内に

        if(!calcImgRectInCanvas(&rcs, m_datDraw.rcfDraw))
            rcs.x = -1;

        //UNDO

        endDrawUndo(rcs);

        //更新

        if(rcs.x != -1)
        {
            updateRect(rcs);
            updateAfterDraw(rcs);
        }
    }
}


//==============================
// 描画前のセット
//==============================


//! 描画前の各情報セット
/*!
    @param toolno    -1 でマスクなどのみセット
    @param brushno   ブラシ描画時、登録ブラシ番号（-1で選択ブラシ）
    @param bDrawFree TRUEで自由線、FALSEでそのほか
*/

void CDraw::setBeforeDraw(int toolno,int brushno,BOOL bDrawFree)
{
    DRAWLAYERINFO *p = &m_datDraw;
    int n;

    //描画時の点セット関数

    p->funcDrawPixel = &CLayerImg::setPixelDraw;

    //デフォルトでオプションのテクスチャ

    p->pTexture = (m_pimg8Tex->isExist())? m_pimg8Tex: NULL;

    //レイヤマスク

    p->pimgMask = m_player->getMaskLayerImg(m_pCurLayer);

    //選択範囲イメージ

    p->pimgSel = (m_rcfSel.flag && m_pimgSel->isExist())? m_pimgSel: NULL;

    //----------------

    if(toolno == -1) return;

    switch(toolno)
    {
        //ブラシ描画
        case TOOL_BRUSH:
            setBeforeDraw_brush(brushno, bDrawFree);
            break;
        //ドット（テクスチャ無効）
        case TOOL_DOT:
            p->pTexture     = NULL;
            p->nColSetFunc  = CLayerImg::COLSETF_OVERWRITE;

            n = m_optToolDot & TOOLDOT_MASK_SIZE;

            if(n == 0)
                p->funcDrawPixel = &CLayerImg::setPixelDraw;
            else if(n == 1)
                p->funcDrawPixel = &CLayerImg::setPixel_dot2px;
            else
                p->funcDrawPixel = &CLayerImg::setPixel_dot3px;

            if(bDrawFree)
                m_nTmp[0] = m_optToolDot & TOOLDOT_FLAG_THIN;
            break;
        //図形塗りつぶし・消し
        case TOOL_POLYPAINT:
        case TOOL_POLYERASE:
            m_optToolTmp = (toolno == TOOL_POLYPAINT)? m_optToolPolyPaint: m_optToolPolyErase;

            p->nColSetFunc = (toolno == TOOL_POLYERASE)? CLayerImg::COLSETF_ERASE
                                : CLayerImg::COLSETF_BLEND + ((m_optToolTmp >> TOOLPOLY_PIX_SHIFT) & TOOLPOLY_PIX_MASK);
            break;
        //塗りつぶし
        case TOOL_PAINT:
            p->nColSetFunc = (m_optToolPaint >> TOOLPAINT_PIX_SHIFT) & TOOLPAINT_PIX_MASK;
            break;
        //不透明範囲消し（テクスチャ無効）
        case TOOL_PAINTERASE:
            p->nColSetFunc  = CLayerImg::COLSETF_ERASE;
            p->pTexture     = NULL;
            break;
        //閉領域塗りつぶし
        case TOOL_PAINTCLOSE:
            p->nColSetFunc = (m_optToolPaintClose >> TOOLPAINTC_PIX_SHIFT) & TOOLPAINTC_PIX_MASK;
            break;
        //グラデーション
        case TOOL_GRAD:
            p->nColSetFunc = (m_optToolGrad >> TOOLGRAD_PIX_SHIFT) & TOOLGRAD_PIX_MASK;
            break;
        //テキスト
        case TOOL_TEXT:
            p->nColSetFunc = CLayerImg::COLSETF_BLEND;
            break;
        //矩形編集/選択
        case TOOL_BOXEDIT:
        case TOOL_SEL:
            p->nColSetFunc = CLayerImg::COLSETF_OVERWRITE;
            setBeforeDraw_clearMask();
            break;
    }
}

//! 選択・レイヤマスク・テクスチャクリア

void CDraw::setBeforeDraw_clearMask()
{
    m_datDraw.pimgMask  = NULL;
    m_datDraw.pimgSel   = NULL;
    m_datDraw.pTexture  = NULL;
}

//! ブラシ描画用データセット
/*!
    自由線時、m_optToolTmp の 0bit が ON で、筆圧最大固定。
*/

void CDraw::setBeforeDraw_brush(int brushno,BOOL bFree)
{
    DRAWLAYERINFO *p = &m_datDraw;
    BRUSHDRAWPARAM *pb;
    CBrushTreeItem *pitem = NULL;

    pb = &m_datDraw.bparam;

    //------- 使用するブラシ

    //登録ブラシ
    if(brushno != -1)
        pitem = BRUSHMAN->getRegBrush(brushno);

    //編集用

    if(!pitem)
        pitem = BRUSHMAN->getEdit();

    //-------

    p->nColSetFunc = pitem->m_btPixType;
    p->pBrush = (pitem->m_pimgBrush)? pitem->m_pimgBrush: m_pimg8BrushDef;

    if(pitem->m_strTexImg[0] == '?')
        p->pTexture = (m_pimg8Tex->isExist())? m_pimg8Tex: NULL;
    else
        p->pTexture = pitem->m_pimgTex;

    //------- ブラシ用

    pb->dMaxSize    = pitem->m_wSize * 0.1;
    pb->dMaxVal     = pitem->m_btVal;
    pb->dSizeMin    = pitem->m_wMinSize * 0.001;
    pb->dValMin     = pitem->m_wMinVal * 0.001;
    pb->nHoseiType  = pitem->m_btHoseiType;
    pb->nHoseiStr   = (pitem->m_btHoseiType == 0)? 0: pitem->m_btHoseiStr + 1;
    pb->bAnti       = pitem->m_btFlag & CBrushTreeItem::BFLAG_ANTIALIAS;
    pb->bCurve      = pitem->m_btFlag & CBrushTreeItem::BFLAG_CURVE;

    //筆圧（100=変化なし の場合は pow 計算をしないように）

    pb->btPressFlag = 0;

    if(pitem->m_wPressSize != 100)
    {
        pb->btPressFlag |= 1;
        pb->dGammaSize = pitem->m_wPressSize * 0.01;
    }

    if(pitem->m_wPressVal != 100)
    {
        pb->btPressFlag |= 2;
        pb->dGammaVal = pitem->m_wPressVal * 0.01;
    }

    //間隔（ぼかし時は濃度で調整）

    if(p->nColSetFunc == CLayerImg::COLSETF_BLUR)
        pb->dInterval = (255 - pitem->m_btVal) / 255.0 + 0.02;
    else
        pb->dInterval = pitem->m_wInterval * 0.01;

    //ランダム

    pb->btRandFlag = (pitem->m_wRan_sizeMin != 1000) | ((pitem->m_wRan_posLen != 0) << 1) |
                ((pitem->m_btFlag & CBrushTreeItem::BFLAG_RANDPOS_BOX)? 4: 0);

    if(pb->btRandFlag & 1)
        pb->dRandSizeMin = pitem->m_wRan_sizeMin * 0.001;

    if(pb->btRandFlag & 2)
        pb->dRandPosLen = pitem->m_wRan_posLen * 0.01;

    //画像回転

    pb->btRotFlag = (pitem->m_btRotType == 1)? 1: 0;
    if(pitem->m_wRotRandom) pb->btRotFlag |= 2;

    pb->nRotAngle  = pitem->m_wRotAngle;
    pb->nRotRandom = pitem->m_wRotRandom;

    //--------

    if(bFree)
    {
        //筆圧最大固定

        if(m_optToolTmp & 1)
        {
            pb->dSizeMin = 1.0;
            pb->dValMin  = 1.0;
        }

        setBeforeDraw_drawBrushFree();
    }
    else
    {
        pb->dT = 0;
        pb->dPressSt = pb->dPressEd = 1.0;
    }
}

//! ブラシ自由線描画時の共通処理

void CDraw::setBeforeDraw_drawBrushFree()
{
    DRAWPOINT pt;
    BRUSHDRAWPARAM *p = &m_datDraw.bparam;

    //位置

    getDrawPoint(&pt, FALSE);

    m_pPosBuf->setPos(pt.x, pt.y, pt.press, p->nHoseiStr + 1);

    //作業用

    p->dBkX     = pt.x;
    p->dBkY     = pt.y;
    p->dBkPress = pt.press;
    p->dT       = 0;

    //曲線時

    if(p->bCurve)
        CLayerImg::setCurvePos(pt.x, pt.y, pt.press);
}

//! 閉領域：ブラシ描画時

void CDraw::setBeforeDraw_paintCloseBrush()
{
    BRUSHDRAWPARAM *p = &m_datDraw.bparam;

    setBeforeDraw_brush(-1, TRUE);
    setBeforeDraw_clearMask();

    m_datDraw.funcDrawPixel = &CLayerImg::setPixel_subDraw;
    m_datDraw.nColSetFunc   = CLayerImg::COLSETSUB_SET;

    p->dMaxVal      = 255;
    p->dValMin      = 1.0;
    p->dInterval    = 0.2;
    p->bAnti        = FALSE;
    p->nHoseiType   = 0;
    p->nHoseiStr    = 0;

    setBeforeDraw_drawBrushFree();
}

//! 選択範囲：ブラシ描画時

void CDraw::setBeforeDraw_selBrush()
{
    BRUSHDRAWPARAM *p = &m_datDraw.bparam;

    setBeforeDraw_brush(-1, TRUE);
    setBeforeDraw_clearMask();

    //+Ctrlまたは消しゴムブラシの場合は範囲削除

    m_datDraw.funcDrawPixel = &CLayerImg::setPixel_subDraw;
    m_datDraw.nColSetFunc   = (isDownKeyCtrl() || m_datDraw.nColSetFunc == CLayerImg::COLSETF_ERASE)?
                                CLayerImg::COLSETSUB_CLEAR: CLayerImg::COLSETSUB_SET;

    p->dMaxVal      = 255;
    p->dValMin      = 1.0;
    p->dInterval    = 0.2;
    p->bAnti        = FALSE;
    p->nHoseiType   = 0;
    p->nHoseiStr    = 0;

    setBeforeDraw_drawBrushFree();
}

//! 選択範囲：ドラッグ移動時

void CDraw::setBeforeDraw_selDragMove(BOOL bOverwrite)
{
    m_datDraw.pimgMask      = m_player->getMaskLayerImg(m_pCurLayer);
    m_datDraw.pimgSel       = NULL;
    m_datDraw.pTexture      = NULL;
    m_datDraw.nColSetFunc   = (bOverwrite)? CLayerImg::COLSETF_OVERWRITE: CLayerImg::COLSETF_BLEND;
}

//! フィルタ用

void CDraw::setBeforeDraw_filter(BOOL bPrev)
{
    m_datDraw.pTexture = NULL;

    if(bPrev)
    {
        m_datDraw.pimgMask      = NULL;
        m_datDraw.pimgSel       = NULL;
        m_datDraw.funcDrawPixel = &CLayerImg::setPixel_create2;
    }
    else
    {
        m_datDraw.pimgMask      = m_player->getMaskLayerImg(m_pCurLayer);
        m_datDraw.pimgSel       = (isSelExist())? m_pimgSel: NULL;
        m_datDraw.funcDrawPixel = &CLayerImg::setPixelDraw;
        m_datDraw.nColSetFunc   = CLayerImg::COLSETF_OVERWRITE;
    }
}

//! フィルタ、フラッシュ描画時用

void CDraw::setBeforeDraw_filterDrawFlash()
{
    BRUSHDRAWPARAM *p = &m_datDraw.bparam;

    //他はフィルタ処理中にセット

    m_datDraw.pBrush = m_pimg8BrushDef;

    p->dMaxVal      = 255;
    p->dSizeMin     = 0;
    p->dValMin      = 0;
    p->dInterval    = 0.3;
    p->nRotAngle    = 0;
    p->btRandFlag   = 0;
    p->btRotFlag    = 0;
}


//==============================
// 描画位置取得
//==============================


//! イメージ位置データ取得
/*!
    @param bRuleInt  定規時、x,y を int 値で計算（ドット描画時）
*/

void CDraw::getDrawPoint(DRAWPOINT *pdst,BOOL bEnableRule,BOOL bRuleInt)
{
    double x,y,xx,yy,len,ad;
    int a;

    //イメージ位置に変換

    calcWindowToImage(&x, &y, m_ptNowWinPos.x, m_ptNowWinPos.y);

    //定規時の位置補正

    if(bEnableRule && m_ruleType)
    {
        if(bRuleInt)
        {
            x = ::floor(x);
            y = ::floor(y);
        }

        switch(m_ruleType)
        {
            //======== 平行線・格子線・集中線（角度：360度 = 1024）
            case RULETYPE_LINE:
            case RULETYPE_GRIDLINE:
            case RULETYPE_CONCLINE:
                xx = x - m_rule.ptDown.x;
                yy = y - m_rule.ptDown.y;

                len = ::sqrt(xx * xx + yy * yy);            //押し位置からの距離
                a   = (int)(::atan2(yy, xx) * 512 / M_PI);  //押し位置からの角度(int)

                ad = (m_ruleType == RULETYPE_LINE)? m_rule.dAngleLine: m_rule.dTmp;

                /* [格子線]時は長さが一定になるまでそのまま。
                   長さが一定まで来たら、角度を計算 */

                if(m_ruleType == RULETYPE_GRIDLINE && !m_rule.nTmp)
                {
                    x = m_rule.ptDown.x;
                    y = m_rule.ptDown.y;

                    if(len >= 2.0)
                    {
                        //45〜135度、245〜335度の範囲は+90度角度

                        a = (a - ((int)(m_rule.dAngleGrid * 512 / M_PI)) + 1024) & 1023;

                        if((a > 128 && a < 128 + 256) || (a > 128 + 256 * 2 && a < 1024 - 128))
                            m_rule.dTmp += M_PI / 2;

                        m_rule.nTmp = TRUE;
                    }

                    break;
                }

                //-90〜+90 度の範囲ならそのまま、反対側なら反転角度

                a = (a - ((int)(ad * 512 / M_PI)) + 1024) & 1023;
                if(a > 256 && a < 1024 - 256) ad += M_PI;

                x = len * ::cos(ad) + m_rule.ptDown.x;
                y = len * ::sin(ad) + m_rule.ptDown.y;
                break;
            //========= 正円
            case RULETYPE_CIRCLE:
                xx = x - m_rule.ptCtCir.x;
                yy = y - m_rule.ptCtCir.y;
                ad = ::atan2(yy, xx);

                x = m_rule.dTmp * ::cos(ad) + m_rule.ptCtCir.x;
                y = m_rule.dTmp * ::sin(ad) + m_rule.ptCtCir.y;
                break;
            //========= 楕円
            case RULETYPE_ELLIPSE:
                //角度

                xx = x - m_rule.ptCtEll.x; if(m_rule.bEllRevH) xx = -xx;
                yy = y - m_rule.ptCtEll.y;

                x = (xx * m_rule.dEllTmp[2] - yy * m_rule.dEllTmp[3]) * m_rule.dEllHV[1];
                y =  xx * m_rule.dEllTmp[3] + yy * m_rule.dEllTmp[2];

                ad = ::atan2(y, x);

                //楕円

                xx = m_rule.dTmp * m_rule.dEllHV[0] * ::cos(ad);
                yy = m_rule.dTmp * ::sin(ad);

                x = xx * m_rule.dEllTmp[0] - yy * m_rule.dEllTmp[1];
                y = xx * m_rule.dEllTmp[1] + yy * m_rule.dEllTmp[0] + m_rule.ptCtEll.y;

                if(m_rule.bEllRevH) x = -x;
                x += m_rule.ptCtEll.x;
                break;
        }
    }

    pdst->x = x;
    pdst->y = y;
    pdst->press = m_ptNowWinPos.press;
}

//! ドット描画用、イメージ位置取得

void CDraw::getDrawPointInt(AXPoint *ppt,BOOL bEnableRule)
{
    DRAWPOINT pt;

    getDrawPoint(&pt, bEnableRule, TRUE);

    if(bEnableRule && m_ruleType)
    {
        //定規時は繰り上げ

        ppt->x = (int)(pt.x + 0.5);
        ppt->y = (int)(pt.y + 0.5);
    }
    else
    {
        ppt->x = (int)pt.x;
        ppt->y = (int)pt.y;
    }
}
