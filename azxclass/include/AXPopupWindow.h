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

#ifndef _AX_POPUPWINDOW_H
#define _AX_POPUPWINDOW_H

#include "AXTopWindow.h"

class AXPopupWindow:public AXTopWindow
{
protected:
    BOOL    m_bGrab;

public:
    virtual ~AXPopupWindow();
    AXPopupWindow(AXWindow *pOwner,UINT uStyle);

    void runPopup(int rootx,int rooty);
    virtual void endPopup(BOOL bCancel);

    void grabPopup(BOOL bOn);

    virtual BOOL onMap();
    virtual BOOL onEnter(AXHD_ENTERLEAVE *phd);
    virtual BOOL onLeave(AXHD_ENTERLEAVE *phd);
    virtual BOOL onUngrab(AXHD_ENTERLEAVE *phd);
    virtual BOOL onMouseMove(AXHD_MOUSE *phd);
    virtual BOOL onButtonDown(AXHD_MOUSE *phd);
    virtual BOOL onKeyDown(AXHD_KEY *phd);
};

#endif
