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

#include "CCanvasWin.h"

#include "CDraw.h"
#include "CConfig.h"
#include "CDevList.h"
#include "CKeyDat.h"
#include "CMainWin.h"
#include "CApp.h"

#include "defGlobal.h"
#include "defStrID.h"

#include "AXScrollBar.h"
#include "AXImage.h"
#include "AXApp.h"
#include "AXKey.h"


//********************************
// CCanvasWin
//********************************

/*!
    @class CCanvasWin
    @brief キャンバスウィンドウ
*/

CCanvasWin *CCanvasWin::m_pSelf = NULL;


CCanvasWin::CCanvasWin(AXWindow *pParent)
    : AXScrollView(pParent, SVS_HORZVERT | SVS_SUNKEN, LF_EXPAND_WH)
{
    m_pSelf = this;

    m_uFlags |= FLAG_TAKE_FOCUS;

    m_uLastDownKey = 0;

    m_pScrArea = new CCanvasWinArea(this);
}

//! すべてのキー許可

BOOL CCanvasWin::isAcceptKey(UINT keytype)
{
    return TRUE;
}

//! キー押し

BOOL CCanvasWin::onKeyDown(AXHD_KEY *phd)
{
    UINT key;

    key = phd->keysym;

    //最後に押されたキー保存（装飾キーは除く）
    /* グラブ中も判定 */

    if(key != KEY_CTRL_L && key != KEY_CTRL_R &&
       key != KEY_SHIFT_L && key != KEY_CTRL_R &&
       key != KEY_ALT_L && key != KEY_ALT_R)
        m_uLastDownKey = key;

    //

    if(!g_pdraw->isNowCtlNone())
    {
        //操作中時、各ツールに対応した処理
        //（キャンセル処理などの場合、グラブ解放）

        if(g_pdraw->onKey_inCtrl(key))
            CAPP->ungrabBoth();
    }
    else
    {
        //ショートカットキー

        int cmd = g_pconf->pKey->getCmd(key);

        if(cmd != -1)
            MAINWIN->sendCommand(cmd, 0, 0);
    }

    return TRUE;
}

//! キー離し

BOOL CCanvasWin::onKeyUp(AXHD_KEY *phd)
{
    if(phd->keysym == m_uLastDownKey)
        m_uLastDownKey = 0;

    return TRUE;
}

//! フォーカス消去

BOOL CCanvasWin::onFocusOut(int detail)
{
    m_uLastDownKey = 0;

    return TRUE;
}

//! コマンド

BOOL CCanvasWin::onCommand(UINT uID,ULONG lParam,int from)
{
    //ダイアログ開始
    /* ボタン押し時にダイアログを表示するものは、ここで実行させる。
       グラブ解放やXI2のデータを解放させてから行わせるため。 */

    switch(uID)
    {
        case CMDID_TEXTDLG:
            g_pdraw->onDown_text();
            break;
        case CMDID_SCALEROTDLG:
            g_pdraw->draw_boxedit_scaleRot();
            break;
    }

    return TRUE;
}

//! 通知

BOOL CCanvasWin::onNotify(AXWindow *pwin,UINT uNotify,ULONG lParam)
{
    //スクロール

    if(pwin == m_pScrH && uNotify == AXScrollBar::SBN_SCROLL)
        g_pdraw->m_ptScr.x = m_pScrH->getPos() - m_nScrCtX;
    else if(pwin == m_pScrV && uNotify == AXScrollBar::SBN_SCROLL)
        g_pdraw->m_ptScr.y = m_pScrV->getPos() - m_nScrCtY;

    g_pdraw->updateCanvas(TRUE);

    return TRUE;
}


//=====================


//! スペースキーが押されているか

BOOL CCanvasWin::isDownKeySpace()
{
    return (m_uLastDownKey == KEY_SPACE || m_uLastDownKey == KEY_NUM_SPACE);
}

//! スクロール範囲セット

void CCanvasWin::setScroll()
{
    AXSize size;

    g_pdraw->calcCanvasScrollMax(&size);

    //スクロールバー中央位置

    m_nScrCtX = (size.w - g_pdraw->m_szCanvas.w) / 2;
    m_nScrCtY = (size.h - g_pdraw->m_szCanvas.h) / 2;

    //セット

    m_pScrH->setStatus(0, size.w, g_pdraw->m_szCanvas.w);
    m_pScrH->setPos(m_nScrCtX);

    m_pScrV->setStatus(0, size.h, g_pdraw->m_szCanvas.h);
    m_pScrV->setPos(m_nScrCtY);
}

//! スクロール位置セット

void CCanvasWin::setScrollPos()
{
    m_pScrH->setPos(g_pdraw->m_ptScr.x + m_nScrCtX);
    m_pScrV->setPos(g_pdraw->m_ptScr.y + m_nScrCtY);
}



//********************************
// CCanvasWinArea
//********************************


/*!
    @class CCanvasWinArea
    @brief キャンバスウィンドウ表示部分
*/

CCanvasWinArea *CCanvasWinArea::m_pSelf = NULL;


CCanvasWinArea::CCanvasWinArea(AXWindow *pParent)
    : AXScrollArea(pParent, 0)
{
    m_pSelf = this;

    //ホイール動作を onButtonDown/Up で処理する
    m_uFlags |= FLAG_WHEELEVENT_NORMAL;

    m_rcsTimerUpdate.x = -1;

    g_pdraw->setCursorTool();
}

//! スクロールを表示するか

BOOL CCanvasWinArea::isShowScroll(int size,BOOL bHorz)
{
    return g_pconf->isCanvasScroll();
}


//===========================
//タイマー
//===========================


//! タイマー更新クリア（残っていた場合は処理する）

void CCanvasWinArea::clearTimerUpdate(UINT uTimerID)
{
    if(isTimerExist(uTimerID))
        onTimer(uTimerID, NULL);
}

//! UPDATECANVAS タイマークリア
/*!
    タイマーでの更新中は低画質のため、最後に高品質で更新
*/

void CCanvasWinArea::clearTimer_updateCanvas()
{
    delTimer(TIMERID_UPDATECANVAS);
    g_pdraw->updateCanvas();
}

//-----------

void CCanvasWinArea::setTimer_updateCanvas()
{
    addTimer(TIMERID_UPDATECANVAS, 5);
}

void CCanvasWinArea::setTimer_updateRect(const AXRectSize &rcs,int time)
{
    g_pdraw->calcUnionRectSize(&m_rcsTimerUpdate, rcs);

    if(!AXWindow::isTimerExist(TIMERID_UPDATERECT))
        addTimer(TIMERID_UPDATERECT, time);
}

void CCanvasWinArea::setTimer_updateMove()
{
    addTimer(TIMERID_UPDATE_MOVE, 5);
}

void CCanvasWinArea::setTimer_update()
{
    addTimer(TIMERID_UPDATE, 5);
}


//===========================
//ハンドラ
//===========================


//! サイズ変更時

BOOL CCanvasWinArea::onSize()
{
    //メインウィンドウが初期化状態の時は更新なし

    g_pdraw->changeCanvasWinSize(MAINWIN->isShowMain());

    return TRUE;
}

//! 描画

BOOL CCanvasWinArea::onPaint(AXHD_PAINT *phd)
{
    g_pdraw->m_pimgCanvas->put(m_id, phd->x, phd->y, phd->x, phd->y, phd->w, phd->h);

    return TRUE;
}

//! タイマー

BOOL CCanvasWinArea::onTimer(UINT uTimerID,ULONG lParam)
{
    switch(uTimerID)
    {
        //キャンバス更新（低品質）
        case TIMERID_UPDATECANVAS:
            g_pdraw->updateCanvas(TRUE, FALSE);
            break;
        //イメージ・キャンバス更新（範囲）
        case TIMERID_UPDATERECT:
            g_pdraw->updateRect(m_rcsTimerUpdate);
            axapp->update();

            m_rcsTimerUpdate.x = -1;
            break;
        //イメージ移動時の更新
        case TIMERID_UPDATE_MOVE:
            g_pdraw->updateImage();
            g_pdraw->updateCanvas(TRUE, FALSE);
            break;
        //更新
        case TIMERID_UPDATE:
            axapp->update();
            break;
    }

    delTimer(uTimerID);

    return TRUE;
}

//! ボタン押し

BOOL CCanvasWinArea::onButtonDown(AXHD_MOUSE *phd)
{
    CDraw::DRAWPOINT pt;
    UINT btt;

    pt.x = phd->x;
    pt.y = phd->y;
    pt.press = 1;

    btt = (g_pconf->pDev)->getButtonAction(2, phd->button,
                        phd->state & STATE_CTRL, phd->state & STATE_SHIFT, phd->state & STATE_ALT);

    if(g_pdraw->onDown(pt, btt))
        grabPointer();

    return TRUE;
}

//! ボタン離し

BOOL CCanvasWinArea::onButtonUp(AXHD_MOUSE *phd)
{
    CDraw::DRAWPOINT pt;

    pt.x = phd->x;
    pt.y = phd->y;
    pt.press = 0;

    if(g_pdraw->onUp(pt, phd->button))
        ungrabPointer();

    return TRUE;
}

//! マウス移動

BOOL CCanvasWinArea::onMouseMove(AXHD_MOUSE *phd)
{
    CDraw::DRAWPOINT pt;

    pt.x = phd->x;
    pt.y = phd->y;
    pt.press = 1;

    g_pdraw->onMove(pt, phd->state & STATE_CTRL, phd->state & STATE_SHIFT);

    return TRUE;
}

//! ダブルクリック

BOOL CCanvasWinArea::onDblClk(AXHD_MOUSE *phd)
{
    if(phd->button == BUTTON_LEFT)
    {
        if(g_pdraw->onLDblClk())
        {
            ungrabPointer();
            return TRUE;
        }
    }

    return FALSE;
}

//! ダイアログ中の操作

BOOL CCanvasWinArea::onMouseInSkip(AXHD_MOUSE *phd)
{
    //描画位置変更

    if(phd->type == EVENT_BUTTONDOWN && phd->button == BUTTON_LEFT)
        g_pdraw->onLDownInDlg(phd->x, phd->y);

    return TRUE;
}
