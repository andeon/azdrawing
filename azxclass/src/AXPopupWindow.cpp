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

#include "AXPopupWindow.h"

#include "AXApp.h"
#include "AXKey.h"


/*!
    @class AXPopupWindow
    @brief ポップアップウィンドウ派生元クラス

    - オーナーウィンドウは、トップレベルウィンドウ。
    - デフォルトスタイル（WS_HIDE/WS_TRANSIENT_FOR/WS_DISABLE_WM）
    - カーソルがウィンドウ外にある場合は、ウィンドウ外でのボタン押しを検知するためポインタをグラブする。@n
      ウィンドウ内にある場合、グラブは解除する。
    - ウィンドウ外がクリックされた時、またはESCキーで終了。

    @ingroup window
*/


AXPopupWindow::~AXPopupWindow()
{

}

AXPopupWindow::AXPopupWindow(AXWindow *pOwner,UINT uStyle)
    : AXTopWindow(pOwner, uStyle | WS_HIDE | WS_TRANSIENT_FOR | WS_DISABLE_WM)
{
    m_bGrab = FALSE;
}

//! 実行
/*!
    ※表示・移動は行うが、サイズ変更・レイアウトは行わない。
*/

void AXPopupWindow::runPopup(int rootx,int rooty)
{
    moveInRoot(rootx, rooty);
    show();

    axapp->runPopup(this);
}

//! 終了
/*!
    @param bCancel ウィンドウ外でクリックされたり、ESCキーが押された場合、TRUE
*/

void AXPopupWindow::endPopup(BOOL bCancel)
{
    grabPopup(FALSE);

    axapp->exit();
}

//! マウスグラブ

void AXPopupWindow::grabPopup(BOOL bOn)
{
    if(bOn)
    {
        if(!m_bGrab)
            grabPointer();
    }
    else
    {
        if(m_bGrab)
            ungrabPointer();
    }

    m_bGrab = bOn;
}


//===========================
//ハンドラ
//===========================


//! マップされた

BOOL AXPopupWindow::onMap()
{
    //カーソルがウィンドウ外ならグラブ
    //※runPopup() 時に判定しても表示状態が確実ではない（グラブが失敗する）ので、ここで行う

    if(!isCursorIn())
        grabPopup(TRUE);

    return TRUE;
}

//! 範囲内に入った

BOOL AXPopupWindow::onEnter(AXHD_ENTERLEAVE *phd)
{
    if(isContain(phd->x, phd->y))
        grabPopup(FALSE);

    return TRUE;
}

//! 範囲外に出た

BOOL AXPopupWindow::onLeave(AXHD_ENTERLEAVE *phd)
{
    if(!isContain(phd->x, phd->y))
        grabPopup(TRUE);

    return TRUE;
}

//! グラブが解除された

BOOL AXPopupWindow::onUngrab(AXHD_ENTERLEAVE *phd)
{
    /* 内部ウィジェットでのグラブが解除された時に、
       カーソルがウィンドウ外に出ていた場合グラブさせる */

    if(!isContain(phd->x, phd->y))
        grabPopup(TRUE);

    return TRUE;
}

//! マウス移動（ウィンドウ内外を判定しグラブ）

BOOL AXPopupWindow::onMouseMove(AXHD_MOUSE *phd)
{
    BOOL bNoIn = !isContain(phd->x, phd->y);

    if(bNoIn != m_bGrab)
        grabPopup(bNoIn);

    return TRUE;
}

//! ボタン押し時（ウィンドウ外でクリックされた時は終了）

BOOL AXPopupWindow::onButtonDown(AXHD_MOUSE *phd)
{
    if(!isContain(phd->x, phd->y))
    {
        endPopup(TRUE);
        return TRUE;
    }

    return FALSE;
}

//! キー押し時（ESCキーで終了）

BOOL AXPopupWindow::onKeyDown(AXHD_KEY *phd)
{
    if(phd->keysym == KEY_ESCAPE)
    {
        endPopup(TRUE);
        return TRUE;
    }

    return AXTopWindow::onKeyDown(phd);
}
