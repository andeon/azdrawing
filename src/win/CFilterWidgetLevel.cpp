/************************************************************************
*  Copyright (C) 2013-2015 Azel.
*
*  This file is part of AzDrawing.
*
*  AzDrawing is free software: you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation, either version 3 of the License, or
*  (at your option) any later version.
*
*  AzDrawing is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*************************************************************************/

#include "CFilterWidgetLevel.h"

#include "AXApp.h"
#include "AXAppRes.h"

//----------------------

#define SPACE_CURSOR    3
#define CURSOR_H        8
#define HISTOGRAM_H     150
#define OUTGRAD_H       12
#define SPACE_MIDDLE    8

#define DRAGF_IN    1
#define DRAGF_OUT   2

//----------------------


/*!
    @class CFilterWidgetLevel
    @brief フィルター用ウィジェット、レベル補正
*/


CFilterWidgetLevel::CFilterWidgetLevel(AXWindow *pParent,UINT uID,DWORD dwPadding)
    : AXWindow(pParent, 0, 0, uID, dwPadding)
{
    m_nMinW = 256 + SPACE_CURSOR * 2;
    m_nMinH = HISTOGRAM_H + CURSOR_H * 2 + SPACE_MIDDLE + OUTGRAD_H;

    m_fDrag = 0;

    m_img.create(m_nMinW, m_nMinH);

    m_nVal[0] = 0;
    m_nVal[1] = 128;
    m_nVal[2] = 255;
    m_nVal[3] = 0;
    m_nVal[4] = 255;
}

//! 値取得

void CFilterWidgetLevel::getVal(int *pVal)
{
    int i;

    for(i = 0; i < 5; i++)
        pVal[i] = m_nVal[i];
}

//! 全体イメージ描画

void CFilterWidgetLevel::drawAll(LPDWORD pHistogram)
{
    int i,n;
    DWORD peek = 0,peekBk = 0,val;

    m_img.clear(axres->colRGB(AXAppRes::FACE));

    //--------- ヒストグラム

    //ピーク値（2番目に大きい値の 1.2 倍）

    for(i = 0; i < 256; i++)
    {
        val = pHistogram[i];

        if(val > peek)
        {
            peekBk = peek;
            peek   = val;
        }

        if(val > peekBk && val < peek)
            peekBk = val;
    }

    peekBk = (int)(peekBk * 1.2);
    if(peekBk != 0) peek = peekBk;

    //白背景

    m_img.fillBox(SPACE_CURSOR, 0, 256, HISTOGRAM_H, 0xffffff);

    //中央線

    m_img.lineV(SPACE_CURSOR + 128, 0, HISTOGRAM_H, 0xdddddd);

    //線

    for(i = 0; i < 256; i++)
    {
        if(pHistogram[i])
        {
            n = (int)((double)pHistogram[i] / peek * HISTOGRAM_H);
            if(n > HISTOGRAM_H) n = HISTOGRAM_H;

            if(n) m_img.lineV(SPACE_CURSOR + i, HISTOGRAM_H - n, n, 0);
        }
    }

    //-------- 出力グラデーション

    for(i = 0; i < 256; i++)
    {
        n = 255 - i;
        m_img.lineV(SPACE_CURSOR + i, HISTOGRAM_H + CURSOR_H + SPACE_MIDDLE, OUTGRAD_H, _RGB(n, n, n));
    }

    //-------- カーソル

    _drawCursor();
}

//! カーソル描画

void CFilterWidgetLevel::_drawCursor()
{
    int i,x,y;

    //消去

    m_img.fillBox(0, HISTOGRAM_H, m_nMinW, CURSOR_H, axres->colRGB(AXAppRes::FACE));
    m_img.fillBox(0, HISTOGRAM_H + CURSOR_H + SPACE_MIDDLE + OUTGRAD_H, m_nMinW, CURSOR_H, axres->colRGB(AXAppRes::FACE));

    //各カーソル

    y = HISTOGRAM_H;

	for(i = 0; i < 5; i++)
	{
		x = m_nVal[i] + SPACE_CURSOR;

		if(i == 3)
            y = HISTOGRAM_H + CURSOR_H + SPACE_MIDDLE + OUTGRAD_H;

		//

		m_img.line(x, y, x - 3, y + 7, 0);
		m_img.line(x, y, x + 3, y + 7, 0);
		m_img.line(x - 3, y + 7, x + 3, y + 7, 0);
	}
}

//! クリック/移動時

void CFilterWidgetLevel::_changePos(int x,int y)
{
    int top,cnt,pos,i,len[3],n,curno;

    if(m_fDrag == DRAGF_IN)
        top = 0, cnt = 3;
    else
        top = 3, cnt = 2;

    //

    pos = x - SPACE_CURSOR;
    if(pos < 0) pos = 0; else if(pos > 255) pos = 255;

    //各カーソルとの距離

    for(i = 0; i < cnt; i++)
    {
        n = pos - m_nVal[top + i];
        if(n < 0) n = -n;

        len[i] = n;
    }

    //距離の近いカーソル

    for(i = 0, n = 256, curno = top; i < cnt; i++)
    {
        if(len[i] < n)
        {
            n     = len[i];
            curno = top + i;
        }
    }

    //

    m_nVal[curno] = pos;

    //カーソル描画

    _drawCursor();

    redrawUpdate();
}


//==============================
//ハンドラ
//==============================


//! 描画

BOOL CFilterWidgetLevel::onPaint(AXHD_PAINT *phd)
{
    m_img.put(m_id);
    return TRUE;
}

//! ボタン押し時

BOOL CFilterWidgetLevel::onButtonDown(AXHD_MOUSE *phd)
{
    if(phd->button == BUTTON_LEFT && !m_fDrag)
    {
        //入力 or 出力

        if(phd->y >= HISTOGRAM_H && phd->y < HISTOGRAM_H + CURSOR_H)
            m_fDrag = DRAGF_IN;
        else if(phd->y >= HISTOGRAM_H + CURSOR_H + SPACE_MIDDLE + OUTGRAD_H && phd->y < m_nMinH)
            m_fDrag = DRAGF_OUT;
        else
            return TRUE;

        //

        grabPointer();

        _changePos(phd->x, phd->y);
    }

    return TRUE;
}

//! ボタン離し時

BOOL CFilterWidgetLevel::onButtonUp(AXHD_MOUSE *phd)
{
    if(phd->button == BUTTON_LEFT && m_fDrag)
    {
        m_fDrag = 0;
        ungrabPointer();

        getNotify()->onNotify(this, 0, 0);
    }

    return TRUE;
}

//! マウス移動時

BOOL CFilterWidgetLevel::onMouseMove(AXHD_MOUSE *phd)
{
    if(m_fDrag)
        _changePos(phd->x, phd->y);

    return TRUE;
}

