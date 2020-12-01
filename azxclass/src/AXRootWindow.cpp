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

#include "AXRootWindow.h"

/*!
    @class AXRootWindow
    @brief ルートウィンドウ

    - AXApp で作成される。
    - AXLayoutItem から継承している m_pad* のメンバには、ルートウィンドウの作業領域範囲が入っている。

    @ingroup window
*/

//---------------------


//! デストラクタ

AXRootWindow::~AXRootWindow()
{
    //全ての子ウィンドウ削除

    while(m_pFirst)
        delete m_pFirst;

    //AXWindow::~AXWindow() の処理を実行させない

    m_id = 0;
}

//! コンストラクタ
/*!
    初期化は AXWindow::AXWindow() で行われる。
*/

AXRootWindow::AXRootWindow()
	: AXWindow()
{

}
