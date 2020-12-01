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

#ifndef _AX_STRING_H
#define _AX_STRING_H

#include "AXDef.h"

class AXByteString;

class AXString
{
protected:
    LPUSTR  m_pBuf;
    int     m_nLen,
            m_nMallocLen;

protected:
    void _alloc(int len);
    int _calcAllocLen(int len);

public:
    ~AXString();

    AXString();
    AXString(int len);
    AXString(LPCSTR text);
    AXString(LPCUSTR pstr);

    operator LPCUSTR() const { return m_pBuf; }
    operator LPUSTR() const { return m_pBuf; }
    operator LPVOID() const { return (LPVOID)m_pBuf; }

    int getLen() const { return m_nLen; }
    LPUSTR at(int pos) const { return m_pBuf + pos; }
    int getMaxLen() const { return m_nMallocLen - 1; }
    int getMallocSize() const;
    UNICHAR getLastChar() const;

    BOOL resize(int len);
    void recalcLen();

    BOOL isEmpty() const;
    BOOL isNoEmpty() const;
    void empty();
    void clearZero();
    void end(int len);

    void operator =(char ch);
    void operator =(const AXString &str);
    void operator =(LPCSTR text);
    void operator =(LPCUSTR pBuf);
    void operator =(LPCWSTR pwstr);
    void operator +=(char ch);
    void operator +=(UNICHAR code);
    void operator +=(const AXString &str);
    void operator +=(LPCSTR text);
    void operator +=(LPCUSTR pBuf);
    void operator +=(int val);
    UNICHAR operator [](int pos) const;
    BOOL operator ==(LPCUSTR pstr) const;
    BOOL operator !=(LPCUSTR pstr) const;
    BOOL operator ==(LPCSTR text) const;
    BOOL operator !=(LPCSTR text) const;

    void appendAscii(LPCSTR text,int len);
    void prefix(LPCSTR text);
    void setAt(int pos,UNICHAR code);
    void set(LPCUSTR pBuf,int len);
    void setWide(LPCWSTR pw,int len);
    void setUTF8(LPCSTR text);
    void setUTF8(LPCSTR text,int len);
    void setLocal(LPCSTR text);
    void setLocal(LPCSTR text,int len);
    void setMid(const AXString &str,int pos,int len=-1);
    void setMidEnd(const AXString &str,int pos,UNICHAR code);
    void setInt(int val);
    void setIntFloat(int val,int dig);
    void setFileSize(LONGLONG size);
    void setSplitCharNULL(LPCUSTR pstr,UNICHAR code);

    int compare(LPCUSTR pstr) const;
    int compare(LPCSTR text,int len) const;
    int compareCase(LPCUSTR pstr) const;
    int compareCase(LPCSTR pstr) const;
    int compareCharEnd(LPCUSTR pstr,UNICHAR codeEnd) const;
    BOOL compareMatch(LPCUSTR pstr,BOOL bCase=FALSE) const;
    BOOL compareMatch(LPCSTR text,BOOL bCase=FALSE) const;
    BOOL compareEnd(const AXString &str,BOOL bCase) const;
    BOOL compareFilter(const AXString &filter,char cSplit,BOOL bCase) const;

    int find(UNICHAR code,int pos=0) const;
    int findRev(UNICHAR code) const;
    void findAndEnd(UNICHAR code);
    void replace(UNICHAR code,UNICHAR codeNew);
    void replace(LPCSTR szSrc,LPCSTR szDst);
    void replaceEscapeStr();

    BOOL getSplit(LPINT pLeft,AXString *pRight,UNICHAR code) const;
    BOOL getSplit(AXString *pLeft,AXString *pRight,UNICHAR code) const;
    BOOL getSplitPosStr(AXString *pdst,int pos,UNICHAR code) const;

    void toLower();
    void toUpper();
    int toInt() const;
    int toIntSearch() const;
    double toDouble() const;
    LPSTR toAscii(AXByteString *pstr) const;
    LPSTR toUTF8(AXByteString *pstr) const;
    LPSTR toLocal(AXByteString *pstr) const;
    void toLocal(AXByteString *pstr,int maxlen) const;
    void toWide(LPWSTR pbuf) const;
    void toBuf(LPUSTR pbuf,int maxlen) const;

    void path_setExePath();
    void path_setHomePath();
    void path_removeFileName();
    void path_removeFileName(const AXString &str);
    void path_removeEndSlash();
    void path_filename(const AXString &str);
    void path_filenameNoExt(const AXString &str);
    void path_ext(const AXString &str);
    void path_add(LPCSTR szPath);
    void path_add(const AXString &str);
    void path_setExt(LPCSTR szExt);
    void path_changeExt(LPCSTR szExt);
    BOOL path_compareExt(const AXString &exts) const;

    void setSplitText(int pos,LPCUSTR pText,UNICHAR code);
    void removeNonVisibleChar();
    void removeEndChar();
};

#endif
