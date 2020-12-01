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

#include "AXTarBall.h"

#include "AXFile.h"


/*!
    @class AXTarBall
    @brief tarアーカイブの操作クラス

    - アーカイブの内容は全てバッファに読み込まれる。
    - パス名は100文字以内であること。

    @ingroup format
*/


AXTarBall::AXTarBall()
{
    m_pBuf = NULL;
}

AXTarBall::~AXTarBall()
{
    free();
}

//! 解放

void AXTarBall::free()
{
    AXFree((void **)&m_pBuf);
}

//! ファイルからtarアーカイブ読み込み

BOOL AXTarBall::loadFile(const AXString &filename)
{
    free();

    return AXFile::readFileFull(filename, (void **)&m_pBuf, &m_dwSize, FALSE);
}

//! パス名から通常ファイルを検索
/*!
    @param pSize ファイルのデータサイズが返る
    @return 見つかった場合、ファイルデータのバッファ位置。見つからなかった場合、NULL。
*/

LPBYTE AXTarBall::findFile(LPCSTR szPath,DWORD *pSize) const
{
    LPBYTE p;
    LPCSTR p1,p2;
    DWORD pos,size;
    int len;

    if(!m_pBuf) return NULL;

    for(pos = 0; pos + 512 <= m_dwSize;)
    {
        p = m_pBuf + pos;

        //パス名がなしなら終了とする

        if(*p == 0) break;

        //ファイルサイズ

        size = _getOctVal(p + 124, 11);

        if(pos + 512 + size > m_dwSize) return NULL;

        //通常ファイルの時のみ比較

        if(*(p + 156) == '0')
        {
            //最大100文字を比較

            p1 = (LPCSTR)p;
            p2 = szPath;

            for(len = 0; *p1 && *p2 && *p1 == *p2 && len < 99; p1++, p2++, len++);

            if(*p1 == *p2)
            {
                *pSize = size;
                return p + 512;
            }
        }

        //次へ

        pos += 512 + ((size + 511) & (~511));
    }

    return NULL;
}

//! 8進数文字列から値取得

DWORD AXTarBall::_getOctVal(LPBYTE pbuf,int dig) const
{
    DWORD val = 0;
    int cnt = 0;

    for(; *pbuf && cnt < dig; pbuf++, cnt++)
    {
        val <<= 3;

        if(*pbuf >= '1' && *pbuf <= '7')
            val |= *pbuf - '0';
    }

    return val;
}
