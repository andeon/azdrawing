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

#ifndef _AZDRAW_STATUSBAR_H_
#define _AZDRAW_STATUSBAR_H_

#include "AXWindow.h"

class AXLabel;

class CStatusBar:public AXWindow
{
public:
    static CStatusBar *m_pSelf;

protected:
    AXLabel *m_pInfo,
            *m_pHelp;

public:
    CStatusBar(AXWindow *pParent);

    void setInfoLabel();
    void setLabel_help();
    void setLabel_rule(double dLen,int angle);
};

#define STATUSBAR   (CStatusBar::m_pSelf)

#endif
