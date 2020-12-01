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

#ifndef _AX_POPUPLISTBOX_H
#define _AX_POPUPLISTBOX_H

#include "AXPopupWindow.h"

class AXListBoxArea;
class AXListBoxItemManager;

class AXPopupListBox:public AXPopupWindow
{
public:
    enum POPUPLISTBOXSTYLE
    {
        PLBS_OWNERDRAW   = WS_EXTRA,
        PLBS_SCRV        = WS_EXTRA << 1
    };

protected:
    AXListBoxItemManager    *m_pDat;
    AXListBoxArea           *m_pArea;
    int     m_nBeginSel;

public:
    virtual ~AXPopupListBox();
    AXPopupListBox(AXWindow *pOwner,UINT uStyle,AXListBoxItemManager *pDat,int x,int y,int w,int h,int nItemH);

    virtual void endPopup(BOOL bCancel);

    virtual BOOL onNotify(AXWindow *pwin,UINT uNotify,ULONG lParam);
    virtual BOOL onKeyDown(AXHD_KEY *phd);
};

#endif
