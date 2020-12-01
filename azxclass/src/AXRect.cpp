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

#include "AXRect.h"
#include "AXDef.h"


//********************************
// AXRect
//********************************

/*!
    @class AXRect
    @brief int left,top,right,bottom

    @ingroup value
*/

//! 幅取得

int AXRect::width() const
{
    if(left < right)
        return right - left + 1;
    else
        return left - right + 1;
}

//! 高さ取得

int AXRect::height() const
{
    if(top < bottom)
        return bottom - top + 1;
    else
        return top - bottom + 1;
}

//! 全てに同じ値セット

void AXRect::set(int val)
{
    left = top = right = bottom = val;
}

//! AXRectSizeからセット

void AXRect::set(const AXRectSize &rcs)
{
    left   = rcs.x;
    top    = rcs.y;
    right  = rcs.x + rcs.w - 1;
    bottom = rcs.y + rcs.h - 1;
}

//! 位置とサイズからセット

void AXRect::setFromSize(int x,int y,int w,int h)
{
    left = x; top = y;
    right = x + w - 1; bottom = y + h - 1;
}

//! 2点からセット（値の小さいほうが left,top）

void AXRect::setFromPoint(AXPoint &pt1,AXPoint &pt2)
{
    if(pt1.x < pt2.x)
        left = pt1.x, right = pt2.x;
    else
        left = pt2.x, right = pt1.x;

    if(pt1.y < pt2.y)
        top = pt1.y, bottom = pt2.y;
    else
        top = pt2.y, bottom = pt1.y;
}

//! 2点からセット（値の小さいほうが left,top）

void AXRect::setMinMax(int x1,int y1,int x2,int y2)
{
    if(x1 < x2)
        left = x1, right = x2;
    else
        left = x2, right = x1;

    if(y1 < y2)
        top = y1, bottom = y2;
    else
        top = y2, bottom = y1;
}

//! 値の小さい方を left,top。値の大きい方を right,bottom に

void AXRect::minmax()
{
    int n;

    if(left > right)
        n = left, left = right, right = n;

    if(top > bottom)
        n = top, top = bottom, bottom = n;
}

//! 範囲結合
/*!
    @attention left,top が左上の状態であること。
*/

void AXRect::combine(const AXRect &rc)
{
    if(rc.left   < left)   left = rc.left;
    if(rc.top    < top)    top = rc.top;
    if(rc.right  > right)  right = rc.right;
    if(rc.bottom > bottom) bottom = rc.bottom;
}

//! 範囲結合

void AXRect::combine(int x,int y,int w,int h)
{
    if(x < left) left = x;
    if(y < top) top = y;
    if(x + w - 1 > right) right = x + w - 1;
    if(y + h - 1 > bottom) bottom = y + h - 1;
}

//! 指定幅分収縮させる

void AXRect::deflate(const AXRect &rc)
{
    left += rc.left;
    top  += rc.top;
    right  -= rc.right;  if(right < left) right = left;
    bottom -= rc.bottom; if(bottom < top) bottom = top;
}

//! 指定幅分収縮させる

void AXRect::deflate(int width)
{
    left += width;
    top  += width;
    right  -= width; if(right < left) right = left;
    bottom -= width; if(bottom < top) bottom = top;
}

//! 移動

void AXRect::move(int x,int y)
{
    left += x;
    top  += y;
    right  += x;
    bottom += y;
}

//! 範囲に点を含める

void AXRect::incPoint(int x,int y)
{
    if(x < left) left = x;
    if(y < top) top = y;
    if(x > right) right = x;
    if(y > bottom) bottom = y;
}


//********************************
// AXRectSize
//********************************

/*!
    @class AXRectSize
    @brief int x,y,w,h

    @ingroup value
*/


//! AXRectの範囲をセット

void AXRectSize::set(const AXRect &rc)
{
    if(rc.left < rc.right)
        x = rc.left, w = rc.right - rc.left + 1;
    else
        x = rc.right, w = rc.left - rc.right + 1;

    if(rc.top < rc.bottom)
        y = rc.top, h = rc.bottom - rc.top + 1;
    else
        y = rc.bottom, h = rc.top - rc.bottom + 1;
}

//! 2点から範囲セット

void AXRectSize::setFromPoint(int x1,int y1,int x2,int y2)
{
    int n;

    //入れ替え

    if(x1 > x2) n = x1, x1 = x2, x2 = n;
    if(y1 > y2) n = y1, y1 = y2, y2 = n;

    x = x1;
    y = y1;
    w = x2 - x1 + 1;
    h = y2 - y1 + 1;
}

//! 範囲結合

void AXRectSize::combine(const AXRectSize &rcs)
{
    int x2,y2;

    x2 = x + w; if(rcs.x + rcs.w > x2) x2 = rcs.x + rcs.w;
    y2 = y + h; if(rcs.y + rcs.h > y2) y2 = rcs.y + rcs.h;

    if(rcs.x < x) x = rcs.x;
    if(rcs.y < y) y = rcs.y;
    w = x2 - x;
    h = y2 - y;
}

//! 点が範囲内に入っているか

bool AXRectSize::isContain(int xx,int yy) const
{
	return (x <= xx && xx < x + w && y <= yy && yy < y + h);
}

//! 現在のサイズを、指定ボックスサイズ内で縦横比を維持した矩形に調整（拡大縮小）
/*!
    ※元サイズはあらかじめセットしておく。

    @param boxw,boxh    収めるサイズ
    @param bNoScaleUp   拡大はしない（拡大する代わりに余白をつける）
*/

void AXRectSize::inBoxKeepAspect(int boxw,int boxh,bool bNoScaleUp)
{
    int dw,dh;

    if(bNoScaleUp && w <= boxw && h <= boxh)
    {
        //ボックスサイズより小さければ拡大せず余白を付ける

        x = (boxw - w) / 2;
        y = (boxh - h) / 2;
    }
    else
    {
        //縦横比維持で拡大縮小

        dw = (int)((double)boxh * w / h + 0.5);
        dh = (int)((double)boxw * h / w + 0.5);

        if(dw < boxw)
        {
            //縦長

            x = (boxw - dw) / 2;
            y = 0;
            w = dw;
            h = boxh;
        }
        else
        {
            //横長

            x = 0;
            y = (boxh - dh) / 2;
            w = boxw;
            h = dh;
        }
    }
}
