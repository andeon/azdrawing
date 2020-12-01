/************************************************************************
*  Copyright (C) 2013-2015 Azel.
*
*  This file is part of AzDrawing.
*
*  AzDrawing is free software: you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation, either version 3 of the License, or
*  (at your option) any later version.
*
*  AzDrawing is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*************************************************************************/

#include "CKeyDat.h"

#include "AXFileWriteBuf.h"


/*!
    @class CKeyDat
    @brief キー設定データ

    - データが変更された時のみ保存する。
*/


CKeyDat::~CKeyDat()
{
    AXFree((void **)&m_pBuf);
}

CKeyDat::CKeyDat()
{
    m_pBuf    = NULL;
    m_nCnt    = 0;
    m_bChange = FALSE;
}

//! メモリ確保

BOOL CKeyDat::alloc(int cnt)
{
    AXFree((void **)&m_pBuf);
    m_nCnt = 0;

    if(cnt == 0) return FALSE;

    //確保

    m_pBuf = (LPDWORD)AXMalloc(cnt * sizeof(DWORD));
    if(!m_pBuf) return FALSE;

    m_nCnt    = cnt;
    m_bChange = TRUE;

    return TRUE;
}

//! コマンドIDからキー取得
/*!
    @return 0 で見つからなかった
*/

int CKeyDat::getKey(int id)
{
    LPDWORD p = m_pBuf;
    int i;

    for(i = m_nCnt; i > 0; i--, p++)
    {
        if((int)(*p >> 16) == id)
            return *p & 0xffff;
    }

    return 0;
}

//! キーからコマンドID取得
/*!
    @return -1 で見つからなかった
*/

int CKeyDat::getCmd(UINT key)
{
    LPDWORD p = m_pBuf;
    int i;

    for(i = m_nCnt; i > 0; i--, p++)
    {
        if((int)(*p & 0xffff) == key)
            return *p >> 16;
    }

    return -1;
}

//! キー＋操作時の検索用、キーからコマンドID取得

int CKeyDat::getCmdDraw(UINT key)
{
    LPDWORD p = m_pBuf;
    int i,cmd;

    for(i = m_nCnt; i > 0; i--, p++)
    {
        cmd = *p >> 16;

        /* キー＋操作は 2000 〜 2999 の範囲。
           この範囲はデータの先頭にあるので、この範囲以外のコマンドIDが来れば終了 */

        if(cmd < 2000 || cmd >= 3000)
            return -1;

        if((*p & 0xffff) == key)
            return cmd;
    }

    return -1;
}


//============================
//ファイル
//============================


//! 保存

void CKeyDat::saveFile(const AXString &filename)
{
    AXFileWriteBuf file;
    int i;

    //データの変更なし

    if(!m_bChange) return;

    //---------

    if(!file.open(filename)) return;

    //ヘッダ
    file.putStr("AZDWLKEY");
    //バージョン
    file.putBYTE(0);

    //キー数

    file.putDWORDLE(m_nCnt);

    //キーデータ

    for(i = 0; i < m_nCnt; i++)
        file.putDWORDLE(m_pBuf[i]);

    file.close();
}

//! 読み込み

void CKeyDat::loadFile(const AXString &filename)
{
    AXFile file;
    BYTE ver;
    int cnt;

    if(!file.openRead(filename)) return;

    file.setEndian(AXFile::ENDIAN_LITTLE);

    //ヘッダ

    if(!file.readCompare("AZDWLKEY")) return;

    //バージョン

    file.read(&ver, 1);
    if(ver != 0) return;

    //キー数

    if(!file.readDWORD(&cnt)) return;

    //確保

    if(!alloc(cnt)) return;

    m_bChange = FALSE;

    //キーデータ

    file.read(m_pBuf, cnt * sizeof(DWORD));

    //リトルエンディアンから変換

#ifdef __BIG_ENDIAN__

    LPBYTE p = (LPBYTE)m_pBuf;

    for(int i = 0; i < cnt; i++, p += 4)
        m_pBuf[i] = ((DWORD)p[3] << 24) | (p[2] << 16) | (p[1] << 8) | p[0];

#endif

    file.close();
}
