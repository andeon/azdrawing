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

#ifndef _AZDRAW_KEYDAT_H_
#define _AZDRAW_KEYDAT_H_

#include "AXDef.h"

class AXString;

class CKeyDat
{
protected:
    LPDWORD m_pBuf;
    int     m_nCnt;
    BOOL    m_bChange;

public:
    ~CKeyDat();
    CKeyDat();

    LPDWORD getBuf() const { return m_pBuf; }
    int getCnt() const { return m_nCnt; }

    BOOL alloc(int cnt);
    int getKey(int id);
    int getCmd(UINT key);
    int getCmdDraw(UINT key);

    void saveFile(const AXString &filename);
    void loadFile(const AXString &filename);
};

#endif
