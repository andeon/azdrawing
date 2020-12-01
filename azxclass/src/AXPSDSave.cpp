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

#include "AXPSDSave.h"

#include "AXRect.h"
#include "AXBuf.h"
#include "AXUtil.h"



/*!
    @class AXPSDSave
    @brief PSD (PhotoShopファイル) 保存クラス

    - 合成モードなどの数値は "AXPSDdef.h" で定義されている。
    - ver 2.5/3.0 フォーマットのみ。
    - ビッグエンディアン。
    - [ファイルヘッダ][カラーモードデータ][画像リソースデータ][レイヤデータ][一枚絵イメージデータ] の順。
    - レイヤイメージは RGBA のみ対応。
    - レイヤなしの場合は beginLayer(0) とする。endLayer() はあってもなくても良い。

    @ingroup format

    @code
openFile(...);

//画像リソースデータ（必須）

beginRes();
...          //writeRes_* を必要な分だけ
endRes();

//レイヤデータ

beginLayer(layercnt);

for(i = 0; i < layercnt; i++)
    writeLayerInfo(...);

beginLayerImageTop();

for(i = 0; i < layercnt; i++)
{
    if(beginLayerImageEach(&rcs))
    {
        for(ch = 0; ch < 4; ch++)
        {
            beginLayerImageCh();

            for(y = 0; y < rcs.h; y++)
            {
                ...
                writeLayerImageChLine();
            }

            endLayerImageCh();
        }
    }
}

endLayer();

//一枚絵イメージ（必須）

beginImage();

for(ch = 0; ch < 3; ch++)
{
    beginImageCh();

    for(y = 0; y < imgh; y++)
        writeImageChLine();

    endImageCh();
}

close();
@endcode
*/

/*!
    @var AXPSDSave::INFO::nImgCh
    @brief 一枚絵イメージのチャンネル数

    [1]グレイスケール [3]RGB [4]RGBA

    @var AXPSDSave::INFO::nBits
    @brief ビット数（1,8,16）

    1bit はレイヤなしの場合のみ。

    @var AXPSDSave::INFO::nColMode
    @brief カラーモード
*/
/*!
    @var AXPSDSave::LAYERINFO::nRight
    @brief 右端位置（nLeft == nRight で範囲なしで空の状態。幅 1 なら nLeft + 1 の値となる）
    @var AXPSDSave::LAYERINFO::btOpacity
    @brief 不透明度（0-255）
    @var AXPSDSave::LAYERINFO::bHide
    @brief TRUE でレイヤ非表示
    @var AXPSDSave::LAYERINFO::szName
    @brief レイヤ名（最大で255文字）。ポインタをセットする。
*/


//! ファイルを開く

BOOL AXPSDSave::openFile(const AXString &filename,INFO *pInfo)
{
    m_pinfo      = pInfo;
    m_nMaxWidth  = 0;
    m_nMaxHeight = 0;
    m_pLayerWork = NULL;

    //出力バッファ

    if(!m_memOut.alloc(16 * 1024)) return FALSE;

    //----------- ファイル

    if(!m_file.openWrite(filename)) return FALSE;

    m_file.setEndian(AXFile::ENDIAN_BIG);

    //ヘッダ

    _write_header();

    //カラーモードデータ（データサイズ:0 でなし）

    m_file.writeDWORD((DWORD)0);

    return TRUE;
}

//! 閉じる

void AXPSDSave::close()
{
    m_file.close();
}


//=============================
// 一枚絵イメージ
//=============================


//! 一枚絵イメージ書き込み開始
/*!
    レイヤデータ書き込み後。@n
    一枚絵は 1bit イメージの場合あり。
*/

BOOL AXPSDSave::beginImage()
{
    int i,pitch,linesize;

    m_dwFilePos = m_file.getPosition() + 2;

    //Y1列サイズ

    if(m_pinfo->nBits == 1)
        pitch = (m_pinfo->nWidth + 7) >> 3;
    else
        pitch = m_pinfo->nWidth;

    //Y1列イメージ確保

    if(!m_memLineSrc.alloc(pitch)) return FALSE;

    //ラインサイズメモリ確保

    linesize = m_pinfo->nHeight * sizeof(WORD);

    if(!m_memLineSize.allocClear(linesize)) return FALSE;

    //圧縮タイプ (2b)

    m_file.writeWORD(1);

    //ラインサイズ一時出力（チャンネル数分）

    for(i = m_pinfo->nImgCh; i > 0; i--)
        m_file.write(m_memLineSize, linesize);

    return TRUE;
}

//! 一枚絵イメージ、各チャンネル書き込み開始

void AXPSDSave::beginImageCh()
{
    m_nNowY     = 0;
    m_dwOutSize = 0;
}

//! 一枚絵イメージ、各チャンネルのY1列書き込み

void AXPSDSave::writeImageChLine()
{
    int size;

    if(m_nNowY >= m_pinfo->nHeight) return;

    //圧縮

    size = _encPackBits(m_memLineSrc.getSize());

    //ラインサイズ

    AXSetWORDBE((LPWORD)m_memLineSize + m_nNowY, size);

    //

    m_nNowY++;
}

//! 一枚絵イメージ、各チャンネル書きこみ終了

void AXPSDSave::endImageCh()
{
    //残り出力

    _writeOutBuf();

    //ラインサイズ

    m_file.seekTop(m_dwFilePos);
    m_file.write(m_memLineSize, m_memLineSize.getSize());
    m_file.seekEnd(0);

    //次のチャンネル位置

    m_dwFilePos += m_memLineSize.getSize();
}


//=============================
// レイヤデータ
//=============================
/*
    m_dwFilePos  : 全体データサイズのファイル位置
    m_pLayerWork : 現在のレイヤの作業用データ
*/


//! レイヤ書き込み開始
/*!
    @param nCnt レイヤ数（0 でレイヤなし）
*/

BOOL AXPSDSave::beginLayer(int nCnt)
{
    m_nLayerCnt = nCnt;
    m_dwFilePos = m_file.getPosition();

    if(nCnt == 0)
    {
        //------- レイヤなし時

        m_file.writeDWORD((DWORD)0);
    }
    else
    {
        //------ レイヤあり時

        BYTE dat[10];

        //作業用メモリ確保

        if(!m_memLayerInfo.alloc(nCnt * sizeof(LAYERWORKINFO))) return FALSE;

        m_pLayerWork = (LAYERWORKINFO *)m_memLayerInfo.getBuf();

        //[4b]全体サイズ、[4b]データサイズ(2byte単位)、[2b]レイヤ数

        AXMemZero(dat, 10);
        AXSetWORDBE(dat + 8, nCnt);

        m_file.write(dat, 10);
    }

    return TRUE;
}

//! レイヤ書き込み終了

void AXPSDSave::endLayer()
{
    DWORD size;

    //レイヤあり時

    if(m_nLayerCnt)
    {
        size = m_file.getPosition() - (m_dwFilePos + 4);

        //データが2Byte単位でないとき余白追加

        if(size & 1)
        {
            m_file.writeBYTE(0);
            size++;
        }

        //データサイズ書き込み

        m_file.seekTop(m_dwFilePos);
        m_file.writeDWORD(&size);
        m_file.writeDWORD(size - 4);
        m_file.seekEnd(0);
    }

    //解放

    m_pLayerWork = NULL;

    m_memLayerInfo.free();
}

//! レイヤ情報書き込み
/*!
    一番下層のレイヤを先頭にして、順に書きこんでいく。
*/

void AXPSDSave::writeLayerInfo(LAYERINFO *pInfo)
{
    AXBuf buf;
    int i;
    DWORD filepos;

    buf.init(m_memOut, 0, AXBuf::ENDIAN_BIG);

    filepos = m_file.getPosition();

    //作業用データ：位置と幅・高さ

    m_pLayerWork->nLeft   = pInfo->nLeft;
    m_pLayerWork->nTop    = pInfo->nTop;
    m_pLayerWork->nWidth  = pInfo->nRight - pInfo->nLeft;
    m_pLayerWork->nHeight = pInfo->nBottom - pInfo->nTop;

    //幅・高さ最大値

    if(m_pLayerWork->nWidth > m_nMaxWidth)   m_nMaxWidth = m_pLayerWork->nWidth;
    if(m_pLayerWork->nHeight > m_nMaxHeight) m_nMaxHeight = m_pLayerWork->nHeight;

    //-------------- 基本データ

    //範囲 (4b x 4)

    buf.setDWORD(&pInfo->nTop);
    buf.setDWORD(&pInfo->nLeft);
    buf.setDWORD(&pInfo->nBottom);
    buf.setDWORD(&pInfo->nRight);

    //チャンネル数 (2b)

    buf.setWORD(4);

    //チャンネルデータ (6b x チャンネル数)

    for(i = 0; i < 4; i++)
    {
        //ID (2b)

        buf.setWORD((i == 3)? 0xffff: i);

        //データサイズ (4b)
        //範囲なしの場合用に 2 を入れておく

        m_pLayerWork->dwChSizeFilePos[i] = filepos + buf.getPos();

        buf.setDWORD(2);
    }

    //"8BIM" (4b)

    buf.setStr("8BIM");

    //合成モード (4b)

    buf.setDWORD(&pInfo->dwBlend);

    //不透明度 (1b)

    buf.setDat(&pInfo->btOpacity, 1);

    //クリップ (1b) [0:base 1:non-base]

    buf.setBYTE(0);

    //フラグ (1b) [0bit:透明色保護 1bit:非表示]

    buf.setBYTE((pInfo->bHide)? 2: 0);

    //(1b)

    buf.setBYTE(0);

    //------------- 拡張データ
    /*
        [4b] 拡張データサイズ
        [4b] レイヤマスクデータサイズ, ...
        [4b] 合成データサイズ,...
        [パスカル文字列] レイヤ名 ※4byte単位(長さのバイト含む)
    */

    int len,len4;

    //レイヤ名長さ

    len = ::strlen(pInfo->szName);
    if(len > 255) len = 255;

    len4 = (len + 1 + 3) & (~3);    //長さのバイトも含め4Byte単位

    //全体サイズ (4b)

    buf.setDWORD(8 + len4);

    //レイヤマスクデータサイズ(4b)＋合成データサイズ(4b)

    buf.setZero(8);

    //レイヤ名

    buf.setBYTE(len);
    buf.setDat(pInfo->szName, len);
    buf.setZero(len4 - len - 1);    //4byte 単位の残りを 0 で埋める

    //---------- 書き込み

    m_file.write(m_memOut, buf.getPos());

    //------------ 次へ

    m_pLayerWork++;
}


//=============================
// レイヤイメージ
//=============================
/*
    ※ 幅・高さが 0 （レイヤが空状態）の場合があるので注意。

    m_nChNo       : 現在のチャンネル
    m_nNowY       : 現在のY位置
    m_dwOutSize   : 圧縮データ出力サイズ
    m_dwFilePosCh : チャンネル開始ファイル位置
*/


//! レイヤイメージ書き込み開始
/*!
    レイヤ情報書き込み後に一度だけ行う。

    @return 成功したか
*/

BOOL AXPSDSave::beginLayerImageTop()
{
    m_pLayerWork = NULL;

    //Y1列イメージバッファ

    if(!m_memLineSrc.alloc(m_nMaxWidth)) return FALSE;

    //PackBits ラインサイズ用バッファ

    if(!m_memLineSize.allocClear(m_nMaxHeight * sizeof(WORD))) return FALSE;

    return TRUE;
}

//! 各レイヤイメージ書き込み開始
/*!
    範囲が空の場合はチャンネルデータがまとめて書き込まれる。

    @param prcs  範囲が入る（NULL でなし）
    @return イメージ範囲があるかどうか（空でないか）
*/

BOOL AXPSDSave::beginLayerImageEach(AXRectSize *prcs)
{
    BOOL bEmpty;

    m_nChNo = 0;

    if(m_pLayerWork)
        m_pLayerWork++;
    else
        m_pLayerWork = (LAYERWORKINFO *)m_memLayerInfo.getBuf();

    //範囲

    if(prcs)
    {
        prcs->x = m_pLayerWork->nLeft;
        prcs->y = m_pLayerWork->nTop;
        prcs->w = m_pLayerWork->nWidth;
        prcs->h = m_pLayerWork->nHeight;
    }

    //空かどうか

    bEmpty = (m_pLayerWork->nWidth == 0 || m_pLayerWork->nHeight == 0);

    //範囲が空の場合、まとめて書き込み
    /* 無圧縮でデータなし x 4ch。
       チャンネルデータサイズはデフォルトで 2 を書き込んでいる */

    if(bEmpty)
    {
        AXMemZero(m_memOut, 4 * sizeof(WORD));
        m_file.write(m_memOut, 4 * sizeof(WORD));
    }

    return !bEmpty;
}


//==========================
// レイヤイメージチャンネル
//==========================


//! レイヤイメージの各チャンネル書き込み開始
/*!
    チャンネルは R,G,B,A 順に。
*/

void AXPSDSave::beginLayerImageCh()
{
    if(m_nChNo >= 4) return;

    m_nNowY     = 0;
    m_dwOutSize = 0;

    //チャンネル開始位置

    m_dwFilePosCh = m_file.getPosition();

    //圧縮フラグ (2b) [0:圧縮なし 1:PackBits]

    m_file.writeWORD(1);

    //ラインサイズ (2b x 高さ) 一時出力

    m_file.write(m_memLineSize, m_pLayerWork->nHeight * sizeof(WORD));
}

//! イメージ（チャンネル）のY1列を出力
/*!
    m_memLineSrc にデータを入れておく。@n
    y:0 から順に上から下へ行う。@n
    m_memOut には続けてデータが溜まっていく。
*/

void AXPSDSave::writeLayerImageChLine()
{
    int size;

    //圧縮

    size = _encPackBits(m_pLayerWork->nWidth);

    //ラインサイズ

    AXSetWORDBE((LPWORD)m_memLineSize + m_nNowY, size);

    //

    m_nNowY++;
}

//! レイヤイメージの各チャンネル書き込み終了

void AXPSDSave::endLayerImageCh()
{
    DWORD size;

    //圧縮の残り出力

    _writeOutBuf();

    //チャンネルデータサイズ

    size = m_file.getPosition() - m_dwFilePosCh;

    //レイヤ情報のチャンネルデータサイズ書き込み

    m_file.seekTop(m_pLayerWork->dwChSizeFilePos[m_nChNo]);
    m_file.writeDWORD(&size);

    //ラインサイズ再書き込み

    m_file.seekTop(m_dwFilePosCh + 2);
    m_file.write(m_memLineSize, m_pLayerWork->nHeight * sizeof(WORD));

    m_file.seekEnd(0);

    //

    m_nChNo++;
}


//=============================
// 画像リソースデータ
//=============================
/*
    ※各リソースデータは 2Byte 単位。
    （奇数の場合、データサイズはそのままで、データを 1Byte 追加）

    m_dwFilePos : 全体サイズファイル位置
*/


//! 画像リソースデータ書き込み開始

void AXPSDSave::beginRes()
{
    //サイズ一時出力

    m_dwFilePos = m_file.getPosition();

    m_file.writeDWORD((DWORD)0);
}

//! 画像リソースデータ書き込み終了

void AXPSDSave::endRes()
{
    DWORD size;

    size = m_file.getPosition() - (m_dwFilePos + 4);

    //データ無しの場合は データサイズ:0 で書き込み済みのため処理なし

    if(size)
    {
        //データサイズ書き込み

        m_file.seekTop(m_dwFilePos);
        m_file.writeDWORD(&size);
        m_file.seekEnd(0);
    }
}

//! 画像リソースデータ:解像度書き込み

void AXPSDSave::writeRes_resolution(int nDPIH,int nDPIV)
{
    BYTE dat[28] = {
        0x38,0x42,0x49,0x4d,    //'8BIM'
        0x03,0xed,              //ID (0x03ed)
        0x00,0x00,              //名前
        0x00,0x00,0x00,0x10,    //サイズ
        0x00,0x00,0x00,0x00, 0x00,0x01, 0x00,0x02, //H [解像度 16bit固定小数点](4b),[pixel per inch = 1](2b),[cm = 2](2b)
        0x00,0x00,0x00,0x00, 0x00,0x01, 0x00,0x02  //V
    };

    //固定小数点の整数部分のみセット

    AXSetWORDBE(dat + 12, nDPIH);
    AXSetWORDBE(dat + 20, nDPIV);

    m_file.write(dat, 28);
}

//! 画像リソースデータ:カレントレイヤ番号書き込み
/*!
    @param no  一番下層のレイヤが 0
*/

void AXPSDSave::writeRes_curlayer(int no)
{
    BYTE dat[14] = {
        0x38,0x42,0x49,0x4d,    //'8BIM'
        0x04,0x00,              //ID (0x0400)
        0x00,0x00,              //名前
        0x00,0x00,0x00,0x02,    //サイズ
        0x00,0x00
    };

    AXSetWORDBE(dat + 12, no);

    m_file.write(dat, 14);
}


//=============================
// サブ
//=============================


//! ヘッダ書き込み

void AXPSDSave::_write_header()
{
    AXBuf buf;

    buf.init(m_memOut, 0, AXBuf::ENDIAN_BIG);

    //識別子 (4b)
    buf.setStr("8BPS");

    //バージョン (2b)
    buf.setWORD(1);

    //予約 (6b)
    buf.setZero(6);

    //合成後イメージのチャンネル数 (2b)
    buf.setWORD(m_pinfo->nImgCh);

    //高さ (4b)
    buf.setDWORD(m_pinfo->nHeight);

    //幅 (4b)
    buf.setDWORD(m_pinfo->nWidth);

    //ビット数 (2b)
    buf.setWORD(m_pinfo->nBits);

    //カラーモード (2b);
    buf.setWORD(m_pinfo->nColMode);

    m_file.write(m_memOut, 26);
}


//=============================
// PackBits 圧縮
//=============================


//! PackBits 圧縮
/*!
    m_memLineSrc の length 分を m_memOut へ出力。@n
    バッファサイズを超えたらファイルに書き込み。

    @return 圧縮後のサイズ
*/

int AXPSDSave::_encPackBits(int length)
{
    LPBYTE p,pbk,pend;
    BYTE val;
    int size = 0;

    p    = m_memLineSrc;
    pend = p + length;

    while(p < pend)
    {
        if(p == pend - 1 || *p != p[1])
        {
            //非連続

            pbk = p;

            for(; p - pbk < 128; p++)
            {
                if(p == pend - 1)
                {
                    p++;
                    break;
                }
                else if(*p == p[1])
                    break;
            }

            size += _writeRLE(pbk, p - pbk, 0);
        }
        else
        {
            //連続

            val = *p;
            pbk = p;

            for(p += 2; p < pend && p - pbk < 128 && *p == val; p++);

            size += _writeRLE(pbk, p - pbk, 1);
        }
    }

    return size;
}

//! PackBits書き込み
/*!
    @return 出力されたサイズ
*/

int AXPSDSave::_writeRLE(LPBYTE pDat,int cnt,int type)
{
    char len;
    int datsize;
    LPBYTE pd;

    //長さ

    len = cnt - 1;
    if(type == 1) len = -len;

    //データのサイズ

    datsize = (type == 0)? cnt: 1;

    //バッファを越える場合、出力

    if(m_dwOutSize + 1 + datsize >= m_memOut.getSize())
        _writeOutBuf();

    //バッファに追加

    pd = (LPBYTE)m_memOut + m_dwOutSize;

    *((char *)pd) = len;

    ::memcpy(pd + 1, pDat, datsize);

    m_dwOutSize += datsize + 1;

    return datsize + 1;
}

//! m_memOut をファイルに出力

void AXPSDSave::_writeOutBuf()
{
    if(m_dwOutSize)
    {
        m_file.write(m_memOut, m_dwOutSize);
        m_dwOutSize = 0;
    }
}
