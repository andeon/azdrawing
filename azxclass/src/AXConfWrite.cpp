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

#include "AXConfWrite.h"

#include "AXString.h"
#include "AXRect.h"
#include "AXMem.h"
#include "AXUtil.h"
#include "AXUtilStr.h"


//----------------------------

/*!
    @class AXConfWrite
    @brief 構成ファイル書き込みクラス（UTF8）

    - グループ名・キー名は読み込み時、大/小文字を区別しない。

    @ingroup etc
*/


//! ホームディレクトリ＋szPath のファイルを開く

BOOL AXConfWrite::openHome(LPCSTR szPath)
{
    AXString str;

    str.path_setHomePath();
    str.path_add(szPath);

    return open(str);
}


//==============================
//出力サブ
//==============================


//! m_str の内容を出力

void AXConfWrite::_put()
{
    put((LPSTR)m_str, m_str.getLen());
}

//! 値出力

void AXConfWrite::_putVal(LPCSTR szKey,LPCSTR szVal)
{
    m_str = szKey;
    m_str += '=';
    m_str += szVal;
    m_str += '\n';

    _put();
}

//! 値出力（番号をキー名に）

void AXConfWrite::_putVal(int keyno,LPCSTR szVal)
{
    char m[16];

    AXIntToStr(m, keyno);

    m_str = m;
    m_str += '=';
    m_str += szVal;
    m_str += '\n';

    _put();
}


//==============================
//出力
//==============================


//! グループ名出力

void AXConfWrite::putGroup(LPCSTR szName)
{
    m_str = '[';
    m_str += szName;
    m_str += "]\n";

    _put();
}

//! int値出力

void AXConfWrite::putInt(LPCSTR szKey,int val)
{
    char m[16];

    AXIntToStr(m, val);
    _putVal(szKey, m);
}

//! 16進数値出力

void AXConfWrite::putHex(LPCSTR szKey,DWORD val)
{
    char m[16];

    AXValToHexStr(m, val);
    _putVal(szKey, m);
}

//! 文字列出力

void AXConfWrite::putStr(LPCSTR szKey,LPCSTR szText)
{
    _putVal(szKey, szText);
}

//! 文字列出力

void AXConfWrite::putStr(LPCSTR szKey,const AXString &str)
{
    AXByteString strb;

    str.toUTF8(&strb);
    _putVal(szKey, strb);
}

//! 番号をキー名にして文字列出力

void AXConfWrite::putStrNo(int keyno,const AXString &str)
{
    AXByteString strb;

    str.toUTF8(&strb);
    _putVal(keyno, strb);
}

//! 番号をキー名として文字列配列出力
/*!
    文字列が空の場合は出力しない。

    @param keytop キー番号の先頭
*/

void AXConfWrite::putStrArray(int keytop,AXString *pstrArray,int cnt)
{
    int i;

    for(i = 0; i < cnt; i++)
    {
        if(pstrArray[i].isNoEmpty())
            putStrNo(keytop + i, pstrArray[i]);
    }
}

//! AXRectSize をカンマで区切って出力

void AXConfWrite::putRectSize(LPCSTR szKey,const AXRectSize &rc)
{
    m_str = szKey;
    m_str += '=';
    m_str += rc.x;
    m_str += ',';
    m_str += rc.y;
    m_str += ',';
    m_str += rc.w;
    m_str += ',';
    m_str += rc.h;
    m_str += '\n';

    _put();
}

//! AXPoint をカンマで区切って出力

void AXConfWrite::putPoint(LPCSTR szKey,const AXPoint &pt)
{
    m_str = szKey;
    m_str += '=';
    m_str += pt.x;
    m_str += ',';
    m_str += pt.y;
    m_str += '\n';

    _put();
}

//! AXSize をカンマで区切って出力

void AXConfWrite::putSize(LPCSTR szKey,const AXSize &size)
{
    m_str = szKey;
    m_str += '=';
    m_str += size.w;
    m_str += ',';
    m_str += size.h;
    m_str += '\n';

    _put();
}

//! 配列の値をカンマで区切って出力
/*!
    @param cnt 値の数
    @param bytecnt 値のバイト数（1,2,4）
*/

void AXConfWrite::putArrayVal(LPCSTR szKey,const void *pVal,int cnt,int bytecnt)
{
    LPBYTE p = (LPBYTE)pVal;
    int n;

    m_str = szKey;
    m_str += '=';

    for(; cnt > 0; cnt--)
    {
        switch(bytecnt)
        {
            case 1:
                n = *(p++);
                break;
            case 2:
                n = *((LPWORD)p);
                p += 2;
                break;
            default:
                n = *((LPINT)p);
                p += 4;
                break;
        }

        m_str += n;
        if(cnt > 1) m_str += ',';
    }

    m_str += '\n';

    _put();
}

//! DWORD値の配列をカンマで区切って出力

void AXConfWrite::putArrayHex(LPCSTR szKey,const LPDWORD pVal,int cnt)
{
    char m[16];
    LPDWORD p = pVal;

    m_str = szKey;
    m_str += '=';

    for(; cnt > 0; cnt--, p++)
    {
        AXValToHexStr(m, *p);

        m_str += m;
        if(cnt > 1) m_str += ',';
    }

    m_str += '\n';

    _put();
}

//! Base64でバイナリデータ出力
/*!
    key=size:base64
*/

void AXConfWrite::putBase64(LPCSTR szKey,const void *pBuf,int size)
{
    AXMem mem;
    int encsize;
    char enter = '\n';

    if(!pBuf || size == 0) return;

    //

    encsize = AXGetBase64EncSize(size);

    if(!mem.alloc(encsize)) return;

    AXEncodeBase64(mem, pBuf, size);

    //

    m_str = szKey;
    m_str += '=';
    m_str += size;
    m_str += ':';

    _put();

    put(mem, encsize);
    put(&enter, 1);
}
