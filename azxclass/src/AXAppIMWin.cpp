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

#include "AXAppIMWin.h"

#include "AXFont.h"
#include "AXApp.h"
#include "AXAppRes.h"


/*!
    @class AXAppIMWin
    @brief 入力メソッドの前編集時の文字列表示ウィンドウ

    - AXAppIC で使われる。

    @ingroup core
*/


//! 作成

AXAppIMWin::AXAppIMWin(int x,int y)
    : AXWindow(NULL, WS_HIDE | WS_DISABLE_WM)
{
    selectInput(ExposureMask);

    move(x, y);
    show();
    above(TRUE);
}

//! コールバック描画時

void AXAppIMWin::PE_draw(LPVOID pParam)
{
    XIMPreeditDrawCallbackStruct *pds = (XIMPreeditDrawCallbackStruct *)pParam;

    //テキストセット

    if(pds->text->encoding_is_wchar)
        m_strText.setWide(pds->text->string.wide_char, pds->text->length);
    else
        m_strText.setLocal(pds->text->string.multi_byte);

    //カーソル位置

    m_nCurX = m_pFont->getTextWidth(m_strText, pds->caret);

    //反転文字範囲

    m_nRevTop = -1;

    if(pds->text->feedback)
    {
        XIMFeedback *pFB = pds->text->feedback;
        ULONG bk = pFB[0];
        UINT pos;

        if(bk & XIMReverse) m_nRevTop = 0;

        for(pos = 0; pos < pds->text->length; pos++, pFB++)
        {
            if((bk & XIMReverse) != (*pFB & XIMReverse))
            {
                bk = *pFB;

                if(bk & XIMReverse)
                    m_nRevTop = pos;
                else
                {
                    m_nRevEnd = pos;
                    break;
                }
            }
        }

        //最後まで反転の場合

        if(bk & XIMReverse)
            m_nRevEnd = m_strText.getLen();
    }

    //位置・サイズ

    int x,y,w,h;

    x = m_nX, y = m_nY;
    w = m_pFont->getTextWidth(m_strText) + 1;
    h = m_pFont->getHeight() + 1;

    if(x + w > axapp->getRootWindow()->getWidth())
        x = axapp->getRootWindow()->getWidth() - w;

    if(y + h > axapp->getRootWindow()->getHeight())
        y = axapp->getRootWindow()->getHeight() - h;

    if(x < 0) x = 0;
    if(y < 0) y = 0;

    //

    moveresize(x, y, w, h);
    redrawUpdate();
}

//! 描画

BOOL AXAppIMWin::onPaint(AXHD_PAINT *phd)
{
    LPUSTR p;
    int pos,x,w;

    //背景

    drawFillBox(0, 0, m_nW, m_nH, AXAppRes::WHITE);

    if(m_strText.isEmpty()) return TRUE;

    //テキスト

    AXDrawText dt(m_id);

    for(p = m_strText, pos = 0, x = 0; *p; p++, pos++, x += w)
    {
        w = m_pFont->getTextWidth(p, 1);

        if(m_nRevTop != -1 && m_nRevTop <= pos && pos < m_nRevEnd)
        {
            drawFillBox(x, 0, w, m_pFont->getHeight(), AXAppRes::BLACK);
            dt.draw(*m_pFont, x, 0, p, 1, AXAppRes::TC_WHITE);
        }
        else
            dt.draw(*m_pFont, x, 0, p, 1, AXAppRes::TC_BLACK);
    }

    dt.end();

    //下線

    drawLine(0, m_nH - 1, m_nW - 2, m_nH - 1, AXAppRes::BLACK);

    //カーソル

    if(m_nRevTop == -1)
        drawLine(m_nCurX, 0, m_nCurX, m_pFont->getHeight() - 2, axres->gcXor());

    return TRUE;
}
