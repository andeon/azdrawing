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

#ifndef _AX_LINEEDIT_H
#define _AX_LINEEDIT_H

#include "AXWindow.h"

class AXString;
class AXEditString;

class AXLineEdit:public AXWindow
{
public:
    enum LINEEDITSTYLE
    {
        ES_READONLY = WS_EXTRA,
        ES_SPIN     = WS_EXTRA << 1
    };

    enum LINEEDITNOTIFY
    {
        EN_CHANGE
    };

protected:
    AXEditString    *m_pString;
    int     m_nScrX,
            m_nTextScrTop,
            m_nScrAdjX,
            m_fBtt,
            m_nBkPos,
            m_nValMin,
            m_nValMax,
            m_nValDig;

protected:
    virtual void getICPos(AXPoint *pPos);
    void _createLineEdit();
    int _getEditRight();
    void _getSpinHeight(LPINT pH1,LPINT pH2);
    void _adjustScroll();
    void _setScroll(int scrx);
    void _drawText();
    void _drawSpin();
    void _spinUpDown();

public:
    virtual ~AXLineEdit();

    AXLineEdit(AXWindow *pParent,UINT uStyle,UINT uLayoutFlags);
    AXLineEdit(AXWindow *pParent,UINT uStyle,UINT uLayoutFlags,UINT uItemID,DWORD dwPadding);

    BOOL isSpin() const { return (m_uStyle & ES_SPIN); }

    void setWidthFromLen(int len);

    void setText(LPCUSTR pText);
    void setInt(int val);
    void setValStatus(int min,int max,int dig=0);
    void setVal(int val);
    void getText(AXString *pstr);
    int getTextLen();
    int getVal();
    int getInt();
    double getDouble();
    void selectAll();
    void setInit(int widthlen,int min,int max,int val);
    void setInit(int widthlen,int min,int max,int dig,int val);

    virtual void calcDefSize();

    virtual BOOL onFocusIn(int detail);
    virtual BOOL onFocusOut(int detail);
    virtual BOOL onPaint(AXHD_PAINT *phd);
    virtual BOOL onKeyDown(AXHD_KEY *phd);
    virtual BOOL onButtonDown(AXHD_MOUSE *phd);
    virtual BOOL onButtonUp(AXHD_MOUSE *phd);
    virtual BOOL onMouseMove(AXHD_MOUSE *phd);
    virtual BOOL onTimer(UINT uTimerID,ULONG lParam);
};

#endif
