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

#ifndef _AZDRAW_PREVWIN_H_
#define _AZDRAW_PREVWIN_H_

#include "AXTopWindow.h"

class AXImage;
class CArrowMenuBtt;

class CPrevWin:public AXTopWindow
{
public:
    static CPrevWin *m_pSelf;

protected:
    CArrowMenuBtt   *m_pbtt;
    AXImage         *m_pimg;

    double  m_dScale,
            m_dScaleDiv;
    AXPoint m_ptScr,
            m_ptImgBase;
    int     m_nBkX,m_nBkY,
            m_fDrag;

protected:
    void _setWinTitle();
    void _adjustScale();
    void _adjustScrPos();
    void _changeScale();
    void _scrollCanvas(int x,int y);
    void _showMenuNormal();
    void _showMenuLoupe();
    void _drawCross(const AXRectSize &rcs);

public:
    virtual ~CPrevWin();
    CPrevWin(AXWindow *pOwner,UINT addstyle);

    void showChange();
    void changeImgSize();
    void moveCanvasPos(double x,double y);
    void draw();
    void draw(const AXRectSize &rcs);

    virtual BOOL onClose();
    virtual BOOL onPaint(AXHD_PAINT *phd);
    virtual BOOL onSize();
    virtual BOOL onNotify(AXWindow *pwin,UINT uNotify,ULONG lParam);
    virtual BOOL onButtonDown(AXHD_MOUSE *phd);
    virtual BOOL onButtonUp(AXHD_MOUSE *phd);
    virtual BOOL onMouseMove(AXHD_MOUSE *phd);
    virtual BOOL onTimer(UINT uTimerID,ULONG lParam);
};

#define PREVWIN (CPrevWin::m_pSelf)

#endif
