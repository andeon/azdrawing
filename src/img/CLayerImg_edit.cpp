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
    CLayerImg [edit] - 編集関連
*/

#include <math.h>

#include "CLayerImg.h"

#include "CTileImg.h"
#include "CImage8.h"
#include "defStruct.h"
#include "AXRect.h"


//! 全体を左右反転

void CLayerImg::reverseHorzFull()
{
    int cnt,x,y;
    CTileImg **pp,**pp2,*pt;

    //オフセット

    m_nOffX = (m_pinfo->nImgW - 1) - m_nOffX - (m_nTileXCnt * 64 - 1);

    //各タイルを左右反転

    pp = m_ppTile;

    for(x = m_nTileXCnt * m_nTileYCnt; x > 0; x--, pp++)
    {
        if(*pp) (*pp)->reverseHorz();
    }

    //配列の位置を左右反転

    cnt	= m_nTileXCnt / 2;

    for(y = 0; y < m_nTileYCnt; y++)
    {
        pp  = m_ppTile + y * m_nTileXCnt;
        pp2 = pp + m_nTileXCnt - 1;

        for(x = 0; x < cnt; x++, pp++, pp2--)
        {
            pt   = *pp;
            *pp  = *pp2;
            *pp2 = pt;
        }
    }
}

//! 範囲左右反転
/*!
    @param rcs イメージ範囲
*/

void CLayerImg::reverseHorz(const AXRectSize &rcs)
{
    int cnt,ix,iy,x1,x2,y;
    BYTE c1,c2;

    x1  = rcs.x;
    x2  = rcs.x + rcs.w - 1;
    cnt = rcs.w / 2;

    for(iy = rcs.h, y = rcs.y; iy > 0; iy--, y++)
    {
        for(ix = 0; ix < cnt; ix++)
        {
            c1 = getPixel(x1 + ix, y);
            c2 = getPixel(x2 - ix, y);

            setPixelDraw(x1 + ix, y, c2);
            setPixelDraw(x2 - ix, y, c1);
        }
    }
}

//! 範囲上下反転

void CLayerImg::reverseVert(const AXRectSize &rcs)
{
    int c1,c2,ix,iy,y1,y2,x;

    y1 = rcs.y;
    y2 = rcs.y + rcs.h - 1;

    for(iy = rcs.h / 2; iy > 0; iy--, y1++, y2--)
    {
        for(ix = rcs.w, x = rcs.x; ix > 0; ix--, x++)
        {
            c1 = getPixel(x, y1);
            c2 = getPixel(x, y2);

            setPixelDraw(x, y1, c2);
            setPixelDraw(x, y2, c1);
        }
    }
}

//! 矩形塗りつぶし

void CLayerImg::fillBox(const AXRect &rc,BYTE val,void (CLayerImg::*funcPix)(int,int,BYTE))
{
    int x,y;

    for(y = rc.top; y <= rc.bottom; y++)
        for(x = rc.left; x <= rc.right; x++)
            (this->*funcPix)(x, y, val);
}


//===========================
//選択範囲用
//===========================


//! 選択範囲のイメージコピー
/*!
    selImg で点がある範囲を srcImg から this へコピー。@n
    setPixel_subDraw() で点をセットする。
*/

void CLayerImg::copySelectImage(const CLayerImg &srcImg,const CLayerImg &selImg)
{
    CTileImg **pp;
    LPBYTE ps;
    int x,y,xcnt,ycnt,topx,topy,px,py,xx,yy;
    BYTE val;

    pp   = selImg.getTileBuf();
    xcnt = selImg.getTileXCnt();
    ycnt = selImg.getTileYCnt();

    selImg.calcTileToPixel(&topx, &topy, 0, 0);

    //

    for(y = 0, py = topy; y < ycnt; y++, py += 64)
    {
        for(x = 0, px = topx; x < xcnt; x++, px += 64, pp++)
        {
            if(!(*pp)) continue;

            ps = (*pp)->getBuf();

            for(yy = 0; yy < 64; yy++)
            {
                for(xx = 0; xx < 64; xx++, ps++)
                {
                    if(*ps)
                    {
                        val = srcImg.getPixel(px + xx, py + yy);
                        if(val) setPixel_subDraw(px + xx, py + yy, val);
                    }
                }
            }
        }
    }
}

//! 選択範囲内を消去
/*!
    setPixelDraw() で点をセット。@n
    （消去すると同時にアンドゥイメージを作らせる）
*/

void CLayerImg::cutSelectImage(const CLayerImg &selImg)
{
    CTileImg **pp;
    LPBYTE ps;
    int x,y,xcnt,ycnt,topx,topy,px,py,xx,yy;

    pp   = selImg.getTileBuf();
    xcnt = selImg.getTileXCnt();
    ycnt = selImg.getTileYCnt();

    selImg.calcTileToPixel(&topx, &topy, 0, 0);

    //

    for(y = 0, py = topy; y < ycnt; y++, py += 64)
    {
        for(x = 0, px = topx; x < xcnt; x++, px += 64, pp++)
        {
            if(!(*pp)) continue;

            ps = (*pp)->getBuf();

            for(yy = 0; yy < 64; yy++)
            {
                for(xx = 0; xx < 64; xx++, ps++)
                {
                    if(*ps)
                        setPixelDraw(px + xx, py + yy, 0);
                }
            }
        }
    }
}

//! 選択範囲のドラッグ移動後の合成
/*!
    @param srcImg     貼り付けるイメージ
    @param bOverwrite 上書き
*/

void CLayerImg::blendSelImage(const CLayerImg &srcImg,const CLayerImg &selImg,BOOL bOverwrite)
{
    CTileImg **pp;
    LPBYTE ps;
    int x,y,xcnt,ycnt,topx,topy,px,py,xx,yy;

    pp   = srcImg.getTileBuf();
    xcnt = srcImg.getTileXCnt();
    ycnt = srcImg.getTileYCnt();

    srcImg.calcTileToPixel(&topx, &topy, 0, 0);

    //

    for(y = 0, py = topy; y < ycnt; y++, py += 64)
    {
        for(x = 0, px = topx; x < xcnt; x++, px += 64, pp++)
        {
            if(!(*pp)) continue;

            ps = (*pp)->getBuf();

            for(yy = 0; yy < 64; yy++)
            {
                for(xx = 0; xx < 64; xx++, ps++)
                {
                    if(bOverwrite)
                    {
                        //上書き（選択範囲で点があれば処理）

                        if(selImg.getPixel(px + xx, py + yy))
                            setPixelDraw(px + xx, py + yy, *ps);
                    }
                    else
                    {
                        //合成（貼り付けイメージで点があれば処理）

                        if(*ps)
                            setPixelDraw(px + xx, py + yy, *ps);
                    }
                }
            }
        }
    }
}

//! 選択範囲インポート
/*!
    @param img8Layer 確保されていなければ選択範囲のみ
*/

void CLayerImg::inportSel(const CImage8 &img8Sel,const CImage8 &img8Layer,CLayerImg *pimgSel,const AXRectSize &rcs)
{
    int ix,iy,x,y;
    LPBYTE pLayer,pSel;

    pSel   = img8Sel.getBuf();
    pLayer = img8Layer.getBuf();    //NULLの場合あり

    for(iy = rcs.h, y = rcs.y; iy > 0; iy--, y++)
    {
        for(ix = rcs.w, x = rcs.x; ix > 0; ix--, x++, pSel++)
        {
            if(*pSel)
            {
                //イメージ

                if(pLayer)
                    setPixelDraw(x, y, *pLayer);

                //選択範囲

                pimgSel->setPixel_create(x, y, 255);
            }

            if(pLayer) pLayer++;
        }
    }
}


//=============================
//拡大縮小＆回転
//=============================


//! 拡大縮小＆回転

void CLayerImg::scaleAndRotate(const CImage8 &img8,const AXRect &rc,
    double dScale,double dAngle,BOOL bHiQuality,
    void (CLayerImg::*funcPix)(int,int,BYTE))
{
    AXRect rcDraw;
    int cx,cy,x,y,ix,iy,w,h,sw,isx,isy,c,xx,yy,type;
    double dcos,dsin,dcos_div,dsin_div,wx[4],wy[4],wf;
    double nxxx,nyyy,nxx,nyy,nxxs,nyys,nxxs2,nyys2;
    LPBYTE pSrcBuf;

    //描画範囲

    rcDraw = rc;
    if(!getScaleRotRect(&rcDraw, dScale, dAngle)) return;

    //

    if(bHiQuality)
        type = (dScale <= 0.9)? 1: 2;
    else
        type = 0;

    //

    w = rc.right - rc.left + 1;
    h = rc.bottom - rc.top + 1;

    cx = w / 2;
    cy = h / 2;

    dScale = 1.0 / dScale;
    dcos   = ::cos(-dAngle);
    dsin   = ::sin(-dAngle);

    x    = rcDraw.left - (rc.left + cx);
    y    = rcDraw.top  - (rc.top + cy);
    nxxx = (x * dcos - y * dsin) * dScale + cx;
    nyyy = (x * dsin + y * dcos) * dScale + cy;

    dcos *= dScale;
    dsin *= dScale;
    dcos_div = dcos / 5;
    dsin_div = dsin / 5;

    pSrcBuf = img8.getBuf();
    sw      = img8.getWidth();

    //----------------

    for(y = rcDraw.top; y <= rcDraw.bottom; y++)
    {
        nxx = nxxx;
        nyy = nyyy;

        for(x = rcDraw.left; x <= rcDraw.right; x++, nxx += dcos, nyy += dsin)
        {
            //範囲外

            ix = (int)nxx;
            iy = (int)nyy;

            if(ix < 0 || iy < 0 || ix >= w || iy >= h) continue;

            //色

            switch(type)
            {
                //低品質
                case 0:
                    c = *(pSrcBuf + iy * sw + ix);
                    break;

                //高品質 縮小用: 5x5
                case 1:
                    c = 0;
                    nxxs2 = nxx, nyys2 = nyy;

                    for(isy = 0; isy < 5; isy++)
                    {
                        nxxs = nxxs2, nyys = nyys2;

                        for(isx = 0; isx < 5; isx++)
                        {
                            ix = (int)nxxs;
                            iy = (int)nyys;

                            if(ix < 0) ix = 0; else if(ix >= w) ix = w - 1;
                            if(iy < 0) iy = 0; else if(iy >= h) iy = h - 1;

                            c += *(pSrcBuf + iy * sw + ix);

                            nxxs += dcos_div;
                            nyys += dsin_div;
                        }

                        nxxs2 -= dsin_div;
                        nyys2 += dcos_div;
                    }

                    c /= 25;
                    break;

                //高品質 拡大用: Lagrange
                default:
                    //重み

                    for(isx = 0; isx < 4; isx++)
                    {
                        //X

                        wf = ::fabs(nxx - (ix - 1 + isx));

                        if(wf < 1) wx[isx] = 0.5 * (wf - 2.0) * (wf + 1.0) * (wf - 1.0);
                        else wx[isx] = -(wf - 3.0) * (wf - 2.0) * (wf - 1.0) / 6.0;

                        //Y

                        wf = ::fabs(nyy - (iy - 1 + isx));

                        if(wf < 1) wy[isx] = 0.5 * (wf - 2.0) * (wf + 1.0) * (wf - 1.0);
                        else wy[isx] = -(wf - 3.0) * (wf - 2.0) * (wf - 1.0) / 6.0;
                    }

                    //4x4 近傍

                    wf = 0;

                    for(isy = 0, yy = iy - 1; isy < 4; isy++, yy++)
                    {
                        for(isx = 0, xx = ix - 1; isx < 4; isx++, xx++)
                        {
                            if(xx >= 0 && xx < w && yy >= 0 && yy < h)
                                wf += *(pSrcBuf + yy * sw + xx) * wx[isx] * wy[isy];
                        }
                    }

                    c = (int)(wf + 0.5);
                    if(c < 0) c = 0; else if(c > 255) c = 255;
                    break;
            }

            //セット

            if(c) (this->*funcPix)(x, y, c);
        }

        nxxx -= dsin;
        nyyy += dcos;
    }
}

//! 拡大・回転後の範囲取得
/*!
    @param prc 元矩形範囲を入れておく。結果が入る。
    @return FALSE で描画範囲外
*/

BOOL CLayerImg::getScaleRotRect(AXRect *prc,double dScale,double dAngle)
{
    AXPoint pt[4],ptMin,ptMax;
    int i,cx,cy,ix,iy;
    double xx,yy,dcos,dsin;

    cx = (prc->right - prc->left + 1) / 2 + prc->left;
    cy = (prc->bottom - prc->top + 1) / 2 + prc->top;

    dcos = ::cos(dAngle);
    dsin = ::sin(dAngle);

    //四隅の点を拡大・回転

    pt[0].x = pt[3].x = prc->left;
    pt[0].y = pt[1].y = prc->top;
    pt[1].x = pt[2].x = prc->right + 1;
    pt[2].y = pt[3].y = prc->bottom + 1;

    for(i = 0; i < 4; i++)
    {
        xx = (pt[i].x - cx) * dScale;
        yy = (pt[i].y - cy) * dScale;

        ix = (int)(xx * dcos - yy * dsin) + cx;
        iy = (int)(xx * dsin + yy * dcos) + cy;

        if(i == 0)
        {
            ptMin.x = ptMax.x = ix;
            ptMin.y = ptMax.y = iy;
        }
        else
        {
            if(ix < ptMin.x) ptMin.x = ix;
            if(iy < ptMin.y) ptMin.y = iy;
            if(ix > ptMax.x) ptMax.x = ix;
            if(iy > ptMax.y) ptMax.y = iy;
        }
    }

    //セット＆調整

    prc->set(ptMin.x - 1, ptMin.y - 1, ptMax.x + 1, ptMax.y + 1);

    return clipRectInEnableDraw(prc);
}

//! コピーしてある8bitイメージを元に戻す
/*!
    @param rc   矩形範囲
    @param img8 rc の範囲分のイメージ
*/

void CLayerImg::restoreImageFrom8bit(const AXRect &rc,const CImage8 &img8)
{
    int x,y;
    LPBYTE ps,pd;

    ps = img8.getBuf();

    for(y = rc.top; y <= rc.bottom; y++)
    {
        for(x = rc.left; x <= rc.right; x++, ps++)
        {
            if(*ps)
            {
                pd = getPixelPoint(x, y);
                if(pd) *pd = *ps;
            }
        }
    }
}
