/************************************************************************
*  Copyright (C) 2013-2015 Azel.
*
*  This file is part of AzDrawing.
*
*  AzDrawing is free software: you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation, either version 3 of the License, or
*  (at your option) any later version.
*
*  AzDrawing is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*************************************************************************/

#include "CCmdListTreeView.h"

#include "CConfig.h"
#include "CDraw.h"
#include "CKeyDat.h"
#include "CMainWin.h"

#include "AXMenu.h"
#include "AXMenuBar.h"
#include "AXMenuItem.h"
#include "AXTreeView.h"
#include "AXMessageBox.h"
#include "AXApp.h"
#include "AXUtilX.h"
#include "AXKey.h"

#include "defGlobal.h"
#include "defStrID.h"

#include "menudat.h"

//---------------------------

#define TREEPARAM_NONE  0xffffffff

//---------------------------


/*!
    @class CCmdListTreeView
    @brief キー設定/デバイス設定時のコマンドリストツリービュー
*/
/*
    ツリーアイテム

    m_lParam  : TREEPARAM_NONE で設定項目ではない
                <キー設定時> [上位WORD]コマンドID [下位WORD]キー
                <デバイス設定時> コマンドID
    m_nIconNo : 元のテキストの長さ（負の値）
*/


CCmdListTreeView::CCmdListTreeView(AXWindow *pParent,UINT uStyle,UINT uLayoutFlags,UINT uItemID,DWORD dwPadding)
    : AXTreeView(pParent, uStyle | SVS_HORZVERTSUNKEN, uLayoutFlags, uItemID, dwPadding)
{
    m_bKey = (uStyle & STYLE_KEY);
}

//! キーデータをリストからセット（キー設定時）

void CCmdListTreeView::setKeyDat()
{
    AXTreeViewItem *p;
    int cnt = 0;
    LPDWORD pbuf;

    //キーデータ数

    for(p = AXTreeView::getTopItem(); p; p = (AXTreeViewItem *)p->nextTreeItem())
    {
        if(p->m_lParam != TREEPARAM_NONE && (p->m_lParam & 0xffff))
            cnt++;
    }

    //確保

    if(!g_pconf->pKey->alloc(cnt)) return;

    //セット

    pbuf = g_pconf->pKey->getBuf();

    for(p = AXTreeView::getTopItem(); p; p = (AXTreeViewItem *)p->nextTreeItem())
    {
        if(p->m_lParam != TREEPARAM_NONE && (p->m_lParam & 0xffff))
            *(pbuf++) = (DWORD)p->m_lParam;
    }
}

//! 全てクリア（キー設定時）

void CCmdListTreeView::clearAll()
{
    AXTreeViewItem *p;

    //キーをクリア＆テキスト変更

    for(p = AXTreeView::getTopItem(); p; p = (AXTreeViewItem *)p->nextTreeItem())
    {
        if(p->m_lParam != TREEPARAM_NONE && (p->m_lParam & 0xffff))
        {
            p->m_lParam &= 0xffff0000;
            p->m_uFlags |= AXTreeViewItem::FLAG_GRAY;

            p->m_strText.end(-(p->m_nIconNo));

            AXTreeView::updateItem(p);
        }
    }
}

//! コマンドIDから選択（デバイス設定時）

void CCmdListTreeView::setSelCmd(int cmdid)
{
    AXTreeViewItem *p;

    if(cmdid == 0xffff)
        //デフォルトと同じ
        p = AXTreeView::getTopItem();
    else
        p = AXTreeView::findItemParam(cmdid);

    if(p)
    {
        AXTreeView::setFocusItem(p);
        AXTreeView::setScrollItem(p, 1);
    }
}

//! リストセット（共通）

void CCmdListTreeView::setCmdList()
{
    int addDev;

    addDev = (m_bKey)? 0: 1;

    //デバイス設定時

    if(!m_bKey)
    {
        _trgroup(STRGID_COMMANDLIST);

        _add_item(NULL, 0xffff, 0); //デフォルトと同じ
        _add_item(NULL, 0, 1);      //何もしない
        _add_item(NULL, 1, 2);      //メイン操作
    }

    //キー＋[ツール]
    _add_itemMul(STRID_CMDLIST_KEY_TOOL + addDev, STRGID_TOOLNAME, 0, 2000, CDraw::TOOL_NUM);

    //キー＋[描画タイプ]
    _add_itemMul(STRID_CMDLIST_KEY_DRAWTYPE + addDev, STRGID_DRAWTYPE, 0, 2100, CDraw::DRAWTYPE_NUM);

    //キー＋[登録ブラシ]
    _set_registBrush(STRID_CMDLIST_KEY_REGBRUSH + addDev, 2200);

    //キー＋[登録ブラシ](筆圧最大)
    _set_registBrush(STRID_CMDLIST_KEY_REGBRUSH_PRESSMAX + addDev, 2250);

    //キー＋[その他]
    _add_itemMul(STRID_CMDLIST_KEY_ETC + addDev, STRGID_COMMANDLIST, 2300, 2300, 4);

    //メインメニュー
    _set_mainmenu();

    //レイヤメニュー
    _set_layermenu();

    //ツール選択
    _add_itemMul(STRID_CMDLIST_TOOL, STRGID_TOOLNAME, 0, 1000, CDraw::TOOL_NUM);

    //描画タイプ選択
    _add_itemMul(STRID_CMDLIST_DRAWTYPE, STRGID_DRAWTYPE, 0, 1100, CDraw::DRAWTYPE_NUM);

    //レイヤ拡張コマンド
    _add_itemMul(STRID_CMDLIST_LAYEREXCMD, STRGID_COMMANDLIST, 1200, 1200, 5);

    //他コマンド
    _add_itemMul(STRID_CMDLIST_ETCCMD, STRGID_COMMANDLIST, 1300, 1300, 8);

    //登録ブラシ選択
    _set_registBrush(STRID_CMDLIST_REGBRUSH, 1400);
}

//! キー押し

BOOL CCmdListTreeView::onKeyDown(AXHD_KEY *phd)
{
    UINT key = phd->keysym;
    UINT exKey[] = {
        KEY_INSERT, KEY_NUM_INSERT, KEY_DELETE, KEY_NUM_DELETE,
        KEY_BACKSPACE, KEY_CTRL_L, KEY_CTRL_R, KEY_SHIFT_L, KEY_SHIFT_R,
        KEY_ALT_L, KEY_ALT_R, KEY_SUPER_L, KEY_SUPER_R
    };
    UINT i;
    AXTreeViewItem *p;

    //キー設定時のみ

    if(!m_bKey) return TRUE;

    //キーが範囲外

    if(key >= 0xffff) return TRUE;

    //除外キー

    for(i = 0; i < sizeof(exKey)/sizeof(UINT); i++)
    {
        if(key == exKey[i]) return TRUE;
    }

    //Spaceはクリア

    if(key == KEY_SPACE || key == KEY_NUM_SPACE)
        key = 0;

    //選択アイテム

    p = AXTreeView::getFocusItem();
    if(!p) return TRUE;

    if(p->m_lParam == TREEPARAM_NONE) return TRUE;

    if((p->m_lParam & 0xffff) == key) return TRUE;

    //他のコマンドで使われているか

    if(_check_sameKey(key)) return TRUE;

    //セット

    p->m_lParam &= 0xffff0000;
    p->m_lParam |= key;

    //灰色/通常

    if(key)
        p->m_uFlags &= ~AXTreeViewItem::FLAG_GRAY;
    else
        p->m_uFlags |= AXTreeViewItem::FLAG_GRAY;

    //項目名変更

    p->m_strText.end(-(p->m_nIconNo));

    if(key)
    {
        AXString name;

        AXGetKeysymString(&name, key);

        p->m_strText += " = [";
        p->m_strText += name;
        p->m_strText += ']';
    }

    AXTreeView::updateItem(p);

    return TRUE;
}


//==============================
// サブ
//==============================


//! キー押し時、他のコマンドで同じキーが使われているか

BOOL CCmdListTreeView::_check_sameKey(UINT key)
{
    AXTreeViewItem *p;

    if(key == 0) return FALSE;

    for(p = AXTreeView::getTopItem(); p; p = (AXTreeViewItem *)p->nextTreeItem())
    {
        if(p->m_lParam != TREEPARAM_NONE && (p->m_lParam & 0xffff) == key)
        {
            AXMessageBox::error(m_pTopLevel, _string(STRGID_MESSAGE, STRID_MES_KEY_SAMEKEY));
            return TRUE;
        }
    }

    return FALSE;
}


//==============================
// リストセット
//==============================


//! メインメニュー

void CCmdListTreeView::_set_mainmenu()
{
    AXTreeViewItem *pParent;

    pParent = AXTreeView::addItem(NULL, _string(STRGID_COMMANDLIST, STRID_CMDLIST_MAINMENU), -1,
                   AXTreeViewItem::FLAG_EXPAND, TREEPARAM_NONE);

    _trgroup(STRGID_MAINMENU);

    _set_mainmenu_sub((MAINWIN->getMenuBar())->getMenu(), pParent);
}

//! メインメニュー、サブメニュー処理（再帰）

void CCmdListTreeView::_set_mainmenu_sub(AXMenu *pMenu,AXTreeViewItem *pParent)
{
    AXTreeViewItem *pSub;
    AXMenuItem *pi;
    UINT id;

    for(pi = pMenu->getTopItem(); pi; pi = pi->next())
    {
        id = pi->m_uID;

        if(id == STRID_MENU_TOP_HELP)
            //「ヘルプ」で終了
            break;
        else if(id == STRID_MENU_FILE_RECENT || (pi->m_uFlags & AXMenu::MIF_SEP))
            //「最近使ったファイル」とセパレータは除く
            continue;
        else if(pi->m_pSubMenu)
        {
            //サブメニュー（再帰）

            pSub = AXTreeView::addItem(pParent, _str(id), -1, AXTreeViewItem::FLAG_EXPAND, TREEPARAM_NONE);

            _set_mainmenu_sub(pi->m_pSubMenu, pSub);
        }
        else
            //項目
            _add_item(pParent, id);
    }
}

//! レイヤメニュー

void CCmdListTreeView::_set_layermenu()
{
    AXTreeViewItem *pParent;
    int i;
    LPWORD pw;

    pParent = AXTreeView::addItem(NULL, _string(STRGID_COMMANDLIST, STRID_CMDLIST_LAYERMENU), -1,
                   AXTreeViewItem::FLAG_EXPAND, TREEPARAM_NONE);

    _trgroup(STRGID_LAYERWINMENU);

    //

    pw = g_layermenudat;

    for(i = sizeof(g_layermenudat)/sizeof(WORD); i > 0; i--, pw++)
    {
        if(*pw != 0xffff)
            _add_item(pParent, *pw);
    }
}

//! 登録ブラシ

void CCmdListTreeView::_set_registBrush(WORD wTopStrID,int cmdid)
{
    AXTreeViewItem *pParent;
    int i;

    pParent = AXTreeView::addItem(NULL, _string(STRGID_COMMANDLIST, wTopStrID), -1,
                   AXTreeViewItem::FLAG_EXPAND, TREEPARAM_NONE);

    for(i = 0; i < 5; i++)
        _add_item(pParent, cmdid + i, -100 - i);
}


//==============================
// リストセットサブ
//==============================


//! ツリーアイテム追加
/*!
    @param cmdid コマンドID
    @param strid 文字列ID（-1 でコマンドIDと同じ。-100〜で登録ブラシ）
*/

void CCmdListTreeView::_add_item(AXTreeViewItem *pParent,UINT cmdid,int strid)
{
    AXString str,name;
    int key,len;

    //テキスト

    if(strid <= -100)
    {
        str = "No.";
        str += (-strid - 100 + 1);
    }
    else
    {
        if(strid == -1)
            strid = cmdid;

        str = _str(strid);
        str.findAndEnd('\t');
    }

    len = str.getLen();

    //

    if(!m_bKey)
        //デバイス設定
        AXTreeView::addItem(pParent, str, -1, 0, cmdid);
    else
    {
        //キー設定

        key = g_pconf->pKey->getKey(cmdid);

        if(key)
        {
            AXGetKeysymString(&name, key);

            str += " = [";
            str += name;
            str += ']';
        }

        AXTreeView::addItem(pParent, str, -len, (key)? 0: AXTreeViewItem::FLAG_GRAY, (cmdid << 16) | key);
    }
}

//! 連続したアイテム追加

void CCmdListTreeView::_add_itemMul(WORD wTopStrID,WORD wGroupID,int strid,int cmdid,int cnt)
{
    AXTreeViewItem *pParent;

    pParent = AXTreeView::addItem(NULL, _string(STRGID_COMMANDLIST, wTopStrID), -1,
                   AXTreeViewItem::FLAG_EXPAND, TREEPARAM_NONE);

    _trgroup(wGroupID);

    for(; cnt > 0; cnt--, strid++, cmdid++)
        _add_item(pParent, cmdid, strid);
}
