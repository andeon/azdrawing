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

#ifndef _AZDRAW_OPTIONWIN_H_
#define _AZDRAW_OPTIONWIN_H_

#include "AXTopWindow.h"

class AXTab;

class COptionWin:public AXTopWindow
{
public:
    static COptionWin *m_pSelf;

protected:
    AXTab       *m_ptab;
    AXWindow    *m_pwin;

protected:
    void _clearLayout();
    void _setTab_main();
    void _setTab_tool();

public:
    COptionWin(AXWindow *pOwner,UINT addstyle);

    void showChange();
    void changeTool();

    virtual BOOL onClose();
    virtual BOOL onNotify(AXWindow *pwin,UINT uNotify,ULONG lParam);
};

#define OPTWIN (COptionWin::m_pSelf)

#endif
