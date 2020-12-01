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

#ifndef _AZDRAW_POLYGONPOS_H_
#define _AZDRAW_POLYGONPOS_H_

#include "AXDef.h"

class CLayerImg;

class CPolygonPos
{
    friend class CLayerImg;

protected:
    typedef struct
    {
        int x,y;
    }POSDAT;

    typedef struct
    {
        int x[4],bUpper;
    }EDGEDAT;

protected:
    POSDAT  *m_pBuf;
    EDGEDAT *m_pEdge;
    LPWORD  m_pValBuf;      //!< 濃度バッファ

    int	    m_nCnt,         //!< 点の数
            m_nBufCnt,
            m_nMinY,        //!< 最小・最大Y (px)
            m_nMaxY,
            m_nMinX,        //!< 最小・最大X (px)
            m_nMaxX,
            m_nEdgeCnt,     //!< 交点数
            m_nEdgeBufCnt,
            m_nValBufCnt;   //!< 濃度バッファ確保数

protected:
    BOOL _resize();

    BOOL allocEdge();
    BOOL findIntersection(int y);
    BOOL findIntersectionNoAnti(int y);
    void setValBuf();

    BOOL _addEdge(LPINT pVal,BOOL bUpper);
    void _sortEdge(int pos);

public:
    CPolygonPos();
    ~CPolygonPos();

	int getPosCnt() { return m_nCnt; }
	int getMinY() { return m_nMinY; }
	int getMaxY() { return m_nMaxY; }
	int getMinX() { return m_nMinX; }
	int getMaxWidth() { return m_nMaxX - m_nMinX + 1; }
	EDGEDAT *getEdgeBuf() { return m_pEdge; }
	int getEdgeCnt() { return m_nEdgeCnt; }
	LPWORD getValBuf() { return m_pValBuf; }

    void free();
    BOOL alloc(int cnt);
    BOOL add(int x,int y);
    void endPos();
};

#endif
