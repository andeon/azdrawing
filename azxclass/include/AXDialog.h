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

#ifndef _AX_DIALOG_H
#define _AX_DIALOG_H

#include "AXTopWindow.h"

class AXDialog:public AXTopWindow
{
protected:
    int     m_nDialogRet;

public:
    virtual ~AXDialog();
    AXDialog(AXWindow *pOwner,UINT uStyle);

    int getResult() const { return m_nDialogRet; }

    int runDialog(BOOL bDelete=TRUE);
    void endDialog(int ret);

    AXLayout *createOKCancelButton(int minWidth=-1);
    void createOKCancelAndShow(int paddingTop=20);

    virtual BOOL onKeyDown(AXHD_KEY *phd);
};

#endif
