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

#ifndef _AZDRAW_LAYERWINAREA_H_
#define _AZDRAW_LAYERWINAREA_H_

#include "AXWindow.h"

class AXImage;
class AXScrollBar;

class CLayerWinArea:public AXWindow
{
public:
    enum
    {
        LAYERITEM_H = 35
    };

protected:
    AXScrollBar *m_pScrV;
    AXImage     *m_pimg;

    int     m_nViewCnt,
            m_fDrag,
            m_nDragPos;

protected:
    int _getAlphaBarW();
    int _checkDownInfo(int x,int y,LPINT pNo,LPINT pAlpha,UINT state,BOOL bDblClk);
    void _drawDND(int newpos,int flag);
    DWORD _getLayerFrameCol(int no);
    void _showRMenu(AXHD_MOUSE *phd);

public:
    virtual ~CLayerWinArea();
    CLayerWinArea(AXWindow *pParent);

    void setScrollBar(AXScrollBar *p) { m_pScrV = p; }

    void setScrollInfo();
    void draw();
    BOOL drawOne(int layerno,BOOL bPrev,BOOL bInfo,BOOL bRedraw);

    virtual BOOL onSize();
    virtual BOOL onPaint(AXHD_PAINT *phd);
    virtual BOOL onButtonDown(AXHD_MOUSE *phd);
    virtual BOOL onButtonUp(AXHD_MOUSE *phd);
    virtual BOOL onMouseMove(AXHD_MOUSE *phd);
    virtual BOOL onDblClk(AXHD_MOUSE *phd);
    virtual BOOL onMouseWheel(AXHD_MOUSE *phd,BOOL bUp);
    virtual BOOL onOwnerDraw(AXDrawable *pdraw,AXHD_OWNERDRAW *phd);
};

#endif
