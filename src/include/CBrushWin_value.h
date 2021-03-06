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

#ifndef _AZDRAW_BRUSHWINVALUE_H_
#define _AZDRAW_BRUSHWINVALUE_H_

#include "AXWindow.h"
#include "AXImageList.h"

class AXComboBox;
class AXTab;
class CValBar2;
class CBrushWinTab_base;

class CBrushWin_value:public AXWindow
{
protected:
    enum
    {
        WID_CB_DATTYPE = 100,
        WID_BT_SAVE,
        WID_BAR_SIZE,
        WID_BT_SIZEMENU,
        WID_BAR_VAL,
        WID_CB_HOSEITYPE,
        WID_CB_HOSEISTR,
        WID_CB_PIXTYPE,
        WID_TAB
    };

protected:
    AXImageList     m_ilTab;

    AXTab           *m_ptab;
    CBrushWinTab_base *m_pwinTab;

    AXComboBox  *m_pcbDatType,
                *m_pcbHoseiType,
                *m_pcbHoseiStr,
                *m_pcbPixType;
    CValBar2    *m_pbarSize,
                *m_pbarVal;

protected:
    void _createCommon();
    void _changeTab(BOOL bLayout);
    void _showSizeMenu();

public:
    CBrushWin_value(AXWindow *pParent);

    void setBrushSize();
    void setBrushVal();
    void changeBrush();
    void setValue();

    virtual BOOL onNotify(AXWindow *pwin,UINT uNotify,ULONG lParam);
};

#endif
