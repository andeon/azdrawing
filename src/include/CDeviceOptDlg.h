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

#ifndef _AZDRAW_DEVICEOPTDLG_H_
#define _AZDRAW_DEVICEOPTDLG_H_

#include "AXDialog.h"

class AXComboBox;
class AXListBox;
class CCmdListTreeView;
class CDevOpt_DevItem;

class CDeviceOptDlg:public AXDialog
{
protected:
    AXComboBox          *m_pcbDevice;
    AXListBox           *m_pBttList;
    CCmdListTreeView    *m_pTree;

    CDevOpt_DevItem      *m_pSelDev;

protected:
    void _setDevList();
    void _setBttList();
    void _changeCmd();
    void _changeDevice();
    void _setDatToCombo();
    void _clearAll();
    void _deviceDelete();
    void _endOK();

public:
    CDeviceOptDlg(AXWindow *pOwner);

    virtual BOOL onNotify(AXWindow *pwin,UINT uNotify,ULONG lParam);
};

#endif
