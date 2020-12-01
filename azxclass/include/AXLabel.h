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

#ifndef _AX_LABEL_H
#define _AX_LABEL_H

#include "AXWindow.h"
#include "AXString.h"

class AXLabel:public AXWindow
{
public:
    enum LABELSTYLE
    {
        LS_LEFT     = 0,
        LS_TOP      = 0,
        LS_RIGHT    = WS_EXTRA,
        LS_CENTER   = WS_EXTRA << 1,
        LS_BOTTOM   = WS_EXTRA << 2,
        LS_VCENTER  = WS_EXTRA << 3,
        LS_BORDER   = WS_EXTRA << 4,
        LS_SUNKEN   = WS_EXTRA << 5,
        LS_BKLIGHT  = WS_EXTRA << 6
    };

protected:
    AXString    m_strText;
    AXSize      m_sizeText;

protected:
    void _createLabel();
    void _getTextSize(AXSize *psize);

public:
    virtual ~AXLabel();
    AXLabel(AXWindow *pParent,UINT uStyle,UINT uLayoutFlags);
    AXLabel(AXWindow *pParent,UINT uStyle,UINT uLayoutFlags,DWORD dwPadding,LPCUSTR pstr);
    AXLabel(AXWindow *pParent,UINT uStyle,UINT uLayoutFlags,DWORD dwPadding,LPCSTR szText);
    AXLabel(AXWindow *pParent,UINT uStyle,UINT uLayoutFlags,DWORD dwPadding,WORD wStrID);

    void getText(AXString *pstr) { *pstr = m_strText; }
    void setText(const AXString &str);
    void setText(LPCUSTR pText);
    void setText(LPCSTR pText);
    void setWidthFromLen(int len);

    virtual void calcDefSize();
    virtual BOOL onPaint(AXHD_PAINT *phd);
};

#endif
