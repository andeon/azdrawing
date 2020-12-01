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
    CLayerImg [draw] - 描画関連
*/

#include <math.h>
#include <stdlib.h>

#include "CLayerImg.h"

#include "defStruct.h"
#include "CPolygonPos.h"
#include "CSplinePos.h"



//! ドット線直線（ブレゼンハム）
/*!
    @param bNoStart 開始位置は点を打たない
*/

void CLayerImg::drawLineB(int x1,int y1,int x2,int y2,BYTE col,BOOL bNoStart)
{
    int sx,sy,dx,dy,a,a1,e;
    void (CLayerImg::*func)(int,int,BYTE) = m_pinfo->funcDrawPixel;

    if(x1 == x2 && y1 == y2)
    {
        if(!bNoStart) (this->*func)(x1, y1, col);
        return;
    }

    //

    dx = (x1 < x2)? x2 - x1: x1 - x2;
    dy = (y1 < y2)? y2 - y1: y1 - y2;
    sx = (x1 <= x2)? 1: -1;
    sy = (y1 <= y2)? 1: -1;

    if(dx >= dy)
    {
        a  = 2 * dy;
        a1 = a - 2 * dx;
        e  = a - dx;

        if(bNoStart)
        {
            if(e >= 0) y1 += sy, e += a1;
            else e += a;
            x1 += sx;
        }

        while(x1 != x2)
        {
            (this->*func)(x1, y1, col);

            if(e >= 0) y1 += sy, e += a1;
            else e += a;
            x1 += sx;
        }
    }
    else
    {
        a  = 2 * dx;
        a1 = a - 2 * dy;
        e  = a - dy;

        if(bNoStart)
        {
            if(e >= 0) x1 += sx, e += a1;
            else e += a;
            y1 += sy;
        }

        while(y1 != y2)
        {
            (this->*func)(x1, y1, col);

            if(e >= 0) x1 += sx, e += a1;
            else e += a;
            y1 += sy;
        }
    }

    (this->*func)(x1, y1, col);
}

//! ドット線直線（固定小数点）
/*!
    ※終点は描画しない（ドット細線用）
*/

void CLayerImg::drawLineF(int x1,int y1,int x2,int y2,BYTE val)
{
    int dx,dy,f,d,add = 1;
    void (CLayerImg::*func)(int,int,BYTE) = m_pinfo->funcDrawPixel;

    if(x1 == x2 && y1 == y2) return;

    dx = (x1 < x2)? x2 - x1: x1 - x2;
    dy = (y1 < y2)? y2 - y1: y1 - y2;

    if(dx > dy)
    {
        f = y1 << 16;
        d = ((y2 - y1) << 16) / (x2 - x1);
        if(x1 > x2) add = -1, d = -d;

        while(x1 != x2)
        {
            (this->*func)(x1, (f + 0x8000) >> 16, val);
            f  += d;
            x1 += add;
        }
    }
    else
    {
        f = x1 << 16;
        d = ((x2 - x1) << 16) / (y2 - y1);
        if(y1 > y2) add = -1, d = -d;

        while(y1 != y2)
        {
            (this->*func)((f + 0x8000) >> 16, y1, val);
            f  += d;
            y1 += add;
        }
    }
}

//! 円描画

void CLayerImg::drawCircle(double cx,double cy,double xr,double yr,
    double *dParam,BOOL bHRev,BOOL bBrush)
{
    int i,div;
    double add,rr,xx,yy,x1,y1,sx,sy,nx,ny;

    //開始位置（0度）

    x1 = xr * dParam[2]; if(bHRev) x1 = -x1;

    sx = cx + x1;
    sy = cy + xr * dParam[3];

    //分割数

    rr = (xr > yr)? xr: yr;

    div = (int)((rr * M_PI * 2) / 4.0);
    if(div < 30) div = 30; else if(div > 400) div = 400;

    //

    add = M_PI * 2 / div;
    rr  = add;

    for(i = 0; i < div; i++, rr += add)
    {
        xx = xr * ::cos(rr);
        yy = yr * ::sin(rr);

        x1 = xx * dParam[2] + yy * dParam[3]; if(bHRev) x1 = -x1;
        y1 = xx * dParam[3] - yy * dParam[2];

        nx = x1 + cx;
        ny = y1 + cy;

        if(bBrush)
            drawBrush_line(sx, sy, nx, ny);
        else
            drawLineB((int)(sx + 0.5), (int)(sy + 0.5), (int)(nx + 0.5), (int)(ny + 0.5), 255, i);

        sx = nx, sy = ny;
    }
}

//! ベジェ曲線

void CLayerImg::drawBezier(double *pos,WORD wHeadTail,BOOL bBrush)
{
    double ind,outd;
    double nx,ny,sx,sy;
    double t,tt,t1,t2,t3,t4,len,add;
    int i,head,tail,div,nT;
    BRUSHDRAWPARAM *p = &m_pinfo->bparam;

    //入り抜き

    if(bBrush)
    {
        head = wHeadTail >> 8;
        tail = wHeadTail & 0xff;

        ind  = (head)? 1.0 / ((double)head / 100.0): 0;
        outd = (tail)? 1.0 / ((double)tail / 100.0): 0;

        p->dPressSt = (head)? 0: 1.0;
    }

    //4分割で長さ取得 -> 分割数計算

    sx = pos[0], sy = pos[1];

    for(i = 0, t = 0.25, len = 0; i < 3; i++, t += 0.25)
    {
        tt = 1 - t;

        t1 = tt * tt * tt;
        t2 = 3 * t * tt * tt;
        t3 = 3 * t * t * tt;
        t4 = t * t * t;

        nx = pos[0] * t1 + pos[4] * t2 + pos[6] * t3 + pos[2] * t4;
        ny = pos[1] * t1 + pos[5] * t2 + pos[7] * t3 + pos[3] * t4;

        len += ::sqrt((nx - sx) * (nx - sx) + (ny - sy) * (ny - sy));

        sx = nx, sy = ny;
    }

    if(len == 0) return;

    div = (int)(len / 4.0);
    if(div < 30) div = 30; else if(div > 400) div = 400;

    add = 1.0 / div;

    //曲線

    sx = pos[0], sy = pos[1];

    for(i = 0, t = add; i < div; i++, t += add)
    {
        tt = 1 - t;

        t1 = tt * tt * tt;
        t2 = 3 * t * tt * tt;
        t3 = 3 * t * t * tt;
        t4 = t * t * t;

        nx = pos[0] * t1 + pos[4] * t2 + pos[6] * t3 + pos[2] * t4;
        ny = pos[1] * t1 + pos[5] * t2 + pos[7] * t3 + pos[3] * t4;

        //描画

        if(!bBrush)
            drawLineB((int)sx, (int)sy, (int)nx, (int)ny, 255, i);
        else
        {
            nT = (int)(t * 100);

            if(nT < head)
                p->dPressEd = t * ind;
            else if(nT > 100 - tail)
                p->dPressEd = (1.0 - t) * outd;
            else
                p->dPressEd = 1.0;

            drawBrush_line(sx, sy, nx, ny);

            p->dPressSt = p->dPressEd;
        }

        //

        sx = nx, sy = ny;
    }
}


//-------------------

typedef struct
{
    double a,b,c;
}SPLINETMP;

//! スプライン曲線

void CLayerImg::drawSpline(CSplinePos *pPos,BOOL bBrush)
{
    CSplinePos::POSDAT *p,*pBuf;
    int cnt,i,j;
    double bx,by,nx,ny;
    double *pLength = NULL,a,b;
    double xx1,xx2,yy1,yy2,t;
    float press_len,press_t;
    SPLINETMP *pTempA = NULL;
    DPOINT *pTempB = NULL, *pTempC = NULL;
    BRUSHDRAWPARAM *pParam = &m_pinfo->bparam;

    pBuf = pPos->getBuf();
    cnt  = pPos->getCnt();

    if(!pBuf || cnt < 2) return;

    //メモリ確保

    pLength = (double *)AXMalloc(sizeof(double) * cnt);
    pTempA = (SPLINETMP *)AXMalloc(sizeof(SPLINETMP) * cnt);
    pTempB = (DPOINT *)AXMalloc(sizeof(DPOINT) * cnt);
    pTempC = (DPOINT *)AXMalloc(sizeof(DPOINT) * cnt);

    if(!pLength || !pTempA || !pTempB || !pTempC) goto END;

    //各線の長さ計算

    for(i = 0, p = pBuf; i < cnt - 1; i++, p++)
    {
        pLength[i] = ::sqrt((p[0].x - p[1].x) * (p[0].x - p[1].x) +
                            (p[0].y - p[1].y) * (p[0].y - p[1].y));
    }

    //A,Bパラメータ

    pTempA[0].a = 0, pTempA[0].b = 1, pTempA[0].c = 0.5;
    pTempA[cnt - 1].a = 1, pTempA[cnt - 1].b = 2, pTempA[cnt - 1].c = 0;

    pTempB[0].x = (3 / (2 * pLength[0])) * (pBuf[1].x - pBuf[0].x);
    pTempB[0].y = (3 / (2 * pLength[0])) * (pBuf[1].y - pBuf[0].y);
    pTempB[cnt - 1].x = (3 / pLength[cnt - 2]) * (pBuf[cnt - 1].x - pBuf[cnt - 2].x);
    pTempB[cnt - 1].y = (3 / pLength[cnt - 2]) * (pBuf[cnt - 1].y - pBuf[cnt - 2].y);

    //

    for(i = 1, p = pBuf + 1; i < cnt - 1; i++, p++)
    {
        a = pLength[i - 1];
        b = pLength[i];

        pTempA[i].a = b, pTempA[i].b = 2.0 * (b + a), pTempA[i].c = a;

        pTempB[i].x = (3.0 * (a * a * (p[1].x - p[0].x)) + 3.0 * b * b * (p[0].x - p[-1].x)) / (b * a);
        pTempB[i].y = (3.0 * (a * a * (p[1].y - p[0].y)) + 3.0 * b * b * (p[0].y - p[-1].y)) / (b * a);
   	}

    //

    for(i = 1; i < cnt; i++)
    {
        a = pTempA[i - 1].b / pTempA[i].a;

        pTempA[i].a = 0;
        pTempA[i].b = pTempA[i].b * a - pTempA[i - 1].c;
        pTempA[i].c = pTempA[i].c * a;

        pTempB[i].x = pTempB[i].x * a - pTempB[i - 1].x;
        pTempB[i].y = pTempB[i].y * a - pTempB[i - 1].y;

        pTempA[i].c /= pTempA[i].b;
        pTempB[i].x /= pTempA[i].b;
        pTempB[i].y /= pTempA[i].b;
        pTempA[i].b = 1;
    }

    //

    pTempC[cnt - 1].x = pTempB[cnt - 1].x;
    pTempC[cnt - 1].y = pTempB[cnt - 1].y;

    for(i = cnt - 1; i > 0; i--)
    {
        pTempC[i - 1].x = pTempB[i - 1].x - pTempA[i - 1].c * pTempC[i].x;
        pTempC[i - 1].y = pTempB[i - 1].y - pTempA[i - 1].c * pTempC[i].y;
    }

    //描画

    bx = pBuf[0].x, by = pBuf[0].y;

    for(i = 0, p = pBuf; i < cnt - 1; i++, p++)
    {
        a = pLength[i];
        b = a / 20;

        xx1 = (p[1].x - p[0].x) * 3 / (a * a) - (pTempC[i + 1].x + 2 * pTempC[i].x) / a;
        xx2 = (p[1].x - p[0].x) * (-2 / (a * a * a)) + (pTempC[i + 1].x + pTempC[i].x) * (1 / (a * a));

        yy1 = (p[1].y - p[0].y) * 3 / (a * a) - (pTempC[i + 1].y + 2 * pTempC[i].y) / a;
        yy2 = (p[1].y - p[0].y) * (-2 / (a * a * a)) + (pTempC[i + 1].y + pTempC[i].y) * (1 / (a * a));

        //

        pParam->dPressSt = p[0].press;
        press_len = p[1].press - p[0].press;

        for(j = 0, t = b, press_t = 0.05f; j < 20; j++, t += b, press_t += 0.05f)
        {
            nx = ((xx2 * t + xx1) * t + pTempC[i].x) * t + p->x;
            ny = ((yy2 * t + yy1) * t + pTempC[i].y) * t + p->y;

            if(bBrush)
            {
                pParam->dPressEd = p[0].press + press_len * press_t;

                drawBrush_line(bx, by, nx, ny);

                pParam->dPressSt = pParam->dPressEd;
            }
            else
                drawLineB((int)bx, (int)by, (int)nx, (int)ny, 255, FALSE);

            bx = nx, by = ny;
        }
    }

END:
    AXFree((void **)&pLength);
    AXFree((void **)&pTempA);
    AXFree((void **)&pTempB);
    AXFree((void **)&pTempC);
}

//! 多角形塗りつぶし

BOOL CLayerImg::drawFillPolygon(CPolygonPos *pPos,int val,BOOL bAnti)
{
    int miny,maxy,y,minx,xw,i,x1,c;
    AXRect rc;
    LPWORD pValBuf,pw;

    if(pPos->getPosCnt() < 4) return FALSE;

    //Yの範囲

    getEnableDrawRectPixel(&rc);

    miny = pPos->getMinY();
    maxy = pPos->getMaxY();

    if(miny > rc.bottom) return FALSE;
    if(maxy < rc.top) return FALSE;

    if(miny < rc.top) miny = rc.top;
    if(maxy > rc.bottom) maxy = rc.bottom;

    //交点バッファ確保

    if(!pPos->allocEdge()) return FALSE;

    //濃度バッファ

    pValBuf = pPos->getValBuf();
    minx    = pPos->getMinX();
    xw      = pPos->getMaxWidth();

    //-------- Yの範囲でスキャン

    for(y = miny; y <= maxy; y++)
    {
        //交点リスト

        if(!pPos->findIntersection(y)) continue;

        //濃度バッファセット

        pPos->setValBuf();

        //描画

        pw = pValBuf;
        x1 = minx;

        if(bAnti)
        {
            for(i = xw; i > 0; i--, pw++, x1++)
            {
                c = (*pw) * val / 255;
                if(c) setPixelDraw(x1, y, c);
            }
        }
        else
        {
            for(i = xw; i > 0; i--, pw++, x1++)
            {
                if(*pw) setPixelDraw(x1, y, val);
            }
        }
    }

    return TRUE;
}

//! 円塗りつぶし描画

void CLayerImg::drawFillCircle(double cx,double cy,double xr,double yr,
    double *dParam,BOOL bHRev,int val,BOOL bAnti)
{
    int nx,ny,i,j,c,flag;
    double xx,yy,rr,x1,y1,dx,dy,xx2,yy2,xt;
    DPOINT pt[4];
    AXRect rc;

    //--------- 描画範囲計算 (10度単位)

    for(i = 0, rr = 0; i < 36; i++, rr += M_PI / 18)
    {
        xx = xr * ::cos(rr);
        yy = yr * ::sin(rr);

        x1 = xx * dParam[2] + yy * dParam[3]; if(bHRev) x1 = -x1;
        y1 = xx * dParam[3] - yy * dParam[2];

        xx = x1 + cx;
        yy = y1 + cy;

        nx = (int)xx, ny = (int)yy;

        if(i == 0)
        {
            rc.left = rc.right = nx;
            rc.top = rc.bottom = ny;
        }
        else
        {
            if(nx < rc.left  ) rc.left = nx;
            if(ny < rc.top   ) rc.top = ny;
            if(nx > rc.right ) rc.right = nx;
            if(ny > rc.bottom) rc.bottom = ny;
        }
    }

    //一応拡張

    rc.left -= 2, rc.top -= 2;
    rc.right += 2, rc.bottom += 2;

    //範囲内に調整

    if(!clipRectInEnableDraw(&rc)) return;

    //-------------

    if(xr < yr)
    {
        rr = xr * xr;
        dx = 1.0, dy = xr / yr;
    }
    else
    {
        rr = yr * yr;
        dx = yr / xr, dy = 1.0;
    }

    //--------- 描画

    yy = rc.top - cy;

    for(ny = rc.top; ny <= rc.bottom; ny++, yy += 1.0)
    {
        xx = rc.left - cx;

        for(nx = rc.left; nx <= rc.right; nx++, xx += 1.0)
        {
            if(bAnti)
            {
                //-------- アンチエイリアス - 4x4

                //内外判定

                pt[0].x = pt[3].x = xx;
                pt[0].y = pt[1].y = yy;
                pt[1].x = pt[2].x = xx + 1;
                pt[2].y = pt[3].y = yy + 1;

                for(i = 0, flag = 0; i < 4; i++)
                {
                    xt = pt[i].x;
                    if(bHRev) xt = -xt;

                    x1 = xt * dParam[4] - pt[i].y * dParam[5];
                    y1 = xt * dParam[5] + pt[i].y * dParam[4];

                    x1 *= dx;
                    y1 *= dy;

                    if(x1 * x1 + y1 * y1 <= rr) flag |= (1 << i);
                }

                if(flag == 0) continue;

                if(flag == 15)
                {
                    setPixelDraw(nx, ny, val);
                    continue;
                }

                //4x4 平均

                c = 0;

                for(i = 0, yy2 = yy; i < 4; i++, yy2 += 0.25)
                {
                    for(j = 0, xx2 = xx; j < 4; j++, xx2 += 0.25)
                    {
                        xt = xx2;
                        if(bHRev) xt = -xt;

                        x1 = xt * dParam[4] - yy2 * dParam[5];
                        y1 = xt * dParam[5] + yy2 * dParam[4];

                        x1 *= dx;
                        y1 *= dy;

                        if(x1 * x1 + y1 * y1 < rr)
                            c += 255;
                    }
                }

                c = (c >> 4) * val / 255;
                if(c) setPixelDraw(nx, ny, c);
            }
            else
            {
                //--------- 非アンチ

                xt = xx;
                if(bHRev) xt = -xt;

                x1 = xt * dParam[4] - yy * dParam[5];
                y1 = xt * dParam[5] + yy * dParam[4];

                x1 *= dx;
                y1 *= dy;

                if(x1 * x1 + y1 * y1 < rr)
                    setPixelDraw(nx, ny, val);
            }
        }
    }
}


//===========================
// グラデーション
//===========================


//! グラデーション（線形）
/*!
    @param rc   描画する矩形範囲(px)
    @param flag 1bit:繰り返し
*/

void CLayerImg::drawGradient_line(int x1,int y1,int x2,int y2,const AXRect &rc,LPBYTE pTable,int flag)
{
    int x,y,c;
    double abx,aby,abab,dd,dd2,addx,addy;

    if(x1 == x2 && y1 == y2) return;

    abx  = x2 - x1;
    aby  = y2 - y1;
    abab = abx * abx + aby * aby;

    dd   = abx * (rc.left - x1) + aby * (rc.top - y1);
    dd   = dd * 1023 / abab;

    addx = abx * 1023 / abab;
    addy = aby * 1023 / abab;

    //

    for(y = rc.top; y <= rc.bottom; y++, dd += addy)
    {
        for(x = rc.left, dd2 = dd; x <= rc.right; x++, dd2 += addx)
        {
            c = (int)dd2;

        /*--------
            abap = abx * (x - x1) + aby * (y - y1);
            c = (int)(abap * 1023 / abab);

            x が +1 するごとに abx * 1 * 1023 / abab,
            y が +1 するごとに aby * 1 * 1023 / abab,
            +1 するごとの差分は一定なので、開始時点での c 値を求めておいて、加算していく
        --------*/

            if(flag & 1)
                //繰り返し
                c &= 1023;
            else
            {
                if(c < 0) c = 0;
                else if(c > 1023) c = 1023;
            }

            setPixelDraw(x, y, pTable[c]);
        }
    }
}

//! グラデーション（円形）

void CLayerImg::drawGradient_circle(int x1,int y1,int x2,int y2,const AXRect &rc,LPBYTE pTable,int flag)
{
    int ix,iy,c;
    double val,x,y;

    if(x1 == x2 && y1 == y2) return;

    x = x1 - x2;
    y = y1 - y2;

    val = 1023.0 / ::sqrt(x * x + y * y);

    //

    for(iy = rc.top; iy <= rc.bottom; iy++)
    {
        y = iy - y1;
        y = y * y;

        for(ix = rc.left; ix <= rc.right; ix++)
        {
            x = ix - x1;

            c = (int)(::sqrt(x * x + y) * val);

        /*--------
            c = (ini)( sqrt((ix - x1) * (ix - x1) + (iy - y1) * (iy - y1)) * 1023 /
                        sqrt((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2) );
        --------*/

            if(flag & 1)
                c &= 1023;
            else
            {
                if(c > 1023) c = 1023;
            }

            setPixelDraw(ix, iy, pTable[c]);
        }
    }
}

//! グラデーション（矩形）

void CLayerImg::drawGradient_box(int x1,int y1,int x2,int y2,const AXRect &rc,LPBYTE pTable,int flag)
{
    int ix,iy,c,absy;
    double val,x,y,xx,yy,dx,dy;

    if(x1 == x2 && y1 == y2) return;

    x = ::abs(x2 - x1);
    y = ::abs(y2 - y1);

    val = 1023.0 / ::sqrt(x * x + y * y);

    dx = (y1 == y2)? 0: x / y;
    dy = (x1 == x2)? 0: y / x;

    //

    for(iy = rc.top; iy <= rc.bottom; iy++)
    {
        absy = (y1 < iy)? iy - y1: y1 - iy;
        xx   = absy * dx;

        for(ix = rc.left; ix <= rc.right; ix++)
        {
            x = (x1 < ix)? ix - x1: x1 - ix;
            y = absy;

            if(y1 == y2)
                y = 0;
            else if(x < xx)
                x = xx;

            if(x1 == x2)
                x = 0;
            else
            {
                yy = x * dy;
                if(y < yy) y = yy;
            }

        /*--------
            x = abs(x1 - ix);
            y = abs(y1 - iy);

            if(y1 == y2)
                y = 0;
            else
            {
                xx = y * abs(x1 - x2) / abs(y1 - y2);
                if(xx > x) x = xx;
            }

            if(x1 == x2)
                x = 0;
            else
            {
                yy = x * abs(y1 - y2) / abs(x1 - x2);
                if(yy > y) y = yy;
            }
        --------*/

            //

            c = (int)(::sqrt(x * x + y * y) * val);

            if(flag & 1)
                c &= 1023;
            else
            {
                if(c > 1023) c = 1023;
            }

            setPixelDraw(ix, iy, pTable[c]);
        }
    }
}


//===========================
// 作業用
//===========================


//! 作業用に多角形塗りつぶし

BOOL CLayerImg::drawFillPolygonTmp(CPolygonPos *pPos,int val,void (CLayerImg::*funcPix)(int,int,BYTE))
{
    int miny,maxy,y,edgecnt,i,j,cnt,xx,x2;
    CPolygonPos::EDGEDAT *pEdge,*pe,*pe1;
    AXRect rc;

    if(pPos->getPosCnt() < 4) return FALSE;

    //Yの範囲

    getEnableDrawRectPixel(&rc);

    miny = pPos->getMinY();
    maxy = pPos->getMaxY();

    if(miny > rc.bottom || maxy < rc.top) return FALSE;

    if(miny < rc.top) miny = rc.top;
    if(maxy > rc.bottom) maxy = rc.bottom;

    //交点バッファ確保

    if(!pPos->allocEdge()) return FALSE;

    //-------- Yの範囲でスキャン

    for(y = miny; y <= maxy; y++)
    {
        //交点リスト

        if(!pPos->findIntersectionNoAnti(y)) continue;

        //水平線

        pEdge   = pPos->getEdgeBuf();
        edgecnt = pPos->getEdgeCnt();

        for(i = 0, pe = pEdge; i < edgecnt - 1; i++, pe++)
        {
            //カウント

            for(j = 0, cnt = 0, pe1 = pEdge; j <= i; j++, pe1++)
            {
                if(pe1->bUpper) cnt++; else cnt--;
            }

            if(cnt == 0) continue;

            //塗りつぶし

            x2 = pe[1].x[0];

            for(xx = pe->x[0]; xx <= x2; xx++)
                (this->*funcPix)(xx, y, val);
        }
    }

    return TRUE;
}

