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

#include "AXTree.h"


//******************************
// AXTreeItem
//******************************

/*!
    @class AXTreeItem
    @brief ツリーデータアイテムクラス

    @ingroup etc
*/


AXTreeItem::AXTreeItem()
{
    m_pParent = m_pFirst = m_pLast = m_pPrev = m_pNext = NULL;
}

AXTreeItem::~AXTreeItem()
{
    //前後をつなげる

    if(m_pPrev)
        m_pPrev->m_pNext = m_pNext;
    else if(m_pParent)
        m_pParent->m_pFirst = m_pNext;

    if(m_pNext)
        m_pNext->m_pPrev = m_pPrev;
    else if(m_pParent)
        m_pParent->m_pLast = m_pPrev;
}

//! 次のアイテム取得（ツリーアイテム全て対象）
/*!
    @return NULL で終了
*/

AXTreeItem *AXTreeItem::nextTreeItem()
{
    AXTreeItem *p = this;

    if(m_pFirst)
        p = m_pFirst;
    else
    {
        do
        {
            if(p->m_pNext)
            {
                p = p->m_pNext;
                break;
            }
            else
                p = p->m_pParent;
        } while(p);
    }

    return p;
}

//! 次のアイテム取得（指定した親の下位のみ）

AXTreeItem *AXTreeItem::nextTreeItem(AXTreeItem *pParent)
{
    if(m_pFirst)
        return m_pFirst;
    else
        return nextTreeItemPass(pParent);
}

//! 次のアイテム取得
/*!
    this が親の場合、子には行かず次のアイテムへ。
*/

AXTreeItem *AXTreeItem::nextTreeItemPass()
{
    AXTreeItem *p = this;

    do
    {
        if(p->m_pNext)
        {
            p = p->m_pNext;
            break;
        }
        else
            p = p->m_pParent;
    } while(p);

    return p;
}

//! 次のアイテム取得
/*!
    this が親の場合、子には行かない。また、pParent の以下の子のみ。
*/

AXTreeItem *AXTreeItem::nextTreeItemPass(AXTreeItem *pParent)
{
    AXTreeItem *p = this;

    if(this == pParent) return NULL;

    do
    {
        if(p->m_pNext)
        {
            p = p->m_pNext;
            break;
        }
        else
        {
            p = p->m_pParent;
            if(p == pParent) return NULL;
        }
    } while(p);

    return p;
}

//! 前のツリーアイテム取得
/*!
    前のアイテムに子アイテムがある場合は、その最後のアイテム。
*/

AXTreeItem *AXTreeItem::prevTreeItem()
{
    AXTreeItem *p;

    if(!m_pPrev)
        return m_pParent;
    else
    {
        //前のアイテムの最後のアイテム

        for(p = m_pPrev; p->m_pLast; p = p->m_pLast);

        return p;
    }
}

//! 前のツリーアイテム取得
/*!
    pParent の下位のアイテムのみ。
*/

AXTreeItem *AXTreeItem::prevTreeItem(AXTreeItem *pParent)
{
    AXTreeItem *p;

    if(this == pParent) return NULL;

    if(!m_pPrev)
    {
        if(m_pParent == pParent)
            return NULL;
        else
            return m_pParent;
    }
    else
    {
        for(p = m_pPrev; p->m_pLast; p = p->m_pLast);
        return p;
    }
}

//! 前のツリーアイテム取得
/*!
    前のアイテムに子があっても、その子へは行かない。
*/

AXTreeItem *AXTreeItem::prevTreeItemStop()
{
    AXTreeItem *p = this;

    do {
        if(p->m_pPrev)
        {
            p = p->m_pPrev;
            break;
        }
        else
            p = p->m_pParent;
    } while(p);

    return p;
}

//! this のツリー上における最後のアイテムを取得
/*!
    @return 子がなければ NULL
*/

AXTreeItem *AXTreeItem::lastTreeItem()
{
    AXTreeItem *p;

    for(p = m_pLast; p && p->m_pLast; p = p->m_pLast);

    return p;
}

//! this が pParent 以下の子アイテムかどうか

BOOL AXTreeItem::isChild(AXTreeItem *pParent)
{
    AXTreeItem *p;

    for(p = m_pParent; p; p = p->m_pParent)
    {
        if(p == pParent) return TRUE;
    }

    return FALSE;
}


//******************************
// AXTree
//******************************


/*!
    @class AXTree
    @brief ツリーデータクラス

    @ingroup etc
*/


AXTree::AXTree()
{
    m_pTop = m_pBottom = NULL;
}

AXTree::~AXTree()
{
    deleteAll();
}

//! 全て削除

void AXTree::deleteAll()
{
    AXTreeItem *p,*pnext;

    for(p = m_pTop; p; p = pnext)
    {
        pnext = p->m_pNext;

        _deleteChild(p);

        delete p;
    }

    m_pTop = m_pBottom = NULL;
}

//! 親の最後にアイテム追加
//! @param pParent NULL でルート

AXTreeItem *AXTree::add(AXTreeItem *pParent,AXTreeItem *pItem)
{
    if(pItem)
        _linkAdd(pParent, pItem);

    return pItem;
}

//! 指定アイテムの前に挿入

AXTreeItem *AXTree::insert(AXTreeItem *pIns,AXTreeItem *pItem)
{
    if(pItem && pIns)
        _linkInsert(pIns, pItem);

    return pItem;
}

//! アイテム削除（子も削除される）

void AXTree::deleteItem(AXTreeItem *pItem)
{
    if(pItem)
    {
        _linkRemove(pItem);

        _deleteChild(pItem);

        delete pItem;
    }
}

//! アイテムを取り除く（削除は行わない）

void AXTree::remove(AXTreeItem *pItem)
{
    if(pItem) _linkRemove(pItem);
}

//! 移動（pSrc を pDst の前に）

void AXTree::move(AXTreeItem *pSrc,AXTreeItem *pDst)
{
    if(pDst && pSrc != pDst)
    {
        _linkRemove(pSrc);
        _linkInsert(pDst, pSrc);
    }
}

//! 移動（pSrc を pParent の最後に）
/*!
    @param pParent NULL でルート
*/

void AXTree::moveLast(AXTreeItem *pSrc,AXTreeItem *pParent)
{
    _linkRemove(pSrc);
    _linkAdd(pParent, pSrc);
}

//! アイテム総数取得

int AXTree::getItemCnt()
{
    AXTreeItem *p;
    int cnt = 0;

    for(p = m_pTop; p; p = p->nextTreeItem(), cnt++);

    return cnt;
}

//! ツリー上で一番最後のアイテム取得

AXTreeItem *AXTree::getTreeLastItem()
{
    AXTreeItem *p;

    for(p = m_pBottom; p && p->m_pLast; p = p->m_pLast);

    return p;
}


//==========================
//サブ
//==========================


//! 親の最後にリンク

void AXTree::_linkAdd(AXTreeItem *pParent,AXTreeItem *p)
{
    //親

    p->m_pParent = pParent;

    //

    if(pParent)
    {
        //------ 親アイテムあり

        p->m_pPrev = pParent->m_pLast;  //prev = 親の最後の子
        pParent->m_pLast = p;           //親の最後の子 = p

        if(p->m_pPrev)
            p->m_pPrev->m_pNext = p;    //prevの次 = p
        else
            pParent->m_pFirst = p;      //親の最初の子
    }
    else
    {
        //------ 親がルート

        if(!m_pTop)
            //データがひとつもない時
            m_pTop = m_pBottom = p;
        else
        {
            //データが一つ以上存在する時

            m_pBottom->m_pNext = p;
            p->m_pPrev = m_pBottom;
            m_pBottom  = p;
        }
    }
}

//! 挿入リンク

void AXTree::_linkInsert(AXTreeItem *pIns,AXTreeItem *p)
{
    p->m_pParent = pIns->m_pParent;

    /*
        pIns の前に挿入するので、p が親の最後のアイテムになることはない。
    */

    if(pIns->m_pPrev)
        pIns->m_pPrev->m_pNext = p; //前のアイテムあり:前のnext = p
    else if(p->m_pParent)
        p->m_pParent->m_pFirst = p; //親の先頭:親あり
    else
        m_pTop = p;                 //親の先頭:ルート

    //

    p->m_pPrev    = pIns->m_pPrev;
    pIns->m_pPrev = p;
    p->m_pNext    = pIns;
}

//! リンクを外す（子のリンクは繋がっている）

void AXTree::_linkRemove(AXTreeItem *p)
{
    //前後をつなげる

    if(p->m_pPrev)
        p->m_pPrev->m_pNext = p->m_pNext;     //前のアイテムがある
    else if(p->m_pParent)
        p->m_pParent->m_pFirst = p->m_pNext;  //親の先頭:親がある
    else
        m_pTop = p->m_pNext;                  //親の先頭:親がルート

    if(p->m_pNext)
        p->m_pNext->m_pPrev = p->m_pPrev;
    else if(p->m_pParent)
        p->m_pParent->m_pLast = p->m_pPrev;
    else
        m_pBottom = p->m_pPrev;

    //

    p->m_pParent = p->m_pPrev = p->m_pNext = NULL;
}

//! 子アイテムをすべて削除（pRoot 自身は削除しない）

void AXTree::_deleteChild(AXTreeItem *pRoot)
{
    AXTreeItem *pParent,*p,*pnext;

    p       = pRoot->m_pFirst;
    pParent = pRoot;

    while(p)
    {
        if(p->m_pFirst)
        {
            //子アイテムありの場合 -> 子アイテムなしに来るまで繰り返す

            pParent = p;
            p       = p->m_pFirst;
        }
        else
        {
            //子アイテムがない場合 -> 削除

            pnext = p->m_pNext;

            delete p;

            //次のアイテム（NULLの場合は親へ戻る）

            p = pnext;

            while(!p)
            {
                if(pParent == pRoot) return;

                p       = pParent;
                pParent = p->m_pParent;
            }
        }
    }
}
