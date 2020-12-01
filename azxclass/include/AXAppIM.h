/*****************************************************************************
 *  Copyright (C) 2012-2015 Azel.
 *
 *  This file is part of AzXClass.
 *
 *  This library is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this library.  If not, see <http://www.gnu.org/licenses/>.
******************************************************************************/

#ifndef _AX_APPIM_H
#define _AX_APPIM_H

#include "AXDef.h"

class AXWindow;
class AXTopWindow;
class AXAppIMWin;


class AXAppIM
{
protected:
    LPVOID  m_pIM;
    UINT    m_uStyle;

public:
    ~AXAppIM();
    AXAppIM();

    LPVOID getIM() const { return m_pIM; }
    UINT getStyle() const { return m_uStyle; }

    BOOL isEmpty() const { return (m_pIM == NULL); }
    BOOL isExist() const { return (m_pIM != NULL); }

    BOOL init(LPVOID pDisp);
    void empty() { m_pIM = NULL; }
};



class AXAppIC
{
protected:
    LPVOID      m_pIC;
    AXAppIM     *m_pAXIM;
    AXTopWindow *m_pwinClient;
    AXAppIMWin  *m_pwinPE;

public:
    ~AXAppIC();
    AXAppIC();

    LPVOID getIC() const { return m_pIC; }

    BOOL init(AXAppIM *pAXIM,AXWindow *pwinClient);

    void setFocus();
    void killFocus();
    void moveWinPos();

    void PE_start();
    void PE_done();
    void PE_draw(LPVOID pParam);
};

#endif
