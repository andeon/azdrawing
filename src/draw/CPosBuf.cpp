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

#include <stdlib.h>
#include <math.h>

#include "CPosBuf.h"


/*!
    @class CPosBuf
    @brief ブラシ描画時の点バッファ

    - 手ぶれ補正用に過去の位置を保存するバッファ。
    - 32個をリング状のデータとする。
    - 最後の点に手ぶれ補正をかけていく。
*/


CPosBuf::CPosBuf()
{
    m_pBuf = (POSDAT *)::malloc(sizeof(POSDAT) * 32);
    m_nCnt = 0;
}

CPosBuf::~CPosBuf()
{
    if(m_pBuf) ::free(m_pBuf);
}

//! 位置セット（初期時。cnt 分同じ値を追加する）

void CPosBuf::setPos(double x,double y,double press,int cnt)
{
    int i;
    POSDAT *p;

    for(i = 0; i < cnt; i++)
    {
        p = getBufPt(i);

        p->x = x;
        p->y = y;
        p->press = press;
    }

    m_nCnt = cnt;
}

//! 位置追加（移動時）

CPosBuf::POSDAT *CPosBuf::addPos(double x,double y,double press,int nHoseiType,int nHoseiStr)
{
    POSDAT *p;

    //追加

    p = getBufPt(m_nCnt);

    p->x = x;
    p->y = y;
    p->press = press;

    m_nCnt++;

    //手ブレ補正適用

    if(nHoseiStr)
        _hosei(nHoseiType, nHoseiStr);

    return p;
}


//===================


//! 補正

void CPosBuf::_hosei(int type,int str)
{
    POSDAT *p;
    int i,pos = m_nCnt - 1;
    double x = 0,y = 0,/*press = 0,*/w,div = 0;

    switch(type)
    {
        //強（単純平均）
        case 1:
            w = 1.0 / (str + 1);

            for(i = str; i >= 0; i--, pos--)
            {
                p = getBufPt(pos);

                x += p->x * w;
                y += p->y * w;
                //press += p->press * w;

            }
            break;
        //中（位置により重みをつける）
        case 2:
            w = str + 1;

            for(i = str; i >= 0; i--, pos--, w -= 1)
            {
                p = getBufPt(pos);

                div += w;

                x += p->x * w;
                y += p->y * w;
                //press += p->press * w;
            }

            div = 1.0 / div;
            x *= div;
            y *= div;
            //press *= div;
            break;
        //弱（ガウスで重みをつける）
        case 3:
            double v,v2;

            v  = (str + 1) / 3.0;   //ar
            v2 = 1.0 / (2 * v * v);

            for(i = 0; i <= str; i++, pos--)
            {
                p = getBufPt(pos);

                w = ::exp(-(i * i) * v2);
                div += w;

                x += p->x * w;
                y += p->y * w;
                //press += p->press * w;
            }

            div = 1.0 / div;
            x *= div;
            y *= div;
            //press *= div;
            break;
    }

    //セット

    p = getLast();

    p->x = x;
    p->y = y;
    //p->press = press;
}
