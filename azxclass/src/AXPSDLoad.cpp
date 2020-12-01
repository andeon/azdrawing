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

#include "AXPSDLoad.h"

#include "AXRect.h"
#include "AXBuf.h"
#include "AXUtil.h"


/*!
    @class AXPSDLoad
    @brief PSD(PhotoShop)ファイル読み込みクラス

    - 1bit・8bit RGB・8bit グレイスケールのみ対応。
    - レイヤ数が 0 の場合は一枚絵イメージから読み込むこと。

    @ingroup format

    @code
openFile(...);

//画像リソース（必須）

beginRes();
...          //readRes_* を必要な分だけ
endRes();

//レイヤ

beginLayer();

for(i = 0; i < layercnt; i++)
    readLayerInfo();

beginLayerImageTop();

for(i = 0; i < layercnt; i++)
{
    if(beginLayerImageEach(&rcs))
    {
        beginLayerImageCh(chid);

        for(y = 0; y < rcs.h; y++)
            readLayerImageChLine();
    }
}

endLayer();

close();
@endcode
*/


//! ファイル開く

int AXPSDLoad::openFile(const AXString &filename)
{
    int ret;
    DWORD size;

    m_pLayerWork = NULL;
    m_nMaxWidth  = 0;
    m_nMaxHeight = 0;
    m_nLayerCnt  = 0;
    m_nChNo      = 0;

    //バッファ

    if(!m_memBuf.alloc(16 * 1024)) return ERR_MEMORY;

    //--------- ファイル

    if(!m_file.openRead(filename)) return ERR_OPENFILE;

    m_file.setEndian(AXFile::ENDIAN_BIG);

    //ヘッダ読み込み

    ret = _read_header();
    if(ret != ERR_SUCCESS) return ret;

    //カラーモードデータをスキップ

    if(!m_file.readDWORD(&size)) return ERR_FILEDAT;

    m_file.seekCur(size);

    return ERR_SUCCESS;
}

//! 閉じる

void AXPSDLoad::close()
{
    m_file.close();
}


//==========================
// 一枚絵イメージ
//==========================
/*
    m_dwFilePos    : ラインサイズのファイル位置(PackBits時)
    m_dwFilePosImg : イメージ位置(PackBits時)
*/


//! 一枚絵イメージ読込開始

int AXPSDLoad::beginImage()
{
    WORD comp;

    //---------- メモリ確保

    //Y1列出力用バッファ

    if(!m_memLine.alloc(m_nImgW)) return ERR_MEMORY;

    //ラインサイズバッファ

    if(!m_memLineSize.alloc(m_nImgH * sizeof(WORD))) return ERR_MEMORY;

    //-----------

    //圧縮タイプ (2b)

    if(!m_file.readWORD(&comp)) return ERR_FILEDAT;

    if(comp != 0 && comp != 1) return ERR_FORMAT;

    m_nCompType = comp;

    //ファイル位置

    if(m_nCompType == 1)
    {
        m_dwFilePos    = m_file.getPosition();
        m_dwFilePosImg = m_dwFilePos + m_nImgH * sizeof(WORD) * m_nImgChCnt;
    }

    return ERR_SUCCESS;
}

//! 一枚絵イメージ各チャンネル読込開始

int AXPSDLoad::beginImageCh()
{
    int ret;

    //PackBits 時

    if(m_nCompType == 1)
    {
        //ラインサイズ読み込み

        m_file.seekTop(m_dwFilePos);

        ret = _read_lineSize(m_nImgH, &m_dwChSize);
        if(ret) return ret;

        //次のラインサイズ位置

        m_dwFilePos += m_nImgH * sizeof(WORD);

        //イメージ位置へ

        m_file.seekTop(m_dwFilePosImg);

        //次のイメージ位置

        m_dwFilePosImg += m_dwChSize;
    }

    m_nNowY     = 0;
    m_dwBufSize = 0;
    m_dwBufPos  = 0;

    return ERR_SUCCESS;
}

//! 一枚絵イメージY1列読み込み

int AXPSDLoad::readImageChLine()
{
    return _read_chLine(m_nImgW);
}

//! 一枚絵イメージ・チャンネルスキップ

void AXPSDLoad::skipImageCh()
{
    //次のチャンネル位置へ

    if(m_nCompType == 0)
        m_file.seekCur(m_nImgW * m_nImgH);
    else
        m_file.seekTop(m_dwFilePosImg);
}


//==========================
// レイヤデータ
//==========================
/*
    m_dwFilePos   : レイヤデータ全体サイズの次のファイル位置
    m_dwBlockSize : レイヤデータ全体サイズ
*/


//! レイヤ読み込み開始
/*!
    endRes() の直後に行うこと。@n
    全体サイズまたはレイヤ数が 0 の場合、または 1bit の場合はレイヤなし。
*/

int AXPSDLoad::beginLayer()
{
    DWORD size;
    short lcnt;

    //全体サイズ (4b) [0 でレイヤなし]

    if(!m_file.readDWORD(&m_dwBlockSize)) return ERR_FILEDAT;

    m_dwFilePos = m_file.getPosition();

    if(m_dwBlockSize == 0) return ERR_SUCCESS;

    //レイヤデータサイズ (4b) [0 でレイヤなし]

    if(!m_file.readDWORD(&size)) return ERR_FILEDAT;

    if(size == 0) return ERR_SUCCESS;

    //レイヤ数 (2b) [負の値の場合があるので絶対値に。0 でレイヤなし]

    if(!m_file.readWORD(&lcnt)) return ERR_FILEDAT;

    m_nLayerCnt = (lcnt < 0)? -lcnt: lcnt;

    //レイヤ作業用データ確保

    if(m_nLayerCnt)
    {
        if(!m_memLayerWork.alloc(sizeof(LAYERWORKINFO) * m_nLayerCnt))
            return ERR_MEMORY;

        m_pLayerWork = (LAYERWORKINFO *)m_memLayerWork.getBuf();
    }

    return ERR_SUCCESS;
}

//! レイヤ読み込み終了

void AXPSDLoad::endLayer()
{
    m_file.seekTop(m_dwFilePos + m_dwBlockSize);

    m_memLayerWork.free();
    m_pLayerWork = NULL;
}


//==========================
// レイヤイメージ
//==========================
/*
    m_dwFilePosImg : レイヤイメージの先頭ファイル位置
*/


//! レイヤイメージ読み込み前に一度だけ実行

int AXPSDLoad::beginLayerImageTop()
{
    m_pLayerWork = NULL;

    if(m_nLayerCnt)
    {
        //Y1列出力用バッファ

        if(!m_memLine.alloc(m_nMaxWidth)) return ERR_MEMORY;

        //ラインサイズバッファ

        if(!m_memLineSize.alloc(m_nMaxHeight * sizeof(WORD))) return ERR_MEMORY;
    }

    return ERR_SUCCESS;
}

//! 各レイヤイメージ読み込み開始
/*!
    必ず各レイヤごとに行う。@n
    範囲が空の場合は読み込み処理は行わないこと。

    @param prcs イメージ範囲が入る（NULL でなし）
    @return イメージ範囲があるかどうか（空でないか）
*/

BOOL AXPSDLoad::beginLayerImageEach(AXRectSize *prcs)
{
    if(!m_pLayerWork)
        //先頭レイヤ
        m_pLayerWork = (LAYERWORKINFO *)m_memLayerWork.getBuf();
    else
    {
        DWORD size = 0;
        int i;

        //前回のイメージ先頭位置から、全チャンネル分移動

        for(i = 0; i < m_pLayerWork->nChCnt; i++)
            size += m_pLayerWork->dwChSize[i];

        m_file.seekTop(m_dwFilePosImg + size);

        //

        m_pLayerWork++;
    }

    //イメージ先頭位置取得

    m_dwFilePosImg = m_file.getPosition();

    //範囲

    if(prcs)
    {
        prcs->x = m_pLayerWork->nLeft;
        prcs->y = m_pLayerWork->nTop;
        prcs->w = m_pLayerWork->nWidth;
        prcs->h = m_pLayerWork->nHeight;
    }

    return (m_pLayerWork->nWidth && m_pLayerWork->nHeight);
}


//=============================
// レイヤイメージチャンネル
//=============================


//! レイヤイメージチャンネル読み込み開始
/*!
    @param chID 読み込むチャンネルID
    @return 指定チャンネルがなかった場合は ERR_NO_CHANNEL
*/

int AXPSDLoad::beginLayerImageCh(WORD chID)
{
    int i,ret;
    DWORD pos;
    WORD comp;

    //------------- チャンネル検索

    for(i = 0, pos = 0; i < m_pLayerWork->nChCnt; i++)
    {
        if(m_pLayerWork->wChID[i] == chID)
            break;

        pos += m_pLayerWork->dwChSize[i];
    }

    if(i >= m_pLayerWork->nChCnt) return ERR_NO_CHANNEL;

    m_nChNo = i;

    //指定チャンネルの位置へ移動

    m_file.seekTop(m_dwFilePosImg + pos);

    //--------------

    //圧縮フラグ (2b) [0:無圧縮 1:PackBits]

    if(!m_file.readWORD(&comp)) return ERR_FILEDAT;

    if(comp != 0 && comp != 1) return ERR_FORMAT;

    m_nCompType = comp;

    //PackBits 時、ラインサイズ

    if(m_nCompType == 1)
    {
        ret = _read_lineSize(m_pLayerWork->nHeight);
        if(ret) return ret;
    }

    //セット

    m_dwChSize  = m_pLayerWork->dwChSize[m_nChNo] - 2;  //圧縮フラグの分を引く
    m_nNowY     = 0;
    m_dwBufSize = 0;
    m_dwBufPos  = 0;

    return ERR_SUCCESS;
}

//! レイヤイメージ・各チャンネルY1列読み込み

int AXPSDLoad::readLayerImageChLine()
{
    return _read_chLine(m_pLayerWork->nWidth);
}


//==========================
// レイヤ情報
//==========================


//! レイヤ情報読み込み
/*!
    一番下層のレイヤから順にレイヤ数だけ実行。
*/

int AXPSDLoad::readLayerInfo(LAYERINFO *pInfo)
{
    AXBuf buf;
    int i,chcnt;
    BYTE byte;

    //=============== 基本データ

    //------ （範囲 〜 チャンネル数 まで読み込み）

    if(!m_file.readSize(m_memBuf, 18)) return ERR_FILEDAT;

    buf.init(m_memBuf, 18, AXBuf::ENDIAN_BIG);

    //範囲 (4b x 4)

    buf.getDWORD(&pInfo->nTop);
    buf.getDWORD(&pInfo->nLeft);
    buf.getDWORD(&pInfo->nBottom);
    buf.getDWORD(&pInfo->nRight);

    //作業用データ

    m_pLayerWork->nLeft   = pInfo->nLeft;
    m_pLayerWork->nTop    = pInfo->nTop;
    m_pLayerWork->nWidth  = pInfo->nRight - pInfo->nLeft;
    m_pLayerWork->nHeight = pInfo->nBottom - pInfo->nTop;

    if(m_pLayerWork->nWidth > m_nMaxWidth)   m_nMaxWidth = m_pLayerWork->nWidth;
    if(m_pLayerWork->nHeight > m_nMaxHeight) m_nMaxHeight = m_pLayerWork->nHeight;

    //チャンネル数 (2b)

    chcnt = buf.getWORD();
    if(chcnt > 5) return ERR_FORMAT;

    m_pLayerWork->nChCnt = chcnt;

    //------ （チャンネルデータ 〜 拡張データサイズ まで読み込み）

    if(!m_file.readSize(m_memBuf, chcnt * 6 + 16)) return ERR_FILEDAT;

    buf.init(m_memBuf, chcnt * 6 + 16, AXBuf::ENDIAN_BIG);

    //各チャンネルデータ (6b x チャンネル数)

    for(i = 0; i < chcnt; i++)
    {
        buf.getWORD(&m_pLayerWork->wChID[i]);
        buf.getDWORD(&m_pLayerWork->dwChSize[i]);
    }

    //"8BIM" (4b)

    if(!buf.getStrCompare("8BIM")) return ERR_FORMAT;

    //合成モード (4b)

    buf.getDWORD(&pInfo->dwBlend);

    //不透明度 (1b)

    buf.getBYTE(&pInfo->btOpacity);

    //クリップ (1b)

    buf.seek(1);

    //フラグ (1b) [1bit:非表示]

    buf.getBYTE(&byte);

    pInfo->bHide = (byte & 2)? TRUE: FALSE;

    //0

    buf.seek(1);

    //================== 拡張データ

    DWORD exsize,size,pos;
    BYTE blen;

    //拡張データサイズ

    buf.getDWORD(&exsize);

    //拡張データ

    if(exsize)
    {
        pos = m_file.getPosition();

        //レイヤマスクデータ

        if(!m_file.readDWORD(&size)) return ERR_FILEDAT;
        m_file.seekCur(size);

        //合成データ

        if(!m_file.readDWORD(&size)) return ERR_FILEDAT;
        m_file.seekCur(size);

        //レイヤ名（m_memBuf に読み込んで、ポインタを szName にセット）

        if(m_file.read(&blen, 1) != 1) return ERR_FILEDAT;

        if(!m_file.readSize(m_memBuf, blen)) return ERR_FILEDAT;
        *((LPBYTE)m_memBuf + blen) = 0;

        pInfo->szName = m_memBuf;

        //

        m_file.seekTop(pos + exsize);
    }

    //--------

    m_pLayerWork++;

    return ERR_SUCCESS;
}


//==========================
// 画像リソース
//==========================


//! 画像リソース読み込み開始（必須）
/*!
    openFile() の直後に行うこと。
*/

int AXPSDLoad::beginRes()
{
    //サイズ

    if(!m_file.readDWORD(&m_dwBlockSize)) return ERR_FILEDAT;

    m_dwFilePos = m_file.getPosition();

    return ERR_SUCCESS;
}

//! 画像リソース読み込み終了（必須）

void AXPSDLoad::endRes()
{
    m_file.seekTop(m_dwFilePos + m_dwBlockSize);
}

//! 画像リソース：カレントレイヤ番号読み込み
/*!
    @return -1 で失敗。一番下層のレイヤを 0 とする。
*/

int AXPSDLoad::readRes_curlayer()
{
    DWORD size;
    WORD wd;

    if(!_search_res(0x0400, &size)) return -1;

    if(size != 2) return -1;

    if(!m_file.readWORD(&wd)) return -1;

    return wd;
}

//! 画像リソース：解像度読み込み

BOOL AXPSDLoad::readRes_resolution(int *pDPIH,int *pDPIV)
{
    DWORD size,val;
    WORD type;
    LPBYTE p;

    if(!_search_res(0x03ed, &size)) return FALSE;

    if(size != 16) return FALSE;

    if(!m_file.readSize(m_memBuf, size)) return FALSE;

    p = m_memBuf;

    //h

    AXGetDWORDBE(p, &val);
    AXGetWORDBE(p + 4, &type);     //[1] pixel/inch [2]pixel/cm

    if(type == 1)
        *pDPIH = val >> 16;
    else
        *pDPIH = (int)((double)val / (1 << 16) * 0.0254 + 0.5);

    //v

    AXGetDWORDBE(p + 8, &val);
    AXGetWORDBE(p + 12, &type);

    if(type == 1)
        *pDPIV = val >> 16;
    else
        *pDPIV = (int)((double)val / (1 << 16) * 0.0254 + 0.5);

    return TRUE;
}


//==========================
// サブ
//==========================


//! ヘッダ読み込み

int AXPSDLoad::_read_header()
{
    AXBuf buf;
    WORD wd;

    if(!m_file.readSize(m_memBuf, 26)) return ERR_FILEDAT;

    buf.init(m_memBuf, 26, AXBuf::ENDIAN_BIG);

    //識別子 (4b)

    if(!buf.getStrCompare("8BPS")) return ERR_FORMAT;

    //バージョン (2b) : "1" のみ

    buf.getWORD(&wd);
    if(wd != 1) return ERR_FORMAT;

    //予約 (6b)

    buf.seek(6);

    //一枚絵のチャンネル数 (2b)

    buf.getWORD(&wd);

    m_nImgChCnt = wd;

    //高さ (4b)

    buf.getDWORD(&m_nImgH);

    if(m_nImgH < 1 || m_nImgH > 30000) return ERR_FILEDAT;

    //幅 (4b)

    buf.getDWORD(&m_nImgW);

    if(m_nImgW < 1 || m_nImgW > 30000) return ERR_FILEDAT;

    //ビット数 (2b) : 1bit or 8bit のみ対応

    buf.getWORD(&wd);

    if(wd != 1 && wd != 8) return ERR_FORMAT;

    m_nBits = wd;

    //カラーモード (2b)

    buf.getWORD(&wd);

    if(wd != axpsd::COLMODE_BITMAP && wd != axpsd::COLMODE_GRAYSCALE && wd != axpsd::COLMODE_RGB)
        return ERR_FORMAT;

    m_nColMode = wd;

    //------------ ビット数＋カラーモード判定

    //1bit

    if(m_nBits == 1 && m_nColMode != axpsd::COLMODE_BITMAP) return ERR_FORMAT;

    //8bit は グレイスケールかRGBのみ

    if(m_nBits == 8 && m_nColMode != axpsd::COLMODE_GRAYSCALE && m_nColMode != axpsd::COLMODE_RGB)
        return ERR_FORMAT;

    return ERR_SUCCESS;
}

//! 画像リソースをIDから検索
/*!
    見つかった場合、ファイル位置はデータ部分の位置。
*/

BOOL AXPSDLoad::_search_res(WORD wID,DWORD *pSize)
{
    DWORD sig,size,allsize = 0;
    WORD id;
    BYTE byte;
    int len;

    m_file.seekTop(m_dwFilePos);

    while(allsize < m_dwBlockSize)
    {
        //識別子 (4b)

        if(!m_file.readDWORD(&sig)) return FALSE;

        if(sig != (('8'<<24)|('B'<<16)|('I'<<8)|'M')) return FALSE;

        //ID (2b)

        if(!m_file.readWORD(&id)) return FALSE;

        //名前（2Byte 単位）

        if(m_file.read(&byte, 1) != 1) return FALSE;

        len = (byte + 2) & (~1);
        m_file.seekCur(len - 1);

        //データサイズ

        if(!m_file.readDWORD(&size)) return FALSE;

        //-----------

        if(id == wID)
        {
            *pSize = size;
            return TRUE;
        }
        else
        {
            //データは 2Byte 単位（データサイズの値は 1Byte 単位）
            if(size & 1) size++;

            m_file.seekCur(size);

            allsize += 4 + 2 + len + 4 + size;
        }
    }

    return FALSE;
}

//! PackBits 時、ラインサイズ読み込み＆読み込みバッファリサイズ
/*!
    @param pChSize ラインサイズをすべて加算したチャンネルサイズが入る（NULL でなし）
*/

int AXPSDLoad::_read_lineSize(int height,DWORD *pChSize)
{
    DWORD max,allsize = 0;
    WORD val;
    int i;
    LPWORD pw;

    if(m_nCompType == 1 && height)
    {
        //バッファに読み込み

        if(!m_file.readSize(m_memLineSize, height * sizeof(WORD)))
            return ERR_FILEDAT;

        //ビッグエンディアン -> リトルエンディアン ＋ 最大ラインサイズ計算

        pw  = m_memLineSize;
        max = 0;

        for(i = height; i > 0; i--, pw++)
        {
            val = (*((LPBYTE)pw) << 8) | *((LPBYTE)pw + 1);

            *pw = val;

            if(val > max) max = val;

            allsize += val;
        }

        //読み込みバッファを最大ラインサイズ以上確保

        if(max > m_memBuf.getSize())
        {
            if(!m_memBuf.alloc(max)) return ERR_MEMORY;
        }
    }

    if(pChSize) *pChSize = allsize;

    return ERR_SUCCESS;
}

//! チャンネルイメージY1列読み込み

int AXPSDLoad::_read_chLine(int width)
{
    int ret,linesize;

    if(m_nCompType == 0)
    {
        //無圧縮時

        if(!m_file.readSize(m_memLine, width))
            return ERR_FILEDAT;
    }
    else if(m_nCompType == 1)
    {
        //PackBits

        ret = _read_ch_buf();
        if(ret != ERR_SUCCESS) return ret;

        linesize = *((LPWORD)m_memLineSize + m_nNowY);

        if(!_decPackBits(width, linesize))
            return ERR_DECODE;

        m_dwBufPos += linesize;
    }

    m_nNowY++;

    return ERR_SUCCESS;
}

//! チャンネルデータ読み込み (PackBits)
/*
    m_dwBufPos  : 現在の処理バッファ位置
    m_dwBufSize : バッファに読み込んだサイズ
    m_dwChSize  : チャンネルデータ残りサイズ（残りの読み込みサイズ）
*/

int AXPSDLoad::_read_ch_buf()
{
    DWORD size,remain;
    int linesize;
    LPBYTE pbuf;

    //現在のYラインサイズ

    linesize = *((LPWORD)m_memLineSize + m_nNowY);

    //ラインサイズ分のデータがバッファにある

    if(m_dwBufPos + linesize <= m_dwBufSize) return ERR_SUCCESS;

    //------- 読み込み

    pbuf = m_memBuf;

    remain = m_dwBufSize - m_dwBufPos;

    size = m_dwChSize;

    if(size > m_memBuf.getSize() - remain)
        size = m_memBuf.getSize() - remain;

    //バッファに残っているデータを移動

    if(remain)
    {
        ::memmove(pbuf, pbuf + m_dwBufPos, remain);

        m_dwBufSize = remain;
    }
    else
        m_dwBufSize = 0;

    //読み込み

    if(!m_file.readSize(pbuf + m_dwBufSize, size))
        return ERR_FILEDAT;

    m_dwBufSize += size;
    m_dwBufPos  = 0;
    m_dwChSize  -= size;

    return ERR_SUCCESS;
}

//! PackBits展開

BOOL AXPSDLoad::_decPackBits(int dstsize,int srcsize)
{
    int len,i;
    LPBYTE pSrc,pDst,pDstEnd,pSrcEnd;
    BYTE val;

    if(dstsize == 0) return TRUE; //サイズ0の場合がある

    pDst    = m_memLine;
    pSrc    = (LPBYTE)m_memBuf + m_dwBufPos;
    pDstEnd = pDst + dstsize;
    pSrcEnd = pSrc + srcsize;

    while(pDst < pDstEnd && pSrc < pSrcEnd)
    {
        len = *((char *)pSrc);
        pSrc++;

        //

        if(len == 128)
            continue;
        else if(len <= 0)
        {
            //連続

            len = -len + 1;

            if(pDst + len > pDstEnd) return FALSE;
            if(pSrc >= pSrcEnd) return FALSE;

            val = *(pSrc++);

            for(i = len; i > 0; i--)
                *(pDst++) = val;
        }
        else
        {
            //非連続

            len++;

            if(pDst + len > pDstEnd) return FALSE;
            if(pSrc + len > pSrcEnd) return FALSE;

            for(i = len; i > 0; i--)
                *(pDst++) = *(pSrc++);
        }
    }

    return TRUE;
}
