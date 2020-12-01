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

#include <stdio.h>
#include <setjmp.h>
#include <jpeglib.h>

#include "AXJPEG.h"

#include "AXString.h"
#include "AXByteString.h"
#include "AXUtil.h"


//----------------------

#define JCOMMON  ((jpeg_common_struct *)m_pInfo)
#define JDEC     ((jpeg_decompress_struct *)m_pInfo)
#define JENC     ((jpeg_compress_struct *)m_pInfo)
#define JERR     ((JPEGMYERR *)m_pErr)

struct JPEGMYERR
{
    struct jpeg_error_mgr pub;
    jmp_buf jmp_buffer;
};

void jpeg_myerror_exit(j_common_ptr cinfo)
{
    JPEGMYERR *perr = (JPEGMYERR *)cinfo->err;

    (*cinfo->err->output_message)(cinfo);

    ::longjmp(perr->jmp_buffer, 1);
}

//-----------------------

/*!
    @class AXJPEG
    @brief JPEG画像読み書きクラス

    - Y1列毎に読み書き。
    - JPEGの生データはRGB順(各1Byte)に並ぶ。グレイスケールの場合は1pixel=1Byte。

    @ingroup format
*/
/*!
    @var AXJPEG::SAVEINFO::nQuality
    @brief 品質（1〜100）
    @var AXJPEG::SAVEINFO::nSamplingType
    @brief サンプリングタイプ（ダウンサンプリング方法）

    @var AXJPEG::SAMP_444
    @brief 4:4:4。最高品質（ダウンサンプリングなし）
    @var AXJPEG::SAMP_422
    @brief 4:2:2。中品質
    @var AXJPEG::SAMP_411
    @brief 4:1:1。低品質（デフォルト）
*/


AXJPEG::~AXJPEG()
{
    close();
}

AXJPEG::AXJPEG()
{
    m_bEncode = FALSE;
    m_pInfo   = NULL;
    m_pErr    = NULL;
    m_pFile   = NULL;
    m_pRawBuf = NULL;
}


//=======================
//解放
//=======================


//! 閉じる

void AXJPEG::close()
{
    //JPEG終了

    if(m_pInfo)
    {
        if(m_bEncode)
        {
            jpeg_finish_compress(JENC);
            jpeg_destroy_compress(JENC);
        }
        else
        {
            jpeg_finish_decompress(JDEC);
            jpeg_destroy_decompress(JDEC);
        }
    }

    //他

    _close();
}

//! 通常解放処理

void AXJPEG::_close()
{
    //ファイル閉じる

    if(m_pFile)
    {
        ::fclose((FILE *)m_pFile);
        m_pFile = NULL;
    }

    //解放

    AXFree((void **)&m_pInfo);
    AXFree((void **)&m_pErr);
    AXFree((void **)&m_pRawBuf);
}

//! 初期化途中のエラー時の解放

void AXJPEG::_closeErr()
{
    if(m_bEncode)
        jpeg_destroy_compress(JENC);
    else
        jpeg_destroy_decompress(JDEC);

    _close();
}


//=============================
//共通処理
//=============================


//! 初期化

BOOL AXJPEG::_init()
{
    int size;

    //JPEG 構造体確保

    size = (m_bEncode)? sizeof(jpeg_compress_struct): sizeof(jpeg_decompress_struct);

    m_pInfo = AXMalloc(size);
    m_pErr  = AXMalloc(sizeof(JPEGMYERR));

    if(!m_pInfo || !m_pErr)
    {
        _close();
        return FALSE;
    }

    //エラー

    JCOMMON->err = jpeg_std_error(&JERR->pub);

    JERR->pub.error_exit = jpeg_myerror_exit;

    return TRUE;
}

//! イメージ用メモリ確保

BOOL AXJPEG::_mallocBuf(DWORD size)
{
    m_pRawBuf = (LPBYTE)AXMalloc(size);

    if(m_pRawBuf)
        return TRUE;
    else
    {
        close();
        return FALSE;
    }
}


//=============================
//読み込み
//=============================


//! 読み込み用にファイルを開く

BOOL AXJPEG::openFileLoad(const AXString &filename)
{
    AXByteString strb;

    close();

    m_bEncode = FALSE;

    //ファイル開く

    filename.toLocal(&strb);

    m_pFile = (LPVOID)::fopen(strb, "rb");
    if(!m_pFile) return FALSE;

    //初期化

    if(!_init()) return FALSE;

    //エラー時用

    if(::setjmp(JERR->jmp_buffer))
    {
        /* エラー発生時はここに来る */

        _closeErr();
        return FALSE;
    }

    //JPEG初期化

    jpeg_create_decompress(JDEC);

    jpeg_stdio_src(JDEC, (FILE *)m_pFile);

    //ヘッダ読み込み

    jpeg_save_markers(JDEC, JPEG_APP0, 0xffff);

    if(jpeg_read_header(JDEC, TRUE) == JPEG_SUSPENDED)
    {
        _closeErr();
        return FALSE;
    }

    //展開開始

    if(!jpeg_start_decompress(JDEC))
    {
        _closeErr();
        return FALSE;
    }

    //情報

    m_nWidth  = JDEC->output_width;
    m_nHeight = JDEC->output_height;

    m_nDPI_H = m_nDPI_V = -1;

    //APP0マーカー

    jpeg_marker_struct *plist;
    LPBYTE p;

    for(plist = JDEC->marker_list; plist; plist = plist->next)
    {
        if(plist->marker == JPEG_APP0 && plist->data_length >= 14)
        {
            p = plist->data;

            if(p[0] == 'J' && p[1] == 'F' && p[2] == 'I' && p[3] == 'F')
            {
                m_nDPI_H = (p[8] << 8) | p[9];
                m_nDPI_V = (p[10] << 8) | p[11];

                if(p[7] == 2)
                {
                    //dpcm -> dpi

                    m_nDPI_H = AXDPMtoDPI(m_nDPI_H * 100);
                    m_nDPI_V = AXDPMtoDPI(m_nDPI_V * 100);
                }
            }
        }
    }

    //1行分メモリ確保

    if(!_mallocBuf(m_nWidth * JDEC->out_color_components))
        return FALSE;

    return TRUE;
}

//! Y1列分読み込み
/*!
    @return Y位置。-1 でエラー。
*/

int AXJPEG::readLine()
{
    int y = JDEC->output_scanline;

    if(y >= m_nHeight) return -1;

    if(!jpeg_read_scanlines(JDEC, (JSAMPARRAY)&m_pRawBuf, 1))
        return -1;
    else
        return y;
}

//! Y1列分を32bitイメージに変換

void AXJPEG::lineTo32bit(LPDWORD pDst)
{
    LPBYTE ps = m_pRawBuf;
    int i;

    if(JDEC->out_color_components == 1)
    {
        //グレイスケール

        for(i = m_nWidth; i > 0; i--, ps++)
            *(pDst++) = _RGBA(*ps, *ps, *ps, 255);
    }
    else if(JDEC->out_color_components == 3)
    {
        //RGB

        for(i = m_nWidth; i > 0; i--, ps += 3)
            *(pDst++) = _RGBA(ps[0], ps[1], ps[2], 255);
    }
}

//! 読み込みイメージがグレイスケールか

BOOL AXJPEG::isGrayscale()
{
    if(JDEC)
        return (JDEC->out_color_components == 1);
    else
        return FALSE;
}


//============================
//書き込み
//============================


//! 書き込み用にファイル開く

BOOL AXJPEG::openFileSave(const AXString &filename,AXJPEG::SAVEINFO *pInfo)
{
    AXByteString strb;

    close();

    //

    m_bEncode   = TRUE;
    m_nWidth    = pInfo->nWidth;
    m_nHeight   = pInfo->nHeight;

    //ファイル開く

    filename.toLocal(&strb);

    m_pFile = (LPVOID)::fopen(strb, "wb");
    if(!m_pFile) return FALSE;

    //初期化

    if(!_init()) return FALSE;

    //エラー時用

    if(::setjmp(JERR->jmp_buffer))
    {
        /* エラー発生時はここに来る */

        _closeErr();
        return FALSE;
    }

    //JPEG初期化

    jpeg_create_compress(JENC);

    jpeg_stdio_dest(JENC, (FILE *)m_pFile);

    //パラメータ

    JENC->image_width       = pInfo->nWidth;
    JENC->image_height      = pInfo->nHeight;
    JENC->input_components  = 3;
    JENC->in_color_space    = JCS_RGB;

    jpeg_set_defaults(JENC);
    jpeg_set_colorspace(JENC, JCS_YCbCr);

    JENC->write_JFIF_header = TRUE;
    JENC->density_unit      = 1;
    JENC->X_density         = pInfo->nDPI_H;
    JENC->Y_density         = pInfo->nDPI_V;

    _setDownsamp(pInfo->nSamplingType);

    jpeg_set_quality(JENC, pInfo->nQuality, TRUE);

    //圧縮開始

    jpeg_start_compress(JENC, TRUE);

    //メモリ確保

    if(!_mallocBuf(m_nWidth * 3))
        return FALSE;

    return TRUE;
}

//! ダウンサンプリング設定

void AXJPEG::_setDownsamp(int type)
{
    int yh,yv;

    switch(type)
    {
        case SAMP_444: yh = 1, yv = 1; break;
        case SAMP_422: yh = 2, yv = 1; break;

        default: yh = yv = 2; break;
    }

    //セット

    JENC->comp_info[0].h_samp_factor = yh;
    JENC->comp_info[0].v_samp_factor = yv;
    JENC->comp_info[1].h_samp_factor = 1;
    JENC->comp_info[1].v_samp_factor = 1;
    JENC->comp_info[2].h_samp_factor = 1;
    JENC->comp_info[2].v_samp_factor = 1;
}

//! Y1列分のイメージを出力

void AXJPEG::putLine()
{
    jpeg_write_scanlines(JENC, (JSAMPARRAY)&m_pRawBuf, 1);
}

//! 32bitイメージからY1列分出力

void AXJPEG::putLineFrom32bit(const DWORD *pSrc)
{
    int i;
    DWORD col;
    LPBYTE pd = m_pRawBuf;

    for(i = m_nWidth; i > 0; i--, pd += 3)
    {
        col = *(pSrc++);

        pd[0] = _GETR(col);
        pd[1] = _GETG(col);
        pd[2] = _GETB(col);
    }

    putLine();
}
