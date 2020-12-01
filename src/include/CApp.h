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

#ifndef _AZDRAW_APP_H_
#define _AZDRAW_APP_H_

#include "AXApp.h"

class CDevList;

class CApp:public AXApp
{
protected:
    int     m_nGrabDevID;
    ULONG   m_XIdbcWin,
            m_XIdbcTime;
    int     m_XIdbcX,m_XIdbcY;
	double  m_dLastPressure;

    AXString    m_strTmpDir;

protected:
    virtual void onInputEvent(AXWindow *pwin,LPVOID pEvent,BOOL bSkip);
    double _getPress(LPVOID pEvent);

public:
    CApp();

    const AXString& getTmpDir() const { return m_strTmpDir; }

    virtual BOOL isGrab();

    void init2();
    void end2();
    void ungrabBoth();

    void initXI(AXWindow *pwin);
    void setDevice(CDevList *pDev);

    void grabXI(ULONG idWin,int srcid);
    void ungrabXI();
};

#define CAPP    ((CApp *)(AXApp::m_pApp))

#endif
