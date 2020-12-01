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

#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>

#include "AXFile.h"

#include "AXString.h"
#include "AXByteString.h"
#include "AXMem.h"


//-------------------------

#define FILE_NONE   (-1)
#define ISOPEN()    (m_fd != -1)


/*!
    @class AXFile
    @brief ファイルクラス

    - setEndian() でファイルのエンディアンを指定して値を読み書きできる。

    @ingroup etc
*/

/*!
    @var AXFile::OF_SYNC
    @brief 同期する
    @var AXFile::OF_EXCL
    @brief ファイルが存在していた場合失敗
*/


AXFile::~AXFile()
{
    close();
}

AXFile::AXFile()
{
    m_fd     = FILE_NONE;
    m_endian = ENDIAN_SYSTEM;
}

//! 閉じる

BOOL AXFile::close()
{
    BOOL ret = TRUE;

    if(ISOPEN())
    {
		ret  = (::close(m_fd) == 0);
		m_fd = FILE_NONE;
    }

    return ret;
}

//! 書き込みオープン
/*!
    @param uFlags AXFile::OPENFLAGS
    @param perm パーミッション。8進数
*/

BOOL AXFile::openWrite(const AXString &filename,UINT uFlags,int perm)
{
    AXByteString str;
    int flag;

    if(ISOPEN()) close();

    flag = O_WRONLY | O_CREAT | O_TRUNC;
    if(uFlags & OF_SYNC) flag |= O_SYNC;
    if(uFlags & OF_EXCL) flag |= O_EXCL;

    filename.toLocal(&str);

    m_fd = ::open(str, flag, perm);

    return (m_fd != FILE_NONE);
}

//! 読み込みオープン

BOOL AXFile::openRead(const AXString &filename)
{
    AXByteString str;

    if(ISOPEN()) close();

    filename.toLocal(&str);

    m_fd = ::open(str, O_RDONLY);

    return (m_fd != FILE_NONE);
}

//------------------

//! 現在位置取得
//! @return 現在の位置。エラー時は-1

DWORD AXFile::getPosition() const
{
    return (DWORD)::lseek(m_fd, 0, SEEK_CUR);
}

//! 現在位置取得
//! @return 現在の位置。エラー時は-1

LONGLONG AXFile::getPositionLong() const
{
    return ::lseek(m_fd, 0, SEEK_CUR);
}

//! 現在位置移動

void AXFile::seekTop(LONGLONG pos)
{
    ::lseek(m_fd, pos, SEEK_SET);
}

void AXFile::seekCur(LONGLONG pos)
{
    ::lseek(m_fd, pos, SEEK_CUR);
}

void AXFile::seekEnd(LONGLONG pos)
{
    ::lseek(m_fd, pos, SEEK_END);
}

//------------------

//! 読み込み ※エンディアン関係なし
//! @return 読み込んだバイト数（負の値でエラー）

int AXFile::read(LPVOID pBuf,DWORD size)
{
    ssize_t ret;

    do
    {
        ret = ::read(m_fd, pBuf, size);
    } while(ret < 0 && errno == EINTR);

    return (int)ret;
}

//! 読み込み（指定サイズを必ず読み込む）
//! @return 指定したサイズ分読み込めたか

BOOL AXFile::readSize(LPVOID pBuf,DWORD size)
{
    ssize_t ret;
    DWORD rsize = 0;

    while(rsize < size)
    {
        ret = ::read(m_fd, (LPBYTE)pBuf + rsize, size - rsize);

        if(ret < 0)
        {
            if(errno == EINTR) continue;
            else break;
        }
        else
            rsize += ret;
    }

    return (rsize == size);
}

//! 書き込み ※エンディアン関係なし
//! @return 書き込んだバイト数（負の値でエラー）

int AXFile::write(const void *pBuf,DWORD size)
{
    ssize_t ret;

    do
    {
        ret = ::write(m_fd, pBuf, size);
    } while(ret < 0 && errno == EINTR);

    return (int)ret;
}

//! （書き込み時）指定サイズに切り詰める

BOOL AXFile::cutEnd(DWORD size)
{
    return (::ftruncate(m_fd, size) != -1);
}

//! 書き込みフラッシュ

void AXFile::flush()
{
    ::fsync(m_fd);
}

//! サイズ取得

DWORD AXFile::getSize()
{
    struct stat dat;

    if(::fstat(m_fd, &dat) == 0)
        return (DWORD)dat.st_size;
    else
        return 0;
}

//! サイズ取得

LONGLONG AXFile::getSizeLong()
{
    struct stat dat;

    if(::fstat(m_fd, &dat) == 0)
        return dat.st_size;
    else
        return 0;
}


//=====================================
//読み込み
//=====================================


//! 2バイト値読み込み

BOOL AXFile::readWORD(LPVOID pd)
{
    BYTE b[2];

    switch(m_endian)
    {
        case ENDIAN_SYSTEM:
            return (read(pd, 2) == 2);
            break;
        case ENDIAN_LITTLE:
            if(read(b, 2) != 2) return FALSE;
            *((LPWORD)pd) = ((WORD)b[1] << 8) | b[0];
            break;
        case ENDIAN_BIG:
            if(read(b, 2) != 2) return FALSE;
            *((LPWORD)pd) = ((WORD)b[0] << 8) | b[1];
            break;
    }

    return TRUE;
}

//! 4バイト値読み込み

BOOL AXFile::readDWORD(LPVOID pd)
{
    BYTE b[4];

    switch(m_endian)
    {
        case ENDIAN_SYSTEM:
            return (read(pd, 4) == 4);
            break;
        case ENDIAN_LITTLE:
            if(read(b, 4) != 4) return FALSE;
            *((LPDWORD)pd) = ((DWORD)b[3] << 24) | (b[2] << 16) | (b[1] << 8) | b[0];
            break;
        case ENDIAN_BIG:
            if(read(b, 4) != 4) return FALSE;
            *((LPDWORD)pd) = ((DWORD)b[0] << 24) | (b[1] << 16) | (b[2] << 8) | b[3];
            break;
    }

    return TRUE;
}


//=====================================
//書き込み
//=====================================


//! 2バイト書き込み

void AXFile::writeWORD(const void *pval)
{
    WORD wd = *((LPWORD)pval);
    BYTE b[2];

    switch(m_endian)
    {
        case ENDIAN_SYSTEM:
            write(pval, 2);
            break;
        case ENDIAN_LITTLE:
            b[0] = wd & 0xff;
            b[1] = wd >> 8;
            write(b, 2);
            break;
        case ENDIAN_BIG:
            b[0] = wd >> 8;
            b[1] = wd & 0xff;
            write(b, 2);
            break;
    }
}

//! 4バイト書き込み

void AXFile::writeDWORD(const void *pval)
{
    DWORD wd = *((LPDWORD)pval);
    BYTE b[4];

    switch(m_endian)
    {
        case ENDIAN_SYSTEM:
            write(pval, 4);
            break;
        case ENDIAN_LITTLE:
            b[0] = wd & 0xff;
            b[1] = (wd >> 8) & 0xff;
            b[2] = (wd >> 16) & 0xff;
            b[3] = wd >> 24;
            write(b, 4);
            break;
        case ENDIAN_BIG:
            b[0] = wd >> 24;
            b[1] = (wd >> 16) & 0xff;
            b[2] = (wd >> 8) & 0xff;
            b[3] = wd & 0xff;
            write(b, 4);
            break;
    }
}

//! 1バイト値書き込み

void AXFile::writeBYTE(BYTE val)
{
    write(&val, 1);
}

//! 2バイト値書き込み

void AXFile::writeWORD(WORD val)
{
    writeWORD(&val);
}

//! 4バイト値書き込み

void AXFile::writeDWORD(DWORD val)
{
    writeDWORD(&val);
}

//! ASCII文字列書き込み

void AXFile::writeStr(LPCSTR text)
{
    write(text, ::strlen(text));
}

//! マルチバイト文字列書き込み

void AXFile::writeStr(const AXByteString &str)
{
    write((LPCSTR)str, str.getLen());
}

//! UNICODE(16bit)文字列書き込み

void AXFile::writeStrUni(const AXString &str)
{
    write((LPCUSTR)str, str.getLen() * sizeof(UNICHAR));
}

//! UTF8文字列書き込み

void AXFile::writeStrUTF8(const AXString &str)
{
    AXByteString str2;

    str.toUTF8(&str2);

    write((LPCSTR)str2, str2.getLen());
}


//===============================
//読み込み特殊
//===============================


//! 指定文字列分を読み込んで、文字列比較(ASCII)

BOOL AXFile::readCompare(LPCSTR szText)
{
    int len;
    LPSTR pbuf;
    BOOL ret = FALSE;

    len = ::strlen(szText);

    pbuf = new char[len];

    if(read(pbuf, len) == len)
    {
        if(::strncmp(pbuf, szText, len) == 0) ret = TRUE;
    }

    delete []pbuf;

    return ret;
}

//! 1行分の文字列読み込み (ASCII/UTF8)
/*!
    @param pstr 文字列が入る。改行文字は含まない。
    @return 読み込んだバイト数（改行文字も含む）
*/

int AXFile::readLine(AXString *pstr)
{
    char c;
    int cnt = 0;

    pstr->empty();

    while(1)
    {
        if(read(&c, 1) != 1) break;
        cnt++;

        if(c == '\n') break;

        if(c == '\r')
        {
            if(read(&c, 1) == 1)
            {
                //"\r\n" でなければ1文字戻る
                if(c == '\n')
                    cnt++;
                else
                    seekCur(-1);
            }
            break;
        }

        *pstr += c;
    }

    return cnt;
}

//! 長さ（可変）+UTF8 から文字列読み込み
/*!
    @return 読み込んだサイズ
*/

int AXFile::readStrLenAndUTF8(AXString *pstr)
{
    int len,shift,size = 0;
    BYTE bt;
    char *pbuf;

    //長さ

    for(len = 0, shift = 0; 1; shift += 7)
    {
        if(read(&bt, 1) == 0) return FALSE;
        size++;

        len |= (int)(bt & 0x7f) << shift;

        if(bt < 128) break;
    }

    //UTF8

    if(len == 0)
        pstr->empty();
    else
    {
		pbuf = new char[len];

		read(pbuf, len);

		pstr->setUTF8(pbuf, len);

		delete []pbuf;
    }

    return size + len;
}


//===============================
// 関数
//===============================


//! ファイルの先頭から指定サイズ分読み込み

BOOL AXFile::readFile(const AXString &filename,void *pBuf,DWORD size)
{
    AXFile file;

    if(!file.openRead(filename)) return FALSE;

    if(!file.readSize(pBuf, size)) return FALSE;

    file.close();

    return TRUE;
}

//! ファイルの内容すべて AXMem に読み込み
/*!
    @param bAddNULL 最後に1バイトのNULL文字を追加
*/

BOOL AXFile::readFileFull(const AXString &filename,AXMem *pmem,BOOL bAddNULL)
{
    AXFile file;
    DWORD size;

    if(!file.openRead(filename)) return FALSE;

    size = file.getSize();
    if(size == 0) return FALSE;

    if(!pmem->alloc(bAddNULL? size + 1: size)) return FALSE;

    if(!file.readSize(*pmem, size)) return FALSE;

    if(bAddNULL)
        *((LPBYTE)*pmem + size) = 0;

    file.close();

    return TRUE;
}

//! ファイルの内容すべて読み込み（malloc で確保）
/*!
    @param pSize ファイルサイズが返る。NULL指定可。
*/

BOOL AXFile::readFileFull(const AXString &filename,void **ppBuf,LPDWORD pSize,BOOL bAddNULL)
{
    AXFile file;
    DWORD size;
    LPVOID pbuf;

    if(!file.openRead(filename)) return FALSE;

    //サイズ取得

    size = file.getSize();
    if(size == 0) return FALSE;

    //確保

    pbuf = AXMalloc(size + (bAddNULL? 1: 0));
    if(!pbuf) return FALSE;

    //読込

    if(!file.readSize(pbuf, size))
    {
        AXFreeNormal(pbuf);
        return FALSE;
    }

    //NULL追加

    if(bAddNULL)
        *((LPBYTE)pbuf + size) = 0;

    file.close();

    //

    *ppBuf = pbuf;
    if(pSize) *pSize = size;

    return TRUE;
}

//! ファイルコピー

BOOL AXFile::copyFile(const AXString &srcname,const AXString &dstname,DWORD dwBufSize)
{
    AXFile fileSrc,fileDst;
    AXMem mem;
    int size,outsize;
    LPBYTE pbuf;

    if(!mem.alloc(dwBufSize)) return FALSE;

    pbuf = mem;

    //

    if(!fileSrc.openRead(srcname)) return FALSE;
    if(!fileDst.openWrite(dstname)) return FALSE;

    while(1)
    {
        size = fileSrc.read(pbuf, dwBufSize);
        if(size < 0) return FALSE;
        if(size == 0) break;

        outsize = fileDst.write(pbuf, size);
        if(outsize != size) return FALSE;
    }

    fileDst.close();
    fileSrc.close();

    return TRUE;
}
