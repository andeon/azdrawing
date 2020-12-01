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
/*
    CLayerImg [filter1] - フィルタ処理

    - フィルタプレビュー時は、イメージが空の状態で setPixel_create2 が点関数。
      実際の描画時は、setPixelDraw で COLSETF_OVERWRITE による上書き。
    - プレビューなしの場合は直接 setPixelDraw を使っても良い。
    - CProgressDlg* はフィルタプレビュー時は NULL なので注意。
    - 負の値の位置も正しく描画できているか確認すること。
*/

#include <math.h>
#include <stdlib.h>

#include "CLayerImg.h"

#include "CImage8.h"
#include "CImage32.h"
#include "CProgressDlg.h"
#include "defStruct.h"

#include "AXRand.h"
#include "AXMem.h"


//----------------

typedef struct
{
    int x,y,cnt,col;
}FILTERDAT_CRYSTAL;

//----------------


//=========================
//サブ
//=========================


//! モザイク平均色バッファ取得
/*!
    各ブロックの平均色を取得し、pmem にメモリ確保してセット。
*/

BOOL CLayerImg::_filter_sub_getMozaicCol(const CLayerImg &imgSrc,const RECTANDSIZE &rs,
    int nSize,BOOL bClip,AXMem *pmem,AXSize *pszBuf)
{
    int x,y,xx,yy,c,div,half,xcnt,ycnt;
    LPBYTE p;

    //確保

    xcnt = (rs.w + nSize - 1) / nSize;
    ycnt = (rs.h + nSize - 1) / nSize;

    if(!pmem->alloc(xcnt * ycnt)) return FALSE;

    //

    div  = nSize * nSize;
    half = div / 2;
    p    = *pmem;

    for(y = rs.y1; y <= rs.y2; y += nSize)
    {
        for(x = rs.x1; x <= rs.x2; x += nSize)
        {
            c = 0;

            for(yy = 0; yy < nSize; yy++)
            {
                for(xx = 0; xx < nSize; xx++)
                    c += imgSrc.getPixel(x + xx, y + yy, bClip);
            }

            *(p++) = (c + half) / div;
        }
    }

    //

    pszBuf->w = xcnt;
    pszBuf->h = ycnt;

    return TRUE;
}


//=========================
//色操作
//=========================


//! 色反転

void CLayerImg::filter_col_inverse(const RECTANDSIZE &rs,CProgressDlg *pdlg)
{
    int x,y,c;

    for(y = rs.y1; y <= rs.y2; y++)
    {
        for(x = rs.x1; x <= rs.x2; x++)
        {
            c = 255 - getPixel(x, y);

            setPixelDraw(x, y, c);
        }

        pdlg->incProgSub();
    }
}

//! テクスチャ適用

void CLayerImg::filter_setTexture(const RECTANDSIZE &rs,CProgressDlg *pdlg,CImage8 *pTexture)
{
    int x,y,c;

    for(y = rs.y1; y <= rs.y2; y++)
    {
        for(x = rs.x1; x <= rs.x2; x++)
        {
            c = getPixel(x, y);

            if(c)
            {
                c = (c * pTexture->getPixelTexture(x, y) + 127) / 255;

                setPixelDraw(x, y, c);
            }
        }

        pdlg->incProgSub();
    }
}

//! レベル補正
/*
    pVal : [0]入力最小 [1]入力中間 [2]入力最大 [3]出力最小 [4]出力最大
*/

BOOL CLayerImg::filter_col_level(const CLayerImg &imgSrc,RECTANDSIZE &rs,CProgressDlg *pdlg,
                                 LPINT pVal)
{
    LPBYTE pTable;
    int x,y,c,outd,outd_hf;
    void (CLayerImg::*funcPix)(int,int,BYTE) = m_pinfo->funcDrawPixel;

    //テーブル作成

    pTable = (LPBYTE)::malloc(256);
    if(!pTable) return FALSE;

    outd    = pVal[4] - pVal[3];
    outd_hf	= (int)(outd * 0.5 + 0.5);

    for(x = 0; x < 256; x++)
    {
        //入力値制限

        if(x < pVal[0]) c = pVal[0];
        else if(x > pVal[2]) c = pVal[2];
        else c = x;

        //補正

        if(c < pVal[1])
            c = (int)((double)(c - pVal[0]) * outd_hf / (pVal[1] - pVal[0]) + 0.5);
        else
            c = (int)((double)(c - pVal[1]) * (outd - outd_hf) / (pVal[2] - pVal[1]) + outd_hf + 0.5);

        pTable[x] = c + pVal[3];
    }

    //描画

    for(y = rs.y1; y <= rs.y2; y++)
    {
        for(x = rs.x1; x <= rs.x2; x++)
        {
            c = imgSrc.getPixel(x, y);

            (this->*funcPix)(x, y, pTable[c]);
        }

        if(pdlg) pdlg->incProgSub();
    }

    ::free(pTable);

    return TRUE;
}

//! ガンマ補正

BOOL CLayerImg::filter_col_gamma(const CLayerImg &imgSrc,RECTANDSIZE &rs,CProgressDlg *pdlg,int nGamma)
{
    int x,y,c;
    LPBYTE pTable;
    double gamma;
    void (CLayerImg::*funcPix)(int,int,BYTE) = m_pinfo->funcDrawPixel;

    //テーブル作成

    pTable = (LPBYTE)::malloc(256);
    if(!pTable) return FALSE;

    gamma = 1.0 / (nGamma * 0.01);

    for(x = 0; x < 256; x++)
    {
        y = (int)(::pow(x / 255.0, gamma) * 255.0);
        if(y < 0) y = 0; else if(y > 255) y = 255;

        pTable[x] = y;
    }

    //

    for(y = rs.y1; y <= rs.y2; y++)
    {
        for(x = rs.x1; x <= rs.x2; x++)
        {
            c = imgSrc.getPixel(x, y);

            (this->*funcPix)(x, y, pTable[c]);
        }

        if(pdlg) pdlg->incProgSub();
    }

    //

    ::free(pTable);

    return TRUE;
}

//! 2値化

BOOL CLayerImg::filter_2col(const CLayerImg &imgSrc,RECTANDSIZE &rs,CProgressDlg *pdlg,int nBorder)
{
    int x,y,c;
    void (CLayerImg::*funcPix)(int,int,BYTE) = m_pinfo->funcDrawPixel;

    for(y = rs.y1; y <= rs.y2; y++)
    {
        for(x = rs.x1; x <= rs.x2; x++)
        {
            c = imgSrc.getPixel(x, y);

            if(c < nBorder) c = 0; else c = 255;

            (this->*funcPix)(x, y, c);
        }

        if(pdlg) pdlg->incProgSub();
    }

    return TRUE;
}

//! 2値化（ディザ）

BOOL CLayerImg::filter_2col_dither(const CLayerImg &imgSrc,RECTANDSIZE &rs,CProgressDlg *pdlg,int nType)
{
    int x,y,c,ccmp,shift,mask;
    BYTE val_bayer2x2[4] = {0,2*64,3*64,1*64},
        val_bayer4x4[16] = {0,8*16,2*16,10*16, 12*16,4*16,14*16,6*16, 3*16,11*16,1*16,9*16, 15*16,7*16,13*16,5*16},
        val_screw4x4[16] = {13*16,7*16,6*16,12*16, 8*16,1*16,0,5*16, 9*16,2*16,3*16,4*16, 14*16,10*16,11*16,15*16},
        val_half4x4[16]  = {10*16,4*16,6*16,8*16, 12*16,0,2*16,14*16, 7*16,9*16,11*16,5*16, 3*16,15*16,13*16,1*16};
    LPBYTE ppat;
    void (CLayerImg::*funcPix)(int,int,BYTE) = m_pinfo->funcDrawPixel;

    //

    shift = 2;

    switch(nType)
    {
        case 0: ppat = val_bayer2x2; shift = 1; break;
        case 1: ppat = val_bayer4x4; break;
        case 2: ppat = val_screw4x4; break;
        case 3: ppat = val_half4x4; break;
    }

    mask = (1 << shift) - 1;

    //

    for(y = rs.y1; y <= rs.y2; y++)
    {
        for(x = rs.x1; x <= rs.x2; x++)
        {
            c = imgSrc.getPixel(x, y);

            //比較値

            if(nType == 4)
                ccmp = m_pRand->getRange(0, 254);
            else
                ccmp = ppat[((y & mask) << shift) + (x & mask)];

            //比較

            if(c <= ccmp)
                c = 0;
            else
                c = 255;

            //

            (this->*funcPix)(x, y, c);
        }

        if(pdlg) pdlg->incProgSub();
    }

    return TRUE;
}


//=========================
//その他
//=========================


//! 他のレイヤとアルファ操作（プレビューなし）

BOOL CLayerImg::filter_layercol(const RECTANDSIZE &rs,CProgressDlg *pdlg,const CLayerImg &imgRef,int nType)
{
    int x,y,c,c2;

    for(y = rs.y1; y <= rs.y2; y++)
    {
        for(x = rs.x1; x <= rs.x2; x++)
        {
            c  = getPixel(x, y);
            c2 = imgRef.getPixel(x, y);

            switch(nType)
            {
                case 0:     //透明部分は透明に
                    if(c2 == 0) c = 0;
                    break;
                case 1:     //不透明部分は透明に
                    if(c2 != 0) c = 0;
                    break;
                case 2:     //合成
                    c = (((c + c2) << 5) - ((c * c2 << 5) / 255) + (1 << 4)) >> 5;
                    break;
                case 3:     //コピー
                    c = c2;
                    break;
                case 4:     //足す
                    c += c2;
                    if(c > 255) c = 255;
                    break;
                case 5:     //引く
                    c -= c2;
                    if(c < 0) c = 0;
                    break;
                case 6:     //乗算
                    c = ((c * c2 << 5) / 255 + (1 << 4)) >> 5;
                    break;
            }

            setPixelDraw(x, y, c);
        }

        pdlg->incProgSub();
    }

    return TRUE;
}

//! モザイク

BOOL CLayerImg::filter_mozaic(const CLayerImg &imgSrc,RECTANDSIZE &rs,CProgressDlg *pdlg,
                              int nSize,BOOL bClip)
{
    int x,y,c,xx,yy,div,half;
    void (CLayerImg::*funcPix)(int,int,BYTE) = m_pinfo->funcDrawPixel;

    div  = nSize * nSize;
    half = div / 2;

    if(bClip)
    {
        if(!clipRectInCanvas(&rs)) return TRUE;
    }

    //

    if(pdlg)
        pdlg->beginProgSub(50, (rs.h + nSize - 1) / nSize, TRUE);

    for(y = rs.y1; y <= rs.y2; y += nSize)
    {
        for(x = rs.x1; x <= rs.x2; x += nSize)
        {
            //平均

            c = 0;

            for(yy = 0; yy < nSize; yy++)
            {
                for(xx = 0; xx < nSize; xx++)
                    c += imgSrc.getPixel(x + xx, y + yy, bClip);
            }

            c = (c + half) / div;

            //セット

            for(yy = 0; yy < nSize; yy++)
            {
                if(y + yy > rs.y2) break;

                for(xx = 0; xx < nSize; xx++)
                {
                    if(x + xx > rs.x2) break;

                    (this->*funcPix)(x + xx, y + yy, c);
                }
            }
        }

        if(pdlg) pdlg->incProgSub();
    }

    return TRUE;
}

//! モザイク（網目）

BOOL CLayerImg::filter_mozaic2(const CLayerImg &imgSrc,RECTANDSIZE &rs,CProgressDlg *pdlg,
                              int nSize,BOOL bClip)
{
    AXMem memBuf;
    int x,y,xx,yy,c,pitch;
    AXSize szBuf;
    LPBYTE p,p2;
    char cPat[4] = {0,1,2,-1};
    void (CLayerImg::*funcPix)(int,int,BYTE) = m_pinfo->funcDrawPixel;

    if(bClip)
    {
        if(!clipRectInCanvas(&rs)) return TRUE;
    }

    //バッファに各ブロックの平均色取得

    if(!_filter_sub_getMozaicCol(imgSrc, rs, nSize, bClip, &memBuf, &szBuf))
        return FALSE;

    //--------- バッファ色を網目に調整

    //横ブロック

    p2 = memBuf;

    for(y = 0; y < szBuf.h; y++, p2 += szBuf.w)
    {
        x = cPat[y & 3];
        p = p2 + x;

        for(; x < szBuf.w - 1; x += 4, p += 4)
        {
            if(x >= 0)
            {
                c = (*p + p[1]) / 2;
                *p = c;
                p[1] = c;
            }
        }
    }

    //縦ブロック

    p2    = memBuf;
    pitch = szBuf.w * 4;

    for(x = 0; x < szBuf.w; x++, p2++)
    {
        y = cPat[(x + 1) & 3];
        p = p2 + y * szBuf.w;

        for(; y < szBuf.h - 1; y += 4, p += pitch)
        {
            if(y >= 0)
            {
                c = (*p + p[szBuf.w]) / 2;
                *p = c;
                p[szBuf.w] = c;
            }
        }
    }

    //--------- 色セット

    if(pdlg) pdlg->beginProgSub(50, szBuf.h, TRUE);

    p = memBuf;

    for(y = rs.y1; y <= rs.y2; y += nSize)
    {
        for(x = rs.x1; x <= rs.x2; x += nSize)
        {
            c = *(p++);

            for(yy = 0; yy < nSize; yy++)
            {
                if(y + yy > rs.y2) break;

                for(xx = 0; xx < nSize; xx++)
                {
                    if(x + xx > rs.x2) break;

                    (this->*funcPix)(x + xx, y + yy, c);
                }
            }
        }

        if(pdlg) pdlg->incProgSub();
    }

    return TRUE;
}

//! 水晶

BOOL CLayerImg::filter_crystal(const CLayerImg &imgSrc,RECTANDSIZE &rs,CProgressDlg *pdlg,
                               int nSize,BOOL bAdv,BOOL bClip)
{
    AXMem memBuf;
    int xnum,ynum,x,y,xx,yy,i,j;
    int minx,miny,d,mind;
    FILTERDAT_CRYSTAL *pbuf,*p;
    void (CLayerImg::*funcPix)(int,int,BYTE) = m_pinfo->funcDrawPixel;

    if(bClip)
    {
        if(!clipRectInCanvas(&rs)) return TRUE;
    }

    //

    xnum = (rs.w + nSize - 1) / nSize;
    ynum = (rs.h + nSize - 1) / nSize;

    //メモリ確保

    if(!memBuf.allocClear(xnum * ynum * sizeof(FILTERDAT_CRYSTAL)))
        return FALSE;

    pbuf = (FILTERDAT_CRYSTAL *)memBuf.getBuf();

    //各ブロックの中央位置をランダムで決める

    p = pbuf;

    for(y = 0; y < ynum; y++)
    {
        for(x = 0; x < xnum; x++, p++)
        {
            xx = rs.x1 + x * nSize + m_pRand->getRange(0, nSize - 1);
            yy = rs.y1 + y * nSize + m_pRand->getRange(0, nSize - 1);

            if(xx > rs.x2) xx = rs.x2;
            if(yy > rs.y2) yy = rs.y2;

            p->x = xx;
            p->y = yy;
        }
    }

    //平均色の場合：ブロック内の色を加算

    if(bAdv)
    {
        for(y = rs.y1; y <= rs.y2; y++)
        {
            yy = (y - rs.y1) / nSize;

            for(x = rs.x1; x <= rs.x2; x++)
            {
                xx = (x - rs.x1) / nSize;

                //中央位置からの距離が一番短いブロック

                mind = 0x7fffffff;
                minx = miny = 0;

                for(i = yy - 1; i < yy + 2; i++)
                {
                    if(i < 0 || i >= ynum) continue;

                    for(j = xx - 1; j < xx + 2; j++)
                    {
                        if(j < 0 || j >= xnum) continue;

                        p = pbuf + i * xnum + j;

                        d = (p->x - x) * (p->x - x) + (p->y - y) * (p->y - y);

                        if(d < mind)
                        {
                            mind = d;
                            minx = j;
                            miny = i;
                        }
                    }
                }

                //

                p = pbuf + miny * xnum + minx;

                p->col += imgSrc.getPixel(x, y);
                p->cnt++;
            }
        }
    }

    //ブロック色を計算

    p = pbuf;

    for(i = 0; i < ynum; i++)
    {
        for(j = 0; j < xnum; j++, p++)
        {
            if(bAdv)
                //平均色
                p->col /= p->cnt;
            else
                //中央色
                p->col = imgSrc.getPixel(p->x, p->y);
        }
    }

    //色セット

    if(pdlg) pdlg->beginProgSub(50, rs.h, TRUE);

    for(y = rs.y1; y <= rs.y2; y++)
    {
        yy = (y - rs.y1) / nSize;

        for(x = rs.x1; x <= rs.x2; x++)
        {
            xx = (x - rs.x1) / nSize;

            mind = 0x7fffffff;
            minx = miny = 0;

            for(i = yy - 1; i < yy + 2; i++)
            {
                if(i < 0 || i >= ynum) continue;

                for(j = xx - 1; j < xx + 2; j++)
                {
                    if(j < 0 || j >= xnum) continue;

                    p = pbuf + i * xnum + j;

                    d = (p->x - x) * (p->x - x) + (p->y - y) * (p->y - y);

                    if(d < mind)
                    {
                        mind = d;
                        minx = j;
                        miny = i;
                    }
                }
            }

            p = pbuf + miny * xnum + minx;

            (this->*funcPix)(x, y, p->col);
        }

        if(pdlg) pdlg->incProgSub();
    }

    return TRUE;
}

//! ノイズ

BOOL CLayerImg::filter_noise(const CLayerImg &imgSrc,RECTANDSIZE &rs,CProgressDlg *pdlg,
            int nDensity,int nSize,BOOL bGray)
{
    int x,y,c,c2,xx,yy,rndmax;
    void (CLayerImg::*funcPix)(int,int,BYTE) = m_pinfo->funcDrawPixel;

    rndmax = nDensity * 2 - 1;

    if(pdlg) pdlg->beginProgSub(50, (rs.h + nSize - 1) / nSize);

    for(y = rs.y1; y <= rs.y2; y += nSize)
    {
        for(x = rs.x1; x <= rs.x2; x += nSize)
        {
            //ランダムで 0 以外の場合は点を描画しない

            if(m_pRand->getRange(0, rndmax))
                c2 = -1;
            else
            {
                if(bGray)
                    c2 = m_pRand->getRange(0, 255);
                else
                    c2 = 255;
            }

            //セット（色を合成）

            for(yy = 0; yy < nSize; yy++)
            {
                for(xx = 0; xx < nSize; xx++)
                {
                    c = imgSrc.getPixel(x + xx, y + yy);

                    if(c2 != -1)
                        c = (((c + c2) << 5) - ((c * c2 << 5) / 255) + (1 << 4)) >> 5;

                    (this->*funcPix)(x + xx, y + yy, c);
                }
            }
        }

        if(pdlg) pdlg->incProgSub();
    }

    return TRUE;
}

//! 1px線の補正 （ダイアログなし）

void CLayerImg::filter_thinning(const RECTANDSIZE &rs,CProgressDlg *pdlg)
{
    int x,y,i,j,pos,flag,n;
    BYTE c[25],val[25];

    pdlg->setProgMax(50);

    //--------- phase1 (余分な点を消す)

    pdlg->beginProgSub(25, rs.h);

    for(y = rs.y1; y <= rs.y2; y++)
    {
        for(x = rs.x1; x <= rs.x2; x++)
        {
            //透明なら処理なし

            n = getPixel(x, y);
            if(n == 0) continue;

            //3x3 四方の色

            for(i = -1, pos = 0; i <= 1; i++)
                for(j = -1; j <= 1; j++, pos++)
                    c[pos] = (getPixel(x + j, y + i) != 0);

            //上下が0と1か
            if((c[3] ^ c[5]) == 0) continue;
            //左右が0と1か
            if((c[1] ^ c[7]) == 0) continue;

            //

            flag = 0;
            n    = c[0] + c[2] + c[6] + c[8];

            if(n == 0)
                //斜めがすべて0
                flag = 1;
            else if(n == 1)
            {
                //斜めの1つが1で、他3つが0の場合
                //斜めの点の左右・上下どちらかに点があるか

                if(c[0])
                    flag = c[1] ^ c[3];
                else if(c[2])
                    flag = c[1] ^ c[5];
                else if(c[6])
                    flag = c[3] ^ c[7];
                else
                    flag = c[5] ^ c[7];
            }

            //消す

            if(flag) setPixelDraw(x, y, 0);
        }

        pdlg->incProgSub();
    }

    //-------- phase2 (5x5 不自然な線の補正)

    pdlg->beginProgSub(25, rs.h);

    for(y = rs.y1; y <= rs.y2; y++)
    {
        for(x = rs.x1; x <= rs.x2; x++)
        {
            //透明でなければ処理なし

            if(getPixel(x, y)) continue;

            //5x5 四方の色

            for(i = -2, pos = 0; i <= 2; i++)
            {
                for(j = -2; j <= 2; j++, pos++)
                {
                    val[pos] = getPixel(x + j, y + i);
                    c[pos]   = (val[pos] != 0);
                }
            }

            //3x3内に点が4つか

            n = 0;

            for(i = 0, pos = 6; i < 3; i++, pos += 5 - 3)
                for(j = 0; j < 3; j++, pos++)
                    n += c[pos];

            if(n != 4) continue;

            //各判定

            if(c[6] + c[7] + c[13] + c[18] == 4)
            {
                if(c[1] + c[2] + c[3] + c[9] + c[14] + c[19] == 0)
                {
                    setPixelDraw(x, y, val[13]);
                    setPixelDraw(x + 1, y, 0);
                    setPixelDraw(x, y - 1, 0);
                }
            }
            else if(c[8] + c[13] + c[16] + c[17] == 4)
            {
                if(c[9] + c[14] + c[19] + c[21] + c[22] + c[23] == 0)
                {
                    setPixelDraw(x, y, val[13]);
                    setPixelDraw(x + 1, y, 0);
                    setPixelDraw(x, y + 1, 0);
                }
            }
            else if(c[7] + c[8] + c[11] + c[16] == 4)
            {
                if(c[1] + c[2] + c[3] + c[5] + c[10] + c[15]  == 0)
                {
                    setPixelDraw(x, y, val[11]);
                    setPixelDraw(x - 1, y, 0);
                    setPixelDraw(x, y - 1, 0);
                }
            }
            else if(c[6] + c[11] + c[17] + c[18] == 4)
            {
                if(c[5] + c[10] + c[15] + c[21] + c[22] + c[23] == 0)
                {
                    setPixelDraw(x, y, val[11]);
                    setPixelDraw(x - 1, y, 0);
                    setPixelDraw(x, y + 1, 0);
                }
            }
        }

        pdlg->incProgSub();
    }
}

