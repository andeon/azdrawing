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

#ifndef _AX_GROUPBOX_H
#define _AX_GROUPBOX_H

#include "AXWindow.h"
#include "AXString.h"

class AXGroupBox:public AXWindow
{
public:
    enum GROUPBOXSTYLE
    {
        GBS_BKLIGHT = WS_EXTRA
    };

protected:
    AXString    m_strText;
    AXSize      m_sizeText;

    void _createGroupBox();

public:
    virtual ~AXGroupBox();

    AXGroupBox(AXWindow *pParent,UINT uStyle,UINT uLayoutFlags);
    AXGroupBox(AXWindow *pParent,UINT uStyle,UINT uLayoutFlags,DWORD dwPadding);
    AXGroupBox(AXWindow *pParent,UINT uStyle,UINT uLayoutFlags,DWORD dwPadding,LPCUSTR pstr);

    void setText(const AXString &str);

    virtual void calcDefSize();
    virtual void getClientRect(AXRect *prc);

    virtual BOOL onPaint(AXHD_PAINT *phd);
};

#endif
