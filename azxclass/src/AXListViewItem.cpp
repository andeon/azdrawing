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

#include "AXListViewItem.h"
#include "AXListViewItemManager.h"



//*************************************
// AXListViewItemManager - リスト
//*************************************

/*!
    @class AXListViewItemManager
    @brief リストビューのアイテムリスト管理

    @ingroup widgetdat
*/


AXListViewItemManager::AXListViewItemManager()
{
    m_pFocus = NULL;
}

//! 全アイテム削除

void AXListViewItemManager::deleteAllItem()
{
    AXList::deleteAll();

    m_pFocus = NULL;
}

//! アイテム削除

void AXListViewItemManager::delItem(AXListViewItem *pItem)
{
    if(pItem == m_pFocus) m_pFocus = NULL;

    AXList::deleteItem(pItem);
}

//! 位置からアイテム取得
/*!
    @param pos 負の値で現在のフォーカス
*/

AXListViewItem *AXListViewItemManager::getItemPos(int pos)
{
    if(pos < 0)
        return m_pFocus;
    else
        return (AXListViewItem *)AXList::getItem(pos);
}

//! 次の選択アイテム取得
/*!
    @param pTop 検索開始位置。このアイテム自体は検索に含まない。NULLで先頭から
*/

AXListViewItem *AXListViewItemManager::getSelNext(AXListViewItem *pTop)
{
    AXListViewItem *p;

    if(!pTop)
        pTop = getTopItem();
    else
        pTop = pTop->next();

    for(p = pTop; p; p = p->next())
    {
        if(p->isSelect()) return p;
    }

    return NULL;
}

//! フォーカス位置のインデックス位置取得

int AXListViewItemManager::getFocusPos()
{
    if(m_pFocus)
        return getPos(m_pFocus);
    else
        return 0;
}


//=====================


//! 全てのアイテムを選択

void AXListViewItemManager::selectAll()
{
    AXListViewItem *p;

    for(p = getTopItem(); p; p = p->next())
        p->select();

    m_pFocus = getTopItem();
}

//! すべてのアイテムの選択を解除

void AXListViewItemManager::unselectAll()
{
    AXListViewItem *p;

    for(p = getTopItem(); p; p = p->next())
        p->unselect();
}

//! フォーカス位置を上下に移動
//! @return フォーカス位置が変わったか

BOOL AXListViewItemManager::updownFocus(BOOL bDown)
{
    AXListViewItem *p;

    if(!m_pFocus)
        p = getTopItem();
    else if(bDown)
        p = (m_pFocus->m_pNext)? m_pFocus->next(): m_pFocus;
    else
        p = (m_pFocus->m_pPrev)? m_pFocus->prev(): m_pFocus;

    //

    if(p == m_pFocus)
        return FALSE;
    else
    {
        //全選択を消してフォーカスのみ選択

        unselectAll();

        m_pFocus = p;
        m_pFocus->select();

        return TRUE;
    }
}

//! フォーカスを先頭または終端に移動(Home/End)

BOOL AXListViewItemManager::moveFocusTopEnd(BOOL bTop)
{
    AXListViewItem *p = (AXListViewItem *)((bTop)? m_pTop: m_pBottom);

    if(p != m_pFocus)
    {
        unselectAll();

        if(p) p->select();
        m_pFocus = p;

        return TRUE;
    }
    else
        return FALSE;
}

//! 指定位置のアイテムをフォーカスに

BOOL AXListViewItemManager::moveFocusFromPos(int pos)
{
    AXListViewItem *p;

    p = (AXListViewItem *)AXList::getItem(pos);

    if(p != m_pFocus)
    {
        unselectAll();

        if(p) p->select();
        m_pFocus = p;

        return TRUE;
    }
    else
        return FALSE;
}

//! アイテム選択処理
/*!
    @param type [0]単一選択 [1]複数,装飾なし [2]複数,Ctrl（反転） [3]複数,Shift（範囲）
    @return フォーカス位置が変わったか
*/

BOOL AXListViewItemManager::selectItem(int type,AXListViewItem *pItem)
{
    BOOL ret;

    //+Shift時、フォーカスがなければ指定アイテムのみ選択

    if(type == SELECT_MULTI_SHIFT && !m_pFocus)
        type = SELECT_MULTI;

    switch(type)
    {
        //単一選択
        case SELECT_SINGLE:
            if(m_pFocus) m_pFocus->unselect();
            pItem->select();
            break;
        //複数 : 全選択を解除してこのアイテムのみ選択
        case SELECT_MULTI:
            unselectAll();
            pItem->select();
            break;
        //複数+Ctrl ： 選択反転
        case 2:
            pItem->m_uFlags ^= AXListViewItem::FLAG_SELECTED;
            break;
        //複数+Shift : フォーカス位置から指定位置まで選択
        case 3:
            AXListViewItem *p;

            unselectAll();

            if(getDir(m_pFocus, pItem) == -1)
            {
                //focus -> pItem

                for(p = m_pFocus; p; p = p->next())
                {
                    p->select();
                    if(p == pItem) break;
                }
            }
            else
            {
                //pItem -> focus

                for(p = pItem; p; p = p->next())
                {
                    p->select();
                    if(p == m_pFocus) break;
                }
            }
            break;
    }

    ret = (m_pFocus != pItem);

    m_pFocus = pItem;

    return ret;
}


//*************************************
// AXListViewItem - リストアイテム
//*************************************


/*!
    @class AXListViewItem
    @brief リストビューの個々のアイテムクラス

    @ingroup widgetdat
*/


//! 作成

AXListViewItem::AXListViewItem(LPCUSTR pText,int nIconNo,UINT uFlags,ULONG lParam)
{
    m_strText   = pText;
    m_uFlags    = uFlags;
    m_nIconNo   = nIconNo;
    m_lParam    = lParam;
}
