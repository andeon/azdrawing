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

#ifndef _AZDRAW_TOOLWIN_H_
#define _AZDRAW_TOOLWIN_H_

#include "AXTopWindow.h"

class AXToolBar;
class CScaleRotBar;

class CToolWin:public AXTopWindow
{
public:
    static CToolWin *m_pSelf;

    enum
    {
        CMDID_TOOL      = 5000,
        CMDID_UNDO      = 5100,
        CMDID_REDO      = 5101,
        CMDID_DESELECT  = 5102,
        CMDID_CANVASHREV = 5103,

        CMDID_SUB_TOP   = 6000,

        SUBNO_DRAW      = 0,
        SUBNO_POLYGON   = 1,
        SUBNO_SEL       = 2,
        SUBNO_GRAD      = 3,
        SUBNO_BOXEDIT   = 4
    };

protected:
    AXToolBar   *m_ptbTool,
                *m_ptbSub[5],
                *m_ptbSubNow;
    CScaleRotBar    *m_pwidScale,
                    *m_pwidRot;

protected:
    void _addTrString(AXString *pstr,WORD wGroupID,int cnt);
    void _createTB_tool();
    void _createTB_sub();
    void _initTB_sub();
    void _createWidget();

public:
    CToolWin(AXWindow *pOwner,UINT addstyle);

    void showChange();
    void changeTool();
    void changeScale();
    void changeRotate();
    void checkTB_tool(int no);
    void checkTB_toolsub_pen(int no);
    void checkTB_cmd(UINT uID,BOOL bCheck);

    virtual BOOL onClose();
    virtual BOOL onCommand(UINT uID,ULONG lParam,int from);
    virtual BOOL onNotify(AXWindow *pwin,UINT uNotify,ULONG lParam);
};

#define TOOLWIN (CToolWin::m_pSelf)

#endif
