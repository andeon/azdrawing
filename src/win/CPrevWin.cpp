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

#include <stdio.h>

#include "CPrevWin.h"

#include "CCanvasWin.h"
#include "CImage32.h"
#include "CArrowMenuBtt.h"
#include "CConfig.h"
#include "CDraw.h"
#include "defGlobal.h"
#include "defStrID.h"

#include "AXImage.h"
#include "AXMenu.h"
#include "AXApp.h"

//--------------------

enum PREVWINCMDID
{
    PREVCMDID_LEFT_PREVSCR = 1000,
    PREVCMDID_LEFT_CANVASSCR,
    PREVCMDID_RIGHT_PREVSCR,
    PREVCMDID_RIGHT_CANVASSCR,
    PREVCMDID_SCALE_TOP = 10000
};

//--------------------

/*!
    @class CPrevWin
    @brief プレビューウィンドウ
*/
/*
    - CDraw::m_pimgBlend のイメージを元に拡大縮小表示。
    - 中ボタンドラッグでプレビュー内スクロール。
*/


CPrevWin *CPrevWin::m_pSelf = NULL;


CPrevWin::~CPrevWin()
{
    delete m_pimg;
}

CPrevWin::CPrevWin(AXWindow *pOwner,UINT addstyle)
    : AXTopWindow(pOwner,
                  WS_HIDE | WS_TITLE | WS_CLOSE | WS_BORDER | WS_DISABLE_IM | addstyle)
{
    m_pSelf = this;

    m_pbtt = new CArrowMenuBtt(this);
    m_pimg = new AXImage;

    m_fDrag = 0;

    //

    if(g_pconf->isPrevWinLoupe())
        m_dScale = g_pconf->nPrevWinLoupeScale * 0.01;
    else
        m_dScale = g_pconf->nPrevWinScale / 10000.0;

    m_dScaleDiv = 1.0 / m_dScale;

    m_ptScr.zero();

    m_ptImgBase.x = g_pconf->nInitImgW / 2;
    m_ptImgBase.y = g_pconf->nInitImgH / 2;

    //

    _setWinTitle();

    attachAccel(((AXTopWindow *)m_pOwner)->getAccel());
}

//! 表示/非表示 切り替え

void CPrevWin::showChange()
{
    if(g_pconf->isViewPrevWin())
    {
        showRestore();
        draw();
    }
    else
        hide();
}

//! 編集ファイルやイメージサイズが変わった時（描画は後で行われる）

void CPrevWin::changeImgSize()
{
    if(g_pconf->isPrevWinLoupe())
    {
        //ルーペ（カーソル位置から再計算）

        CANVASAREA->getCursorPos(&m_ptImgBase);

        g_pdraw->calcWindowToImage(&m_ptImgBase, m_ptImgBase.x, m_ptImgBase.y);
    }
    else
    {
        //通常

        _adjustScale();

        m_ptImgBase.x = g_pdraw->m_nImgW / 2;
        m_ptImgBase.y = g_pdraw->m_nImgH / 2;

        m_ptScr.zero();
    }
}

//! ルーペ時、キャンバス位置移動

void CPrevWin::moveCanvasPos(double x,double y)
{
    AXPoint pt;

    //イメージ位置計算

    g_pdraw->calcWindowToImage(&pt, x, y);

    //位置が変わった場合、更新タイマーセット

    if(pt != m_ptImgBase)
    {
        m_ptImgBase = pt;

        addTimer(0, 10);
    }
}


//=========================
//サブ
//=========================


//! 全体描画

void CPrevWin::draw()
{
    CImage32::CANVASDRAW info;

    if(!isVisible()) return;

    info.rcsDst.set(0, 0, m_nW, m_nH);

    info.nBaseX     = m_ptImgBase.x;
    info.nBaseY     = m_ptImgBase.y;
    info.nScrollX   = m_nW / 2 - m_ptScr.x;
    info.nScrollY   = m_nH / 2 - m_ptScr.y;
    info.dScaleDiv  = m_dScaleDiv;
    info.bHRev      = FALSE;
    info.dwExCol    = g_pconf->dwCanvasBkCol;

    if(m_dScale >= 1.0)
        g_pdraw->m_pimgBlend->drawCanvasNormal(m_pimg, &info);
    else
        g_pdraw->m_pimgBlend->drawCanvasScaleDown(m_pimg, &info);

    if(g_pconf->isPrevWinLoupe())
        _drawCross(info.rcsDst);

    redraw();
}

//! 範囲更新
/*!
    @param rcs イメージ範囲
*/

void CPrevWin::draw(const AXRectSize &rcs)
{
    CImage32::CANVASDRAW info;
    int x1,y1,x2,y2,cw,ch;

    if(!isVisible()) return;

    x1 = rcs.x - 1, y1 = rcs.y - 1;
    x2 = rcs.x + rcs.w, y2 = rcs.y + rcs.h;

    cw = m_nW / 2, ch = m_nH / 2;

    //------- イメージ -> ウィンドウ座標

    x1 = (int)((x1 - m_ptImgBase.x) * m_dScale + cw - m_ptScr.x);
    y1 = (int)((y1 - m_ptImgBase.y) * m_dScale + ch - m_ptScr.y);

    x2 = (int)((x2 - m_ptImgBase.x) * m_dScale + cw - m_ptScr.x);
    y2 = (int)((y2 - m_ptImgBase.y) * m_dScale + ch - m_ptScr.y);

    x1--, y1--, x2++, y2++;

    //範囲内判定

    if(x2 < 0 || y2 < 0 || x1 >= m_nW || y1 >= m_nH) return;

    //調整

    if(x1 < 0) x1 = 0;
    if(y1 < 0) y1 = 0;
    if(x2 >= m_nW) x2 = m_nW - 1;
    if(y2 >= m_nH) y2 = m_nH - 1;

    //------- 更新

    info.rcsDst.set(x1, y1, x2 - x1 + 1, y2 - y1 + 1);

    info.nBaseX     = m_ptImgBase.x;
    info.nBaseY     = m_ptImgBase.y;
    info.nScrollX   = cw - m_ptScr.x;
    info.nScrollY   = ch - m_ptScr.y;
    info.dScaleDiv  = m_dScaleDiv;
    info.bHRev      = FALSE;
    info.dwExCol    = g_pconf->dwCanvasBkCol;

    if(m_dScale >= 1.0)
        g_pdraw->m_pimgBlend->drawCanvasNormal(m_pimg, &info);
    else
        g_pdraw->m_pimgBlend->drawCanvasScaleDown(m_pimg, &info);

    if(g_pconf->isPrevWinLoupe())
        _drawCross(info.rcsDst);

    redraw(info.rcsDst);
}

//! ルーペ時の中央線描画

void CPrevWin::_drawCross(const AXRectSize &rcs)
{
    int n,i;

    //横線

    n = m_nH / 2;

    if(rcs.y < n && n < rcs.y + rcs.h)
    {
        for(i = rcs.w - 1; i >= 0; i--)
            m_pimg->blendPixel(rcs.x + i, n, 0x600000ff);
    }

    //縦線

    n = m_nW / 2;

    if(rcs.x < n && n < rcs.x + rcs.w)
    {
        for(i = rcs.h - 1; i >= 0; i--)
            m_pimg->blendPixel(n, rcs.y + i, 0x600000ff);
    }
}

//! タイトルセット

void CPrevWin::_setWinTitle()
{
    char m[32];

    if(g_pconf->isPrevWinLoupe())
        ::sprintf(m, "prev [LOUPE %d%%]", g_pconf->nPrevWinLoupeScale);
    else
        ::sprintf(m, "prev [%d.%02d%%]", g_pconf->nPrevWinScale / 100, g_pconf->nPrevWinScale % 100);

    setTitle(m);
}

//! 表示倍率変更時

void CPrevWin::_changeScale()
{
    if(g_pconf->isPrevWinLoupe())
    {
        //ルーペ時

        m_dScale    = g_pconf->nPrevWinLoupeScale * 0.01;
        m_dScaleDiv = 1.0 / m_dScale;
        m_ptScr.zero();
    }
    else
    {
        //通常時

        if(g_pconf->uPrevWinFlags & CConfig::PREVF_FULLVIEW)
            _adjustScale();
        else
        {
            m_dScale    = g_pconf->nPrevWinScale / 10000.0;
            m_dScaleDiv = 1.0 / m_dScale;

            _adjustScrPos();
        }
    }

    _setWinTitle();
    draw();
}

//! 全体表示時の表示倍率調整

void CPrevWin::_adjustScale()
{
    if(g_pconf->uPrevWinFlags & CConfig::PREVF_FULLVIEW)
    {
        double x,y;

        //倍率が低い方

        x = (double)m_nW / g_pdraw->m_nImgW;
        y = (double)m_nH / g_pdraw->m_nImgH;

        if(x > y) x = y;
        if(x < 0.01) x = 0.01;

        //

        m_dScale    = x;
        m_dScaleDiv = 1.0 / m_dScale;

        m_ptScr.zero();

        g_pconf->nPrevWinScale = (int)(m_dScale * 10000 + 0.5);
        if(g_pconf->nPrevWinScale == 0) g_pconf->nPrevWinScale = 1;

        //タイトル

        _setWinTitle();
    }
}

//! スクロール位置調整（固定表示時）
/*!
    左上・右下はクライアント中心まで
*/

void CPrevWin::_adjustScrPos()
{
    int x1,y1,x2,y2,cw,ch;

    cw = m_nW / 2;
    ch = m_nH / 2;

    x1 = (int)(-m_ptImgBase.x * m_dScale + cw - m_ptScr.x);
    y1 = (int)(-m_ptImgBase.y * m_dScale + ch - m_ptScr.y);
    x2 = (int)((g_pdraw->m_nImgW - 1 - m_ptImgBase.x) * m_dScale + cw - m_ptScr.x);
    y2 = (int)((g_pdraw->m_nImgH - 1 - m_ptImgBase.y) * m_dScale + ch - m_ptScr.y);

    if(x1 > cw)
        m_ptScr.x = (int)(-m_ptImgBase.x * m_dScale);
    else if(x2 < cw)
        m_ptScr.x = (int)((g_pdraw->m_nImgW - 1 - m_ptImgBase.x) * m_dScale);

    if(y1 > ch)
        m_ptScr.y = (int)(-m_ptImgBase.y * m_dScale);
    else if(y2 < ch)
        m_ptScr.y = (int)((g_pdraw->m_nImgH - 1 - m_ptImgBase.y) * m_dScale);
}

//! キャンバススクロール（固定表示時）
/*!
    プレビューウィンドウ上のイメージ位置が、キャンバスの中央位置になるように
*/

void CPrevWin::_scrollCanvas(int x,int y)
{
    AXPoint pt;

    pt.x = (int)((x + m_ptScr.x - m_nW / 2) * m_dScaleDiv + m_ptImgBase.x);
    pt.y = (int)((y + m_ptScr.y - m_nH / 2) * m_dScaleDiv + m_ptImgBase.y);

    g_pdraw->changeScrPos(&pt, FALSE);
    CANVASAREA->setTimer_updateCanvas();
}

//! 通常メニュー表示

void CPrevWin::_showMenuNormal()
{
    AXMenu *pm,*pm2;
    AXRectSize rcs;
    AXString str;
    int i,id;

    //------------- メニュー

    _trgroup(STRGID_PREVWINMENU);

    pm = new AXMenu;

    pm->addTr(STRID_PREVW_CHANGE_LOUPE);
    pm->addSep();

    //左ボタン

    i = g_pconf->uPrevWinFlags & CConfig::PREVF_LEFT_CANVAS;

    pm2 = new AXMenu;
    pm2->addCheck(PREVCMDID_LEFT_PREVSCR, _str(STRID_PREVW_BTT_PREVSCR), !i);
    pm2->addCheck(PREVCMDID_LEFT_CANVASSCR, _str(STRID_PREVW_BTT_CANVASSCR), i);
    pm->addTr(STRID_PREVW_LEFTBTT, pm2);

    //右ボタン

    i = g_pconf->uPrevWinFlags & CConfig::PREVF_RIGHT_CANVAS;

    pm2 = new AXMenu;
    pm2->addCheck(PREVCMDID_RIGHT_PREVSCR, _str(STRID_PREVW_BTT_PREVSCR), !i);
    pm2->addCheck(PREVCMDID_RIGHT_CANVASSCR, _str(STRID_PREVW_BTT_CANVASSCR), i);
    pm->addTr(STRID_PREVW_RIGHTBTT, pm2);

    //

    pm->addSep();
    pm->addTrCheck(STRID_PREVW_FULLVIEW, g_pconf->uPrevWinFlags & CConfig::PREVF_FULLVIEW);

    //倍率

    for(i = 10; i <= 400; )
    {
        str.setInt(i);
        str += '%';

        pm->add(PREVCMDID_SCALE_TOP + i, str);

        i += (i >= 100)? 100: 10;
    }

    //

    m_pbtt->getWindowRect(&rcs);
    id = (int)pm->popup(NULL, rcs.x + rcs.w, rcs.y + rcs.h, AXMenu::POPF_RIGHT);

    delete pm;

    //------------ 処理

    if(id == -1) return;

    if(id >= PREVCMDID_SCALE_TOP)
    {
        //表示倍率指定（全体表示の場合はOFFにする）

        g_pconf->nPrevWinScale = (id - PREVCMDID_SCALE_TOP) * 100;
        g_pconf->uPrevWinFlags &= ~CConfig::PREVF_FULLVIEW;

        _changeScale();
    }
    else
    {
        switch(id)
        {
            //ルーペに切替
            case STRID_PREVW_CHANGE_LOUPE:
                g_pconf->uPrevWinFlags |= CConfig::PREVF_MODE_LOUPE;

                _changeScale();
                break;
            //左ボタン
            case PREVCMDID_LEFT_PREVSCR:
                g_pconf->uPrevWinFlags &= ~CConfig::PREVF_LEFT_CANVAS;
                break;
            case PREVCMDID_LEFT_CANVASSCR:
                g_pconf->uPrevWinFlags |= CConfig::PREVF_LEFT_CANVAS;
                break;
            //右ボタン
            case PREVCMDID_RIGHT_PREVSCR:
                g_pconf->uPrevWinFlags &= ~CConfig::PREVF_RIGHT_CANVAS;
                break;
            case PREVCMDID_RIGHT_CANVASSCR:
                g_pconf->uPrevWinFlags |= CConfig::PREVF_RIGHT_CANVAS;
                break;
            //全体表示
            case STRID_PREVW_FULLVIEW:
                g_pconf->uPrevWinFlags ^= CConfig::PREVF_FULLVIEW;
                _changeScale();
                break;
        }
    }
}

//! ルーペメニュー表示

void CPrevWin::_showMenuLoupe()
{
    AXMenu *pm;
    AXRectSize rcs;
    AXString str;
    int i,id;

    //------------- メニュー

    _trgroup(STRGID_PREVWINMENU);

    pm = new AXMenu;

    pm->addTr(STRID_PREVW_CHANGE_NORMAL);
    pm->addSep();

    for(i = 100; i <= 800; )
    {
        str.setInt(i);
        str += '%';

        pm->add(PREVCMDID_SCALE_TOP + i, str);

        i += (i < 200)? 50: 100;
    }

    //

    m_pbtt->getWindowRect(&rcs);
    id = (int)pm->popup(NULL, rcs.x + rcs.w, rcs.y + rcs.h, AXMenu::POPF_RIGHT);

    delete pm;

    //------------ 処理

    if(id == -1) return;

    if(id == STRID_PREVW_CHANGE_NORMAL)
    {
        //固定表示に切替

        g_pconf->uPrevWinFlags &= ~CConfig::PREVF_MODE_LOUPE;

        m_ptScr.zero();

        m_ptImgBase.x = g_pdraw->m_nImgW / 2;
        m_ptImgBase.y = g_pdraw->m_nImgH / 2;

        _changeScale();
    }
    else
    {
        //倍率

        g_pconf->nPrevWinLoupeScale = id - PREVCMDID_SCALE_TOP;

        _changeScale();
    }
}



//=========================
//ハンドラ
//=========================


//! 閉じる（非表示）

BOOL CPrevWin::onClose()
{
    m_pOwner->onCommand(STRID_MENU_VIEW_PREVIEWWIN, 0, 0);

    return TRUE;
}

//! 描画

BOOL CPrevWin::onPaint(AXHD_PAINT *phd)
{
    m_pimg->put(m_id, phd->x, phd->y, phd->x, phd->y, phd->w, phd->h);

    return TRUE;
}

//! サイズ変更時

BOOL CPrevWin::onSize()
{
    m_pimg->recreate(m_nW, m_nH, 32, 32);

    m_pbtt->moveParent();

    if(!g_pconf->isPrevWinLoupe())
    {
        _adjustScale();
        _adjustScrPos();
    }

    draw();

    return TRUE;
}

//! 通知

BOOL CPrevWin::onNotify(AXWindow *pwin,UINT uNotify,ULONG lParam)
{
    //メニュー表示

    if(pwin == m_pbtt)
    {
        if(g_pconf->isPrevWinLoupe())
            _showMenuLoupe();
        else
            _showMenuNormal();
    }

    return TRUE;
}

//! ボタン押し時

BOOL CPrevWin::onButtonDown(AXHD_MOUSE *phd)
{
    int f;

    if(m_fDrag == 0 && !(g_pconf->isPrevWinLoupe()))
    {
        if(phd->button == BUTTON_LEFT)
            f = g_pconf->uPrevWinFlags & CConfig::PREVF_LEFT_CANVAS;
        else if(phd->button == BUTTON_RIGHT)
            f = g_pconf->uPrevWinFlags & CConfig::PREVF_RIGHT_CANVAS;
        else if(phd->button == BUTTON_MIDDLE)
            f = 0;
        else
            f = -1;

        //全体表示の場合、プレビュー内スクロールは無効

        if((g_pconf->uPrevWinFlags & CConfig::PREVF_FULLVIEW) && f == 0)
            f = -1;

        //キャンバススクロールの場合

        if(f > 0)
            _scrollCanvas(phd->x, phd->y);

        //ドラッグ開始

        if(f != -1)
        {
            /* 下位8bitはボタン番号。8bit目がONでキャンバススクロール */

            m_fDrag = phd->button;
            if(f) m_fDrag |= 256;

            m_nBkX = phd->rootx;
            m_nBkY = phd->rooty;

            grabPointer();
        }
    }

    return TRUE;
}

//! ボタン離し時

BOOL CPrevWin::onButtonUp(AXHD_MOUSE *phd)
{
    if(m_fDrag && phd->button == (m_fDrag & 255))
    {
        //キャンバススクロール、タイマークリア

        if(m_fDrag & 256)
            CANVASAREA->clearTimer_updateCanvas();

        //

        m_fDrag = 0;
        ungrabPointer();
    }

    return TRUE;
}

//! マウス移動時

BOOL CPrevWin::onMouseMove(AXHD_MOUSE *phd)
{
    if(m_fDrag == 0) return TRUE;

    if(m_fDrag & 256)
    {
        //キャンバススクロール

        _scrollCanvas(phd->x, phd->y);
    }
    else
    {
        //プレビュー内スクロール

        m_ptScr.x += m_nBkX - phd->rootx;
        m_ptScr.y += m_nBkY - phd->rooty;

        _adjustScrPos();

        m_nBkX = phd->rootx;
        m_nBkY = phd->rooty;

        addTimer(0, 5);
    }

    return TRUE;
}

//! タイマー

BOOL CPrevWin::onTimer(UINT uTimerID,ULONG lParam)
{
    //更新

    delTimer(uTimerID);

    draw();
    redrawUpdate();

    return TRUE;
}
