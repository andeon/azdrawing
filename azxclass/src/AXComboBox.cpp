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

#include "AXComboBox.h"

#include "AXListBoxItem.h"
#include "AXListBoxItemManager.h"
#include "AXPopupListBox.h"
#include "AXApp.h"
#include "AXAppRes.h"
#include "AXKey.h"
#include "AXUtilStr.h"


#define FRAMEW          1   //枠太さ
#define TEXTSPACEX      3   //テキストのX余白（枠は含まない）
#define SPACEY          2   //Y余白（枠も含む）
#define ITEMSPACEY      1   //リストボックス項目のY余白

//--------------------------

/*!
    @class AXComboBox
    @brief コンボボックスウィジェット

    - スペースキーでポップアップ、上下キー/マウスホイールで選択移動。
    - pos は -1 で現在の選択位置。

    @ingroup widget
*/

/*!
    @var AXComboBox::CBS_OWNERDRAW
    @brief アイテムをオーナードロー

    @var AXComboBox::CBN_SELCHANGE
    @brief 選択変更時
*/


AXComboBox::~AXComboBox()
{
    delete m_pDat;
}

AXComboBox::AXComboBox(AXWindow *pParent,UINT uStyle,UINT uLayoutFlags)
    : AXWindow(pParent, uStyle, uLayoutFlags)
{
    _createComboBox();
}

AXComboBox::AXComboBox(AXWindow *pParent,UINT uStyle,UINT uLayoutFlags,UINT uItemID,DWORD dwPadding)
    : AXWindow(pParent, uStyle, uLayoutFlags, uItemID, dwPadding)
{
    _createComboBox();
}

//! 作成

void AXComboBox::_createComboBox()
{
    m_uType  = TYPE_COMBOBOX;
    m_uFlags |= FLAG_TAKE_FOCUS;

    m_pDat = new AXListBoxItemManager;

    m_nItemH = m_pFont->getHeight() + 2;
}

//! 標準サイズ計算

void AXComboBox::calcDefSize()
{
    //高さ

    m_nDefH = SPACEY * 2 + m_nItemH;
    if(m_nDefH < 9) m_nDefH = 9;

    //幅

    m_nDefW = (FRAMEW + TEXTSPACEX) * 2 + 5 + (m_nDefH - 2);
}

//! アイテム高さセット

void AXComboBox::setItemH(int h)
{
    m_nItemH = (h < 9)? 9: h;
}


//==============================
//アイテム
//==============================
/*
    データを追加しても、選択が変わらない限り表示状態は変わらないので、
    redraw() を行う必要はない。
*/


//! 先頭アイテム取得

AXListBoxItem *AXComboBox::getTopItem()
{
    return m_pDat->getTopItem();
}

//! 選択アイテム取得

AXListBoxItem *AXComboBox::getSelItem()
{
    return m_pDat->getSelItem();
}

//! アイテム追加
//! @return 追加された位置

int AXComboBox::addItem(LPCUSTR pstr)
{
    return m_pDat->addItem(pstr, 0);
}

//! アイテム追加

int AXComboBox::addItem(LPCUSTR pstr,ULONG lParam)
{
    return m_pDat->addItem(pstr, lParam);
}

//! アイテム追加(ASCII文字列)

int AXComboBox::addItem(LPCSTR pText,ULONG lParam)
{
    AXString str(pText);
    return m_pDat->addItem(str, lParam);
}

//! アイテム追加（直接データを）

int AXComboBox::addItem(AXListBoxItem *pItem)
{
    return m_pDat->addItem(pItem);
}

//! 文字列IDからアイテム追加

int AXComboBox::addItemTr(WORD wStrID,ULONG lParam)
{
    return m_pDat->addItem(_str(wStrID), lParam);
}

//! 連続した翻訳文字列IDから複数アイテム追加

void AXComboBox::addItemMulTr(int startID,int cnt)
{
    for(; cnt > 0; cnt--, startID++)
        m_pDat->addItem(_str(startID), 0);
}

//! ソートして追加（全アイテムが順に並んでいることが前提）

int AXComboBox::addItemSort(LPCUSTR pstr,ULONG lParam)
{
    int cnt,div,i;
    AXListBoxItem *p,*pn;

    cnt = m_pDat->getCnt();
    if(cnt == 0)
        return addItem(pstr, lParam);

    //先頭位置

    div = (cnt + 9) / 10;
    if(div < 2) div = 2;

    for(p = m_pDat->getTopItem(), i = 1; i < div; i++)
    {
        pn = m_pDat->getItemFromPos(cnt * i / div);

        if(AXUStrCompare(pstr, pn->m_strText) <= 0) break;

        p = pn;
    }

    //比較

    for(; p; p = p->next())
    {
        if(AXUStrCompare(pstr, p->m_strText) < 0) break;
    }

    if(p)
        return m_pDat->insertItem(m_pDat->getPos(p), pstr, lParam);
    else
        return m_pDat->addItem(pstr, lParam);
}

//! アイテム挿入

int AXComboBox::insertItem(int pos,LPCUSTR pstr,ULONG lParam)
{
    return m_pDat->insertItem(pos, pstr, lParam);
}

//! アイテムすべて削除

void AXComboBox::deleteItemAll()
{
    m_pDat->deleteAllItem();
    redraw();
}

//! アイテム削除

void AXComboBox::deleteItem(int pos)
{
    if(m_pDat->deleteItemPos(pos))
        redraw();
}

//! アイテム削除

void AXComboBox::deleteItem(AXListBoxItem *pItem)
{
    m_pDat->deleteItem(pItem);
    redraw();
}

//----------------

//! アイテム数取得

int AXComboBox::getItemCnt()
{
    return m_pDat->getCnt();
}

//! 現在の選択番号取得
//! @return -1 で選択なし

int AXComboBox::getCurSel()
{
    return m_pDat->getSel();
}

//! 選択変更

void AXComboBox::setCurSel(int pos)
{
    if(m_pDat->setSel(pos))
        redraw();
}

//! 指定パラメータ値を検索し、見つかったアイテムを選択

void AXComboBox::setCurSel_findParam(ULONG lParam)
{
    int no = findItemParam(lParam);
    if(no != -1) setCurSel(no);
}

//-----------------

//! テキスト取得

void AXComboBox::getItemText(int pos,AXString *pstr)
{
    m_pDat->getItemText(pos, pstr);
}

//! パラメータ値取得

ULONG AXComboBox::getItemParam(int pos)
{
    return m_pDat->getItemParam(pos);
}

//! テキストセット

void AXComboBox::setItemText(int pos,LPCUSTR pstr)
{
    if(m_pDat->setItemText(pos, pstr))
        redraw();
}

//! パラメータ値セット

void AXComboBox::setItemParam(int pos,ULONG lParam)
{
    m_pDat->setItemParam(pos, lParam);
}

//-------------------

//! パラメータ値検索
//! @return 見つかった場合、アイテム位置。-1 で見つからなかった。

int AXComboBox::findItemParam(ULONG lParam)
{
    return m_pDat->findItemParam(lParam);
}

//! テキスト検索

int AXComboBox::findItemText(LPCUSTR pstr)
{
    return m_pDat->findItemText(pstr);
}

//! 全アイテムの幅から最小幅セット

void AXComboBox::setAutoWidth()
{
    AXListBoxItem *p;
    int w,max = 0;

    calcDefSize();

    for(p = m_pDat->getTopItem(); p; p = p->next())
    {
        w = m_pFont->getTextWidth(p->m_strText);
        if(w > max) max = w;
    }

    m_nMinW = max + (FRAMEW + TEXTSPACEX) * 2 + (m_nDefH - 2);
}


//==============================
//サブ処理
//==============================


//! リストをポップアップ

void AXComboBox::_popupList()
{
    UINT style;
    int selbk,h,x,y;

    if(m_pDat->getCnt() == 0) return;

    selbk = m_pDat->getSel();

    //

    style = (m_uStyle & CBS_OWNERDRAW)? AXPopupListBox::PLBS_OWNERDRAW: 0;

    h = m_nItemH * m_pDat->getCnt();
    if(h > 300)
    {
        h = 300;
        style |= AXPopupListBox::PLBS_SCRV;
    }

    x = 0, y = m_nH;
    translateTo(NULL, &x, &y);

    //表示

    new AXPopupListBox(this, style, m_pDat, x, y, m_nW, h, m_nItemH);

    //選択が変わっていた場合

    if(m_pDat->getSel() != selbk)
        _notifySel();
}

//! 選択変更時の通知

void AXComboBox::_notifySel()
{
    getNotify()->sendNotify(this, CBN_SELCHANGE, m_pDat->getSel());
    redraw();
}


//==============================
//ハンドラ
//==============================


//! ボタン押し時

BOOL AXComboBox::onButtonDown(AXHD_MOUSE *phd)
{
    if(phd->button == BUTTON_LEFT)
    {
        setFocus();
        _popupList();
    }

    return TRUE;
}

//! キー押し時

BOOL AXComboBox::onKeyDown(AXHD_KEY *phd)
{
    if(ISKEY_SPACE(phd->keysym))
        //ポップアップ
        _popupList();
    else if(ISKEY_UP(phd->keysym))
    {
        //一つ前を選択

        if(m_pDat->moveSelUpDown(-1))
            _notifySel();
    }
    else if(ISKEY_DOWN(phd->keysym))
    {
        //一つ次を選択

        if(m_pDat->moveSelUpDown(1))
            _notifySel();
    }
    else
        return FALSE;

    return TRUE;
}

//! マウスホイール（選択移動）

BOOL AXComboBox::onMouseWheel(AXHD_MOUSE *phd,BOOL bUp)
{
    if(m_pDat->moveSelUpDown((bUp)? -1: 1))
        _notifySel();

    return TRUE;
}

//! 描画

BOOL AXComboBox::onPaint(AXHD_PAINT *phd)
{
    BOOL bEnable;
    AXListBoxItem *p;
    AXHD_OWNERDRAW od;
    int boxsize,n;

    bEnable = m_uFlags & FLAG_ENABLED;
    boxsize = m_nH - 2;

    //枠

    drawFrameSunken(0, 0, m_nW, m_nH);

    //------ 内側背景

    n = m_nW - boxsize - 2;

    if(!bEnable)
        //無効時
        drawFillBox(1, 1, n, m_nH - 2, AXAppRes::FACE);
    else if(m_uFlags & FLAG_FOCUSED)
    {
        //フォーカス時
        drawBox(1, 1, n, m_nH - 2, AXAppRes::FACEDOC);
        drawFillBox(2, 2, n - 2, m_nH - 4, AXAppRes::BACKREVTEXT);
    }
    else
        //通常時
        drawFillBox(1, 1, n, m_nH - 2, AXAppRes::FACEDOC);

    //------- 項目

    p = m_pDat->getSelItem();

    if(p)
    {
        AXDrawText dt(m_id);
        AXRectSize rcs;

        rcs.set(FRAMEW + 1, SPACEY, m_nW - (FRAMEW + 1) * 2 - boxsize, m_nH - SPACEY * 2);
        dt.setClipRect(rcs);

        if(m_uStyle & CBS_OWNERDRAW)
        {
            //オーナードロー

            od.pwinFrom = this;
            od.nItemNo  = m_pDat->getSel();
            od.uState   = 0;
            if(bEnable) od.uState |= ODS_ENABLED;
            if(m_uFlags & FLAG_FOCUSED) od.uState |= ODS_SELECTED | ODS_FOCUSED;
            od.pItem    = (LPVOID)p;
            od.lParam1  = p->m_lParam;
            od.lParam2  = (ULONG)(LPCUSTR)p->m_strText;
            od.pDrawText = &dt;
            od.rcs      = rcs;

            getNotify()->onOwnerDraw(this, &od);
        }
        else
        {
            //通常

            if(m_uFlags & FLAG_FOCUSED)
                n = AXAppRes::TC_REV;
            else
                n = (bEnable)? AXAppRes::TC_NORMAL: AXAppRes::TC_DISABLE;

            dt.draw(*m_pFont, FRAMEW + TEXTSPACEX, (m_nH - m_pFont->getHeight()) / 2, p->m_strText, n);
        }

        dt.end();
    }

    //矢印ボックス

    n = m_nW - FRAMEW - boxsize;

    drawFrameRaise(n, FRAMEW, boxsize, boxsize);
    drawFillBox(n + 1, FRAMEW + 1, boxsize - 2, boxsize - 2, AXAppRes::FACE);

    drawArrowDown(n + boxsize / 2, m_nH / 2, (bEnable)? AXAppRes::TEXTNORMAL: AXAppRes::TEXTDISABLE);

    return TRUE;
}
