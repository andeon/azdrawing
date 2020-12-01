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

#ifndef _AZDRAW_CANVASWIN_H_
#define _AZDRAW_CANVASWIN_H_

#include "AXScrollView.h"
#include "AXScrollArea.h"


class CCanvasWinArea:public AXScrollArea
{
public:
    static CCanvasWinArea *m_pSelf;

    enum TIMERID
    {
        TIMERID_UPDATECANVAS = 1,
        TIMERID_UPDATERECT,
        TIMERID_UPDATE_MOVE,
        TIMERID_UPDATE
    };

protected:
    AXRectSize  m_rcsTimerUpdate;

public:
    CCanvasWinArea(AXWindow *pParent);

    void clearTimerUpdate(UINT uTimerID);
    void clearTimer_updateCanvas();

    void setTimer_updateCanvas();
    void setTimer_updateRect(const AXRectSize &rcs,int time=2);
    void setTimer_updateMove();
    void setTimer_update();

    virtual BOOL isShowScroll(int size,BOOL bHorz);

    virtual BOOL onSize();
    virtual BOOL onPaint(AXHD_PAINT *phd);
    virtual BOOL onTimer(UINT uTimerID,ULONG lParam);
    virtual BOOL onButtonDown(AXHD_MOUSE *phd);
    virtual BOOL onButtonUp(AXHD_MOUSE *phd);
    virtual BOOL onMouseMove(AXHD_MOUSE *phd);
    virtual BOOL onDblClk(AXHD_MOUSE *phd);
    virtual BOOL onMouseInSkip(AXHD_MOUSE *phd);
};

//-----------

class CCanvasWin:public AXScrollView
{
public:
    static CCanvasWin *m_pSelf;

    enum
    {
        CMDID_TEXTDLG = 100,
        CMDID_SCALEROTDLG
    };

protected:
    int     m_nScrCtX,
            m_nScrCtY;
    UINT    m_uLastDownKey;

public:
    CCanvasWin(AXWindow *pParent);

    UINT getLastDownKey() const { return m_uLastDownKey; }
    BOOL isDownKeySpace();

    void setScroll();
    void setScrollPos();

    virtual BOOL isAcceptKey(UINT keytype);
    virtual BOOL onKeyDown(AXHD_KEY *phd);
    virtual BOOL onKeyUp(AXHD_KEY *phd);
    virtual BOOL onFocusOut(int detail);
    virtual BOOL onCommand(UINT uID,ULONG lParam,int from);
    virtual BOOL onNotify(AXWindow *pwin,UINT uNotify,ULONG lParam);
};

#define CANVASAREA  (CCanvasWinArea::m_pSelf)
#define CANVASWIN   (CCanvasWin::m_pSelf)

#endif
