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

#ifndef _AX_PROGRESSBAR_H
#define _AX_PROGRESSBAR_H

#include "AXWindow.h"
#include "AXString.h"

class AXProgressBar:public AXWindow
{
public:
    enum PROGRESSBARSTYLE
    {
        PBS_SIMPLE  = 0,
        PBS_PERS    = WS_EXTRA,
        PBS_TEXT    = WS_EXTRA << 1
    };

protected:
    AXString    m_strText;
    UINT        m_uMin,m_uMax,m_uPos,m_uRange;

protected:
    void _createProgressBar();

public:
    virtual ~AXProgressBar();

    AXProgressBar(AXWindow *pParent,UINT uStyle,UINT uLayoutFlags);
    AXProgressBar(AXWindow *pParent,UINT uStyle,UINT uLayoutFlags,UINT uItemID,DWORD dwPadding);

    virtual void calcDefSize();

    UINT getPos() const { return m_uPos; }
    UINT getMin() const { return m_uMin; }
    UINT getMax() const { return m_uMax; }

    void setStatus(UINT min,UINT max,UINT pos);
    void setPos(UINT pos);
    void incPos();
    void setText(const AXString &str);

    virtual BOOL onPaint(AXHD_PAINT *phd);
};

#endif
