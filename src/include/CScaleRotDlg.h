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

#ifndef _AZDRAW_SCALEROTDLG_H_
#define _AZDRAW_SCALEROTDLG_H_

#include "AXDialog.h"

class AXLineEdit;
class AXCheckButton;
class AXColorButton;
class CValBar;
class CScaleRot_Angle;
class CImage8;

class CScaleRotDlg:public AXDialog
{
protected:
    CImage8         *m_pimg8;
    AXRect          *m_prc;
    double          *m_pResVal;

    CValBar         *m_pbarScale;
    CScaleRot_Angle *m_pAngle;
    AXLineEdit      *m_peditScale,
                    *m_peditAngle;
    AXColorButton   *m_pbtCol;

protected:
    void _prev();
    void _drawprev();

public:
    CScaleRotDlg(CImage8 *pimg8,AXRect *prc,double *pResVal);

    BOOL onNotify(AXWindow *pwin,UINT uNotify,ULONG lParam);
    BOOL onTimer(UINT uTimerID,ULONG lParam);
};

#endif
