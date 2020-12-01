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

#ifndef _AZDRAW_DRAWPAINT_H_
#define _AZDRAW_DRAWPAINT_H_

#include "CLayerImg.h"
#include "AXRect.h"

struct FLAGRECT;

class CDrawPaint
{
public:
    enum
    {
        TYPE_ANTIAUTO   = 0,
        TYPE_LESS       = 1,
        TYPE_ERASE      = 10,
        TYPE_CLOSE      = 11
    };

    enum
    {
        RET_SUCCESS     = 0,
        RET_NODRAW      = 1,
        RET_ERR_MEMORY  = 2
    };

protected:
    enum { BUFSIZE = 8192 };

    struct PAINTBUF
    {
        short lx,rx,y,oy;
    };

protected:
    PAINTBUF    *m_pBuf;        //!< 塗りつぶしバッファ

    CLayerImg   m_imgRef,       //!< 判定元イメージ
                m_imgDraw;      //!< 描画イメージ

    CLayerImg   *m_pimgDst,     //!< 描画先
                *m_pimgRefSt;   //!< 判定元開始位置

    int         m_nType,        //!< タイプ
                m_nVal,         //!< 描画濃度
                m_nCompVal,     //!< 比較用濃度
                m_nStartVal;    //!< 最初の点の濃度

    AXPoint     m_ptStart;      //!< 開始位置(px)
    AXRect      m_rcArea,       //!< 判定元全範囲(px)
                m_rcDrawArea;   //!< 描画用のタイルpx範囲

    CTileImg    *m_pCurTile;    //!< NULLで確保失敗
    int         m_nCurTileX,
                m_nCurTileY,
                m_nCurPX,
                m_nCurPY,
                m_nCurSubX,
                m_nCurSubY;

protected:
    void _runErase();
    void _erase_scan(int lx,int rx,int y,int oy,PAINTBUF **pped);

    void _runLess();
    void _less_scan(int lx,int rx,int y,int oy,PAINTBUF **pped);

    void _runAuto();
    void _runAuto_horz();
    void _runAuto_vert();
    void _auto_scan_h(int lx,int rx,int y,int oy,PAINTBUF **pped);
    void _auto_scan_v(int ly,int ry,int x,int ox,PAINTBUF **pped);

    void _drawDst();
    void _drawClose();

    BOOL _drawRefLineH(int x1,int x2,int y,BYTE val);
    BOOL _drawRefLineV(int y1,int y2,int x,BYTE val);
    void _drawDrawLineH(int x1,int x2,int y,BYTE val);
    void _drawDrawLineV(int y1,int y2,int x,BYTE val);

    void _close_blendRefTile(CTileImg *ptile,int xtop,int ytop,CLayerImg *pArea,CLayerImg *pRefSt);
    void _getAreaRect();
    void _getAllRefRect(AXRect *prc);
    int _getRefPixel(int x,int y);
    BOOL _setRefTile(int tx,int ty,CTileImg **ppDstTile);
    void _setCurrent(int px,int py);
    int _getCurPixel();
    BOOL _setCurPixel(BYTE val);
    void _movCurRight();
    void _movCurLeft();
    void _movCurDown();
    void _movCurUp();

public:
    CDrawPaint();
    ~CDrawPaint();

    void free();

    int init(CLayerImg *pDst,CLayerImg *pRefSt,const AXPoint &ptStart,int nVal,int nCompVal,int type);
    int setCloseRefImg(CLayerImg *pimgDst,CLayerImg *pimgArea,const FLAGRECT &rcf,CLayerImg *pRefSt);
    void run();
};

#endif
