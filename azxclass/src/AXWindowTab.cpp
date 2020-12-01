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

#include "AXWindowTab.h"

#include "AXTabItem.h"
#include "AXLayout.h"
#include "AXApp.h"


/*!
    @class AXWindowTab
    @brief タブの中身がウィンドウのタブ

    - 中身のウィンドウはタブの子として作成する。
    - ウィンドウポインタはタブアイテムの lParam にセットされる。

    @ingroup widget
*/


AXWindowTab::~AXWindowTab()
{

}

AXWindowTab::AXWindowTab(AXWindow *pParent,UINT uStyle,UINT uLayoutFlags)
    : AXTab(pParent, uStyle, uLayoutFlags)
{
    _createWindowTab();
}

AXWindowTab::AXWindowTab(AXWindow *pParent,UINT uStyle,UINT uLayoutFlags,UINT uItemID,DWORD dwPadding)
    : AXTab(pParent, uStyle, uLayoutFlags, uItemID, dwPadding)
{
    _createWindowTab();
}

//! 作成

void AXWindowTab::_createWindowTab()
{
    m_pwinCurrent = NULL;

    //レイアウト

    setLayout(new AXLayoutVert);
}

//! 通知先

AXWindow *AXWindowTab::getNotify()
{
    return this;
}

//! 標準サイズ計算

void AXWindowTab::calcDefSize()
{
    AXTabItem *p;
    AXWindow *pwin;
    int maxw = 1,maxh = 1;

    for(p = (AXTabItem *)m_pDat->getTop(); p; p = p->next())
    {
        pwin = (AXWindow *)p->m_lParam;

        if(pwin)
        {
            pwin->calcDefSize();

            if(maxw < pwin->getDefW()) maxw = pwin->getDefW();
            if(maxh < pwin->getDefH()) maxh = pwin->getDefH();
        }
    }

    AXTab::_calcTabH();

    m_nDefW = maxw + 2;
    m_nDefH = maxh + m_nTabH + 1;
}


//================================
//
//================================


//! タブの中身のウィンドウ取得

AXWindow *AXWindowTab::getTabWindow(int no)
{
    return (AXWindow *)AXTab::getItemParam(no);
}

//! 翻訳文字列からタブ追加

void AXWindowTab::addTabItemTr(WORD wStrID,AXWindow *pwin)
{
    AXTab::addItem(_str(wStrID), -1, (ULONG)pwin);
}

//! タブの中身のウィンドウセット

void AXWindowTab::setTabWindow(int no,AXWindow *pwin)
{
    AXTab::setItemParam(no, (ULONG)pwin);
}

//! 選択変更

void AXWindowTab::setTabSel(int no)
{
    AXTab::setCurSel(no);

    _changeTabSel();
}



//=======================


//! 選択変更処理

void AXWindowTab::_changeTabSel()
{
    //現在のを非表示にして取り外す

    m_pLayout->removeItemAll();

    if(m_pwinCurrent)
        m_pwinCurrent->hide();

    //選択されているものをセット

    if(AXTab::m_pFocus)
        m_pwinCurrent = (AXWindow *)m_pFocus->m_lParam;
    else
        m_pwinCurrent = NULL;

    if(m_pwinCurrent)
    {
        m_pwinCurrent->show();

        m_pLayout->addItem(m_pwinCurrent);
    }

    //レイアウト

    if(m_uFlags & FLAG_LAYOUTED)
        layout();
}

//! 通知

BOOL AXWindowTab::onNotify(AXWindow *pwin,UINT uNotify,ULONG lParam)
{
    if(pwin == this && uNotify == TABN_SELCHANGE)
    {
        _changeTabSel();

        return TRUE;
    }

    return FALSE;
}
