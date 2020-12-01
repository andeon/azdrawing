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

#ifndef _AX_BYTESTRING_H
#define _AX_BYTESTRING_H

#include "AXDef.h"

class AXByteString
{
protected:
    LPSTR   m_pBuf;
    int     m_nLen,
            m_nMallocSize;

protected:
    void _malloc(int len);
    int _calcSize(int len);

public:
    ~AXByteString();
    AXByteString();
    AXByteString(int len);

    operator LPCSTR() const { return m_pBuf; }
    operator LPSTR() const { return m_pBuf; }
    operator LPBYTE() const { return (LPBYTE)m_pBuf; }
    operator LPVOID() const { return (LPVOID)m_pBuf; }

    int getLen() const { return m_nLen; }
    int getMallocSize() const { return m_nMallocSize; }
    int getMaxLen() const { return m_nMallocSize - 1; }
    LPSTR at(int pos) const { return m_pBuf + pos; }

    BOOL resize(int len);
    void recalcLen();
    void empty();
    BOOL isEmpty() const;
    BOOL isNoEmpty() const;

    char operator [](int pos) const;
    void operator =(char ch);
    void operator =(LPCSTR text);
    void operator =(const AXByteString &str);
    void operator +=(char ch);
    void operator +=(LPCSTR text);
    void operator +=(int val);
    BOOL operator ==(LPCSTR text) const;

    void set(LPCSTR text,int len);
    void append(LPCSTR text,int len);
    void setAddInt(LPCSTR text,int val);
    void setAt(int pos,char ch);
    void prefix(LPCSTR text);

    void toLower();
    void replace(char ch,char chNew);
    int find(char ch);
};

#endif
