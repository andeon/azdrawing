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

#include <string.h>
#include <stdlib.h>

#include "AXUtilStr.h"

#include "AXString.h"
#include "AXByteString.h"


//UTF8の先頭文字に対する文字のバイト数

const BYTE g_UTF8CharWidth[256] =
{
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
  3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,4,4,4,4,4,4,4,4,5,5,5,5,6,6,1,1
};

//-----------------------

/*!
    @defgroup AXUtilStr AXUtilStr
    文字列関連ユーティリティ

    @ingroup util

    @{
*/


//! 文字列の履歴リストに追加
/*!
    ※リスト内に同じ文字列があれば、それを一番上に持ってくる。
*/

void AXAddRecentString(AXString *pstrArray,int nArrayCnt,const AXString &strAdd)
{
    int i,top;
    AXString str;

    //strAdd が pstrArray 内のデータの場合正しく動作しないので、コピーする

    str = strAdd;

    //すでに存在する場合、その位置以降は入れ替えない

    top = nArrayCnt - 1;

    for(i = 0; i < nArrayCnt; i++)
    {
        if(pstrArray[i] == str)
        {
            top = i;
            break;
        }
    }

    //先頭を空ける

    for(i = top; i > 0; i--)
        pstrArray[i] = pstrArray[i - 1];

    //先頭にセット

    pstrArray[0] = str;
}

//! text/uri-list の文字列を変換（\\tで区切る）
/*!
    @param bLocalFile "file://" のファイルのみ取得、かつ "file://" を除去
*/

void AXURIListToString(AXString *pstr,LPCSTR szURI,BOOL bLocalFile)
{
    AXByteString str;
    LPCSTR p;
    BOOL bTop = TRUE,bAddTab = TRUE;
    char m[3] = {0,0,0};

    pstr->empty();

    for(p = szURI; *p; )
    {
        //先頭文字

        if(bTop)
        {
            bAddTab = TRUE;
            bTop    = FALSE;

            //"file://" のみ＆除去

            if(bLocalFile)
            {
                if(*p == 'f' && ::strncmp(p, "file://", 7) == 0)
                    p += 7;
                else
                {
                    //次行へ

                    for(; *p && *p != '\r' && *p != '\n'; p++);
                    bAddTab = FALSE;
                }
            }
        }

        //

        if(*p == '\r' || *p == '\n')
        {
            //改行（区切りを \t に）

            if(*p == '\r' && p[1] == '\n')
                p += 2;
            else
                p++;

            if(bAddTab) str += '\t';
            bTop = TRUE;
        }
        else if(*p == '%')
        {
            //%xx - 16進数値

            m[0] = p[1];
            m[1] = p[2];

            str += (char)::strtoul(m, NULL, 16);
            p += 3;
        }
        else
        {
            //ASCII文字

            str += *(p++);
        }
    }

    str += '\t';

    pstr->setLocal(str);
}


//================================
//UTF8
//================================


//! UTF8の1文字のバイト数取得

int AXGetUTF8CharWidth(LPCSTR szText)
{
    return g_UTF8CharWidth[*((LPBYTE)szText)];
}


//================================
//文字コード変換
//================================


//! Unicode -> UTF8 1文字変換
/*!
    @param szBuf UTF8文字列が返る（NULL含めて最大5バイト）
    @param code  変換元のUnicode文字
    @return UTF8文字バイト数（NULL文字含まない。0で変換なし）
*/

int AXUnicodeToUTF8Char(LPSTR szBuf,UINT code)
{
    int len,i,s,n;
    LPBYTE p;

    if(code < 0x80)
    {
        szBuf[0] = (char)code;
        szBuf[1] = 0;
        return 1;
    }
    else if(code <= 0x7ff)
        len = 2;
    else if(code <= 0xffff)
        len = 3;
    else if(code <= 0x1fffff)
        len = 4;
    else
    {
        *szBuf = 0;
        return 0;
    }

    //文字セット

    p = (LPBYTE)szBuf;
    s = len - 1;

    *p = (BYTE)(0xc0 | (code >> (s * 6)));
    for(i = 0, n = 0x20; i < s - 1; i++, n >>= 1) *p |= n;

    for(i = 0, p++, n = (s - 1) * 6; i < s; i++, p++, n -= 6)
        *p = (BYTE)(0x80 | ((code >> n) & 0x3f));

    *p = 0;

    return len;
}

//! UTF8 -> Unicode 1文字変換
/*!
    @param ppRet szText の次の文字の位置が返る
    @return Unicode文字。-1で変換できない文字
*/

UINT AXUTF8toUnicodeChar(LPCSTR szText,char **ppRet)
{
    LPBYTE p = (LPBYTE)szText;
    int w,n,s,i;
    UINT ret;

    w = AXGetUTF8CharWidth(szText);

    if(*p < 0x80)
        //1バイト文字
        ret = *p;
    else if(w <= 4 && (*p >= 0xc2 && *p <= 0xfd))
    {
        //2-4バイト

        n = w - 1;
        s = 6 - n;

        ret = (UINT)(*p & ((1 << s) - 1)) << (n * 6);

        for(i = 0, s = (n - 1) * 6, p++; i < n; i++, s -= 6, p++)
        {
            //2バイト目以降が範囲外の値の場合
            if(*p < 0x80 || *p >= 0xc0) { ret = (UINT)-1; w = 1 + i; break; }

            ret |= (UINT)(*p & 0x3f) << s;
        }
    }
    else
        //先頭バイトが範囲外。もしくは5,6バイト文字
        ret = (UINT)-1;

    if(ppRet) *ppRet = (LPSTR)szText + w;

    return ret;
}


//================================
//Unicode文字列
//================================


//! Unicode文字列の長さ取得

int AXUStrLen(LPCUSTR pBuf)
{
    int len;

    for(len = 0; *pBuf; pBuf++, len++);

    return len;
}

//! Unicode文字列を比較

int AXUStrCompare(LPCUSTR pBuf1,LPCUSTR pBuf2)
{
    for(; *pBuf1 && *pBuf2 && *pBuf1 == *pBuf2; pBuf1++, pBuf2++);

    if(*pBuf1 == 0 && *pBuf2 == 0) return 0;

    return (*pBuf1 < *pBuf2)? -1: 1;
}

//! Unicode文字列とASCII文字列を比較

int AXUStrCompare(LPCUSTR pBuf1,LPCSTR pBuf2)
{
    for(; *pBuf1 && *pBuf2 && *pBuf1 == *pBuf2; pBuf1++, pBuf2++);

    if(*pBuf1 == 0 && *pBuf2 == 0) return 0;

    return (*pBuf1 < *pBuf2)? -1: 1;
}

//! Unicode文字列を比較（比較文字数指定）

int AXUStrCompare(LPCUSTR pBuf1,LPCUSTR pBuf2,int len)
{
    for(; len && *pBuf1 && *pBuf2 && *pBuf1 == *pBuf2; pBuf1++, pBuf2++, len--);

    if(len == 0) return 0;

    return (*pBuf1 < *pBuf2)? -1: 1;
}

//! codeEnd の文字までを比較

int AXUStrCompareCharEnd(LPCUSTR pBuf1,LPCUSTR pBuf2,UNICHAR codeEnd)
{
    UNICHAR c1,c2;

    c1 = *pBuf1, c2 = *pBuf2;

    for(; c1 && c2 && c1 == c2; )
    {
        c1 = *(pBuf1++);
        c2 = *(pBuf2++);

        if(c1 == codeEnd) c1 = 0;
        if(c2 == codeEnd) c2 = 0;
    }

    if(c1 == 0 && c2 == 0) return 0;

    return (c1 < c2)? -1: 1;
}

//! Unicode文字列比較（大/小文字比較なし）

int AXUStrCompareCase(LPCUSTR pBuf1,LPCUSTR pBuf2)
{
    UNICHAR u1,u2;

    for(; *pBuf1 && *pBuf2; pBuf1++, pBuf2++)
    {
        u1 = *pBuf1;
        u2 = *pBuf2;

        if(u1 >= 'A' && u1 <= 'Z') u1 += 0x20;
        if(u2 >= 'A' && u2 <= 'Z') u2 += 0x20;

        if(u1 != u2) return (u1 < u2)? -1: 1;
    }

    if(*pBuf1 == 0 && *pBuf2 == 0) return 0;

    return (*pBuf1 < *pBuf2)? -1: 1;
}

//! Unicode文字列とASCII文字列を比較（大/小文字比較なし）

int AXUStrCompareCase(LPCUSTR pBuf1,LPCSTR pBuf2)
{
    UNICHAR u1,u2;

    for(; *pBuf1 && *pBuf2; pBuf1++, pBuf2++)
    {
        u1 = *pBuf1;
        u2 = *pBuf2;

        if(u1 >= 'A' && u1 <= 'Z') u1 += 0x20;
        if(u2 >= 'A' && u2 <= 'Z') u2 += 0x20;

        if(u1 != u2) return (u1 < u2)? -1: 1;
    }

    if(*pBuf1 == 0 && *pBuf2 == 0) return 0;

    return (*pBuf1 < *pBuf2)? -1: 1;
}

//! */? ありのUnicode文字列比較
/*!
    @param pSrc     比較元
    @param pPattern 比較パターン文字列
    @param bCase    TRUEで大小文字比較しない、FALSEで大小文字比較する
*/

BOOL AXUStrCompareMatch(LPCUSTR pSrc,LPCUSTR pPattern,BOOL bCase)
{
    UNICHAR u1,u2;

    while(*pSrc && *pPattern)
    {
        if(*pPattern == '*')
        {
            //最後の*なら以降すべてOK
            if(pPattern[1] == 0) return TRUE;

            for(pPattern++; *pSrc; pSrc++)
            {
                if(AXUStrCompareMatch(pSrc, pPattern, bCase)) return TRUE;
            }

            return FALSE;
        }
        else if(*pPattern == '?')
        {
            //?は無条件で1文字進める
            pSrc++;
            pPattern++;
        }
        else
        {
            //通常文字比較

            u1 = *(pSrc++);
            u2 = *(pPattern++);

            if(bCase)
            {
                if(u1 >= 'A' && u1 <= 'Z') u1 += 0x20;
                if(u2 >= 'A' && u2 <= 'Z') u2 += 0x20;
            }

            if(u1 != u2) return FALSE;
        }
    }

    //両方最後まで来たらOK
    return (*pSrc == 0 && *pPattern == 0);
}

//! Unicode文字列を数値に変換（+,- 記号も有効）

int AXUStrToInt(LPCUSTR pBuf)
{
    LPCUSTR p = pBuf;
    int val = 0;

    if(*p == '-' || *p == '+') p++;

    for( ; *p; p++)
    {
        if(*p < '0' || *p > '9') break;

        val *= 10;
        val += *p - '0';
    }

    if(*pBuf == '-') val = -val;

    return val;
}

//! Unicode 16進数文字列を数値に変換
/*!
    @param ppRet 次のポインタ位置が入る。NULL で使用しない
*/

DWORD AXUStrHexToVal(LPCUSTR pBuf,int maxlen,UNICHAR **ppRet)
{
    int len;
    DWORD val = 0,n;

    for(len = 0; *pBuf && len < maxlen; len++, pBuf++)
    {
        if(*pBuf >= '0' && *pBuf <= '9')
            n = *pBuf - '0';
        else if(*pBuf >= 'A' && *pBuf <= 'F')
            n = *pBuf - 'A' + 10;
        else if(*pBuf >= 'a' && *pBuf <= 'f')
            n = *pBuf - 'a' + 10;
        else
            break;

        val <<= 4;
        val |= n;
    }

    if(ppRet) *ppRet = (LPUSTR)pBuf;

    return val;
}

//! Unicode文字列をdouble値に変換

double AXUStrToDouble(LPCUSTR pBuf)
{
    LPCUSTR p = pBuf;
    double val = 0, mul = 1;
    BOOL under = FALSE;

    if(*p == '-' || *p == '+') p++;

    for(; *p; p++)
    {
        if(!under && *p == '.')
        {
            under = TRUE;
            continue;
        }
        else if(*p < '0' || *p > '9')
            break;

        if(under)
        {
            mul *= 0.1;
            val += (*p - '0') * mul;
        }
        else
        {
            val *= 10;
            val += *p - '0';
        }
    }

    if(*pBuf == '-') val = -val;

    return val;
}


//================================
//ASCII文字列
//================================


//! 文字列の並びを入れ替える

void AXStrRev(LPSTR szBuf,int len)
{
    int cnt;
    LPSTR p1,p2;
    char c;

    p1 = szBuf;
    p2 = szBuf + len - 1;

    for(cnt = len / 2; cnt; cnt--, p1++, p2--)
    {
        c = *p1; *p1 = *p2; *p2 = c;
    }
}

//! int値を文字列に変換
/*!
    @return 文字数（NULLを含まない）
*/

int AXIntToStr(LPSTR szBuf,int nVal)
{
    LPSTR pd = szBuf;
    int val,len = 0;

    val = nVal;

    if(val < 0)
    {
        val = -val;
        *(pd++) = '-';
        len = 1;
    }

    do
    {
        *(pd++) = '0' + val % 10;
        len++;

        val /= 10;
    }while(val);

    *pd = 0;

    if(nVal < 0)
        AXStrRev(szBuf + 1, len - 1);
    else
        AXStrRev(szBuf, len);

    return len;
}

//! int値を小数点文字列に変換
/*!
    @param dig 小数点桁数（0で通常のint数値）
*/

int AXIntToFloatStr(LPSTR szBuf,int val,int dig)
{
    int len = 0,div,i;

    for(i = 0, div = 1; i < dig; i++, div *= 10);

    //符号

    if(val < 0)
    {
        szBuf[len++] = '-';
        val = -val;
    }

    //整数部分

    len += AXIntToStr(szBuf + len, val / div);

    //小数点部分

    if(dig > 0)
    {
        szBuf[len++] = '.';
        len += AXIntToStrDig(szBuf + len, val % div, dig);
    }

    return len;
}

//! 桁数を指定してint値を文字列へ
/*!
    @param dig	桁数。正で0で埋める、負で空白で埋める
*/

int AXIntToStrDig(LPSTR szBuf,int val,int dig)
{
	int bSpace,len = 0,i,div,n,bStart=FALSE;
	LPSTR pd = szBuf;

	if(dig >= 0)
        bSpace = FALSE;
	else
        bSpace = TRUE, dig = -dig;

	if(val < 0)
	{
	    *(pd++) = '-';
	    len = 1;
        val = -val;
    }

	for(i = 0, div = 1; i < dig - 1; i++, div *= 10);

	for(i = 0; i < dig; i++, div /= 10, len++)
	{
		n = (val / div) % 10;
		if(!bStart && n) bStart = TRUE;

		*(pd++) = (bSpace && !bStart)? ' ': '0' + n;
	}

	*pd = 0;

	return len;
}

//! DWORD値を16進数文字列に変換

int AXValToHexStr(LPSTR szBuf,DWORD val)
{
    int len,sf,n,bStart = FALSE;

    for(sf = 28, len = 0; sf >= 0; sf -= 4)
    {
        n = (val >> sf) & 15;
        if((!bStart && n) || sf == 0) bStart = TRUE;

        if(bStart)
            szBuf[len++] = (n < 10)? '0' + n: 'a' + n - 10;
    }

    szBuf[len] = 0;

    return len;
}

//! DWORD値を16進数文字列に変換（桁数指定）

int AXValToHexStrDig(LPSTR szBuf,DWORD val,int dig)
{
    int len,sf,n;

    for(sf = 0, n = 1; n < dig; n++, sf += 4);

    for(len = 0; sf >= 0; sf -= 4)
    {
        n = (val >> sf) & 15;

        szBuf[len++] = (n < 10)? '0' + n: 'a' + n - 10;
    }

    szBuf[len] = 0;

    return len;
}

//@}
