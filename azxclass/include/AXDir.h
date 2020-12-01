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

#ifndef _AX_DIR_H
#define _AX_DIR_H

#include "AXString.h"

struct AXFILESTAT;

class AXDir
{
protected:
    LPVOID      m_pDir;
    AXString    m_strDir,
                m_strCurName;

public:
    AXDir();
    ~AXDir();

    const AXString& getDirName() const { return m_strDir; }
    const AXString& getCurName() const { return m_strCurName; }

    void close();
    BOOL open(const AXString &strDir);
    BOOL read();

    void getFileName(AXString *pstr,BOOL bFullPath);
    BOOL getFileStat(AXFILESTAT *pdst);
    BOOL isSpecPath();
};

#endif
