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

#include <stdlib.h>
#include <math.h>

#include "CResizeImage.h"

#include "CProgressDlg.h"

#include "AXMem.h"

//-------------------------

#if defined(__BIG_ENDIAN__)
struct PIXEL32
{
    BYTE a,r,g,b;
};
#else
struct PIXEL32
{
    BYTE b,g,r,a;
};
#endif

//-------------------------


/*!
    @class CResizeImage
    @brief 8bit/32bitイメージ拡大縮小クラス
*/


CResizeImage::CResizeImage()
{
    m_pWeight = NULL;
    m_pIndex  = NULL;
}

CResizeImage::~CResizeImage()
{
    free();
}

//! 解放

void CResizeImage::free()
{
    //重み

    if(m_pWeight)
    {
        ::free(m_pWeight);
        m_pWeight = NULL;
    }

    //インデックス

    if(m_pIndex)
    {
        ::free(m_pIndex);
        m_pIndex = NULL;
    }
}

//! リサイズ実行

BOOL CResizeImage::resize(LPBYTE pSrc,LPBYTE pDst,const AXSize &sizeOld,const AXSize &sizeNew,
                int type,IMGTYPE imgtype,CProgressDlg *pProgDlg)
{
    AXMem memWork;
    DWORD size;

    m_pProgDlg = pProgDlg;
    m_szOld = sizeOld;
    m_szNew = sizeNew;
    m_nType	= type;

    if(m_pProgDlg)
        m_pProgDlg->setProgMax(40);

    //水平リサイズ用メモリ

    size = m_szNew.w * m_szOld.h;
    if(imgtype != IMGTYPE_8BIT) size <<= 2;

    if(!memWork.alloc(size)) return FALSE;

    //------- 水平リサイズ

    if(!_setParam(m_szOld.w, m_szNew.w)) goto ERR;

    if(m_pProgDlg)
        m_pProgDlg->beginProgSub(20, m_szOld.h);

    if(imgtype == IMGTYPE_8BIT)
        _resizeH_8bit(memWork, pSrc);
    else if(imgtype == IMGTYPE_32BIT_RGB)
        _resizeH_32bit(memWork, pSrc);
    else
        _resizeH_32bitA(memWork, pSrc);

    free();

    //------- 垂直リサイズ

    if(!_setParam(m_szOld.h, m_szNew.h)) goto ERR;

    if(m_pProgDlg)
        m_pProgDlg->beginProgSub(20, m_szNew.h);

    if(imgtype == IMGTYPE_8BIT)
        _resizeV_8bit(pDst, memWork);
    else if(imgtype == IMGTYPE_32BIT_RGB)
        _resizeV_32bit(pDst, memWork);
    else
        _resizeV_32bitA(pDst, memWork);

    free();

    return TRUE;

ERR:
    free();
    return FALSE;
}


//=============================
//パラメータ
//=============================


//! パラメータセット

BOOL CResizeImage::_setParam(int srcl,int dstl)
{
    if(dstl < srcl)
        //縮小
        return _setParam_down(srcl, dstl);
    else
        //拡大
        return _setParam_up(srcl, dstl);
}

//! パラメータメモリ確保
/*!
    m_nTap はあらかじめセットしておく。
*/

BOOL CResizeImage::_allocParam(int len)
{
    m_pWeight = (double *)::malloc(sizeof(double) * len * m_nTap);
    if(!m_pWeight) return FALSE;

    m_pIndex = (short *)::malloc(sizeof(short) * len * m_nTap);
    if(!m_pIndex) return FALSE;

    return TRUE;
}

//! 縮小パラメータセット

BOOL CResizeImage::_setParam_down(int srcl,int dstl)
{
    int i,j,pos,range;
    double *pWork,sum,scale;
    double *pWeight;
    short *pIndex;

    range = (m_nType == 0)? 2: 3;

    m_nTap = ((range * 2) * srcl + (dstl - 1)) / dstl;

    //メモリ確保

    if(!_allocParam(dstl)) return FALSE;

    //

    pWork = (double *)::malloc(sizeof(double) * m_nTap);
    if(!pWork) return FALSE;

    //--------------

    pWeight = m_pWeight;
    pIndex  = m_pIndex;
    scale   = (double)dstl / srcl;

    for(i = 0; i < dstl; i++)
    {
        pos = (int)::floor((i - range + 0.5) * srcl / dstl + 0.5);
        sum = 0;

        for(j = 0; j < m_nTap; j++, pIndex++)
        {
            if(pos < 0)
                *pIndex = 0;
            else if(pos >= srcl)
                *pIndex = srcl - 1;
            else
                *pIndex = pos;

            pWork[j] = _getWeight((pos + 0.5) * scale - (i + 0.5));

            sum += pWork[j];
            pos++;
        }

        for(j = 0; j < m_nTap; j++)
            *(pWeight++) = pWork[j] / sum;
    }

    ::free(pWork);

    return TRUE;
}

//! 拡大パラメータセット

BOOL CResizeImage::_setParam_up(int srcl,int dstl)
{
    int i,j,n;
    double *pWork,pos,sum;
    double *pWeight;
    short *pIndex;

    m_nTap = 6;

    //メモリ確保

    if(!_allocParam(dstl)) return FALSE;

    //

    pWork = (double *)::malloc(sizeof(double) * m_nTap);
    if(!pWork) return FALSE;

    //---------

    pWeight = m_pWeight;
    pIndex  = m_pIndex;

    for(i = 0; i < dstl; i++)
    {
        pos = (i + 0.5) * srcl / dstl;
        n   = (int)::floor(pos - 2.5);
        pos = n + 0.5 - pos;
        sum = 0;

        for(j = 0; j < m_nTap; j++, pIndex++)
        {
            if(n < 0)
                *pIndex = 0;
            else if(n >= srcl)
                *pIndex = srcl - 1;
            else
                *pIndex = n;

            pWork[j] = _getWeight(pos);

            sum += pWork[j];
            pos += 1;
            n++;
        }

        for(j = 0; j < m_nTap; j++)
            *(pWeight++) = pWork[j] / sum;
    }

    ::free(pWork);

    return TRUE;
}

//! 重み計算

double CResizeImage::_getWeight(double d)
{
    d = ::fabs(d);

    switch(m_nType)
    {
        //Lanczos2
        case 0:
            if(d < 2.2204460492503131e-016)
                return 1.0;
            else if(d >= 2.0)
                return 0.0;
            else
            {
                d *= M_PI;
                return ::sin(d) * ::sin(d / 2.0) / (d * d / 2.0);
            }
            break;

        //Lanczos3
        default:
            if(d < 2.2204460492503131e-016)
                return 1.0;
            else if(d >= 3.0)
                return 0.0;
            else
            {
                d *= M_PI;
                return ::sin(d) * ::sin(d / 3.0) / (d * d / 3.0);
            }
            break;
    }

    return 0;
}


//=============================
//8bit リサイズ
//=============================


//! 水平リサイズ

void CResizeImage::_resizeH_8bit(LPBYTE pDst,LPBYTE pSrc)
{
    int x,y,i;
    double *pWeight,d;
    short *pIndex;

    for(y = m_szOld.h; y > 0; y--)
    {
        pWeight = m_pWeight;
        pIndex  = m_pIndex;

        for(x = m_szNew.w; x > 0; x--)
        {
            //色

            d = 0;

            for(i = 0; i < m_nTap; i++, pWeight++, pIndex++)
                d += pSrc[*pIndex] * (*pWeight);

            //セット

            i = (int)(d + 0.5);
            if(i < 0) i = 0; else if(i > 255) i = 255;

            *(pDst++) = i;
        }

        pSrc += m_szOld.w;
    }
}

//! 垂直リサイズ

void CResizeImage::_resizeV_8bit(LPBYTE pDst,LPBYTE pSrc)
{
    LPBYTE ps;
    int x,y,i,nw;
    double *pWeight,d;
    short *pIndex;

    nw = m_szNew.w;

    pWeight = m_pWeight;
    pIndex  = m_pIndex;

    for(y = m_szNew.h; y > 0; y--)
    {
        ps = pSrc;

        for(x = nw; x > 0; x--, ps++)
        {
            //色

            d = 0;

            for(i = 0; i < m_nTap; i++)
                d += ps[pIndex[i] * nw] * pWeight[i];

            //セット

            i = (int)(d + 0.5);
            if(i < 0) i = 0; else if(i > 255) i = 255;

            *(pDst++) = i;
        }

        pWeight += m_nTap;
        pIndex  += m_nTap;
    }
}


//=============================
//32bit リサイズ（アルファなし）
//=============================


//! 水平リサイズ

void CResizeImage::_resizeH_32bit(LPBYTE pDst,LPBYTE pSrc)
{
    PIXEL32 *pd,*ps;
    int x,y,i,pitch,n;
    BYTE c[3];
    double *pWeight,w,d[3];
    short *pIndex;

    pd    = (PIXEL32 *)pDst;
    pitch = m_szOld.w << 2;

    for(y = m_szOld.h; y > 0; y--)
    {
        pWeight = m_pWeight;
        pIndex  = m_pIndex;

        for(x = m_szNew.w; x > 0; x--, pd++)
        {
            //色

            d[0] = d[1] = d[2] = 0;

            for(i = 0; i < m_nTap; i++, pWeight++, pIndex++)
            {
                ps = (PIXEL32 *)(pSrc + (*pIndex << 2));
                w  = *pWeight;

                d[0] += ps->r * w;
                d[1] += ps->g * w;
                d[2] += ps->b * w;
            }

            //

            for(i = 0; i < 3; i++)
            {
                n = (int)(d[i] + 0.5);
                if(n < 0) n = 0; else if(n > 255) n = 255;

                c[i] = n;
            }

            pd->r = c[0];
            pd->g = c[1];
            pd->b = c[2];
        }

        pSrc += pitch;

        m_pProgDlg->incProgSub();
    }
}

//! 垂直リサイズ

void CResizeImage::_resizeV_32bit(LPBYTE pDst,LPBYTE pSrc)
{
    PIXEL32 *pd,*ps,*p;
    int x,y,i,pitch,n;
    BYTE c[3];
    double *pWeight,w,d[3];
    short *pIndex;

    pd      = (PIXEL32 *)pDst;
    pitch   = m_szNew.w;
    pWeight = m_pWeight;
    pIndex  = m_pIndex;

    //

    for(y = m_szNew.h; y > 0; y--)
    {
        ps = (PIXEL32 *)pSrc;

        for(x = m_szNew.w; x > 0; x--, pd++, ps++)
        {
            //色

            d[0] = d[1] = d[2] = 0;

            for(i = 0; i < m_nTap; i++)
            {
                p = ps + pIndex[i] * pitch;
                w = pWeight[i];

                d[0] += p->r * w;
                d[1] += p->g * w;
                d[2] += p->b * w;
            }

            //セット

            for(i = 0; i < 3; i++)
            {
                n = (int)(d[i] + 0.5);
                if(n < 0) n = 0; else if(n > 255) n = 255;

                c[i] = n;
            }

            pd->r = c[0];
            pd->g = c[1];
            pd->b = c[2];
        }

        pWeight += m_nTap;
        pIndex  += m_nTap;

        m_pProgDlg->incProgSub();
    }
}


//=============================
//32bit リサイズ（アルファあり）
//=============================


//! 水平リサイズ

void CResizeImage::_resizeH_32bitA(LPBYTE pDst,LPBYTE pSrc)
{
    PIXEL32 *pd,*ps;
    int x,y,i,pitch,n;
    BYTE c[3];
    double *pWeight,w,d[4];
    short *pIndex;

    pd    = (PIXEL32 *)pDst;
    pitch = m_szOld.w << 2;

    for(y = m_szOld.h; y > 0; y--)
    {
        pWeight = m_pWeight;
        pIndex  = m_pIndex;

        for(x = m_szNew.w; x > 0; x--, pd++)
        {
            //色

            d[0] = d[1] = d[2] = d[3] = 0;

            for(i = 0; i < m_nTap; i++, pWeight++, pIndex++)
            {
                ps = (PIXEL32 *)(pSrc + (*pIndex << 2));
                w  = *pWeight;

                d[0] += ps->r * ps->a * w;
                d[1] += ps->g * ps->a * w;
                d[2] += ps->b * ps->a * w;
                d[3] += ps->a * w;
            }

            //A

            n = (int)(d[3] + 0.5);
            if(n < 0) n = 0; else if(n > 255) n = 255;

            pd->a = n;

            //RGB

            if(n == 0)
                pd->r = pd->g = pd->b = 0;
            else
            {
                for(i = 0; i < 3; i++)
                {
                    n = (int)(d[i] / d[3] + 0.5);
                    if(n < 0) n = 0; else if(n > 255) n = 255;

                    c[i] = n;
                }

                pd->r = c[0];
                pd->g = c[1];
                pd->b = c[2];
            }
        }

        pSrc += pitch;

        m_pProgDlg->incProgSub();
    }
}

//! 垂直リサイズ

void CResizeImage::_resizeV_32bitA(LPBYTE pDst,LPBYTE pSrc)
{
    PIXEL32 *pd,*ps,*p;
    int x,y,i,pitch,n;
    BYTE c[3];
    double *pWeight,w,d[4];
    short *pIndex;

    pd      = (PIXEL32 *)pDst;
    pitch   = m_szNew.w;
    pWeight = m_pWeight;
    pIndex  = m_pIndex;

    //

    for(y = m_szNew.h; y > 0; y--)
    {
        ps = (PIXEL32 *)pSrc;

        for(x = m_szNew.w; x > 0; x--, pd++, ps++)
        {
            //色

            d[0] = d[1] = d[2] = d[3] = 0;

            for(i = 0; i < m_nTap; i++)
            {
                p = ps + pIndex[i] * pitch;
                w = pWeight[i];

                d[0] += p->r * p->a * w;
                d[1] += p->g * p->a * w;
                d[2] += p->b * p->a * w;
                d[3] += p->a * w;
            }

            //A

            n = (int)(d[3] + 0.5);
            if(n < 0) n = 0; else if(n > 255) n = 255;

            pd->a = n;

            //RGB

            if(n == 0)
                pd->r = pd->g = pd->b = 0;
            else
            {
                for(i = 0; i < 3; i++)
                {
                    n = (int)(d[i] / d[3] + 0.5);
                    if(n < 0) n = 0; else if(n > 255) n = 255;

                    c[i] = n;
                }

                pd->r = c[0];
                pd->g = c[1];
                pd->b = c[2];
            }
        }

        pWeight += m_nTap;
        pIndex  += m_nTap;

        m_pProgDlg->incProgSub();
    }
}
