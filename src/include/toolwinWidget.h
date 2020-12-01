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

#ifndef _AZDRAW_TOOLWINWIDGET_H_
#define _AZDRAW_TOOLWINWIDGET_H_

#include "AXButton.h"

class AXImageList;


class CImgListLabel:public AXWindow
{
protected:
    AXImageList *m_pimgList;
    int         m_nImgNo;

public:
    CImgListLabel(AXWindow *pParent,UINT uLayoutFlags,DWORD dwPadding,AXImageList *pimgList,int nImgNo);

    virtual void calcDefSize();
    virtual BOOL onPaint(AXHD_PAINT *phd);
};

//-------

class CImgListButton:public AXButton
{
protected:
    AXImageList *m_pimgList;
    int         m_nImgNo;

public:
    CImgListButton(AXWindow *pParent,UINT uLayoutFlags,UINT uID,DWORD dwPadding,AXImageList *pimgList,int nImgNo);

    virtual void calcDefSize();
    virtual BOOL onPaint(AXHD_PAINT *phd);
};

//-------

class CScaleRotBar:public AXWindow
{
public:
    enum
    {
        SRBS_SCALE  = 0,
        SRBS_ROTATE = WS_EXTRA,

        FLAG_DRAG   = FLAG_TEMP1,

        SRBN_DOWN   = 1,
        SRBN_MOVE   = 2,
        SRBN_UP     = 3
    };

protected:
    int     m_nPos,m_nMin,m_nMax;

    BOOL _changePos(int x);

public:
    CScaleRotBar(AXWindow *pParent,UINT uStyle,UINT uID,DWORD dwPadding,int min,int max);

    int getPos() { return m_nPos; }
    BOOL isDrag() { return m_uFlags & FLAG_DRAG; }
    void setPos(int pos);

    virtual void calcDefSize();

    virtual BOOL onPaint(AXHD_PAINT *phd);
    virtual BOOL onButtonDown(AXHD_MOUSE *phd);
    virtual BOOL onButtonUp(AXHD_MOUSE *phd);
    virtual BOOL onMouseMove(AXHD_MOUSE *phd);
    virtual BOOL onTimer(UINT uTimerID,ULONG lParam);
};

#endif
