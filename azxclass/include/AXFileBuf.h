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

#ifndef _AX_FILEBUF_H
#define _AX_FILEBUF_H

#include "AXFile.h"

class AXString;

class AXFileBuf
{
protected:
    AXFile  m_file;
    LPBYTE  m_pBufNow,
            m_pBufTop;
    DWORD   m_dwSize;

public:
    AXFileBuf();

    DWORD getSize() const { return m_dwSize; }

    void close();
    BOOL openFile(const AXString &filename);
    void openBuf(LPVOID pBuf,DWORD dwSize);

    void seek(int pos);
    void setPos(DWORD pos);

    BOOL read(LPVOID pBuf,DWORD dwSize);
};

#endif
