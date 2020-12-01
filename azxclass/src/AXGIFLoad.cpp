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

#include "AXGIFLoad.h"

#include "AXFile.h"


//*******************************
// LZW展開クラス
//*******************************


class AXGIF_LZWDecode
{
public:
    struct INFO
    {
        int     nOffX,
                nOffY,
                nWidth,
                nHeight,
                nImgW,
                nImgH;
        BOOL    bInterlace;
        LPBYTE  pImgBuf;
    };

private:
    AXGIF_LZWDecode::INFO *m_pInfo;
    AXBuf   *m_pbuf;

    BYTE    m_btNow;        //現在のGIFデータ
    int     m_min_bits,     //最小ビット長
            m_cur_bits,     //現在のビット長
            m_clear,        //クリアコード
            m_endcode,      //終了コード
            m_newcode,
            m_topslot,
            m_slot,
            m_blockremain,  //ブロックの残りサイズ
            m_leftbits,
            m_nNowX,
            m_nNowY,
            m_nInterPass;

    BOOL init();
    int getNextCode();
    BOOL setPixel(int c);

public:
    BOOL decode(AXGIF_LZWDecode::INFO *pInfo,AXBuf *pbuf);
};


//************************************
// AXGIFLoad
//************************************


/*!
    @class AXGIFLoad
    @brief GIF画像読み込みクラス

    @ingroup format
*/

/*!
    @var AXGIFLoad::m_nBits
    @brief ビット数(1〜8)
    @var AXGIFLoad::m_nBkIndex
    @brief 背景色のインデックス
    @var AXGIFLoad::m_nTPIndex
    @brief 透過色のインデックス（画像ブロック毎に変わる）。-1でなし。
    @var AXGIFLoad::m_memGlobalPal
    @brief グローバルパレットデータ（DWORD、*RGB 順。256個確保されている）
    @var AXGIFLoad::m_nGlobalPalCnt
    @brief グローバルパレット個数
    @var AXGIFLoad::m_memLocalPal
    @brief ローカルパレットデータ（DWORD、*RGB 順。256個確保されている）
    @var AXGIFLoad::m_nLocalPalCnt
    @brief ローカルパレット個数
    @var AXGIFLoad::m_memImg
    @brief 画像データ（幅×高さ×1バイト でパレット番号が並んでいる）
*/


//! 解放

void AXGIFLoad::free()
{
    m_memDat.free();
    m_memImg.free();
    m_memGlobalPal.free();
    m_memLocalPal.free();
}

//! ファイルから読み込み

BOOL AXGIFLoad::loadFile(const AXString &filename)
{
    free();

    if(!AXFile::readFileFull(filename, &m_memDat)) return FALSE;

    m_buf.init(m_memDat, m_memDat.getSize(), AXBuf::ENDIAN_LITTLE);

    return _topheader();
}

//! バッファから読み込み

BOOL AXGIFLoad::loadBuf(LPVOID pBuf,DWORD dwSize)
{
    free();

    m_buf.init(pBuf, dwSize, AXBuf::ENDIAN_LITTLE);

    return _topheader();
}

//! 次のイメージブロック読み込み

BOOL AXGIFLoad::getNextImage()
{
    m_nTPIndex = -1;

    if(!_nextImgBlock()) return FALSE;

    if(!_block_image()) return FALSE;

    return TRUE;
}

//! パレットカラー取得
/*!
    ローカルパレットが優先。
*/

DWORD AXGIFLoad::getPalCol(int index)
{
    if(m_nLocalPalCnt)
    {
        if(index >= m_nLocalPalCnt)
            return 0;
        else
            return *((LPDWORD)m_memLocalPal + index);
    }
    else if(m_nGlobalPalCnt)
    {
        if(index >= m_nGlobalPalCnt)
            return 0;
        else
            return *((LPDWORD)m_memGlobalPal + index);
    }
    else
        return 0;
}

//! パレットデータ取得

void AXGIFLoad::getPalette(DWORD **ppBuf,LPINT pCnt)
{
    if(m_nLocalPalCnt)
    {
        *ppBuf = m_memLocalPal;
        *pCnt  = m_nLocalPalCnt;
    }
    else
    {
        *ppBuf = m_memGlobalPal;
        *pCnt  = m_nGlobalPalCnt;
    }
}

//! Y1列を32bitカラーに変換
/*!
    @param bTPZero 透過色のカラーを 0 にする
*/

void AXGIFLoad::lineTo32bit(LPDWORD pDst,int y,BOOL bTPZero)
{
    LPBYTE ps;
    LPDWORD pPal;
    int i,n,pcnt;

    getPalette(&pPal, &pcnt);

    ps = (LPBYTE)m_memImg + y * m_nWidth;

    for(i = m_nWidth; i > 0; i--)
    {
        n = *(ps++);
        if(n >= pcnt) n = 0;

        if(bTPZero && n == m_nTPIndex)
            *(pDst++) = 0;
        else
            *(pDst++) = pPal[n] | 0xff000000;
    }
}

//! 32bitイメージに変換
/*!
    @param bTPZero 透過色のカラーを 0 にする
*/

void AXGIFLoad::to32bit(LPDWORD pDst,BOOL bTPZero)
{
    LPBYTE ps;
    LPDWORD pPal;
    int i,n,pcnt;

    getPalette(&pPal, &pcnt);

    ps = m_memImg;

    for(i = m_nWidth * m_nHeight; i > 0; i--)
    {
        n = *(ps++);
        if(n >= pcnt) n = 0;

        if(bTPZero && n == m_nTPIndex)
            *(pDst++) = 0;
        else
            *(pDst++) = pPal[n] | 0xff000000;
    }
}


//================================
//
//================================


//! トップヘッダ

BOOL AXGIFLoad::_topheader()
{
    char m[3];
    BYTE flag;

    if(!m_buf.isRemain(13)) return FALSE;

    //"GIF"

    m_buf.getDat(m, 3);
    if(::strncmp(m, "GIF", 3)) return FALSE;

    //バージョン

    m_buf.getDat(m, 3);
    if(::strncmp(m, "87a", 3) && ::strncmp(m, "89a", 3)) return FALSE;

    //幅・高さ

    m_nWidth  = m_buf.getWORD();
    m_nHeight = m_buf.getWORD();

    //フラグ

    m_buf.getBYTE(&flag);

    //ビット数

    m_nBits = ((flag >> 4) & 7) + 1;

    //背景色のインデックス

    m_nBkIndex = m_buf.getBYTE();

    //縦横比

    m_buf.seek(1);

    //グローバルパレット

    if(flag & 0x80)
    {
        m_nGlobalPalCnt = 1 << ((flag & 7) + 1);

        if(!_palette(&m_memGlobalPal, m_nGlobalPalCnt))
            return FALSE;
    }
    else
        m_nGlobalPalCnt = 0;

    return TRUE;
}

//! パレット読み込み

BOOL AXGIFLoad::_palette(AXMem *pmem,int cnt)
{
    int i;
    LPBYTE ps;
    LPDWORD pd;

    if(!m_buf.isRemain(cnt * 3)) return FALSE;

    if(!pmem->allocClear(256 * 4)) return FALSE;

    //

    ps = m_buf.getNowBuf();
    pd = *pmem;

    for(i = cnt; i > 0; i--, ps += 3)
        *(pd++) = (ps[0] << 16) | (ps[1] << 8) | ps[2];

    //

    m_buf.seek(cnt * 3);

    return TRUE;
}

//! 次のイメージブロックまで移動

BOOL AXGIFLoad::_nextImgBlock()
{
    BYTE sig,bsig;

    while(1)
    {
        //識別子

        if(!m_buf.getBYTE(&sig)) return FALSE;

        switch(sig)
        {
            //画像ブロック
            case 0x2c:
                return TRUE;
            //サブブロック付きブロック
            case 0x21:
                //ブロック識別子
                if(!m_buf.getBYTE(&bsig)) return FALSE;

                //グラフィック制御ブロック
                if(bsig == 0xf9)
                {
                    if(!_block_grpctrl()) return FALSE;
                }

                //サブブロックを飛ばす
                if(!_skipSubBlock()) return FALSE;
                break;
            //他
            default:
                return FALSE;
        }
    }

    return FALSE;
}

//! 画像ブロック

BOOL AXGIFLoad::_block_image()
{
    BYTE flag;
    AXGIF_LZWDecode lzw;
    AXGIF_LZWDecode::INFO info;

    if(!m_buf.isRemain(9)) return FALSE;

    //----------- 情報

    //位置とサイズ

    info.nOffX = m_buf.getWORD();
    info.nOffY = m_buf.getWORD();
    info.nImgW = m_buf.getWORD();
    info.nImgH = m_buf.getWORD();

    //フラグ

    m_buf.getBYTE(&flag);

    //ローカルパレット

    if(flag & 0x80)
    {
        m_nLocalPalCnt = 1 << ((flag & 7) + 1);

        if(!_palette(&m_memLocalPal, m_nLocalPalCnt))
            return FALSE;
    }
    else
    {
        m_nLocalPalCnt = 0;
        m_memLocalPal.free();
    }

    //------------

    //画像バッファ

    if(!m_memImg.alloc(m_nWidth * m_nHeight))
        return FALSE;

    //背景色で塗りつぶす

    ::memset(m_memImg, m_nBkIndex, m_nWidth * m_nHeight);

    //LZW展開

    info.nWidth     = m_nWidth;
    info.nHeight    = m_nHeight;
    info.pImgBuf    = m_memImg;
    info.bInterlace = flag & 0x40;

    return lzw.decode(&info, &m_buf);
}

//! グラフィック制御ブロック（直後の画像ブロックの情報）

BOOL AXGIFLoad::_block_grpctrl()
{
    LPBYTE p;

    if(!m_buf.isRemain(6)) return FALSE;

    /*
        BYTE blocksize(4)
        BYTE flag
        WORD delaytime
        BYTE tpindex
        BYTE blocksize(0)
    */

    p = m_buf.getNowBuf();

    if(p[1] & 1)
        m_nTPIndex = p[4];

    return TRUE;
}

//! サブブロックを飛ばす

BOOL AXGIFLoad::_skipSubBlock()
{
    BYTE size;

    while(1)
    {
        if(!m_buf.getBYTE(&size)) return FALSE;

        if(size == 0) break;

        if(!m_buf.seek(size)) return FALSE;
    }

    return TRUE;
}



//*******************************
// AXGIF_LZWDecode
//*******************************


//! デコード

BOOL AXGIF_LZWDecode::decode(AXGIF_LZWDecode::INFO *pInfo,AXBuf *pbuf)
{
    AXMem memSuffix,memPrefix,memStack;
    LPBYTE sp,pSuffix,pStack;
    LPWORD pPrefix;
    int c,oc,fc,code;

    //初期化

    m_pInfo = pInfo;
    m_pbuf  = pbuf;

    if(!init()) return FALSE;

    //メモリ確保

    if(!memStack.alloc(4096)) return FALSE;
    if(!memSuffix.alloc(4096)) return FALSE;
    if(!memPrefix.alloc(4096 * 2)) return FALSE;

    //

    oc = fc = 0;

    pStack  = memStack;
    pSuffix = memSuffix;
    pPrefix = memPrefix;

    sp = pStack;

    while(1)
    {
        //コード取得

        c = getNextCode();
        if(c == -1) return FALSE;

        //終了コード

        if(c == m_endcode) break;

        //

        if(c == m_clear)
        {
            //クリアコード

            m_cur_bits  = m_min_bits + 1;
            m_slot      = m_newcode;
            m_topslot   = 1 << m_cur_bits;

            while((c = getNextCode()) == m_clear);

            if(c == -1) return FALSE;
            if(c == m_endcode) break;

            if(c >= m_slot) c = 0;

            oc = fc = c;

            if(setPixel(c)) break;
        }
        else
        {
            code = c;

            if(code >= m_slot)
            {
                code    = oc;
                *(sp++) = fc;
            }

            while(code >= m_newcode)
            {
                *(sp++) = pSuffix[code];
                code    = pPrefix[code];
            }

            *(sp++) = code;

            if(m_slot < m_topslot)
            {
                pSuffix[m_slot] = fc = code;
                pPrefix[m_slot++] = oc;
                oc = c;
            }

            if(m_slot >= m_topslot)
            {
                if(m_cur_bits < 12)
                {
                    m_topslot <<= 1;
                    m_cur_bits++;
                }
            }

            while(sp > pStack)
            {
                if(setPixel(*(--sp))) goto END;
            }
        }
    }

END:
    //ブロックの残りと終端バイトを飛ばす
    m_pbuf->seek(m_blockremain + 1);

    return TRUE;
}

//! 初期化

BOOL AXGIF_LZWDecode::init()
{
    //LZW最小ビット長

    m_min_bits = m_pbuf->getBYTE();
    if(m_min_bits < 2 || m_min_bits > 9) return FALSE;

    //

    m_cur_bits  = m_min_bits + 1;
    m_topslot   = 1 << m_cur_bits;
    m_clear     = 1 << m_min_bits;
    m_endcode   = m_clear + 1;

    m_slot = m_newcode = m_endcode + 1;
    m_blockremain = m_leftbits = 0;

    m_nNowX = m_nNowY = 0;
    m_nInterPass = (m_pInfo->bInterlace)? 1: 0;

    return TRUE;
}

//! 次のコード取得
//! @return -1でデータが足りない

int AXGIF_LZWDecode::getNextCode()
{
    int ret;
    BYTE byte;

    if(m_leftbits == 0)
    {
        //次のブロックへ

        if(m_blockremain <= 0)
        {
            if(!m_pbuf->getBYTE(&byte)) return -1;
            if(!m_pbuf->isRemain(byte)) return -1;
            m_blockremain = byte;
        }

        //次のバイト

        m_pbuf->getBYTE(&m_btNow);
        m_leftbits = 8;

        m_blockremain--;
    }

    //

    ret = m_btNow >> (8 - m_leftbits);

    //値が次のバイトをまたぐ場合

    while(m_cur_bits > m_leftbits)
    {
        if(m_blockremain <= 0)
        {
            if(!m_pbuf->getBYTE(&byte)) return -1;
            if(!m_pbuf->isRemain(byte)) return -1;
            m_blockremain = byte;
        }

        m_pbuf->getBYTE(&m_btNow);

        ret |= (int)m_btNow << m_leftbits;

        m_leftbits += 8;
        m_blockremain--;
    }

    m_leftbits -= m_cur_bits;

    return ret & ((1 << m_cur_bits) - 1);
}

//! 点をセット
//戻り値： TRUEで全部の点を終えた

BOOL AXGIF_LZWDecode::setPixel(int c)
{
    int x,y;
    BYTE pat[5][2] = {{1,0},{8,0},{8,4},{4,2},{2,1}};

    //色セット

    x = m_pInfo->nOffX + m_nNowX;
    y = m_pInfo->nOffY + m_nNowY;

    if(x >= 0 && x < m_pInfo->nWidth && y >= 0 && y < m_pInfo->nHeight)
        m_pInfo->pImgBuf[y * m_pInfo->nWidth + x] = c;

    //次の位置

    m_nNowX++;

    if(m_nNowX >= m_pInfo->nImgW)
    {
        m_nNowY += pat[m_nInterPass][0];
        m_nNowX = 0;

        while(m_nNowY >= m_pInfo->nImgH && m_nInterPass >= 1 && m_nInterPass <= 3)
        {
            m_nInterPass++;
            m_nNowY = pat[m_nInterPass][1];
        }

        if(m_nNowY >= m_pInfo->nImgH) return TRUE;
    }

    return FALSE;
}

