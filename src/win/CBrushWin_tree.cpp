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

#include "CBrushWin_tree.h"

#include "CBrushTreeItem.h"
#include "CBrushManager.h"
#include "CResource.h"

#include "AXMenu.h"
#include "AXMessageBox.h"
#include "AXStrDialog.h"
#include "AXClipboard.h"
#include "AXApp.h"

#include "defGlobal.h"
#include "defStrID.h"


//---------------------

//[AXTreeViewItem::m_lParam]

#define ITEMTYPE_TOP    0
#define ITEMTYPE_FOLDER 1
#define ITEMTYPE_BRUSH  2

//---------------------


/*!
    @class CBrushWin_tree
    @brief [ブラシウィンドウ]ブラシリストのツリービュー
*/


CBrushWin_tree::CBrushWin_tree(AXWindow *pParent)
    : AXTreeView(pParent, SVS_HORZVERT | SVS_SUNKEN | TVS_DND, LF_EXPAND_WH)
{
    m_nMinH = 50;

    //アイコン

    AXTreeView::setImageList(g_pres->m_pilBrushTreeIcon);

    //トップ項目

    addItem(NULL, _string(STRGID_WORD, STRID_WORD_BRUSH), -1, AXTreeViewItem::FLAG_EXPAND, ITEMTYPE_TOP);
}

//! 通知を自身で受け取る

AXWindow *CBrushWin_tree::getNotify()
{
    return this;
}


//============================
//ハンドラ
//============================


//! 通知

BOOL CBrushWin_tree::onNotify(AXWindow *pwin,UINT uNotify,ULONG lParam)
{
    AXTreeViewItem *pItem;

    if(AXTreeView::onNotify(pwin, uNotify, lParam))
        return TRUE;

    //

    pItem = (AXTreeViewItem *)lParam;

    switch(uNotify)
    {
        //選択変更
        case TVN_SELCHANGE:
            _notifyChange(AXTreeView::getFocusItem());
            break;
        //右ボタン押し
        case TVN_RBUTTON:
            if(pItem)
                _showMenu(pItem);
            break;
        //ダブルクリック（名前変更）
        case TVN_DBLCLK:
            _rename(pItem);
            break;
        //D&D開始（トップ項目は除く）
        case TVN_DRAG_BEGIN:
            m_bDragFolder = (pItem->m_lParam != ITEMTYPE_BRUSH);
            return (pItem->m_lParam != ITEMTYPE_TOP);
        //D&D移動中
        case TVN_DRAG_DST:
            if(m_bDragFolder)
            {
                //ドラッグ元がフォルダの場合、ドロップ先がドロップ元の子の場合は除く
                return !AXTreeView::isItemChild(pItem, AXTreeView::getDragSrcItem());
            }
            else
                return TRUE;
        //D&D終了
        case TVN_DRAG_END:
            {
            AXTreeViewItem **p = (AXTreeViewItem **)lParam;
            if(p[1])
            {
                if(p[1]->m_lParam == ITEMTYPE_BRUSH)
                    //ドラッグ先がブラシ -> ブラシの上へ
                    AXTreeView::moveItem(p[0], p[1]);
                else
                    //ドラッグ先がトップ/フォルダ -> フォルダの最後へ
                    AXTreeView::moveItemLast(p[0], p[1]);
            }
            }
            break;
    }

    return TRUE;
}


//============================
//
//============================


//! 選択変更時の通知

void CBrushWin_tree::_notifyChange(AXTreeViewItem *p)
{
    m_pNotify->onNotify(this, NOTIFY_SELCHANGE, (ULONG)p);
}

//! メニュー表示

void CBrushWin_tree::_showMenu(AXTreeViewItem *pItem)
{
    AXMenu *pm;
    AXPoint pt;
    int n,i;
    WORD id_t[] = { STRID_BRMENU_ADD_BRUSH, STRID_BRMENU_ADD_FOLDER, STRID_BRMENU_PASTE };
    WORD id_f[] = {
        STRID_BRMENU_ADD_BRUSH, STRID_BRMENU_ADD_FOLDER, 0xffff,
        STRID_BRMENU_DELETE, STRID_BRMENU_RENAME, STRID_BRMENU_PASTE
    };
    WORD id_b[] = {
        STRID_BRMENU_ADD_BRUSH, STRID_BRMENU_ADD_FOLDER,
        STRID_BRMENU_COPY, STRID_BRMENU_DELETE,
        STRID_BRMENU_RENAME, 0xffff, STRID_BRMENU_PASTE, STRID_BRMENU_COPYTEXT,
        0xffff, STRID_BRMENU_REGCLEAR, STRID_BRMENU_REG1, STRID_BRMENU_REG2,
        STRID_BRMENU_REG3, STRID_BRMENU_REG4, STRID_BRMENU_REG5
    };

    //------ メニュー

    _trgroup(STRGID_BRUSHMENU);

    pm = new AXMenu;

    switch(pItem->m_lParam)
    {
        case ITEMTYPE_TOP:
            pm->addTrArray(id_t, sizeof(id_t)/sizeof(WORD));
            break;
        case ITEMTYPE_FOLDER:
            pm->addTrArray(id_f, sizeof(id_f)/sizeof(WORD));
            break;
        //ブラシ
        case ITEMTYPE_BRUSH:
            pm->addTrArray(id_b, sizeof(id_b)/sizeof(WORD));

            //登録ブラシ

            n = BRUSHMAN->getRegBrushNo((CBrushTreeItem *)pItem);
            pm->enable(STRID_BRMENU_REGCLEAR, (n != -1));

            for(i = 0; i < CBrushManager::REGITEM_NUM; i++)
                pm->enable(STRID_BRMENU_REG1 + i, (n == -1 && BRUSHMAN->getRegBrush(i) == NULL));
            break;
    }

    //

    axapp->getCursorPos(&pt);
    n = (int)pm->popup(NULL, pt.x, pt.y, 0);

    delete pm;

    //------ 処理

    if(n != -1)
        funcCommand(n);
}


//============================
//コマンド
//============================


//! コマンド処理

void CBrushWin_tree::funcCommand(int id)
{
    AXTreeViewItem *pItem = AXTreeView::getFocusItem();

    switch(id)
    {
        //ブラシ追加
        case STRID_BRMENU_ADD_BRUSH:
            _addBrush(pItem);
            break;
        //フォルダ追加
        case STRID_BRMENU_ADD_FOLDER:
            _addFolder(pItem);
            break;
        //複製
        case STRID_BRMENU_COPY:
            _copyItem((CBrushTreeItem *)pItem);
            break;
        //削除
        case STRID_BRMENU_DELETE:
            _delItem(pItem);
            break;
        //名前の変更
        case STRID_BRMENU_RENAME:
            _rename(pItem);
            break;
        //貼り付け
        case STRID_BRMENU_PASTE:
            if(!_paste(pItem))
                ((AXTopWindow *)m_pTopLevel)->errMes(STRGID_MESSAGE, STRID_MES_ERR_PASTE);
            break;
        //コピー（テキスト形式）
        case STRID_BRMENU_COPYTEXT:
            _copyCBText((CBrushTreeItem *)pItem);
            break;
        //登録ブラシ指定解除
        case STRID_BRMENU_REGCLEAR:
            BRUSHMAN->setRegBrush((CBrushTreeItem *)pItem, -1);
            AXTreeView::redrawArea();
            break;
        //登録ブラシ指定
        case STRID_BRMENU_REG1:
        case STRID_BRMENU_REG2:
        case STRID_BRMENU_REG3:
        case STRID_BRMENU_REG4:
        case STRID_BRMENU_REG5:
            BRUSHMAN->setRegBrush((CBrushTreeItem *)pItem, id - STRID_BRMENU_REG1);
            AXTreeView::redrawArea();
            break;
    }
}

//! ブラシ追加（選択も変更）

void CBrushWin_tree::_addBrush(AXTreeViewItem *pItem)
{
    CBrushTreeItem *p;

    p = BRUSHMAN->newBrush();

    if(pItem->m_lParam == ITEMTYPE_BRUSH)
        AXTreeView::insertItem(pItem, p);
    else
        AXTreeView::addItem(pItem, p);

    //選択変更

    AXTreeView::setFocusItem(p);

    _notifyChange(p);
}

//! フォルダ追加（選択も変更）

void CBrushWin_tree::_addFolder(AXTreeViewItem *pItem)
{
    AXString str("new");
    AXTreeViewItem *pNew;

    //追加

    if(pItem->m_lParam == ITEMTYPE_BRUSH)
        pNew = AXTreeView::insertItem(pItem, str, 0, 0, ITEMTYPE_FOLDER);
    else
        pNew = AXTreeView::addItem(pItem, str, 0, 0, ITEMTYPE_FOLDER);

    //選択

    AXTreeView::setFocusItem(pNew);

    _notifyChange(pNew);
}

//! ブラシ複製

void CBrushWin_tree::_copyItem(CBrushTreeItem *pItem)
{
    CBrushTreeItem *p;

    p = BRUSHMAN->cloneBrush(pItem);

    AXTreeView::insertItem(pItem, p);

    //選択変更

    AXTreeView::setFocusItem(p);

    _notifyChange(p);
}

//! 削除
/*!
    削除後は選択なしの状態
*/

void CBrushWin_tree::_delItem(AXTreeViewItem *pItem)
{
    //フォルダの場合、確認

    if(pItem->m_lParam == ITEMTYPE_FOLDER)
    {
        if(AXMessageBox::message(m_pTopLevel, NULL, _string(STRGID_MESSAGE, STRID_MES_BRUSH_FOLDER_DEL),
                                 AXMessageBox::YES | AXMessageBox::NO,
                                 AXMessageBox::YES) == AXMessageBox::NO)
                    return;
    }

    //ブラシの場合、削除処理

    if(pItem->m_lParam == ITEMTYPE_BRUSH)
        BRUSHMAN->delBrush((CBrushTreeItem *)pItem);

    //削除

    AXTreeView::delItem(pItem);

    //選択をなしに

    _notifyChange(NULL);
}

//! 名前の変更

void CBrushWin_tree::_rename(AXTreeViewItem *pItem)
{
    AXString str;

    if(!pItem) return;
    if(((CBrushTreeItem *)pItem)->m_lParam == ITEMTYPE_TOP) return;

    str = pItem->m_strText;

    if(AXStrDialog::getString(m_pTopLevel, NULL, _string(STRGID_WORD, STRID_WORD_NAME), &str))
        AXTreeView::setItemText(pItem, str);
}

//! 貼り付け

BOOL CBrushWin_tree::_paste(AXTreeViewItem *pItem)
{
    AXString str;
    CBrushTreeItem *p;

    if(!axclipb->getText(m_id, &str)) return FALSE;

    //作成

    p = BRUSHMAN->newBrushFromText(str);
    if(!p) return FALSE;

    if(pItem->m_lParam == ITEMTYPE_BRUSH)
        AXTreeView::insertItem(pItem, p);
    else
        AXTreeView::addItem(pItem, p);

    //選択変更

    AXTreeView::setFocusItem(p);
    _notifyChange(p);

    return TRUE;
}

//! クリップボードにコピー

void CBrushWin_tree::_copyCBText(CBrushTreeItem *pItem)
{
    AXString str;

    pItem->getTextFormat(&str);

    axclipb->setText(m_id, str);
}
