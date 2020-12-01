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
    CDraw [draw] - 描画操作関連
*/

#include <math.h>

#include "CDraw.h"

#include "CMainWin.h"
#include "CCanvasWin.h"
#include "CPrevWin.h"
#include "CConfig.h"
#include "CDevList.h"
#include "CKeyDat.h"
#include "CSplinePos.h"
#include "CBrushManager.h"
#include "CLayerImg.h"
#include "defGlobal.h"

#include "AXKey.h"



//! 押し
/*!
    @param btt  CDevItem のボタン＋動作値
    @return グラブするかどうか
*/

BOOL CDraw::onDown(const DRAWPOINT &pt,UINT btt)
{
    m_ptDownWinPos = pt;
    m_ptNowWinPos  = pt;
    m_ptBkWinPos   = pt;
    m_uDownBtt     = btt;

    //操作中

    if(m_nNowCtlNo != NOWCTL_NONE)
    {
        //メイン操作ボタン

        if(isBttActionMain(btt))
        {
            switch(m_nNowCtlNo)
            {
                //連続直線/集中線/多角形の2点目以降
                case NOWCTL_XORLINE2:
                    onDown2_xorLine2();
                    break;
            }
        }

        return FALSE;
    }

    //---------- 左ボタン＋キー処理

    if((btt & CDevItem::MASK_BUTTON) == 0)
    {
        //+スペースキー

        if(CANVASWIN->isDownKeySpace())
        {
            if((btt & CDevItem::MASK_MOD) == CDevItem::FLAG_CTRL)
                //+Ctrl : キャンバス回転
                return onDown_canvasRotate();
            else if((btt & CDevItem::MASK_MOD) == CDevItem::FLAG_SHIFT)
                //Shift+上下ドラッグ : 表示倍率変更
                return onDown_scaleDrag();
            else
                //キャンバス移動
                return onDown_canvasMove();
        }
    }

    //------------ コマンド

    int cmdid;
    UINT key;

    cmdid = btt >> CDevItem::SHIFT_ACTION;

    if(cmdid == CDevItem::ACTION_MAIN)
    {
        //メイン操作（キー＋操作判定あり）

        key = CANVASWIN->getLastDownKey();

        if(key)
        {
            cmdid = g_pconf->pKey->getCmdDraw(key);

            if(cmdid != -1)
                return onDown_keycmd(cmdid);
        }

        return onDown_tool(m_toolno, -1);
    }
    else
        //他コマンド
        return onDown_keycmd(cmdid);

    return FALSE;
}

//! 左ダブルクリック
/*!
    @return TRUE でダブルクリックとして処理し、グラブ解除。FALSE で通常の押しとして処理する。
*/

BOOL CDraw::onLDblClk()
{
    //ベジェは対象外

    if(m_nNowCtlNo == NOWCTL_BEZIER) return FALSE;

    //[連続直線/集中線/多角形/スプライン] の場合、終了

    if(onCancel())
    {
        m_nNowCtlNo = NOWCTL_NONE;
        return TRUE;
    }

    return FALSE;
}

//! 移動

void CDraw::onMove(const DRAWPOINT &pt,BOOL bCtrl,BOOL bShift)
{
    //プレビューウィンドウ、ルーペ時

    if(g_pconf->isPrevWinLoupe() && g_pconf->isViewPrevWin())
    {
        PREVWIN->moveCanvasPos(pt.x, pt.y);
    }

    //操作中でない時

    if(m_nNowCtlNo == NOWCTL_NONE)
    {
        /* カーソルがメインウィンドウ上を移動した時、
           メインウィンドウがアクティブになっていない場合は、アクティブにする */

        if(g_pconf->uFlags & CConfig::FLAG_AUTOACTIVE)
        {
            if(!MAINWIN->isFocused())
                MAINWIN->setActive();
        }

        return;
    }

    //位置

    m_ptNowWinPos = pt;

    //--------------

    switch(m_nNowCtlNo)
    {
        //ブラシ、自由線
        case NOWCTL_BRUSH_FREE:
            onMove_brush_free();
            break;
        //ドット、自由線
        case NOWCTL_DOT_FREE:
            onMove_dot_free();
            break;
        //ベジェ曲線
        case NOWCTL_BEZIER:
            onMove_bezier();
            break;
        //閉領域、ブラシ描画
        case NOWCTL_PAINTCLOSE_BRUSH:
            onMove_paintclose_brush();
            break;
        //選択範囲、ブラシ描画
        case NOWCTL_SEL_BRUSH:
            onMove_sel_brush();
            break;

        //XOR直線
        case NOWCTL_XORLINE:
        case NOWCTL_XORLINE2:
            onMove_xorLine(bShift);
            break;
        //XOR四角
        case NOWCTL_XORBOXWIN:
            onMove_xorBoxWin(bShift);
            break;
        //XOR四角（イメージ）
        case NOWCTL_XORBOXIMG:
            onMove_xorBoxImg(bShift);
            break;
        //XOR円
        case NOWCTL_XORCIRCLE:
            onMove_xorCircle(bShift);
            break;
        //XOR投げ縄
        case NOWCTL_XORLASSO:
            onMove_xorLasso();
            break;

        //イメージ移動
        case NOWCTL_IMGMOVE:
            onMove_imgMove(bShift, bCtrl);
            break;
        //選択範囲ドラッグ移動
        case NOWCTL_SEL_DRAGMOVE:
            onMove_sel_dragMove();
            break;

        //キャンバス移動
        case NOWCTL_CANVASMOVE:
            onMove_canvasMove();
            break;
        //キャンバス回転
        case NOWCTL_CANVASROTATE:
            onMove_canvasRotate(FALSE, bShift);
            break;
        //ドラッグでのブラシサイズ変更
        case NOWCTL_DRAGBRUSHSIZE:
            onMove_dragBrushSize();
            break;
        //ドラッグでの表示倍率変更
        case NOWCTL_SCALEDRAG:
            onMove_scaleDrag();
            break;
    }

    //位置保存

    m_ptBkWinPos = m_ptNowWinPos;
}

//! 離し
/*!
    @param bttno ボタン番号
    @return グラブ解除するか
*/

BOOL CDraw::onUp(const DRAWPOINT &pt,int bttno)
{
    BOOL bRelease = TRUE;

    if(m_nNowCtlNo == NOWCTL_NONE) return FALSE;

    m_ptNowWinPos = pt;

    //右ボタンキャンセル動作

    if(bttno == 3)
    {
        if(onCancel()) goto END;
    }

    //押し時と同じボタンか

    if(bttno - 1 != (m_uDownBtt & CDevItem::MASK_BUTTON))
        return FALSE;

    //---------------

    switch(m_nNowCtlNo)
    {
        //ブラシ、自由線
        case NOWCTL_BRUSH_FREE:
            onUp_brush_free();
            break;
        //ドット、自由線
        case NOWCTL_DOT_FREE:
            onUp_dot_free();
            break;
        //ベジェ曲線
        case NOWCTL_BEZIER:
            bRelease = onUp_bezier();
            break;
        //スプライン曲線
        case NOWCTL_SPLINE:
            bRelease = onUp_spline();
            break;
        //閉領域、ブラシ描画
        case NOWCTL_PAINTCLOSE_BRUSH:
            onUp_paintclose_brush();
            break;
        //選択範囲、ブラシ描画
        case NOWCTL_SEL_BRUSH:
            onUp_sel_brush();
            break;

        //XOR直線
        case NOWCTL_XORLINE:
            bRelease = onUp_xorLine();
            break;
        //XOR直線2
        case NOWCTL_XORLINE2:
            bRelease = FALSE;
            break;
        //XOR四角
        case NOWCTL_XORBOXWIN:
            onUp_xorBoxWin();
            break;
        //XOR四角（イメージ）
        case NOWCTL_XORBOXIMG:
            onUp_xorBoxImg();
            break;
        //XOR円
        case NOWCTL_XORCIRCLE:
            onUp_xorCircle();
            break;
        //XOR投げ縄
        case NOWCTL_XORLASSO:
            onUp_xorLasso();
            break;
        //定規中央位置
        case NOWCTL_RULEPOS:
            drawXorRulePos(m_ptTmp[0]);
            break;

        //イメージ移動
        case NOWCTL_IMGMOVE:
            onUp_imgMove();
            break;
        //選択範囲ドラッグ移動
        case NOWCTL_SEL_DRAGMOVE:
            onUp_sel_dragMove();
            break;
        //離し後に実行
        case NOWCTL_FUNCUP:
            onUp_funcUp();
            break;

        //キャンバス移動/キャンバス回転/ドラッグでの表示倍率変更
        case NOWCTL_CANVASMOVE:
        case NOWCTL_CANVASROTATE:
        case NOWCTL_SCALEDRAG:
            CANVASAREA->clearTimer_updateCanvas();
            restoreCursor();
            break;
        //ドラッグでのブラシサイズ変更
        case NOWCTL_DRAGBRUSHSIZE:
            drawXorBrushCircle(TRUE);
            break;
    }

    //------- 操作終了

END:
    if(bRelease)
        m_nNowCtlNo = NOWCTL_NONE;

    return bRelease;
}

//! 右ボタン離し/ダブルクリック 時のキャンセル処理
/*!
    @return TRUEで操作終了
*/

BOOL CDraw::onCancel()
{
    switch(m_nNowCtlNo)
    {
        //連続直線/集中線/多角形
        case NOWCTL_XORLINE2:
            return onCancel_xorLine2();
        //ベジェ曲線
        case NOWCTL_BEZIER:
            return onCancel_bezier();
        //スプライン曲線
        case NOWCTL_SPLINE:
            return onCancel_spline();
    }

    return FALSE;
}

//! ダイアログ中に左クリックされた時

void CDraw::onLDownInDlg(int x,int y)
{
    switch(m_nNowCtlNo)
    {
        //テキスト
        case NOWCTL_TEXT:
            onDown_textInDlg(x, y);
            break;
        //フィルタ、キャンバスプレビュー
        case NOWCTL_FILTER_CANVASPREV:
            onDown_filterPrevInDlg(x, y);
            break;
    }
}

//! 操作中のキー操作
/*!
    @return TRUEでグラブ解放
*/

BOOL CDraw::onKey_inCtrl(UINT uKey)
{
    BOOL bRelease = FALSE;
    int no,after;

    no    = m_nNowCtlNo;
    after = m_nAfterCtlType;

    switch(uKey)
    {
        //ENTER
        case KEY_ENTER:
        case KEY_NUM_ENTER:
            //[連続直線/集中線/多角形/ベジェ/スプライン] 描画またはキャンセル
            if(no == NOWCTL_XORLINE2 || no == NOWCTL_BEZIER || no == NOWCTL_SPLINE)
                bRelease = onCancel();
            break;

        //ESC
        case KEY_ESCAPE:
            //[多角形/ベジェ] キャンセル
            if(no == NOWCTL_XORLINE2 && after == AFTERLINE2_FILLPOLY)
                bRelease = onCancel_fillPoly(FALSE);
            else if(no == NOWCTL_BEZIER)
                bRelease = onCancel();
            break;

        //BACKSPACE
        case KEY_BACKSPACE:
            if(no == NOWCTL_XORLINE2 && after == AFTERLINE2_SUCC)
                //[連続直線] 始点と結び終了
                bRelease = onCancel_lineSuccConc(TRUE);
            else if(no == NOWCTL_SPLINE)
                //[スプライン] 一つ戻る
                bRelease = onKeyBack_spline();
            break;
    }

    if(bRelease)
    {
        m_nNowCtlNo = NOWCTL_NONE;
        return TRUE;
    }
    else
        return FALSE;
}


//=============================
// メイン操作
//=============================


//! 押し時（ツール動作）
/*!
    @param subno -1 で指定ツールのサブタイプ
*/

BOOL CDraw::onDown_tool(int tool,int subno)
{
    if(subno == -1)
        subno = m_toolSubNo[tool];

    //ブラシ/ドット描画

    if(tool == TOOL_BRUSH || tool == TOOL_DOT)
        return onDown_draw(tool, subno, -1);

    //他ツール

    switch(tool)
    {
        //図形塗り・消し
        case TOOL_POLYPAINT:
        case TOOL_POLYERASE:
            return onDown_drawPolygon(tool, subno);
        //塗りつぶし
        case TOOL_PAINT:
            return onDown_funcUp(AFTERUP_PAINT);
        //不透明消し
        case TOOL_PAINTERASE:
            return onDown_funcUp(AFTERUP_PAINTERASE);
        //閉領域
        case TOOL_PAINTCLOSE:
            return onDown_drawPaintClose(subno);
        //グラデーション
        case TOOL_GRAD:
            return onDown_xorLine(AFTERLINE_GRAD);
        //イメージ移動
        case TOOL_MOVE:
            return onDown_imgMove();
        //キャンバス回転
        case TOOL_ROTATE:
            return onDown_canvasRotate();
        //矩形編集
        case TOOL_BOXEDIT:
            return onDown_xorBoxImg(subno);
        //選択範囲
        case TOOL_SEL:
            return onDown_drawSel(subno);
        //テキスト
        case TOOL_TEXT:
            //後で実行する
            CANVASWIN->sendCommand(CCanvasWin::CMDID_TEXTDLG, 0, 0);
            return FALSE;
        //ものさし
        case TOOL_RULE:
            return onDown_xorLine(AFTERLINE_CALCRULE);
        //スポイト
        case TOOL_SPOIT:
            return onDown_spoit();
    }

    return FALSE;
}

//! 押し時（ブラシ/ドットペン描画）
/*!
    @param brushno   登録ブラシ番号（-1 で選択ブラシ）
    @param bPressMax 筆圧を最大で固定
*/

BOOL CDraw::onDown_draw(int toolno,int drawtype,int brushno,BOOL bPressMax)
{
    BOOL bBrush;

    m_nDrawToolNo    = toolno; //自由線以外の時に参照
    m_rcfDrawUp.flag = FALSE;

    switch(drawtype)
    {
        //自由線
        case DRAWTYPE_FREE:
            if(m_ruleType && isDownKeyCtrl())
            {
                //+Ctrl で定規ONの場合は定規設定

                if(m_ruleType == RULETYPE_LINE)
                    return onDown_xorLine(AFTERLINE_RULE_LINE);
                else if(m_ruleType == RULETYPE_GRIDLINE)
                    return onDown_xorLine(AFTERLINE_RULE_GRID);
                else if(m_ruleType == RULETYPE_CONCLINE)
                    return onDown_ruleCenterPos();
                else if(m_ruleType == RULETYPE_CIRCLE)
                    return onDown_ruleCenterPos();
                else if(m_ruleType == RULETYPE_ELLIPSE)
                    return onDown_xorCircle(AFTERCIR_RULE);
                else
                    return FALSE;
            }
            else if(toolno == TOOL_BRUSH && isDownKeyShift())
            {
                //ブラシ時 +Shift でブラシサイズ変更

                return onDown_dragBrushSize(FALSE);
            }
            else
            {
                //ブラシで描画するか(bBrush)
                /* ドット時は、ブラシが消しゴムタイプならブラシで描画 */

                if(toolno == TOOL_BRUSH)
                    bBrush = TRUE;
                else
                    bBrush = BRUSHMAN->isEditBrushErase();

                //

                onDown_rule(!bBrush);

                //

                if(!bBrush)
                    //ドット（+Shift で1px消しゴム）
                    onDown_dot_free(m_uDownBtt & CDevItem::FLAG_SHIFT);
                else
                    //ブラシ
                    onDown_brush_free(brushno, bPressMax);
            }
            return TRUE;
        //直線
        case DRAWTYPE_LINE:
            return onDown_xorLine(AFTERLINE_DRAWLINE);
        //四角枠
        case DRAWTYPE_BOX:
            return onDown_xorBoxWin(AFTERBOXWIN_DRAWBOX);
        //円枠
        case DRAWTYPE_CIRCLE:
            return onDown_xorCircle(AFTERCIR_DRAWCIRCLE);
        //連続直線
        case DRAWTYPE_SUCCLINE:
            return onDown_xorLine2(AFTERLINE2_SUCC);
        //集中線
        case DRAWTYPE_CONCLINE:
            return onDown_xorLine2(AFTERLINE2_CONC);
        //ベジェ曲線
        case DRAWTYPE_BEZIER:
            if(!create1bitWork(m_szCanvas.w, m_szCanvas.h))
                return FALSE;

            return onDown_xorLine(AFTERLINE_BEZIER);
        //スプライン曲線
        case DRAWTYPE_SPLINE:
            if(!m_pSplinePos->alloc()) return FALSE;

            if(!create1bitWork(m_szCanvas.w, m_szCanvas.h))
                return FALSE;

            return onDown_spline();
    }

    return FALSE;
}

//! 押し時（図形塗りつぶし）

BOOL CDraw::onDown_drawPolygon(int toolno,int type)
{
    setBeforeDraw(toolno);

    switch(type)
    {
        //四角
        case 0:
            return onDown_xorBoxWin(AFTERBOXWIN_DRAWFILL);
        //円
        case 1:
            return onDown_xorCircle(AFTERCIR_DRAWFILL);
        //多角形
        case 2:
            m_nAfterCtlType2 = AFTER2FILLPOLY_FILL;
            return onDown_xorLine2(AFTERLINE2_FILLPOLY);
        //投げ縄
        default:
            return onDown_xorLasso(AFTERLASSO_FILL);
    }

    return FALSE;
}

//! 押し時（閉領域塗りつぶし）

BOOL CDraw::onDown_drawPaintClose(int type)
{
    switch(type)
    {
        //ブラシ
        case 0:
            return onDown_paintclose_brush();
        //四角
        case 1:
            return onDown_xorBoxWin(AFTERBOXWIN_PAINTCLOSE);
        //多角形
        case 2:
            m_nAfterCtlType2 = AFTER2FILLPOLY_PAINTCLOSE;
            return onDown_xorLine2(AFTERLINE2_FILLPOLY);
        //投げ縄
        default:
            return onDown_xorLasso(AFTERLASSO_PAINTCLOSE);
    }

    return FALSE;
}

//! 押し時（選択範囲）

BOOL CDraw::onDown_drawSel(int type)
{
    AXPoint pt;

    //+Shift で範囲内が押されたら、ドラッグ移動
    //(+Shift+Ctrl で上書き移動）

    if(isDownKeyShift())
    {
        getDrawPointInt(&pt, FALSE);

        if(pt.x >= 0 && pt.y >= 0 && pt.x < m_nImgW && pt.y < m_nImgH &&
           m_pimgSel->isExist() && m_pimgSel->getPixel(pt.x, pt.y))
        {
            return onDown_sel_dragMove();
        }
    }

    //通常（範囲追加）

    switch(type)
    {
        //ブラシ
        case 0:
            return onDown_sel_brush();
        //四角
        case 1:
            return onDown_xorBoxWin(AFTERBOXWIN_DRAWSEL);
        //多角形
        case 2:
            m_nAfterCtlType2 = AFTER2FILLPOLY_SEL;
            return onDown_xorLine2(AFTERLINE2_FILLPOLY);
        //投げ縄
        case 3:
            return onDown_xorLasso(AFTERLASSO_SEL);
    }

    return FALSE;
}

//! キー＋操作時、またはデバイス設定の動作

BOOL CDraw::onDown_keycmd(int cmdid)
{
    if(cmdid >= 2000 && cmdid < 3000)
    {
        if(cmdid < 2100)
            //ツール動作 (2000-)
            return onDown_tool(cmdid - 2000, -1);
        else if(cmdid < 2200)
        {
            //描画タイプ（ブラシ/ドット時。それ以外は通常動作）

            if(m_toolno == TOOL_BRUSH || m_toolno == TOOL_DOT)
                return onDown_tool(m_toolno, cmdid - 2100);
            else
                return onDown_tool(m_toolno, -1);
        }
        else if(cmdid < 2300)
        {
            //登録ブラシで描画 (2200-)

            if(cmdid < 2250)
                //筆圧あり
                return onDown_draw(TOOL_BRUSH, DRAWTYPE_FREE, cmdid - 2200, FALSE);
            else
                //筆圧最大
                return onDown_draw(TOOL_BRUSH, DRAWTYPE_FREE, cmdid - 2250, TRUE);
        }
        else
        {
            //他 (2300-)

            switch(cmdid - 2300)
            {
                case 0: return onDown_canvasMove();     //キャンバス移動
                case 1: return onDown_scaleDrag();      //表示倍率変更
                case 2: return onDown_dragBrushSize(FALSE);  //ブラシサイズ変更
                case 3: return onDown_dragBrushSize(TRUE);   //登録ブラシ[1]サイズ変更
            }
        }
    }
    else
        //実行コマンド
        MAINWIN->sendCommand(cmdid, 0, 0);

    return FALSE;
}


//===========================
// サブ
//===========================


//! 定規時、押し時の計算

void CDraw::onDown_rule(BOOL bDot)
{
    DRAWPOINT pt;
    double xx,yy,x,y;

    //押し時の位置

    getDrawPoint(&pt, FALSE, FALSE);

    m_rule.ptDown.x = pt.x;
    m_rule.ptDown.y = pt.y;

    //ドット線の場合切り捨て

    if(bDot)
    {
        m_rule.ptDown.x = ::floor(m_rule.ptDown.x);
        m_rule.ptDown.y = ::floor(m_rule.ptDown.y);
    }

    //各タイプ別

    switch(m_ruleType)
    {
        //格子線
        case RULETYPE_GRIDLINE:
            m_rule.dTmp = m_rule.dAngleGrid;
            m_rule.nTmp = FALSE;
            break;
        //集中線 (dTmp = 角度)
        case RULETYPE_CONCLINE:
            xx = m_rule.ptDown.x - m_rule.ptCtConc.x;
            yy = m_rule.ptDown.y - m_rule.ptCtConc.y;
            m_rule.dTmp = ::atan2(yy, xx);
            break;
        //正円 (dTmp = 半径)
        case RULETYPE_CIRCLE:
            xx = m_rule.ptDown.x - m_rule.ptCtCir.x;
            yy = m_rule.ptDown.y - m_rule.ptCtCir.y;
            m_rule.dTmp = ::sqrt(xx * xx + yy * yy);
            break;
        //楕円 (dTmp = 半径)
        case RULETYPE_ELLIPSE:
            x = m_rule.ptDown.x - m_rule.ptCtEll.x;
            y = m_rule.ptDown.y - m_rule.ptCtEll.y;
            if(m_rule.bEllRevH) x = -x;

            xx = (x * m_rule.dEllTmp[2] - y * m_rule.dEllTmp[3]) * m_rule.dEllHV[1];
            yy =  x * m_rule.dEllTmp[3] + y * m_rule.dEllTmp[2];

            m_rule.dTmp = ::sqrt(xx * xx + yy * yy);
            break;
    }
}
