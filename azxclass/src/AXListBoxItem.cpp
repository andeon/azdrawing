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

#include "AXListBoxItem.h"
#include "AXListBoxItemManager.h"


//**********************************
// AXListBoxItem - リストデータ
//**********************************

/*!
    @class AXListBoxItem
    @brief リストボックスアイテム個々のデータ

    @brief widgetdat
*/


AXListBoxItem::AXListBoxItem(LPCUSTR pstr,ULONG lParam)
{
    m_strText   = pstr;
    m_lParam    = lParam;
}


//**********************************
// AXListBoxItemManager
//**********************************


/*!
    @class AXListBoxItemManager
    @brief リストボックスアイテム管理クラス

    @ingroup widgetdat
*/

/*
    m_nSelNo    : 現在の選択番号。-1で選択なし
    m_pSelItem  : 現在の選択アイテム。NULLで選択なし
*/


AXListBoxItemManager::AXListBoxItemManager()
{
    m_nSelNo    = -1;
    m_pSelItem  = NULL;
}

//! データ変更による選択変更時

void AXListBoxItemManager::_changeSel()
{
    if(m_pSelItem)
    {
        m_nSelNo = AXList::getPos(m_pSelItem);
        if(m_nSelNo == -1) m_pSelItem = NULL;
    }
}

//! 選択をなしにする

void AXListBoxItemManager::_setSelNone()
{
    m_nSelNo    = -1;
    m_pSelItem  = NULL;
}


//========================


//! データ追加
/*!
    @return 追加されたデータの位置
*/

int AXListBoxItemManager::addItem(LPCUSTR pstr,ULONG lParam)
{
    AXList::add(new AXListBoxItem(pstr, lParam));

    return m_nCnt - 1;
}

//! データ追加（直接）

int AXListBoxItemManager::addItem(AXListBoxItem *pItem)
{
    AXList::add(pItem);

    return m_nCnt - 1;
}

//! データ挿入

int AXListBoxItemManager::insertItem(int pos,LPCUSTR pstr,ULONG lParam)
{
    AXListItem *p;

    p = AXList::insert(new AXListBoxItem(pstr, lParam), pos);

    //現在の選択位置より前に挿入された場合、選択位置が変わるので再計算
    _changeSel();

    return AXList::getPos(p);
}

//! すべて削除

void AXListBoxItemManager::deleteAllItem()
{
    AXList::deleteAll();
    _setSelNone();
}

//! データ削除

BOOL AXListBoxItemManager::deleteItem(AXListBoxItem *pItem)
{
    AXList::deleteItem(pItem);
    _changeSel();

    return TRUE;
}

//! 指定位置のデータ削除

BOOL AXListBoxItemManager::deleteItemPos(int pos)
{
    if(pos < 0) pos = m_nSelNo;

    if(!AXList::deleteItem(pos))
        return FALSE;
    else
    {
        _changeSel();
        return TRUE;
    }
}


//=======================


//! 位置からアイテム取得

AXListBoxItem *AXListBoxItemManager::getItemFromPos(int pos)
{
    if(pos < 0) pos = m_nSelNo;

    return (AXListBoxItem *)AXList::getItem(pos);
}

//! 選択変更

BOOL AXListBoxItemManager::setSel(int pos)
{
    if(pos < -1 || pos >= m_nCnt)
        pos = -1;

    if(pos == m_nSelNo)
        return FALSE;
    else if(pos == -1)
    {
        _setSelNone();
        return TRUE;
    }
    else
    {
        m_nSelNo    = pos;
        m_pSelItem  = AXList::getItem(pos);

        return TRUE;
    }
}

//! 選択変更

BOOL AXListBoxItemManager::setSel(AXListBoxItem *pItem)
{
    if(m_pSelItem == pItem)
        return FALSE;
    else
    {
        if(!pItem)
            _setSelNone();
        else
        {
            m_pSelItem = pItem;
            m_nSelNo   = AXList::getPos(pItem);
        }

        return TRUE;
    }
}

//! 選択を上下に移動

BOOL AXListBoxItemManager::moveSelUpDown(int dir)
{
    int pos = m_nSelNo;

    if(m_nCnt == 0) return FALSE;

    if(pos == -1)
        pos = 0;
    else
    {
        pos += dir;

        if(pos < 0)
            pos = 0;
        else if(pos >= m_nCnt - 1)
            pos = m_nCnt - 1;
    }

    return setSel(pos);
}


//=========================


//! テキスト取得

void AXListBoxItemManager::getItemText(int pos,AXString *pstr)
{
    AXListBoxItem *p;

    p = getItemFromPos(pos);

    if(p)
        *pstr = p->m_strText;
    else
        pstr->empty();
}

//! テキスト変更

BOOL AXListBoxItemManager::setItemText(int pos,LPCUSTR pstr)
{
    AXListBoxItem *p;

    p = getItemFromPos(pos);

    if(!p)
        return FALSE;
    else
    {
        p->m_strText = pstr;
        return TRUE;
    }
}

//! パラメータ値取得

ULONG AXListBoxItemManager::getItemParam(int pos)
{
    AXListBoxItem *p;

    p = getItemFromPos(pos);

    if(p)
        return p->m_lParam;
    else
        return 0;
}

//! パラメータ値セット

void AXListBoxItemManager::setItemParam(int pos,ULONG lParam)
{
    AXListBoxItem *p;

    p = getItemFromPos(pos);
    if(p) p->m_lParam = lParam;
}

//! パラメータ値検索

int AXListBoxItemManager::findItemParam(ULONG lParam)
{
    AXListBoxItem *p;
    int i;

    for(i = 0, p = getTopItem(); p; i++, p = p->next())
    {
        if(p->m_lParam == lParam) return i;
    }

    return -1;
}

//! テキスト検索

int AXListBoxItemManager::findItemText(LPCUSTR pstr)
{
    AXListBoxItem *p;
    int i;

    for(i = 0, p = getTopItem(); p; i++, p = p->next())
    {
        if(p->m_strText == pstr) return i;
    }

    return -1;
}

//! 指定アイテムを上下に移動

BOOL AXListBoxItemManager::moveItemUpDown(int pos,BOOL bDown)
{
    AXListBoxItem *p,*p2;
    AXString str;
    ULONG param;

    p = getItemFromPos(pos);
    if(!p) return FALSE;

    //移動先

    if(bDown)
        p2 = p->next();
    else
        p2 = p->prev();

    if(!p2) return FALSE;

    //データ入れ替え

    str   = p->m_strText;
    param = p->m_lParam;

    p->m_strText = p2->m_strText;
    p->m_lParam  = p2->m_lParam;

    p2->m_strText = str;
    p2->m_lParam  = param;

    //選択

    if(p == m_pSelItem)
    {
        if(bDown) m_nSelNo++; else m_nSelNo--;

        m_pSelItem = p2;
    }

    return TRUE;
}
