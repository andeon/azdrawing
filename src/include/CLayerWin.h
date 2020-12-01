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

#ifndef _AZDRAW_LAYERWIN_H_
#define _AZDRAW_LAYERWIN_H_

#include "AXTopWindow.h"

class AXToolBar;
class AXScrollBar;
class CLayerWinArea;
class CLayerItem;

class CLayerWin:public AXTopWindow
{
public:
    static CLayerWin *m_pSelf;

protected:
    CLayerWinArea   *m_pArea;
    AXScrollBar     *m_pScrV;

    AXToolBar   *m_pToolBar;

protected:
    void _createToolBar();
    void _showMenu();
    void _layerNewFromImg();
    void _layerSavePNG();

public:
    CLayerWin(AXWindow *pOwner,UINT addstyle);

    void showChange();

    void layerOption(CLayerItem *pItem);

    void updateLayerAll(BOOL bScr);
    void updateLayerNo(int no);
    void updateLayerNoPrev(int no);
    void updateLayerCurInfo();

    virtual BOOL onClose();
    virtual BOOL onCommand(UINT uID,ULONG lParam,int from);
    virtual BOOL onNotify(AXWindow *pwin,UINT uNotify,ULONG lParam);
    virtual BOOL onDND_Drop(AXDND *pDND);
};

#define LAYERWIN (CLayerWin::m_pSelf)

#endif
