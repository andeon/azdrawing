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

#include "AXArrowButton.h"

#include "AXAppRes.h"


/*!
    @class AXArrowButton
    @brief 矢印ボタンウィジェット

    @ingroup widget
*/
/*!
    @var AXArrowButton::ARBTS_UP
    @brief 上矢印
    @var AXArrowButton::ARBTS_DOWN
    @brief 下矢印
    @var AXArrowButton::ARBTS_LEFT
    @brief 左矢印
    @var AXArrowButton::ARBTS_RIGHT
    @brief 右矢印
*/


AXArrowButton::~AXArrowButton()
{

}

AXArrowButton::AXArrowButton(AXWindow *pParent,UINT uStyle,UINT uLayoutFlags)
    : AXButton(pParent, uStyle, uLayoutFlags)
{

}

AXArrowButton::AXArrowButton(AXWindow *pParent,UINT uStyle,UINT uLayoutFlags,UINT uItemID,DWORD dwPadding)
    : AXButton(pParent, uStyle, uLayoutFlags, uItemID, dwPadding)
{

}

//! 標準サイズ計算

void AXArrowButton::calcDefSize()
{
    m_nDefW = 17;
    m_nDefH = 17;
}

//! 描画

BOOL AXArrowButton::onPaint(AXHD_PAINT *phd)
{
    UINT n;
    int bPress,col;

    bPress = (isPress())? 1: 0;
    col    = (m_uFlags & FLAG_ENABLED)? AXAppRes::TEXTNORMAL: AXAppRes::TEXTDISABLE;

    //ボタン

    n = 0;
    if(bPress) n |= DRAWBUTTON_DOWN;
    if(m_uFlags & FLAG_FOCUSED) n |= DRAWBUTTON_FOCUSED;
    if(m_uFlags & FLAG_DEFAULTBUTTON) n |= DRAWBUTTON_DEFAULT;
    if(!(m_uFlags & FLAG_ENABLED)) n= DRAWBUTTON_DISABLE;

    drawButton(0, 0, m_nW, m_nH, n);

    //矢印

    n = m_uStyle & (ARBTS_UP | ARBTS_DOWN | ARBTS_LEFT | ARBTS_RIGHT);

    switch(n)
    {
        case ARBTS_UP:
            drawArrowUp(m_nW / 2 + bPress, m_nH / 2 + bPress, col);
            break;
        case ARBTS_LEFT:
            drawArrowLeft(m_nW / 2 + bPress, m_nH / 2 + bPress, col);
            break;
        case ARBTS_RIGHT:
            drawArrowRight(m_nW / 2 + bPress, m_nH / 2 + bPress, col);
            break;
        //下（デフォルト）
        default:
            drawArrowDown(m_nW / 2 + bPress, m_nH / 2 + bPress, col);
            break;
    }

    return TRUE;
}
