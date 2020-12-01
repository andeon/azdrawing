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

#include <string.h>

#include "AXAppWinHash.h"
#include "AXWindow.h"


#define TABLECNT    256
#define HASH(a)     ((a) & (TABLECNT - 1))

//---------------------

/*!
    @class AXAppWinHash
    @brief ウィンドウ用ハッシュテーブル（AXApp で管理される）

    @ingroup core
*/


AXAppWinHash::AXAppWinHash()
{
    ::memset(m_pTable, 0, sizeof(AXWindow *) * TABLECNT);
}

//! テーブルに追加

void AXAppWinHash::add(AXWindow *pwin)
{
    AXWindow *pbk;
    int hash;

    //すでに存在している場合は取り外す

    remove(pwin);

    //追加

    hash = HASH(pwin->m_id);
    pbk  = m_pTable[hash];

    m_pTable[hash] = pwin;

    pwin->m_pHashPrev = NULL;
    pwin->m_pHashNext = pbk;

    if(pbk) pbk->m_pHashPrev = pwin;
}

//! テーブルから取り除く

void AXAppWinHash::remove(AXWindow *pwin)
{
    AXWindow *pPrev,*pNext;
    int hash;

    //登録されているか

    if(!search(pwin->m_id)) return;

    //---------- 取り除く

    pPrev = pwin->m_pHashPrev;
    pNext = pwin->m_pHashNext;

    //前の位置と次の位置とつなげる

    if(pPrev) pPrev->m_pHashNext = pNext;
    if(pNext) pNext->m_pHashPrev = pPrev;

    //テーブルの先頭の場合、次の位置をセット

    if(!pPrev)
    {
        hash = HASH(pwin->m_id);
        m_pTable[hash] = pNext;
    }
}

//! ウィンドウIDから検索
//! @return NULLで見つからなかった

AXWindow *AXAppWinHash::search(ULONG id)
{
    AXWindow *p;
    int hash;

    hash = HASH(id);

    for(p = m_pTable[hash]; p; p = p->m_pHashNext)
    {
        if(p->m_id == id) return p;
    }

    return NULL;
}
