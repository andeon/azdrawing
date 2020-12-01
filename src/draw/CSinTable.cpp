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

#include <math.h>

#include "CSinTable.h"


/*!
    @class CSinTable
    @brief sin 値テーブル（360個 float）
*/


CSinTable::CSinTable()
{
    int i;
    double angle,add;

    angle = 0;
    add   = M_PI / 180.0;

    for(i = 0; i < 360; i++, angle += add)
        m_pBuf[i] = (float)::sin(angle);
}

//! sin値取得 (0-360)

double CSinTable::getSin(int angle)
{
    while(angle < 0) angle += 360;

    angle %= 360;

    return (double)m_pBuf[angle];
}

//! 指定半径と角度で、X・Yに位置加算（ランダム位置時）

void CSinTable::addPosCircle(double *pX,double *pY,double r,int angle)
{
    *pX += r * getCos(angle);
    *pY += r * getSin(angle);
}
