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

#include <unistd.h>
#include <stdlib.h>

#include "AXUtil.h"

#include "AXString.h"
#include "AXByteString.h"


/*!
    @defgroup axutil AXUtil
    @brief 通常ユーティリティ

    @ingroup util
    @{
*/


//! double値からint取得

int AXDoubleToInt(double val)
{
    if(val < 0)
        return (int)(val - 0.5);
    else
        return (int)(val + 0.5);
}

//! dot/meter を DPI に単位変換

int AXDPMtoDPI(int dpm)
{
    return (int)(dpm * 0.0254 + 0.5);
}

//! DPI を dot/meter に単位変換

int AXDPItoDPM(int dpi)
{
    return (int)(dpi / 0.0254 + 0.5);
}

//! バッファからリトルエンディアンでWORD値取得

void AXGetWORDLE(const LPBYTE pBuf,LPVOID pVal)
{
    *((LPWORD)pVal) = (pBuf[1] << 8) | pBuf[0];
}

//! バッファからリトルエンディアンでDWORD値取得

void AXGetDWORDLE(const LPBYTE pBuf,LPVOID pVal)
{
    *((LPDWORD)pVal) = ((DWORD)pBuf[3] << 24) | (pBuf[2] << 16) | (pBuf[1] << 8) | pBuf[0];
}

//! バッファからビッグエンディアンでWORD値取得

void AXGetWORDBE(const LPBYTE pBuf,LPVOID pVal)
{
    *((LPWORD)pVal) = (pBuf[0] << 8) | pBuf[1];
}

//! バッファからビッグエンディアンでDWORD値取得

void AXGetDWORDBE(const LPBYTE pBuf,LPVOID pVal)
{
    *((LPDWORD)pVal) = ((DWORD)pBuf[0] << 24) | (pBuf[1] << 16) | (pBuf[2] << 8) | pBuf[3];
}

//! バッファにWORD値をリトルエンディアンでセット

void AXSetWORDLE(LPVOID pBuf,WORD val)
{
    *((LPBYTE)pBuf)     = (BYTE)val;
    *((LPBYTE)pBuf + 1) = (BYTE)(val >> 8);
}

//! バッファにDWORD値をリトルエンディアンでセット

void AXSetDWORDLE(LPVOID pBuf,DWORD val)
{
    *((LPBYTE)pBuf)     = (BYTE)val;
    *((LPBYTE)pBuf + 1) = (BYTE)(val >> 8);
    *((LPBYTE)pBuf + 2) = (BYTE)(val >> 16);
    *((LPBYTE)pBuf + 3) = (BYTE)(val >> 24);
}

//! バッファにWORD値をビッグエンディアンでセット

void AXSetWORDBE(LPVOID pBuf,WORD val)
{
    *((LPBYTE)pBuf)     = (BYTE)(val >> 8);
    *((LPBYTE)pBuf + 1) = (BYTE)val;
}

//! バッファにDWORD値をビッグエンディアンでセット

void AXSetDWORDBE(LPVOID pBuf,DWORD val)
{
    *((LPBYTE)pBuf)     = (BYTE)(val >> 24);
    *((LPBYTE)pBuf + 1) = (BYTE)(val >> 16);
    *((LPBYTE)pBuf + 2) = (BYTE)(val >> 8);
    *((LPBYTE)pBuf + 3) = (BYTE)val;
}

//! ビットが最初に1になっている位置を取得

int AXGetFirstOnBit(DWORD val)
{
    int n;

    for(n = 0; n < 32; n++)
    {
        if(val & (1 << n)) break;
    }

    return n;
}

//! ビットが最初に0になっている位置を取得

int AXGetFirstOffBit(DWORD val)
{
    int n;

    for(n = 0; n < 32; n++)
    {
        if(!(val & (1 << n))) break;
    }

    return n;
}

//! 可変長数値をセット
/*!
    7bitがONで次のバイトに続く。最大5バイト。
*/

int AXSetVariableLenVal(LPVOID pBuf,UINT val)
{
    LPBYTE p;
    int len,i;

    //長さ取得

    if(val < (1 << 7)) len = 1;
    else if(val < (1 << 14)) len = 2;
    else if(val < (1 << 21)) len = 3;
    else if(val < (1 << 28)) len = 4;
    else len = 5;

    //後ろからセット

    p = (LPBYTE)pBuf + len - 1;

    for(i = 0; i < len; i++, p--, val >>= 7)
    {
        *p = val & 127;
        if(i) *p |= 0x80;
    }

    return len;
}

//! 可変長数値取得
/*!
    @param ppRet 進めた分のポインタが入る。NULL指定可。
*/

UINT AXGetVariableLenVal(const void *pBuf,void **ppRet)
{
    LPBYTE p = (LPBYTE)pBuf;
    UINT val = 0;
    BYTE bt;
    int i;

    for(i = 5; i > 0; i--)
    {
        bt = *(p++);

        val |= bt & 127;

        if(bt & 0x80)
            val <<= 7;
        else
            break;
    }

    if(ppRet) *ppRet = (void *)p;

    return val;
}

//! ブラウザを開く
/*!
    ローカルファイルの場合、先頭に "file://" を付ける。
*/

BOOL AXExeBrowser(const AXString &url)
{
    pid_t pid;
    AXByteString strb,strb2;

    url.toLocal(&strb2);

    strb = "/usr/bin/x-www-browser \"";
    strb += strb2;
    strb += '"';

    //プロセス複製

    pid = ::fork();
    if(pid < 0) return FALSE;

    if(pid == 0)
    {
        ::execl("/bin/sh", "sh", "-c", (LPSTR)strb, (char *)0);
        ::exit(-1);
    }

    return TRUE;
}


//==============================
// Base64
//==============================


//! Base64エンコード後のサイズ取得

int AXGetBase64EncSize(int size)
{
    return ((size + 2) / 3) * 4;
}

//! Base64エンコード
/*!
    NULL 文字はセットされない。
*/

void AXEncodeBase64(LPVOID pDst,const void *pSrc,int size)
{
    LPSTR pd;
    LPBYTE ps;
    DWORD val;
    int i,n,shift;
    char m[4];

    pd = (LPSTR)pDst;
    ps = (LPBYTE)pSrc;

    for(; size > 0; size -= 3, ps += 3, pd += 4)
    {
        //元データ 3Byte -> val

        if(size >= 3)
            val = (ps[0] << 16) | (ps[1] << 8) | ps[2];
        else if(size == 2)
            val = (ps[0] << 16) | (ps[1] << 8);
        else
            val = ps[0] << 16;

        //3Byte -> 4文字

        for(i = 0, shift = 18; i < 4; i++, shift -= 6)
        {
            n = (val >> shift) & 63;

            if(n < 26) n += 'A';
            else if(n < 52) n += 'a' - 26;
            else if(n < 62) n += '0' - 52;
            else if(n == 62) n = '+';
            else n = '/';

            m[i] = n;
        }

        //'='

        if(size == 1)
            m[2] = m[3] = '=';
        else if(size == 2)
            m[3] = '=';

        //dst

        pd[0] = m[0];
        pd[1] = m[1];
        pd[2] = m[2];
        pd[3] = m[3];
    }
}

//! Base64デコード
/*!
    @param bufsize pDst のバッファサイズ
    @return デコードされたデータサイズ。-1でエラー
*/

int AXDecodeBase64(LPVOID pDst,LPCSTR pSrc,int bufsize)
{
    LPBYTE pd;
    LPCSTR ps;
    DWORD val;
    int i,n,shift,cnt,dstsize = 0;

    pd = (LPBYTE)pDst;
    ps = pSrc;

    while(*ps)
    {
        //4文字 -> 24bit

        for(i = 0, val = 0, shift = 18; i < 4; i++, shift -= 6)
        {
            n = *(ps++);
            if(n == 0) return -1;

            if(n >= 'A' && n <= 'Z') n -= 'A';
            else if(n >= 'a' && n <= 'z') n = n - 'a' + 26;
            else if(n >= '0' && n <= '9') n = n - '0' + 52;
            else if(n == '+') n = 62;
            else if(n == '/') n = 63;
            else if(n == '=') n = 0;
            else
                return -1;

            val |= n << shift;
        }

        //サイズ
        /* 最後が ??== なら 1Byte。???= なら 2Byte */

        cnt = 3;

        if(*ps == 0 && ps[-1] == '=')
            cnt = (ps[-2] == '=')? 1: 2;

        //バッファサイズを超える

        if(dstsize + cnt > bufsize) return -1;

        //24bit -> 1〜3Byte

        *(pd++) = (BYTE)(val >> 16);
        if(cnt >= 2) *(pd++) = (BYTE)(val >> 8);
        if(cnt == 3) *(pd++) = (BYTE)val;

        dstsize += cnt;
    }

    return dstsize;
}

//@}
