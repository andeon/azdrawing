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

#include "AXGSUBVertTbl.h"

#include "AXMem.h"
#include "AXBuf.h"


/*
    GSUBテーブルは、グリフ置き換えの情報を定義するテーブル。
    ScriptList、FeatureList、LookupList の３つがある
*/


//***************************************
// AXParseGSUB - GSUB解析クラス
//***************************************


class AXParseGSUB
{
protected:
    AXBuf       m_buf;
    AXMemAuto   m_mem;

    int     m_nFeaturePos,
            m_nLookupPos;

protected:
    void _parse();
    void _parseLookup(int index);
    void _parseLookupTable(int offset);
    void _parseLookupSubTable(int offset);
    void _parseCoverage(int offset);

public:
    BOOL parseGSUB(LPBYTE pBuf);

    LPBYTE getBuf() { return (LPBYTE)m_mem; }
    DWORD getSize() { return m_mem.getNowSize(); }
};


//***************************************
// AXGSUBVertTbl
//***************************************


/*!
    @class AXGSUBVertTbl
    @brief フォントのGSUBデータの縦書き用グリフ置き換え

    @ingroup etc
*/


AXGSUBVertTbl::AXGSUBVertTbl()
{
    m_pBuf = NULL;
}

AXGSUBVertTbl::~AXGSUBVertTbl()
{
    free();
}

//! 解放

void AXGSUBVertTbl::free()
{
    if(m_pBuf)
    {
        ::free(m_pBuf);
        m_pBuf = NULL;
    }
}

//! GSUBデータ読み込み

BOOL AXGSUBVertTbl::loadGSUB(LPBYTE pGSUB)
{
    AXParseGSUB parse;

    free();

    if(!parse.parseGSUB(pGSUB))
        return FALSE;
    else
    {
        //コピー

        m_pBuf = (LPBYTE)::malloc(parse.getSize());
        if(!m_pBuf) return FALSE;

        ::memcpy(m_pBuf, parse.getBuf(), parse.getSize());

        return TRUE;
    }
}

//! 縦書き用グリフに置き換え
/*!
    [freetype] FT_Get_Char_Index() で取得したグリフ番号を指定する。

    @return 縦書き用のグリフがあれば置き換わった番号が返る。なければそのまま返る。
*/

UINT AXGSUBVertTbl::getGlyphIndex(UINT glyph)
{
    LPBYTE p = m_pBuf;
    LPWORD pw;
    int type,cnt,i,cindex;

    if(!m_pBuf) return glyph;

    while(1)
    {
        //Coverage
        //（指定グリフがテーブル上に存在するか検索）

        type = *(p++);
        if(type == 255) break;

        cnt = *((LPWORD)p);
        p += 2;

        pw      = (LPWORD)p;
        cindex  = -1;

        switch(type)
        {
            case 1:
                for(i = 0; i < cnt; i++, pw++)
                {
                    if(*pw == glyph) { cindex = i; break; }
                }

                p += cnt * 2;
                break;
            case 2:
                for(i = 0; i < cnt; i++, pw += 3)
                {
                    if(pw[0] + pw[2] <= glyph && glyph <= pw[2] + pw[1])
                    {
                        cindex = glyph - pw[0] + pw[2];
                        break;
                    }
                }

                p += cnt * 3 * 2;
                break;
        }

        //グリフ番号取得

        type = *(p++);

        if(cindex >= 0)
        {
            //見つかった

            if(type == 1)
                return glyph + *((short *)p);
            else
            {
                cnt = *((LPWORD)p);
                pw  = (LPWORD)(p + 2);

                if(cindex < cnt)
                    return pw[cindex];
                else
                    return glyph;
            }
        }
        else
        {
            //次へ

            if(type == 1)
                p += 2;
            else
            {
                cnt = *((LPWORD)p);
                p += 2 + cnt * 2;
            }
        }
    }

    return glyph;
}


//***************************************
// AXParseGSUB - GSUB解析クラス
//***************************************


//! 解析開始

BOOL AXParseGSUB::parseGSUB(LPBYTE pBuf)
{
    if(!pBuf) return FALSE;

    m_buf.init(pBuf, 0x1000000, AXBuf::ENDIAN_BIG);

    //GSUBバージョン

    if(m_buf.getDWORD() != 0x00010000) return FALSE;

    //各情報の位置

    m_buf.seek(2);  //ScriptList位置

    m_nFeaturePos = m_buf.getWORD();
    m_nLookupPos  = m_buf.getWORD();

    //メモリ確保

    if(!m_mem.alloc(1024, 1024)) return FALSE;

    //解析処理

    _parse();

    return (m_mem.getNowSize() > 1);
}

//! 解析処理

void AXParseGSUB::_parse()
{
    int i,j,offset,bkpos;
    DWORD tag;

    m_buf.setPos(m_nFeaturePos);

    //データ

    for(i = m_buf.getWORD(); i > 0; i--)
    {
        m_buf.getDWORD(&tag);       //識別タグ
        offset = m_buf.getWORD();   //レコードのオフセット

        //"vert" か "vrt2" でない場合は除く

        if(tag != 0x76657274 && tag != 0x76727432)
            continue;

        //----------

        bkpos = m_buf.setPosRetBk(m_nFeaturePos + offset);

        //レコードデータ
        /*
            WORD param     : 予約
            WORD indexcnt  : LookupListインデックスの数
            WORD[indexcnt] : LookupListのインデックス番号
        */

        m_buf.seek(2);

        for(j = m_buf.getWORD(); j > 0; j--)
            _parseLookup(m_buf.getWORD());

        //

        m_buf.setPos(bkpos);
    }

    //終了

    m_mem.addBYTE(255);
}

//! LookupListを解析

void AXParseGSUB::_parseLookup(int index)
{
    int cnt,bkpos;

    bkpos = m_buf.setPosRetBk(m_nLookupPos);

    //数

    cnt = m_buf.getWORD();

    //WORDのオフセット値が数分並んでいる

    if(index < cnt)
    {
        m_buf.seek(index * 2);

        //Lookupテーブル解析

        _parseLookupTable(m_nLookupPos + m_buf.getWORD());
    }

    //

    m_buf.setPos(bkpos);
}

//! Lookupテーブル解析

void AXParseGSUB::_parseLookupTable(int offset)
{
    int i,bkpos;

    bkpos = m_buf.setPosRetBk(offset);

    /*
        WORD type   : 1で単独置き換え
        WORD flag   : フラグ
        WORD subcnt : サブテーブル数
    */

    if(m_buf.getWORD() == 1)
    {
        //サブテーブル

        m_buf.seek(2);

        for(i = m_buf.getWORD(); i > 0; i--)
            _parseLookupSubTable(offset + m_buf.getWORD());
    }

    //

    m_buf.setPos(bkpos);
}

//! Lookupサブテーブル解析

void AXParseGSUB::_parseLookupSubTable(int offset)
{
    int format,pos,cnt,i,bkpos;
    short delta;

    bkpos = m_buf.setPosRetBk(offset);

    //共通データ

    format  = m_buf.getWORD();
    pos     = m_buf.getWORD();

    //

    if(format == 1 || format == 2)
    {
        //Coverage解析

        _parseCoverage(offset + pos);

        //データセット

        m_mem.addBYTE(format);

        if(format == 1)
        {
            //format[1] : 元のグリフ値に固定の差分値を加算する

            m_buf.getWORD(&delta);

            m_mem.addDat(&delta, 2);
        }
        else
        {
            //format[2] : グリフ値指定

            cnt = m_buf.getWORD();

            m_mem.addWORD(cnt);

            for(i = cnt; i > 0; i--)
                m_mem.addWORD(m_buf.getWORD());
        }
    }

    //

    m_buf.setPos(bkpos);
}

//! Coverage解析

void AXParseGSUB::_parseCoverage(int offset)
{
    int format,cnt,i,bkpos;

    bkpos = m_buf.setPosRetBk(offset);

    format  = m_buf.getWORD();
    cnt     = m_buf.getWORD();

    if(format == 1 || format == 2)
    {
        //format[1] : グリフ番号の配列
        //format[2] : (開始番号、終了番号、先頭番号)の３つのデータの配列

        m_mem.addBYTE(format);
        m_mem.addWORD(cnt);

        if(format == 2) cnt *= 3;

        for(i = cnt; i > 0; i--)
            m_mem.addWORD(m_buf.getWORD());
    }

    m_buf.setPos(bkpos);
}

