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

#ifndef _AZDRAW_IMAGE32_H_
#define _AZDRAW_IMAGE32_H_

#include "AXImage32.h"
#include "AXRect.h"

class AXString;
class AXImage;
class AXMem;

class CImage32:public AXImage32
{
public:
    typedef struct
    {
        int     nDPI;
        BOOL    bAlpha;
        char    type;  //B,P,G,J
    }LOADINFO;

    typedef struct
    {
        AXRectSize  rcsDst;
        int         nBaseX,
                    nBaseY,
                    nScrollX,
                    nScrollY;
        BOOL        bHRev;
        DWORD       dwExCol;
        double      dScaleDiv,
                    dScale,
                    dCos,
                    dSin;

        int     nGridW,
                nGridH,
                nGridSplitW,
                nGridSplitH;
        DWORD   dwGridCol,
                dwGridSplitCol,
                dwGridFlags;
    }CANVASDRAW;

public:
    void drawCanvasNormal(AXImage *pimgDst,CANVASDRAW *pinfo);
    void drawCanvasScaleDown(AXImage *pimgDst,CANVASDRAW *pinfo);
    void drawCanvasRotNormal(AXImage *pimgDst,CANVASDRAW *pinfo);
    void drawCanvasRotHiQuality(AXImage *pimgDst,CANVASDRAW *pinfo);
    void drawCanvasGrid(AXImage *pimgDst,CANVASDRAW *pinfo,int gridw,int gridh,DWORD dwGridCol);
    void drawCanvasRotNormalGrid(AXImage *pimgDst,CANVASDRAW *pinfo);
    void drawCanvasRotHiQualityGrid(AXImage *pimgDst,CANVASDRAW *pinfo);

    //file

    BOOL loadImage(const AXString &filename,CImage32::LOADINFO *pInfo);
    BOOL loadBMP(const AXString &filename,CImage32::LOADINFO *pInfo);
    BOOL loadPNG(const AXString &filename,CImage32::LOADINFO *pInfo);
    BOOL loadGIF(const AXString &filename,CImage32::LOADINFO *pInfo);
    BOOL loadJPEG(const AXString &filename,CImage32::LOADINFO *pInfo);

    BOOL createCursor(const AXString &filename,AXMem *pmem);

};

#endif
