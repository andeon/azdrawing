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

#ifndef _AZDRAW_NEWDLG_H_
#define _AZDRAW_NEWDLG_H_

#include "AXDialog.h"

class AXLineEdit;

class CNewDlg:public AXDialog
{
protected:
    AXLineEdit  *m_peditWpx,
                *m_peditHpx,
                *m_peditWcm,
                *m_peditHcm,
                *m_peditDPI;

protected:
    void _changeVal(int no);
    void _setPXandDPI(int w,int h,int dpi);
    void _recentList();
    void _registList();
    void _definedList();
    void _setDefinedName(AXString *pstr,char c,int no,DWORD size);

public:
    CNewDlg(AXWindow *pOwner);

    virtual BOOL onNotify(AXWindow *pwin,UINT uNotify,ULONG lParam);
};

#endif
