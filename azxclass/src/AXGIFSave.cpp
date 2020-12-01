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

#include "AXGIFSave.h"

#include "AXUtil.h"


//*******************************
// AXGIFSave_LZW クラス
//*******************************


class AXGIFSave_LZW
{
    AXFile  *m_pFile;

    LPBYTE  m_pBuf,     //圧縮データ用バッファ
            m_pNowBlock;
    LPDWORD m_pHashTbl; //LZW辞書テーブル

    int     m_clearcode,
            m_endcode,
            m_runcode,
            m_runbits,
            m_depth,
            m_maxcode,
            m_shift,
            m_bufpos,
            m_blockpos;
    DWORD   m_nowVal;


    BOOL init(int bits);
    int searchHash(DWORD key);
    void putCode(int code);
    void putByte(int ch);

public:
    BOOL encode(AXFile *pFile,int bits,LPBYTE pSrcBuf,int nSrcBufSize);
};


//*******************************
// AXGIFSave
//*******************************


/*!
    @class AXGIFSave
    @brief GIF画像保存クラス

    @ingroup format
*/

/*!
    @enum AXGIFSave::DISPOSALTYPE
    @brief 画像の破棄方法

    @var AXGIFSave::DISPOSAL_NONE
    @brief 指定しない
    @var AXGIFSave::DISPOSAL_STAY
    @brief そのまま画像を残す
    @var AXGIFSave::DISPOSAL_FILLBK
    @brief 背景色で塗りつぶす
    @var AXGIFSave::DISPOSAL_BACKIMG
    @brief 前の画像に戻す
*/

/*!
    @var AXGIFSave::GLOBALINFO::wWidth
    @brief 全体の幅
    @var AXGIFSave::GLOBALINFO::wHeight
    @brief 全体の高さ
    @var AXGIFSave::GLOBALINFO::nBits
    @brief ビット数（1〜8）
    @var AXGIFSave::GLOBALINFO::nBKIndex
    @brief 背景色のパレットインデックス
    @var AXGIFSave::GLOBALINFO::nGlobalPalCnt
    @brief グローバルパレット個数（0 でなし）
    @var AXGIFSave::GLOBALINFO::memGlobalPal
    @brief グローバルパレットデータ（DWORDのRGB値）

    @var AXGIFSave::IMGINFO::wOffX
    @brief Xオフセット位置
    @var AXGIFSave::IMGINFO::wOffY
    @brief Yオフセット位置
    @var AXGIFSave::IMGINFO::wWidth
    @brief 画像の幅
    @var AXGIFSave::IMGINFO::wHeight
    @brief 画像の高さ
    @var AXGIFSave::IMGINFO::wDelayTime
    @brief 表示遅延時間（1/100秒単位）
    @var AXGIFSave::IMGINFO::nTPIndex
    @brief 透過色のパレットインデックス（-1 でなし）
    @var AXGIFSave::IMGINFO::nDisposal
    @brief 画像の破棄方法
    @var AXGIFSave::IMGINFO::nLocalPalCnt
    @brief ローカルパレット個数（0 でなし）
    @var AXGIFSave::IMGINFO::memLocalPal
    @brief ローカルパレットデータ（DWORDのRGB値）
    @var AXGIFSave::IMGINFO::memImg
    @brief 画像データ（幅x高さx1Byte）
*/


AXGIFSave::~AXGIFSave()
{
    close();
}

//! 閉じる

void AXGIFSave::close()
{
    if(m_file.isOpen())
    {
        BYTE end = 0x3b;

        m_file.write(&end, 1);  //終端
        m_file.close();
    }
}

//! ファイルを開く

BOOL AXGIFSave::openFile(const AXString &filename,AXGIFSave::GLOBALINFO *pInfo)
{
    if(!m_file.openWrite(filename)) return FALSE;

    m_pGInfo = pInfo;

    return _header();
}

//! イメージ出力
/*!
    wDelayTime、nTPIndex、nDisposal は制御ブロックでの情報。@n
    bGrpCtl を FALSE にした場合は出力されない。

    @param pImgBuf 幅x高さのバイトデータ
    @param bGrpCtl TRUEで、透過色などの情報の制御ブロックを付ける
*/

BOOL AXGIFSave::putImg(const AXGIFSave::IMGINFO &info,LPBYTE pImgBuf,BOOL bGrpCtl)
{
    if(bGrpCtl) _block_grpctl(info);

    return _block_image(info, pImgBuf);
}

//! ループ指定ブロックを出力
/*!
    画像の後に出力する。

    @param wLoopCnt ループ回数。0 で無限。
*/

void AXGIFSave::putLoopBlock(WORD wLoopCnt)
{
    BYTE dat[19] = {0x21,0xff,0x0b,
        'N','E','T','S','C','A','P','E','2','.','0',
        0x03,0x01,0,0,0};

    AXSetWORDLE(dat + 16, wLoopCnt);

    m_file.write(dat, 19);
}

//--------------


//! パレット数からビット数計算

int AXGIFSave::calcBits(int pcnt)
{
    int i;

    for(i = 1; i < 8; i++)
    {
        if(pcnt <= (1 << i)) return i;
    }

    return 8;
}

//! IMGINFO 構造体データを初期化

void AXGIFSave::initIMGINFO(AXGIFSave::IMGINFO *pInfo,int w,int h)
{
    pInfo->wOffX        = 0;
    pInfo->wOffY        = 0;
    pInfo->wWidth       = w;
    pInfo->wHeight      = h;
    pInfo->wDelayTime   = 0;
    pInfo->nTPIndex     = -1;
    pInfo->nDisposal    = 0;
    pInfo->nLocalPalCnt = 0;
}

//! パレットから指定したRGBカラーのインデックス位置を取得
/*!
    @return -1 で見つからなかった
*/

int AXGIFSave::getColIndex(DWORD col,LPDWORD pPalBuf,int pcnt)
{
    int i;

    col &= 0xffffff;

    for(i = 0; i < pcnt; i++, pPalBuf++)
    {
        if((*pPalBuf & 0xffffff) == col)
            return i;
    }

    return -1;
}

//! 32bitイメージからGIFイメージへ変換
/*!
    画像バッファが確保されていない場合は自動で確保される。

    @param pInfo    幅・高さをセットしておく。
    @param pmem     イメージバッファ
    @param pPalette パレットバッファ（DWORDのRGB。NULLでローカルパレット）
    @param palcnt   パレット個数（0 でローカルパレット個数）
*/

BOOL AXGIFSave::convFrom32bit(LPDWORD pSrc,AXGIFSave::IMGINFO *pInfo,AXMem *pmem,LPDWORD pPalette,int palcnt)
{
    int i,j;
    LPBYTE pd;
    LPDWORD ppal;
    DWORD col;

    //画像バッファ確保

    if(pmem->isNULL())
    {
        if(!pmem->alloc((int)pInfo->wWidth * pInfo->wHeight))
            return FALSE;
    }

    //パレットデータ

    if(!pPalette)
    {
        pPalette = pInfo->memLocalPal;
        palcnt   = pInfo->nLocalPalCnt;
    }

    //変換

    pd = *pmem;

    for(i = (int)pInfo->wWidth * pInfo->wHeight; i > 0; i--)
    {
        col = *(pSrc++) & 0xffffff;

        for(j = 0, ppal = pPalette; j < palcnt; j++, ppal++)
        {
            if(col == *ppal) break;
        }

        *(pd++) = (j == palcnt)? 0: j;
    }

    return TRUE;
}


//=================================
//
//=================================


//! ヘッダ

BOOL AXGIFSave::_header()
{
    BYTE dat[13] = {'G','I','F','8','9','a', 0,0, 0,0, 0, 0,0};
    int pbits;

    //幅・高さ

    AXSetWORDLE(dat + 6, m_pGInfo->wWidth);
    AXSetWORDLE(dat + 8, m_pGInfo->wHeight);

    //フラグ

    dat[10] = (m_pGInfo->nBits - 1) << 4;

    if(m_pGInfo->nGlobalPalCnt)
    {
        pbits = calcBits(m_pGInfo->nGlobalPalCnt);
        dat[10] |= 0x80 | (pbits - 1);
    }

    //背景色

    dat[11] = m_pGInfo->nBKIndex;

    //

    m_file.write(dat, 13);

    //グローバルパレット

    if(m_pGInfo->nGlobalPalCnt)
    {
        if(!_palette(m_pGInfo->memGlobalPal, m_pGInfo->nGlobalPalCnt))
            return FALSE;
    }

    return TRUE;
}

//! パレット書き込み

BOOL AXGIFSave::_palette(LPDWORD pSrc,int cnt)
{
    int i,pcnt;
    LPBYTE pd;
    DWORD col;
    AXMem mem;

    pcnt = 1 << calcBits(cnt);

    if(!mem.allocClear(pcnt * 3)) return FALSE;

    //DWORD -> RGB(3Byte)

    pd = mem;

    for(i = cnt; i > 0; i--, pd += 3)
    {
        col = *(pSrc++);

        pd[0] = _GETR(col);
        pd[1] = _GETG(col);
        pd[2] = _GETB(col);
    }

    //

    m_file.write(mem, pcnt * 3);

    return TRUE;
}

//! 画像制御ブロック

void AXGIFSave::_block_grpctl(const AXGIFSave::IMGINFO &info)
{
    BYTE dat[8] = {0x21, 0xf9, 0x04, 0,0,0,0, 0};

    //フラグ

    dat[3] = info.nDisposal << 2;

    //delaytime

    AXSetWORDLE(dat + 4, info.wDelayTime);

    //透過色

    if(info.nTPIndex >= 0)
    {
        dat[3] |= 1;
        dat[6] = info.nTPIndex;
    }

    //

    m_file.write(dat, 8);
}

//! 画像ブロック

BOOL AXGIFSave::_block_image(const AXGIFSave::IMGINFO &info,LPBYTE pImgBuf)
{
    BYTE dat[10];
    int pbits;
    AXGIFSave_LZW lzw;

    //---------- 情報

    dat[0] = 0x2c;

    //オフセット・サイズ

    AXSetWORDLE(dat + 1, info.wOffX);
    AXSetWORDLE(dat + 3, info.wOffY);
    AXSetWORDLE(dat + 5, info.wWidth);
    AXSetWORDLE(dat + 7, info.wHeight);

    //フラグ

    dat[9] = 0;

    if(info.nLocalPalCnt)
    {
        pbits = calcBits(info.nLocalPalCnt);
        dat[9] |= 0x80 | (pbits - 1);
    }

    //

    m_file.write(dat, 10);

    //ローカルパレット

    if(info.nLocalPalCnt)
    {
        if(!_palette(info.memLocalPal, info.nLocalPalCnt))
            return FALSE;
    }

    //-----------

    //イメージ

    return lzw.encode(&m_file, m_pGInfo->nBits, pImgBuf, (int)info.wWidth * info.wHeight);
}


//*****************************
// AXGIFSave_LZW
//*****************************


#define HT_SIZE      8192
#define HT_KEY_MASK  0x1fff


//! 開始

BOOL AXGIFSave_LZW::encode(AXFile *pFile,int bits,LPBYTE pSrcBuf,int nSrcBufSize)
{
    int ccode,newcode,hkey;
    DWORD newkey;
    LPBYTE ps,psEnd;
    BYTE val;

    //初期化

    m_pFile = pFile;

    if(!init(bits)) return FALSE;

    //LZW最小サイズビット

    m_pFile->writeBYTE(m_depth);

    putCode(m_clearcode);

    //

    ps      = pSrcBuf;
    psEnd   = pSrcBuf + nSrcBufSize;
    ccode   = *(ps++);

    while(ps < psEnd)
    {
        val = *(ps++);

        newkey  = ((DWORD)ccode << m_depth) + val;
        newcode = searchHash(newkey);

        if(newcode >= 0)
            ccode = newcode;
        else
        {
            putCode(ccode);

            ccode = val;

            if(m_runcode >= 4095)
            {
                putCode(m_clearcode);

                m_runcode = m_endcode + 1;
                m_runbits = m_depth + 1;
                m_maxcode = 1 << m_runbits;

                ::memset(m_pHashTbl, 0xff, HT_SIZE * sizeof(DWORD));
            }
            else
            {
                hkey = ((newkey >> 12) ^ newkey) & HT_KEY_MASK;

                while((m_pHashTbl[hkey] >> 12) != 0xfffff)
                    hkey = (hkey + 1) & HT_KEY_MASK;

                m_pHashTbl[hkey] = (newkey << 12) | (m_runcode & 0xfff);
                m_runcode++;
            }
        }
    }

    //終了

    putCode(ccode);
    putCode(m_endcode);
    putCode(-1);

    //解放

    ::free(m_pBuf);
    ::free(m_pHashTbl);

    return TRUE;
}

//! 初期化

BOOL AXGIFSave_LZW::init(int bits)
{
    //メモリ確保

    m_pBuf = (LPBYTE)::malloc(1024); //4ブロック分。(ブロックサイズ[1]＋圧縮データ[255])x4
    if(!m_pBuf) return FALSE;

    m_pHashTbl = (LPDWORD)::malloc(HT_SIZE * sizeof(DWORD));
    if(!m_pHashTbl)
    {
        ::free(m_pBuf);
        return FALSE;
    }

    //初期化

    ::memset(m_pHashTbl, 0xff, HT_SIZE * sizeof(DWORD));

    m_pNowBlock = m_pBuf;

    m_depth     = (bits < 2)? 2: bits;
    m_clearcode = 1 << m_depth;
    m_endcode   = m_clearcode + 1;
    m_runcode   = m_endcode + 1;
    m_runbits   = m_depth + 1;
    m_maxcode   = 1 << m_runbits;
    m_shift     = 0;
    m_nowVal    = 0;
    m_bufpos    = 0;
    m_blockpos  = 0;

    return TRUE;
}

//! コード書き出し

void AXGIFSave_LZW::putCode(int code)
{
    if(code < 0)
    {
        //-------- 終了

        while(m_shift > 0)
        {
            putByte(m_nowVal & 0xff);

            m_nowVal >>= 8;
            m_shift  -= 8;
        }

        putByte(-1);
    }
    else
    {
        //--------- 通常

        m_nowVal |= code << m_shift;
        m_shift  += m_runbits;

        while(m_shift >= 8)
        {
            putByte(m_nowVal & 0xff);

            m_nowVal >>= 8;
            m_shift  -= 8;
        }

        if(m_runcode >= m_maxcode && code <= 4095)
        {
            m_runbits++;
            m_maxcode = 1 << m_runbits;
        }
    }
}

//! バイト書き出し

void AXGIFSave_LZW::putByte(int ch)
{
    if(ch < 0)
    {
        //-------- 終了

        int size;

        //残りを出力
        //※blockpos と bufpos が 0 なら丁度4ブロック書き出した後

        if(m_blockpos || m_bufpos)
        {
            m_pNowBlock[0] = m_bufpos;

            size = m_blockpos * 256;
            if(m_bufpos) size += m_bufpos + 1;

            m_pFile->write(m_pBuf, size);
        }

        //終端

        m_pFile->writeBYTE(0);
    }
    else
    {
        //1バイト追加

        m_pNowBlock[1 + m_bufpos] = ch;
        m_bufpos++;

        //ブロックのバッファが一杯なら書き出し

        if(m_bufpos == 255)
        {
            m_pNowBlock[0] = 255;

            m_bufpos = 0;
            m_blockpos++;

            if(m_blockpos == 4)
            {
                //4ブロック分書き出し

                m_pFile->write(m_pBuf, 1024);

                m_blockpos  = 0;
                m_pNowBlock = m_pBuf;
            }
            else
                //次のブロックバッファへ
                m_pNowBlock += 256;
        }
    }
}

//! キーがハッシュテーブルに存在するなら確定

int AXGIFSave_LZW::searchHash(DWORD key)
{
    int hkey = ((key >> 12) ^ key) & HT_KEY_MASK;
    DWORD htkey;

    while(1)
    {
        htkey = m_pHashTbl[hkey] >> 12;
        if(htkey == 0xfffff) break;

        if(key == htkey) return m_pHashTbl[hkey] & 0xfff;
        hkey = (hkey + 1) & HT_KEY_MASK;
    }

    return -1;
}
