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

#include "AXUndo.h"
#include "AXUndoDat.h"


//******************************
// AXUndoDat
//******************************

/*!
    @class AXUndoDat
    @brief アンドゥ個々のデータクラス

    @ingroup etc
*/

/*!
    @fn BOOL AXUndoDat::setReverseDat(AXUndoDat *pSrc,BOOL bUndo)
    @brief pSrc と逆のデータをセット

    @param bUndo TRUEでアンドゥ処理時（リドゥ用のデータセット）、FALSEでリドゥ処理時
    @return データセットが成功したか
*/
/*!
    @fn BOOL AXUndoDat::run(BOOL bUndo)
    @brief アンドゥ/リドゥ処理実行

    @param bUndo TRUEでアンドゥ処理。FALSEでリドゥ処理
*/


//******************************
// AXUndo
//******************************


/*!
    @class AXUndo
    @brief アンドゥ管理クラス

    - createDat() で AXUndoDat の派生クラスを new で作成させること。
    - エラー時、データは全て削除される。

    @ingroup etc
*/

/*
    <リスト順> 左から順に top --> bottom

    [UNDO古] -> [UNDO新](CURRENT) -> [REDO新] -> [REDO古]

    <カレント位置>

    現在のアンドゥデータ位置。次以降のデータはリドゥデータ。
    NULL の場合はアンドゥデータなしで全てリドゥデータの状態。
*/


AXUndo::~AXUndo()
{

}

AXUndo::AXUndo()
{
    m_pCurrent    = NULL;
    m_nMaxUndoCnt = 30;
}

//! すべて削除

void AXUndo::deleteAllDat()
{
    AXList::deleteAll();

    m_pCurrent = NULL;
}

//! アンドゥデータのみ削除（リドゥデータは残す）

void AXUndo::deleteUndo()
{
    AXListItem *p,*pNext;

    /* カレントの位置までがアンドゥデータ。
       カレントがNULLなら全てリドゥデータ */

    if(!m_pCurrent) return;

    for(p = m_pTop; p; p = pNext)
    {
        pNext = p->m_pNext;

        AXList::deleteItem(p);

        if(p == m_pCurrent) break;
    }

    m_pCurrent = NULL;
}

//! リドゥデータのみ全て削除

void AXUndo::deleteRedo()
{
    AXListItem *p,*pNext;

    /* カレントの次以降がリドゥデータ。
       カレントがNULLなら全てがリドゥデータ */

    p = (m_pCurrent)? m_pCurrent->m_pNext: m_pTop;

    for(; p; p = pNext)
    {
        pNext = p->m_pNext;

        AXList::deleteItem(p);
    }
}

//! アンドゥデータ追加
/*!
    @param pDat new で作成する
*/

void AXUndo::addDat(AXUndoDat *pDat)
{
    //現在のリドゥデータを全て削除

    deleteRedo();

    //最大回数を越えたら古いデータを削除
    /* リドゥは全て削除されているので、top が一番古いアンドゥデータ */

    while(m_nCnt >= m_nMaxUndoCnt && m_pTop)
        AXList::deleteItem(m_pTop);

    //リストに追加

    AXList::add(pDat);

    //カレント位置

    m_pCurrent = pDat;

    //

    afterAddUndo();
}

//! アンドゥ最大回数セット（最小値=2）

void AXUndo::setMaxUndoCnt(int cnt)
{
    m_nMaxUndoCnt = (cnt < 2)? 2: cnt;
}

//! アンドゥ処理実行

AXUndo::RETURN AXUndo::undo()
{
    AXUndoDat *p;

    //アンドゥデータがない

    if(m_pCurrent == NULL) return RET_NODATA;

    //リドゥ用データをセット
    /* [UNDO_B][UNDO_A(current)] -> [UNDO_B(current)][_del_][REDO_NEW] */

    p = createDat();

    AXList::insert(p, m_pCurrent->m_pNext);

    if(!p->setReverseDat(this, m_pCurrent, TRUE))
        deleteRedo();

    //アンドゥ処理を実行

    if(!m_pCurrent->run(this, TRUE))
    {
        deleteAllDat();
        return RET_ERROR;
    }

    //カレントデータ削除

    AXList::deleteItem(m_pCurrent);

    //カレント位置

    m_pCurrent = p->prev();

    return RET_OK;
}

//! リドゥ処理実行

AXUndo::RETURN AXUndo::redo()
{
    AXUndoDat *pRedo,*pUndo;

    if(m_nCnt == 0) return RET_NODATA;

    /*
        [UNDO_A(CURRENT)][REDO_CUR] -> [UNDO_A][UNDO_NEW(CURRENT)][_del_]
    */

    //リドゥデータ

    pRedo = (m_pCurrent)? m_pCurrent->next(): (AXUndoDat *)m_pTop;

    if(pRedo == NULL) return RET_NODATA;

    //アンドゥデータセット

    pUndo = createDat();

    AXList::insert(pUndo, pRedo);

    if(!pUndo->setReverseDat(this, pRedo, FALSE))
    {
        deleteAllDat();
        return RET_ERROR;
    }

    //リドゥ処理

    if(!pRedo->run(this, FALSE))
    {
        deleteAllDat();
        return RET_ERROR;
    }

    //リドゥデータ削除

    AXList::deleteItem(pRedo);

    //カレント位置

    m_pCurrent = pUndo;

    return RET_OK;
}

//! アンドゥデータ追加後

void AXUndo::afterAddUndo()
{

}
