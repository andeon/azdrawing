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
#include <zlib.h>

#include "AXPNGSave.h"

#include "AXUtil.h"



#define BUFSIZE     (8 * 1024)


//********************************
// AXPNGSave_z
//********************************


class AXPNGSave_z
{
    AXFile      *m_pfile;
    z_stream    m_stream;
    AXMem       m_memBuf;
    DWORD       m_dwOutSize,
                m_dwCRC;

public:
    DWORD getOutSize() const { return m_dwOutSize; }
    DWORD getCRC() const { return m_dwCRC; }

    BOOL init(AXFile *pfile,int nLevel,LPBYTE thunkName);
    BOOL put(LPBYTE pDat,int nSize);
    BOOL end();
};


//********************************
// AXPNGSave
//********************************


/*!
    @class AXPNGSave
    @brief PNG画像保存クラス

    - startImg() 〜 endImg() 間で、putImg() を使ってイメージデータを出力する。

    @ingroup format
*/

/*!
    @var AXPNGSave::INFO::nBits
    @brief ビット数（1,2,4,8,24,32）
    @var AXPNGSave::INFO::nPalCnt
    @brief パレット個数
    @var AXPNGSave::INFO::memPalette
    @brief パレットデータ（DWORDのRGB値）
*/


AXPNGSave::AXPNGSave()
{
    m_pz = NULL;
}

AXPNGSave::~AXPNGSave()
{
    close();
}

//! 閉じる

void AXPNGSave::close()
{
    BYTE end[12] = {0x00,0x00,0x00,0x00,0x49,0x45,0x4e,0x44,0xae,0x42,0x60,0x82};

    if(m_file.isOpen())
    {
        if(m_pz) endImg();

        m_file.write(end, 12);  //IENDチャンク
        m_file.close();
    }
}

//! ファイル開く

BOOL AXPNGSave::openFile(const AXString &filename,AXPNGSave::INFO *pInfo)
{
    BYTE header[8] = {0x89,0x50,0x4e,0x47,0x0d,0x0a,0x1a,0x0a};

    if(!m_file.openWrite(filename)) return FALSE;

    m_file.setEndian(AXFile::ENDIAN_BIG);

    m_pInfo = pInfo;

    //ヘッダ

    m_file.write(header, 8);

    //IHDR

    _IHDR();

    //PLTE

    if(m_pInfo->nBits <= 8)
        _PLTE();

    //Y1列サイズ（フィルタタイプ1Byte含む）

    m_nPitch = ((m_pInfo->nBits * m_pInfo->nWidth + 7) / 8) + 1;

    return TRUE;
}

//! 画像出力開始
/*!
    @param nLevel 圧縮レベル(0-9)
*/

BOOL AXPNGSave::startImg(int nLevel)
{
    BYTE dat[8] = {0,0,0,0, 'I','D','A','T'};

    if(m_pz) return FALSE;

    //AXPNGSave_z

    m_pz = new AXPNGSave_z;

    if(!m_pz->init(&m_file, nLevel, dat + 4))
    {
        delete m_pz;
        m_pz = NULL;

        return FALSE;
    }

    //チャンクサイズ・チャンク名

    m_dwSizePos = m_file.getPosition();
    m_file.write(dat, 8);

    return TRUE;
}

//! イメージデータ出力
/*!
    自由なサイズで順次出力していく。@n
    Y は上から順に下へ。@n
    Y1列の先頭にはフィルタタイプの 1Byte が含まれる。

    @param nSize 0以下でY1列のサイズ
*/

BOOL AXPNGSave::putImg(LPBYTE pBuf,int nSize)
{
    if(nSize <= 0) nSize = m_nPitch;

    if(m_pz)
        return m_pz->put(pBuf, nSize);
    else
        return FALSE;
}

//! 画像出力終了

BOOL AXPNGSave::endImg()
{
    if(!m_pz) return FALSE;

    //終了処理

    if(!m_pz->end())
    {
        delete m_pz;
        m_pz = NULL;

        return FALSE;
    }

    //CRC

    m_file.writeDWORD(m_pz->getCRC());

    //チャンクサイズ

    m_file.seekTop(m_dwSizePos);
    m_file.writeDWORD(m_pz->getOutSize());
    m_file.seekEnd(0);

    //

    delete m_pz;
    m_pz = NULL;

    return TRUE;
}

//-----------------------

//! tRNSチャンク（透過色）

void AXPNGSave::put_tRNS(DWORD col)
{
    BYTE r,g,b;
    LPBYTE pbuf;
    LPDWORD ppal;
    DWORD size;
    int i,f;

    if(m_pInfo->nBits == 32) return;

    //RGB

    r = _GETR(col);
    g = _GETG(col);
    b = _GETB(col);

    //--------- 透過色データ

    if(m_pInfo->nBits == 24)
    {
        //24BIT (2Byte R,G,B)

        pbuf = new BYTE[6];
        size = 6;

        pbuf[0] = pbuf[2] = pbuf[4] = 0;
        pbuf[1] = r;
        pbuf[3] = g;
        pbuf[5] = b;
    }
    else
    {
        //パレットカラー（各パレットのアルファ値）

        pbuf = new BYTE[256];

        ppal = m_pInfo->memPalette;

        for(i = 0, f = 0; i < m_pInfo->nPalCnt; i++, ppal++)
        {
            if(col == (*ppal & 0xffffff))
            {
                pbuf[i] = 0;
                f = 1;
                break;
            }

            pbuf[i] = 255;
        }

        if(f == 0) { delete []pbuf; return;	}   //パレットに指定色がなかった

        size = i + 1;
    }

    //

    putThunk("tRNS", pbuf, size);

    delete []pbuf;
}

//! pHYsチャンク出力（解像度）
/*!
    値は pixel/meter 単位。
*/

void AXPNGSave::put_pHYs(int h,int v)
{
    BYTE dat[9];

    //X,Y解像度

    AXSetDWORDBE(dat, h);
    AXSetDWORDBE(dat + 4, v);

    //タイプ (1 = pixel per meter)

    dat[8] = 1;

    //

    putThunk("pHYs", dat, 9);
}

//-----------------------

//! Y1列データ変換（32bit -> 8/24/32bit）

void AXPNGSave::lineFrom32bit(LPBYTE pDst,LPDWORD pSrc,int bits)
{
    int x,i,pcnt;
    DWORD col;
    LPDWORD pPal;
    BYTE c;

    pPal = m_pInfo->memPalette;
    pcnt = m_pInfo->nPalCnt;

    //フィルタタイプ
    *(pDst++) = 0;

    for(x = m_pInfo->nWidth; x > 0; x--)
    {
        col = *(pSrc++);

        if(bits == 8)
        {
            col &= 0xffffff;

            for(i = 0, c = 0; i < pcnt; i++)
            {
                if(pPal[i] == col) { c = i; break; }
            }

            *(pDst++) = c;
        }
        else
        {
            pDst[0] = _GETR(col);
            pDst[1] = _GETG(col);
            pDst[2] = _GETB(col);

            if(bits == 32)
            {
                pDst[3] = (BYTE)(col >> 24);
                pDst += 4;
            }
            else
                pDst += 3;
        }
    }
}


//===============================
// 各チャンク出力
//===============================


//! チャンク書き込み

void AXPNGSave::putThunk(LPCSTR name,LPBYTE pDat,int size)
{
    LPBYTE pbuf;
    DWORD crc;

    pbuf = new BYTE[size + 12];

    //チャンクサイズ

    AXSetDWORDBE(pbuf, size);

    //チャンク名

    pbuf[4] = name[0];
    pbuf[5] = name[1];
    pbuf[6] = name[2];
    pbuf[7] = name[3];

    //データ

    ::memcpy(pbuf + 8, pDat, size);

    //CRC

    crc = crc32(0, Z_NULL, 0);
    crc = crc32(crc, (LPBYTE)name, 4);
    crc = crc32(crc, pDat, size);

    AXSetDWORDBE(pbuf + 8 + size, crc);

    //

    m_file.write(pbuf, size + 12);

    delete []pbuf;
}

//! IHDRチャンク

void AXPNGSave::_IHDR()
{
    BYTE dat[13],bit,col;

    //幅・高さ

    AXSetDWORDBE(dat, m_pInfo->nWidth);
    AXSetDWORDBE(dat + 4, m_pInfo->nHeight);

    //ビット深度・カラータイプ

    if(m_pInfo->nBits == 32)
        bit = 8, col = 6;
    else if(m_pInfo->nBits == 24)
        bit = 8, col = 2;
    else
        bit = m_pInfo->nBits, col = 3;

    dat[8] = bit;
    dat[9] = col;

    //圧縮手法・フィルタ形式・インターレース(0)

    dat[10] = dat[11] = dat[12] = 0;

    //

    putThunk("IHDR", dat, 13);
}

//! PLTEチャンク（パレット）

void AXPNGSave::_PLTE()
{
    int i;
    DWORD col;
    LPBYTE pd;
    LPDWORD ps;
    AXMem mem;

    if(m_pInfo->nPalCnt <= 0 || m_pInfo->memPalette.isNULL()) return;

    //DWORDパレット -> RGB

    if(!mem.alloc(m_pInfo->nPalCnt * 3)) return;

    pd = mem;
    ps = m_pInfo->memPalette;

    for(i = m_pInfo->nPalCnt; i > 0; i--, pd += 3)
    {
        col = *(ps++);

        pd[0] = _GETR(col);
        pd[1] = _GETG(col);
        pd[2] = _GETB(col);
    }

    //

    putThunk("PLTE", mem, m_pInfo->nPalCnt * 3);
}


//********************************
// AXPNGSave_z
//********************************


//! 初期化

BOOL AXPNGSave_z::init(AXFile *pfile,int nLevel,LPBYTE thunkName)
{
    //圧縮データ出力用

    if(!m_memBuf.alloc(BUFSIZE)) return FALSE;

    //z_stream

    ::memset(&m_stream, 0, sizeof(z_stream));

    if(deflateInit(&m_stream, nLevel) != Z_OK)
        return FALSE;

    m_stream.next_out  = (LPBYTE)m_memBuf;
    m_stream.avail_out = BUFSIZE;

    //

    m_pfile     = pfile;
    m_dwOutSize = 0;

    m_dwCRC = crc32(0, NULL, 0);
    m_dwCRC = crc32(m_dwCRC, thunkName, 4);

    return TRUE;
}

//! 入力データ渡す

BOOL AXPNGSave_z::put(LPBYTE pDat,int nSize)
{
    int ret;

    m_stream.next_in  = pDat;
    m_stream.avail_in = nSize;

    while(m_stream.avail_in)
    {
        ret = deflate(&m_stream, Z_NO_FLUSH);
        if(ret != Z_OK && ret != Z_STREAM_END) return FALSE;

        //ファイルに書き出し

        if(m_stream.avail_out == 0)
        {
            m_pfile->write(m_memBuf, BUFSIZE);

            m_stream.next_out  = (LPBYTE)m_memBuf;
            m_stream.avail_out = BUFSIZE;

            m_dwCRC = crc32(m_dwCRC, m_memBuf, BUFSIZE);
            m_dwOutSize += BUFSIZE;
        }
    }

    return TRUE;
}

//! 終了

BOOL AXPNGSave_z::end()
{
    int ret;

    //入力終了

    while(1)
    {
        ret = deflate(&m_stream, Z_FINISH);

        if(ret == Z_STREAM_END) break;
        if(ret != Z_OK)
        {
            deflateEnd(&m_stream);
            return FALSE;
        }

        //ファイルに書き出し

        if(m_stream.avail_out == 0)
        {
            m_pfile->write(m_memBuf, BUFSIZE);

            m_stream.next_out  = (LPBYTE)m_memBuf;
            m_stream.avail_out = BUFSIZE;

            m_dwCRC = crc32(m_dwCRC, m_memBuf, BUFSIZE);
            m_dwOutSize += BUFSIZE;
        }
    }

    //残りをファイルに書き出し

    ret = BUFSIZE - m_stream.avail_out;

    if(ret)
    {
        m_pfile->write(m_memBuf, ret);

        m_dwCRC = crc32(m_dwCRC, m_memBuf, ret);
        m_dwOutSize += ret;
    }

    //zlib 終了

    deflateEnd(&m_stream);

    return TRUE;
}
