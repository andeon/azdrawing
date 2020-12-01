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

#include "AXMultiEdit.h"

#include "AXMultiEditArea.h"
#include "AXEditString.h"
#include "AXString.h"
#include "AXScrollBar.h"
#include "AXFont.h"


#define SCRAREA     ((AXMultiEditArea *)m_pScrArea)

/*!
    @class AXMultiEdit
    @brief 複数行エディットウィジェット

    - 改行は \\n で統一。
    - スクロールは水平/垂直共にあり。自動で表示/非表示される。

    @ingroup widget
*/

/*!
    @var AXMultiEdit::MES_READONLY
    @brief 読み込み専用

    @var AXMultiEdit::MEN_CHANGE
    @brief テキスト内容が変化した
*/


AXMultiEdit::~AXMultiEdit()
{
    delete m_pString;
}

AXMultiEdit::AXMultiEdit(AXWindow *pParent,UINT uStyle,UINT uLayoutFlags)
    : AXScrollView(pParent, uStyle | SVS_HORZVERT | SVS_SUNKEN, uLayoutFlags)
{
    _createMultiEdit();
}

AXMultiEdit::AXMultiEdit(AXWindow *pParent,UINT uStyle,UINT uLayoutFlags,UINT uItemID,DWORD dwPadding)
    : AXScrollView(pParent, uStyle | SVS_HORZVERT | SVS_SUNKEN, uLayoutFlags, uItemID, dwPadding)
{
    _createMultiEdit();
}

//! 作成

void AXMultiEdit::_createMultiEdit()
{
    m_uType  = TYPE_MULTIEDIT;
    m_uFlags |= FLAG_TAKE_FOCUS;

    m_pString = new AXEditString;
    m_pString->setMultiLine();

    m_pScrArea = new AXMultiEditArea(this,
                        (m_uStyle & MES_READONLY)? AXMultiEditArea::MEAS_READONLY: 0, m_pString);
}

//! 再構成

void AXMultiEdit::reconfig()
{
    SCRAREA->setScrollInfo();

    AXScrollView::reconfig();

    m_pScrArea->redraw();
}

//! (IM) 前編集表示位置

void AXMultiEdit::getICPos(AXPoint *pPos)
{
    SCRAREA->getICPos(pPos);
}

//! ENTERを許可

BOOL AXMultiEdit::isAcceptKey(UINT keytype)
{
    return (keytype == ACCEPTKEYTYPE_ENTER);
}


//=============================
//
//=============================


//! テキストセット

void AXMultiEdit::setText(LPCUSTR pText)
{
    m_pString->setText(pText, m_pFont);

    m_pScrH->setPos(0);
    m_pScrV->setPos(0);

    setReconfig();
}

//! テキスト取得

void AXMultiEdit::getText(AXString *pstr)
{
    m_pString->getString(pstr);
}

//! テキストの長さ取得

int AXMultiEdit::getTextLen()
{
    return m_pString->getLen();
}

//! すべて選択（フォーカスが自身にあることが前提）

void AXMultiEdit::selectAll()
{
    if(isFocused() && m_pString->selectAll())
        m_pScrArea->redraw();
}


//=============================
//ハンドラ
//=============================


//! フォーカスIN

BOOL AXMultiEdit::onFocusIn(int detail)
{
    if(detail == FOCUSDETAIL_TABMOVE) m_pString->selectAll();
    m_pScrArea->redraw();

    return TRUE;
}

//! フォーカスOUT

BOOL AXMultiEdit::onFocusOut(int detail)
{
    m_pString->clearSel();
    m_pScrArea->redraw();

    return TRUE;
}

//! キー押し

BOOL AXMultiEdit::onKeyDown(AXHD_KEY *phd)
{
    return m_pScrArea->onKeyDown(phd);
}
