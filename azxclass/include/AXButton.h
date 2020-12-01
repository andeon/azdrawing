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

#ifndef _AX_BUTTON_H
#define _AX_BUTTON_H

#include "AXWindow.h"
#include "AXString.h"

class AXButton:public AXWindow
{
    friend class AXTopWindow;

protected:
    enum BUTTONFLAGS
    {
        FLAG_DEFAULTBUTTON  = FLAG_TEMP1,
        FLAG_DOWN_MOUSE     = FLAG_TEMP2,
        FLAG_DOWN_KEY       = FLAG_TEMP3,
        FLAG_PRESS          = FLAG_TEMP4
    };

public:
    enum BUTTONSTYLE
    {
        BS_REAL_W     = WS_EXTRA,
        BS_REAL_H     = WS_EXTRA << 1,
        BS_NOOUTFRAME = WS_EXTRA << 2,

        BS_REAL_WH  = BS_REAL_W | BS_REAL_H,

        BS_EXTRA    = WS_EXTRA << 3
    };

    enum BUTTONNOTIFY
    {
        BN_PRESS
    };

protected:
    AXString    m_strText;
    AXSize      m_sizeText;

protected:
    void _createButton();
    virtual BOOL isAcceptKey(UINT keytype);
    BOOL isPress() { return m_uFlags & (FLAG_DOWN_MOUSE|FLAG_DOWN_KEY|FLAG_PRESS); }

public:
    virtual ~AXButton();

    AXButton(AXWindow *pParent,UINT uStyle,UINT uLayoutFlags);
    AXButton(AXWindow *pParent,UINT uStyle,UINT uLayoutFlags,UINT uItemID,DWORD dwPadding);
    AXButton(AXWindow *pParent,UINT uStyle,UINT uLayoutFlags,UINT uItemID,DWORD dwPadding,LPCUSTR pstr);
    AXButton(AXWindow *pParent,UINT uStyle,UINT uLayoutFlags,UINT uItemID,DWORD dwPadding,LPCSTR pText);
    AXButton(AXWindow *pParent,UINT uStyle,UINT uLayoutFlags,UINT uItemID,DWORD dwPadding,WORD wStrID);
    AXButton(AXWindow *pParent,UINT uStyle,UINT uLayoutFlags,UINT uItemID,DWORD dwPadding,WORD wGroupID,WORD wStrID);

    void getText(AXString *pstr) { *pstr = m_strText; }

    void setText(const AXString &str);
    void setText(LPCUSTR pText);
    void press();

    virtual void calcDefSize();

    virtual BOOL onPress();
    virtual BOOL onButtonDown(AXHD_MOUSE *phd);
    virtual BOOL onButtonUp(AXHD_MOUSE *phd);
    virtual BOOL onKeyDown(AXHD_KEY *phd);
    virtual BOOL onKeyUp(AXHD_KEY *phd);
    virtual BOOL onPaint(AXHD_PAINT *phd);
};

#endif
