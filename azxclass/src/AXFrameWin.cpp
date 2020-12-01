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

#include "AXFrameWin.h"

#include "AXAppRes.h"


/*!
    @class AXFrameWin
    @brief フレーム付きウィンドウ

    - デフォルトでへこみ枠。

    @ingroup window
*/
/*!
    @var AXFrameWin::FS_SUNKEN
    @brief へこみ枠
    @var AXFrameWin::FS_FRAME
    @brief 1px枠（FRAMEDARK）
*/


AXFrameWin::~AXFrameWin()
{

}

AXFrameWin::AXFrameWin(AXWindow *pParent,UINT uStyle,UINT uLayoutFlags)
    : AXWindow(pParent, uStyle, uLayoutFlags)
{

}

AXFrameWin::AXFrameWin(AXWindow *pParent,UINT uStyle,UINT uLayoutFlags,UINT uItemID,DWORD dwPadding)
    : AXWindow(pParent, uStyle, uLayoutFlags, uItemID, dwPadding)
{

}

//! 範囲取得

void AXFrameWin::getClientRect(AXRect *prc)
{
    prc->setFromSize(0, 0, m_nW, m_nH);
    prc->deflate(1);
}

//! 描画

BOOL AXFrameWin::onPaint(AXHD_PAINT *phd)
{
    if(m_uStyle & FS_FRAME)
        drawBox(0, 0, m_nW, m_nH, AXAppRes::FRAMEDARK);
    else
        drawFrameSunken(0, 0, m_nW, m_nH);

    return TRUE;
}
