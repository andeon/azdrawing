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

#ifndef _AZDRAW_RESIZEIMAGE_H_
#define _AZDRAW_RESIZEIMAGE_H_

#include "AXDef.h"

class CProgressDlg;

class CResizeImage
{
public:
    enum IMGTYPE
    {
        IMGTYPE_8BIT,
        IMGTYPE_32BIT_RGB,
        IMGTYPE_32BIT_RGBA
    };

protected:
    CProgressDlg *m_pProgDlg;
    double  *m_pWeight;
    short   *m_pIndex;
    AXSize  m_szOld,
            m_szNew;
    int	    m_nType,    //0:Lanczos2,1:Lanczos3
            m_nTap;

protected:
    BOOL _allocParam(int len);
    BOOL _setParam(int srcl,int dstl);
    BOOL _setParam_down(int srcl,int dstl);
    BOOL _setParam_up(int srcl,int dstl);
    double _getWeight(double d);

    void _resizeH_8bit(LPBYTE pDst,LPBYTE pSrc);
    void _resizeV_8bit(LPBYTE pDst,LPBYTE pSrc);

    void _resizeH_32bit(LPBYTE pDst,LPBYTE pSrc);
    void _resizeV_32bit(LPBYTE pDst,LPBYTE pSrc);

    void _resizeH_32bitA(LPBYTE pDst,LPBYTE pSrc);
    void _resizeV_32bitA(LPBYTE pDst,LPBYTE pSrc);

public:
    CResizeImage();
    ~CResizeImage();

    void free();
    BOOL resize(LPBYTE pSrc,LPBYTE pDst,const AXSize &sizeOld,const AXSize &sizeNew,int type,IMGTYPE imgtype,CProgressDlg *pProgDlg=NULL);
};

#endif
