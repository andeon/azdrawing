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

#ifndef _AZDRAW_DRAWSHAPEDLG_H_
#define _AZDRAW_DRAWSHAPEDLG_H_

#include "AXDialog.h"

class AXLineEdit;
class AXComboBox;
class AXCheckButton;
class AXLabel;

class CDrawShapeDlg:public AXDialog
{
public:
    typedef struct
    {
        UINT    uValue;
        double  dParam[4];
        int     nDPI;
    }VALUE;

protected:
    VALUE       *m_pVal;

    AXComboBox  *m_pcbShape,
                *m_pcbSub;
    AXLineEdit  *m_peditVal,
                *m_pedit[4];
    AXCheckButton   *m_pckAA,
                    *m_pckUnit;
    AXLabel     *m_pLabel[4];

protected:
    void _setSubType();
    void _setParam();

public:
    CDrawShapeDlg(AXWindow *pOwner,VALUE *pVal);

    virtual BOOL onNotify(AXWindow *pwin,UINT uNotify,ULONG lParam);
};

#endif
