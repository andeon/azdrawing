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

#ifndef _AX_DND_H
#define _AX_DND_H

#include "AXDef.h"

class AXWindow;
class AXString;

class AXDND
{
    friend class AXApp;

protected:
    ULONG       *m_patomType;
    int         m_nTypeCnt;
    ULONG       m_idSrcWin,
                m_idDstWin;
    AXWindow	*m_pwinDrop;

protected:
    void _end();
    BOOL onClientMessage(LPVOID pEvent);
    void onEnter(LPVOID pEvent);
    void onPosition(LPVOID pEvent);
    void onDrop(LPVOID pEvent);

public:
    AXDND();
    ~AXDND();

    AXWindow *getDropWindow() const { return m_pwinDrop; }

    BOOL isExistType(ULONG atomType) const;

    void endDrop();

    BOOL getFiles(AXString *pstr);
    BOOL getFirstFile(AXString *pstr);
};

#endif
