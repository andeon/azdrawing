/*****************************************************************************
 *  Copyright (C) 2012-2015 Azel.
 *
 *  This file is part of AzXClass.
 *
 *  This library is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this library.  If not, see <http://www.gnu.org/licenses/>.
******************************************************************************/

#include "AXMenuWindow.h"

#include "AXMenuBar.h"
#include "AXMenuItem.h"
#include "AXMenu.h"
#include "AXApp.h"
#include "AXAppRes.h"
#include "AXKey.h"


//-----------------------

#define FRAME_X         1       //枠の太さ
#define FRAME_Y         1
#define LEFTSPACE       18      //左枠とテキストの間の余白
#define RIGHTSPACE      10      //テキストと右枠の間の余白
#define ACCEL_SEPW      10      //テキストとアクセラレータの間の余白
#define ITEMSPACE_Y     3       //アイテム毎の縦余白(※AXMenu.cpp)
#define SEP_H           5       //セパレータ高さ(※AXMenu.cpp)
#define SCROLL_H        12      //スクロール部分の高さ

#define TIMERID_SHOWPOPUP   0
#define TIMERID_SCROLL      1

//-----------------------

/*!
    @class AXMenuWindow
    @brief メニューウィンドウ（ポップアップ）

    - メニューウィンドウは、表示している間だけ作成し、終了したら削除される（アイテムデータは別に存在しているので残る）
    - トップのポップアップでマウス/キー操作を全て受け取り、サブメニューに処理を振り分ける形。
    - トップポップアップ作成時に全てのサブメニューを作成し、サブメニューは状態に応じて表示/非表示させる。

    <h2>[通知]</h2>
    - 項目が選択された時、onCommand() が通知される。@n
      lParam = 選択項目のパラメータ値。
    - onOwnerDraw() 時、lParam1 = 項目のパラメータ値。lParam2 = テキスト(LPUSTR)。
    - ポップアップが表示される前に onMenuPopup() が呼ばれる。
      チェックを動的にセットしたい場合などに。

    @ingroup window
*/

/*
    m_pTopPopup     : トップのポップアップ
    m_pParentPopup  : [サブ] 親のポップアップ
    m_pCurPopup     : [トップ] 現在のカレントポップアップ
    m_pScrPopup     : [トップ] スクロール中の場合、スクロール対象のポップアップ
*/

//-----------------------



AXMenuWindow::~AXMenuWindow()
{
    AXMenuItem *p;

    //サブメニューウィンドウ全て削除

    for(p = m_pDat->getTopItem(); p; p = p->next())
    {
        if(p->m_pSubMenu && p->m_pSubMenu->m_pwinPopup)
        {
            delete p->m_pSubMenu->m_pwinPopup;
            p->m_pSubMenu->m_pwinPopup = NULL;
        }
    }
}

AXMenuWindow::AXMenuWindow(AXWindow *pOwner,AXMenu *pMenu)
    : AXTopWindow(pOwner, WS_HIDE | WS_DISABLE_WM | WS_DISABLE_IM | WS_TRANSIENT_FOR)
{
    m_pMenuBar  = NULL;
    m_pDat      = pMenu;
    m_pSelItem  = NULL;
    m_pTopPopup = m_pCurPopup = m_pParentPopup = m_pScrPopup = NULL;

    m_lReturn = 0;
}

//! メニューバーから表示（終了時、自身を削除する）
/*!
    @return カーソル移動によりこのアイテムを新たに表示。NULLで通常終了。
*/

AXMenuItem *AXMenuWindow::showFromMenuBar(AXMenuBar *pMenuBar,int rootx,int rooty,UINT uParentID)
{
    AXMenuItem *pItem;
    BOOL bChange;

    m_pMenuBar = pMenuBar;

    //表示前ハンドラ

    (pMenuBar->getNotify())->onMenuPopup(pMenuBar, m_pDat, uParentID);

    //開始

    showTopPopup(pMenuBar->getNotify(), rootx, rooty, 0);

    //

    pItem   = (AXMenuItem *)m_lReturn;      //NULLでキャンセル
    bChange = m_uFlags & FLAG_BARCHANGE;    //ONでメニューバーの項目変更

    //通知

    if(pItem && !bChange)
        (pMenuBar->getNotify())->sendCommand(pItem->m_uID, pItem->m_lParam, COMFROM_MENU);

    //削除

    delete this;
    axapp->sync();

    return (bChange)? pItem: NULL;
}

//! 独立ポップアップ表示（実行後、自身を削除）
/*!
    @param pSend 通知先ウィンドウ（NULL で通知しない）
    @return 選択された項目（NULL でキャンセル）
*/

AXMenuItem *AXMenuWindow::showPopup(AXWindow *pSend,int rootx,int rooty,UINT uFlags)
{
    AXMenuItem *pItem;

    showTopPopup(pSend, rootx, rooty, uFlags);

    pItem = (AXMenuItem *)m_lReturn;

    //通知

    if(pSend && pItem && !(uFlags & AXMenu::POPF_NOSEND))
        pSend->sendCommand(pItem->m_uID, pItem->m_lParam, COMFROM_MENU);

    //

    delete this;
    axapp->sync();

    return pItem;
}


//=================================
//ポップアップ関連
//=================================


//! トップポップアップ表示 [TOP]

void AXMenuWindow::showTopPopup(AXWindow *pNotify,int rootx,int rooty,UINT uFlags)
{
    m_pNotify       = pNotify;
    m_pTopPopup     = this;
    m_pCurPopup     = this;

    //

    _createSubMenu();

    _setWindowSize();

    //表示

    if(uFlags & AXMenu::POPF_RIGHT) rootx -= m_nW;

    moveInRoot(rootx, rooty);
    show();
    above(TRUE);

    //実行

    axapp->runPopup(this);
}

//! サブメニュー表示準備 [SUB]

void AXMenuWindow::showSubMenu(AXMenuWindow *pParent,int rootx,int rooty)
{
    m_nX = rootx;
    m_nY = rooty;

    m_pSelItem      = NULL;
    m_pTopPopup     = pParent->m_pTopPopup;
    m_pParentPopup  = pParent;
    m_pNotify       = m_pTopPopup->getNotify();

    //表示前ハンドラ

    if(m_pNotify)
        m_pNotify->onMenuPopup(m_pMenuBar, m_pDat, pParent->m_pSelItem->m_uID);

    //

    _setWindowSize();

    adjustPosInRoot(&m_nX, &m_nY);

    //タイマーで表示を遅延

    m_pTopPopup->addTimer(TIMERID_SHOWPOPUP, 200, (ULONG)this);
}

//! ポップアップ終了 [TOP]

void AXMenuWindow::endPopup(ULONG ret)
{
    m_lReturn = ret;

    ungrabPointer();

    axapp->exit();
}

//! ウィンドウサイズセット

void AXMenuWindow::_setWindowSize()
{
    AXMenuItem *p;
    int textw = 0,accw = 0,w,h = 0;

    //すでにセット済み

    if(m_uFlags & FLAG_RESIZED) return;

    //全アイテムサイズ計算

    m_pDat->initAllItem(m_pFont);

    //テキスト部分とアクセラレータ部分の最大幅＆高さ

    for(p = m_pDat->getTopItem(); p; p = p->next())
    {
        if(p->m_nTextW > textw) textw = p->m_nTextW;
        if(p->m_nAccW > accw) accw = p->m_nAccW;

        h += p->m_nItemH;
    }

    w = textw + accw;
    if(accw) w += ACCEL_SEPW;

    m_nTextMaxW = textw;
    m_nMaxH     = h;

    h += FRAME_Y * 2;

    //高さがルートより大きい時、スクロールあり

    if(h > axapp->getRootWindow()->getHeight())
    {
        m_uFlags |= FLAG_SCROLL;
        m_nScrY  = 0;

        h = axapp->getRootWindow()->getHeight();
    }

    //

    resize(FRAME_X + LEFTSPACE + w + RIGHTSPACE + FRAME_X, h);

    m_uFlags |= FLAG_RESIZED;
}

//! 全てのサブメニューウィンドウ作成 [TOP]

void AXMenuWindow::_createSubMenu()
{
    AXMenuItem *p;

    for(p = m_pDat->getTopItem(); p; p = p->next())
    {
        if(p->m_pSubMenu && !p->m_pSubMenu->m_pwinPopup)
        {
            p->m_pSubMenu->m_pwinPopup = new AXMenuWindow(m_pOwner, p->m_pSubMenu);

            //以降のサブメニューも作成

            p->m_pSubMenu->m_pwinPopup->_createSubMenu();
        }
    }
}

//! 選択されているサブメニュー以降のサブメニューを非表示に [TOP/SUB]

void AXMenuWindow::_hideSubMenu()
{
    AXMenuItem *p;
    AXMenuWindow *psub;

    for(p = m_pSelItem; p && p->m_pSubMenu;)
    {
        psub = p->m_pSubMenu->m_pwinPopup;

        psub->hide();

        p = psub->m_pSelItem;
    }
}

//! タイマーから表示タイミングが来た時 [SUB]

void AXMenuWindow::showSubMenuFromTimer()
{
    move(m_nX, m_nY);
    show();
    above(TRUE);
}


//==============================
//サブ処理
//==============================


//! マウス位置の（表示中の）ポップアップ取得 [TOP]
/*!
    @param pPt 現在位置のポップアップの座標でのカーソル位置が返る（NULL可）
*/

AXMenuWindow *AXMenuWindow::_getCurPosPopup(int rootx,int rooty,AXPoint *pPt)
{
    AXMenuWindow *p = m_pTopPopup,*pRet = NULL;
    int x,y,retx,rety;

    while(1)
    {
        if(!p->isVisible()) break;

        x = rootx, y = rooty;
        p->translateFrom(NULL, &x, &y);

        if(p->isContain(x, y))
        {
            //ウィンドウ範囲内に含まれていれば記録しておく
            /* サブメニューが他のポップアップと重なっている場合、
               一番下層のポップアップを対象とさせるため */

            pRet = p;
            retx = x;
            rety = y;
        }

        //選択アイテムがサブメニューの場合

        if(p->m_pSelItem && (p->m_pSelItem)->m_pSubMenu)
            p = (p->m_pSelItem->m_pSubMenu)->m_pwinPopup;
        else
            break;
    }

    if(pRet && pPt)
    {
        pPt->x = retx;
        pPt->y = rety;
    }

    return pRet;
}

//! マウス位置からアイテム取得

AXMenuItem *AXMenuWindow::_getCurPosItem(int y)
{
    AXMenuItem *p;
    int pos,bottom;

    if(m_uFlags & FLAG_SCROLL)
    {
        //スクロールあり

        if(y < FRAME_Y + SCROLL_H) return NULL;

        pos     = FRAME_Y + SCROLL_H - m_nScrY;
        bottom  = m_nH - FRAME_Y - SCROLL_H;
    }
    else
    {
        pos     = FRAME_Y;
        bottom  = m_nH - FRAME_Y;
    }

    //

    for(p = m_pDat->getTopItem(); p; p = p->next())
    {
        if(y < pos || pos >= bottom) break;

        if(pos <= y && y < pos + p->m_nItemH)
            return p;

        pos += p->m_nItemH;
    }

    return NULL;
}

//! アイテムから表示位置取得（ポップアップの座標）

void AXMenuWindow::_getShowItemPos(AXMenuItem *pItem,AXPoint *pPt)
{
    AXMenuItem *p;
    int y;

    y = FRAME_Y;
    if(m_uFlags & FLAG_SCROLL) y += SCROLL_H - m_nScrY;

    for(p = m_pDat->getTopItem(); p && p != pItem; p = p->next())
        y += p->m_nItemH;

    pPt->set(0, y);
}

//! ボタン押し時、スクロールボタンの範囲かどうか

BOOL AXMenuWindow::_judgeScroll(int rooty)
{
    int x,y;

    if(!(m_uFlags & FLAG_SCROLL)) return FALSE;

    x = 0, y = rooty;
    translateFrom(NULL, &x, &y);

    //範囲内か & 上下か

    if(y < FRAME_Y + SCROLL_H)
        m_uFlags &= ~FLAG_SCROLLDOWN;
    else if(y >= m_nH - FRAME_Y - SCROLL_H)
        m_uFlags |= FLAG_SCROLLDOWN;
    else
        return FALSE;

    return TRUE;
}

//! タイマー時スクロール [TOP/SUB]

void AXMenuWindow::_scroll()
{
    int scr = m_nScrY;

    if(m_uFlags & FLAG_SCROLLDOWN)
    {
        scr += 20;

        if(scr + m_nH - FRAME_Y * 2 - SCROLL_H * 2 > m_nMaxH)
            scr = m_nMaxH - m_nH + FRAME_Y * 2 + SCROLL_H * 2;
    }
    else
    {
        scr -= 20;
        if(scr < 0) scr = 0;
    }

    if(scr != m_nScrY)
    {
        m_nScrY = scr;
        redraw();
    }
}


//==============================
//サブ処理（アイテム関連）
//==============================


//! 項目決定 [TOP]

void AXMenuWindow::_enterItem(AXMenuItem *p)
{
    if(p && !p->m_pSubMenu && !p->isDisableItem())
    {
        //自動チェック

        if(p->m_uFlags & AXMenu::MIF_AUTOCHECK)
        {
            if(p->m_uFlags & AXMenu::MIF_RADIO)
                p->checkRadio();
            else
                p->m_uFlags ^= AXMenu::MIF_CHECK;
        }

        //終了

        endPopup((ULONG)p);
    }
}

//! 選択アイテム変更 [TOP/SUB]
/*!
    @param pNewItem NULLで選択解除
    @param bScrItem スクロールあり時、アイテムが見えるようにスクロールさせる
*/

void AXMenuWindow::_changeSelItem(AXMenuItem *pNewItem,BOOL bScrItem)
{
    AXPoint pt;
    int x,y;

    if(m_pSelItem == pNewItem) return;

    //ポップアップ表示タイマー削除

    m_pTopPopup->delTimer(TIMERID_SHOWPOPUP);

    //現在の選択がサブメニューありの場合、非表示に

    if(m_pSelItem && m_pSelItem->m_pSubMenu)
        _hideSubMenu();

    //選択変更

    m_pSelItem = pNewItem;
    redraw();

    //

    if(pNewItem)
    {
        //アイテムの表示位置

        _getShowItemPos(pNewItem, &pt);

        //項目が見えるようにスクロール

        if(bScrItem && (m_uFlags & FLAG_SCROLL))
        {
            y = pt.y - FRAME_Y - SCROLL_H;

            if(y < 0)
                m_nScrY += y;
            else if(y + pNewItem->m_nItemH > m_nH - FRAME_Y * 2 - SCROLL_H * 2)
                m_nScrY += y + pNewItem->m_nItemH - (m_nH - FRAME_Y * 2 - SCROLL_H * 2);
        }

        //サブメニューありの場合、表示準備

        if(pNewItem->isEnableSubMenu())
        {
            x = pt.x, y = pt.y;
            translateTo(NULL, &x, &y);

            (pNewItem->m_pSubMenu->m_pwinPopup)->showSubMenu(this, x + m_nW - 2, y - FRAME_Y);
        }
    }
}

//! キー押し時 [TOP/SUB]

void AXMenuWindow::_keyDown(UINT key)
{
    AXMenuItem *p;

    if(ISKEY_UP(key))           //上
        _moveUpDownSel(TRUE);
    else if(ISKEY_DOWN(key))    //下
        _moveUpDownSel(FALSE);
    else if(ISKEY_LEFT(key))
    {
        //左（前階層へ）

        if(m_pParentPopup)
        {
            m_pTopPopup->m_pCurPopup = m_pParentPopup;
            _changeSelItem(NULL);
        }
    }
    else if(ISKEY_RIGHT(key))
    {
        //右（次階層へ）

        if(m_pSelItem && m_pSelItem->isEnableSubMenu())
        {
            AXMenuWindow *pPop = m_pSelItem->m_pSubMenu->m_pwinPopup;

            if(pPop->isVisible())
            {
                m_pTopPopup->m_pCurPopup = pPop;
                pPop->_changeSelItem(pPop->m_pDat->getFirstItem());
            }
        }
    }
    else if(ISKEY_SPACE(key) || ISKEY_ENTER(key))
        //決定
        m_pTopPopup->_enterItem(m_pSelItem);
    else if(key > 32 && key < 127)
    {
        //------ ホットキー

        p = m_pDat->findHotKey(key);

        if(p)
        {
            if(p->m_pSubMenu)
                _changeSelItem(p, TRUE);
            else
                m_pTopPopup->_enterItem(p);
        }
    }
}

//! 選択位置を一つ上下に移動（無効アイテムは除く）

void AXMenuWindow::_moveUpDownSel(BOOL bUp)
{
    AXMenuItem *p;

    if(bUp)
    {
        //上

        p = (m_pSelItem)? m_pSelItem->prev(): m_pDat->getTopItem();

        for(; p && p->isDisableItem(); p = p->prev());
    }
    else
    {
        //下

        p = (m_pSelItem)? m_pSelItem->next(): m_pDat->getTopItem();

        for(; p && p->isDisableItem(); p = p->next());
    }

    if(p) _changeSelItem(p, TRUE);
}



//==============================
//ハンドラ
//==============================


//! マップ時

BOOL AXMenuWindow::onMap()
{
    //トップポップアップの場合、グラブ開始

    if(!m_pParentPopup)
        grabPointer();

    return TRUE;
}

//! タイマー [TOP]

BOOL AXMenuWindow::onTimer(UINT uTimerID,ULONG lParam)
{
    if(uTimerID == TIMERID_SHOWPOPUP)
    {
        //サブメニュー表示

        ((AXMenuWindow *)lParam)->showSubMenuFromTimer();
        delTimer(TIMERID_SHOWPOPUP);
    }
    else if(uTimerID == TIMERID_SCROLL)
    {
        //スクロール中

        if(m_pScrPopup) m_pScrPopup->_scroll();
    }

    return TRUE;
}

//! ボタン押し時 [TOP]

BOOL AXMenuWindow::onButtonDown(AXHD_MOUSE *phd)
{
    AXMenuWindow *p = _getCurPosPopup(phd->rootx, phd->rooty, NULL);

    if(!p)
        //ポップアップ範囲外の場合は終了（すべてのボタン）
        endPopup(0);
    else if(phd->button == BUTTON_LEFT)
    {
        //左クリック：スクロールの範囲の場合、スクロール開始

        if(p->_judgeScroll(phd->rooty))
        {
            addTimer(TIMERID_SCROLL, 100);
            m_pScrPopup = p;
        }
    }

    return TRUE;
}

//! ボタン離し時 [TOP]

BOOL AXMenuWindow::onButtonUp(AXHD_MOUSE *phd)
{
    if(phd->button == BUTTON_LEFT)
    {
        if(m_pScrPopup)
        {
            //スクロール中の場合

            m_pScrPopup = NULL;
            delTimer(TIMERID_SCROLL);
        }
        else
        {
            //項目決定
            //※押し時ではなく離した時点でのアイテム

            AXMenuWindow *pPop = _getCurPosPopup(phd->rootx, phd->rooty, NULL);

            if(pPop && pPop->m_pSelItem)
                _enterItem(pPop->m_pSelItem);
        }
    }

    return TRUE;
}

//! マウス移動時 [TOP]

BOOL AXMenuWindow::onMouseMove(AXHD_MOUSE *phd)
{
    AXPoint pt;
    AXMenuWindow *pPop;

    //スクロール中は除く

    if(m_pScrPopup) return TRUE;

    //どのポップアップ上にあるか

    pPop = _getCurPosPopup(phd->rootx, phd->rooty, &pt);

    if(pPop)
    {
        //-------- ポップアップ内

        //カレント

        m_pCurPopup = pPop;

        //選択変更

        pPop->_changeSelItem(pPop->_getCurPosItem(pt.y));
    }
    else
    {
        //-------- ポップアップ外

        //選択解除

        m_pCurPopup->_changeSelItem(NULL);

        //メニューバー上の別の項目にカーソルが乗ったら、一度終了させて新たに表示させる。

        if(m_pMenuBar)
        {
            AXMenuItem *pItem = m_pMenuBar->_movePopup(this, phd->rootx, phd->rooty);

            if(pItem)
            {
                m_uFlags |= FLAG_BARCHANGE;
                endPopup((ULONG)pItem);
            }
        }
    }

    return TRUE;
}

//! キー押し時 [TOP]

BOOL AXMenuWindow::onKeyDown(AXHD_KEY *phd)
{
    if(!m_pScrPopup)
    {
        if(phd->keysym == KEY_ESCAPE)
            //ESC : 終了
            endPopup(0);
        else
        {
            //カレントポップアップで処理

            if(m_pCurPopup)
                m_pCurPopup->_keyDown(phd->keysym);
        }
    }

    return TRUE;
}

//! 描画

BOOL AXMenuWindow::onPaint(AXHD_PAINT *phd)
{
    AXMenuItem *p;
    int y,bSel,bScr,col,fh;
    AXHD_OWNERDRAW od;

    bScr = m_uFlags & FLAG_SCROLL;

    //枠

    drawBox(0, 0, m_nW, m_nH, AXAppRes::FRAMEMENU);

    //背景

    drawFillBox(1, 1, m_nW - 2, m_nH - 2, AXAppRes::FACEMENU);

    //-------- 項目

    AXDrawText dt(m_id);

    //

    y  = FRAME_Y;
    fh = m_pFont->getHeight();

    if(bScr)
    {
        y += SCROLL_H - m_nScrY;
        //スクロール部分にテキストが表示されないように
        dt.setClipRect(0, FRAME_Y + SCROLL_H, m_nW, m_nH - FRAME_Y * 2 - SCROLL_H * 2);
    }

    //

    od.pwinFrom  = this;
    od.pDrawText = &dt;

    //

    for(p = m_pDat->getTopItem(); p; y += p->m_nItemH, p = p->next())
    {
        if(bScr)
        {
            if(y + p->m_nItemH < FRAME_Y) continue;
            if(y >= m_nH - FRAME_Y - SCROLL_H) continue;
        }

        //

        bSel = (p == m_pSelItem);

        if(p->m_uFlags & AXMenu::MIF_SEP)
        {
            //セパレータ

            dt.drawRect(FRAME_X + 1, y + SEP_H / 2, m_nW - FRAME_X * 2 - 2, 1, AXAppRes::TC_MENUDISABLE);
        }
        else if(p->m_uFlags & AXMenu::MIF_OWNERDRAW)
        {
            //オーナードロー

            od.nItemNo  = p->m_uID;
            od.uState   = 0;
            if(!(p->m_uFlags & AXMenu::MIF_DISABLE)) od.uState |= ODS_ENABLED;
            if(bSel) od.uState |= ODS_SELECTED;
            od.pItem    = (LPVOID)p;
            od.lParam1  = p->m_lParam;
            od.lParam2  = (ULONG)(LPUSTR)p->m_strText;
            od.rcs.set(FRAME_X, y, m_nW - FRAME_X * 2, p->m_nItemH);

            m_pNotify->onOwnerDraw(this, &od);
        }
        else
        {
            //選択カーソル

            if(bSel && !(p->m_uFlags & AXMenu::MIF_DISABLE))
                drawFillBox(FRAME_X, y, m_nW - FRAME_X * 2, p->m_nItemH, AXAppRes::BACKMENUSEL);

            //チェック

            if(p->m_uFlags & AXMenu::MIF_CHECK)
            {
                if(p->m_uFlags & AXMenu::MIF_RADIO)
                    drawRadioCheck(FRAME_X + 6, y + (p->m_nItemH - 4) / 2, AXAppRes::TEXTMENU);
                else
                    drawCheck(FRAME_X + 5, y + (p->m_nItemH - 7) / 2, AXAppRes::TEXTMENU);
            }

            //テキスト

            col = (p->m_uFlags & AXMenu::MIF_DISABLE)? AXAppRes::TC_MENUDISABLE: AXAppRes::TC_MENU;

            dt.draw(*m_pFont, FRAME_X + LEFTSPACE, y + ITEMSPACE_Y, (LPWORD)p->m_strText, p->m_nAccTextPos, col);

            if(p->m_nAccW)
            {
                dt.draw(*m_pFont, FRAME_X + LEFTSPACE + m_nTextMaxW + ACCEL_SEPW, y + ITEMSPACE_Y,
                            p->m_strText.at(p->m_nAccTextPos + 1), p->m_strText.getLen() - p->m_nAccTextPos - 1,
                            AXAppRes::TC_MENUDISABLE);
            }

            //ホットキー下線

            if(p->m_cHotKey)
            {
                dt.drawRect(FRAME_X + LEFTSPACE + p->m_wHotKeyX, y + ITEMSPACE_Y + fh,
                            p->m_wHotKeyW, 1, col);
            }

            //サブメニュー矢印

            if(p->m_pSubMenu)
                drawArrowRight(m_nW - FRAME_X - 6, y + p->m_nItemH / 2, AXAppRes::TEXTMENU);
        }
    }

    dt.end();

    //スクロール
    //※カーソルがはみ出る場合があるのでここで描画

    if(bScr)
    {
        DRAWRECT rc[2];

        setDrawRect(rc, FRAME_X, FRAME_Y, m_nW - FRAME_X * 2, SCROLL_H);
        setDrawRect(rc + 1, FRAME_X, m_nH - FRAME_Y - SCROLL_H, m_nW - FRAME_X * 2, SCROLL_H);

        drawFillBoxs(rc, 2, AXAppRes::FACEMENU);

        drawArrowUp(m_nW / 2, FRAME_Y + SCROLL_H / 2, AXAppRes::TEXTMENU);
        drawArrowDown(m_nW / 2, m_nH - FRAME_Y - SCROLL_H + SCROLL_H / 2, AXAppRes::TEXTMENU);
    }

    return TRUE;
}
