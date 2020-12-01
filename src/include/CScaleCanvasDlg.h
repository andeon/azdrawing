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

#ifndef _AZDRAW_SCALECANVASDLG_H_
#define _AZDRAW_SCALECANVASDLG_H_

#include "AXDialog.h"

class AXLineEdit;
class AXCheckButton;
class AXComboBox;

class CScaleCanvasDlg:public AXDialog
{
public:
    typedef struct
    {
        int     nWidth,
                nHeight,
                nDPI,       //!< -1で変更なし
                nType;
    }VALUE;

protected:
    VALUE       *m_pVal;

    AXSize      m_szPix,
                m_szPer;

    AXLineEdit  *m_peditSizeW,
                *m_peditSizeH,
                *m_peditPersW,
                *m_peditPersH,
                *m_peditDPI;
    AXCheckButton   *m_pckAspect,
                    *m_pckDPI;
    AXComboBox  *m_pcbType;

protected:
    void _changeVal(int type);
    void _changeDPI();
    void _setChangeDPI();
    void _setVal(int type);

public:
    CScaleCanvasDlg(AXWindow *pOwner,VALUE *pVal);

    virtual BOOL onNotify(AXWindow *pwin,UINT uNotify,ULONG lParam);
};

#endif
