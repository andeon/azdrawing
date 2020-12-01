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

#ifndef _AX_CHECKBUTTON_H
#define _AX_CHECKBUTTON_H

#include "AXWindow.h"
#include "AXString.h"

class AXCheckButton:public AXWindow
{
public:
    enum CHECKBUTTONSTYLE
    {
        CBS_BUTTON   = WS_EXTRA,
        CBS_RADIO    = WS_EXTRA << 1,
        CBS_GROUP    = WS_EXTRA << 2,
        CBS_REALSIZE = WS_EXTRA << 3,
        CBS_CHECK    = WS_EXTRA << 4,
        CBS_BKLIGHT  = WS_EXTRA << 5
    };

    enum CHECKBUTTONNOTIFY
    {
        CBN_TOGGLE
    };

protected:
    enum CHECKBUTTONFLAGS
    {
        FLAG_DOWN_MOUSE = FLAG_TEMP1,
        FLAG_DOWN_KEY   = FLAG_TEMP2
    };

protected:
    AXString    m_strText;
    AXSize      m_sizeText;

protected:
    void _createCheckButton(LPINT pCheck);
    void _check(BOOL bPress);
    AXCheckButton *_getRadioGroup(AXCheckButton **pTop,AXCheckButton **pEnd);
    void _drawCheckBox();
    void _drawCheckButton();

public:
    virtual ~AXCheckButton();

    AXCheckButton(AXWindow *pParent,UINT uStyle,UINT uLayoutFlags);
    AXCheckButton(AXWindow *pParent,UINT uStyle,UINT uLayoutFlags,UINT uItemID,DWORD dwPadding,LPCUSTR pstr);
    AXCheckButton(AXWindow *pParent,UINT uStyle,UINT uLayoutFlags,UINT uItemID,DWORD dwPadding,LPCUSTR pstr,BOOL bCheck);
    AXCheckButton(AXWindow *pParent,UINT uStyle,UINT uLayoutFlags,UINT uItemID,DWORD dwPadding,LPCSTR pText,BOOL bCheck);
    AXCheckButton(AXWindow *pParent,UINT uStyle,UINT uLayoutFlags,UINT uItemID,DWORD dwPadding,WORD wStrID,BOOL bCheck);

    void setText(const AXString &str);
    void setCheck(BOOL bCheck);
    BOOL isChecked();
    BOOL isButtonDown();
    int getGroupSel();

    virtual void calcDefSize();

    virtual BOOL onPress();
    virtual BOOL onButtonDown(AXHD_MOUSE *phd);
    virtual BOOL onButtonUp(AXHD_MOUSE *phd);
    virtual BOOL onKeyDown(AXHD_KEY *phd);
    virtual BOOL onKeyUp(AXHD_KEY *phd);
    virtual BOOL onPaint(AXHD_PAINT *phd);
};

#endif
