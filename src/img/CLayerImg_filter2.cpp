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
    CLayerImg [filter2] - フィルタ処理
*/

#include <math.h>

#include "CLayerImg.h"

#include "CProgressDlg.h"
#include "defStruct.h"

#include "AXRand.h"
#include "AXMem.h"


//============================
//ぼかし
//============================


//! ぼかし

BOOL CLayerImg::filter_blur(const CLayerImg &imgSrc,RECTANDSIZE &rs,CProgressDlg *pdlg,
                            int nRange,BOOL bClip)
{
    CLayerImg imgTmp;
    int x,y,c,div,divhalf,i;
    RECTANDSIZE rsTmp;
    AXRect rc;
    void (CLayerImg::*funcPix)(int,int,BYTE) = m_pinfo->funcDrawPixel;

    if(bClip)
    {
        if(!clipRectInCanvas(&rs)) return TRUE;
    }

    div     = nRange * 2 + 1;
    divhalf = div / 2;

    //作業用の範囲

    rsTmp = rs;
    rsTmp.inflate(nRange);

    //

    if(pdlg) pdlg->setProgMax(50);

    //----------- imgTmp に水平方向へぼかし ※nRange 分余分に

    rsTmp.toRect(&rc);
    if(!imgTmp.allocTileArrayFromPx(rc)) return FALSE;

    if(pdlg) pdlg->beginProgSub(25, rsTmp.h);

    for(y = rsTmp.y1; y <= rsTmp.y2; y++)
    {
        for(x = rsTmp.x1; x <= rsTmp.x2; x++)
        {
            for(i = -nRange, c = 0; i <= nRange; i++)
                c += imgSrc.getPixel(x + i, y, bClip);

            c = (c + divhalf) / div;

            if(c) imgTmp.setPixel_create(x, y, c);
        }

        if(pdlg) pdlg->incProgSub();
    }

    //---------- 垂直方向にぼかし(imgTmp をソースとする)

    if(pdlg) pdlg->beginProgSub(25, rs.h);

    for(y = rs.y1; y <= rs.y2; y++)
    {
        for(x = rs.x1; x <= rs.x2; x++)
        {
            for(i = -nRange, c = 0; i <= nRange; i++)
                c += imgTmp.getPixel(x, y + i, bClip);

            c = (c + divhalf) / div;

            (this->*funcPix)(x, y, c);
        }

        if(pdlg) pdlg->incProgSub();
    }

    return TRUE;
}

//! ガウスぼかし

BOOL CLayerImg::filter_gaussBlur(const CLayerImg &imgSrc,RECTANDSIZE &rs,CProgressDlg *pdlg,
                                 int nStrong,BOOL bClip)
{
    CLayerImg imgTmp;
    AXMem memTbl;
    int x,y,cnt,range,i,c;
    RECTANDSIZE rsTmp;
    AXRect rc;
    double *pTable,d,dWeight;
    void (CLayerImg::*funcPix)(int,int,BYTE) = m_pinfo->funcDrawPixel;

    if(bClip)
    {
        if(!clipRectInCanvas(&rs)) return TRUE;
    }

    range = nStrong * 3;
    cnt   = range * 2 + 1;

    //----------- ガウステーブル

    if(!memTbl.alloc(sizeof(double) * cnt)) return FALSE;

    pTable = (double *)memTbl.getBuf();

    d       = 1.0 / (2 * nStrong * nStrong);
    dWeight = 0;

    for(i = 0, c = -range; i < cnt; i++, c++)
    {
        pTable[i] = ::exp(-(c * c) * d);

        dWeight += pTable[i];
    }

    dWeight = 1.0 / dWeight;

    //---------- 準備

    //作業用範囲

    rsTmp = rs;
    rsTmp.inflate(range);

    if(pdlg) pdlg->setProgMax(50);

    //----------- imgTmp に水平方向へぼかし ※nRange 分余分に

    rsTmp.toRect(&rc);
    if(!imgTmp.allocTileArrayFromPx(rc)) return FALSE;

    if(pdlg) pdlg->beginProgSub(25, rsTmp.h);

    for(y = rsTmp.y1; y <= rsTmp.y2; y++)
    {
        for(x = rsTmp.x1; x <= rsTmp.x2; x++)
        {
            d = 0;

            for(i = 0; i < cnt; i++)
                d += imgSrc.getPixel(x - range + i, y, bClip) * pTable[i];

            c = (int)(d * dWeight + 0.5);
            if(c < 0) c = 0; else if(c > 255) c = 255;

            if(c) imgTmp.setPixel_create(x, y, c);
        }

        if(pdlg) pdlg->incProgSub();
    }

    //---------- 垂直方向にぼかし(imgTmp をソースとする)

    if(pdlg) pdlg->beginProgSub(25, rs.h);

    for(y = rs.y1; y <= rs.y2; y++)
    {
        for(x = rs.x1; x <= rs.x2; x++)
        {
            d = 0;

            for(i = 0; i < cnt; i++)
                d += imgTmp.getPixel(x, y - range + i, bClip) * pTable[i];

            c = (int)(d * dWeight + 0.5);
            if(c < 0) c = 0; else if(c > 255) c = 255;

            (this->*funcPix)(x, y, c);
        }

        if(pdlg) pdlg->incProgSub();
    }

    return TRUE;
}

//! モーションブラー
/*!
    0.5px 間隔で指定角度+距離分の範囲をガウスぼかし
*/

BOOL CLayerImg::filter_motionBlur(const CLayerImg &imgSrc,RECTANDSIZE &rs,CProgressDlg *pdlg,
                                  int nStrong,int nAngle,BOOL bClip)
{
    AXMem memTbl;
    int x,y,c,i,cnt,range;
    int stx,sty,fx,fy,addx,addy;
    double sind,cosd,dWeight,d,dTmp;
    double *pTable;
    void (CLayerImg::*funcPix)(int,int,BYTE) = m_pinfo->funcDrawPixel;

    if(bClip)
    {
        if(!clipRectInCanvas(&rs)) return TRUE;
    }

    range = nStrong * 3;
    cnt   = (range * 2 + 1) * 2;    //0.5px間隔のため2倍

    //----------- ガウステーブル

    if(!memTbl.alloc(sizeof(double) * cnt)) return FALSE;

    pTable = (double *)memTbl.getBuf();

    d       = -range;
    dTmp    = 1.0 / (2 * nStrong * nStrong);
    dWeight = 0;

    for(i = 0; i < cnt; i++, d += 0.5)
    {
        pTable[i] = ::exp(-(d * d) * dTmp);

        dWeight += pTable[i];
    }

    dWeight = 1.0 / dWeight;

    //----------- 角度

    d    = -nAngle * M_PI / 180;
    sind = ::sin(d);
    cosd = ::cos(d);

    stx  = (int)(cosd * -range * (1 << 16));
    sty  = (int)(sind * -range * (1 << 16));
    addx = (int)(cosd * 0.5 * (1 << 16));
    addy = (int)(sind * 0.5 * (1 << 16));

    //-------------

    if(pdlg) pdlg->beginProgSub(50, rs.h, TRUE);

    for(y = rs.y1; y <= rs.y2; y++)
    {
        for(x = rs.x1; x <= rs.x2; x++)
        {
            fx = stx, fy = sty;
            d = 0;

            for(i = 0; i < cnt; i++)
            {
                d += imgSrc.getPixel(x + (fx >> 16), y + (fy >> 16), bClip) * pTable[i];

                fx += addx;
                fy += addy;
            }

            c = (int)(d * dWeight + 0.5);
            if(c < 0) c = 0; else if(c > 255) c = 255;

            (this->*funcPix)(x, y, c);
        }

        if(pdlg) pdlg->incProgSub();
    }

    return TRUE;
}


//============================
//描画
//============================


//! アミトーン描画
/*!
    @param nLineCnt 線数（1.0=10）
    @param nPers    濃度(%)
    @param nDPI     -1 で nLineCnt が px の長さ(1.0=10)
*/

BOOL CLayerImg::filter_drawAmiTone(RECTANDSIZE &rs,CProgressDlg *pdlg,
                        int nLineCnt,int nPers,int nAngle,int nDPI,BOOL bAnti)
{
    int x,y,sx,sy,c;
    double dsin,dcos,xx,yy,len,len_div,len_half,rr,cx,cy,sdx,sdy,dyy;
    void (CLayerImg::*funcPix)(int,int,BYTE) = m_pinfo->funcDrawPixel;

    //点の範囲の1辺の長さ

    if(nDPI == -1)
        len = nLineCnt * 0.1;
    else
        len	= (double)nDPI / (nLineCnt * 0.1);

    //

    rr   = -nAngle * M_PI / 180.0;
    dcos = ::cos(rr);
    dsin = ::sin(rr);

    c  = (nPers > 50)? 100 - nPers: nPers;              //50%以上は白黒反転
    rr = ::sqrt(len * len * (c * 0.01) * (1.0 / M_PI)); //点の半径
    rr *= rr;

    len_div  = 1.0 / len;
    len_half = len * 0.5;

    //----------------

    for(y = rs.y1; y <= rs.y2; y++)
    {
        sdx = rs.x1;
        sdy = y;

        xx = sdx * dcos - sdy * dsin;
        yy = sdx * dsin + sdy * dcos;

        //

        for(x = rs.x1; x <= rs.x2; x++)
        {
            //枠の中央位置

            cx = ::floor(xx * len_div) * len + len_half;
            cy = ::floor(yy * len_div) * len + len_half;

            //

            if(bAnti)
            {
                //アンチエイリアス (5x5)

                c = 0;

                for(sy = 0, sdy = yy - cy; sy < 5; sy++, sdy += 0.2)
                {
                    dyy = sdy * sdy;

                    for(sx = 0, sdx = xx - cx; sx < 5; sx++, sdx += 0.2)
                    {
                        if(sdx * sdx + dyy < rr)
                            c += 255;
                    }
                }

                c /= 25;
            }
            else
            {
                //非アンチ

                sdx = xx - cx;
                sdy = yy - cy;

                if((int)(sdx * sdx + sdy * sdy + 0.5) < (int)(rr + 0.5))
                    c = 255;
                else
                    c = 0;
            }

            //50%以上は白黒反転

            if(nPers > 50) c = 255 - c;

            (this->*funcPix)(x, y, c);

            //

            xx += dcos;
            yy += dsin;
        }

        if(pdlg) pdlg->incProgSub();
    }

    return TRUE;
}

//! ドットライン描画
/*!
	@param uFlags [0bit]横線 [1bit]縦線
*/

BOOL CLayerImg::filter_drawDotLine(RECTANDSIZE &rs,CProgressDlg *pdlg,
                                   int minWidth,int maxWidth,int minSpace,int maxSpace,UINT uFlags)
{
    int pos,x,y,w,rangeW,rangeS;
    void (CLayerImg::*funcPix)(int,int,BYTE) = m_pinfo->funcDrawPixel;

    if(maxWidth < minWidth) maxWidth = minWidth;
    if(maxSpace < minSpace) maxSpace = minSpace;

    rangeW = maxWidth - minWidth;
    rangeS = maxSpace - minSpace;

    if(pdlg) pdlg->setProgMax(2);

    //横線

    if(uFlags & 1)
    {
        //初期Y位置
        pos = rs.y1 + m_pRand->getRange(0, rangeS);
        if(rangeS == 0) pos += minSpace;

        while(pos <= rs.y2)
        {
            //太さ

            w = m_pRand->getRange(0, rangeW) + minWidth;
            if(pos + w > rs.y2) w = rs.y2 - pos + 1;

            //描画

            for(y = pos; y < pos + w; y++)
            {
                for(x = rs.x1; x <= rs.x2; x++)
                    (this->*funcPix)(x, y, 255);
            }

            //次の位置

            pos += m_pRand->getRange(0, rangeS) + minSpace + w;
        }
    }

    if(pdlg) pdlg->incProg();

    //縦線

    if(uFlags & 2)
    {
        pos = rs.x1 + m_pRand->getRange(0, rangeS);
        if(rangeS == 0) pos += minSpace;

        while(pos <= rs.x2)
        {
            w = m_pRand->getRange(0, rangeW) + minWidth;
            if(pos + w > rs.x2) w = rs.x2 - pos + 1;

            for(y = rs.y1; y <= rs.y2; y++)
            {
                for(x = pos; x < pos + w; x++)
                    (this->*funcPix)(x, y, 255);
            }

            pos += m_pRand->getRange(0, rangeS) + minSpace + w;
        }
    }

    if(pdlg) pdlg->incProg();

    return TRUE;
}

//! チェック柄描画

BOOL CLayerImg::filter_drawCheck(RECTANDSIZE &rs,CProgressDlg *pdlg,
                                 int colW,int rowW,BOOL bSame)
{
    int x,y,colMod,rowMod,yf,xf,n;
    void (CLayerImg::*funcPix)(int,int,BYTE) = m_pinfo->funcDrawPixel;

    if(bSame) rowW = colW;

    colMod = colW * 2;
    rowMod = rowW * 2;

    for(y = rs.y1; y <= rs.y2; y++)
    {
        n = y;
        while(n < 0) n += m_pinfo->nImgH;

        yf = (n % rowMod) / rowW;

        for(x = rs.x1; x <= rs.x2; x++)
        {
            n = x;
            while(n < 0) n += m_pinfo->nImgW;

            xf = (n % colMod) / colW;

            if((xf ^ yf) == 0)
                (this->*funcPix)(x, y, 255);
        }

        if(pdlg) pdlg->incProgSub();
    }

    return TRUE;
}


//============================
//その他
//============================


//! 縁取り

BOOL CLayerImg::filter_edge(const CLayerImg &imgSrc,RECTANDSIZE &rs,CProgressDlg *pdlg,
                            const CLayerImg &imgRef,int nSize,BOOL bCut)
{
    CLayerImg imgTmp1,imgTmp2;
    const CLayerImg *pimg;
    int i,x,y,c;
    void (CLayerImg::*funcPix)(int,int,BYTE);

    if(pdlg) pdlg->setProgMax(nSize * 20 + 20);

    //判定用レイヤをコピー

    if(!imgTmp2.copy(imgRef)) return FALSE;

    //-------- 縁取り描画 (imgTmp1 を参照として、dst と imgTmp2 へ描画)

    if(pdlg)
    {
        //実際の描画時

        funcPix = &CLayerImg::setPixelDraw;
        m_pinfo->nColSetFunc = COLSETF_BLEND;
    }
    else
    {
        //プレビュー時（元画像をコピー）

        funcPix = &CLayerImg::setPixel_blendEx;

        for(y = rs.y1; y <= rs.y2; y++)
        {
            for(x = rs.x1; x <= rs.x2; x++)
            {
                c = imgSrc.getPixel(x, y);
                if(c) setPixel_create(x, y, c);
            }
        }
    }

    //

    for(i = 0; i < nSize; i++)
    {
        //imgTmp2 を imgTmp1 にコピー

        if(!imgTmp1.copy(imgTmp2)) return FALSE;

        //imgTmp1 を判定元として、その点を上下左右に合成

        if(pdlg) pdlg->beginProgSub(20, rs.h);

        for(y = rs.y1; y <= rs.y2; y++)
        {
            for(x = rs.x1; x <= rs.x2; x++)
            {
                c = imgTmp1.getPixel(x, y);
                if(!c) continue;

                //dst 描画

                (this->*funcPix)(x - 1, y, c);
                (this->*funcPix)(x + 1, y, c);
                (this->*funcPix)(x, y - 1, c);
                (this->*funcPix)(x, y + 1, c);

                //次の判定用に imgTmp2 に描画
                //（imgTmp1 に描画すると判定用として使えないため）

                imgTmp2.setPixel_blendEx(x - 1, y, c);
                imgTmp2.setPixel_blendEx(x + 1, y, c);
                imgTmp2.setPixel_blendEx(x, y - 1, c);
                imgTmp2.setPixel_blendEx(x, y + 1, c);
            }

            if(pdlg) pdlg->incProgSub();
        }
    }

    imgTmp1.free();
    imgTmp2.free();

    //-------- 判定元画像を切り取り

    if(bCut)
    {
        //判定用レイヤが自身の場合はイメージが変更されているのでソース画像を使う

        pimg = (&imgRef == this)? &imgSrc: &imgRef;

        if(pdlg)
        {
            funcPix = m_pinfo->funcDrawPixel;
            m_pinfo->nColSetFunc = COLSETF_ERASE;
        }
        else
            funcPix = &CLayerImg::setPixel_erase;

        //

        if(pdlg) pdlg->beginProgSub(20, rs.h);

        for(y = rs.y1; y <= rs.y2; y++)
        {
            for(x = rs.x1; x <= rs.x2; x++)
            {
                c = pimg->getPixel(x, y);

                if(c)
                    (this->*funcPix)(x, y, c);
            }

            if(pdlg) pdlg->incProgSub();
        }
    }

    return TRUE;
}

//! ハーフトーン

BOOL CLayerImg::filter_halftone(const CLayerImg &imgSrc,RECTANDSIZE &rs,CProgressDlg *pdlg,
                                int nSize,int nAngle,BOOL bAnti)
{
    int x,y,c,ix,iy;
    double dsin,dcos,xx,yy,len,len_div,len_half,rr,dx,dy,dyy;
    double rrtmp,dx2,dy2;
    void (CLayerImg::*funcPix)(int,int,BYTE) = m_pinfo->funcDrawPixel;

    len = nSize * 0.1;

    rr   = -nAngle * M_PI / 180.0;
    dcos = ::cos(rr);
    dsin = ::sin(rr);

    len_div  = 1.0 / len;
    len_half = len * 0.5;
    rrtmp    = len * len / M_PI / 255.0;

    //----------------

    for(y = rs.y1; y <= rs.y2; y++)
    {
        dx = rs.x1;
        dy = y;

        xx = dx * dcos - dy * dsin;
        yy = dx * dsin + dy * dcos;

        //

        for(x = rs.x1; x <= rs.x2; x++)
        {
            //枠の中央位置からの距離

            dx = xx - (::floor(xx * len_div) * len + len_half);
            dy = yy - (::floor(yy * len_div) * len + len_half);

            //色から半径取得

            c = imgSrc.getPixel(x, y);

            rr = ::sqrt(c * rrtmp);
            rr *= rr;

            //色取得

            if(c == 255)
                c = 255;
            else if(bAnti)
            {
                //※サブピクセルごとに計算しないとはみ出している部分が問題になる

                c = 0;

                for(iy = 0, dy2 = dy; iy < 5; iy++, dy2 += 0.2)
                {
                    if(dy2 >= len_half) dy2 -= len;
                    dyy = dy2 * dy2;

                    for(ix = 0, dx2 = dx; ix < 5; ix++, dx2 += 0.2)
                    {
                        if(dx2 >= len_half) dx2 -= len;

                        if(dx2 * dx2 + dyy < rr) c += 255;
                    }
                }

                c /= 25;
            }
            else
            {
                if((int)(dx * dx + dy * dy + 0.5) < (int)(rr + 0.5))
                    c = 255;
                else
                    c = 0;
            }

            //描画

            (this->*funcPix)(x, y, c);

            //

            xx += dcos;
            yy += dsin;
        }

        if(pdlg) pdlg->incProgSub();
    }

    return TRUE;
}


//! フラッシュ描画
/*
    type : [0]集中線/フラッシュ (円の外周から中心へ向けて)
           [1]ベタフラッシュ (円の外周から外へ向けて&中を塗りつぶし)
           [2]ウニフラッシュ
    pVal : [0]半径(px)
           [1]縦横比(-1.000〜+1.000)
           [2]密度最小(角度の分割数/2)
           [3]間隔ランダム幅(0-99, 角度ステップに対する%)
           [4]線の太さ(px, 0.5-)
           [5]線の太さランダム(px, 0.0-)
           [6]線の長さ(0.1-100.0, 半径に対する%)
           [7]線の長さランダム幅(0.0-100.0, 半径に対する%)
    nDrawType : [0]ブラシ・アンチエイリアスあり
                [1]ブラシ・アンチエイリアスなし
                [2]1pxドット線
*/

BOOL CLayerImg::filter_drawFlash(CProgressDlg *pdlg,
                                 int type,AXPoint &ptPos,const int *pVal,int nDrawType,BOOL bSample)
{
    double angle,angleStep,angleRnd,r,len,lenRnd,xscale,yscale,size,sizeRnd;
    double xpos,ypos,x1,x2,y1,y2,xx,yy,sind,cosd,tmp,line_r1;
    WORD wHeadTail;

    bSample = (!pdlg && bSample);

    xpos    = ptPos.x;
    ypos    = ptPos.y;
    r       = pVal[0];
    size    = pVal[4] * 0.1;
    sizeRnd = pVal[5] * 0.1;
    len     = r * (pVal[6] * 0.001);
    lenRnd  = r * (pVal[7] * 0.001);
    angleStep = M_PI * 2 / (pVal[2] * 2);
    angleRnd  = angleStep * (pVal[3] * 0.01);

    if(pVal[1] < 0)
        xscale = 1, yscale = 1.0 + pVal[1] * 0.001;
    else
        xscale = 1.0 - pVal[1] * 0.001, yscale = 1;

    wHeadTail = (type == 2)? (50 << 8)|50: 100;

    //

    m_pinfo->bparam.dValMin = (nDrawType == 0)? 0: 1.0;
    m_pinfo->bparam.bAnti   = (nDrawType == 0);
    m_pinfo->nColSetFunc    = COLSETF_BLEND;

    //プレビュー時

    if(!pdlg)
    {
        if(bSample)
            m_pinfo->funcDrawPixel = &CLayerImg::setPixel_calcRect;
        else
            m_pinfo->funcDrawPixel = &CLayerImg::setPixel_blendCalcRect;
    }

    //-------- 描画

    if(pdlg) pdlg->beginProgSub(50, pVal[2] * 2, TRUE);

    for(angle = 0; angle < M_PI * 2; angle += angleStep)
    {
        //角度

        tmp = angle;
        if(pVal[3]) tmp += m_pRand->getDouble() * angleRnd;

        cosd = ::cos(tmp) * xscale;
        sind = ::sin(tmp) * yscale;

        //位置

        switch(type)
        {
            //集中線・フラッシュ
            case 0:
                line_r1 = r;

                tmp = r - len;
                if(pVal[7])
                    tmp -= m_pRand->getDouble() * lenRnd;
                break;
            //ベタフラッシュ
            case 1:
                line_r1 = r;

                tmp = r + len;
                if(pVal[7])
                    tmp += m_pRand->getDouble() * lenRnd;
                break;
            //ウニフラッシュ
            case 2:
                tmp = len;
                if(pVal[7])
                    tmp += m_pRand->getDouble() * lenRnd;

                tmp *= 0.5;

                line_r1 = r + tmp;
                tmp     = r - tmp;
                break;
        }

        xx = cosd * xscale;
        yy = sind * yscale;

        x1 = line_r1 * xx + xpos;
        y1 = line_r1 * yy + ypos;
        x2 = tmp * xx + xpos;
        y2 = tmp * yy + ypos;

        //線のサイズ

        m_pinfo->bparam.dMaxSize = size;

        if(pVal[5])
            m_pinfo->bparam.dMaxSize += m_pRand->getDouble() * sizeRnd;

        //描画

        if(nDrawType == 2)
            drawLineB(x1, y1, x2, y2, 255, FALSE);
        else if(!bSample)
            drawBrush_lineHeadTail(x1, y1, x2, y2, wHeadTail);
        else
        {
            //簡易プレビュー

            if(m_pinfo->bparam.dMaxSize <= 1)
                drawLineB(x1, y1, x2, y2, 255, FALSE);
            else if(type == 2)
            {
                //ウニフラッシュ

                xx = x1 + (x2 - x1) * 0.5;
                yy = y1 + (y2 - y1) * 0.5;

                tmp = m_pinfo->bparam.dMaxSize * 0.5;

                sind = tmp * -sind * xscale;
                cosd = tmp *  cosd * yscale;

                drawLineB(x1, y1, xx + sind, yy + cosd, 255, FALSE);
                drawLineB(xx + sind, yy + cosd, x2, y2, 255, FALSE);
                drawLineB(x2, y2, xx - sind, yy - cosd, 255, FALSE);
                drawLineB(xx - sind, yy - cosd, x1, y1, 255, FALSE);
            }
            else
            {
                //フラッシュ

                tmp = m_pinfo->bparam.dMaxSize * 0.5;

                sind = tmp * -sind * xscale;
                cosd = tmp *  cosd * yscale;

                drawLineB(x2, y2, x1 + sind, y1 + cosd, 255, FALSE);
                drawLineB(x1 + sind, y1 + cosd, x1 - sind, y1 - cosd, 255, FALSE);
                drawLineB(x1 - sind, y1 - cosd, x2, y2, 255, FALSE);
            }
        }

        //

        if(pdlg) pdlg->incProgSub();
    }

    //ベタフラッシュ、内側塗りつぶし

    if(type == 1)
    {
        double dParam[6];

        dParam[2] = -1, dParam[3] = 0;
        dParam[4] = 1, dParam[5] = 0;

        if(bSample)
            drawCircle(xpos, ypos, r * xscale, r * yscale, dParam, FALSE, FALSE);
        else
            drawFillCircle(xpos, ypos, r * xscale, r * yscale, dParam, FALSE, 255, FALSE);
    }

    return TRUE;
}

//! ウニフラッシュ(波)
/*
    pVal : [0]半径(px)
           [1]縦横比(-1.000〜+1.000)
           [2]波の密度(角度の分割数)
           [3]波の長さ(半径に対する%)
           [4]線の太さ(px, 0.5-)
           [5]線の長さ(0.1-100.0, 半径に対する%)
           [6]抜きの最小(%)
    nDrawType : [0]ブラシ・アンチエイリアスあり
                [1]ブラシ・アンチエイリアスなし
                [2]1pxドット線
*/

BOOL CLayerImg::filter_drawUniFlashWave(CProgressDlg *pdlg,
                                 AXPoint &ptPos,const int *pVal,int nDrawType,BOOL bSample)
{
    double angle,angleStep,angleStep2,r,len,xscale,yscale;
    double xpos,ypos,x1,x2,y1,y2,sind,cosd,tmp,tblR[6];
    int i;

    bSample = (!pdlg && bSample);

    xpos    = ptPos.x;
    ypos    = ptPos.y;
    r       = pVal[0];
    len     = r * (pVal[5] * 0.001);
    angleStep  = M_PI * 2 / pVal[2];
    angleStep2 = angleStep / 6;

    if(pVal[1] < 0)
        xscale = 1, yscale = 1.0 + pVal[1] * 0.001;
    else
        xscale = 1.0 - pVal[1] * 0.001, yscale = 1;

    //

    tmp = r * (pVal[3] * 0.001);

    tblR[0] = 0;
    tblR[1] = tmp / 3;
    tblR[2] = tmp * 2 / 3;
    tblR[3] = tmp;
    tblR[4] = tblR[2];
    tblR[5] = tblR[1];

    //

    m_pinfo->bparam.dMaxSize = pVal[4] * 0.1;
    m_pinfo->bparam.dSizeMin = pVal[6] * 0.01;
    m_pinfo->bparam.dValMin  = 1.0;
    m_pinfo->bparam.bAnti    = (nDrawType == 0);
    m_pinfo->nColSetFunc     = COLSETF_BLEND;

    //プレビュー時

    if(!pdlg)
    {
        if(bSample)
            m_pinfo->funcDrawPixel = &CLayerImg::setPixel_calcRect;
        else
            m_pinfo->funcDrawPixel = &CLayerImg::setPixel_blendCalcRect;
    }

    //-------- 描画

    if(pdlg) pdlg->beginProgSub(50, pVal[2], TRUE);

    for(angle = 0; angle < M_PI * 2; angle += angleStep)
    {
        //1つの波に6個描画

        for(i = 0; i < 6; i++)
        {
            tmp = angle + i * angleStep2;

            cosd = ::cos(tmp) * xscale;
            sind = ::sin(tmp) * yscale;

            //位置

            tmp = r + tblR[i];

            x1 = tmp * cosd + xpos;
            y1 = tmp * sind + ypos;

            tmp -= len;

            x2 = tmp * cosd + xpos;
            y2 = tmp * sind + ypos;

            //描画

            if(nDrawType == 2)
                drawLineB(x1, y1, x2, y2, 255, FALSE);
            else if(!bSample)
                drawBrush_lineHeadTail(x1, y1, x2, y2, 100);
            else
            {
                //簡易プレビュー

                tmp = m_pinfo->bparam.dMaxSize * 0.5;

                sind = tmp * -sind * xscale;
                cosd = tmp *  cosd * yscale;

                drawLineB(x2, y2, x1 + sind, y1 + cosd, 255, FALSE);
                drawLineB(x1 + sind, y1 + cosd, x1 - sind, y1 - cosd, 255, FALSE);
                drawLineB(x1 - sind, y1 - cosd, x2, y2, 255, FALSE);
            }
        }

        if(pdlg) pdlg->incProgSub();
    }

    return TRUE;
}
