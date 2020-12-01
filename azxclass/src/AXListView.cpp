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

#include "AXListView.h"

#include "AXListViewItemManager.h"
#include "AXListViewArea.h"
#include "AXHeader.h"
#include "AXScrollBar.h"
#include "AXFont.h"
#include "AXImageList.h"
#include "AXApp.h"
#include "AXUtilStr.h"


#define AREA            ((AXListViewArea *)m_pScrArea)
#define ITEMSPACEX      3
#define CHECKBOXSIZE    13


/*!
    @class AXListView
    @brief リストビューウィジェット

    - AXScrollView のスタイルも有効。@n
      スクロールバーの有無はスタイルで指定する必要がある（AXScrollView::SVS_HORZ or VERT）
    - 一番最初のカラムは必ず左寄せになる。
    - アイテム位置を指定する際は、負の値で現在のフォーカスアイテムとなる。

    <h2>[Notify通知]</h2>
    lParam = AXListViewItem*

    <h2>[オーナードロー]</h2>
    pItem : AXListViewItem* @n
    lParam1 : アイテムの lParam 値 @n
    lParam2 : カラムのインデックス番号（0〜）

    @ingroup widget
*/

/*!
    @var AXListView::LVS_COLLINE
    @brief 列の境界線を描画する
    @var AXListView::LVS_ROWLINE
    @brief 1行ごとに下線を描画する
    @var AXListView::LVS_CHECKBOX
    @brief チェックボックスを付ける
    @var AXListView::LVS_MULTISEL
    @brief 複数選択を有効にする
    @var AXListView::LVS_NOHEADER
    @brief ヘッダを付けない
    @var AXListView::LVS_GRIDLINE
    @brief 列と行の線を描画する

    @var AListView::LVN_CHANGEFOCUS
    @brief フォーカスアイテムが変わった
    @var AListView::LVN_CHECKITEM
    @brief チェックボックスありの場合に、チェックが変更された
    @var AListView::LVN_RIGHTCLICK
    @brief 右ボタンが押された（lParam = 押された位置のアイテム）
    @var AListView::LVN_DBLCLK
    @brief 左ボタンがダブルクリックされた
*/


AXListView::~AXListView()
{
    delete m_pDat;
}

AXListView::AXListView(AXWindow *pParent,UINT uStyle,UINT uLayoutFlags)
    : AXScrollView(pParent, uStyle, uLayoutFlags)
{
    _createListView();
}

AXListView::AXListView(AXWindow *pParent,UINT uStyle,UINT uLayoutFlags,UINT uItemID,DWORD dwPadding)
    : AXScrollView(pParent, uStyle, uLayoutFlags, uItemID, dwPadding)
{
    _createListView();
}

//! 作成

void AXListView::_createListView()
{
    m_uType  = TYPE_LISTVIEW;
    m_uFlags |= FLAG_TAKE_FOCUS;

    m_pDat = new AXListViewItemManager;
    m_pImgList  = NULL;
    m_nCustumH  = -1;

    m_pScrArea = new AXListViewArea(this, 0, m_pDat);

    m_pHeader = ((AXListViewArea *)m_pScrArea)->getHeader();

    _setItemH();
}

//! 標準サイズ計算

void AXListView::calcDefSize()
{
    _setItemH();

    AXScrollView::calcDefSize();
}

//! 再構成

void AXListView::reconfig()
{
    _setScrollInfo();

    AXScrollView::reconfig();

    m_pScrArea->redraw();
}

//! スクロール情報セット

void AXListView::_setScrollInfo()
{
    if(m_pScrH)
    {
        m_pScrH->setStatus(0, m_pHeader->getAllWidth(), m_pScrArea->getWidth());
        m_pHeader->setScroll(m_pScrH->getPos());
    }

    if(m_pScrV)
    {
        m_pScrV->setStatus(0, m_pDat->getCnt() * m_nItemH,
                m_pScrArea->getHeight() - ((m_uStyle & LVS_NOHEADER)? 0: m_pHeader->getDefH()) );
    }
}

//! アイテム高さセット

void AXListView::_setItemH()
{
    if(m_nCustumH > 0)
        m_nItemH = m_nCustumH;
    else
    {
        m_nItemH = m_pFont->getHeight();

        if((m_uStyle & LVS_CHECKBOX) && m_nItemH < CHECKBOXSIZE) m_nItemH = CHECKBOXSIZE;
        if(m_pImgList && m_nItemH < m_pImgList->getHeight()) m_nItemH = m_pImgList->getHeight();

        m_nItemH += 2;
    }
}


//==============================
//
//==============================


//! アイコンのイメージリストセット

void AXListView::setImageList(AXImageList *pimg)
{
    m_pImgList = pimg;
}

//! アイテム高さを任意でセット

void AXListView::setItemHeight(int h)
{
    m_nCustumH = m_nItemH = h;
}

//! 最後にクリックされたカラム位置取得
//! @return -1 でなし、または範囲外

int AXListView::getLastClkColumn()
{
    return AREA->m_nLastClkColumn;
}

//! 列を追加
//! @param uFlags AXListView::COLUMNFLAGS

void AXListView::addColumn(LPCUSTR pText,int width,UINT uFlags)
{
    m_pHeader->addItem(pText, width, uFlags);
    setReconfig();
}

//! カラム幅取得

int AXListView::getColWidth(int no)
{
    return m_pHeader->getItemWidth(no);
}

//! カラム幅セット

void AXListView::setColumnWidth(int colno,int width)
{
    m_pHeader->setItemWidth(colno, width);
    setReconfig();
}

//--------------

//! アイテム追加

AXListViewItem *AXListView::addItem(LPCUSTR pText,int nIconNo,UINT uFlags,ULONG lParam)
{
    AXListViewItem *p;

    p = (AXListViewItem *)m_pDat->add(new AXListViewItem(pText, nIconNo, uFlags, lParam));
    setReconfig();

    return p;
}

AXListViewItem *AXListView::addItem(LPCUSTR pText)
{
    return addItem(pText, -1, 0, 0);
}

AXListViewItem *AXListView::addItem(AXListViewItem *pItem)
{
    m_pDat->add(pItem);
    setReconfig();

    return pItem;
}

//! アイテム挿入

AXListViewItem *AXListView::insertItem(AXListViewItem *pInsert,LPCUSTR pText,int nIconNo,UINT uFlags,ULONG lParam)
{
    AXListViewItem *p;

    p = (AXListViewItem *)m_pDat->insert(new AXListViewItem(pText, nIconNo, uFlags, lParam), pInsert);
    setReconfig();

    return p;
}

AXListViewItem *AXListView::insertItem(AXListViewItem *pInsert,AXListViewItem *pItem)
{
    m_pDat->insert(pItem, pInsert);
    setReconfig();

    return pItem;
}

//! 連続した翻訳文字列IDから複数アイテム追加

void AXListView::addItemMulTr(int startID,int cnt)
{
    for(; cnt > 0; cnt--, startID++)
        addItem(_str(startID));
}

//! 全アイテム削除

void AXListView::deleteAllItem()
{
    m_pDat->deleteAllItem();
    setReconfig();
}

//! アイテム削除

void AXListView::delItem(AXListViewItem *pItem)
{
    m_pDat->delItem(pItem);
    setReconfig();
}

//! アイテム削除

void AXListView::delItem(int pos)
{
    AXListViewItem *p = getItem(pos);

    if(p) delItem(p);
}

//! フォーカスアイテムを削除し、その下または上をフォーカスアイテムとする

void AXListView::delItemFocus()
{
    AXListViewItem *pFocus,*pNext;

    pFocus = m_pDat->getFocusItem();

    if(pFocus)
    {
        pNext = pFocus->next();

        delItem(pFocus);

        if(!pNext) pNext = (AXListViewItem *)m_pDat->getBottom();

        if(pNext) setFocusItem(pNext);
    }
}

//! 指定列のテキストセット

void AXListView::setItemText(AXListViewItem *pItem,int colno,LPCUSTR pText)
{
    if(colno >= 0 && colno < m_pHeader->getItemCnt())
    {
        pItem->m_strText.setSplitText(colno, pText, '\t');
        m_pScrArea->redraw();
    }
}

void AXListView::setItemText(int pos,int colno,LPCUSTR pText)
{
    AXListViewItem *p = getItem(pos);

    if(p) setItemText(p, colno, pText);
}

//! 指定列のテキスト取得

void AXListView::getItemText(AXListViewItem *pItem,int colno,AXString *pstr)
{
    pItem->m_strText.getSplitPosStr(pstr, colno, '\t');
}

//! パラメータ取得

ULONG AXListView::getItemParam(int pos)
{
    AXListViewItem *p = getItem(pos);

    if(p)
        return p->m_lParam;
    else
        return 0;
}

//! パラメータセット

void AXListView::setItemParam(int pos,ULONG lParam)
{
    AXListViewItem *p = getItem(pos);

    if(p) p->m_lParam = lParam;
}

void AXListView::setItemParam(AXListViewItem *p,ULONG lParam)
{
    p->m_lParam = lParam;
}

//! 指定位置からアイテム取得

AXListViewItem *AXListView::getItem(int pos)
{
    return m_pDat->getItemPos(pos);
}

//! アイテム数取得

int AXListView::getItemCnt()
{
    return m_pDat->getCnt();
}

//! 先頭アイテム取得

AXListViewItem *AXListView::getTopItem()
{
    return m_pDat->getTopItem();
}

//! フォーカスアイテム取得

AXListViewItem *AXListView::getFocusItem()
{
    return m_pDat->getFocusItem();
}

//! 次の選択アイテム取得
/*!
    @param pTop 検索開始位置。このアイテム自体は検索に含まない。NULLで先頭から
*/

AXListViewItem *AXListView::getSelNext(AXListViewItem *pTop)
{
    return m_pDat->getSelNext(pTop);
}

//! 終端へスクロール

void AXListView::scrollEnd()
{
    if(m_pScrV)
    {
        //※アイテム追加直後は reconfig() が実行されていないため
        _setScrollInfo();

        if(m_pScrV->setPosEnd())
            m_pScrArea->redraw();
    }
}

//! フォーカスアイテム変更

void AXListView::setFocusItem(AXListViewItem *pItem)
{
    m_pDat->selectItem(
        (m_uStyle & LVS_MULTISEL)? AXListViewItemManager::SELECT_MULTI: AXListViewItemManager::SELECT_SINGLE,
        pItem);

    m_pScrArea->redraw();
}

void AXListView::setFocusItem(int pos)
{
    AXListViewItem *p = getItem(pos);

    if(p) setFocusItem(p);
}

//! ソート
/*!
    @param func 比較関数。1番目のアイテムが2番目より大きければ 0 より大きい値を返す。
    @param lParam 比較関数に渡すパラメータ。
*/

void AXListView::sortItem(int (*func)(AXListItem*,AXListItem*,ULONG),ULONG lParam)
{
    m_pDat->sort(func, lParam);

    m_pScrArea->redraw();
}

//! カラムの幅をテキスト幅に合わせる

void AXListView::setAutoWidth(int colno)
{
    AXListViewItem *p;
    AXString str;
    int w,maxw = 3;

    for(p = m_pDat->getTopItem(); p; p = p->next())
    {
        p->m_strText.getSplitPosStr(&str, colno, '\t');

        w = m_pFont->getTextWidth(str) + ITEMSPACEX * 2;

        if(m_uStyle & LVS_CHECKBOX) w += CHECKBOXSIZE + 4;

        if(w > maxw) maxw = w;
    }

    m_pHeader->setItemWidth(colno, maxw);
    setReconfig();
}

//! 表示エリアを更新

void AXListView::updatePaint()
{
    m_pScrArea->redraw();
}


//==============================
//ハンドラ
//==============================


//! フォーカスIN

BOOL AXListView::onFocusIn(int detail)
{
    m_pScrArea->redraw();
    return TRUE;
}

//! フォーカスOUT

BOOL AXListView::onFocusOut(int detail)
{
    m_pScrArea->redraw();
    return TRUE;
}

//! キー押し時

BOOL AXListView::onKeyDown(AXHD_KEY *phd)
{
    return m_pScrArea->onKeyDown(phd);
}
