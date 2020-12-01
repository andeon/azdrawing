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

#include "AXBMPLoad.h"

#include "AXBuf.h"
#include "AXUtil.h"

#define BI_RGB          0
#define BI_RLE8         1
#define BI_RLE4         2
#define BI_BITFIELDS    3

#define FILEHEADERSIZE  14
#define INFOHEADERSIZE  40


//-------------------------

/*!
    @class AXBMPLoad
    @brief ビットマップ画像読み込みクラス

    - RLE圧縮には非対応。
    - パレットは上位バイトから *RGB 順。

    @ingroup format
*/

/*!
    @var AXBMPLoad::m_nPitch
    @brief Y1列のバイト数
    @var AXBMPLoad::m_nBits
    @brief ビット数（1,4,8,16,24,32）
    @var AXBMPLoad::m_nPalCnt
    @brief パレット数
    @var AXBMPLoad::m_nResoH
    @brief 水平解像度(px/meter)
    @var AXBMPLoad::m_nResoV
    @brief 垂直解像度(px/meter)
*/


//! 閉じる

void AXBMPLoad::close()
{
    m_src.close();

    m_memLine.free();
    m_memPalette.free();
}

//! ファイルを開く

BOOL AXBMPLoad::openFile(const AXString &filename)
{
    m_src.openFile(filename);

    return _load();
}

//! バッファを開く

BOOL AXBMPLoad::openBuf(LPVOID pBuf,DWORD dwSize)
{
    m_src.openBuf(pBuf, dwSize);

    return _load();
}

//! Y1列を読み込み
/*!
    @return Y位置（0〜）。-1 で失敗。
*/

int AXBMPLoad::readLine()
{
    if(m_nYCnt <= 0) return -1;

    if(!m_src.read(m_memLine, m_nPitch)) return -1;

    m_nYCnt--;

    return (m_bVRev)? m_nHeight - m_nYCnt - 1: m_nYCnt;
}

//! 現在のY列を32bitカラーに変換してセット

void AXBMPLoad::lineTo32bit(LPDWORD pDst)
{
    int x,n,shift,mask;
    DWORD c,col;
    LPBYTE ps;
    LPDWORD pPal;

    ps   = m_memLine;
    pPal = m_memPalette;

    if(m_nBits <= 8)
    {
        shift = 8 - m_nBits;
        mask  = (1 << m_nBits) - 1;
    }

    for(x = m_nWidth; x > 0; x--)
    {
        switch(m_nBits)
        {
            case 24:
                col = 0xff000000 | (ps[2] << 16) | (ps[1] << 8) | ps[0];
                ps += 3;
                break;
            case 32:
                c = ((DWORD)ps[3] << 24) | (ps[2] << 16) | (ps[1] << 8) | ps[0];
                ps += 4;

                col = (((c & m_maskR) >> m_nRShiftR) << 16) |
                      (((c & m_maskG) >> m_nGShiftR) << 8)  |
                      ((c & m_maskB) >> m_nBShiftR) | 0xff000000;
                break;
            case 16:
                c = (ps[1] << 8) | ps[0];
                ps += 2;

                col = (((c & m_maskR) >> m_nRShiftR) * 255 / (m_nRbit - 1)) << 16;
                col |= (((c & m_maskG) >> m_nGShiftR) * 255 / (m_nGbit - 1)) << 8;
                col |= ((c & m_maskB) >> m_nBShiftR) * 255 / (m_nBbit - 1);
                col |= 0xff000000;
                break;
            default:
                n = (*ps >> shift) & mask;
                if(n >= m_nPalCnt) n = 0;

                col = pPal[n] | 0xff000000;

                shift -= m_nBits;
                if(shift < 0)
                {
                    shift = 8 - m_nBits;
                    ps++;
                }
                break;
        }

        *(pDst++) = col;
    }
}


//==============================
//
//==============================


//! 最初の読み込み処理

BOOL AXBMPLoad::_load()
{
    DWORD dwOffset;

    //ヘッダ

    if(!_fileheader(&dwOffset)) return FALSE;

    //情報ヘッダ

    if(!_infoheader()) return FALSE;

    //パレット

    if(m_nBits <= 8)
    {
        if(!_palette()) return FALSE;
    }

    //イメージ

    m_src.setPos(dwOffset);

    //Y1列分確保

    if(!m_memLine.alloc(m_nPitch)) return FALSE;

    m_nYCnt = m_nHeight;

    return TRUE;
}

//! ファイルヘッダ

BOOL AXBMPLoad::_fileheader(LPDWORD pOffset)
{
    BYTE dat[FILEHEADERSIZE];

    if(!m_src.read(dat, FILEHEADERSIZE)) return FALSE;

    //"BM"

    if(dat[0] != 'B' || dat[1] != 'M') return FALSE;

    //全体のサイズ(4)・予約(2+2)

    //イメージの位置

    AXGetDWORDLE(dat + 10, pOffset);

    return TRUE;
}

//! 情報ヘッダ (BMPINFOHEADER)

BOOL AXBMPLoad::_infoheader()
{
    AXBuf buf;
    BYTE dat[INFOHEADERSIZE];
    DWORD dwSize,dwComp;
    WORD wd;

    if(!m_src.read(dat, INFOHEADERSIZE)) return FALSE;

    buf.init(dat, INFOHEADERSIZE, AXBuf::ENDIAN_LITTLE);

    //----------

    //サイズ

    buf.getDWORD(&dwSize);

    //幅・高さ

    buf.getDWORD(&m_nWidth);
    buf.getDWORD(&m_nHeight);

    if(m_nWidth <= 0 || m_nHeight == 0) return FALSE;

    if(m_nHeight > 0)
        m_bVRev = FALSE;
    else
    {
        m_bVRev   = TRUE;
        m_nHeight = -m_nHeight;
    }

    //plane

    buf.seek(2);

    //bits

    buf.getWORD(&wd);
    m_nBits = wd;

    if(wd != 1 && wd != 4 && wd != 8 && wd != 24 && wd != 16 && wd != 32)
        return FALSE;

    //圧縮形式

    buf.getDWORD(&dwComp);

    if(dwComp == BI_RLE4 || dwComp == BI_RLE8) return FALSE;    //RLEには非対応

    //イメージサイズ

    buf.seek(4);

    //解像度

    buf.getDWORD(&m_nResoH);
    buf.getDWORD(&m_nResoV);

    //パレット数

    buf.getDWORD(&m_nPalCnt);

    if(m_nBits >= 24)
        m_nPalCnt = 0;
    else
    {
        if(m_nPalCnt < 0) m_nPalCnt = 0;

        if(m_nPalCnt == 0)
            m_nPalCnt = 1 << m_nBits;
        else if(m_nPalCnt > (1 << m_nBits))
            m_nPalCnt = 1 << m_nBits;
    }

    //ビットフィールド

    if(m_nBits == 16 || m_nBits == 32)
    {
        if(!_bitfield(dwComp)) return FALSE;
    }

    //-----------

    //Y1列のバイト数

    m_nPitch = (((m_nWidth * m_nBits + 7) / 8) + 3) & (~3);

    //次の位置へ

    m_src.setPos(FILEHEADERSIZE + dwSize);

    return TRUE;
}

//! ビットフィールド

BOOL AXBMPLoad::_bitfield(DWORD dwComp)
{
    BYTE dat[12];

    if(dwComp == BI_BITFIELDS)
    {
        //指定あり

        if(!m_src.read(dat, 12)) return FALSE;

        AXGetDWORDLE(dat    , &m_maskR);
        AXGetDWORDLE(dat + 4, &m_maskG);
        AXGetDWORDLE(dat + 8, &m_maskB);
    }
    else
    {
        //指定なし・デフォルト

        if(m_nBits == 16)
        {
            m_maskR = 0x7c00;
            m_maskG = 0x03e0;
            m_maskB = 0x001f;
        }
        else
        {
            m_maskR = 0xff0000;
            m_maskG = 0x00ff00;
            m_maskB = 0x0000ff;
        }
    }

    //

    m_nRShiftR  = AXGetFirstOnBit(m_maskR);
    m_nGShiftR  = AXGetFirstOnBit(m_maskG);
    m_nBShiftR  = AXGetFirstOnBit(m_maskB);

    m_nRbit = 1 << AXGetFirstOffBit(m_maskR >> m_nRShiftR); if(m_nRbit <= 1) m_nRbit = 2;
    m_nGbit = 1 << AXGetFirstOffBit(m_maskG >> m_nGShiftR); if(m_nGbit <= 1) m_nGbit = 2;
    m_nBbit = 1 << AXGetFirstOffBit(m_maskB >> m_nBShiftR); if(m_nBbit <= 1) m_nBbit = 2;

    return TRUE;
}

//! パレット

BOOL AXBMPLoad::_palette()
{
    int i;
    LPBYTE p;

    //確保

    if(!m_memPalette.alloc(m_nPalCnt * 4)) return FALSE;

    //読み込み

    if(!m_src.read(m_memPalette, m_nPalCnt * 4)) return FALSE;

    //エンディアン調整（リトル -> システム）

    p = m_memPalette;

    for(i = m_nPalCnt; i > 0; i--, p += 4)
        *((LPDWORD)p) = (p[2] << 16) | (p[1] << 8) | p[0];

    return TRUE;
}
