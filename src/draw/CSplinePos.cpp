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

#include "CSplinePos.h"


/*!
    @class CSplinePos
    @brief スプライン位置クラス
*/


CSplinePos::CSplinePos()
{
    m_pBuf = NULL;
}

CSplinePos::~CSplinePos()
{
    free();
}

//! 解放

void CSplinePos::free()
{
    AXFree((void **)&m_pBuf);
}

//! メモリ確保

BOOL CSplinePos::alloc()
{
    free();

    m_pBuf = (POSDAT *)AXMalloc(sizeof(POSDAT) * 60);
    if(!m_pBuf) return FALSE;

    m_nCnt = 0;

    return TRUE;
}

//! 追加

BOOL CSplinePos::add(int x,int y,double dx,double dy,double press)
{
    if(!m_pBuf || m_nCnt >= 60) return FALSE;

    //前の位置と同じ場合追加しない

    if(m_nCnt != 0)
    {
        if(m_pBuf[m_nCnt - 1].cx == x && m_pBuf[m_nCnt - 1].cy == y)
            return FALSE;
    }

    //追加

    m_pBuf[m_nCnt].cx    = x;
    m_pBuf[m_nCnt].cy    = y;
    m_pBuf[m_nCnt].x     = dx;
    m_pBuf[m_nCnt].y     = dy;
    m_pBuf[m_nCnt].press = press;

    m_nCnt++;

    return TRUE;
}
