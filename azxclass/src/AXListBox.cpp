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

#include "AXListBox.h"

#include "AXListBoxItem.h"
#include "AXListBoxItemManager.h"
#include "AXListBoxArea.h"
#include "AXScrollBar.h"
#include "AXFont.h"
#include "AXApp.h"
#include "AXUtilStr.h"


#define AREA    ((AXListBoxArea *)m_pScrArea)

//--------------------------

/*!
    @class AXListBox
    @brief リストボックスウィジェット

    - AXScrollView のスタイルも含む。スクロールバーの有無はそちらで指定する必要がある。
    - 単体選択のみ。
    - アイテム位置指定（pos）では、-1 で現在の選択位置となる。

    <h2>[オーナードロー]</h2>
    lParam1 : アイテムのパラメータ値 @n
    lParam2 : アイテムの文字列(LPUSTR)
    pItem   : AXListBoxItem*

    @ingroup widget
*/

/*!
    @var AXListBox::LBS_OWNERDRAW
    @brief アイテムをオーナードロー
    @var AXListBox::LBS_FOCUSOUT_SELOUT
    @brief フォーカスがなくなったら選択もなしにする

    @var AXListBox::LBN_SELCHANGE
    @brief マウス操作やキー操作によって選択が変更された
    @var AXListBox::LBN_DBLCLK
    @brief 左ボタンがダブルクリックされた（アイテム上）
*/



AXListBox::~AXListBox()
{
    delete m_pDat;
}

AXListBox::AXListBox(AXWindow *pParent,UINT uStyle,UINT uLayoutFlags)
    : AXScrollView(pParent, uStyle, uLayoutFlags)
{
    _createListBox();
}

AXListBox::AXListBox(AXWindow *pParent,UINT uStyle,UINT uLayoutFlags,UINT uItemID,DWORD dwPadding)
    : AXScrollView(pParent, uStyle, uLayoutFlags, uItemID, dwPadding)
{
    _createListBox();
}

//! 作成

void AXListBox::_createListBox()
{
    m_uType = TYPE_LISTBOX;
    m_uFlags |= FLAG_TAKE_FOCUS;

    m_pDat = new AXListBoxItemManager;

    m_pScrArea = new AXListBoxArea(this, this,
                    (m_uStyle & LBS_OWNERDRAW)? AXListBoxArea::LBAS_OWNERDRAW: 0, m_pDat);
}

//! アイテム数の変化による再構成

void AXListBox::reconfig()
{
    AREA->setScrVInfo();

    AXScrollView::reconfig();

    m_pScrArea->redraw();
}


//==========================
//
//==========================


//! 水平スクロールの幅セット

void AXListBox::setScrollWidth(int width)
{
    if(m_pScrH)
    {
        m_pScrH->setStatus(0, width, m_pScrArea->getWidth());
        setReconfig();
    }
}

//! アイテム高さセット

void AXListBox::setItemHeight(int h)
{
    AREA->setItemH(h);
}

//! アイテム数取得

int AXListBox::getItemCnt()
{
    return m_pDat->getCnt();
}

//! 先頭アイテム取得

AXListBoxItem *AXListBox::getTopItem()
{
    return m_pDat->getTopItem();
}

//! 最後のアイテム取得

AXListBoxItem *AXListBox::getBottomItem()
{
    return (AXListBoxItem *)m_pDat->getBottom();
}

//! 選択アイテム取得

AXListBoxItem *AXListBox::getSelItem()
{
    return m_pDat->getSelItem();
}

//------------------

//! アイテム追加
//! @return 追加された位置

int AXListBox::addItem(LPCUSTR pstr)
{
    setReconfig();

    return m_pDat->addItem(pstr, 0);
}

//! アイテム追加

int AXListBox::addItem(LPCUSTR pstr,ULONG lParam)
{
    setReconfig();

    return m_pDat->addItem(pstr, lParam);
}

//! アイテム追加(ASCII文字列)

int AXListBox::addItem(LPCSTR pstr,ULONG lParam)
{
    AXString str(pstr);

    setReconfig();

    return m_pDat->addItem(str, lParam);
}

//! アイテム追加（直接データを）

int AXListBox::addItem(AXListBoxItem *pItem)
{
    setReconfig();

    return m_pDat->addItem(pItem);
}

//! 文字列IDから連続した複数個セット

void AXListBox::addItemTrMul(int startID,int cnt)
{
    for(; cnt > 0; cnt--, startID++)
        m_pDat->addItem(_str(startID), 0);
}

//! アイテム挿入

int AXListBox::insertItem(int pos,LPCUSTR pstr,ULONG lParam)
{
    setReconfig();

    return m_pDat->insertItem(pos, pstr, lParam);
}

//! アイテムすべて削除

void AXListBox::deleteItemAll()
{
    m_pDat->deleteAllItem();
    setReconfig();
}

//! アイテム削除

void AXListBox::deleteItem(int pos)
{
    m_pDat->deleteItemPos(pos);
    setReconfig();
}

//-------------

//! 現在の選択番号取得
//! @return -1 で選択なし

int AXListBox::getCurSel()
{
    return m_pDat->getSel();
}

//! 選択変更

void AXListBox::setCurSel(int pos)
{
    if(m_pDat->setSel(pos))
        m_pScrArea->redraw();
}

//! 選択変更（アイテムポインタから）

void AXListBox::setCurSel(AXListBoxItem *pItem)
{
    if(m_pDat->setSel(pItem))
        m_pScrArea->redraw();
}

//-----------------

//! テキスト取得

void AXListBox::getItemText(int pos,AXString *pstr)
{
    m_pDat->getItemText(pos, pstr);
}

//! パラメータ値取得

ULONG AXListBox::getItemParam(int pos)
{
    return m_pDat->getItemParam(pos);
}

//! テキストセット

void AXListBox::setItemText(int pos,LPCUSTR pstr)
{
    if(m_pDat->setItemText(pos, pstr))
        m_pScrArea->redraw();
}

//! パラメータ値セット

void AXListBox::setItemParam(int pos,ULONG lParam)
{
    m_pDat->setItemParam(pos, lParam);
}

//-------------------

//! パラメータ値検索
//! @param 見つかった場合、アイテム位置。-1 で見つからなかった。

int AXListBox::findItemParam(ULONG lParam)
{
    return m_pDat->findItemParam(lParam);
}

//! アイテムの位置を上下に移動

BOOL AXListBox::moveItemUpDown(int pos,BOOL bDown)
{
    if(m_pDat->moveItemUpDown(pos, bDown))
    {
        m_pScrArea->redraw();
        return TRUE;
    }

    return FALSE;
}

//! ソート関数

int axlistbox_sort_text(AXListItem *pItem1,AXListItem *pItem2,ULONG lParam)
{
    return ((AXListBoxItem *)pItem1)->m_strText.compare(((AXListBoxItem *)pItem2)->m_strText);
}

//! ソート
/*!
    @param func   比較関数。1番目のアイテムが2番目より大きければ 0 より大きい値を返す。NULL でテキストを昇降順に。
    @param lParam 比較関数に渡すパラメータ。
*/

void AXListBox::sortItem(int (*func)(AXListItem*,AXListItem*,ULONG),ULONG lParam)
{
    m_pDat->sort((func == NULL)? axlistbox_sort_text: func, lParam);

    m_pScrArea->redraw();
}

//! 全アイテムの幅を最小サイズにする
/*!
    @param bScr 垂直スクロール分も含める
*/

void AXListBox::setAutoWidth(BOOL bScr)
{
    AXListBoxItem *p;
    int w,max = 0;

    for(p = m_pDat->getTopItem(); p; p = p->next())
    {
        w = m_pFont->getTextWidth(p->m_strText);
        if(w > max) max = w;
    }

    max += 4;
    if(m_uStyle & (AXScrollView::SVS_SUNKEN|AXScrollView::SVS_FRAME)) max += 2;
    if(bScr && m_pScrV) max += m_pScrV->getScrollBoxWidth();

    m_nMinW = max;
}

//! 全アイテムの高さを最小サイズにする

void AXListBox::setAutoHeight()
{
    m_nMinH = m_pDat->getCnt() * AREA->getItemH();

    if(m_uStyle & (AXScrollView::SVS_SUNKEN|AXScrollView::SVS_FRAME)) m_nMinH += 2;
}

//! 終端へスクロール

void AXListBox::setScrollEnd()
{
    if(m_pScrV)
    {
        //アイテム追加直後は reconfig() が実行されていないのでスクロール情報セット
        AREA->setScrVInfo();

        if(m_pScrV->setPosEnd())
            m_pScrArea->redraw();
    }
}

//! 指定アイテム位置を基準にスクロール
/*!
    @param align   [0]上端 [1]中央
*/

void AXListBox::setScrollItem(AXListItem *pItem,int align)
{
    int y;

    y = m_pDat->getPos(pItem) * AREA->getItemH();

    if(align == 1)
        y -= (m_pScrArea->getHeight() - AREA->getItemH()) / 2;

    if(m_pScrV)
    {
        //初期化中は reconfig() が実行されていないのでスクロール情報セット
        AREA->setScrVInfo();

        if(m_pScrV->setPos(y))
            m_pScrArea->redraw();
    }
}

//! 表示領域更新

void AXListBox::updateArea()
{
    m_pScrArea->redraw();
}


//==========================
//ハンドラ
//==========================


//! フォーカスIN

BOOL AXListBox::onFocusIn(int detail)
{
    m_pScrArea->redraw();
    return FALSE;
}

//! フォーカスOUT

BOOL AXListBox::onFocusOut(int detail)
{
    if(m_uStyle & LBS_FOCUSOUT_SELOUT)
        m_pDat->setSel(-1);

    m_pScrArea->redraw();

    return FALSE;
}

//! 通知

BOOL AXListBox::onNotify(AXWindow *pwin,UINT uNotify,ULONG uParam)
{
    if(AXScrollView::onNotify(pwin, uNotify, uParam)) return TRUE;

    if(pwin == m_pScrArea)
    {
        switch(uNotify)
        {
            //選択変更
            case AXListBoxArea::LBAN_SELCHANGE_MOUSE:
            case AXListBoxArea::LBAN_SELCHANGE_KEY:
                getNotify()->sendNotify(this, LBN_SELCHANGE, uParam);
                break;
            //ダブルクリック
            case AXListBoxArea::LBAN_DBLCLK:
                getNotify()->sendNotify(this, LBN_DBLCLK, uParam);
                break;
        }
    }

    return TRUE;
}

//! キー押し

BOOL AXListBox::onKeyDown(AXHD_KEY *phd)
{
    return m_pScrArea->onKeyDown(phd);
}
