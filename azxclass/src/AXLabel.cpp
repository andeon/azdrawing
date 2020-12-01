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

#include "AXLabel.h"

#include "AXApp.h"
#include "AXAppRes.h"

//----------------------

#define SPACE_X  3
#define SPACE_Y  2

/*!
    @class AXLabel
    @brief ラベルウィジェット

    - 改行（\\n）有効。
    - 折り返しには対応していない。

    @ingroup widget
*/
/*!
    @var AXLabel::LS_BORDER
    @brief FRAMEDARK 色で通常枠
    @var AXLabel::LS_SUNKEN
    @brief くぼみ枠
    @var AXLabel::LS_BKRIGHT
    @brief 背景色を FACELIGHT に
*/


AXLabel::~AXLabel()
{

}

AXLabel::AXLabel(AXWindow *pParent,UINT uStyle,UINT uLayoutFlags)
    : AXWindow(pParent, uStyle, uLayoutFlags)
{
    _createLabel();
}

AXLabel::AXLabel(AXWindow *pParent,UINT uStyle,UINT uLayoutFlags,DWORD dwPadding,LPCUSTR pstr)
    : AXWindow(pParent, uStyle, uLayoutFlags, 0, dwPadding)
{
    _createLabel();

    m_strText = pstr;
}

AXLabel::AXLabel(AXWindow *pParent,UINT uStyle,UINT uLayoutFlags,DWORD dwPadding,LPCSTR szText)
    : AXWindow(pParent, uStyle, uLayoutFlags, 0, dwPadding)
{
    _createLabel();

    m_strText = szText;
}

AXLabel::AXLabel(AXWindow *pParent,UINT uStyle,UINT uLayoutFlags,DWORD dwPadding,WORD wStrID)
    : AXWindow(pParent, uStyle, uLayoutFlags, 0, dwPadding)
{
    _createLabel();

    m_strText = _str(wStrID);
}

//! 作成処理

void AXLabel::_createLabel()
{
    m_uType = TYPE_LABEL;
}

//! 標準サイズ計算

void AXLabel::calcDefSize()
{
    _getTextSize(&m_sizeText);

    m_nDefW = m_sizeText.w;
    m_nDefH = m_sizeText.h;

    if(m_uStyle & (LS_BORDER | LS_SUNKEN))
    {
        m_nDefW += SPACE_X * 2;
        m_nDefH += SPACE_Y * 2;
    }
}

//! テキストセット

void AXLabel::setText(const AXString &str)
{
    m_strText = str;

    if(isLayouted())
    {
        calcDefSize();
        redraw();
    }
}

//! テキストセット

void AXLabel::setText(LPCUSTR pText)
{
    m_strText = pText;

    if(isLayouted())
    {
        calcDefSize();
        redraw();
    }
}

//! テキストセット(ASCII)

void AXLabel::setText(LPCSTR pText)
{
    m_strText = pText;

    if(isLayouted())
    {
        calcDefSize();
        redraw();
    }
}

//! 文字長さから最小幅セット

void AXLabel::setWidthFromLen(int len)
{
    m_nMinW = m_pFont->getTextWidth("9", 1) * len;

    if(m_uStyle & (LS_BORDER | LS_SUNKEN))
        m_nMinW += SPACE_X * 2;
}


//======================


//! 文字列のサイズ取得

void AXLabel::_getTextSize(AXSize *psize)
{
    int maxw = 0,maxh = 0;
    int pos,len,ret,oneh,w;

    oneh = m_pFont->getHeight();

    for(ret = 0, pos = 0; ret != -1; )
    {
        ret = m_strText.find('\n', pos);

        len = ((ret == -1)? m_strText.getLen(): ret) - pos;

        if(len == 0)
        {
            //空白行（改行で終わっている場合は無視）
            if(ret != -1)
                maxh += oneh;
        }
        else
        {
            //テキストあり

            w = m_pFont->getTextWidth(m_strText.at(pos), len);

            if(w > maxw) maxw = w;
            maxh += oneh;
        }

        pos = ret + 1;
    }

    psize->w = maxw;
    psize->h = (maxh == 0)? oneh: maxh;
}

//! 描画

BOOL AXLabel::onPaint(AXHD_PAINT *phd)
{
    int x,y,ret,pos,len,oneh,spacex,spacey,w;

    //余白

    if(m_uStyle & (LS_BORDER | LS_SUNKEN))
        spacex = SPACE_X, spacey = SPACE_Y;
    else
        spacex = spacey = 0;

    //Yテキスト位置

    if(m_uStyle & LS_BOTTOM)
        y = m_nH - 1 - spacey - m_sizeText.h;
    else if(m_uStyle & LS_VCENTER)
        y = spacey + (m_nH - spacey * 2 - m_sizeText.h) / 2;
    else
        y = spacey;

    if(y < spacey) y = spacey;

    //背景

    drawFillBox(0, 0, m_nW, m_nH, (m_uStyle & LS_BKLIGHT)? AXAppRes::FACELIGHT: AXAppRes::FACE);

    //テキスト

    AXDrawText dt(m_id);

    oneh = m_pFont->getHeight();

    for(ret = 0, pos = 0; ret != -1; y += oneh)
    {
        ret = m_strText.find('\n', pos);
        len = ((ret == -1)? m_strText.getLen(): ret) - pos;

        //描画

        if(len)
        {
            if(m_uStyle & LS_RIGHT)
            {
                w = m_pFont->getTextWidth(m_strText.at(pos), len);
                x = m_nW - 1 - spacex - w;
            }
            else if(m_uStyle & LS_CENTER)
            {
                w = m_pFont->getTextWidth(m_strText.at(pos), len);
                x = spacex + (m_nW - spacex * 2 - w) / 2;
            }
            else
                x = spacex;

            dt.draw(*m_pFont, x, y, m_strText.at(pos), len);
        }

        pos = ret + 1;
    }

    dt.end();

    //枠

    if(m_uStyle & LS_BORDER)
        drawBox(0, 0, m_nW, m_nH, AXAppRes::FRAMEDARK);
    else if(m_uStyle & LS_SUNKEN)
        drawFrameSunken(0, 0, m_nW, m_nH);

    return TRUE;
}

