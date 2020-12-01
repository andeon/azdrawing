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

#ifndef _AZDRAW_EXPORTDLG_H_
#define _AZDRAW_EXPORTDLG_H_

#include "AXDialog.h"

class AXLineEdit;
class AXComboBox;
class AXCheckButton;

class CExportDlg:public AXDialog
{
public:
    typedef struct
    {
        int     nFormat,
                nWidth,
                nHeight,
                nScaleType;
    }VALUE;

protected:
    VALUE       *m_pVal;

    BOOL        m_bPers,
                m_bKeepAspect;

    AXComboBox  *m_pcbFormat,
                *m_pcbType;
    AXLineEdit  *m_peditW,
                *m_peditH,
                *m_peditPers;
    AXCheckButton *m_pckPers;

protected:
    void _enable_scale(BOOL bEnable);

public:
    CExportDlg(AXWindow *pOwner,VALUE *pVal);

    virtual BOOL onNotify(AXWindow *pwin,UINT uNotify,ULONG lParam);
};

#endif
