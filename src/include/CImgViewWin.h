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

#ifndef _AZDRAW_IMGVIEWWIN_H_
#define _AZDRAW_IMGVIEWWIN_H_

#include "AXTopWindow.h"
#include "AXString.h"

class CImage32;
class CArrowMenuBtt;
class AXImage;

class CImgViewWin:public AXTopWindow
{
public:
    static CImgViewWin *m_pSelf;

protected:
    AXImage         *m_pimg;
    CImage32        *m_pimgSrc;
    CArrowMenuBtt   *m_pbtt;

    AXString    m_strFileName;

    BOOL    m_bDrag;
    int     m_nScrX,m_nScrY,
            m_nBkX,m_nBkY;
    double  m_dScale,m_dScaleDiv;

protected:
    void _openFile();
    BOOL _loadImg(BOOL bSaveDir);
    void _clearImg();
    void _setWinTitle();
    void _scaleUpDown(BOOL bDown);
    void _setScale(int no);
    void _nextPrevFile(BOOL bNext);
    void _draw();
    void _adjustScrPos();
    BOOL _adjustWinSize();
    void _adjustScale();
    void _spoit(int x,int y,int rootx,int rooty);

public:
    virtual ~CImgViewWin();
    CImgViewWin(AXWindow *pOwner,UINT addstyle);

    void showChange();

    virtual BOOL onClose();
    virtual BOOL onPaint(AXHD_PAINT *phd);
    virtual BOOL onSize();
    virtual BOOL onCommand(UINT uID,ULONG lParam,int from);
    virtual BOOL onNotify(AXWindow *pwin,UINT uNotify,ULONG lParam);
    virtual BOOL onDND_Drop(AXDND *pDND);
    virtual BOOL onKeyDown(AXHD_KEY *phd);
    virtual BOOL onButtonDown(AXHD_MOUSE *phd);
    virtual BOOL onButtonUp(AXHD_MOUSE *phd);
    virtual BOOL onMouseMove(AXHD_MOUSE *phd);
    virtual BOOL onTimer(UINT uTimerID,ULONG lParam);
};

#define IMGVIEWWIN  (CImgViewWin::m_pSelf)

#endif
