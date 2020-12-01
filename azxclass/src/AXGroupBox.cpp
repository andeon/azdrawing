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

#include "AXGroupBox.h"

#include "AXAppRes.h"


#define LABEL_SPACE       3
#define LABEL_TEXTSPACE   2

/*!
    @class AXGroupBox
    @brief グループボックスウィジェット

    - テキストが空の場合は、ラベルなしで枠のみ。
    - グループボックスは、内部にレイアウトを持つ形。
    - グループボックスの内部のアイテムは、グループボックスの子供にする。
    - ボックスの内側余白は、レイアウトの setSpacing() で指定する。
    - デフォルトで、グループボックスの子ウィンドウの通知先は、グループボックスの通知先と同じになる。

    @ingroup widget
*/
/*!
    @var AXGroupBox::GBS_BKLIGHT
    @brief 背景色を FACELIGHT に
*/


AXGroupBox::~AXGroupBox()
{

}

AXGroupBox::AXGroupBox(AXWindow *pParent,UINT uStyle,UINT uLayoutFlags)
    : AXWindow(pParent, uStyle, uLayoutFlags)
{
    _createGroupBox();
}

AXGroupBox::AXGroupBox(AXWindow *pParent,UINT uStyle,UINT uLayoutFlags,DWORD dwPadding)
    : AXWindow(pParent, uStyle, uLayoutFlags, 0, dwPadding)
{
    _createGroupBox();
}

AXGroupBox::AXGroupBox(AXWindow *pParent,UINT uStyle,UINT uLayoutFlags,DWORD dwPadding,LPCUSTR pstr)
    : AXWindow(pParent, uStyle, uLayoutFlags, 0, dwPadding)
{
    _createGroupBox();

    m_strText = pstr;
}

//! 作成

void AXGroupBox::_createGroupBox()
{
    m_uType   = TYPE_GROUPBOX;
    m_uFlags |= AXWindow::FLAG_CHILD_NOTIFY_PARENT;
}

//! ラベルセット

void AXGroupBox::setText(const AXString &str)
{
    m_strText = str;

    if(isLayouted())
    {
        calcDefSize();
        redraw();
    }
}

//! 標準サイズ計算

void AXGroupBox::calcDefSize()
{
    int w;

    //レイアウト全体のサイズ

    AXWindow::calcDefSize();

    //枠分

    if(m_strText.isEmpty())
        m_sizeText.zero();
    else
    {
        m_pFont->getTextSize(m_strText, &m_sizeText);

        w = m_sizeText.w + LABEL_SPACE * 2 + LABEL_TEXTSPACE * 2;
        if(m_nDefW < w) m_nDefW = w;
    }

    m_nDefW += 4;
    m_nDefH += m_sizeText.h + 2;
}

//! 範囲取得

void AXGroupBox::getClientRect(AXRect *prc)
{
    prc->left   = 2;
    prc->top    = m_sizeText.h;
    prc->right  = m_nW - 3;
    prc->bottom = m_nH - 3;
}

//! 描画

BOOL AXGroupBox::onPaint(AXHD_PAINT *phd)
{
    int fy;

    //背景

    drawFillBox(0, 0, m_nW, m_nH, (m_uStyle & GBS_BKLIGHT)? AXAppRes::FACELIGHT: AXAppRes::FACE);

    //枠

    fy = m_sizeText.h / 2;

    drawBox(1, fy + 1, m_nW - 1, m_nH - fy - 1, AXAppRes::FRAMELIGHT);
    drawBox(0, fy    , m_nW - 1, m_nH - fy - 1, AXAppRes::FRAMEDARK);

    //ラベル

    if(m_strText.isNoEmpty())
    {
        //背景（枠を消す）

        drawFillBox(2 + LABEL_SPACE, fy, m_sizeText.w + LABEL_TEXTSPACE * 2, 2, AXAppRes::FACE);

        //テキスト

        AXDrawText dt(m_id);

        dt.draw(*m_pFont, 2 + LABEL_SPACE + LABEL_TEXTSPACE, 0, m_strText);

        dt.end();
    }

    return TRUE;
}
