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

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <wchar.h>
#include <unistd.h>

#include "AXString.h"
#include "AXByteString.h"
#include "AXUtilStr.h"

/*!
    @class AXString
    @brief 文字列クラス（UNICODE-16bit）

    @attention 文字列の長さが変わったら m_nLen の値を変更すること

    @ingroup etc
*/

/*
    m_nLen       : 現在の文字数（NULL文字は含まない）
    m_nMallocLen : 確保長さ（NULL含む）

    ※m_pBuf に直接文字をセットする場合、最後に NULL 文字を入れるのを忘れずに。
*/


//----------------------


AXString::~AXString()
{
    if(m_pBuf)
        ::free(m_pBuf);
}

//------------

AXString::AXString()
{
    _alloc(31);
}

//! 最大長さを指定して確保する
/*!
    @param len  確保する文字列の最大長さ
*/

AXString::AXString(int len)
{
    _alloc(len);
}

//! ASCII文字列をセット

AXString::AXString(LPCSTR text)
{
    _alloc(::strlen(text));

    *this = text;
}

//! UNICODE文字列をセット

AXString::AXString(LPCUSTR pstr)
{
    _alloc(AXUStrLen(pstr));

    *this = pstr;
}


//============================
//確保関連
//============================


//! 最大長さから確保長さ取得

int AXString::_calcAllocLen(int len)
{
    int ret;

    for(ret = 32; ret < (1 << 28); ret <<= 1)
    {
        if(len < ret) break;
    }

    return ret;
}

//! 初期バッファ確保

void AXString::_alloc(int len)
{
    m_nMallocLen = m_nLen = 0;

    //

    len = _calcAllocLen(len);

    m_pBuf = (LPUSTR)::malloc(len * sizeof(UNICHAR));
    if(!m_pBuf) return;

    m_pBuf[0]    = 0;
    m_nMallocLen = len;
}

//! バッファリサイズ

BOOL AXString::resize(int len)
{
    LPUSTR pNew;

    //拡張の必要なし

    if(m_nMallocLen > len) return TRUE;

    //拡張

    len = _calcAllocLen(len);

    pNew = (LPUSTR)::realloc(m_pBuf, len * sizeof(UNICHAR));
    if(!pNew) return FALSE;

    m_pBuf       = pNew;
    m_nMallocLen = len;

    return TRUE;
}

//! 長さ(m_nLen)再計算

void AXString::recalcLen()
{
    LPUSTR p = m_pBuf;

    for(m_nLen = 0; *p; p++, m_nLen++);
}


//============================
//
//============================


//! 確保されているサイズ（バイト数）取得

int AXString::getMallocSize() const
{
    return m_nMallocLen * sizeof(UNICHAR);
}

//! 最後の文字取得

UNICHAR AXString::getLastChar() const
{
    return (m_nLen)? m_pBuf[m_nLen - 1]: 0;
}

//! 空かどうか

BOOL AXString::isEmpty() const
{
    return (*m_pBuf == 0);
}

//! 空ではないか

BOOL AXString::isNoEmpty() const
{
    return (*m_pBuf != 0);
}

//! 空にする

void AXString::empty()
{
    *m_pBuf = 0;
    m_nLen  = 0;
}

//! 全体をゼロでクリアして空にする

void AXString::clearZero()
{
    ::memset(m_pBuf, 0, m_nMallocLen * sizeof(UNICHAR));
    m_nLen = 0;
}

//! 指定文字数で文字列を終了させる

void AXString::end(int len)
{
    if(len < m_nLen)
    {
        m_pBuf[len] = 0;
        m_nLen = len;
    }
}


//============================
//operator関連
//============================


//! ASCII1文字代入

void AXString::operator =(char ch)
{
    m_pBuf[0] = ch;
    m_pBuf[1] = 0;
    m_nLen = 1;
}

//! AXStringの文字列セット

void AXString::operator =(const AXString &str)
{
    int len = str.getLen();

    if(resize(len))
    {
        ::memcpy(m_pBuf, str, (len + 1) * sizeof(UNICHAR));
        m_nLen = len;
    }
}

//! ASCII文字列代入
//! @param text NULLで空にする

void AXString::operator =(LPCSTR text)
{
    if(!text)
        empty();
    else
    {
        LPCSTR ps;
        LPUSTR pd;
        int len;

        len = ::strlen(text);

        if(resize(len))
        {
            for(ps = text, pd = m_pBuf; *ps; ps++)
                *(pd++) = (*ps >= 0)? *ps: '?';

            *pd = 0;

            m_nLen = len;
        }
    }
}

//! Unicode文字列セット
//! @param pBuf NULLで空にする

void AXString::operator =(LPCUSTR pBuf)
{
    if(!pBuf)
        empty();
    else
    {
        int len = AXUStrLen(pBuf);

        if(resize(len))
        {
            ::memcpy(m_pBuf, pBuf, (len + 1) * sizeof(UNICHAR));
            m_nLen = len;
        }
    }
}

//! ワイド文字列セット

void AXString::operator =(LPCWSTR pwstr)
{
    setWide(pwstr, -1);
}

//------------------

//! ASCII 1文字追加

void AXString::operator +=(char ch)
{
    if(ch >= 0 && resize(m_nLen + 1))
    {
        m_pBuf[m_nLen]     = ch;
        m_pBuf[m_nLen + 1] = 0;

        m_nLen++;
    }
}

//! Unicode 1文字追加

void AXString::operator +=(UNICHAR code)
{
    if(resize(m_nLen + 1))
    {
        m_pBuf[m_nLen]     = code;
        m_pBuf[m_nLen + 1] = 0;

        m_nLen++;
    }
}

//! AXString文字列追加

void AXString::operator +=(const AXString &str)
{
    int len = str.getLen();

    if(resize(m_nLen + len))
    {
        ::memcpy(m_pBuf + m_nLen, str, (len + 1) * sizeof(UNICHAR));
        m_nLen += len;
    }
}

//! ASCII文字列追加

void AXString::operator +=(LPCSTR text)
{
    appendAscii(text, ::strlen(text));
}

//! Unicode文字列追加

void AXString::operator +=(LPCUSTR pBuf)
{
    int len = AXUStrLen(pBuf);

    if(resize(m_nLen + len))
    {
        ::memcpy(m_pBuf + m_nLen, pBuf, (len + 1) * sizeof(UNICHAR));
        m_nLen += len;
    }
}

//! int値を文字列にして追加

void AXString::operator +=(int val)
{
    char m[16];

    AXIntToStr(m, val);
    *this += m;
}

//------------------

//! 指定位置の1文字取得

UNICHAR AXString::operator [](int pos) const
{
    return (pos > m_nLen)? 0: m_pBuf[pos];
}

//! Unicode文字列と比較して同じか

BOOL AXString::operator ==(LPCUSTR pstr) const
{
    return (AXUStrCompare(m_pBuf, pstr) == 0);
}

//! Unicode文字列と比較して異なるか

BOOL AXString::operator !=(LPCUSTR pstr) const
{
    return (AXUStrCompare(m_pBuf, pstr) != 0);
}

//! ASCII文字列と比較して同じか

BOOL AXString::operator ==(LPCSTR text) const
{
    return (AXUStrCompare(m_pBuf, text) == 0);
}

//! ASCII文字列と比較して異なるか

BOOL AXString::operator !=(LPCSTR text) const
{
    return (AXUStrCompare(m_pBuf, text) != 0);
}


//============================
//文字列セット/追加
//============================


//! ASCII文字列を指定文字数追加

void AXString::appendAscii(LPCSTR text,int len)
{
    LPCSTR pc;
    LPUSTR pu;
    int cnt;

    if(len > 0 && resize(m_nLen + len))
    {
        for(pc = text, pu = m_pBuf + m_nLen, cnt = len; cnt; cnt--)
            *(pu++) = *(pc++);

        *pu = 0;

        m_nLen += len;
    }
}

//! ASCII文字列を先頭に追加

void AXString::prefix(LPCSTR text)
{
    int i,len = ::strlen(text);
    LPUSTR pu;

    if(len > 0 && resize(m_nLen + len))
    {
        ::memmove(m_pBuf + len, m_pBuf, (m_nLen + 1) * sizeof(UNICHAR));

        for(pu = m_pBuf, i = len; i; i--)
            *(pu++) = *(text++);

        m_nLen += len;
    }
}

//! 指定位置に文字をセット

void AXString::setAt(int pos,UNICHAR code)
{
    if(pos >= 0 && pos < m_nLen)
        m_pBuf[pos] = code;
}

//! Unicode文字列の指定文字数分をセット

void AXString::set(LPCUSTR pBuf,int len)
{
    if(resize(len))
    {
        ::memcpy(m_pBuf, pBuf, len * sizeof(UNICHAR));
        m_pBuf[len] = 0;

        m_nLen = len;
    }
}

//! ワイド文字列の指定文字数分をセット
/*!
    @param len 負の値で、NULLまで
*/

void AXString::setWide(LPCWSTR pw,int len)
{
    LPCWSTR ps = pw;
    LPUSTR pd;

    if(len < 0) len = ::wcslen(pw);

    if(resize(len))
    {
        m_nLen = len;

        for(pd = m_pBuf; len > 0; len--, ps++)
            *(pd++) = (*ps <= 0xffff)? (UNICHAR)*ps: '?';

        *pd = 0;
    }
}

//! UTF8文字列からセット

void AXString::setUTF8(LPCSTR text)
{
    LPSTR pc;
    UINT uc;

    empty();

    for(pc = (LPSTR)text; *pc; )
    {
        uc = AXUTF8toUnicodeChar(pc, &pc);

        if(uc <= 0xffff)
            *this += (UNICHAR)uc;
        else
            *this += '?';
    }
}

//! UTF8文字列からセット（長さ指定）

void AXString::setUTF8(LPCSTR text,int len)
{
    LPSTR pc;
    UINT uc;

    empty();

    for(pc = (LPSTR)text; *pc; )
    {
        if(pc - text >= len) break;

        uc = AXUTF8toUnicodeChar(pc, &pc);

        if(uc <= 0xffff)
            *this += (UNICHAR)uc;
        else
            *this += '?';
    }
}

//! ロケールの文字列からセット

void AXString::setLocal(LPCSTR text)
{
    setLocal(text, ::strlen(text));
}

//! ロケールの文字列からセット（長さ指定）

void AXString::setLocal(LPCSTR text,int len)
{
    WCHAR wc;
    LPCSTR pc = text;
    int ret;
    mbstate_t state;

    empty();

    ::memset(&state, 0, sizeof(mbstate_t));

    for(; len > 0 && *pc; )
    {
        ret = ::mbrtowc(&wc, pc, MB_CUR_MAX, &state);
        if(ret <= 0) break;

        if(wc <= 0xffff)
            *this += (UNICHAR)wc;

        len -= ret;
        pc += ret;
    }
}

//! strの指定位置から指定文字数分を取り出してセット
/*!
    @param len 取り出す文字数。負の値で pos 〜 最後まで
*/

void AXString::setMid(const AXString &str,int pos,int len)
{
    int cnt,maxlen;

    if(pos > str.getLen()) return;

    maxlen = str.getLen() - pos;

    if(len < 0)
        cnt = maxlen;
    else
    {
        cnt = len;
        if(cnt > maxlen) cnt = maxlen;
    }

    set(str.at(pos), cnt);
}

//! strのpos位置から指定文字が現れるまでの文字列をセット

void AXString::setMidEnd(const AXString &str,int pos,UNICHAR code)
{
    int ret;

    ret = str.find(code, pos);

    if(ret == -1)
        setMid(str, pos, -1);
    else
        setMid(str, pos, ret - pos);
}

//! int値を文字列にしてセット

void AXString::setInt(int val)
{
    char m[16];

    AXIntToStr(m, val);
    *this = m;
}

//! int値を小数点数値として文字列でセット
/*!
    val = 1350, dig = 2 の場合、13.50 となる。
    @param dig 小数点の桁数
*/

void AXString::setIntFloat(int val,int dig)
{
    char m[20];

    AXIntToFloatStr(m, val, dig);
    *this = m;
}

//! ファイルサイズを文字列にしてセット
/*!
    Byte/KB/MB/GB。@n
    負の値の場合も対応
*/

void AXString::setFileSize(LONGLONG size)
{
    LONGLONG abssize;
    char m[32];

    abssize = (size >= 0)? size: -size;

    if(abssize < 1024)
        ::sprintf(m, "%d Byte", (int)size);
    else if(abssize < 1024 * 1024)
        ::sprintf(m, "%.1f KB", (double)size / 1024.0);
    else if(abssize < 1024 * 1024 * 1024)
        ::sprintf(m, "%.1f MB", (double)size / (1024.0 * 1024.0));
    else
        ::sprintf(m, "%.1f GB", (double)size / (1024.0 * 1024.0 * 1024.0));

    *this = m;
}

//! 指定文字で区切られた文字列を、区切り文字->NULL文字に変換してセット（最後がNULL文字２つで終わるように）

void AXString::setSplitCharNULL(LPCUSTR pstr,UNICHAR code)
{
    *this = pstr;
    *this += code;
    replace(code, 0);
}


//============================
//比較
//============================


//! 比較
/*!
    @return [0]同じ文字列 [-1]this の方が小さい [1]this の方が大きい
*/

int AXString::compare(LPCUSTR pstr) const
{
    return AXUStrCompare(m_pBuf, pstr);
}

//! ASCII文字列と比較（文字数指定）

int AXString::compare(LPCSTR text,int len) const
{
    LPUSTR pu = m_pBuf;
    LPCSTR pc = text;

    for(; len > 0 && *pu && *pc && *pu == *pc; pu++, pc++, len--);

    if(len == 0) return 0;

    return (*pu < *pc)? -1: 1;
}

//! 大/小文字を比較せずに文字列比較

int AXString::compareCase(LPCUSTR pstr) const
{
    return AXUStrCompareCase(m_pBuf, pstr);
}

//! 大/小文字を比較せずに文字列比較（ASCII文字列）

int AXString::compareCase(LPCSTR pstr) const
{
    return AXUStrCompareCase(m_pBuf, pstr);
}

//! codeEnd の文字までを比較（双方とも）

int AXString::compareCharEnd(LPCUSTR pstr,UNICHAR codeEnd) const
{
    return AXUStrCompareCharEnd(m_pBuf, pstr, codeEnd);
}

//! ワイルドカード(*/?)あり比較
/*!
    @param bCase TRUEで大小文字比較しない、FALSEで大小文字比較する
*/

BOOL AXString::compareMatch(LPCUSTR pstr,BOOL bCase) const
{
    return AXUStrCompareMatch(m_pBuf, pstr, bCase);
}

//! ワイルドカードあり比較（ASCII文字列）

BOOL AXString::compareMatch(LPCSTR text,BOOL bCase) const
{
    AXString str(text);

    return AXUStrCompareMatch(m_pBuf, str, bCase);
}

//! 終端文字列を比較

BOOL AXString::compareEnd(const AXString &str,BOOL bCase) const
{
    int len;

    len = str.getLen();
    if(len > m_nLen) return FALSE;

    if(bCase)
        return (AXUStrCompareCase(m_pBuf + m_nLen - len, str) == 0);
    else
        return (AXUStrCompare(m_pBuf + m_nLen - len, str) == 0);
}

//! 指定文字で区切られたワイルドカード文字列で比較していずれかに一致するか
/*!
    (例) "*.png;*.bmp"
*/

BOOL AXString::compareFilter(const AXString &filter,char cSplit,BOOL bCase) const
{
    AXString str;
    LPUSTR p;

    str = filter;
    str += cSplit;
    str.replace(cSplit, 0);

    for(p = str; *p; p += AXUStrLen(p) + 1)
    {
        if(AXUStrCompareMatch(m_pBuf, p, bCase)) return TRUE;
    }

    return FALSE;
}


//============================
//検索/置き換え
//============================


//! 文字検索
/*!
    @param code 検索するUnicode文字
    @param pos  検索を開始する位置
    @return 見つかった場合、見つかった位置。-1で見つからなかった
*/

int AXString::find(UNICHAR code,int pos) const
{
    LPUSTR p;

    if(pos > m_nLen) return -1;

    for(p = m_pBuf + pos; *p; p++)
    {
        if(*p == code) return (p - m_pBuf);
    }

    return -1;
}

//! 後ろから文字検索

int AXString::findRev(UNICHAR code) const
{
    LPUSTR p;

    if(m_nLen == 0) return -1;

    p = m_pBuf + m_nLen - 1;

    while(1)
    {
        if(*p == code) return (p - m_pBuf);

        if(p == m_pBuf) break;
        p--;
    }

    return -1;
}

//! 指定文字を検索し、その位置を文字列の終わりにする

void AXString::findAndEnd(UNICHAR code)
{
    int pos = find(code);
    if(pos != -1) end(pos);
}

//! 指定文字置き換え

void AXString::replace(UNICHAR code,UNICHAR codeNew)
{
    LPUSTR p;

    for(p = m_pBuf; *p; p++)
    {
        if(*p == code) *p = codeNew;
    }
}

//! 文字列置き換え（ASCII文字列）

void AXString::replace(LPCSTR szSrc,LPCSTR szDst)
{
    AXString src(szSrc),dst(szDst),out;
    int len;
    LPUSTR p = m_pBuf;

    len = src.getLen();

    while(*p)
    {
        if(AXUStrCompare(p, src, len) == 0)
        {
            out += dst;
            p += len;
        }
        else
            out += *(p++);
    }

    *this = out;
}

//! '\\'に続く文字を実際のエスケープ文字に置き換え

void AXString::replaceEscapeStr()
{
    AXString out(m_nLen);
    LPUSTR p;
    UNICHAR c;

    for(p = m_pBuf; *p; p++)
    {
        if(*p != '\\')
            out += *p;
        else
        {
            switch(p[1])
            {
                case 't': c = '\t'; break;
                case 'n': c = '\n'; break;
                case '\\': c = '\\'; break;
                default: c = 0; break;
            }

            if(c == 0)
                out += '\\';
            else
            {
                out += c;
                p++;
            }
        }
    }

    *this = out;
}


//============================
//取得
//============================


//! codeの文字で分割し、左の数値と右の文字列を取得

BOOL AXString::getSplit(LPINT pLeft,AXString *pRight,UNICHAR code) const
{
    int pos;
    AXString str;

    pos = find(code);
    if(pos == -1) return FALSE;

    str.setMid(*this, 0, pos);
    *pLeft = str.toInt();

    *pRight = m_pBuf + pos + 1;

    return TRUE;
}

//! codeの文字で分割し、それぞれ文字列で取得
/*!
    code の文字がない場合、left = this, right = empty。
*/

BOOL AXString::getSplit(AXString *pLeft,AXString *pRight,UNICHAR code) const
{
    int pos;

    pos = find(code);
    if(pos == -1)
    {
        *pLeft = *this;
        pRight->empty();
        return FALSE;
    }

    pLeft->set(m_pBuf, pos);
    *pRight = m_pBuf + pos + 1;

    return TRUE;
}

//! codeで区切られた文字列の指定位置の文字列取得

BOOL AXString::getSplitPosStr(AXString *pdst,int pos,UNICHAR code) const
{
    int ret,index;

    pdst->empty();

    for(index = 0; pos > 0; pos--)
    {
        ret = find(code, index);
        if(ret == -1) return FALSE;

        index = ret + 1;
    }

    pdst->setMidEnd(*this, index, code);

    return TRUE;
}


//============================
//変換
//============================


//! 小文字に変換

void AXString::toLower()
{
    LPUSTR p;

    for(p = m_pBuf; *p; p++)
    {
        if(*p >= 'A' && *p <= 'Z') *p += 0x20;
    }
}

//! 大文字に変換

void AXString::toUpper()
{
    LPUSTR p;

    for(p = m_pBuf; *p; p++)
    {
        if(*p >= 'a' && *p <= 'z') *p -= 0x20;
    }
}

//! int値に変換（+,- 記号も有効）

int AXString::toInt() const
{
    return AXUStrToInt(m_pBuf);
}

//! 文字列中から数字部分を検索し、そこの数値を int として取得（+,- 記号は除く）

int AXString::toIntSearch() const
{
    LPUSTR p;

    //検索

    for(p = m_pBuf; *p && (*p < '0' || *p > '9'); p++);

    //変換

    return AXUStrToInt(p);
}

//! double値に変換

double AXString::toDouble() const
{
    return AXUStrToDouble(m_pBuf);
}

//! AXByteStringに、ASCII文字列として変換

LPSTR AXString::toAscii(AXByteString *pstr) const
{
    LPUSTR pu;
    LPSTR pc;

    if(pstr->resize(m_nLen))
    {
        for(pu = m_pBuf, pc = *pstr; *pu; pu++)
            *(pc++) = (*pu <= 0xff)? *pu: '?';

        *pc = 0;

        pstr->recalcLen();
    }

    return (LPSTR)*pstr;
}

//! AXByteStringに、UTF8文字列として変換

LPSTR AXString::toUTF8(AXByteString *pstr) const
{
    LPUSTR pu;
    char m[6];
    int len;

    pstr->empty();

    for(pu = m_pBuf; *pu; pu++)
    {
        len = AXUnicodeToUTF8Char(m, *pu);

        if(len >= 1 && len <= 3)
            pstr->append(m, len);
        else
            *pstr += '?';
    }

    return (LPSTR)*pstr;
}

//! AXByteStringに、ロケールの文字列として変換

LPSTR AXString::toLocal(AXByteString *pstr) const
{
    LPUSTR pu;
    char m[MB_CUR_MAX];
    mbstate_t state;
    int len;

    pstr->empty();

    ::memset(&state, 0, sizeof(mbstate_t));

    for(pu = m_pBuf; *pu; pu++)
    {
        len = ::wcrtomb(m, *pu, &state);

        if(len != -1)
            pstr->append(m, len);
        else
            *pstr += '?';
    }

    return (LPSTR)*pstr;
}

//! AXByteStringに、ロケールの文字列として変換（最大長さ指定）

void AXString::toLocal(AXByteString *pstr,int maxlen) const
{
    LPUSTR pu;
    char m[MB_CUR_MAX];
    mbstate_t state;
    int len;

    pstr->empty();

    ::memset(&state, 0, sizeof(mbstate_t));

    for(pu = m_pBuf; *pu; pu++)
    {
        len = ::wcrtomb(m, *pu, &state);

        if(len == -1)
        {
            m[0] = '?';
            len  = 1;
        }

        if(pstr->getLen() + len > maxlen) break;

        pstr->append(m, len);
    }
}

//! ワイド文字列に変換

void AXString::toWide(LPWSTR pbuf) const
{
    LPUSTR pu;

    for(pu = m_pBuf; *pu; pu++, pbuf++)
        *pbuf = *pu;

    *pbuf = 0;
}

//! バッファへコピー

void AXString::toBuf(LPUSTR pbuf,int maxlen) const
{
    int len;

    len = (maxlen >= m_nLen)? m_nLen: maxlen;

    ::memcpy(pbuf, m_pBuf, sizeof(UNICHAR) * len);
    pbuf[len] = 0;
}


//============================
//ファイルパス関連
//============================


//! 実行ファイルのパス（ディレクトリ）をセット

void AXString::path_setExePath()
{
    char m[32];
    LPSTR pbuf;

    //"/proc/[PID]/exe" のファイルがシンボリックリンクとなっている

    ::sprintf(m, "/proc/%d/exe", ::getpid());

    //リンク先パス取得

    pbuf = ::realpath(m, NULL);

    if(!pbuf)
        empty();
    else
    {
        setLocal(pbuf);

        ::free(pbuf);

        path_removeFileName();
    }
}

//! ホームディレクトリのパスをセット

void AXString::path_setHomePath()
{
    LPSTR p = ::getenv("HOME");

    if(p)
    {
        setLocal(p);
        path_removeEndSlash();
    }
    else
        empty();
}

//! ファイル名を除いてディレクトリ名のみにする（最後の'/'以降を取り除く）

void AXString::path_removeFileName()
{
    int pos = findRev('/');

    if(pos == -1)
        empty();
    else
        end(pos);
}

//! 指定パスからファイル名を除去して、ディレクトリ名のみセット

void AXString::path_removeFileName(const AXString &str)
{
    *this = str;
    path_removeFileName();
}

//! 末尾の'/'を除去（'/'のみの場合は除く）

void AXString::path_removeEndSlash()
{
    if(m_nLen > 1)
    {
        if(m_pBuf[m_nLen - 1] == '/')
            end(m_nLen - 1);
    }
}

//! ディレクトリ名を除いたファイル名を取得してセット

void AXString::path_filename(const AXString &str)
{
    int pos = str.findRev('/');

    if(pos == -1)
        *this = str;
    else
        setMid(str, pos + 1);
}

//! 拡張子を除いたファイル名のみ取得してセット

void AXString::path_filenameNoExt(const AXString &str)
{
    int pos;

    path_filename(str);

    //※先頭が"."の場合は除く

    pos = findRev('.');
    if(pos > 0) end(pos);
}

//! 拡張子を取得してセット

void AXString::path_ext(const AXString &str)
{
    int top,pos;

    top = str.findRev('/');
    if(top == -1) top = 0; else top++;

    pos = str.findRev('.');

    if(pos <= top)
        empty();
    else
        setMid(str, pos + 1);
}

//! 現在のパスに指定パスを追加（AXString）

void AXString::path_add(const AXString &str)
{
    if(m_nLen && m_pBuf[m_nLen - 1] != '/')
        *this += '/';

    *this += str;
}

//! 現在のパスに指定パスを追加（ASCII文字列）

void AXString::path_add(LPCSTR szPath)
{
    if(m_nLen && m_pBuf[m_nLen - 1] != '/')
        *this += '/';

    *this += szPath;
}

//! 拡張子をセットする（すでにあればそのまま、なければ追加）

void AXString::path_setExt(LPCSTR szExt)
{
    AXString str;

    str = '.';
    str += szExt;

    if(!compareEnd(str, TRUE))
        *this += str;
}

//! 拡張子を変更する（拡張子なければ追加）

void AXString::path_changeExt(LPCSTR szExt)
{
    int top,pos;

    top = findRev('/');
    if(top == -1) top = 0; else top++;

    pos = findRev('.');

    if(pos > top)
    {
        AXString strExt(szExt);

        //拡張子がすでに同じの場合は変更なし（大/小文字比較なし）

        if(AXUStrCompareCase(m_pBuf + pos + 1, strExt) == 0)
            return;

        end(pos);
    }

	*this += '.';
	*this += szExt;
}

//! 拡張子が、指定した拡張子のいずれかであるかどうか
/*!
    @param exts 拡張子のリスト（NULL文字で区切る。NULLが２つ続くと終了。カンマは含めない。大/小文字は比較しない）
*/

BOOL AXString::path_compareExt(const AXString &exts) const
{
    AXString ext;
    LPCUSTR p;

    ext.path_ext(*this);

    for(p = exts; *p; p += AXUStrLen(p) + 1)
    {
        if(AXUStrCompareCase(ext, p) == 0)
            return TRUE;
    }

    return FALSE;
}


//============================
//編集関連
//============================


//! 指定文字で区切られた文字列の指定位置のテキストを変更
//! @param pText 変更後のテキスト。NULLで空

void AXString::setSplitText(int pos,LPCUSTR pText,UNICHAR code)
{
    AXString src;
    int index,ret;

    src = *this;

    //置き換え位置

    for(index = 0; pos > 0; pos--)
    {
        ret = src.find(code, index);
        if(ret == -1)
        {
            ret = src.getLen();
            src += code;
        }

        index = ret + 1;
    }

    //置き換え位置より前のテキストをセット

    setMid(src, 0, index);

    //置き換え

    if(pText) *this += pText;
    *this += code;

    //残りのテキストセット

    ret = src.find(code, index);

    if(ret != -1)
        *this += src.at(ret + 1);
}

//! 非表示文字を削除

void AXString::removeNonVisibleChar()
{
    AXString str(m_nLen);
    LPUSTR p;

    for(p = m_pBuf; *p; p++)
    {
        if(*p < 0x20 || *p == 0x7f) continue;

        str += *p;
    }

    *this = str;
}

//! 最後の文字を削除

void AXString::removeEndChar()
{
    if(m_nLen >= 1)
    {
        m_pBuf[m_nLen - 1] = 0;
        m_nLen--;
    }
}
