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

#include "AXCheckButton.h"

#include "AXAppRes.h"
#include "AXApp.h"
#include "AXKey.h"


#define BUTTON_SPACEX   5       //ボタンタイプの余白
#define BUTTON_SPACEY   4
#define CHECK_BOXSIZE   13
#define CHECK_SPACE     5       //チェックボックスとテキストの間隔
#define CHECK_PADDINGX  3
#define CHECK_PADDINGY  2

//--------------------------------

/*!
    @class AXCheckButton
    @brief チェックボタンウィジェット

    <h2>[Notify通知]</h2>
    - <b>CBN_TOGGLE</b> @n
        チェックが変更された。@n
        lParam : チェックされていれば 1、チェックされていなければ 0。

    @ingroup widget
*/

/*!
    @var AXCheckButton::CBS_BUTTON
    @brief ボタンタイプ
    @var AXCheckButton::CBS_RADIO
    @brief ラジオタイプ
    @var AXCheckButton::CBS_GROUP
    @brief グループの開始
    ラジオタイプの場合、グループの先頭にこのフラグを付ける。@n
    ラジオグループが続けて複数続く場合に必要。@n
    ラジオグループの間に他のアイテムがある場合は必要ない。

    @var AXCheckButton::CBS_REALSIZE
    @brief ボタンタイプの場合、幅・高さを実際のテキストのサイズにする
    @var AXCheckButton::CBS_CHECK
    @brief チェックがONの状態
    @var AXCheckButton::CBS_BKLIGHT
    @brief 背景色を FACELIGHT に
*/


AXCheckButton::~AXCheckButton()
{

}

//! 作成

AXCheckButton::AXCheckButton(AXWindow *pParent,UINT uStyle,UINT uLayoutFlags)
    : AXWindow(pParent, uStyle, uLayoutFlags)
{
    _createCheckButton(NULL);
}

AXCheckButton::AXCheckButton(AXWindow *pParent,UINT uStyle,UINT uLayoutFlags,UINT uItemID,DWORD dwPadding,LPCUSTR pstr)
    : AXWindow(pParent, uStyle, uLayoutFlags, uItemID, dwPadding)
{
    _createCheckButton(NULL);

    m_strText = pstr;
}

AXCheckButton::AXCheckButton(AXWindow *pParent,UINT uStyle,UINT uLayoutFlags,UINT uItemID,DWORD dwPadding,LPCUSTR pstr,BOOL bCheck)
    : AXWindow(pParent, uStyle, uLayoutFlags, uItemID, dwPadding)
{
    _createCheckButton(&bCheck);

    m_strText = pstr;
}

AXCheckButton::AXCheckButton(AXWindow *pParent,UINT uStyle,UINT uLayoutFlags,UINT uItemID,DWORD dwPadding,LPCSTR pText,BOOL bCheck)
    : AXWindow(pParent, uStyle, uLayoutFlags, uItemID, dwPadding)
{
    _createCheckButton(&bCheck);

    m_strText = pText;
}

AXCheckButton::AXCheckButton(AXWindow *pParent,UINT uStyle,UINT uLayoutFlags,UINT uItemID,DWORD dwPadding,WORD wStrID,BOOL bCheck)
    : AXWindow(pParent, uStyle, uLayoutFlags, uItemID, dwPadding)
{
    _createCheckButton(&bCheck);

    m_strText = _str(wStrID);
}


//! 作成処理
//! @param pCheck チェック状態。NULLで変更なし

void AXCheckButton::_createCheckButton(LPINT pCheck)
{
    m_uType     = TYPE_CHECKBUTTON;
    m_uFlags    |= FLAG_TAKE_FOCUS;

    if(pCheck)
    {
        if(*pCheck)
            m_uStyle |= CBS_CHECK;
        else
            m_uStyle &= ~CBS_CHECK;
    }
}

//! 標準サイズ計算

void AXCheckButton::calcDefSize()
{
    m_pFont->getTextSize(m_strText, &m_sizeText);

    m_nDefW = m_sizeText.w;
    m_nDefH = m_sizeText.h;

    if(m_uStyle & CBS_BUTTON)
    {
        //ボタンタイプ

        m_nDefW += BUTTON_SPACEX * 2;

        if(m_uStyle & CBS_REALSIZE)
            m_nDefH += 4;
        else
        {
            //通常

            m_nDefH += BUTTON_SPACEY * 2;

            if(m_nDefW < 60) m_nDefW = 60;
            if(m_nDefH < 18) m_nDefH = 18;
        }
    }
    else
    {
        //チェックボックスタイプ
        //※テキストがない場合、ボックスのみ

        m_nDefW += CHECK_BOXSIZE + CHECK_PADDINGX * 2;
        if(m_sizeText.w) m_nDefW += CHECK_SPACE;

        if(m_nDefH < CHECK_BOXSIZE) m_nDefH = CHECK_BOXSIZE;
        m_nDefH += CHECK_PADDINGY * 2;
    }
}


//===========================
//
//===========================


//! テキストセット

void AXCheckButton::setText(const AXString &str)
{
    m_strText = str;

    if(isLayouted())
    {
        calcDefSize();
        redraw();
    }
}

//! チェックセット

void AXCheckButton::setCheck(BOOL bCheck)
{
    if(!bCheck && isChecked())
    {
        //チェックをOFFにする
        m_uStyle ^= CBS_CHECK;
        redraw();
    }
    else if((!bCheck) != !(m_uStyle & CBS_CHECK))
        _check(FALSE);
}

//! チェックされているか

BOOL AXCheckButton::isChecked()
{
    return ((m_uStyle & CBS_CHECK) != 0);
}

//! ボタンタイプの場合に、押されている状態かどうか

BOOL AXCheckButton::isButtonDown()
{
    return ((m_uStyle & CBS_CHECK) || (m_uFlags & (FLAG_DOWN_MOUSE | FLAG_DOWN_KEY)));
}

//! ラジオグループ中で現在選択されているものをインデックス番号で取得
/*!
    @return 現在選択されているもののインデックス番号（0〜）。-1で選択なし。
*/

int AXCheckButton::getGroupSel()
{
    AXCheckButton *ptop,*pend,*psel;
    AXLayoutItem *p;
    int i;

    if(m_uStyle & CBS_RADIO)
    {
        psel = _getRadioGroup(&ptop, &pend);

        if(!psel)
            return -1;
        else
        {
            //インデックス番号
            for(i = 0, p = ptop; p != psel; i++, p = p->getLayoutNext());

            return i;
        }
    }

    return -1;
}


//============================
//内部処理
//============================


//! チェック処理

void AXCheckButton::_check(BOOL bPress)
{
    //ラジオの場合、現在選択されているものをOFF
    //（自身の場合は変更なし）

    if(m_uStyle & CBS_RADIO)
    {
        AXCheckButton *psel;

        psel = _getRadioGroup(NULL, NULL);
        if(psel)
        {
            if(psel == this) return;

            psel->m_uStyle ^= CBS_CHECK;
            psel->redraw();
        }
    }

    //チェック変更

    m_uStyle ^= CBS_CHECK;
    redraw();

    //

    if(bPress) onPress();
}

//! ラジオグループの範囲と現在の選択取得
/*!
    ※CBS_GROUP フラグが ON で、グループの先頭。

    @param  pTop,pEnd 先頭と終端が返る。NULLで値を受け取らない。
    @return 現在選択されている状態のもの
*/

AXCheckButton *AXCheckButton::_getRadioGroup(AXCheckButton **pTop,AXCheckButton **pEnd)
{
    AXLayoutItem *p,*pSel = NULL;
    UINT style;

    //前を検索（自身も含める）

    for(p = this; p; p = p->getLayoutPrev())
    {
        if(p->getType() != TYPE_CHECKBUTTON) { p = p->getLayoutNext(); break; }

        style = ((AXWindow *)p)->getStyle();

        if(!(style & CBS_RADIO)) { p = p->getLayoutNext(); break; }
        if(style & CBS_CHECK) pSel = p;
        if(style & CBS_GROUP) break;
    }

    if(pTop)
        *pTop = (p)? (AXCheckButton *)p: this;

    //後ろを検索（自身の次から検索）

    for(p = m_pLNext; p; p = p->getLayoutNext())
    {
        if(p->getType() != TYPE_CHECKBUTTON) { p = p->getLayoutPrev(); break; }

        style = ((AXWindow *)p)->getStyle();

        if(!(style & CBS_RADIO) || (style & CBS_GROUP)) { p = p->getLayoutPrev(); break; }
        if(style & CBS_CHECK) pSel = p;
    }

    if(pEnd)
        *pEnd = (p)? (AXCheckButton *)p: this;

    return (AXCheckButton *)pSel;
}


//============================
//ハンドラ
//============================


//! ボタンが押された（親に通知）

BOOL AXCheckButton::onPress()
{
    getNotify()->sendNotify(this, CBN_TOGGLE, ((m_uStyle & CBS_CHECK) != 0));

    return TRUE;
}

//! ボタン押し時

BOOL AXCheckButton::onButtonDown(AXHD_MOUSE *phd)
{
    if(!(m_uFlags & (FLAG_DOWN_MOUSE | FLAG_DOWN_KEY)) && phd->button == BUTTON_LEFT)
    {
        setFocus();

        m_uFlags |= FLAG_DOWN_MOUSE;

        if(!(m_uStyle & CBS_BUTTON))
            _check(TRUE);

        redrawUpdate();
        grabPointer();
    }

    return TRUE;
}

//! ボタン離し時

BOOL AXCheckButton::onButtonUp(AXHD_MOUSE *phd)
{
    if((m_uFlags & FLAG_DOWN_MOUSE) && phd->button == BUTTON_LEFT)
    {
        m_uFlags &= ~FLAG_DOWN_MOUSE;

        ungrabPointer();

        if(m_uStyle & CBS_BUTTON)
        {
            _check(TRUE);
            redrawUpdate();
        }
    }

    return TRUE;
}

//! キー押し時

BOOL AXCheckButton::onKeyDown(AXHD_KEY *phd)
{
    if(!(m_uFlags & (FLAG_DOWN_MOUSE | FLAG_DOWN_KEY)) && ISKEY_SPACE(phd->keysym))
    {
        m_uFlags |= FLAG_DOWN_KEY;

        if(!(m_uStyle & CBS_BUTTON))
            _check(TRUE);

        redrawUpdate();

        return TRUE;
    }

    return FALSE;
}

//! キー離し時

BOOL AXCheckButton::onKeyUp(AXHD_KEY *phd)
{
    if((m_uFlags & FLAG_DOWN_KEY) || ISKEY_SPACE(phd->keysym))
    {
        m_uFlags &= ~FLAG_DOWN_KEY;

        if(m_uStyle & CBS_BUTTON)
        {
            _check(TRUE);
            redrawUpdate();
        }

        return TRUE;
    }

    return FALSE;
}

//! 描画

BOOL AXCheckButton::onPaint(AXHD_PAINT *phd)
{
    if(m_uStyle & CBS_BUTTON)
        _drawCheckButton();
    else
        _drawCheckBox();

    return TRUE;
}


//====================================
//描画
//====================================


//! チェックボックスタイプ描画

void AXCheckButton::_drawCheckBox()
{
    int n,by,bEnable;

    bEnable = m_uFlags & FLAG_ENABLED;

    //背景

    if(m_uFlags & FLAG_FOCUSED)
        n = AXAppRes::FACEFOCUS;
    else
        n = (m_uStyle & CBS_BKLIGHT)? AXAppRes::FACELIGHT: AXAppRes::FACE;

    drawFillBox(0, 0, m_nW, m_nH, n);

    //チェックボックス

    if(m_uStyle & CBS_RADIO)
    {
        //ラジオ

        by = (m_nH - 11) / 2;

        drawFillEllipse(CHECK_PADDINGX + 1, by, 11, 11, (bEnable)? AXAppRes::FACEDOC: AXAppRes::FACE);
        drawEllipse(CHECK_PADDINGX + 1, by, 11, 11, AXAppRes::FRAMEOUTSIDE);

        if(m_uStyle & CBS_CHECK)
            drawRadioCheck(CHECK_PADDINGX + 5, by + 4, (bEnable)? AXAppRes::TEXTNORMAL: AXAppRes::TEXTDISABLE);
    }
    else
    {
        //チェック

        by = (m_nH - CHECK_BOXSIZE) / 2;

        drawBox(CHECK_PADDINGX, by, CHECK_BOXSIZE, CHECK_BOXSIZE, AXAppRes::FRAMEOUTSIDE);

        drawFillBox(CHECK_PADDINGX + 1, by + 1, CHECK_BOXSIZE - 2, CHECK_BOXSIZE - 2,
                    (bEnable)? AXAppRes::FACEDOC: AXAppRes::FACE);

        if(m_uStyle & CBS_CHECK)
            drawCheck(CHECK_PADDINGX + 3, by + 3, (bEnable)? AXAppRes::TEXTNORMAL: AXAppRes::TEXTDISABLE);
    }

    //テキスト

    AXDrawText dt(m_id);

    dt.draw(*m_pFont,
        CHECK_PADDINGX + CHECK_BOXSIZE + CHECK_SPACE, (m_nH - m_sizeText.h) / 2,
        m_strText, (bEnable)? AXAppRes::TC_NORMAL: AXAppRes::TC_DISABLE);

    dt.end();
}

//! ボタンタイプ描画

void AXCheckButton::_drawCheckButton()
{
    int x,y;
    UINT flag = 0;
    BOOL bDown;

    bDown = isButtonDown();

    //テキスト位置

    x = (m_nW - m_sizeText.w) / 2;
    y = (m_nH - m_sizeText.h) / 2;

    if(bDown) x++, y++;

    //ボタン

    if(bDown) flag |= DRAWBUTTON_DOWN;
    if(m_uFlags & FLAG_FOCUSED) flag |= DRAWBUTTON_FOCUSED;
    if(!(m_uFlags & FLAG_ENABLED)) flag |= DRAWBUTTON_DISABLE;

    drawButton(0, 0, m_nW, m_nH, flag);

    //テキスト

    AXDrawText dt(m_id);

    dt.draw(*m_pFont, x, y, m_strText,
        (m_uFlags & FLAG_ENABLED)? AXAppRes::TC_NORMAL: AXAppRes::TC_DISABLE);

    dt.end();
}
