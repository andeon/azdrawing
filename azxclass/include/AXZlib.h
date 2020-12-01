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

#ifndef _AX_ZLIB_H
#define _AX_ZLIB_H

#include "AXMem.h"

class AXFile;

class AXZlib
{
protected:
    LPVOID  m_pz;
    AXMem   m_memBuf;
    AXFile  *m_pfile;
    int     m_nType;
    DWORD   m_dwSize;

protected:
    enum TYPE
    {
        TYPE_NONE,
        TYPE_ENCODE_FILE,
        TYPE_DECODE_FILE,
        TYPE_ENCODE_BUF,
        TYPE_DECODE_BUF
    };

    BOOL _init(int type,int nLevel);
    void _end_zlib();

public:
    virtual ~AXZlib();
    AXZlib();

    DWORD getEncSize() const { return m_dwSize; }
    LPVOID getBuf() const { return (LPVOID)m_memBuf; }

    BOOL allocBuf(int bufsize);

    BOOL end();
    BOOL reset();

    //

    BOOL initEncFile(AXFile *pfile,int nLevel);
    BOOL putEncFile(const void *pBuf,DWORD size);
    BOOL endEncFile();

    BOOL initEncBuf(int nLevel);
    BOOL putEncBuf(void *pDstBuf,DWORD dwDstSize,void *pSrcBuf,DWORD dwSrcSize);

    //

    BOOL initDecFile(AXFile *pfile,DWORD dwSize);
    BOOL getDecFile(void *pBuf,DWORD size);

    BOOL initDecBuf();
    BOOL getDecBuf(void *pDstBuf,DWORD dwDstSize,void *pSrcBuf,DWORD dwSrcSize);
};

#endif
