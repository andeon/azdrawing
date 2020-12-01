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

#include "AXLayout.h"
#include "AXRect.h"

/*!
    @class AXLayout
    @brief レイアウトの基本クラス

    @ingroup layout
*/
/*!
    @var AXLayout::m_nSepW
    @brief アイテムとアイテムの間の余白(px)、AXLayoutMatrix 時は横の数
    @var AXLayout::m_nSpaceW
    @brief レイアウトの内側の余白(px)
*/

//---------------------


AXLayout::~AXLayout()
{
    AXLayoutItem *p,*pnext;

    //全ての子アイテム削除

    for(p = m_pTop; p; p = pnext)
    {
        pnext = p->m_pLNext;

        delete p;
    }
}

//! 初期化

AXLayout::AXLayout()
{
    m_pTop = m_pBottom = NULL;

    m_nSepW = m_nSpaceW = 0;

    m_uFlags = FLAG_VISIBLE;
    m_uType  = TYPE_LAYOUT;
}

AXLayout::AXLayout(UINT uLayoutFlags,int sepw)
{
    m_pTop = m_pBottom = NULL;

    m_nSepW   = sepw;
    m_nSpaceW = 0;

    m_uLFlags = uLayoutFlags;
    m_uFlags  = FLAG_VISIBLE;
    m_uType   = TYPE_LAYOUT;
}

//! アイテム追加

void AXLayout::addItem(AXLayoutItem *p)
{
    if(p->m_pLParent) return;   //すでにどこかにセットされている

    p->m_pLParent = this;

    if(!m_pTop)
    {
        m_pTop = m_pBottom = p;
        p->m_pLNext = p->m_pLPrev = NULL;
    }
    else
    {
        m_pBottom->m_pLNext = p;
        p->m_pLPrev = m_pBottom;
        p->m_pLNext = NULL;
        m_pBottom = p;
    }
}

//! アイテム挿入
/*!
    pIns の前に p が挿入される。

    @param pIns NULL で最後尾へ
*/

void AXLayout::insertItem(AXLayoutItem *p,AXLayoutItem *pIns)
{
    if(p->m_pLParent || pIns->m_pLParent != this) return;

    if(!pIns)
        addItem(p);
    else
    {
        if(pIns->m_pLPrev == NULL)
            m_pTop = p;
        else
            (pIns->m_pLPrev)->m_pLNext = p;

        p->m_pLPrev    = pIns->m_pLPrev;
        pIns->m_pLPrev = p;
        p->m_pLNext    = pIns;
    }
}

//! アイテム取り外す
/*!
    ※アイテムの削除（delete）は行わない。
*/

void AXLayout::removeItem(AXLayoutItem *p)
{
    if(p->m_pLParent == this)
    {
        if(p->m_pLPrev == NULL)
            m_pTop = p->m_pLNext;
        else
            (p->m_pLPrev)->m_pLNext = p->m_pLNext;

        if(p->m_pLNext == NULL)
            m_pBottom = p->m_pLPrev;
        else
            (p->m_pLNext)->m_pLPrev = p->m_pLPrev;

        //

        p->m_pLParent = p->m_pLPrev = p->m_pLNext = NULL;
    }
}

//! 全アイテム取り外す

void AXLayout::removeItemAll()
{
    AXLayoutItem *p,*pnext;

    for(p = m_pTop; p; p = pnext)
    {
        pnext = p->m_pLNext;
        removeItem(p);
    }
}

//! アイテムを置き換える
/*!
    pSrc をレイアウトから取り外し、同じ位置に pReplace をレイアウトにセットする。

    @param pSrc レイアウト済みの、置き換え元アイテム
    @param pReplace 未レイアウトの、置き換えるアイテム（NULL で pSrc を取り外すだけ）
*/

void AXLayout::replaceItem(AXLayoutItem *pSrc,AXLayoutItem *pReplace)
{
    if(pReplace)
        insertItem(pReplace, pSrc);

    removeItem(pSrc);
}


//============================


//! 表示されているか

BOOL AXLayout::isVisible()
{
    return ((m_uFlags & FLAG_VISIBLE) != 0);
}

//! 範囲取得

void AXLayout::getClientRect(AXRect *prc)
{
    /*
        ウィンドウにセットされているレイアウト（トップレイアウト）の場合は
        ウィンドウの範囲を取得。
        親がレイアウトの場合は、自身の範囲を取得。
    */

    if(m_pLParent->getType() == TYPE_LAYOUT)
        AXLayoutItem::getClientRect(prc);
    else
        m_pLParent->getClientRect(prc);
}

//! 位置＆サイズ変更

BOOL AXLayout::moveresize(int x,int y,int w,int h)
{
    if(w < 0) w = 0;
    if(h < 0) h = 0;

    //サイズなどの変更は判定せず常に実行すること

    m_nX = x, m_nY = y;
    m_nW = w, m_nH = h;

    layout();

    return TRUE;
}

//! 表示

void AXLayout::show()
{
    AXLayoutItem *p;

    m_uFlags |= FLAG_VISIBLE;

    for(p = m_pTop; p; p = p->m_pLNext)
        p->show();
}

//! 非表示

void AXLayout::hide()
{
    AXLayoutItem *p;

    m_uFlags &= ~FLAG_VISIBLE;

    for(p = m_pTop; p; p = p->m_pLNext)
        p->hide();
}
