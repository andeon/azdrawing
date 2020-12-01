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
//SubWidget.cpp

#ifndef _AZDRAW_SELIMGBAR_H_
#define _AZDRAW_SELIMGBAR_H_

#include "AXWindow.h"
#include "AXString.h"

class CSelImgBar:public AXWindow
{
public:
    enum
    {
        NOTIFY_LEFT,
        NOTIFY_RIGHT
    };

protected:
    AXString    m_strName;

public:
    CSelImgBar(AXWindow *pParent,UINT uStyle,UINT uLayoutFlags,UINT uItemID,DWORD dwPadding);

    void setName(const AXString &name);
    void setName(LPCUSTR pText);

    virtual void calcDefSize();
    virtual BOOL onPaint(AXHD_PAINT *phd);
    virtual BOOL onButtonDown(AXHD_MOUSE *phd);
};

#endif
