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

#include "AXBMPSave.h"

#include "AXUtil.h"


#define FILEHEADERSIZE  14
#define INFOHEADERSIZE  40


/*!
    @class AXBMPSave
    @brief ビットマップ画像保存クラス

    - 現在は 24bit カラーの出力のみ対応。
    - openFile() でファイルを開き、Y1列ずつ出力、close() で閉じる。
    - ビットマップはボトムアップなので、Yの一番下から上に向かって渡していく。

    @ingroup format
*/

/*!
    @var AXBMPSave::INFO::nBits
    @brief ビット数（現在は24ビットで固定される）
    @var AXBMPSave::INFO::nResoH
    @brief 水平解像度（px/meter）。96dpiで3780。0でもOK
    @var AXBMPSave::INFO::nResoV
    @brief 垂直解像度
*/


//! 閉じる

void AXBMPSave::close()
{
    m_file.close();

    m_memLine.free();
}

//! ファイルを開く

BOOL AXBMPSave::openFile(const AXString &filename,AXBMPSave::INFO *pInfo)
{
    close();

    //

    m_pInfo = pInfo;
    m_pInfo->nBits = 24;

    //Y1列のサイズ

    m_nPitch = (((pInfo->nWidth * pInfo->nBits + 7) / 8) + 3) & (~3);

    //Y1列分確保

    if(!m_memLine.allocClear(m_nPitch)) return FALSE;

    //開く

    if(!m_file.openWrite(filename)) return FALSE;

    //ファイルヘッダ

    _fileheader();

    //情報ヘッダ

    _infoheader();

    return TRUE;
}

//! ラインバッファからY1列出力

void AXBMPSave::putLine()
{
    m_file.write(m_memLine, m_nPitch);
}

//! Y1列を32bitイメージから出力

void AXBMPSave::putLineFrom32bit(LPDWORD pSrc)
{
    int i;
    LPBYTE pd = m_memLine;
    DWORD col;

    //変換

    for(i = m_pInfo->nWidth; i > 0; i--, pd += 3)
    {
        col = *(pSrc++);

        pd[0] = (BYTE)col;
        pd[1] = (BYTE)(col >> 8);
        pd[2] = (BYTE)(col >> 16);
    }

    //書き込み

    m_file.write(m_memLine, m_nPitch);
}


//============================
//
//============================


//! ファイルヘッダ

void AXBMPSave::_fileheader()
{
    BYTE dat[FILEHEADERSIZE];
    DWORD size;

    //タイプ

    dat[0] = 'B', dat[1] = 'M';

    //ファイルサイズ

    size = FILEHEADERSIZE + INFOHEADERSIZE;
    size += m_nPitch * m_pInfo->nHeight;

    AXSetDWORDLE(dat + 2, size);

    //予約

    AXSetDWORDLE(dat + 6, 0);

    //イメージの位置

    AXSetDWORDLE(dat + 10, FILEHEADERSIZE + INFOHEADERSIZE);

    //

    m_file.write(dat, FILEHEADERSIZE);
}

//! 情報ヘッダ

void AXBMPSave::_infoheader()
{
    BYTE dat[INFOHEADERSIZE];

    ::memset(dat, 0, INFOHEADERSIZE);

    //サイズ

    AXSetDWORDLE(dat, INFOHEADERSIZE);

    //幅・高さ

    AXSetDWORDLE(dat + 4, m_pInfo->nWidth);
    AXSetDWORDLE(dat + 8, m_pInfo->nHeight);

    //plane(2)

    AXSetWORDLE(dat + 12, 1);

    //ビット数(2)

    AXSetWORDLE(dat + 14, m_pInfo->nBits);

    //圧縮形式(4)

    //画像データサイズ(4)

    AXSetDWORDLE(dat + 20, m_nPitch * m_pInfo->nHeight);

    //解像度

    AXSetDWORDLE(dat + 24, m_pInfo->nResoH);
    AXSetDWORDLE(dat + 28, m_pInfo->nResoV);

    //パレット数(4)、重要パレット数(4)

    //

    m_file.write(dat, INFOHEADERSIZE);
}
