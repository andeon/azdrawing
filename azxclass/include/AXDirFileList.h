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

#ifndef _AX_DIRFILELIST_H
#define _AX_DIRFILELIST_H

#include "AXDir.h"

struct AXFILESTAT;
struct AXDIRFILELISTINFO;
class AXMemAuto;
class AXString;

class AXDirFileList
{
public:
    enum FLAGS
    {
        FLAG_SUBDIR     = 1
    };

    enum LISTTYPE
    {
        TYPE_FILE   = 0,
        TYPE_DIR    = 1,
        TYPE_DIREND = 2,
        TYPE_END    = 255
    };

protected:
    LPBYTE  m_pBuf,
            m_pNow;
    int     m_nFileCnt;

protected:
    void _find(const AXString &strDir,AXDIRFILELISTINFO *pinfo);
    void _addList(BYTE type,const AXString &strPath,AXMemAuto *pmem);

public:
    AXDirFileList();
    ~AXDirFileList();

    int getFileCnt() const { return m_nFileCnt; }
    void setNowTop() { m_pNow = m_pBuf; }

    void free();

    BOOL setList(const AXString &strDir,const AXString &strFilter,UINT uFlags);
    int getNext(AXString *pstr);
};

#endif
