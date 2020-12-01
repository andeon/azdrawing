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

#include "CBrushWin.h"

#include "CBrushWin_sizeList.h"
#include "CBrushWin_tree.h"
#include "CBrushWin_value.h"

#include "CBrushManager.h"
#include "CBrushTreeItem.h"
#include "CMainWin.h"
#include "CConfig.h"
#include "CResource.h"

#include "defGlobal.h"
#include "defStrID.h"

#include "AXLayout.h"
#include "AXSplitter.h"
#include "AXToolBar.h"
#include "AXApp.h"


/*!
    @class CBrushWin
    @brief ブラシウィンドウ
*/


CBrushWin *CBrushWin::m_pSelf = NULL;


CBrushWin::CBrushWin(AXWindow *pOwner,UINT addstyle)
    : AXTopWindow(pOwner,
                  WS_HIDE | WS_TITLE | WS_CLOSE | WS_BORDER | addstyle)
{
    AXLayout *pl,*plv;
    AXString str;

    m_pSelf = this;

    //

    setTitle("brush");

    setFont(g_pres->m_pfontSubWin);

    attachAccel(((AXTopWindow *)m_pOwner)->getAccel());

    //ウィジェット

    setLayout(pl = new AXLayoutVert);

    pl->addItem(m_pSizeList = new CBrushWin_sizeList(this));
    pl->addItem(new AXSplitter(this, AXSplitter::SPLS_VERT));

    pl->addItem(plv = new AXLayoutVert(LF_EXPAND_WH, 0));
    plv->addItem(m_ptbTree = new AXToolBar(this, AXToolBar::TBS_TOOLTIP | AXToolBar::TBS_TOOLTIP_TR, LF_EXPAND_W));
    plv->addItem(m_pTree = new CBrushWin_tree(this));

    pl->addItem(new AXSplitter(this, AXSplitter::SPLS_VERT));
    pl->addItem(m_pValue = new CBrushWin_value(this));

    _setTB();

    //-------- ブラシデータ読み込み

    axapp->getConfigPath(&str, "brush.dat");

    BRUSHMAN->loadFile(str, m_pTree);
}

//! ツールバーセット

void CBrushWin::_setTB()
{
    int i;
    WORD id[] = {
        STRID_BRMENU_ADD_BRUSH, STRID_BRMENU_ADD_FOLDER, STRID_BRMENU_COPY,
        STRID_BRMENU_DELETE, STRID_BRMENU_RENAME, STRID_BRMENU_PASTE
    };

    m_ptbTree->setImageList(g_pres->m_pilBrushTreeTB);
    m_ptbTree->setToolTipTrGroup(STRGID_BRUSHMENU);

    for(i = 0; i < 6; i++)
        m_ptbTree->addItem(id[i], i, id[i], AXToolBar::BF_BUTTON, 0);
}

//! ブラシデータ保存（CDraw のデストラクタから）

void CBrushWin::saveBrushFile()
{
    AXString str;

    axapp->getConfigPath(&str, "brush.dat");
    BRUSHMAN->saveFile(str, m_pTree->getTopItem(), m_pTree->getFocusItem());
}

//! 表示後の初期化

void CBrushWin::initAfterShow()
{
    AXTreeViewItem *p = BRUSHMAN->getSelBrush();

    //選択アイテムの状態セット

    if(p)
    {
        m_pTree->setFocusItem(p);

        m_pTree->reconfig();
        m_pTree->setScrollItem(p, 1);
    }

    _changeBrushSel(p);
}

//! 表示/非表示 切り替え

void CBrushWin::showChange()
{
    if(g_pconf->isViewBrushWin())
        showRestore();
    else
        hide();
}

//! 子ウィンドウの高さ取得

void CBrushWin::getWinH()
{
    g_pconf->nBrushWinH[0] = m_pSizeList->getHeight();
    g_pconf->nBrushWinH[1] = m_pValue->getHeight();
}

//! ブラシサイズを直接セット

void CBrushWin::setBrushSize(int size)
{
    BRUSHMAN->changeBrushSize(size, TRUE);

    m_pValue->setBrushSize();
}

//! ブラシの濃度を直接セット（スポイト時など）

void CBrushWin::setBrushVal(int val)
{
    BRUSHMAN->changeVal(val);

    m_pValue->setBrushVal();
}

//! ブラシ選択を上下に移動

void CBrushWin::moveBrushSelUpDown(BOOL bDown)
{
    AXTreeViewItem *p = m_pTree->getFocusItem();

    if(!p) return;

    //前後のブラシ項目取得

    while(1)
    {
        if(bDown)
            p = p->nextVisible();
        else
            p = p->prevVisible();

        if(!p) return;

        if(p->m_lParam == CBrushTreeItem::PARAM_BRUSH)
            break;
    }

    //選択変更

    m_pTree->setFocusItem(p);
    _changeBrushSel(p);
}

//! 登録ブラシを選択

void CBrushWin::setSelRegBrush(int no)
{
    CBrushTreeItem *p = BRUSHMAN->getRegBrush(no);

    if(p)
    {
        m_pTree->setFocusItem(p);
        _changeBrushSel(p);
    }
}

//! ツリーアイテム選択変更時
/*!
    @param pItem NULL の場合あり
*/

void CBrushWin::_changeBrushSel(AXTreeViewItem *pItem)
{
    int i,f;
    CBrushTreeItem *p;

    //ツールバーボタンの有効/無効

    if(!pItem)
        f = 0;
    else
    {
        //新規ブラシ・新規フォルダ・貼り付け
        f = (1<<0)|(1<<1)|(1<<5);

        switch(pItem->m_lParam)
        {
            case CBrushTreeItem::PARAM_FOLDER:
                f |= (1<<3)|(1<<4);
                break;
            case CBrushTreeItem::PARAM_BRUSH:
                f |= (1<<2)|(1<<3)|(1<<4);
                break;
        }
    }

    for(i = 0; i < 6; i++)
        m_ptbTree->enableItemNo(i, f & (1 << i));

    //ブラシ以外の場合 NULL に

    p = (CBrushTreeItem *)pItem;
    if(p && p->m_lParam != CBrushTreeItem::PARAM_BRUSH) p = NULL;

    //ブラシ選択変更処理

    BRUSHMAN->changeSelBrush(p);

    m_pValue->changeBrush();
}


//=========================
//ハンドラ
//=========================


//! 閉じる（非表示）

BOOL CBrushWin::onClose()
{
    m_pOwner->onCommand(STRID_MENU_VIEW_BRUSHWIN, 0, 0);

    return TRUE;
}

//! 通知

BOOL CBrushWin::onNotify(AXWindow *pwin,UINT uNotify,ULONG lParam)
{
    if(pwin == m_pTree)
    {
        switch(uNotify)
        {
            //ツリーアイテム選択変更時
            case CBrushWin_tree::NOTIFY_SELCHANGE:
                _changeBrushSel((AXTreeViewItem *)lParam);
                break;
        }
    }

    return TRUE;
}

//! コマンド

BOOL CBrushWin::onCommand(UINT uID,ULONG lParam,int from)
{
    m_pTree->funcCommand(uID);

    return TRUE;
}
