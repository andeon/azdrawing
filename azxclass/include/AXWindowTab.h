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

#ifndef _AX_WINDOWTAB_H
#define _AX_WINDOWTAB_H

#include "AXTab.h"

class AXWindowTab:public AXTab
{
protected:
    AXWindow    *m_pwinCurrent;

protected:
    void _createWindowTab();
    void _changeTabSel();

public:
    virtual ~AXWindowTab();
    AXWindowTab(AXWindow *pParent,UINT uStyle,UINT uLayoutFlags);
    AXWindowTab(AXWindow *pParent,UINT uStyle,UINT uLayoutFlags,UINT uItemID,DWORD dwPadding);

    virtual AXWindow *getNotify();
    virtual void calcDefSize();

    AXWindow *getTabWindow(int no);
    void addTabItemTr(WORD wStrID,AXWindow *pwin);
    void setTabWindow(int no,AXWindow *pwin);
    void setTabSel(int no);

    virtual BOOL onNotify(AXWindow *pwin,UINT uNotify,ULONG lParam);
};

#endif
