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

#include "AXButton.h"

#include "AXAppRes.h"
#include "AXTopWindow.h"
#include "AXKey.h"
#include "AXApp.h"
#include "AXTrSysID.h"


#define SPACE_X  5
#define SPACE_Y  4


//-------------------------

/*!
    @class AXButton
    @brief ボタンウィジェット

    - デフォルトで、幅・高さは適当な大きさに調節される。
    - 幅・高さを調節させたくない場合は、BS_REAL_W/BS_REAL_H を指定する。
    - デフォルトボタンに指定する場合は、AXTopWindow::setDefaultButton() を使う。
    - ボタンが押された場合、通知先ウィンドウに onNotify() で通知される。
    - 派生クラスのボタン自身側で押し時の処理を行いたい場合は、onPress() を継承する。

    @ingroup widget
*/
/*!
    @enum AXButton::BUTTONSTYLE
    @brief ボタンスタイル

    @var AXButton::BS_REAL_W
    @brief 幅を実際のサイズにする
    @var AXButton::BS_REAL_H
    @brief 高さを実際のサイズにする
    @var AXButton::BS_REAL_WH
    @brief 幅と高さを実際のサイズにする
    @var AXButton::BS_NOOUTFRAME
    @brief 一番外側の枠を描画しない
*/


AXButton::~AXButton()
{
    //デフォルトボタンの場合

    if(m_uFlags & FLAG_DEFAULTBUTTON)
        ((AXTopWindow *)m_pTopLevel)->_setDefButtonNone();
}

AXButton::AXButton(AXWindow *pParent,UINT uStyle,UINT uLayoutFlags)
    : AXWindow(pParent, uStyle, uLayoutFlags)
{
    _createButton();
}

AXButton::AXButton(AXWindow *pParent,UINT uStyle,UINT uLayoutFlags,UINT uItemID,DWORD dwPadding)
    : AXWindow(pParent, uStyle, uLayoutFlags, uItemID, dwPadding)
{
    _createButton();
}

AXButton::AXButton(AXWindow *pParent,UINT uStyle,UINT uLayoutFlags,UINT uItemID,DWORD dwPadding,LPCUSTR pstr)
    : AXWindow(pParent, uStyle, uLayoutFlags, uItemID, dwPadding)
{
    _createButton();

    m_strText = pstr;
}

AXButton::AXButton(AXWindow *pParent,UINT uStyle,UINT uLayoutFlags,UINT uItemID,DWORD dwPadding,LPCSTR pText)
    : AXWindow(pParent, uStyle, uLayoutFlags, uItemID, dwPadding)
{
    _createButton();

    m_strText = pText;
}

//! 翻訳文字列から作成

AXButton::AXButton(AXWindow *pParent,UINT uStyle,UINT uLayoutFlags,UINT uItemID,DWORD dwPadding,WORD wStrID)
    : AXWindow(pParent, uStyle, uLayoutFlags, uItemID, dwPadding)
{
    _createButton();

    m_strText = _str(wStrID);
}

//! 指定グループの翻訳文字列から作成

AXButton::AXButton(AXWindow *pParent,UINT uStyle,UINT uLayoutFlags,UINT uItemID,DWORD dwPadding,
                   WORD wGroupID,WORD wStrID)
    : AXWindow(pParent, uStyle, uLayoutFlags, uItemID, dwPadding)
{
    _createButton();

    m_strText = _string(wGroupID, wStrID);
}

//! 作成処理

void AXButton::_createButton()
{
    m_uType     = TYPE_BUTTON;
    m_uFlags    |= FLAG_TAKE_FOCUS;
}

//! 標準サイズ計算

void AXButton::calcDefSize()
{
    m_pFont->getTextSize(m_strText, &m_sizeText);

    //幅

    m_nDefW = m_sizeText.w + SPACE_X * 2;
    if(!(m_uStyle & BS_REAL_W) && m_nDefW < 64) m_nDefW = 64;

    //高さ

    if(m_uStyle & BS_REAL_H)
        m_nDefH = m_sizeText.h + 4;
    else
    {
        m_nDefH = m_sizeText.h + SPACE_Y * 2;
        if(m_nDefH < 22) m_nDefH = 22;
    }
}

//! Enterキーを処理する

BOOL AXButton::isAcceptKey(UINT keytype)
{
    return (keytype == ACCEPTKEYTYPE_ENTER);
}


//! テキストセット

void AXButton::setText(const AXString &str)
{
    m_strText = str;

    if(isLayouted())
    {
        calcDefSize();
        redraw();
    }
}

void AXButton::setText(LPCUSTR pText)
{
    m_strText = pText;

    if(isLayouted())
    {
        calcDefSize();
        redraw();
    }
}

//! 押し状態にする

void AXButton::press()
{
    m_uFlags |= FLAG_PRESS;
    redraw();
}


//============================
//ハンドラ
//============================


//! ボタンが押された（親に通知）

BOOL AXButton::onPress()
{
    getNotify()->sendNotify(this, BN_PRESS, 0);

    return TRUE;
}

//! ボタン押し時

BOOL AXButton::onButtonDown(AXHD_MOUSE *phd)
{
    if(!(m_uFlags & (FLAG_DOWN_MOUSE | FLAG_DOWN_KEY)) && phd->button == BUTTON_LEFT)
    {
        setFocus();

        m_uFlags |= FLAG_DOWN_MOUSE;

        redrawUpdate();
        grabPointer();
    }

    return TRUE;
}

//! ボタン離し時

BOOL AXButton::onButtonUp(AXHD_MOUSE *phd)
{
    if((m_uFlags & FLAG_DOWN_MOUSE) && phd->button == BUTTON_LEFT)
    {
        m_uFlags &= ~FLAG_DOWN_MOUSE;

        redrawUpdate();
        ungrabPointer();

        onPress();
    }

    return TRUE;
}

//! キー押し時

BOOL AXButton::onKeyDown(AXHD_KEY *phd)
{
    if(!(m_uFlags & (FLAG_DOWN_MOUSE | FLAG_DOWN_KEY)))
    {
        if(ISKEY_SPACE(phd->keysym) || ISKEY_ENTER(phd->keysym))
        {
            m_uFlags |= FLAG_DOWN_KEY;
            redrawUpdate();

            return TRUE;
        }
    }

    return FALSE;
}

//! キー離し時

BOOL AXButton::onKeyUp(AXHD_KEY *phd)
{
    if(m_uFlags & FLAG_DOWN_KEY)
    {
        if(ISKEY_SPACE(phd->keysym) || ISKEY_ENTER(phd->keysym))
        {
            m_uFlags &= ~FLAG_DOWN_KEY;
            redrawUpdate();

            onPress();

            return TRUE;
        }
    }

    return FALSE;
}

//! 描画

BOOL AXButton::onPaint(AXHD_PAINT *phd)
{
    int x,y;
    UINT flag = 0;

    //テキスト位置

    x = (m_nW - m_sizeText.w) / 2;
    y = (m_nH - m_sizeText.h) / 2;

    if(isPress())
        x++, y++;

    //ボタン

    if(isPress()) flag |= DRAWBUTTON_DOWN;
    if(m_uFlags & FLAG_FOCUSED) flag |= DRAWBUTTON_FOCUSED;
    if(m_uFlags & FLAG_DEFAULTBUTTON) flag |= DRAWBUTTON_DEFAULT;
    if(!(m_uFlags & FLAG_ENABLED)) { flag |= DRAWBUTTON_DISABLE; flag &= ~DRAWBUTTON_FOCUSED; }
    if(m_uStyle & BS_NOOUTFRAME) flag |= DRAWBUTTON_NOOUTFRAME;

    drawButton(0, 0, m_nW, m_nH, flag);

    //テキスト

    AXDrawText dt(m_id);

    dt.draw(*m_pFont, x, y, m_strText,
        (m_uFlags & FLAG_ENABLED)? AXAppRes::TC_NORMAL: AXAppRes::TC_DISABLE);

    dt.end();

    return TRUE;
}
