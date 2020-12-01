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

#ifndef _AX_RECT_H
#define _AX_RECT_H

class AXRectSize;
class AXPoint;

class AXRect
{
public:
    int left,top,right,bottom;

    int width() const;
    int height() const;

    void set(int val);
    void set(const AXRectSize &rcs);
    void set(int l,int t,int r,int b)
    {
        left = l; top = t; right = r; bottom = b;
    }
    void setFromSize(int x,int y,int w,int h);
    void setFromPoint(AXPoint &pt1,AXPoint &pt2);
    void setMinMax(int x1,int y1,int x2,int y2);
    void minmax();
    void combine(const AXRect &rc);
    void combine(int x,int y,int w,int h);
    void deflate(const AXRect &rc);
    void deflate(int width);
    void move(int x,int y);
    void incPoint(int x,int y);
};

class AXRectSize
{
public:
    int x,y,w,h;

    void set(int xx,int yy,int ww,int hh)
    {
        x = xx; y = yy; w = ww; h = hh;
    }
    void set(const AXRect &rc);
    void setFromPoint(int x1,int y1,int x2,int y2);
    void combine(const AXRectSize &rcs);
    bool isContain(int xx,int yy) const;
    void inBoxKeepAspect(int boxw,int boxh,bool bNoScaleUp);
};

#endif
