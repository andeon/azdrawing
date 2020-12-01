/*****************************************************************************
 *  Copyright (C) 2012-2015 Azel.
 *
 *  This file is part of AzXClass.
 *
 *  This library is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this library.  If not, see <http://www.gnu.org/licenses/>.
******************************************************************************/

#ifndef _AX_HSVPICKER_H
#define _AX_HSVPICKER_H

#include "AXWindow.h"

class AXImage;

class AXHSVPicker:public AXWindow
{
public:
    enum HSVPICKERNOTIFY
    {
        HSVPN_CHANGE_H,
        HSVPN_CHANGE_SV
    };

protected:
    AXImage     *m_pimg;
    DWORD       m_dwCol;
    int         m_nCurX,m_nCurY,m_nCurH,
                m_fBtt;

protected:
    void _createHSVPicker(int h);
    void _initImage();
    void _drawHCur();
    void _drawSVCur();
    void _drawSV();
    void _calcCol();
    void _changeH(int cury,BOOL bSend);
    void _changeSV(int x,int y,BOOL bSend);

public:
    virtual ~AXHSVPicker();

    AXHSVPicker(AXWindow *pParent,UINT uStyle,UINT uLayoutFlags,int h);
    AXHSVPicker(AXWindow *pParent,UINT uStyle,UINT uLayoutFlags,UINT uItemID,DWORD dwPadding,int h);

    DWORD getColor() const { return m_dwCol; }

    void getHSVColor(double *pHSV);
    void setHue(int hue);
    void setSV(double s,double v);
    void setColor(DWORD dwCol);

    virtual BOOL onPaint(AXHD_PAINT *phd);
    virtual BOOL onButtonDown(AXHD_MOUSE *phd);
    virtual BOOL onButtonUp(AXHD_MOUSE *phd);
    virtual BOOL onMouseMove(AXHD_MOUSE *phd);
};

#endif
