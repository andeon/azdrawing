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

#ifndef _AZDRAW_DRAWTEXTDLG_H_
#define _AZDRAW_DRAWTEXTDLG_H_

#include "AXDialog.h"

class CDrawTextDlgEdit;
class AXComboBox;
class AXLineEdit;

class CDrawTextDlg:public AXDialog
{
protected:
    CDrawTextDlgEdit *m_pEdit;
    AXComboBox      *m_pcbFont,
                    *m_pcbStyle,
                    *m_pcbHinting;
    AXLineEdit      *m_peditSize,
                    *m_peditCharSP,
                    *m_peditLineSP;

protected:
    void _end(BOOL bOK);
    void _prev();
    void _setFontFace();
    void _setFontStyle();

public:
    CDrawTextDlg();

    BOOL onClose();
    BOOL onNotify(AXWindow *pwin,UINT uNotify,ULONG lParam);
    BOOL onTimer(UINT uTimerID,ULONG lParam);
};

#endif
