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

#ifndef _AZDRAW_LAYERIMG_H_
#define _AZDRAW_LAYERIMG_H_

#include "AXDef.h"

class CTileImg;
class CImage32;
class CImage8;
class CPolygonPos;
class CSplinePos;
class CSinTable;
class CProgressDlg;
class AXRect;
class AXRectSize;
class AXString;
class AXRand;
class AXImage;
class AXMem;
struct LAYERINFO;
struct DRAWLAYERINFO;
struct RECTANDSIZE;
struct DPOINT;


class CLayerImg
{
public:
    static DRAWLAYERINFO *m_pinfo;
    static CSinTable     *m_pSinTbl;
    static AXRand        *m_pRand;

public:
    enum
    {
        //setPixelDraw 時
        COLSETF_BLEND = 0,
        COLSETF_COMP,
        COLSETF_OVERWRITE,
        COLSETF_OVERWRITE2,
        COLSETF_ERASE,
        COLSETF_BLUR,

        //setPixel_subDraw 時
        COLSETSUB_SET = 0,
        COLSETSUB_CLEAR,
        COLSETSUB_REV,
    };

    enum
    {
        TILEPT_EMPTY = 1
    };

protected:
    CTileImg    **m_ppTile;     //! タイル配列
    CLayerImg   *m_pimgLink;    //! 塗りつぶし判定元などのリンク先

    int     m_nTileXCnt,    //! 配列のタイル数
            m_nTileYCnt,
            m_nOffX,        //! オフセット位置(px)
            m_nOffY,
            m_nTileTopX,    //! タイルの先頭位置
            m_nTileTopY;

protected:
    int _setPixelDraw_getCol(int x,int y,int ns,CTileImg ***pppRetTile,LPINT pTX,LPINT pTY);
    int _setPixelDraw_getBlur(LPBYTE pd,int x,int y);

    void _drawBrush_lineCurve();

    BOOL _filter_sub_getMozaicCol(const CLayerImg &imgSrc,const RECTANDSIZE &rs,int nSize,BOOL bClip,AXMem *pmem,AXSize *pszBuf);

public:
    CLayerImg();
    ~CLayerImg();

    BOOL isExist() const { return (m_ppTile != NULL); }
    int getTileXCnt() const { return m_nTileXCnt; }
    int getTileYCnt() const { return m_nTileYCnt; }
    int getOffX() const { return m_nOffX; }
    int getOffY() const { return m_nOffY; }

    CLayerImg *getLink() const { return m_pimgLink; }
    void setLink(CLayerImg *p) { m_pimgLink = p; }

    CTileImg **getTileBuf() const { return m_ppTile; }
    CTileImg **getTilePt(int tx,int ty) const { return m_ppTile + ty * m_nTileXCnt + tx; }
    CTileImg *getTile(int tx,int ty) const { return *(m_ppTile + ty * m_nTileXCnt + tx); }

    void getOffset(AXPoint *p) { p->x = m_nOffX, p->y = m_nOffY; }

    //

    void free();
    void freeAllTile();
    void freeTile(CTileImg **ppTile);
    void freeTilePos(int tx,int ty);
    void freeEmptyTileFromUndo();
    BOOL freeTileTransparent();

    CTileImg *allocTile();
    CTileImg *allocTileClear();
    BOOL allocTileArray();
    BOOL allocTileArrayFromPx(const AXRect &rc);
    BOOL allocTileArray_incImgAndCanvas(CLayerImg *pSrc);

    BOOL resizeTile(int topx,int topy,int xcnt,int ycnt);
    BOOL resizeTile_incImage();
    BOOL resizeTile_double(const CLayerImg &src);

    BOOL create(int w,int h);
    BOOL copy(const CLayerImg &src);
    BOOL createFromInfo(const LAYERINFO &info);
    BOOL createSame(const CLayerImg &src);

    void setTileFromUndo(int tx,int ty,LPBYTE pSrcBuf,int size);
    DWORD getUndoWriteSize(LPDWORD pCnt);

    void getInfo(LAYERINFO *pd);
    void setOffset(int x,int y,BOOL bAdd);

    BOOL calcPixelToTile(int *pTX,int *pTY,int px,int py) const;
    void calcTileToPixel(int *pPX,int *pPY,int tx,int ty) const;
    void calcTileRectToPixel(AXRect *prc) const;
    BOOL getTileRect_inImage(AXRect *prc,const AXRectSize &rcs);
    BOOL getTileRect_inImageInfo(const AXRectSize &rcs,AXRect *prcTile,AXRect *prcImg,AXPoint *pptTop);
    void getEnableDrawRectPixel(AXRect *prc);
    BOOL clipRectInEnableDraw(AXRect *prc);
    BOOL clipRectInCanvas(RECTANDSIZE *prs);
    int getAllocTileCnt();
    void getExistTileRectPx(AXRect *prc);

    //etc

    void combine(const CLayerImg &src,int alpha,int type=0);
    void blend32Bit(CImage32 *pimgDst,const AXRectSize &rcs,DWORD col,int alpha);
    void blend32Bit_real(CImage32 *pimgDst,const AXRectSize &rcs,DWORD col,int alpha,BOOL bAlpha);

    void convFrom32Bit(const CImage32 &src,BOOL bAlpha);
    void convFrom8Bit(LPBYTE pSrc,int width,int height,BOOL bPadding);
    BOOL convTo8Bit(CImage8 *pimgDst,const AXRectSize &rcs,BOOL bCreate);

    void drawLayerPrev(CImage32 *pimgDst,int srcw,int srch);
    void drawFilterPrev(AXImage *pimgDst,const AXRectSize &rcs) const;

    BOOL savePNG32(const AXString &filename,int w,int h,DWORD col);

    void getHistogram(LPDWORD pBuf);

    //pixel

    BYTE getPixel(int x,int y) const;
    BYTE getPixel(int x,int y,BOOL bClip) const;
    BYTE getPixelClip(int x,int y);
    LPBYTE getPixelPoint(int x,int y);
    LPBYTE getPixelPoint_create(int x,int y);

    void setPixelDraw(int x,int y,BYTE val);

    void setPixel_create(int x,int y,BYTE val);
    void setPixel_create2(int x,int y,BYTE val);
    void setPixel_clear(int x,int y,BYTE val);
    void setPixel_erase(int x,int y,BYTE val);
    void setPixel_calcRect(int x,int y,BYTE val);
    void setPixel_blendCalcRect(int x,int y,BYTE val);
    void setPixel_blendEx(int x,int y,BYTE val);
    void setPixel_subDraw(int x,int y,BYTE val);

    void setPixel_drawfunc(int x,int y,BYTE val);
    void setPixel_dot2px(int x,int y,BYTE val);
    void setPixel_dot3px(int x,int y,BYTE val);

    //draw

    void drawLineB(int x1,int y1,int x2,int y2,BYTE col,BOOL bNoStart);
    void drawLineF(int x1,int y1,int x2,int y2,BYTE val);
    void drawCircle(double cx,double cy,double xr,double yr,double *dParam,BOOL bHRev,BOOL bBrush);
    void drawBezier(double *pos,WORD wHeadTail,BOOL bBrush);
    void drawSpline(CSplinePos *pPos,BOOL bBrush);
    BOOL drawFillPolygon(CPolygonPos *pPos,int val,BOOL bAnti);
    void drawFillCircle(double cx,double cy,double xr,double yr,double *dParam,BOOL bHRev,int val,BOOL bAnti);

    void drawGradient_line(int x1,int y1,int x2,int y2,const AXRect &rc,LPBYTE pTable,int flag);
    void drawGradient_circle(int x1,int y1,int x2,int y2,const AXRect &rc,LPBYTE pTable,int flag);
    void drawGradient_box(int x1,int y1,int x2,int y2,const AXRect &rc,LPBYTE pTable,int flag);

    BOOL drawFillPolygonTmp(CPolygonPos *pPos,int val,void (CLayerImg::*funcPix)(int,int,BYTE));

    //drawBrush

    void drawBrush_free(double x,double y,double press);
    void drawBrush_line(double x1,double y1,double x2,double y2);
    void drawBrush_lineHeadTail(double x1,double y1,double x2,double y2,WORD wHeadTail);
    void drawBrush_point_normal(double xpos,double ypos,double size,int val);
    void drawBrush_point_rotate(double xpos,double ypos,double size,int val);

    //edit

    void reverseHorzFull();
    void reverseHorz(const AXRectSize &rcs);
    void reverseVert(const AXRectSize &rcs);

    void fillBox(const AXRect &rc,BYTE val,void (CLayerImg::*funcPix)(int,int,BYTE));
    void copySelectImage(const CLayerImg &srcImg,const CLayerImg &selImg);
    void cutSelectImage(const CLayerImg &selImg);
    void blendSelImage(const CLayerImg &srcImg,const CLayerImg &selImg,BOOL bOverwrite);
    void inportSel(const CImage8 &img8Sel,const CImage8 &img8Layer,CLayerImg *pimgSel,const AXRectSize &rcs);

    void scaleAndRotate(const CImage8 &img8,const AXRect &rc,double dScale,double dAngle,BOOL bHiQuality,void (CLayerImg::*funcPix)(int,int,BYTE));
    BOOL getScaleRotRect(AXRect *prc,double dScale,double dAngle);
    void restoreImageFrom8bit(const AXRect &rc,const CImage8 &img8);

    //filter

    void filter_col_inverse(const RECTANDSIZE &rs,CProgressDlg *pdlg);
    void filter_setTexture(const RECTANDSIZE &rs,CProgressDlg *pdlg,CImage8 *pTexture);
    BOOL filter_col_gamma(const CLayerImg &imgSrc,RECTANDSIZE &rs,CProgressDlg *pdlg,int nGamma);
    BOOL filter_col_level(const CLayerImg &imgSrc,RECTANDSIZE &rs,CProgressDlg *pdlg,LPINT pVal);
    BOOL filter_2col(const CLayerImg &imgSrc,RECTANDSIZE &rs,CProgressDlg *pdlg,int nBorder);
    BOOL filter_2col_dither(const CLayerImg &imgSrc,RECTANDSIZE &rs,CProgressDlg *pdlg,int nType);

    BOOL filter_layercol(const RECTANDSIZE &rs,CProgressDlg *pdlg,const CLayerImg &imgRef,int nType);
    BOOL filter_mozaic(const CLayerImg &imgSrc,RECTANDSIZE &rs,CProgressDlg *pdlg,int nSize,BOOL bClip);
    BOOL filter_mozaic2(const CLayerImg &imgSrc,RECTANDSIZE &rs,CProgressDlg *pdlg,int nSize,BOOL bClip);
    BOOL filter_crystal(const CLayerImg &imgSrc,RECTANDSIZE &rs,CProgressDlg *pdlg,int nSize,BOOL bAdv,BOOL bClip);
    BOOL filter_noise(const CLayerImg &imgSrc,RECTANDSIZE &rs,CProgressDlg *pdlg,int nDensity,int nSize,BOOL bGray);
    void filter_thinning(const RECTANDSIZE &rs,CProgressDlg *pdlg);

    BOOL filter_blur(const CLayerImg &imgSrc,RECTANDSIZE &rs,CProgressDlg *pdlg,int nRange,BOOL bClip);
    BOOL filter_gaussBlur(const CLayerImg &imgSrc,RECTANDSIZE &rs,CProgressDlg *pdlg,int nStrong,BOOL bClip);
    BOOL filter_motionBlur(const CLayerImg &imgSrc,RECTANDSIZE &rs,CProgressDlg *pdlg,int nStrong,int nAngle,BOOL bClip);

    BOOL filter_drawAmiTone(RECTANDSIZE &rs,CProgressDlg *pdlg,int nLineCnt,int nPers,int nAngle,int nDPI,BOOL bAnti);
    BOOL filter_drawDotLine(RECTANDSIZE &rs,CProgressDlg *pdlg,int minWidth,int maxWidth,int minSpace,int maxSpace,UINT uFlags);
    BOOL filter_drawCheck(RECTANDSIZE &rs,CProgressDlg *pdlg,int colW,int rowW,BOOL bSame);

    BOOL filter_edge(const CLayerImg &imgSrc,RECTANDSIZE &rs,CProgressDlg *pdlg,const CLayerImg &imgRef,int nSize,BOOL bCut);
    BOOL filter_halftone(const CLayerImg &imgSrc,RECTANDSIZE &rs,CProgressDlg *pdlg,int nSize,int nAngle,BOOL bAnti);

    BOOL filter_drawFlash(CProgressDlg *pdlg,int type,AXPoint &ptPos,const int *pVal,int nDrawType,BOOL bSample);
    BOOL filter_drawUniFlashWave(CProgressDlg *pdlg,AXPoint &ptPos,const int *pVal,int nDrawType,BOOL bSample);

    //--------

    static void getCanvasImgSize(int *pW,int *pH);
    static void init();
    static void setCurvePos(double x,double y,double press);
};

#endif
