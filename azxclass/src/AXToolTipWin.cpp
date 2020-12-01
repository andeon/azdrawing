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

#include "AXToolTipWin.h"

#include "AXFont.h"
#include "AXAppRes.h"


#define SPACEX  4
#define SPACEY  3


/*!
    @class AXToolTipWin
    @brief ツールチップのウィンドウ

    - テキストは改行（\\n）有効。

    @ingroup window
*/


AXToolTipWin::~AXToolTipWin()
{

}

AXToolTipWin::AXToolTipWin(UINT uStyle)
    : AXWindow(NULL, uStyle | WS_HIDE | WS_DISABLE_WM)
{
    selectInputExposure();
}

//! 表示

BOOL AXToolTipWin::showTip(int rootx,int rooty,LPCUSTR pText)
{
    AXSize size;

    m_strText = pText;
    if(m_strText.isEmpty()) return FALSE;

    _getTextSize(&size);

    moveresize(rootx, rooty, size.w + SPACEX * 2, size.h + SPACEY * 2);
    show();
    above(TRUE);

    redraw();

    return TRUE;
}

//! 文字列のpxサイズ計算

void AXToolTipWin::_getTextSize(AXSize *pSize)
{
    int maxw = 0, maxh = 0, pos,len,ret,oneh,w;

    oneh = m_pFont->getHeight();

    for(ret = 0, pos = 0; ret != -1; )
    {
        ret = m_strText.find('\n', pos);

        len = ((ret == -1)? m_strText.getLen(): ret) - pos;

        if(len == 0 && ret != -1)
            //次行があって改行のみの場合、空白行
            maxh += oneh;
        else
        {
            w = m_pFont->getTextWidth(m_strText.at(pos), len);

            if(w > maxw) maxw = w;
            maxh += oneh;
        }

        if(ret != -1) pos = ret + 1;
    }

    pSize->set(maxw, maxh);
}


//==============================
//ハンドラ
//==============================


//! 描画

BOOL AXToolTipWin::onPaint(AXHD_PAINT *phd)
{
    int oneh,y,ret,pos,len;

    //枠

    drawBox(0, 0, m_nW, m_nH, AXAppRes::FRAMEMENU);

    //背景

    drawFillBox(1, 1, m_nW - 2, m_nH - 2, AXAppRes::FACEMENU);

    //テキスト

	oneh = m_pFont->getHeight();
	y    = SPACEY;

    AXDrawText dt(m_id);

    for(ret = 0, pos = 0; ret != -1; )
    {
        ret = m_strText.find('\n', pos);
        len = ((ret == -1)? m_strText.getLen(): ret) - pos;

        if(len == 0 && ret != -1)
            y += oneh;
        else
        {
            dt.draw(*m_pFont, SPACEX, y, m_strText.at(pos), len, AXAppRes::TC_MENU);
            y += oneh;
        }

        if(ret != -1) pos = ret + 1;
    }

    dt.end();

    return TRUE;
}
