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

#ifndef _AX_UNDO_H
#define _AX_UNDO_H

#include "AXList.h"

class AXUndoDat;

class AXUndo:protected AXList
{
public:
    enum RETURN
    {
        RET_OK      = 0,
        RET_NODATA  = 1,
        RET_ERROR   = 2
    };

protected:
    AXUndoDat   *m_pCurrent;
    int         m_nMaxUndoCnt;

protected:
    virtual AXUndoDat *createDat() = 0;
    virtual void afterAddUndo();

public:
    virtual ~AXUndo();
    AXUndo();

    int getAllCnt() const { return m_nCnt; }
    int getMaxUndoCnt() const { return m_nMaxUndoCnt; }

    void deleteAllDat();
    void deleteUndo();
    void deleteRedo();

    void addDat(AXUndoDat *pDat);
    void setMaxUndoCnt(int cnt);

    AXUndo::RETURN undo();
    AXUndo::RETURN redo();
};

#endif
