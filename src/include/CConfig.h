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

#ifndef _AZDRAW_CONFIG_H_
#define _AZDRAW_CONFIG_H_

#include "AXRect.h"
#include "AXString.h"
#include "AXMem.h"

class AXConfRead;
class AXConfWrite;
class CDevList;
class CKeyDat;
class CFilterConfigList;


class CConfig
{
public:
    enum
    {
        RECFILE_NUM     = 10,
        WININITVAL      = -10000,
        GRIDLIST_NUM    = 5,
        NEWREG_NUM      = 8,
        NEWRECENT_NUM   = 6,
        LAYERPAL_NUM    = 24,
        RULEANGLE_NUM   = 6,
        RULEPOS_NUM     = 6,
        IMGBKCOL_NUM    = 5,
        HEADTAIL_NUM    = 10
    };

    enum FLAGS
    {
        FLAG_MES_SAVE_OVERWRITE = 1<<0,
        FLAG_MES_SAVE_ADW       = 1<<1,
        FLAG_UNDO_PUTFILE       = 1<<2,
        FLAG_AUTOACTIVE         = 1<<3,
        FLAG_OPENPNG_NO_ALPHA   = 1<<4
    };

    enum VIEWFLAGS
    {
        VIEWFLAG_TOOLWIN      = 0x0001,
        VIEWFLAG_LAYERWIN     = 0x0002,
        VIEWFLAG_OPTIONWIN    = 0x0004,
        VIEWFLAG_BRUSHWIN     = 0x0008,
        VIEWFLAG_PREVWIN      = 0x0010,
        VIEWFLAG_IMGVIEWWIN   = 0x0020,
        VIEWFLAG_CANVASSCROLL = 0x0100,
        VIEWFLAG_GRID         = 0x0200,
        VIEWFLAG_GRIDSPLIT    = 0x0400,
        VIEWFLAG_ALLHIDE      = 0x0800
    };

    enum ETCFLAGS
    {
        ETCFLAG_FILTER_PREVIEW  = 0x0001
    };

    enum PREVWINFLAGS
    {
        PREVF_FULLVIEW      = 1,
        PREVF_LEFT_CANVAS   = 2,
        PREVF_RIGHT_CANVAS  = 4,
        PREVF_MODE_LOUPE    = 8
    };

    enum IMGVIEWFLAGS
    {
        IMGVF_FITSIZE   = 1,
        IMGVF_FULLVIEW  = 2
    };
    
    enum TOOLBOXTYPE
    {
		TOOLBOXTYPE_UTILITY,
		TOOLBOXTYPE_NORMAL
    };

public:
    AXRectSize  rcsMainWin,
                rcsOptWin,
                rcsBrushWin,
                rcsLayerWin,
                rcsPrevWin,
                rcsImgViewWin,
                rcsTextWin;
    AXPoint     ptToolWin;
    BOOL        bMaximized;

    UINT        uFlags,
                uViewFlags,
                uEtcFlags,
                uViewBkFlags,
                uPrevWinFlags,
                uImgVFlags,
                uDrawShapeVal;  //!< [0-7bit]濃度 [8bit]アンチエイリアス [9bit]cm [10-13bit]図形 [14-17bit]サブタイプ
    int         nBrushWinH[2],
                nInitImgW,
                nInitImgH,
                nUndoCnt,
                nPrevWinScale,      //1.0=100
                nPrevWinLoupeScale,
                nScaleWidthUp,
                nScaleWidthDown,
                nRotateWidth,
                nGridW,
                nGridH,
                nGridSplitX,
                nGridSplitY;
    DWORD       dwCanvasBkCol,
                dwImgBkCol,
                dwGridCol,
                dwGridSplitCol,
                dwImgBkColList[IMGBKCOL_NUM],   //!< イメージ背景色リスト
                dwGridList[GRIDLIST_NUM],       //!< グリッド登録リスト（上位bit:幅、下位bit:高さ）
                dwNewRegSize[NEWREG_NUM],       //!< 新規作成・登録リスト（上位bit:幅、下位bit:高さ）
                dwNewRecentSize[NEWRECENT_NUM], //!< 新規作成・履歴（px。上位bit:幅、下位bit:高さ）
                dwLayerPal[LAYERPAL_NUM];       //!< レイヤのカラーパレット
    WORD        wNewRegDPI[NEWREG_NUM],         //!< 新規作成・登録リストのDPI
                wNewRecentDPI[NEWRECENT_NUM],   //!< 新規作成・履歴のDPI
                wRuleAngle[RULEANGLE_NUM],      //!< 定規角度・登録
                wHeadTailList[HEADTAIL_NUM],    //!< 入り抜きリスト
                wJPEGSamp;
    short       sRulePos[RULEPOS_NUM * 2],      //!< 定規中心位置・登録
                sDragBrushSizeW;                //!< ドラッグでのブラシサイズ変更時の変更幅(1pxあたり)
    BYTE        btOptWinTab,
                btImgBkColSel,
                btBrushWinTab,
                btOptHeadTail,
                btScaleType,
                btPNGLevel,
                btJPEGQua,
                btExportFormat,
                btToolboxType;

    AXString    strOpenDir,
                strSaveDir,
                strExportDir,
                strImgViewDir,
                strRecFile[RECFILE_NUM],
                strTextureDir,
                strBrushDir,
                strSubWinFont;

    CDevList    *pDev;
    CKeyDat     *pKey;
    CFilterConfigList   *pFilterConf;

    AXMem       memDrawCursor;  //!< 描画カーソルデータ
    AXMemAuto   memBrushSize;   //!< ブラシサイズ・濃度リスト

public:
    ~CConfig();
    CConfig();

    void load(AXConfRead *pcf);
    void save(AXConfWrite *pcf);

    void addNewRecent(int w,int h,int dpi);

    int getBrushSizeCnt();

    BOOL isViewToolWin() { return uViewFlags & VIEWFLAG_TOOLWIN; }
    BOOL isViewBrushWin() { return uViewFlags & VIEWFLAG_BRUSHWIN; }
    BOOL isViewLayerWin() { return uViewFlags & VIEWFLAG_LAYERWIN; }
    BOOL isViewOptWin() { return uViewFlags & VIEWFLAG_OPTIONWIN; }
    BOOL isViewPrevWin() { return uViewFlags & VIEWFLAG_PREVWIN; }
    BOOL isViewImgViewWin() { return uViewFlags & VIEWFLAG_IMGVIEWWIN; }

    BOOL isCanvasScroll() { return uViewFlags & VIEWFLAG_CANVASSCROLL; }
    BOOL isGridView() { return uViewFlags & (VIEWFLAG_GRID | VIEWFLAG_GRIDSPLIT); }
    BOOL isGrid() { return uViewFlags & VIEWFLAG_GRID; }
    BOOL isGridSplit() { return uViewFlags & VIEWFLAG_GRIDSPLIT; }

    BOOL isPrevWinLoupe() { return uPrevWinFlags & PREVF_MODE_LOUPE; }
};

#endif
