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

#include "AXMenuBar.h"

#include "AXMenuWindow.h"
#include "AXMenuItem.h"
#include "AXMenu.h"
#include "AXApp.h"
#include "AXAppRes.h"


#define SPACE_X     6
#define SPACE_Y     3

//-----------------------

/*!
    @class AXMenuBar
    @brief メニューバーウィジェット

    - 削除時は、サブメニューも含め削除される。

    @ingroup widget
*/

/*!
    @var AXMenuBar::MBS_SEP_BOTTOM
    @brief 下にセパレータ枠を付ける
*/


AXMenuBar::~AXMenuBar()
{
    //サブメニューも含め削除

    delete m_pDat;
}

AXMenuBar::AXMenuBar(AXWindow *pParent,UINT uStyle,UINT uLayoutFlags)
    : AXWindow(pParent, uStyle, uLayoutFlags)
{
    _createMenuBar();
}

AXMenuBar::AXMenuBar(AXWindow *pParent,UINT uStyle,UINT uLayoutFlags,UINT uItemID,DWORD dwPadding)
    : AXWindow(pParent, uStyle, uLayoutFlags, uItemID, dwPadding)
{
    _createMenuBar();
}

//! 作成

void AXMenuBar::_createMenuBar()
{
    m_uType     = TYPE_MENUBAR;
    m_pSelItem  = NULL;

    m_pDat = new AXMenu;
}

//! 標準サイズ計算

void AXMenuBar::calcDefSize()
{
    m_nDefW = 1;
    m_nDefH = m_pFont->getHeight() + SPACE_Y * 2;

    if(m_uStyle & MBS_SEP_BOTTOM) m_nDefH += 5;

    //アイテムサイズ

    m_pDat->initAllItem(m_pFont);
}

//-----------------

//! サブメニュー取得

AXMenu *AXMenuBar::getSubMenu(UINT uID)
{
    AXMenuItem *p = m_pDat->getItem(uID);

    if(p)
        return p->m_pSubMenu;
    else
        return NULL;
}

//! アイテム追加

void AXMenuBar::addItem(UINT uID,UINT uFlags,LPCUSTR pstr,AXMenu *pSubMenu)
{
    m_pDat->add(uID, uFlags, pstr, pSubMenu, 0);
}

//! アイテム追加（翻訳文字列から）

void AXMenuBar::addItemTr(WORD wStrID,AXMenu *pSubMenu)
{
    m_pDat->add(wStrID, 0, _str(wStrID), pSubMenu, 0);
}

//! アイテムチェックON/OFF（全メニュー項目対象）

void AXMenuBar::checkItem(UINT uID,BOOL bCheck)
{
    m_pDat->check(uID, bCheck);
}

//! 指定IDの項目のサブメニューセット（全メニュー項目対象）

void AXMenuBar::setItemSubMenu(UINT uID,AXMenu *pSubMenu)
{
    m_pDat->setSubMenu(uID, pSubMenu);
}

//! IDの配列データからメニューバーとメニューの項目を自動で追加
/*!
    ・メニューバー項目のIDはメニュー項目のIDより小さい値であること。@n
    ・文字列ID は 0x8000 未満の値であること。@n
    ・0xffff はセパレータ。@n
    ・0xfffe でサブメニューの開始と終了（1階層のみでネスト不可。直前の項目が親項目）@n
    ・0x8000 のフラグが ON で自動チェックありのメニュー項目。@n

    @param cmdIDMin この値より小さい値の場合はメニューバーの項目と見なす
*/

void AXMenuBar::addItemTrMultiple(const WORD *pIDArray,int cnt,int cmdIDMin)
{
    AXMenu *pNow = NULL,*pBack = NULL;
    AXMenuItem *pLastItem;
    int id;

    for(; cnt > 0; cnt--)
    {
        id = *(pIDArray++);

        if(id == 0xfffe)
        {
            //サブメニュー開始or終了

            if(pNow)
            {
                if(pBack)
                {
                    //終了して戻る
                    pNow  = pBack;
                    pBack = NULL;
                }
                else
                {
                    //開始

                    pLastItem = pNow->getLastItem();

                    if(pLastItem)
                    {
                        pBack = pNow;
                        pNow  = new AXMenu;
                        pLastItem->m_pSubMenu = pNow;
                    }
                }
            }
        }
        else if(id < cmdIDMin)
        {
            //メニューバーの項目

            pNow = new AXMenu;
            addItemTr(id, pNow);
        }
        else if(pNow)
        {
            //メニュー項目

            if(id == 0xffff)
                pNow->addSep();
            else if(id & 0x8000)
                pNow->addTrCheck(id & 0x7fff, FALSE);
            else
                pNow->addTr(id);
        }
    }
}


//===========================
//サブ処理
//===========================


//! マウス位置からアイテム取得

AXMenuItem *AXMenuBar::_getCurPosItem(int x)
{
    AXMenuItem *p;
    int pos = 0;

    for(p = m_pDat->getTopItem(); p; p = p->next())
    {
        if(pos > x) break;
        if(pos <= x && x < pos + p->m_nTextW + SPACE_X * 2)
            return p;

        pos += p->m_nTextW + SPACE_X * 2;
    }

    return NULL;
}

//! アイテムから、メニューの表示位置（ルート座標）取得

void AXMenuBar::_getShowItemPos(AXMenuItem *pItem,AXPoint *ppt)
{
    AXMenuItem *p;
    int x = 0,y;

    for(p = m_pDat->getTopItem(); p && p != pItem; p = p->next())
        x += p->m_nTextW + SPACE_X * 2;

    y = m_pFont->getHeight() + SPACE_Y * 2;
    translateTo(NULL, &x, &y);

    ppt->set(x, y);
}

//! サブメニュー表示

void AXMenuBar::_showSubMenu(AXMenuItem *pItem)
{
    AXMenuWindow *pmenu;
    AXPoint pt;

    //※マウス移動により表示アイテムが変わる場合があるので、繰り返す

    do
    {
        m_pSelItem = pItem;
        redraw();

        //

        _getShowItemPos(pItem, &pt);

        pmenu = new AXMenuWindow(m_pTopLevel, pItem->m_pSubMenu);

        pItem = pmenu->showFromMenuBar(this, pt.x, pt.y, pItem->m_uID);

    }while(pItem);

    //終了

    m_pSelItem = NULL;
    redraw();
}

//! ポップアップ表示中のマウス移動処理（AXMenuWindow::onMouseMove 時に呼ばれる）
//! @return 新たに表示させるアイテム。NULLで変更なし。

AXMenuItem *AXMenuBar::_movePopup(AXMenuWindow *pTopPopup,int x,int y)
{
    AXMenuItem *p;

    translateFrom(NULL, &x, &y);

    if(isContain(x, y))
    {
        p = _getCurPosItem(x);

        if(p && p != m_pSelItem && p->isEnableSubMenu())
            return p;
    }

    return NULL;
}

//! トップレベルでアタッチされたメニューバーから Alt+ のホットキーで表示開始

BOOL AXMenuBar::_showFromHotKey(UINT key,UINT state)
{
    AXMenuItem *p;

    if((state & STATE_MODMASK) != STATE_ALT) return FALSE;
    if(key <= 32 || key >= 127) return FALSE;

    p = m_pDat->findHotKey((char)key);
    if(!p) return FALSE;

    _showSubMenu(p);

    return TRUE;
}


//==============================
//ハンドラ
//==============================


//! ボタン押し時

BOOL AXMenuBar::onButtonDown(AXHD_MOUSE *phd)
{
    AXMenuItem *p;

    if(phd->button == BUTTON_LEFT)
    {
        p = _getCurPosItem(phd->x);

        if(p && p->isEnableSubMenu())
            _showSubMenu(p);
    }

    return TRUE;
}

//! 描画

BOOL AXMenuBar::onPaint(AXHD_PAINT *phd)
{
    AXMenuItem *p;
    int x,h,fh;

    //背景

    drawFillBox(0, 0, m_nW, m_nH, AXAppRes::FACE);

    //アイテム

    fh = m_pFont->getHeight();
    h  = fh + SPACE_Y * 2;

    AXDrawText dt(m_id);

    for(p = m_pDat->getTopItem(), x = 0; p; p = p->next())
    {
        if(p == m_pSelItem)
        {
            //選択時

            drawFillBox(x, 0, p->m_nTextW + SPACE_X * 2, h, AXAppRes::BACKREVTEXT);

            dt.draw(*m_pFont, x + SPACE_X, SPACE_Y, p->m_strText, p->m_nAccTextPos, AXAppRes::TC_REV);

            if(p->m_cHotKey)
                drawLineH(x + SPACE_X + p->m_wHotKeyX, SPACE_Y + fh, p->m_wHotKeyW, AXAppRes::TEXTREV);
        }
        else
        {
            //通常時

            dt.draw(*m_pFont, x + SPACE_X, SPACE_Y, p->m_strText, p->m_nAccTextPos,
                            (p->m_uFlags & AXMenu::MIF_DISABLE)? AXAppRes::TC_DISABLE: AXAppRes::TC_NORMAL);

            if(p->m_cHotKey)
                drawLineH(x + SPACE_X + p->m_wHotKeyX, SPACE_Y + fh, p->m_wHotKeyW, AXAppRes::TEXTNORMAL);
        }

        x += p->m_nTextW + SPACE_X * 2;
    }

    dt.end();

    //下枠

    if(m_uStyle & MBS_SEP_BOTTOM)
    {
        drawLineH(0, h + 1, m_nW, AXAppRes::FRAMEDARK);
        drawLineH(0, h + 2, m_nW, AXAppRes::FRAMELIGHT);
    }

    return TRUE;
}
