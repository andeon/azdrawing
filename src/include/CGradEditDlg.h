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

#ifndef _AZDRAW_GRADEDITDLG_H_
#define _AZDRAW_GRADEDITDLG_H_

#include "AXDialog.h"
#include "AXMem.h"

class AXLineEdit;
class AXLabel;
class CValBar2;
class CGradListItem;
class CGradEditDlg_edit;

class CGradEditDlg:public AXDialog
{
protected:
    CGradListItem   *m_pItem;

    AXMem       m_memDat;

    AXLineEdit  *m_peditName;
    AXLabel     *m_plbPos;
    CValBar2    *m_pbarVal;
    CGradEditDlg_edit   *m_pGrad;

protected:
    void _setCurrentVal();

public:
    CGradEditDlg(AXWindow *pOwner,CGradListItem *pItem);

    virtual BOOL onNotify(AXWindow *pwin,UINT uNotify,ULONG lParam);
};

#endif
