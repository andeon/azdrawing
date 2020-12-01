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

#ifndef _AZDRAW_FILTERPREV_H_
#define _AZDRAW_FILTERPREV_H_

#include "AXWindow.h"
#include "AXImage.h"
#include "AXRect.h"

class CLayerImg;
struct RECTANDSIZE;

class CFilterPrev:public AXWindow
{
public:
    enum
    {
        NOTIFY_UP
    };

protected:
    AXImage     m_img;
    CLayerImg   *m_pimgSrc;

    int         m_nCanvasW,
                m_nCanvasH;
    BOOL        m_bScroll;
    AXRectSize  m_rcsSrc,
                m_rcsFrame;
    AXSize      m_szAreaBox;

protected:
    BOOL _setDragPos(int x,int y);
    void _drawImgDrag();

public:
    CFilterPrev(AXWindow *pParent,UINT uLayoutFlags,UINT uID,DWORD dwPadding,int w,int h,CLayerImg *pimgSrc);

    void getDrawRect(RECTANDSIZE *prs);
    void drawImg(const CLayerImg &img);

    virtual BOOL onPaint(AXHD_PAINT *phd);
    virtual BOOL onButtonDown(AXHD_MOUSE *phd);
    virtual BOOL onButtonUp(AXHD_MOUSE *phd);
    virtual BOOL onMouseMove(AXHD_MOUSE *phd);
};

#endif
