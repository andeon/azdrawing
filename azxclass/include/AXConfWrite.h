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

#ifndef _AX_CONFWRITE_H
#define _AX_CONFWRITE_H

#include "AXFileWriteBuf.h"
#include "AXByteString.h"

class AXString;
class AXRectSize;

class AXConfWrite:public AXFileWriteBuf
{
protected:
    AXByteString    m_str;

protected:
    void _put();
    void _putVal(LPCSTR szKey,LPCSTR szVal);
    void _putVal(int keyno,LPCSTR szVal);

public:
    BOOL openHome(LPCSTR szPath);

    void putGroup(LPCSTR szName);
    void putInt(LPCSTR szKey,int val);
    void putHex(LPCSTR szKey,DWORD val);
    void putStr(LPCSTR szKey,LPCSTR szText);
    void putStr(LPCSTR szKey,const AXString &str);
    void putStrNo(int keyno,const AXString &str);
    void putStrArray(int keytop,AXString *pstrArray,int cnt);
    void putRectSize(LPCSTR szKey,const AXRectSize &rc);
    void putPoint(LPCSTR szKey,const AXPoint &pt);
    void putSize(LPCSTR szKey,const AXSize &size);
    void putArrayVal(LPCSTR szKey,const void *pVal,int cnt,int bytecnt);
    void putArrayHex(LPCSTR szKey,const LPDWORD pVal,int cnt);
    void putBase64(LPCSTR szKey,const void *pBuf,int size);
};

#endif
