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

#ifndef _AX_CONFREAD_H
#define _AX_CONFREAD_H

#include "AXDef.h"

class AXString;
class AXByteString;
class AXRectSize;
class AXMem;
class AXMemAuto;

class AXConfRead
{
protected:
    LPSTR   m_pBuf;
    char    **m_ppIndex;
    int     m_nIndexCnt,
            m_nGroupIndex,
            m_nNowIndex;

protected:
    BOOL _createIndex();
    int _getLineCnt();
    void _setIndex();
    LPSTR _searchKey(LPCSTR szKey);

public:
    ~AXConfRead();
    AXConfRead();

    void free();

    BOOL loadFile(const AXString &filename);
    BOOL loadFileHome(LPCSTR szPathAdd);
    BOOL loadBuf(LPVOID pBuf,DWORD dwSize);

    BOOL setGroup(LPCSTR szName);

    int getInt(LPCSTR szKey,int nDef);
    DWORD getHex(LPCSTR szKey,DWORD dwDef);
    void getStr(LPCSTR szKey,AXString *pstr,LPCSTR szDef=NULL);
    void getStr(LPCSTR szKey,AXByteString *pstr,LPCSTR szDef=NULL);
    void getStr(LPCSTR szKey,LPSTR szBuf,int bufsize,LPCSTR szDef=NULL);
    void getStrNo(int keyno,AXString *pstr,LPCSTR szDef=NULL);
    void getStrArray(int keytop,AXString *pstrArray,int maxcnt);
    int getArrayVal(LPCSTR szKey,LPVOID pBuf,int bufcnt,int bytecnt,BOOL bHex=FALSE);
    BOOL getArrayVal(LPCSTR szKey,AXMemAuto *pmem,int bytecnt,BOOL bHex=FALSE);
    void getRectSize(LPCSTR szKey,AXRectSize *prc,int nDefX,int nDefY,int nDefW,int nDefH);
    void getRectSize(LPCSTR szKey,AXRectSize *prc,int nDef);
    void getPoint(LPCSTR szKey,AXPoint *ppt,int nDef);
    void getSize(LPCSTR szKey,AXSize *psize,int defw,int defh);
    void getBase64(LPCSTR szKey,AXMem *pmem);

    BOOL getNextLine(AXByteString *pstr);
    BOOL getNextStr(int *pKey,AXByteString *pstr);
};

#endif
