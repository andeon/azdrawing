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
    CLayerImg [drawBrush] - ブラシ描画関連
*/

#include <math.h>

#include "CLayerImg.h"
#include "CImage8.h"
#include "CSinTable.h"
#include "defStruct.h"

#include "AXRand.h"


//-----------------------

struct DRAWPOINT
{
    double x,y,press;
};

//-----------------------

DRAWPOINT g_posCurve[4];          //曲線補間用
double    g_dCurveParam[20][4];   //曲線補間用、パラメータ

//-----------------------

//! （関数）初期化

void CLayerImg::init()
{
    int i;
    double t,tt,t1_,t2_,t3_;

    //曲線補間用パラメータセット

    tt = 1.0 / 6.0;

    for(i = 0, t = 0.05; i < 20; i++, t += 0.05)
    {
        t1_ = 4.0 - (t + 3.0);
        t2_ = t + 2.0;
        t3_ = t + 1.0;

        g_dCurveParam[i][0] = tt * t1_ * t1_ * t1_;
        g_dCurveParam[i][1] = tt * ( 3.0 * t2_ * t2_ * t2_ - 24.0 * t2_ * t2_ + 60.0 * t2_ - 44.0);
        g_dCurveParam[i][2] = tt * (-3.0 * t3_ * t3_ * t3_ + 12.0 * t3_ * t3_ - 12.0 * t3_ + 4.0);
        g_dCurveParam[i][3] = tt * t * t * t;
    }
}

//! （関数）曲線、最初の3点セット

void CLayerImg::setCurvePos(double x,double y,double press)
{
    int i;

    for(i = 0; i < 3; i++)
    {
        g_posCurve[i].x = x;
        g_posCurve[i].y = y;
        g_posCurve[i].press = press;
    }
}

//-----------------------


//! 自由線の描画

void CLayerImg::drawBrush_free(double x,double y,double press)
{
    BRUSHDRAWPARAM *pParam = &m_pinfo->bparam;
    int i;

    pParam->dPressSt = pParam->dBkPress;
    pParam->dPressEd = press;

    //描画

    if(pParam->bCurve)
    {
        //曲線

        g_posCurve[3].x = x;
        g_posCurve[3].y = y;
        g_posCurve[3].press = press;

        _drawBrush_lineCurve();

        for(i = 0; i < 3; i++)
            g_posCurve[i] = g_posCurve[i + 1];
    }
    else
    {
        //直線

        drawBrush_line(pParam->dBkX, pParam->dBkY, x, y);

        pParam->dBkX     = x;
        pParam->dBkY     = y;
        pParam->dBkPress = press;
    }
}

//! 曲線補間の線描画

void CLayerImg::_drawBrush_lineCurve()
{
    int i;
    double bx,by,nx,ny;
    double t1,t2,t3,t4;
    DRAWPOINT *pPos = g_posCurve;
    BRUSHDRAWPARAM *p = &m_pinfo->bparam;

    bx = p->dBkX;
    by = p->dBkY;

    //

    for(i = 0; i < 20; i++)
    {
        t1 = g_dCurveParam[i][0];
        t2 = g_dCurveParam[i][1];
        t3 = g_dCurveParam[i][2];
        t4 = g_dCurveParam[i][3];

        //

        nx = pPos[0].x * t1 + pPos[1].x * t2 + pPos[2].x * t3 + pPos[3].x * t4;
        ny = pPos[0].y * t1 + pPos[1].y * t2 + pPos[2].y * t3 + pPos[3].y * t4;

        p->dPressEd = pPos[0].press * t1 + pPos[1].press * t2 + pPos[2].press * t3 + pPos[3].press * t4;

        drawBrush_line(bx, by, nx, ny);

        //

        bx = nx;
        by = ny;
        p->dPressSt = p->dPressEd;
    }

    //

    p->dBkX = nx;
    p->dBkY = ny;
    p->dBkPress = p->dPressEd;
}


//=================================
//線を引く
//（通常用と入り抜きあり用があるので、
//データの変更などがあった場合は両方対応すること）
//=================================


//! ブラシで線を引く（線形補間）
/*!
    dPressSt, dPressEd に各筆圧、dT に次の t 値が入る
*/

void CLayerImg::drawBrush_line(double x1,double y1,double x2,double y2)
{
    double len,t,tt,dx,dy,ttm;
    double press_len,size_len,val_len;
    double press,press_s,press_v,size,x,y;
    int nRotBasic;
    BRUSHDRAWPARAM *p = &m_pinfo->bparam;

    //線の長さ

    dx = x2 - x1;
    dy = y2 - y1;

    len = ::sqrt(dx * dx + dy * dy);
    if(len == 0) return;

    //間隔をt値に合わせる

    ttm = p->dInterval / len;

    //各幅

    size_len  = 1.0 - p->dSizeMin;
    val_len   = 1.0 - p->dValMin;
    press_len = p->dPressEd - p->dPressSt;

    //画像回転角度・基本（回転角度＋進行方向）
    //※ランダムの場合点ごとに行うため

    nRotBasic = p->nRotAngle;

    if(p->btRotFlag & 1)
        nRotBasic += (int)(::atan2(dy, dx) * 180 / M_PI);

    //------------------

    t = p->dT / len;

    while(t < 1.0)
    {
        //筆圧

        press   = press_len * t + p->dPressSt;  //開始点〜終了点補間

        press_s = (p->btPressFlag & 1)? ::pow(press, p->dGammaSize): press; //ガンマ補正
        press_v = (p->btPressFlag & 2)? ::pow(press, p->dGammaVal): press;

        press_s = press_s * size_len + p->dSizeMin;    //最小補正
        press_v = press_v * val_len + p->dValMin;

        //サイズ

        if(p->btRandFlag & 1)
            press_s = press_s * (m_pRand->getDouble() * (1.0 - p->dRandSizeMin) + p->dRandSizeMin);

        size = press_s * p->dMaxSize;

        //位置

        x = x1 + dx * t;
        y = y1 + dy * t;

        if(p->btRandFlag & 2)
        {
            if(p->btRandFlag & 4)
            {
                //矩形範囲

                tt = size * p->dRandPosLen;
                x += tt * (m_pRand->getDouble() - 0.5) * 2;
                y += tt * (m_pRand->getDouble() - 0.5) * 2;
            }
            else
            {
                //円形範囲

                tt = (size * p->dRandPosLen) * m_pRand->getDouble();
                m_pSinTbl->addPosCircle(&x, &y, tt, m_pRand->getRange(0, 359));
            }
        }

        //画像回転角度

        p->nRotAngleRes = nRotBasic;

        if(p->btRotFlag & 2)
            p->nRotAngleRes += m_pRand->getRange(-(p->nRotRandom), p->nRotRandom);

        //点描画

        if(p->nRotAngleRes == 0)
            drawBrush_point_normal(x, y, size, (int)(p->dMaxVal * press_v + 0.5));
        else
            drawBrush_point_rotate(x, y, size, (int)(p->dMaxVal * press_v + 0.5));

        //次へ

        tt = size * ttm;
        if(tt < 0.0001) tt = 0.0001;

        t += tt;
    }

    //次回の開始t値（長さ(px)に変換）

    p->dT = len * (t - 1.0);
}

//! ブラシ直線描画（入り抜き指定あり）

void CLayerImg::drawBrush_lineHeadTail(double x1,double y1,double x2,double y2,WORD wHeadTail)
{
    double len,t,tt,dx,dy,ttm,dIn,dOut;
    double size_len,val_len;
    double press,press_s,press_v,size,x,y;
    int nIn,nOut,nT,nRotBasic;
    BRUSHDRAWPARAM *p = &m_pinfo->bparam;

    //線の長さ

    dx = x2 - x1;
    dy = y2 - y1;

    len = ::sqrt(dx * dx + dy * dy);
    if(len == 0) return;

    //間隔をt値に合わせる

    ttm = p->dInterval / len;

    //各幅

    size_len = 1.0 - p->dSizeMin;
    val_len  = 1.0 - p->dValMin;

    //入り抜き位置

    nIn  = wHeadTail >> 8;
    nOut = wHeadTail & 0xff;

    dIn  = (nIn )? 1.0 / ((double)nIn  * 0.01): 0;
    dOut = (nOut)? 1.0 / ((double)nOut * 0.01): 0;

    //画像回転角度・基本（回転角度＋進行方向）

    nRotBasic = p->nRotAngle;

    if(p->btRotFlag & 1)
        nRotBasic += (int)(::atan2(dy, dx) * 180 / M_PI);

    //------------------

    t = 0;

    while(t <= 1.0)
    {
        //筆圧

        nT = (int)(t * 100);

        if(nT < nIn) press = t * dIn;
        else if(nT > 100 - nOut) press = (1.0 - t) * dOut;
        else press = 1.0;

        press_s = press * size_len + p->dSizeMin;  //最小補正
        press_v = press * val_len + p->dValMin;

        //サイズ

        if(p->btRandFlag & 1)
            press_s = press_s * (m_pRand->getDouble() * (1.0 - p->dRandSizeMin) + p->dRandSizeMin);

        size = press_s * p->dMaxSize;

        //位置

        x = x1 + dx * t;
        y = y1 + dy * t;

        if(p->btRandFlag & 2)
        {
            if(p->btRandFlag & 4)
            {
                //矩形範囲

                tt = size * p->dRandPosLen;
                x += tt * (m_pRand->getDouble() - 0.5) * 2;
                y += tt * (m_pRand->getDouble() - 0.5) * 2;
            }
            else
            {
                //円形範囲

                tt = (size * p->dRandPosLen) * m_pRand->getDouble();
                m_pSinTbl->addPosCircle(&x, &y, tt, m_pRand->getRange(0, 359));
            }
        }

        //画像回転角度

        p->nRotAngleRes = nRotBasic;

        if(p->btRotFlag & 2)
            p->nRotAngleRes += m_pRand->getRange(-(p->nRotRandom), p->nRotRandom);

        //点描画

        if(p->nRotAngleRes == 0)
            drawBrush_point_normal(x, y, size, (int)(p->dMaxVal * press_v + 0.5));
        else
            drawBrush_point_rotate(x, y, size, (int)(p->dMaxVal * press_v + 0.5));

        //次へ

        tt = size * ttm;
        if(tt < 0.00001) tt = 0.00001;

        t += tt;
    }
}


//=================================
//点の描画
//=================================


//! ブラシ・点の描画（回転なし）

void CLayerImg::drawBrush_point_normal(double xpos,double ypos,double size,int val)
{
    int isize,ix,iy,subnum,subnum2,brimgsize;
    int hf,x1,y1,x2,y2,sx,sy;
    int i,j,xx,yy,fx,fy,addf,addf2,c,fst;
    double dscale;
    LPBYTE tblY[11],pBrushBuf,pBrushY;
    int tblX[11];
    BYTE bDrawTP;
    BRUSHDRAWPARAM *p = &m_pinfo->bparam;
    void (CLayerImg::*funcPix)(int,int,BYTE) = m_pinfo->funcDrawPixel;

    if(val == 0) return;

    //上書き2、透明部分も描画するか

    bDrawTP = (m_pinfo->nColSetFunc == COLSETF_OVERWRITE2);

    //点の大きさによりサブピクセル数調整

    isize = (int)size;

    if(isize < 8) subnum = 11;
    else if(isize < 40) subnum = 9;
    else if(isize < 150) subnum = 5;
    else subnum = 3;

    isize = (int)(size * subnum + 0.5);
    if(isize < 2) return;

    subnum2 = subnum * subnum;

    //

    ix  = (int)(xpos * subnum + 0.5);   //[sub]中心位置
    iy  = (int)(ypos * subnum + 0.5);

    hf  = isize >> 1;                   //[sub]半径

    sx  = ix - hf;                      //[sub]左上位置
    sy  = iy - hf;
    x1  = sx / subnum;                  //[px]描画左上位置
    y1  = sy / subnum;
    x2  = (sx + isize - 1) / subnum;    //[px]描画右下位置
    y2  = (sy + isize - 1) / subnum;

    pBrushBuf = (m_pinfo->pBrush)->getBuf();
    brimgsize = (m_pinfo->pBrush)->getWidth();

    dscale = (double)brimgsize / isize * (1 << 12);

    addf  = (int)(dscale * subnum);
    addf2 = (int)dscale;

    //ブラシの中心から回転角度 0 とした初期値

    fst = (int)((x1 * subnum - ix) * dscale) + (brimgsize << 11);
    fy  = (int)((y1 * subnum - iy) * dscale) + (brimgsize << 11);

    //--------------

    for(yy = y1; yy <= y2; yy++, fy += addf)
    {
        //Yテーブル

        for(i = 0, j = fy; i < subnum; i++, j += addf2)
        {
            c = j >> 12;

            if(c >= 0 && c < brimgsize)
                tblY[i] = pBrushBuf + c * brimgsize;
            else
                tblY[i] = NULL;
        }

        //

        for(xx = x1, fx = fst; xx <= x2; xx++, fx += addf)
        {
            //Xテーブル

            for(i = 0, j = fx; i < subnum; i++, j += addf2)
            {
                c = j >> 12;

                if(c >= 0 && c < brimgsize)
                    tblX[i] = c;
                else
                    tblX[i] = -1;
            }

            //サブピクセル平均

            c = 0;

            for(i = 0; i < subnum; i++)
            {
                pBrushY = tblY[i];
                if(!pBrushY) continue;

                for(j = 0; j < subnum; j++)
                {
                    if(tblX[j] != -1)
                        c += *(pBrushY + tblX[j]);
                }
            }

            c /= subnum2;

            //点セット

            if(c || bDrawTP)
            {
                if(c)
                {
                    if(p->bAnti)
                        c = (val * c + 127) / 255;
                    else
                        c = val;
                }

                (this->*funcPix)(xx, yy, c);
            }
        }
    }
}

//! ブラシ・点の描画（回転あり）

void CLayerImg::drawBrush_point_rotate(double xpos,double ypos,double size,int val)
{
    int isize,isize_draw,ix,iy,subnum,subnum2,brimgsize;
    int hf,x1,y1,x2,y2,sx,sy;
    int i,j,xx,yy,fstx,fsty,fx,fy,add_cos,add_sin,add_cos2,add_sin2,c;
    int bx,by,bfx,bfy,bfx2,bfy2;
    double scaled,dsin,dcos;
    BYTE bDrawTP,bFlagNoEx;
    BRUSHDRAWPARAM *p = &m_pinfo->bparam;
    void (CLayerImg::*funcPix)(int,int,BYTE) = m_pinfo->funcDrawPixel;

    if(val == 0) return;

    //上書き2、透明部分も描画するか

    bDrawTP = (m_pinfo->nColSetFunc == COLSETF_OVERWRITE2);

    //点の大きさによりサブピクセル数調整

    isize = (int)size;

    if(isize < 8) subnum = 11;
    else if(isize < 40) subnum = 9;
    else if(isize < 150) subnum = 5;
    else subnum = 3;

    isize = (int)(size * subnum + 0.5);
    if(isize < 2) return;

    subnum2 = subnum * subnum;

    //回転時用に1.44倍ほど大きめに描画

    isize_draw = isize * 13 / 9;

    //

    ix  = (int)(xpos * subnum + 0.5);   //[sub]中心位置
    iy  = (int)(ypos * subnum + 0.5);

    hf  = isize_draw >> 1;              //[sub]半径

    sx  = ix - hf;                      //[sub]左上位置
    sy  = iy - hf;
    x1  = sx / subnum;                  //[px]描画左上位置
    y1  = sy / subnum;
    x2  = (sx + isize_draw - 1) / subnum;   //[px]描画右下位置
    y2  = (sy + isize_draw - 1) / subnum;

    brimgsize = (m_pinfo->pBrush)->getWidth();

    scaled = (double)brimgsize / isize * (1 << 12);

    //回転用

    xx = (x1 * subnum) - ix;
    yy = (y1 * subnum) - iy;

    dcos = m_pSinTbl->getCos(-(p->nRotAngleRes));
    dsin = m_pSinTbl->getSin(-(p->nRotAngleRes));

    fstx = (int)((xx * dcos - yy * dsin) * scaled) + (brimgsize << 11);
    fsty = (int)((xx * dsin + yy * dcos) * scaled) + (brimgsize << 11);

    add_cos = (int)(dcos * scaled);
    add_sin = (int)(dsin * scaled);
    add_cos2 = add_cos * subnum;
    add_sin2 = add_sin * subnum;

    //xx,yy:cos, xy:sin, yx:-sin

    //--------------

    for(yy = y1; yy <= y2; yy++)
    {
        fx = fstx;
        fy = fsty;

        for(xx = x1; xx <= x2; xx++)
        {
            //サブピクセル平均

            bfx = fx;
            bfy = fy;

            c = 0;
            bFlagNoEx = FALSE;  //FALSE ですべて範囲外

            for(i = subnum; i > 0; i--)
            {
                bfx2 = bfx;
                bfy2 = bfy;

                for(j = subnum; j > 0; j--)
                {
                    bx = bfx2 >> 12;
                    by = bfy2 >> 12;

                    if(bx >= 0 && bx < brimgsize && by >= 0 && by < brimgsize)
                    {
                        c += (m_pinfo->pBrush)->getPixel(bx, by);

                        bFlagNoEx = TRUE;
                    }

                    bfx2 += add_cos;
                    bfy2 += add_sin;
                }

                bfx -= add_sin;
                bfy += add_cos;
            }

            c /= subnum2;

            //点セット
            /* 完全上書き2 時は、回転時に範囲外の部分は描画しない */

            if(c || (bDrawTP && bFlagNoEx))
            {
                if(c)
                {
                    if(p->bAnti)
                        c = (val * c + 127) / 255;
                    else
                        c = val;
                }

                (this->*funcPix)(xx, yy, c);
            }

            //

            fx += add_cos2;
            fy += add_sin2;
        }

        fstx -= add_sin2;
        fsty += add_cos2;
    }
}
