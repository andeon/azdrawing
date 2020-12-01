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

#include "AXTreeView.h"

#include "AXTreeViewItemManager.h"
#include "AXTreeViewArea.h"
#include "AXImageList.h"
#include "AXFont.h"
#include "AXScrollBar.h"


//--------------------------

#define SPACELEVEL      14
#define EXPANDBOXSIZE   11
#define EXPANDBOXSPACE  4
#define CHECKBOXSIZE    13
#define CHECKBOXSPACE   3
#define ICONSPACE       3
#define SPACEITEMY      1
#define SPACEVIEW       3

//--------------------------

/*!
    @class AXTreeView
    @brief ツリービューウィジェット

    - AXScrollView のスタイルも有効。スクロールバーの有無は AXScrollView のスタイルで指定する。

    @ingroup widget
*/

/*!
    @var AXTreeView::TVS_DND
    @brief アイテムのD＆Dを有効にする（通知で処理）

    @var AXTreeView::TVN_SELCHANGE
    @brief 選択が変更された（lParam = 0）
    @var AXTreeView::TVN_EXPAND
    @brief 展開状態が変更された（lParam = AXTreeViewItem*）
    @var AXTreeView::TVN_CHECKITEM
    @brief チェック状態が変更された（lParam = AXTreeViewItem*）
    @var AXTreeView::TVN_RBUTTON
    @brief 右ボタンが押された（lParam = AXTreeViewItem*。押されたアイテム。NULLで範囲外）
    @var AXTreeView::TVN_DBLCLK
    @brief 左ボタンがダブルクリックされた（lParam = AXTreeViewItem*）

    @var AXTreeView::TVN_DRAG_BEGIN
    @brief ドラッグ開始（lParam = AXTreeViewItem*。ドラッグ元のアイテム）

    TRUE を返すとドラッグ開始。FALSE を返すとドラッグ中止。

    @var AXTreeView::TVN_DRAG_DST
    @brief ドロップ先判定（lParam = AXTreeViewItem*。ドロップ先のアイテム）

    TRUE を返すとドロップ先として承認。FALSE を返すとドロップ先として認めない。

    @var AXTreeView::TVN_DRAG_END
    @brief ドラッグ終了（lParam = AXTreeViewItem*[2]）

    配列の [0] にはドラッグ元アイテム。[1] にはドロップ先アイテム（NULL でドロップ先なし）@n
    ドラッグ元とドロップ先が同じ場合は [1] が NULL となる。
*/


AXTreeView::~AXTreeView()
{
    delete m_pDat;
}

AXTreeView::AXTreeView(AXWindow *pParent,UINT uStyle,UINT uLayoutFlags)
    : AXScrollView(pParent, uStyle, uLayoutFlags)
{
    _createTreeView();
}

AXTreeView::AXTreeView(AXWindow *pParent,UINT uStyle,UINT uLayoutFlags,UINT uItemID,DWORD dwPadding)
    : AXScrollView(pParent, uStyle, uLayoutFlags, uItemID, dwPadding)
{
    _createTreeView();
}

//! 作成

void AXTreeView::_createTreeView()
{
    m_uType  = TYPE_TREEVIEW;
    m_uFlags |= FLAG_TAKE_FOCUS;

    m_pDat      = new AXTreeViewItemManager;
    m_pImgList  = NULL;
    m_nMaxWidth  = 0;
    m_nMaxHeight = 0;

    m_pScrArea = new AXTreeViewArea(this, 0, m_pDat);

    _setItemH();
}

//! 再構成

void AXTreeView::reconfig()
{
    _setItemH();

    //データの変更時：サイズ変更時などは処理しない

    if(m_uFlags & FLAG_DATRECONFIG)
    {
        _calcReconfig();
        m_uFlags &= ~FLAG_DATRECONFIG;
    }

    _setScrollInfo();

    AXScrollView::reconfig();

    m_pScrArea->redraw();
}


//==========================
//サブ
//==========================


//! データが更新された

void AXTreeView::_updateDat()
{
    m_uFlags |= FLAG_DATRECONFIG;
    setReconfig();
}

//! アイテム高さセット

void AXTreeView::_setItemH()
{
    m_nItemH = m_pFont->getHeight();
    if(m_nItemH < EXPANDBOXSIZE) m_nItemH = EXPANDBOXSIZE;
    if(m_pImgList && m_nItemH < m_pImgList->getHeight()) m_nItemH = m_pImgList->getHeight();

    m_nItemH += SPACEITEMY * 2;
}

//! スクロール情報セット

void AXTreeView::_setScrollInfo()
{
    if(m_pScrH)
        m_pScrH->setStatus(0, m_nMaxWidth, m_pScrArea->getWidth());

    if(m_pScrV)
        m_pScrV->setStatus(0, m_nMaxHeight, m_pScrArea->getHeight());
}

//! アイテム初期化

void AXTreeView::_initItem(AXTreeViewItem *pItem)
{
    //幅

    pItem->m_nW = EXPANDBOXSIZE + EXPANDBOXSPACE;
    if(pItem->m_uFlags & AXTreeViewItem::FLAG_CHECKBOX) pItem->m_nW += CHECKBOXSIZE + CHECKBOXSPACE;
    if(m_pImgList && pItem->m_nIconNo >= 0) pItem->m_nW += m_pImgList->getOneW() + ICONSPACE;
    pItem->m_nW += m_pFont->getTextWidth(pItem->m_strText);
}

//! 最大幅・最大高さ・各X位置 計算

void AXTreeView::_calcReconfig()
{
    m_pDat->calcReconfig(SPACELEVEL, m_nItemH, &m_nMaxWidth, &m_nMaxHeight);

    m_nMaxWidth  += SPACEVIEW * 2;
    m_nMaxHeight += SPACEVIEW * 2;
}


//==========================
//
//==========================


//! 最初のアイテム取得

AXTreeViewItem *AXTreeView::getTopItem()
{
    return m_pDat->getTopItem();
}

//! フォーカスのあるアイテム取得

AXTreeViewItem *AXTreeView::getFocusItem()
{
    return m_pDat->getFocusItem();
}

//! ドラッグ中のドラッグ元アイテム取得

AXTreeViewItem *AXTreeView::getDragSrcItem()
{
    return ((AXTreeViewArea *)m_pScrArea)->getDragSrcItem();
}

//! アイコンのイメージリストセット

void AXTreeView::setImageList(AXImageList *pImg)
{
    m_pImgList = pImg;
    _setItemH();
}

//! ツリー表示部分を再描画

void AXTreeView::redrawArea()
{
    m_pScrArea->redraw();
}

//! アイテム更新

void AXTreeView::updateItem(AXTreeViewItem *pItem)
{
    _initItem(pItem);
    _updateDat();
}

//! アイテム新規追加（親の最後に）
/*!
    @param pParent NULL でルート
    @param nIconNo -1 でなし
*/

AXTreeViewItem *AXTreeView::addItem(AXTreeItem *pParent,LPCUSTR pText,int nIconNo,UINT uFlags,ULONG lParam)
{
    AXTreeViewItem *p;

    p = (AXTreeViewItem *)m_pDat->add(pParent, new AXTreeViewItem(pText, nIconNo, uFlags, lParam));
    _initItem(p);

    _updateDat();

    return p;
}

//! アイテム新規挿入

AXTreeViewItem *AXTreeView::insertItem(AXTreeItem *pInsert,LPCUSTR pText,int nIconNo,UINT uFlags,ULONG lParam)
{
    AXTreeViewItem *p;

    p = (AXTreeViewItem *)m_pDat->insert(pInsert, new AXTreeViewItem(pText, nIconNo, uFlags, lParam));
    _initItem(p);

    _updateDat();

    return p;
}

//! アイテム新規追加
/*!
    アイテムのポインタを渡す。AXTreeViewItem を継承したクラスでも可。
*/

AXTreeViewItem *AXTreeView::addItem(AXTreeItem *pParent,AXTreeViewItem *pItem)
{
    m_pDat->add(pParent, pItem);
    _initItem(pItem);

    _updateDat();

    return pItem;
}

//! アイテム新規挿入

AXTreeViewItem *AXTreeView::insertItem(AXTreeItem *pInsert,AXTreeViewItem *pItem)
{
    m_pDat->insert(pInsert, pItem);
    _initItem(pItem);

    _updateDat();

    return pItem;
}

//! 全アイテム削除

void AXTreeView::deleteAllItem()
{
    m_pDat->deleteAllItem();
    _updateDat();
}

//! アイテム削除

void AXTreeView::delItem(AXTreeViewItem *pItem)
{
    m_pDat->delItem(pItem);
    _updateDat();
}

//! 展開状態変更
//! @param expand [0]閉じる [正]開く [負]反転

void AXTreeView::expandItem(AXTreeViewItem *pItem,int expand)
{
    if(pItem->m_pFirst)
    {
        if(expand == 0)
            pItem->m_uFlags &= ~AXTreeViewItem::FLAG_EXPAND;
        else if(expand > 0)
            pItem->m_uFlags |= AXTreeViewItem::FLAG_EXPAND;
        else
            pItem->m_uFlags ^= AXTreeViewItem::FLAG_EXPAND;

        _updateDat();

        //フォーカスアイテムが隠れた場合は選択変更

        if(!m_pDat->isFocusVisible())
            m_pDat->setFocusItem(NULL);
    }
}

//! フォーカスアイテムセット

void AXTreeView::setFocusItem(AXTreeViewItem *pItem)
{
    //隠れている場合は展開される

    if(m_pDat->setFocusItemVisible(pItem))
        _updateDat();
}

//! アイテム位置移動（pDst の前に）

void AXTreeView::moveItem(AXTreeViewItem *pItem,AXTreeViewItem *pDst)
{
    m_pDat->move(pItem, pDst);
    m_pDat->setFocusItem(NULL);

    _updateDat();
}

//! アイテム位置移動（pParent の最後に）

void AXTreeView::moveItemLast(AXTreeViewItem *pItem,AXTreeViewItem *pParent)
{
    m_pDat->moveLast(pItem, pParent);
    m_pDat->setFocusItem(NULL);

    _updateDat();
}

//! アイテムのテキスト変更

void AXTreeView::setItemText(AXTreeViewItem *pItem,LPCUSTR pText)
{
    pItem->m_strText = pText;

    _initItem(pItem);

    _updateDat();
}

//! アイテムのチェック変更
//! @param check [0]OFF [正]ON [負]反転

void AXTreeView::checkItem(AXTreeViewItem *pItem,int check)
{
    if(check == 0)
        pItem->uncheck();
    else if(check > 0)
        pItem->check();
    else
        pItem->m_uFlags ^= AXTreeViewItem::FLAG_CHECKED;

    m_pScrArea->redraw();
}

//! pItem が pParent より下位のアイテムかどうか

BOOL AXTreeView::isItemChild(AXTreeViewItem *pItem,AXTreeViewItem *pParent)
{
    AXTreeViewItem *p;

    for(p = pItem->parent(); p; p = p->parent())
    {
        if(p == pParent) return TRUE;
    }

    return FALSE;
}

//! パラメータ値から全アイテム検索

AXTreeViewItem *AXTreeView::findItemParam(ULONG lParam)
{
    AXTreeViewItem *p;

    for(p = getTopItem(); p; p = (AXTreeViewItem *)p->nextTreeItem())
    {
        if(p->m_lParam == lParam) return p;
    }

    return NULL;
}

//! 指定アイテム位置を基準に垂直スクロールセット
/*!
    @param align   [0]上端 [1]中央
*/

void AXTreeView::setScrollItem(AXTreeViewItem *pItem,int align)
{
    AXTreeViewItem *p;
    int y = SPACEVIEW;

    if(!m_pScrV) return;

    for(p = getTopItem(); p; p = p->nextVisible(), y += m_nItemH)
    {
        if(p == pItem) break;
    }

    if(align == 1)
        y -= (m_pScrArea->getHeight() - m_nItemH) / 2;

    m_pScrV->setPos(y);

    m_pScrArea->redraw();
}


//===============================
//ハンドラ
//===============================


//! フォーカスIN

BOOL AXTreeView::onFocusIn(int detail)
{
    m_pScrArea->redraw();
    return TRUE;
}

//! フォーカスOUT

BOOL AXTreeView::onFocusOut(int detail)
{
    m_pScrArea->redraw();
    return TRUE;
}

//! キー押し時

BOOL AXTreeView::onKeyDown(AXHD_KEY *phd)
{
    return m_pScrArea->onKeyDown(phd);
}
