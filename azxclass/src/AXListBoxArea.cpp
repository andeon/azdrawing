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

#include "AXListBoxArea.h"

#include "AXScrollBar.h"
#include "AXListBoxItem.h"
#include "AXListBoxItemManager.h"
#include "AXAppRes.h"
#include "AXKey.h"


//-------------------

#define TEXT_SPACEX     2

//-------------------

/*!
    @class AXListBoxArea
    @brief リストボックスの表示部分

    - onNotify() は通知ウィンドウで実行される。
    - onOwnerDraw() はオーナーウィンドウの通知ウィンドウで実行される。

    <h2>[Notify通知]</h2>
    lParam : 現在の選択番号

    <h2>[オーナードロー]</h2>
    lParam1 : アイテムのlParam値
    lParam2 : アイテムのテキスト(LPUSTR)
    pItem   : AXListBoxItem*

    @ingroup widgetsub
*/

/*!
    @var AXListBoxArea::LBAS_OWNERDRAW
    @brief アイテムをオーナードロー（作成時の pOwner の通知ウィンドウで onOwnerDraw() が実行される）
    @var AXListBoxArea::LBAS_POPUP
    @brief ポップアップタイプ（コンボボックスのポップアップ時など）
*/
/*!
    @var AXListBoxArea::LBAN_SELCHANGE_MOUSE
    @brief マウス操作によって選択が変更された
    @var AXListBoxArea::LBAN_SELCHANGE_KEY
    @brief キー操作によって選択が変更された
    @var AXListBoxArea::LBAN_DBLCLK
    @brief 左ボタンがダブルクリックされた（アイテム上）
    @var AXListBoxArea::LBAN_POPUPEND
    @brief ポップアップタイプ時、ポップアップの終了
*/


AXListBoxArea::~AXListBoxArea()
{

}

//! 作成

AXListBoxArea::AXListBoxArea(AXWindow *pParent,AXWindow *pOwner,UINT uStyle,AXListBoxItemManager *pDat)
    : AXScrollArea(pParent, uStyle)
{
    m_pOwner    = pOwner;
    m_pDat      = pDat;
    m_nItemH    = m_pFont->getHeight() + 2;
}

//! 垂直スクロール情報セット

void AXListBoxArea::setScrVInfo()
{
    if(scrV())
        scrV()->setStatus(0, m_nItemH * m_pDat->getCnt(), m_nH, m_nItemH);
}

//! 選択位置が表示されるように垂直スクロール位置調整
/*!
    @param dir 移動方向(-1, 1)。0 で選択が一番上に来るように
*/

void AXListBoxArea::adjustScrVSel(int dir)
{
    int pos,scry;

    if(scrV())
    {
        pos = m_pDat->getSel();
        if(pos == -1) return;

        scry  = getVertPos();
        pos   *= m_nItemH;

        if(dir < 0 && pos < scry)
            scrV()->setPos(pos);
        else if(dir > 0 && pos - scry > m_nH - m_nItemH)
            scrV()->setPos(pos - m_nH + m_nItemH);
        else
        {
            //dir == 0
            if(pos < scry || pos - scry > m_nH - m_nItemH)
                scrV()->setPos(pos);
        }
    }
}

//! スクロール表示判定

BOOL AXListBoxArea::isShowScroll(int size,BOOL bHorz)
{
    if(bHorz)
        return (size < getHorzMax());
    else
        return (size < m_nItemH * m_pDat->getCnt());
}


//===========================
//サブ処理
//===========================


//! 通知

void AXListBoxArea::_sendNotify(UINT uNotify)
{
    getNotify()->sendNotify(this, uNotify, m_pDat->getSel());
}

//! カーソル位置からアイテム番号取得

int AXListBoxArea::_getCurItemNo(int y)
{
    y += getVertPos();

    if(y < 0 || y >= m_nItemH * m_pDat->getCnt())
        return -1;

    return y / m_nItemH;
}


//===========================
//ハンドラ
//===========================


//! サイズ変更時

BOOL AXListBoxArea::onSize()
{
    setScrVInfo();

    if(scrH())
        scrH()->setPage(m_nW);

    return TRUE;
}

//! 通知時

BOOL AXListBoxArea::onNotify(AXWindow *pwin,UINT uNotify,ULONG uParam)
{
    if(uNotify == SAN_SCROLL_HORZ || uNotify == SAN_SCROLL_VERT)
        redraw();

    return TRUE;
}

//! ボタン押し時

BOOL AXListBoxArea::onButtonDown(AXHD_MOUSE *phd)
{
    int no;

    if(phd->button == BUTTON_LEFT)
    {
        if(m_uStyle & LBAS_POPUP)
            //ポップアップ時は選択＝即終了
            _sendNotify(LBAN_POPUPEND);
        else
        {
            m_pParent->setFocus();

            //選択変更

            no = _getCurItemNo(phd->y);
            if(no != -1)
            {
                if(m_pDat->setSel(no))
                {
                    _sendNotify(LBAN_SELCHANGE_MOUSE);
                    redrawUpdate();
                }
            }
        }
    }

    return TRUE;
}

//! ダブルクリック時

BOOL AXListBoxArea::onDblClk(AXHD_MOUSE *phd)
{
    if(phd->button == BUTTON_LEFT)
    {
        //選択があり、かつ押されたのがアイテムの上の場合
        if(m_pDat->getSel() != -1 && _getCurItemNo(phd->y) != -1)
            _sendNotify(LBAN_DBLCLK);
    }

    return TRUE;
}

//! マウスホイール（垂直スクロール）

BOOL AXListBoxArea::onMouseWheel(AXHD_MOUSE *phd,BOOL bUp)
{
    if(scrV())
    {
        int pos = getVertPos();

        if(bUp)
            pos -= m_nItemH * 3;
        else
            pos += m_nItemH * 3;

        if(scrV()->setPos(pos))
            redraw();
    }

    return TRUE;
}

//! マウス移動（ポップアップ時、移動中の選択変更）

BOOL AXListBoxArea::onMouseMove(AXHD_MOUSE *phd)
{
    if(m_uStyle & LBAS_POPUP)
    {
        int no = _getCurItemNo(phd->y);

        if(no != -1)
        {
            if(m_pDat->setSel(no))
                redrawUpdate();
        }
    }

    return TRUE;
}

//! キー押し

BOOL AXListBoxArea::onKeyDown(AXHD_KEY *phd)
{
    int dir;

    if(ISKEY_UP(phd->keysym))
        dir = -1;
    else if(ISKEY_DOWN(phd->keysym))
        dir = 1;
    else if(ISKEY_PAGEUP(phd->keysym))
        dir = -m_nH / m_nItemH;
    else if(ISKEY_PAGEDOWN(phd->keysym))
        dir = m_nH / m_nItemH;
    else
        return FALSE;

    if(m_pDat->moveSelUpDown(dir))
    {
        adjustScrVSel(dir);
        _sendNotify(LBAN_SELCHANGE_KEY);
        redrawUpdate();
    }

    return TRUE;
}

//! 描画

BOOL AXListBoxArea::onPaint(AXHD_PAINT *phd)
{
    int sel,scrx,y,no,bSel,selbkcol,vspace;
    AXListBoxItem *p;
    AXHD_OWNERDRAW od;
    AXWindow *pOD;

    sel    = m_pDat->getSel();
    scrx   = getHorzPos();
    vspace = (m_nItemH - m_pFont->getHeight()) / 2;

    selbkcol = (m_pParent->isFocused() && isEnabled())? AXAppRes::BACKREVTEXT: AXAppRes::FACEDARK;

    //背景

    drawFillBox(0, 0, m_nW, m_nH, AXAppRes::FACEDOC);

    //--------- 項目

    AXDrawText dt(m_id);

    //

    if(m_uStyle & LBAS_OWNERDRAW)
    {
        od.pwinFrom  = m_pOwner;
        od.pDrawText = &dt;
        od.uState    = (m_uFlags & FLAG_ENABLED)? ODS_ENABLED: 0;

        pOD = m_pOwner->getNotify();
    }

    //

    y = -getVertPos();

    for(p = m_pDat->getTopItem(), no = 0; p; p = p->next(), no++, y += m_nItemH)
    {
        if(y <= -m_nItemH) continue;
        if(y >= m_nH) break;

        bSel = (no == sel);

        if(m_uStyle & LBAS_OWNERDRAW)
        {
            //オーナードロー

            od.nItemNo  = no;
            od.pItem    = (LPVOID)p;
            od.lParam1  = p->m_lParam;
            od.lParam2  = (ULONG)(LPUSTR)p->m_strText;
            od.rcs.set(0, y, m_nW, m_nItemH);

            if(bSel)
                od.uState |= ODS_SELECTED | ((m_pParent->isFocused())? ODS_FOCUSED: 0);
            else
                od.uState &= ~(ODS_SELECTED | ODS_FOCUSED);

            pOD->onOwnerDraw(this, &od);
        }
        else
        {
            //通常描画

            if(bSel) drawFillBox(0, y, m_nW, m_nItemH, selbkcol);

            dt.draw(*m_pFont, TEXT_SPACEX - scrx, y + vspace, p->m_strText,
                    (bSel)? AXAppRes::TC_REV: AXAppRes::TC_NORMAL);
        }
    }

    dt.end();

    return TRUE;
}
