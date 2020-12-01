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

#include "AXColorButton.h"

#include "AXColorDialog.h"
#include "AXAppRes.h"


/*!
    @class AXColorButton
    @brief 色ボタン

    @ingroup widget
*/

/*!
    @var AXColorButton::CBTS_CHOOSE
    @brief ボタンが押された時、色選択ダイアログで色を選択させる

    @var AXColorButton::CBTN_PRESS
    @brief ボタンが押された（色選択ありの場合は、色が変更された）。lParam = 現在色。
*/


AXColorButton::~AXColorButton()
{

}

AXColorButton::AXColorButton(AXWindow *pParent,UINT uStyle,UINT uLayoutFlags)
    : AXButton(pParent, uStyle, uLayoutFlags)
{
    _createColorButton(0xffffff);
}

AXColorButton::AXColorButton(AXWindow *pParent,UINT uStyle,UINT uLayoutFlags,UINT uItemID,DWORD dwPadding)
    : AXButton(pParent, uStyle, uLayoutFlags, uItemID, dwPadding)
{
    _createColorButton(0xffffff);
}

AXColorButton::AXColorButton(AXWindow *pParent,UINT uStyle,UINT uLayoutFlags,UINT uItemID,DWORD dwPadding,DWORD col)
    : AXButton(pParent, uStyle, uLayoutFlags, uItemID, dwPadding)
{
    _createColorButton(col & 0xffffff);
}

//! 作成

void AXColorButton::_createColorButton(DWORD col)
{
    m_uType = TYPE_COLORBUTTON;
    m_dwCol = col;

    m_gc.createColor(col);
}

//! 標準サイズ計算

void AXColorButton::calcDefSize()
{
    m_nDefW = 60;
    m_nDefH = 22;
}

//! 色セット

void AXColorButton::setColor(DWORD col)
{
    col &= 0xffffff;

    if(col != m_dwCol)
    {
        m_dwCol = col;

        m_gc.createColor(col);

        redraw();
    }
}


//==============================
//ハンドラ
//==============================


//! ボタン押し時

BOOL AXColorButton::onPress()
{
    BOOL bNotify = TRUE;

    if(m_uStyle & CBTS_CHOOSE)
    {
        //色選択

        if(!AXColorDialog::getColor(m_pTopLevel, &m_dwCol))
            bNotify = FALSE;
        else
        {
            m_gc.createColor(m_dwCol);
            redraw();
        }
    }

    //通知

    if(bNotify)
        getNotify()->onNotify(this, CBTN_PRESS, m_dwCol);

    return TRUE;
}

//! 描画

BOOL AXColorButton::onPaint(AXHD_PAINT *phd)
{
    UINT flag;
    int add;

    //ボタン

    flag = 0;
    if(isPress()) flag |= DRAWBUTTON_DOWN;
    if(m_uFlags & FLAG_FOCUSED) flag |= DRAWBUTTON_FOCUSED;
    if(m_uFlags & FLAG_DEFAULTBUTTON) flag |= DRAWBUTTON_DEFAULT;
    if(!(m_uFlags & FLAG_ENABLED)) flag |= DRAWBUTTON_DISABLE;

    drawButton(0, 0, m_nW, m_nH, flag);

    //色

    if(m_uFlags & FLAG_ENABLED)
    {
        add = (isPress())? 1: 0;

        drawBox(4 + add, 4 + add, m_nW - 8, m_nH - 8, AXAppRes::BLACK);
        drawFillBox(5 + add, 5 + add, m_nW - 10, m_nH - 10, m_gc);
    }
    else
    {
        //無効時

        drawBox(4, 4, m_nW - 8, m_nH - 8, AXAppRes::FRAMELIGHT);
    }

    return TRUE;
}
