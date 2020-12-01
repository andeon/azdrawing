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

#ifndef _AX_FILE_H
#define _AX_FILE_H

#include "AXDef.h"

class AXString;
class AXByteString;
class AXMem;

class AXFile
{
public:
    enum OPENFLAGS
    {
        OF_SYNC     = 1,
        OF_EXCL     = 2
    };

    enum ENDIAN
    {
        ENDIAN_SYSTEM,
        ENDIAN_LITTLE,
        ENDIAN_BIG
    };

protected:
    int     m_fd,
            m_endian;

public:
    virtual ~AXFile();
    AXFile();

    BOOL isOpen() const { return (m_fd != -1); }
    void setEndian(int type) { m_endian = type; }

    BOOL close();
    BOOL openWrite(const AXString &filename,UINT uFlags=0,int perm=0664);
    BOOL openRead(const AXString &filename);

    DWORD getPosition() const;
    LONGLONG getPositionLong() const;
    void seekTop(LONGLONG pos);
    void seekCur(LONGLONG pos);
    void seekEnd(LONGLONG pos);

    int read(LPVOID pBuf,DWORD size);
    BOOL readSize(LPVOID pBuf,DWORD size);
    int write(const void *pBuf,DWORD size);

    BOOL cutEnd(DWORD size);
    void flush();
    DWORD getSize();
    LONGLONG getSizeLong();

    BOOL readWORD(LPVOID pd);
    BOOL readDWORD(LPVOID pd);

    void writeWORD(const void *pval);
    void writeDWORD(const void *pval);
    void writeBYTE(BYTE val);
    void writeWORD(WORD val);
    void writeDWORD(DWORD val);
    void writeStr(LPCSTR text);
    void writeStr(const AXByteString &str);
    void writeStrUni(const AXString &str);
    void writeStrUTF8(const AXString &str);

    BOOL readCompare(LPCSTR szText);
    int readLine(AXString *pstr);
    int readStrLenAndUTF8(AXString *pstr);

    static BOOL readFile(const AXString &filename,void *pBuf,DWORD size);
    static BOOL readFileFull(const AXString &filename,AXMem *pmem,BOOL bAddNULL=FALSE);
    static BOOL readFileFull(const AXString &filename,void **ppBuf,LPDWORD pSize,BOOL bAddNULL=FALSE);
    static BOOL copyFile(const AXString &srcname,const AXString &dstname,DWORD dwBufSize=8192);
};

#endif
