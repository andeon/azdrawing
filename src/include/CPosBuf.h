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

#ifndef _AZDRAW_POSBUF_H_
#define _AZDRAW_POSBUF_H_

#include "AXDef.h"

class CPosBuf
{
public:
    typedef struct
    {
        double x,y,press;
    }POSDAT;

protected:
    POSDAT  *m_pBuf;
    int     m_nCnt;

protected:
    void _hosei(int type,int str);

public:
    CPosBuf();
    ~CPosBuf();

	int getCnt() { return m_nCnt; }
	POSDAT *getBufPt(int pos) { return m_pBuf + (pos & 31); }
	POSDAT *getLast() { return m_pBuf + ((m_nCnt - 1) & 31); }

	void setPos(double x,double y,double press,int cnt);
	CPosBuf::POSDAT *addPos(double x,double y,double press,int nHoseiType,int nHoseiStr);
};

#endif
