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

#ifndef _AZDRAW_PROGRESSDLG_H_
#define _AZDRAW_PROGRESSDLG_H_

#include "AXDialog.h"

class AXProgressBar;

class CProgressDlg:public AXDialog
{
public:
    AXProgressBar   *m_pProg;

    void *(*m_func)(void *);
    LPVOID  m_pParam1,
            m_pParam2;

    int     m_nProgStep,
            m_nProgMax,
            m_nProgPos,
            m_nProgNext,
            m_nProgTopPos,
            m_nProgSubPos;

public:
    CProgressDlg(AXWindow *pOwner,void *(*func)(void *),LPVOID pParam1=NULL,LPVOID pParam2=NULL);

    int run();
    void endThread(int ret);

    void setProgMax(UINT max);
    void setProgMaxOutThread(UINT max);
    void incProg();
    void setProgPos(int pos);
    void addProgPos(int add);

    void beginProgSub(int step,int max,BOOL bOneStep=FALSE);
    void incProgSub();

    virtual BOOL onClose();
};

#endif
