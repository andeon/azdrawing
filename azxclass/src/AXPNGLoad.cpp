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

#include "AXPNGLoad.h"

#include "AXFile.h"
#include "AXBuf.h"
#include "AXImage32.h"
#include "AXImage8.h"


/*!
    @class AXPNGLoad
    @brief PNG画像読み込みクラス

    - 読み込まれると、public メンバに各値が入るので、それを参照する。
    - 16ビットカラーには非対応。

    @ingroup format
*/

/*!
    @var AXPNGLoad::m_nWidth
    @brief 幅(px)
    @var AXPNGLoad::m_nHeight
    @brief 高さ(px)
    @var AXPNGLoad::m_nPitch
    @brief Y1列のバイト数（フィルタタイプの1バイトを含む）
    @var AXPNGLoad::m_nBits
    @brief ビット数（1,2,4,8,24,32）
    @var AXPNGLoad::m_nPalCnt
    @brief パレット数
    @var AXPNGLoad::m_nTPCol
    @brief 透過色のRGBカラー。-1 で透過色なし
    @var AXPNGLoad::m_nResoH
    @brief 水平解像度。通常はドット/メートル（-1でなし）
    @var AXPNGLoad::m_nResoV
    @brief 垂直解像度（-1でなし）
    @var AXPNGLoad::m_memImg
    @brief イメージバッファ（Yは上から順。Y列の先頭に1バイトのフィルタタイプがある※フィルタは展開済み）
    @var AXPNGLoad::m_memPalette
    @brief パレットバッファ（DWORD配列。RGB）
*/


//! 解放

void AXPNGLoad::free()
{
    m_memImg.free();
    m_memPalette.free();
}

//! ファイルから読み込み

BOOL AXPNGLoad::loadFile(const AXString &filename)
{
    AXMem mem;
    AXBuf buf;

    free();

    if(!AXFile::readFileFull(filename, &mem)) return FALSE;

    buf.init(mem, mem.getSize(), AXBuf::ENDIAN_BIG);

    return _load(&buf);
}

//! バッファから読み込み

BOOL AXPNGLoad::loadBuf(LPVOID pBuf,DWORD dwSize)
{
    AXBuf buf;

    free();

    if(!pBuf) return FALSE;

    buf.init(pBuf, dwSize, AXBuf::ENDIAN_BIG);

    return _load(&buf);
}

//! Y1列を32bitカラーに変換
/*!
    @param bTPAlpha  透過色を A = 0 にする
*/

void AXPNGLoad::lineTo32bit(LPDWORD pDst,int y,BOOL bTPAlpha)
{
    LPBYTE pSrc;
    LPDWORD pPal;
    int shift,mask,x,n;
    DWORD col;

    if(y >= m_nHeight) return;
    if(m_nBits <= 8 && m_memPalette.isNULL()) return;

    //

    pSrc = (LPBYTE)m_memImg + m_nPitch * y + 1;
    pPal = m_memPalette;

    if(m_nBits <= 8)
    {
        shift = 8 - m_nBits;
        mask  = (1 << m_nBits) - 1;
    }

    //

    for(x = m_nWidth; x > 0; x--)
    {
        switch(m_nBits)
        {
            case 32:
                col = ((DWORD)pSrc[3] << 24) | (pSrc[0] << 16) | (pSrc[1] << 8) | pSrc[2];
                pSrc += 4;
                break;
            case 24:
                col = 0xff000000 | (pSrc[0] << 16) | (pSrc[1] << 8) | pSrc[2];
                pSrc += 3;
                break;
            //8bit以下
            default:
                n = (*pSrc >> shift) & mask;
                if(n >= m_nPalCnt) n = 0;

                col = pPal[n] | 0xff000000;

                shift -= m_nBits;
                if(shift < 0)
                {
                    shift = 8 - m_nBits;
                    pSrc++;
                }
                break;
        }

        //透過色をA=0に

        if(bTPAlpha && (col & 0xffffff) == (DWORD)m_nTPCol)
            col &= 0xffffff;

        *(pDst++) = col;
    }
}

//! AXImage32に変換
/*!
    @param bTPAlpha  透過色を A = 0 にする
*/

BOOL AXPNGLoad::toImage32(AXImage32 *pimg,BOOL bTPAlpha)
{
    LPDWORD pDst;
    int y;

    if(m_nBits <= 8 && m_memPalette.isNULL()) return FALSE;

    if(!pimg->create(m_nWidth, m_nHeight))
        return FALSE;

    //

    pDst = pimg->getBuf();

    for(y = 0; y < m_nHeight; y++)
    {
        lineTo32bit(pDst, y, bTPAlpha);

        pDst += m_nWidth;
    }

    return TRUE;
}

//! AXImage8 に変換（PNGが8ビット以下の場合）

BOOL AXPNGLoad::toImage8(AXImage8 *pimg)
{
    LPBYTE pSrcY,pSrc,pDst;
    int x,y,shift,mask,n,addDst;

    if(m_nBits > 8) return FALSE;

    if(!pimg->create(m_nWidth, m_nHeight, m_memPalette, m_nPalCnt))
        return FALSE;

    //

    pSrcY  = m_memImg;
    pDst   = pimg->getBuf();
    addDst = pimg->getPitch() - pimg->getWidth();

    for(y = m_nHeight; y > 0; y--)
    {
        pSrc  = pSrcY + 1;
        shift = 8 - m_nBits;
        mask  = (1 << m_nBits) - 1;

        for(x = m_nWidth; x > 0; x--)
        {
            n = (*pSrc >> shift) & mask;
            if(n >= m_nPalCnt) n = 0;

            *(pDst++) = n;

            //

            shift -= m_nBits;
            if(shift < 0)
            {
                shift = 8 - m_nBits;
                pSrc++;
            }
        }

        pSrcY += m_nPitch;
        pDst  += addDst;
    }

    return TRUE;
}

//! アプリケーションアイコンイメージに変換
/*!
    long 値で32bitイメージを作成。透過色は A=0 とする。
*/

BOOL AXPNGLoad::toAppIconImg(AXMem *pmem)
{
    LPBYTE pSrcY,pSrc;
    LPDWORD pPal;
    unsigned long *pDst;
    int x,y,shift,mask,n;
    DWORD col;

    if(m_nBits <= 8 && m_memPalette.isNULL()) return FALSE;

    if(!pmem->alloc((2 + m_nWidth * m_nHeight) * sizeof(long)))
		return FALSE;

    //

    pDst  = (unsigned long *)pmem->getBuf();
    pSrcY = m_memImg;
    pPal  = m_memPalette;

    *(pDst++) = m_nWidth;
    *(pDst++) = m_nHeight;

    for(y = m_nHeight; y > 0; y--)
    {
        pSrc = pSrcY + 1;

        if(m_nBits <= 8)
        {
            shift = 8 - m_nBits;
            mask  = (1 << m_nBits) - 1;
        }

        for(x = m_nWidth; x > 0; x--)
        {
            switch(m_nBits)
            {
                case 32:
                    col = ((DWORD)pSrc[3] << 24) | (pSrc[0] << 16) | (pSrc[1] << 8) | pSrc[2];
                    pSrc += 4;
                    break;
                case 24:
                    col = 0xff000000 | (pSrc[0] << 16) | (pSrc[1] << 8) | pSrc[2];
                    pSrc += 3;
                    break;
                default:
                    n = (*pSrc >> shift) & mask;
                    if(n >= m_nPalCnt) n = 0;

                    col = pPal[n] | 0xff000000;

                    shift -= m_nBits;
                    if(shift < 0)
                    {
                        shift = 8 - m_nBits;
                        pSrc++;
                    }
                    break;
            }

            //透過色を0に

            if((col & 0xffffff) == (DWORD)m_nTPCol)
                col = 0;

            *(pDst++) = col;
        }

        pSrcY += m_nPitch;
    }

    return TRUE;
}



//==============================
//
//==============================


//! 読み込み処理

BOOL AXPNGLoad::_load(AXBuf *pbuf)
{
    DWORD thunksize,name;
    LPBYTE pBufBk;
    BYTE IHDRflag;

    m_pbuf = pbuf;

    m_nPalCnt   = 0;
    m_nTPCol    = -1;
    m_nResoH = m_nResoV = -1;

    //ヘッダ

    if(!_checkHeader()) return FALSE;

    //----------- 各チャンク

    while(1)
    {
        //データサイズ

        if(!m_pbuf->getDWORD(&thunksize)) break;

        //チャンク名

        m_pbuf->getDWORD(&name);

        //サイズが足りない

        if(!m_pbuf->isRemain(thunksize)) return FALSE;

        //位置保存

        pBufBk = m_pbuf->getNowBuf();

        //各チャンク

        switch(name)
        {
            case 0x49484452:    //IHDR
                if(!_thunk_IHDR(&IHDRflag)) return FALSE;
                break;
            case 0x504c5445:    //PLTE
                if(!_thunk_PLTE(thunksize)) return FALSE;
                break;
            case 0x74524e53:    //tRNS
                _thunk_tRNS(IHDRflag & 2);
                break;
            case 0x70485973:    //pHYs
                _thunk_pHYs();
                break;
            case 0x49444154:    //IDAT
                if(IHDRflag & 1)
                    return _thunk_IDAT_Interlace(thunksize);
                else
                    return _thunk_IDAT_Normal(thunksize);
            case 0x49454e44:    //IEND
                //IDATが来る前に来た場合エラー
                return FALSE;
        }

        //シーク

        m_pbuf->setNowBuf(pBufBk);
        m_pbuf->seek(thunksize + 4);
    }

    return FALSE;
}

//! ヘッダをチェック

BOOL AXPNGLoad::_checkHeader()
{
    DWORD head1,head2;

    m_pbuf->getDWORD(&head1);
    m_pbuf->getDWORD(&head2);

    return (head1 == 0x89504e47 && head2 == 0x0d0a1a0a);
}


//===============================
// 各チャンク読み込み
//===============================


//! IHDRチャンク読み込み
// [pFlag] 0bit:インターレース 1bit:グレイスケール

BOOL AXPNGLoad::_thunk_IHDR(LPBYTE pFlag)
{
    BYTE bit,col,byte;

    *pFlag = 0;

    //幅・高さ

    m_nWidth  = m_pbuf->getDWORD();
    m_nHeight = m_pbuf->getDWORD();

    //ビット深度

    m_pbuf->getBYTE(&bit);

    if(bit != 1 && bit != 2 && bit != 4 && bit != 8) return FALSE;

    //カラータイプ

    m_pbuf->getBYTE(&col);

    if(col != 0 && col != 2 && col != 3 && col != 6) return FALSE;

    //ビット数セット

    if(col == 2) m_nBits = 24;
    else if(col == 6) m_nBits = 32;
    else m_nBits = bit;

    //グレイスケールの場合、パレット作成

    if(col == 0)
    {
        if(!_createGrayPalette()) return FALSE;

        *pFlag |= 2;
    }

    //Y1列バイト数セット

    m_nPitch = ((m_nBits * m_nWidth + 7) / 8) + 1;

    //圧縮手法

    if(m_pbuf->getBYTE() != 0) return FALSE;

    //フィルター手法

    if(m_pbuf->getBYTE() != 0) return FALSE;

    //インターレース手法

    m_pbuf->getBYTE(&byte);
    if(byte >= 2) return FALSE;

    if(byte == 1) *pFlag |= 1;

    return TRUE;
}

//! PLTEチャンク読み込み

BOOL AXPNGLoad::_thunk_PLTE(DWORD size)
{
    LPBYTE pSrc;
    LPDWORD pDst;
    int i;

    if(m_nBits > 8) return FALSE;

    m_nPalCnt = size / 3;
    if(m_nPalCnt > 256) return FALSE;

    //確保

    if(!m_memPalette.alloc(m_nPalCnt * sizeof(DWORD))) return FALSE;

    //RGB -> DWORD

    pDst = m_memPalette;
    pSrc = m_pbuf->getNowBuf();

    for(i = m_nPalCnt; i > 0; i--, pSrc += 3)
        *(pDst++) = _RGB(pSrc[0], pSrc[1], pSrc[2]);

    return TRUE;
}

//! tRNSチャンク読み込み

void AXPNGLoad::_thunk_tRNS(BOOL bGray)
{
    BYTE buf[6];
    int i;

    if(bGray)
    {
        //グレイスケール - 2Byte (range)

        i = m_pbuf->getWORD();
        if(i >= (1 << m_nBits)) i = 0;

        m_nTPCol = *((LPDWORD)m_memPalette + i);
    }
    else if(m_nBits == 24)
    {
        //24bit - 2Byte RGB

        m_pbuf->getDat(buf, 6);

        m_nTPCol = ((int)buf[1] << 16) | (buf[3] << 8) | buf[5];
    }
    else if(m_nBits <= 8 && m_memPalette.isExist())
    {
        //8bit以下 - パレット番号（最初に値が0のパレットを探す）

        for(i = 0; i < m_nPalCnt; i++)
        {
            if(m_pbuf->getBYTE() == 0)
            {
                m_nTPCol = *((LPDWORD)m_memPalette + i);
                break;
            }
        }
    }
}

//! pHYsチャンク読み込み

void AXPNGLoad::_thunk_pHYs()
{
    m_nResoH = m_pbuf->getDWORD();
    m_nResoV = m_pbuf->getDWORD();

    //タイプが 1 でドット/メートル
}

//! IDATチャンク読み込み（非インターレース時）

BOOL AXPNGLoad::_thunk_IDAT_Normal(DWORD thunksize)
{
    DWORD imgsize;
    LPBYTE pbuf;
    int y,bpp;

    imgsize = m_nPitch * m_nHeight;

    //イメージメモリ確保

    if(!m_memImg.alloc(imgsize)) return FALSE;

    //デコード

    if(!_decode_IDAT(m_memImg, imgsize, thunksize)) return FALSE;

    //フィルタ復元

    pbuf = m_memImg;
    bpp  = (m_nBits + 7) / 8;

    for(y = 0; y < m_nHeight; y++)
    {
        _restoreFilter(pbuf, m_nPitch, bpp, y);

        pbuf += m_nPitch;
    }

    return TRUE;
}

//! IDATチャンク読み込み（インターレース時）

BOOL AXPNGLoad::_thunk_IDAT_Interlace(DWORD thunksize)
{
    AXMem memOut;
    DWORD imgsize = 0;
    LPBYTE pSrc,pSrcX,pDst,pDstBuf;
    int dat[7][3],i,x,y,bpp,pitch,xx,yy,shift,mask;
    BYTE c;
    BYTE btAdam7[7][2][2] =
    {
        {{ 8, 0 }, { 8, 0 }},
        {{ 8, 4 }, { 8, 0 }}, {{ 4, 0 }, { 8, 4 }},
        {{ 4, 2 }, { 4, 0 }}, {{ 2, 0 }, { 4, 2 }},
        {{ 2, 1 }, { 2, 0 }}, {{ 1, 0 }, { 2, 1 }},
    };

    //インターレース各情報計算

    for(i = 0; i < 7; i++)
    {
        dat[i][0] = (m_nWidth  + btAdam7[i][0][0] - btAdam7[i][0][1] - 1) / btAdam7[i][0][0];
        dat[i][1] = (m_nHeight + btAdam7[i][1][0] - btAdam7[i][1][1] - 1) / btAdam7[i][1][0];
        dat[i][2] = (dat[i][0] * m_nBits + 7) / 8 + 1;

        imgsize	+= dat[i][1] * dat[i][2];
    }

    //出力メモリ確保

    if(!memOut.alloc(imgsize)) return FALSE;

    //デコード

    if(!_decode_IDAT(memOut, imgsize, thunksize)) return FALSE;

    //実イメージ確保

    if(!m_memImg.alloc(m_nPitch * m_nHeight)) return FALSE;

    if(m_nBits < 8)
        ::memset(m_memImg, 0, m_nPitch * m_nHeight);

    //--------- インターレスのイメージを実際のイメージに変換

    pDstBuf = m_memImg;
    pSrc    = memOut;
    bpp     = (m_nBits + 7) / 8;

    for(i = 0; i < 7; i++)
    {
        pitch = dat[i][2];

        for(y = 0; y < dat[i][1]; y++, pSrc += pitch)
        {
            //Y1列、フィルタ復元

            _restoreFilter(pSrc, pitch, bpp, y);

            //X方向

            if(m_nBits <= 8)
            {
                shift = 8 - m_nBits;
                mask  = (1 << m_nBits) - 1;
            }

            //

            for(x = 0, pSrcX = pSrc + 1; x < dat[i][0]; x++)
            {
                xx = x * btAdam7[i][0][0] + btAdam7[i][0][1];
                yy = y * btAdam7[i][1][0] + btAdam7[i][1][1];

                if(xx < 0 || yy < 0 || xx >= m_nWidth || yy >= m_nHeight)
                    continue;

                if(m_nBits == 24)
                {
                    //24bit

                    pDst = pDstBuf + yy * m_nPitch + xx * 3 + 1;

                    pDst[0] = pSrcX[0];
                    pDst[1] = pSrcX[1];
                    pDst[2] = pSrcX[2];

                    pSrcX += 3;
                }
                else if(m_nBits == 32)
                {
                    //32bit

                    pDst = pDstBuf + yy * m_nPitch + xx * 4 + 1;

                    pDst[0] = pSrcX[0];
                    pDst[1] = pSrcX[1];
                    pDst[2] = pSrcX[2];
                    pDst[3] = pSrcX[3];

                    pSrcX += 4;
                }
                else
                {
                    //1,2,4,8bit

                    c = (*pSrcX >> shift) & mask;

                    pDst = pDstBuf + yy * m_nPitch + (xx * m_nBits) / 8 + 1;

                    switch(m_nBits)
                    {
                        case 8:
                            *pDst = c;
                            break;
                        case 4:
                            *pDst |= c << (4 - (xx & 1) * 4);
                            break;
                        case 2:
                            *pDst |= c << (6 - (xx & 3) * 2);
                            break;
                        case 1:
                            *pDst |= c << (7 - (xx & 7));
                            break;
                    }

                    shift -= m_nBits;
                    if(shift < 0)
                    {
                        shift = 8 - m_nBits;
                        pSrcX++;
                    }
                }
            }
        }
    }

    return TRUE;
}


//===========================
//サブ
//===========================


//! グレイスケールパレット作成

BOOL AXPNGLoad::_createGrayPalette()
{
    LPBYTE p;
    int i,cnt;

    m_nPalCnt = 1 << m_nBits;

    if(!m_memPalette.alloc(m_nPalCnt * sizeof(DWORD))) return FALSE;

    //

    p   = m_memPalette;
    cnt = m_nPalCnt - 1;

    for(i = 0; i < m_nPalCnt; i++, p += 4)
    {
        p[0] = p[1] = p[2] = 255 * i / cnt;
        p[3] = 0;
    }

    return TRUE;
}

//! IDATデコード

BOOL AXPNGLoad::_decode_IDAT(LPBYTE pDstBuf,DWORD dwDstSize,DWORD thunksize)
{
    z_stream z;
    int ret;
    DWORD name;
    BOOL bEnd = FALSE;

    //zlib初期化

    ::memset(&z, 0, sizeof(z_stream));
    if(inflateInit(&z) != Z_OK) return FALSE;

    z.next_out  = pDstBuf;
    z.avail_out = dwDstSize;

    //-----------------

    while(1)
    {
        //入力バッファ

        if(z.avail_in == 0)
        {
            //IDATが続いていないなら終了
            if(bEnd) break;

            //データが足りない
            if(!m_pbuf->isRemain(thunksize)) return FALSE;

            z.next_in   = m_pbuf->getNowBuf();
            z.avail_in  = thunksize;

            m_pbuf->seek(thunksize + 4);

            //IDATが続く場合は続ける

            m_pbuf->getDWORD(&thunksize);
            m_pbuf->getDWORD(&name);

            if(name != 0x49444154) bEnd = TRUE;
        }

        //

        ret = inflate(&z, Z_NO_FLUSH);

        if(ret == Z_STREAM_END)
            break;
        else if(ret != Z_OK)
        {
            inflateEnd(&z);
            return FALSE;
        }
    }

    inflateEnd(&z);

    return TRUE;
}

//! Y1列、フィルタ復元

void AXPNGLoad::_restoreFilter(LPBYTE pbuf,int pitch,int bpp,int y)
{
    LPBYTE p1,p2;
    int type,x;

    type = *pbuf;

    p1 = pbuf + 1;
    p2 = p1 - pitch;

    switch(type)
    {
        //なし
        case 0:
            break;
        //sub 左との差
        case 1:
            for(x = bpp, p1 += bpp; x < pitch - 1; x++, p1++)
                *p1 = *p1 + *(p1 - bpp);
            break;
        //up 上との差
        case 2:
            if(y == 0) break;
            for(x = 0; x < pitch - 1; x++, p1++, p2++)
                *p1 = *p1 + *p2;
            break;
        //average 左と上の平均の差
        case 3:
            if(y != 0)
            {
                for(x = 0; x < bpp; x++, p1++, p2++)
                    *p1 = *p1 + (*p2 >> 1);
                for(x = bpp; x < pitch - 1; x++, p1++, p2++)
                    *p1 = *p1 + ((*(p1 - bpp) + *p2) >> 1);
            }
            else
            {
                p1 += bpp;
                for(x = bpp; x < pitch - 1; x++, p1++)
                    *p1 = *p1 + (*(p1 - bpp) >> 1);
            }
            break;
        //paeth
        case 4:
            if(y != 0)
            {
                for(x = 0; x < bpp; x++, p1++, p2++)
                    *p1 = *p1 + *p2;
                for(x = bpp; x < pitch - 1; x++, p1++, p2++)
                    *p1 = *p1 + _paeth(*(p1 - bpp), *p2, *(p2 - bpp));
            }
            else
            {
                p1 += bpp;
                for(x = bpp; x < pitch - 1; x++, p1++)
                    *p1 = *p1 + *(p1 - bpp);
            }
            break;
    }
}

//! Paethフィルタ関数

BYTE AXPNGLoad::_paeth(BYTE a,BYTE b,BYTE c)
{
    int p,pa,pb,pc;

    p  = b - c;
    pc = a - c;
    pa = (p < 0)? -p : p;
    pb = (pc < 0)? -pc : pc;
    pc = ((p + pc) < 0)? -(p + pc) : p + pc;
    p  = (pa <= pb && pa <=pc) ? a : (pb <= pc) ? b : c;

    return p;
}
