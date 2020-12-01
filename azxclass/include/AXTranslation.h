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

#ifndef _AX_TRANSLATION_H
#define _AX_TRANSLATION_H

#include "AXDef.h"

class AXString;

class AXTranslation
{
protected:
    LPBYTE  m_pBuf,
            m_pNowGroup;
    DWORD   m_dwGroupSize,
            m_dwStringSize;
    BOOL    m_bLoadFile;

protected:
    LPBYTE _findGroup(WORD wID) const;
    LPCUSTR _findString(LPBYTE pGroup,WORD wID) const;
    void _getFileName(AXString *pstr,LPCSTR szLang,const AXString &strDir);

public:
    ~AXTranslation();
    AXTranslation();

    void free();
    BOOL loadFile(LPCSTR szLang,const AXString &strDir);
    void setBuf(LPBYTE pbuf);

    BOOL setGroup(WORD wGroupID);
    LPCUSTR getString(WORD wStrID) const;
    LPCUSTR getString(WORD wGroupID,WORD wStrID) const;
};

#endif
