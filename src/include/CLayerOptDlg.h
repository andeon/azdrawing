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

#ifndef _AZDRAW_LAYEROPTDLG_H_
#define _AZDRAW_LAYEROPTDLG_H_

#include "AXDialog.h"

class CLayerItem;
class AXLineEdit;
class AXColorPrev;
class AXHSVPicker;
class AXLabel;
class AXLayout;
class CValBar;
class CLayerOptDlg_pal;

class CLayerOptDlg:public AXDialog
{
protected:
    CLayerItem  *m_pItem;

    AXLineEdit  *m_peditName,
                *m_peditOpacity,
                *m_peditCol[3];
    AXColorPrev *m_pColPrev;
    AXHSVPicker *m_pHSV;
    CValBar     *m_pbar[3];
    AXLabel     *m_plabelCol[3];
    AXLayout    *m_playoutCol;
    CLayerOptDlg_pal *m_pPalette;

    int     m_nColType;

protected:
    void _changeColAll(BOOL bInit);
    void _updateColPrev();

public:
    CLayerOptDlg(AXWindow *pOwner,CLayerItem *pItem);

    virtual BOOL onNotify(AXWindow *pwin,UINT uNotify,ULONG lParam);
};

#endif
