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

#include "AXX11.h"

#include "AXAccelerator.h"

#include "AXWindow.h"
#include "AXApp.h"


//********************************
//リストデータクラス
//********************************


//! アクセラレータの各アイテム

class AXAccelItem:public AXListItem
{
public:
    UINT        m_uCmdID,
                m_uKey,
                m_uCode;    //!< 上位16bitは装飾キーフラグ
    AXWindow    *m_pwin;

    AXAccelItem(UINT uCmdID,UINT key,AXWindow *pwin);
    AXAccelItem *next() const { return (AXAccelItem *)m_pNext; }
};

//! 作成

AXAccelItem::AXAccelItem(UINT uCmdID,UINT key,AXWindow *pwin)
{
    m_uCmdID = uCmdID;
    m_uKey   = key;
    m_pwin   = pwin;

    //キーコード

    m_uCode = ::XKeysymToKeycode(axdisp(), key & 0xffffff);

    //装飾キー

    if(m_uKey & ACKEY_SHIFT) m_uCode |= AXWindow::STATE_SHIFT << 16;
    if(m_uKey & ACKEY_CTRL)  m_uCode |= AXWindow::STATE_CTRL << 16;
    if(m_uKey & ACKEY_ALT)   m_uCode |= AXWindow::STATE_ALT << 16;
}


//********************************
//AXAccelerator
//********************************

/*!
    @class AXAccelerator
    @brief アクセラレータクラス

    - Ctrl+S など、指定されたキーが押された時にウィンドウの onCommand() が実行される。
    - イベントを送るウィンドウは setDefaultWindow() で指定したウィンドウか、キー毎にウィンドウを指定できる。
    - アクセラレータを処理するトップレベルウィンドウそれぞれで attachAccel() でアクセラレータを関連付ける必要がある。@n
      もしくは、トップレベルの onKeyDown/onKeyUp 内で直接 AXAccelerator::OnKey() を処理する。
    - キーは KEY_** (AXKey.h)。ASCII文字の場合はそのまま文字で指定。装飾キーは ACKEY_** を OR で指定。

    @ingroup etc
*/


AXAccelerator::AXAccelerator()
{
    m_pwinDef = NULL;
}

//! 追加

void AXAccelerator::add(UINT uCmdID,UINT key)
{
    m_dat.add(new AXAccelItem(uCmdID, key, NULL));
}

//! 追加（ウィンドウ指定）

void AXAccelerator::add(UINT uCmdID,UINT key,AXWindow *pwin)
{
    m_dat.add(new AXAccelItem(uCmdID, key, pwin));
}

//! キーイベント時の処理
/*!
    @param bUp FALSEでキー押し時。TRUEでキー離し時
    @return	TRUEで処理された。FALSEで処理されなかった
*/

BOOL AXAccelerator::onKey(const AXHD_KEY *phd,BOOL bUp)
{
    AXAccelItem *p;
    UINT code;
    AXWindow *pwin;

    code = phd->code;
    code |= (phd->state & AXWindow::STATE_MODMASK) << 16;

    //検索

    for(p = (AXAccelItem *)m_dat.getTop(); p; p = p->next())
    {
        if(p->m_uCode == code) break;
    }

    if(!p) return FALSE;

    //

    if(!bUp)
    {
        //実行（離し時は行わない）
        //※ウィンドウがNULLの場合はデフォルトのウィンドウ

        pwin = p->m_pwin;

        if(!pwin)
        {
            pwin = m_pwinDef;
            if(!pwin) return FALSE;
        }

        pwin->sendCommand(p->m_uCmdID, 0, AXWindow::COMFROM_ACCEL);
    }

    return TRUE;
}
