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

#include "AXList.h"


//******************************
// AXListItem
//******************************

/*!
    @class AXListItem
    @brief AXList の各アイテムの基底クラス

    @ingroup etc
*/

AXListItem::~AXListItem()
{

}

AXListItem::AXListItem()
{
    m_pPrev = m_pNext = NULL;
}


//******************************
// AXList
//******************************

/*!
    @class AXList
    @brief 双方向リストデータクラス

    - アイテムは、AXListItem の派生クラスを作る。
    - アイテムの作成は new で行い、それを add/insert でリストに追加する。

    @ingroup etc
*/


AXList::AXList()
{
    m_pTop = m_pBottom = NULL;
    m_nCnt = 0;
}

AXList::~AXList()
{
    deleteAll();
}

//! 全て削除

void AXList::deleteAll()
{
    AXListItem *p,*next;

    for(p = m_pTop; p; p = next)
    {
        next = p->m_pNext;

        delete p;
    }

    m_pTop = m_pBottom = NULL;
    m_nCnt = 0;
}

//! 追加

AXListItem *AXList::add(AXListItem *p)
{
    if(p)
    {
        linkAdd(p);
        m_nCnt++;
    }

    return p;
}

//! 挿入
/*!
    @param pIns 挿入先。pItem -> pIns の順になる。NULLで最後尾。
*/

AXListItem *AXList::insert(AXListItem *pItem,AXListItem *pIns)
{
    if(pItem)
    {
        linkInsert(pItem, pIns);
        m_nCnt++;
    }

    return pItem;
}

//! 位置を指定して挿入

AXListItem *AXList::insert(AXListItem *pItem,int pos)
{
    if(pItem)
    {
        linkInsert(pItem, getItem(pos));
        m_nCnt++;
    }

    return pItem;
}

//! リストから取り外す（削除はしない）

void AXList::remove(AXListItem *p)
{
    if(p)
    {
        linkRemove(p);
        m_nCnt--;
    }
}

//! アイテム削除

void AXList::deleteItem(AXListItem *p)
{
    if(p)
    {
        linkRemove(p);

        delete p;

        m_nCnt--;
    }
}

//! 位置からアイテム削除
/*!
    @return 削除されたか
*/

BOOL AXList::deleteItem(int pos)
{
    AXListItem *p = getItem(pos);

    if(!p)
        return FALSE;
    else
    {
        deleteItem(p);
        return TRUE;
    }
}

//! 位置移動
/*!
    @param pSrc 移動元アイテム
    @param pDst 移動先。pSrc -> pDst の順になる。NULLで最後尾へ。
*/

void AXList::move(AXListItem *pSrc,AXListItem *pDst)
{
    //同じ位置

    if(pSrc == pDst) return;

    //最後尾指定の場合、すでに最後尾にある

    if(pDst == NULL && m_pBottom == pSrc) return;

    //pSrcをリストからはずす

    linkRemove(pSrc);

    //pSrcをpDstの前にセット

    linkInsert(pSrc, pDst);
}

//! アイテムを先頭へ移動

void AXList::moveTop(AXListItem *pItem)
{
    move(pItem, m_pTop);
}

//! 位置を入れ替え

void AXList::swap(AXListItem *p1,AXListItem *p2)
{
    AXListItem *ptmp = p1->m_pNext;

    move(p1, p2->m_pNext);
    move(p2, ptmp);
}

//! 先頭からpos番目のアイテム取得
/*!
    @return NULLでアイテムが存在しない
*/

AXListItem *AXList::getItem(int pos) const
{
    AXListItem *p;
    int i;

    for(i = 0, p = m_pTop; p; i++, p = p->m_pNext)
    {
        if(i == pos) return p;
    }

    return NULL;
}

//! 終端からpos番目のアイテム取得
/*!
    @return NULLでアイテムが存在しない
*/

AXListItem *AXList::getItemBottom(int pos) const
{
    AXListItem *p;
    int i;

    for(i = 0, p = m_pBottom; p; i++, p = p->m_pPrev)
    {
        if(i == pos) return p;
    }

    return NULL;
}

//! アイテムから位置取得（先頭から）
/*!
    @return -1で指定アイテムが存在しない
*/

int AXList::getPos(AXListItem *pItem) const
{
    AXListItem *p;
    int i;

    for(i = 0, p = m_pTop; p; i++, p = p->m_pNext)
    {
        if(p == pItem) return i;
    }

    return -1;
}

//! アイテムから位置取得（後ろから）
/*!
    @return -1で指定アイテムが存在しない
*/

int AXList::getPosBottom(AXListItem *pItem) const
{
    AXListItem *p;
    int i;

    for(i = 0, p = m_pBottom; p; i++, p = p->m_pPrev)
    {
        if(p == pItem) return i;
    }

    return -1;
}

//! pItem1 が pItem2 より上にあるか下にあるか
/*!
    @return [-1] 上 [1] 下 [0] 同じ
*/

int AXList::getDir(AXListItem *pItem1,AXListItem *pItem2) const
{
    AXListItem *p;

    if(pItem1 == pItem2) return 0;

    for(p = pItem1->m_pPrev; p; p = p->m_pPrev)
    {
        if(p == pItem2) return 1;
    }

    return -1;
}

//! ソート(コムソート)
/*!
    @param func 比較関数。1番目のアイテムが2番目より大きければ 0 より大きい値を返す。
    @param lParam 比較関数に渡すパラメータ。
*/

void AXList::sort(int (*func)(AXListItem*,AXListItem*,ULONG),ULONG lParam)
{
    int h,i,flag = FALSE;
    AXListItem *p1,*p2,*ptmp;

    h = m_nCnt;

    while(h > 1 || flag)
    {
        if(h > 1)
        {
            h = (h * 10) / 13;
            if(h == 9 || h == 10) h = 11;
        }

        //先頭から +h の位置

        p1 = m_pTop;

        for(i = h, p2 = p1; i > 0 && p2; i--)
            p2 = p2->m_pNext;

        //

        for(flag = FALSE; p2; )
        {
            if((*func)(p1, p2, lParam) > 0)
            {
                //入れ替え

                swap(p1, p2);
                flag = TRUE;

                ptmp = p1;
                p1   = p2->m_pNext;
                p2   = ptmp->m_pNext;
            }
            else
            {
                p1 = p1->m_pNext;
                p2 = p2->m_pNext;
            }
        }
    }
}


//==============================
//リストのリンク関連
//==============================


//! pをリストの最後尾にセット

void AXList::linkAdd(AXListItem *p)
{
    if(!m_pTop)
    {
        //データがひとつもない時

        m_pTop = m_pBottom = p;
        p->m_pPrev = NULL;
        p->m_pNext = NULL;
    }
    else
    {
        //データが一つ以上存在する時

        m_pBottom->m_pNext = p;
        p->m_pPrev         = m_pBottom;
        p->m_pNext         = NULL;
        m_pBottom        = p;
    }
}

//! pSrc を pIns の前に挿入
/*!
    @param pIns NULLで最後尾
*/

void AXList::linkInsert(AXListItem *pSrc,AXListItem *pIns)
{
    if(pIns == NULL)
    {
        //最後尾へ

        linkAdd(pSrc);
    }
    else
    {
        //挿入

        if(pIns->m_pPrev == NULL) //挿入位置が先頭位置の場合
            m_pTop = pSrc;
        else
            (pIns->m_pPrev)->m_pNext = pSrc;

        pSrc->m_pPrev = pIns->m_pPrev;
        pIns->m_pPrev = pSrc;
        pSrc->m_pNext = pIns;
    }
}

//! 指定データをリストからはずす

void AXList::linkRemove(AXListItem *p)
{
    if(p->m_pPrev == NULL)
        //先頭データの場合、次のデータを先頭に持ってくる
        m_pTop = p->m_pNext;
    else
        //先頭ではない場合、前のデータと次のデータをつなげる
        (p->m_pPrev)->m_pNext = p->m_pNext;

    if(p->m_pNext == NULL)
        //最後尾データの場合、前のデータを最後尾に持ってくる
        m_pBottom = p->m_pPrev;
    else
        //最後尾でない場合、次のデータと前のデータをつなげる
        (p->m_pNext)->m_pPrev = p->m_pPrev;
}
