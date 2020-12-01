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

#ifndef _AX_BUF_H
#define _AX_BUF_H

#include "AXDef.h"

class AXString;

class AXBuf
{
public:
    enum ENDIANTYPE
    {
        ENDIAN_SYSTEM,
        ENDIAN_LITTLE,
        ENDIAN_BIG
    };

protected:
    LPBYTE  m_pNow,
            m_pTop;
    DWORD   m_dwSize;
    int     m_endian;

protected:
    BOOL _read(LPVOID pbuf,DWORD size);

public:
    AXBuf();

    void init(LPVOID pBuf,DWORD dwSize,int endian=ENDIAN_SYSTEM);
    void setEndian(int endian) { m_endian = endian; }

    int getPos() const { return m_pNow - m_pTop; }
    LPBYTE getNowBuf() const { return m_pNow; }
    void setNowBuf(LPBYTE pBuf) { m_pNow = pBuf; }

    //

    int getRemainSize() const;
    BOOL isRemain(DWORD size) const;

    BOOL seek(int size);
    BOOL setPos(DWORD pos);
    int setPosRetBk(int pos);

    BOOL getDat(LPVOID pbuf,DWORD size);
    BOOL getBYTE(LPVOID pbuf);
    BOOL getWORD(LPVOID pbuf);
    BOOL getDWORD(LPVOID pbuf);

    BYTE getBYTE();
    WORD getWORD();
    DWORD getDWORD();

    BOOL getStrCompare(LPCSTR pText);
    BOOL getStrLenAndUTF8(AXString *pstr);

    //

    void setDat(const void *pbuf,DWORD size);
    void setWORD(const void *pbuf);
    void setDWORD(const void *pbuf);

    void setZero(int size);
    void setStr(LPCSTR pText);
    void setBYTE(BYTE val);
    void setWORD(WORD val);
    void setDWORD(DWORD val);
};

#endif
