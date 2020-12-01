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

#include "AXListViewArea.h"

#include "AXListView.h"
#include "AXListViewItem.h"
#include "AXListViewItemManager.h"
#include "AXHeader.h"
#include "AXScrollBar.h"
#include "AXPixmap.h"
#include "AXApp.h"
#include "AXAppRes.h"
#include "AXFont.h"
#include "AXImageList.h"
#include "AXKey.h"


#define LVP             ((AXListView *)m_pParent)
#define ITEMSPACEX      3
#define CHECKBOXSIZE    13

/*!
    @class AXListViewArea
    @brief リストビューのリスト部分

    - ヘッダはなしの場合はヘッダは非表示（ヘッダ自体は作成される）

    @ingroup widgetsub
*/

/*
    ※ フォーカス位置が変わったら LVN_CHANGEFOCUS を通知すること。
*/


AXListViewArea::~AXListViewArea()
{
    delete m_pImg;
}

AXListViewArea::AXListViewArea(AXWindow *pParent,UINT uStyle,AXListViewItemManager *pDat)
    : AXScrollArea(pParent, uStyle)
{
    m_pDat = pDat;
    m_pImg = new AXPixmap;
    m_nLastClkColumn = -1;

    m_uFlags |= FLAG_REDRAW;

    //ヘッダ

    m_pHeader = new AXHeader(this,
                    (m_pParent->getStyle() & AXListView::LVS_NOHEADER)? WS_HIDE: 0,
                    0);

    m_pHeader->calcDefSize();
}

BOOL AXListViewArea::isShowScroll(int size,BOOL bHorz)
{
    if(bHorz)
    {
        //水平（ヘッダ）

        AXHeaderItem *p = m_pHeader->getTopItem();

        if(p && p->m_uFlags & AXHeaderItem::FLAG_EXPAND)
            //最初の列を拡張する場合はスクロールなし
            return FALSE;
        else
            return (size < m_pHeader->getAllWidth());
    }
    else
    {
        //垂直

        if(m_pParent->getStyle() & AXListView::LVS_NOHEADER)
            return (size < m_pDat->getCnt() * LVP->getItemH());
        else
            return (size - m_pHeader->getDefH() < m_pDat->getCnt() * LVP->getItemH());
    }
}


//==============================
//サブ
//==============================


//! 通知

void AXListViewArea::_notify(UINT notify)
{
    (m_pParent->getNotify())->sendNotify(m_pParent, notify, (ULONG)m_pDat->getFocusItem());
}

void AXListViewArea::_notify(UINT notify,AXListViewItem *pItem)
{
    (m_pParent->getNotify())->sendNotify(m_pParent, notify, (ULONG)pItem);
}

//! フォーカス位置が見えるようにスクロール位置調整

void AXListViewArea::_adjustScroll(int dir)
{
    if(scrV())
    {
        int scr = getVertPos();
        int y = m_pDat->getFocusPos() * LVP->getItemH();

        if(dir < 0 && y < scr)
            scrV()->setPos(y);
        else if(dir > 0 && y - scr > _getAreaH() - LVP->getItemH())
            scrV()->setPos(y - _getAreaH() + LVP->getItemH());
    }
}

//! リスト部分のY位置取得

int AXListViewArea::_getTopY()
{
    return (m_pParent->getStyle() & AXListView::LVS_NOHEADER)? 0: m_pHeader->getDefH();
}

//! リスト部分の高さ取得

int AXListViewArea::_getAreaH()
{
    return (m_pParent->getStyle() & AXListView::LVS_NOHEADER)? m_nH: m_nH - m_pHeader->getDefH();
}

//! マウス位置からアイテム取得

AXListViewItem *AXListViewArea::_getCurPosItem(int x,int y)
{
    y = y + getVertPos() - _getTopY();

    if(y >= 0 && y < m_pDat->getCnt() * LVP->getItemH())
        return (AXListViewItem *)m_pDat->getItem(y / LVP->getItemH());
    else
        return NULL;
}

//! 最後にクリックされたカラム位置セット

void AXListViewArea::_setLastClkColumn(int x)
{
    int y = 0;

    translateTo(m_pHeader, &x, &y);

    m_pHeader->getCurPosItem(x, &m_nLastClkColumn);
}

//! チェックボックスのON/OFF処理

BOOL AXListViewArea::_checkbox(AXListViewItem *pi,int x,int y)
{
    if(m_pParent->getStyle() & AXListView::LVS_CHECKBOX)
    {
        x += getHorzPos();

        y += getVertPos() - _getTopY();
        y = y - (y / LVP->getItemH()) * LVP->getItemH() - (LVP->getItemH() - CHECKBOXSIZE) / 2;

        if(x >= ITEMSPACEX && x < ITEMSPACEX + CHECKBOXSIZE && y >= 0 && y < CHECKBOXSIZE)
        {
            pi->m_uFlags ^= AXListViewItem::FLAG_CHECKED;
            _notify(AXListView::LVN_CHECKITEM, pi);

            return TRUE;
        }
    }

    return FALSE;
}

//! PageUp/Down

void AXListViewArea::_pageupdown(BOOL bUp)
{
    int pos;

    if(scrV() && scrV()->isVisible())
    {
        //スクロール

        pos = getVertPos();

        if(bUp) pos -= _getAreaH();
        else pos += _getAreaH();

        scrV()->setPos(pos);

        //フォーカス位置

        pos = getVertPos();

        if(bUp)
            pos += LVP->getItemH() - 1;
        else
            pos += _getAreaH() - LVP->getItemH();

        if(m_pDat->moveFocusFromPos(pos / LVP->getItemH()))
            _notify(AXListView::LVN_CHANGEFOCUS);

        redraw();
    }
}


//==============================
//ハンドラ
//==============================


//! サイズ変更時

BOOL AXListViewArea::onSize()
{
    m_pHeader->resize(m_nW, m_pHeader->getDefH());

    LVP->_setScrollInfo();

    m_pImg->recreate(m_nW, m_nH, 32, 32);

    redraw();

    return TRUE;
}

//! 通知

BOOL AXListViewArea::onNotify(AXWindow *pwin,UINT uNotify,ULONG lParam)
{
    if(pwin == m_pHeader && uNotify == AXHeader::HDN_RESIZE)
    {
        //ヘッダの列サイズが変更 -> 再構成

        LVP->setReconfig();
    }
    else if(pwin == this)
    {
        //スクロールされた

        if(uNotify == SAN_SCROLL_HORZ)
        {
            m_pHeader->setScroll(lParam);
            redraw();
        }
        else if(uNotify == SAN_SCROLL_VERT)
            redraw();
    }

    return TRUE;
}

//! ホイールスクロール

BOOL AXListViewArea::onMouseWheel(AXHD_MOUSE *phd,BOOL bUp)
{
    if(scrV())
    {
        int pos = getVertPos();

        if(bUp) pos -= LVP->getItemH() * 3;
        else pos += LVP->getItemH() * 3;

        if(scrV()->setPos(pos))
            redraw();
    }

    return TRUE;
}

//! ボタン押し時

BOOL AXListViewArea::onButtonDown(AXHD_MOUSE *phd)
{
    AXListViewItem *pi;
    int n;

    if(phd->button == BUTTON_LEFT || phd->button == BUTTON_RIGHT)
    {
        m_pParent->setFocus();

        //アイテム位置

        pi = _getCurPosItem(phd->x, phd->y);

        if(pi)
        {
            //カラム位置

            _setLastClkColumn(phd->x);

            //選択

            if(!(m_pParent->getStyle() & AXListView::LVS_MULTISEL))
                n = AXListViewItemManager::SELECT_SINGLE;
            else
            {
                if(phd->state & STATE_CTRL)
                    n = AXListViewItemManager::SELECT_MULTI_CTRL;
                else if(phd->state & STATE_SHIFT)
                    n = AXListViewItemManager::SELECT_MULTI_SHIFT;
                else
                    n = AXListViewItemManager::SELECT_MULTI;
            }

            if(m_pDat->selectItem(n, pi))
                _notify(AXListView::LVN_CHANGEFOCUS, pi);

            //チェックボックス

            if(phd->button == BUTTON_LEFT)
                _checkbox(pi, phd->x, phd->y);

            redraw();
        }

        //右ボタン

        if(phd->button == BUTTON_RIGHT)
            _notify(AXListView::LVN_RIGHTCLICK, pi);
    }

    return TRUE;
}

//! ダブルクリック

BOOL AXListViewArea::onDblClk(AXHD_MOUSE *phd)
{
    AXListViewItem *pi;

    if(phd->button == BUTTON_LEFT)
    {
        //アイテム位置

        pi = _getCurPosItem(phd->x, phd->y);
        if(!pi) return TRUE;

        //チェックボックス（範囲外の場合はダブルクリック通知）

        if(_checkbox(pi, phd->x, phd->y))
            redraw();
        else
            _notify(AXListView::LVN_DBLCLK, pi);
    }

    return TRUE;
}

//! キー押し時

BOOL AXListViewArea::onKeyDown(AXHD_KEY *phd)
{
    if(m_pParent->getStyle() & AXListView::LVS_MULTISEL)
    {
        if(phd->state == STATE_CTRL && (phd->keysym == 'a' || phd->keysym == 'A'))
        {
            //Ctrl+A（すべて選択）

            m_pDat->selectAll();
            _notify(AXListView::LVN_CHANGEFOCUS);
            redraw();

            return TRUE;
        }
    }

    //

    switch(phd->keysym)
    {
        //上
        case KEY_UP:
        case KEY_NUM_UP:
            if(m_pDat->updownFocus(FALSE))
            {
                _adjustScroll(-1);
                redraw();
                _notify(AXListView::LVN_CHANGEFOCUS);
            }
            break;
        //下
        case KEY_DOWN:
        case KEY_NUM_DOWN:
            if(m_pDat->updownFocus(TRUE))
            {
                _adjustScroll(1);
                redraw();
                _notify(AXListView::LVN_CHANGEFOCUS);
            }
            break;
        //PageUp
        case KEY_PAGEUP:
        case KEY_NUM_PAGEUP:
            _pageupdown(TRUE);
            break;
        //PageDown
        case KEY_PAGEDOWN:
        case KEY_NUM_PAGEDOWN:
            _pageupdown(FALSE);
            break;
        //Home
        case KEY_HOME:
        case KEY_NUM_HOME:
            if(scrV()) scrV()->setPos(0);

            if(m_pDat->moveFocusTopEnd(TRUE))
                _notify(AXListView::LVN_CHANGEFOCUS);

            redraw();
            break;
        //End
        case KEY_END:
        case KEY_NUM_END:
            if(scrV()) scrV()->setPosEnd();

            if(m_pDat->moveFocusTopEnd(FALSE))
                _notify(AXListView::LVN_CHANGEFOCUS);

            redraw();
            break;

        default:
            return FALSE;
    }

    return TRUE;
}

//! 描画

BOOL AXListViewArea::onPaint(AXHD_PAINT *phd)
{
    if(m_uFlags & FLAG_REDRAW)
        _draw();

    m_pImg->put(this, phd->x, phd->y, phd->w, phd->h);

    return TRUE;
}


//! イメージに描画

void AXListViewArea::_draw()
{
    AXHeaderItem *pHI;
    AXListViewItem *pi;
    AXImageList *pIL;
    AXWindow *pOD;
    AXHD_OWNERDRAW od;
    int x,y,scrx,topy,itemh,bRowLine,bSel,pos,posEnd,xx,spacey,col,hpos,itemno,tmp;

    if(!m_pImg->isExist()) return;

    scrx     = getHorzPos();
    topy     = _getTopY();
    itemh    = LVP->getItemH();
    bRowLine = m_pParent->getStyle() & AXListView::LVS_ROWLINE;
    spacey   = (itemh - m_pFont->getHeight()) / 2;

    //背景

    m_pImg->drawFillBox(0, 0, m_nW, m_nH, AXAppRes::FACEDOC);

    //アイテム

    AXDrawText dt(m_pImg->getid());

    pOD = m_pParent->getNotify();
    od.pwinFrom     = m_pParent;
    od.pDrawText    = &dt;
    od.uState       = (m_uFlags & FLAG_ENABLED)? ODS_ENABLED: 0;

    y = topy - getVertPos();

    for(pi = m_pDat->getTopItem(), itemno = 0; pi; y += itemh, pi = pi->next(), itemno++)
    {
        if(y + itemh <= 0) continue;
        if(y >= m_nH) break;

        //

        x    = -scrx;
        bSel = pi->isSelect();

        //選択背景色

        if(bSel)
        {
            if(m_pParent->isFocused())
                col = (m_pDat->getFocusItem() == pi)? AXAppRes::BACKREVTEXT: AXAppRes::BACKREVTEXTLT;
            else
                col = AXAppRes::FACEDARK;

            m_pImg->drawFillBox(0, y, m_nW, itemh, col);
        }
        else
            col = AXAppRes::FACEDOC;

        //各列

        for(pHI = m_pHeader->getTopItem(), pos = 0, hpos = 0; pHI; pHI = pHI->next(), hpos++)
        {
            posEnd = pi->m_strText.find('\t', pos);
            if(posEnd == -1) posEnd = pi->m_strText.getLen();

            if(x + pHI->m_nWidth > 0 && x < m_nW)
            {
                if(pHI->m_uFlags & AXHeaderItem::FLAG_COL_OWNERDRAW)
                {
                    //---- この列はオーナードロー

                    od.nItemNo  = itemno;
                    od.pItem    = (LPVOID)pi;
                    od.lParam1  = pi->m_lParam;
                    od.lParam2  = hpos;
                    od.rcs.set(x, y, pHI->m_nWidth, itemh);

                    if(bSel)
                        od.uState |= ODS_SELECTED | ((m_pDat->getFocusItem() == pi)? ODS_FOCUSED: 0);
                    else
                        od.uState &= ~(ODS_SELECTED | ODS_FOCUSED);

                    pOD->onOwnerDraw(this, &od);
                }
                else if(posEnd - pos > 0)
                {
                    //---- 通常テキスト

                    xx = x + ITEMSPACEX;

                    //最初の列のみ

                    if(hpos == 0)
                    {
                        //チェックボックス

                        if(m_pParent->getStyle() & AXListView::LVS_CHECKBOX)
                        {
                            if(bSel)
                                m_pImg->drawFillBox(xx, y + (itemh - CHECKBOXSIZE) / 2, CHECKBOXSIZE, CHECKBOXSIZE, AXAppRes::FACEDOC);

                            m_pImg->drawBox(xx, y + (itemh - CHECKBOXSIZE) / 2, CHECKBOXSIZE, CHECKBOXSIZE, AXAppRes::TEXTNORMAL);

                            if(pi->isCheck())
                                m_pImg->drawCheck(xx + (CHECKBOXSIZE-7)/2, y + (itemh - CHECKBOXSIZE)/2 + (CHECKBOXSIZE-7)/2, AXAppRes::TEXTNORMAL);

                            xx += CHECKBOXSIZE + 4;
                        }

                        //アイコン

                        pIL = LVP->getImageList();

                        if(pi->m_nIconNo >= 0 && pIL)
                        {
                            pIL->put(m_pImg->getid(), xx, y + (itemh - pIL->getHeight()) / 2, pi->m_nIconNo, FALSE);

                            xx += pIL->getOneW() + 3;
                        }

                        //チェックやアイコンがはみ出した場合は消す

                        if(xx >= x + pHI->m_nWidth)
                        {
                            tmp = x + pHI->m_nWidth - ITEMSPACEX;
                            if(tmp < x) tmp = x;

                            m_pImg->drawFillBox(tmp, y, xx - tmp, itemh, col);
                        }
                    }

                    //テキスト（右寄せは2番目のカラム以降）

                    if(hpos && pHI->m_uFlags & AXHeaderItem::FLAG_RIGHT)
                        xx = x + pHI->m_nWidth - m_pFont->getTextWidth(pi->m_strText.at(pos), posEnd - pos) - ITEMSPACEX;

                    dt.setClipRect(x + ITEMSPACEX, y, pHI->m_nWidth - ITEMSPACEX * 2, itemh);
                    dt.draw(*m_pFont, xx, y + spacey, pi->m_strText.at(pos), posEnd - pos, (bSel)? AXAppRes::TC_REV: AXAppRes::TC_NORMAL);
                }
            }

            //次へ

            if(pi->m_strText[posEnd] == '\t') posEnd++;
            pos = posEnd;

            x += pHI->m_nWidth;
        }

        //横グリッド線

        if(bRowLine)
            m_pImg->drawLineH(0, y + itemh - 1, m_nW, AXAppRes::FACE);
    }

    dt.end();

    //縦グリッド線

    if(m_pParent->getStyle() & AXListView::LVS_COLLINE)
    {
        for(pHI = m_pHeader->getTopItem(), x = -scrx - 1; pHI; pHI = pHI->next())
        {
            x += pHI->m_nWidth;

            if(x >= 0 && x < m_nW)
                m_pImg->drawLineV(x, topy, m_nH - topy, AXAppRes::FACE);
        }
    }
}
