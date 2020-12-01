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

#ifndef _AZDRAW_SPLINEPOS_H_
#define _AZDRAW_SPLINEPOS_H_

#include "AXDef.h"

class CSplinePos
{
public:
    typedef struct
    {
        int     cx,cy;  //!< ウィンドウ位置
        double  x,y,    //!< 描画位置
                press;  //!< 筆圧
    }POSDAT;

protected:
    POSDAT  *m_pBuf;
    int     m_nCnt;

public:
    CSplinePos();
    ~CSplinePos();

    POSDAT *getBuf() { return m_pBuf; }
    int getCnt() { return m_nCnt; }

    void free();
    BOOL alloc();
    BOOL add(int x,int y,double dx,double dy,double press);
    void delLast() { m_nCnt--; }
};

#endif
