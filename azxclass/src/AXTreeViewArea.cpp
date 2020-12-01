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

#include "AXTreeViewArea.h"

#include "AXTreeView.h"
#include "AXTreeViewItemManager.h"
#include "AXScrollBar.h"
#include "AXPixmap.h"
#include "AXAppRes.h"
#include "AXFont.h"
#include "AXImageList.h"
#include "AXKey.h"


//---------------------------

#define EXPANDBOXSIZE   11
#define EXPANDBOXSPACE  4
#define CHECKBOXSIZE    13
#define CHECKBOXSPACE   3
#define ICONSPACE       3
#define SPACEITEMY      1
#define SPACEVIEW       3

#define BTTF_DOWN       1
#define BTTF_DRAG       2
#define BTTF_CANCEL     3

#define TVP     ((AXTreeView *)m_pParent)
#define NOTIFY  (m_pParent->getNotify())

//------------------------

/*!
    @class AXTreeViewArea
    @brief ツリービュー、表示部分

    @ingroup widgetsub
*/


AXTreeViewArea::~AXTreeViewArea()
{
    delete m_pImg;
}

AXTreeViewArea::AXTreeViewArea(AXWindow *pParent,UINT uStyle,AXTreeViewItemManager *pDat)
    : AXScrollArea(pParent, uStyle)
{
    m_pDat = pDat;
    m_pImg = new AXPixmap;

    m_fBtt = 0;
    m_pDragItem = NULL;

    m_uFlags |= FLAG_REDRAW;
}

//! スクロール表示するか

BOOL AXTreeViewArea::isShowScroll(int size,BOOL bHorz)
{
    if(bHorz)
        return size < TVP->m_nMaxWidth;
    else
        return size < TVP->m_nMaxHeight;

    return TRUE;
}


//==============================
//サブ
//==============================


//! 通知

void AXTreeViewArea::_notify(UINT notify,ULONG param)
{
    (m_pParent->getNotify())->sendNotify(m_pParent, notify, param);
}

//! アイテム展開反転

void AXTreeViewArea::_expandItem(AXTreeViewItem *p)
{
    p->m_uFlags ^= AXTreeViewItem::FLAG_EXPAND;

    TVP->_updateDat();

    //フォーカスアイテムが隠れた場合は選択変更

    if(!m_pDat->isFocusVisible())
    {
        if(m_pDat->setFocusItem(p))
            _notify(AXTreeView::TVN_SELCHANGE, 0);
    }

    //通知

    _notify(AXTreeView::TVN_EXPAND, (ULONG)p);
}

//! カーソル位置からアイテム取得
/*!
    @param pType どの上に乗っているか（AXTreeViewArea::ONTYPE）。NULLで取得しない。
    @param pTopY アイテムのY位置（ウィンドウ座標）。NULLで取得しない。
*/

AXTreeViewItem *AXTreeViewArea::_getCurItem(int x,int y,LPINT pType,LPINT pTopY)
{
    AXTreeViewItem *p;
    int type,topy,h;

    h = TVP->m_nItemH;

    //アイテム取得

    x += getHorzPos() - SPACEVIEW;
    y += getVertPos() - SPACEVIEW;

    p = m_pDat->getCurItem(x, y, h, &topy);
    if(!p) return NULL;

    //どの上か

    x -= p->m_nX;
    y -= topy;

    if(p->m_pFirst && x >= 0 && x < EXPANDBOXSIZE &&
        y >= (h - EXPANDBOXSIZE) / 2 && y < (h - EXPANDBOXSIZE) / 2 + EXPANDBOXSIZE)
        //展開ボックス
        type = ONTYPE_EXPAND;
    else if(p->isCheckBox() &&
        x >= EXPANDBOXSIZE + EXPANDBOXSPACE && x < EXPANDBOXSIZE + EXPANDBOXSPACE + CHECKBOXSIZE &&
        y >= (h - CHECKBOXSIZE) / 2 && y < (h - CHECKBOXSIZE) / 2 + CHECKBOXSIZE)
        //チェックボックス
        type = ONTYPE_CHECKBOX;
    else
        type = ONTYPE_NONE;

    //

    if(pType) *pType = type;
    if(pTopY) *pTopY = topy - getVertPos() + SPACEVIEW;

    return p;

}

//! フォーカス位置が見えるようにスクロール位置調整

void AXTreeViewArea::_adjustScroll(int dir)
{
    if(scrV())
    {
        int scr = getVertPos();
        int y = SPACEVIEW + m_pDat->getFocusYPos(TVP->m_nItemH);

        if(dir <= 0 && y < scr)
            scrV()->setPos(y);
        else if(dir >= 0 && y - scr > m_nH - TVP->m_nItemH)
            scrV()->setPos(y - m_nH + TVP->m_nItemH);
    }
}


//==============================
//ハンドラ
//==============================


//! サイズ変更時

BOOL AXTreeViewArea::onSize()
{
    TVP->_setScrollInfo();

    m_pImg->recreate(m_nW, m_nH, 32, 32);

    redraw();

    return TRUE;
}

//! 通知

BOOL AXTreeViewArea::onNotify(AXWindow *pwin,UINT uNotify,ULONG lParam)
{
    //スクロール

    if(pwin == this && (uNotify == SAN_SCROLL_HORZ || uNotify == SAN_SCROLL_VERT))
        redraw();

    return TRUE;
}

//! ホイール

BOOL AXTreeViewArea::onMouseWheel(AXHD_MOUSE *phd,BOOL bUp)
{
    if(scrV())
    {
        int pos = getVertPos();

        if(bUp) pos -= TVP->m_nItemH * 3;
        else pos += TVP->m_nItemH * 3;

        if(scrV()->setPos(pos))
            redraw();
    }

    return TRUE;
}

//! ボタン押し時

BOOL AXTreeViewArea::onButtonDown(AXHD_MOUSE *phd)
{
    AXTreeViewItem *p;
    int ontype,topy;

    if(m_fBtt) return TRUE;

    if(phd->button == BUTTON_LEFT || phd->button == BUTTON_RIGHT)
    {
        m_pParent->setFocus();

        //

        p = _getCurItem(phd->x, phd->y, &ontype, &topy);

        //----- 左ボタン

        if(p && phd->button == BUTTON_LEFT)
        {
            if(ontype == ONTYPE_EXPAND)
            {
                //展開

                _expandItem(p);
            }
            else if(ontype == ONTYPE_CHECKBOX)
            {
                //チェック

                p->m_uFlags ^= AXTreeViewItem::FLAG_CHECKED;
                redraw();

                _notify(AXTreeView::TVN_CHECKITEM, (ULONG)p);
            }
            else
            {
                //通常選択

                if(m_pDat->setFocusItem(p))
                {
                    redraw();
                    _notify(AXTreeView::TVN_SELCHANGE, 0);
                }

                //D&D 判定開始

                if(m_pParent->getStyle() & AXTreeView::TVS_DND)
                {
                    m_fBtt         = BTTF_DOWN;
                    m_pDragItem    = p;
                    m_pDragDstItem = NULL;
                    m_nDragTopY    = topy;

                    grabPointer();
                }
            }
        }

        //----- 右ボタン

        if(phd->button == BUTTON_RIGHT)
        {
            //選択

            if(p && m_pDat->setFocusItem(p))
            {
                _notify(AXTreeView::TVN_SELCHANGE, 0);
                redraw();
            }

            //通知

            _notify(AXTreeView::TVN_RBUTTON, (ULONG)p);
        }
    }

    return TRUE;
}

//! ダブルクリック時

BOOL AXTreeViewArea::onDblClk(AXHD_MOUSE *phd)
{
    AXTreeViewItem *p;
    int ontype;

    if(m_fBtt) return TRUE;

    if(phd->button == BUTTON_LEFT)
    {
        p = _getCurItem(phd->x, phd->y, &ontype);

        //※チェック部分は通常のボタン押しで処理

        if(p && ontype != ONTYPE_CHECKBOX)
        {
            if(p->m_pFirst)
                //子がある -> 展開
                _expandItem(p);
            else
                //子がない場合は通知
                _notify(AXTreeView::TVN_DBLCLK, (ULONG)p);

            return TRUE;
        }
    }

    //通常のボタン押し
    return FALSE;
}

//! ボタン離し時

BOOL AXTreeViewArea::onButtonUp(AXHD_MOUSE *phd)
{
    if(m_fBtt && phd->button == BUTTON_LEFT)
    {
        AXTreeViewItem *pItem[2];
        int flag = m_fBtt;

        m_fBtt = 0;
        ungrabPointer();

        //タイマ

        if(m_uFlags & FLAG_TIMER)
        {
            delTimer(0);
            m_uFlags &= ~FLAG_TIMER;
        }

        //枠を消す

        if(flag == BTTF_DRAG && m_pDragDstItem)
            redraw();

        //通知（ドロップ）

        if(flag == BTTF_DRAG)
        {
            pItem[0] = m_pDragItem;
            pItem[1] = m_pDragDstItem;

            NOTIFY->onNotify(m_pParent, AXTreeView::TVN_DRAG_END, (ULONG)pItem);
        }

        m_pDragItem = NULL;
    }

    return TRUE;
}

//! マウス移動時

BOOL AXTreeViewArea::onMouseMove(AXHD_MOUSE *phd)
{
    AXTreeViewItem *p,*pDst;

    //開始判定中またはドラッグ中の場合

    if(m_fBtt == BTTF_DOWN || m_fBtt == BTTF_DRAG)
    {
        pDst = m_pDragDstItem;

        p = _getCurItem(phd->x, phd->y, NULL);

        //範囲外に出ている間スクロール

        if((phd->y < 0 || phd->y >= m_nH) && scrV())
        {
            if(!(m_uFlags & FLAG_TIMER))
            {
                addTimer(0, 80, (phd->y < 0));
                m_uFlags |= FLAG_TIMER;
            }
        }
        else if(m_uFlags & FLAG_TIMER)
        {
            delTimer(0);
            m_uFlags &= ~FLAG_TIMER;
        }

        //

        if(!p)
            pDst = NULL;
        else
        {
            if(m_fBtt == BTTF_DOWN)
            {
                //-------- 開始判定中の場合
                /*
                    一つ上 or 下のアイテムの境界近くまで来た時に開始。
                    （アイテムが変わった時だけで判定すると、マウスがちょっと滑っただけでD&Dになってしまうため）
                */

                if(p != m_pDragItem &&
                   (phd->y < m_nDragTopY - TVP->m_nItemH + 2 || phd->y > m_nDragTopY + TVP->m_nItemH * 2 - 2))
                {
                    if(NOTIFY->onNotify(m_pParent, AXTreeView::TVN_DRAG_BEGIN, (ULONG)m_pDragItem))
                    {
                        //ドラッグ開始

                        m_fBtt = BTTF_DRAG;

                        if(NOTIFY->onNotify(m_pParent, AXTreeView::TVN_DRAG_DST, (ULONG)p))
                            pDst = p;
                    }
                    else
                    {
                        //ドラッグ中止（ボタンが離されるまで何もしない）

                        m_fBtt = BTTF_CANCEL;
                    }
                }
            }
            else
            {
                //------- ドラッグ中

                if(p == m_pDragItem)
                    pDst = NULL;
                else if(p != m_pDragDstItem)
                {
                    if(NOTIFY->onNotify(m_pParent, AXTreeView::TVN_DRAG_DST, (ULONG)p))
                        pDst = p;
                    else
                        pDst = NULL;
                }
            }
        }

        //ドロップ先変更

        if(m_fBtt == BTTF_DRAG && pDst != m_pDragDstItem)
        {
            m_pDragDstItem = pDst;
            redraw();
        }
    }

    return TRUE;
}

//! タイマー

BOOL AXTreeViewArea::onTimer(UINT uTimerID,ULONG lParam)
{
    int pos = getVertPos() + (lParam? -TVP->m_nItemH: TVP->m_nItemH);

    if(scrV()->setPos(pos))
        redraw();

    return TRUE;
}

//! キー押し時

BOOL AXTreeViewArea::onKeyDown(AXHD_KEY *phd)
{
    if(m_fBtt) return TRUE;

    switch(phd->keysym)
    {
        //上
        case KEY_UP:
        case KEY_NUM_UP:
            if(m_pDat->moveFocusUpDown(TRUE))
            {
                _adjustScroll(-1);
                redraw();
                _notify(AXTreeView::TVN_SELCHANGE, 0);
            }
            break;
        //下
        case KEY_DOWN:
        case KEY_NUM_DOWN:
            if(m_pDat->moveFocusUpDown(FALSE))
            {
                _adjustScroll(1);
                redraw();
                _notify(AXTreeView::TVN_SELCHANGE, 0);
            }
            break;
        //スペース（展開）
        case KEY_SPACE:
        case KEY_NUM_SPACE:
            if(m_pDat->isFocusEnableExpand())
                _expandItem(m_pDat->getFocusItem());
            break;
        //Home
        case KEY_HOME:
        case KEY_NUM_HOME:
            if(m_pDat->moveFocusHomeEnd(TRUE))
            {
                if(scrV()) scrV()->setPos(0);

                redraw();
                _notify(AXTreeView::TVN_SELCHANGE, 0);
            }
            break;
        //End
        case KEY_END:
        case KEY_NUM_END:
            if(m_pDat->moveFocusHomeEnd(FALSE))
            {
                if(scrV()) scrV()->setPosEnd();

                redraw();
                _notify(AXTreeView::TVN_SELCHANGE, 0);
            }
            break;

        default:
            return FALSE;
    }

    return TRUE;
}

//! 描画

BOOL AXTreeViewArea::onPaint(AXHD_PAINT *phd)
{
    if(m_uFlags & FLAG_REDRAW)
        _drawimg();

    m_pImg->put(this, phd->x, phd->y, phd->w, phd->h);

    return TRUE;
}

//! イメージに描画

void AXTreeViewArea::_drawimg()
{
    AXTreeViewItem *p,*pFocus;
    AXImageList *pImgList;
    int x,y,yy,h,ty,bEnable,col;

    //背景

    m_pImg->drawFillBox(0, 0, m_nW, m_nH, AXAppRes::FACEDOC);

    //--------- アイテム

    AXDrawText dt(m_pImg->getid());

    y        = SPACEVIEW - getVertPos();
    h        = TVP->m_nItemH;
    ty       = (h - m_pFont->getHeight()) / 2;
    pImgList = TVP->m_pImgList;
    pFocus   = TVP->getFocusItem();
    bEnable  = (m_pParent->isEnabled() && m_pParent->isFocused());

    p = TVP->getTopItem();

    while(p)
    {
        //--------- アイテム描画

        if(y > -h && y < m_nH)
        {
            x = p->m_nX + SPACEVIEW - getHorzPos();

            //選択背景

            if(p == pFocus)
                m_pImg->drawFillBox(0, y, m_nW, h, (bEnable)? AXAppRes::BACKREVTEXT: AXAppRes::FACEDARK);

            //展開ボックス

            if(p->m_pFirst)
            {
                yy = y + (h - EXPANDBOXSIZE) / 2;

                m_pImg->drawBox(x, yy, EXPANDBOXSIZE, EXPANDBOXSIZE, AXAppRes::BLACK);
                m_pImg->drawFillBox(x + 1, yy + 1, EXPANDBOXSIZE - 2, EXPANDBOXSIZE - 2, AXAppRes::WHITE);

                m_pImg->drawLineH(x + 2, yy + EXPANDBOXSIZE / 2, EXPANDBOXSIZE - 4, AXAppRes::BLACK);

                if(!p->isExpand())
                    m_pImg->drawLineV(x + EXPANDBOXSIZE / 2, yy + 2, EXPANDBOXSIZE - 4, AXAppRes::BLACK);
            }

            x += EXPANDBOXSIZE + EXPANDBOXSPACE;

            //チェックボックス

            if(p->isCheckBox())
            {
                yy = y + (h - CHECKBOXSIZE) / 2;

                m_pImg->drawBox(x, yy, CHECKBOXSIZE, CHECKBOXSIZE, AXAppRes::BLACK);
                m_pImg->drawFillBox(x + 1, yy + 1, CHECKBOXSIZE - 2, CHECKBOXSIZE - 2, AXAppRes::WHITE);

                if(p->isChecked())
                    m_pImg->drawCheck(x + (CHECKBOXSIZE - 7) / 2, yy + (CHECKBOXSIZE - 7) / 2, AXAppRes::BLACK);

                x += CHECKBOXSIZE + CHECKBOXSPACE;
            }

            //アイコン

            if(pImgList && p->m_nIconNo >= 0)
            {
                pImgList->put(m_pImg->getid(), x, y + (h - pImgList->getHeight()) / 2, p->m_nIconNo);

                x += pImgList->getOneW() + ICONSPACE;
            }

            //テキスト

            if(p == pFocus)
                col = AXAppRes::TC_REV;
            else
                col = (p->isGray())? AXAppRes::TC_DISABLE: AXAppRes::TC_NORMAL;

            dt.draw(*m_pFont, x, y + ty, p->m_strText, col);

            //D&D ドロップ先枠

            if(m_fBtt == BTTF_DRAG && p == m_pDragDstItem)
                m_pImg->drawBox(-getHorzPos(), y, (m_nW > TVP->m_nMaxWidth)? m_nW: TVP->m_nMaxWidth, h, AXAppRes::BACKREVTEXTLT);
        }

        //--------- 次

        y += h;

        p = p->nextVisible();
    }

    dt.end();
}
