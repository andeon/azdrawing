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

#ifndef _AZDRAW_OPTIONDLG_H_
#define _AZDRAW_OPTIONDLG_H_

#include "AXDialog.h"
#include "AXString.h"
#include "AXMem.h"

class AXListBox;
class AXLayout;
class COptDlg_base;

class COptionDlg:public AXDialog
{
public:
    typedef struct
    {
        int     nInitW,
                nInitH,
                nUndoCnt,
                nStepScaleD,
                nStepScaleU,
                nStepRotate,
                nStepBrushSize,
                nToolboxType;
        DWORD   dwCanvasCol;
        UINT    uFlags;

        AXMem   memDrawCursor;

        AXString    strSubWinFont,
                    strTextureDir,
                    strBrushDir;
    }OPTIONDAT;

protected:
    AXListBox   *m_plbMenu;
    AXLayout    *m_playout;

    COptDlg_base    *m_pwin;

    OPTIONDAT   m_dat;

protected:
    void _getConf();
    BOOL _setConf();
    void _setWidget(BOOL bInit);

public:
    COptionDlg(AXWindow *pOwner);

    virtual BOOL onNotify(AXWindow *pwin,UINT uNotify,ULONG lParam);
};

#endif
