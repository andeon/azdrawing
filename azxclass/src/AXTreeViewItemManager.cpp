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

#include "AXTreeViewItemManager.h"

#include "AXTreeViewItem.h"



//***********************************
// AXTreeViewItem
//***********************************


/*!
    @class AXTreeViewItem
    @brief ツリービューアイテム

    @ingroup widgetdat
*/

/*
    m_nX    : 展開部分のX位置。表示部分の余白は含まない
    m_nW    : 展開部分からテキストまでの幅
*/


AXTreeViewItem::AXTreeViewItem(LPCUSTR pText,int nIconNo,UINT uFlags,ULONG lParam)
{
    m_strText   = pText;
    m_nIconNo   = nIconNo;
    m_uFlags    = uFlags;
    m_lParam    = lParam;
}

//! 次の表示アイテム取得

AXTreeViewItem *AXTreeViewItem::nextVisible()
{
    AXTreeItem *p = this;

    if(m_pFirst && (m_uFlags & FLAG_EXPAND))
        //子があり展開されている場合は子へ
        p = m_pFirst;
    else
    {
        //子がない場合は次へ
        /*
            次があれば次へ。
            親の最後のアイテムだった場合は、親へ戻りその次のアイテムへ（繰り返す）
        */

        while(1)
        {
            if(p->m_pNext)
            {
                p = p->m_pNext;
                break;
            }
            else
            {
                p = p->m_pParent;
                if(!p) break;
            }
        }
    }

    return (AXTreeViewItem *)p;
}

//! 前の表示アイテム取得

AXTreeViewItem *AXTreeViewItem::prevVisible()
{
    if(!m_pPrev)
        //前がなければ、親へ戻る
        return parent();
    else
    {
        //前がある -> 展開されている場合は、展開の最後のアイテム

        AXTreeViewItem *p = prev();

        while(p->m_pFirst && p->isExpand())
            p = p->last();

        return p;
    }
}


//***********************************
// AXTreeViewItemManager
//***********************************


/*!
    @class AXTreeViewItemManager
    @brief ツリービューアイテム管理

    @ingroup widgetdat
*/


AXTreeViewItemManager::AXTreeViewItemManager()
{
    m_pFocus = NULL;
}

//! フォーカスアイテムセット

BOOL AXTreeViewItemManager::setFocusItem(AXTreeViewItem *p)
{
    if(p == m_pFocus)
        return FALSE;
    else
    {
        m_pFocus = p;
        return TRUE;
    }
}

//! フォーカスアイテムセット＆隠れている場合は展開

BOOL AXTreeViewItemManager::setFocusItemVisible(AXTreeViewItem *p)
{
    AXTreeViewItem *p1;

    //隠れている場合は親をすべて展開

    if(p && !isVisibleItem(p))
    {
        for(p1 = p->parent(); p1; p1 = p1->parent())
            p1->expand();
    }

    //

    return setFocusItem(p);
}

//! 全アイテム削除

void AXTreeViewItemManager::deleteAllItem()
{
    AXTree::deleteAll();
    m_pFocus = NULL;
}

//! アイテム削除

void AXTreeViewItemManager::delItem(AXTreeViewItem *p)
{
    //フォーカスが削除アイテムまたは削除アイテムの子の場合

    if(p == m_pFocus || isItemChild(p, m_pFocus))
        m_pFocus = NULL;

    //

    AXTree::deleteItem(p);
}

//! 再構成時の各アイテム計算と最大幅・高さ取得

void AXTreeViewItemManager::calcReconfig(int nSpaceLevel,int nItemH,LPINT pMaxWidth,LPINT pMaxHeight)
{
    AXTreeViewItem *p,*p1;
    int maxw = 0,maxh = 0;

    p = getTopItem();

    while(p)
    {
        //X

        for(p1 = p->parent(), p->m_nX = 0; p1; p1 = p1->parent())
            p->m_nX += nSpaceLevel;

        //最大幅・高さ

        if(p->m_nX + p->m_nW > maxw) maxw = p->m_nX + p->m_nW;

        maxh += nItemH;

        //

        p = p->nextVisible();
    }

    *pMaxWidth  = maxw;
    *pMaxHeight = maxh;
}

//! x,y 位置からアイテム取得

AXTreeViewItem *AXTreeViewItemManager::getCurItem(int x,int y,int itemh,LPINT pTopY)
{
    AXTreeViewItem *p;
    int yy = 0;

    if(y < 0) return NULL;

    p = getTopItem();

    while(p)
    {
        if(yy <= y && y < yy + itemh)
        {
            if(pTopY) *pTopY = yy;
            return p;
        }

        //

        yy += itemh;

        p = p->nextVisible();
    }

    return NULL;
}

//! 指定アイテムが展開されているか（表示されているか）

BOOL AXTreeViewItemManager::isVisibleItem(AXTreeViewItem *pItem)
{
    AXTreeViewItem *p;

    if(!pItem) return TRUE;

    for(p = pItem->parent(); p; p = p->parent())
    {
        if(!p->isExpand()) return FALSE;
    }

    return TRUE;
}

//! フォーカスが展開されているか（表示されているか）

BOOL AXTreeViewItemManager::isFocusVisible()
{
    return isVisibleItem(m_pFocus);
}

//! フォーカスアイテムが展開可能か

BOOL AXTreeViewItemManager::isFocusEnableExpand()
{
    if(!m_pFocus)
        return FALSE;
    else
        return (m_pFocus->m_pFirst != NULL);
}

//! pItem が pParent の子かどうか

BOOL AXTreeViewItemManager::isItemChild(AXTreeViewItem *pParent,AXTreeViewItem *pItem)
{
    AXTreeItem *p;

    if(!pItem) return FALSE;

    for(p = pItem->m_pParent; p; p = p->m_pParent)
    {
        if(p == pParent) return TRUE;
    }

    return FALSE;
}

//! フォーカスを先頭/終端に移動

BOOL AXTreeViewItemManager::moveFocusHomeEnd(BOOL bHome)
{
    AXTreeViewItem *p;

    if(bHome)
        p = (AXTreeViewItem *)m_pTop;
    else
    {
        p = (AXTreeViewItem *)m_pBottom;
        if(!p) return FALSE;

        //展開の最後へ

        while(p->m_pFirst && p->isExpand())
            p = p->last();
    }

    return setFocusItem(p);
}

//! フォーカスを上下に移動

BOOL AXTreeViewItemManager::moveFocusUpDown(BOOL bUp)
{
    AXTreeViewItem *p = m_pFocus;

    if(!p)
        p = getTopItem();
    else if(bUp)
        p = p->prevVisible();
    else
        p = p->nextVisible();

    if(!p) return FALSE;

    return setFocusItem(p);
}

//! フォーカスのY位置取得

int AXTreeViewItemManager::getFocusYPos(int itemh)
{
    AXTreeViewItem *p;
    int y = 0;

    for(p = getTopItem(); p; p = p->nextVisible(), y += itemh)
    {
        if(p == m_pFocus) break;
    }

    return y;
}
