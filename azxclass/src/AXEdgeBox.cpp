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

#include "AXEdgeBox.h"

#include "AXAppRes.h"


/*!
    @class AXEdgeBox
    @brief スクロールビューの余白部分など、単に色を表示するだけのウィジェット

    - デフォルトで FACE 色。

    @ingroup widget
*/


AXEdgeBox::~AXEdgeBox()
{

}

AXEdgeBox::AXEdgeBox(AXWindow *pParent,UINT uStyle,UINT uLayoutFlags)
    : AXWindow(pParent, uStyle, uLayoutFlags)
{
    m_uType = TYPE_EDGEBOX;
}

//! 描画

BOOL AXEdgeBox::onPaint(AXHD_PAINT *phd)
{
    drawFillBox(0, 0, m_nW, m_nH, AXAppRes::FACE);

    return TRUE;
}
