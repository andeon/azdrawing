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

#include "AXColorPrev.h"

#include "AXAppRes.h"


/*!
    @class AXColorPrev
    @brief カラープレビューウィジェット

    @ingroup widget
*/

/*!
    @var AXColorPrev::CPS_SUNKEN
    @brief へこみ枠を付ける
    @var AXColorPrev::CPS_FRAME
    @brief 1px枠を付ける
*/


AXColorPrev::~AXColorPrev()
{

}

AXColorPrev::AXColorPrev(AXWindow *pParent,UINT uStyle,UINT uLayoutFlags)
    : AXWindow(pParent, uStyle, uLayoutFlags)
{
    _createColorPrev();
}

AXColorPrev::AXColorPrev(AXWindow *pParent,UINT uStyle,UINT uLayoutFlags,UINT uItemID,DWORD dwPadding)
    : AXWindow(pParent, uStyle, uLayoutFlags, uItemID, dwPadding)
{
    _createColorPrev();
}

//! 作成

void AXColorPrev::_createColorPrev()
{
    m_uType = TYPE_COLORPREV;
    m_dwCol = 0xffffff;

    m_gc.createColor(m_dwCol);
}

//! 色セット

void AXColorPrev::setColor(DWORD col)
{
    col &= 0xffffff;

    if(col != m_dwCol)
    {
        m_dwCol = col;

        m_gc.createColor(m_dwCol);

        redraw();
    }
}

//! 描画

BOOL AXColorPrev::onPaint(AXHD_PAINT *phd)
{
    if(m_uStyle & CPS_SUNKEN)
    {
        //へこみ枠
        drawFrameSunken(0, 0, m_nW, m_nH);
        drawFillBox(1, 1, m_nW - 2, m_nH - 2, m_gc);
    }
    else if(m_uStyle & CPS_FRAME)
    {
        //通常枠
        drawBox(0, 0, m_nW, m_nH, AXAppRes::FRAMEOUTSIDE);
        drawFillBox(1, 1, m_nW - 2, m_nH - 2, m_gc);
    }
    else
        drawFillBox(0, 0, m_nW, m_nH, m_gc);

    return TRUE;
}
