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

#ifndef _AZDRAW_DEFSTRUCT_H_
#define _AZDRAW_DEFSTRUCT_H_

#include "AXRect.h"

class CLayerImg;
class CImage8;


struct DPOINT
{
    double x,y;

    void zero() { x = y = 0; }
};

struct DRECT
{
    double x1,y1,x2,y2;
};

struct FLAGRECT
{
    int x1,y1,x2,y2,flag;

    void set(int _x1,int _y1,int _x2,int _y2) { x1 = _x1, y1 = _y1, x2 = _x2, y2 = _y2, flag = TRUE; }
    void toRect(AXRect *prc) { prc->left = x1, prc->top = y1, prc->right = x2, prc->bottom = y2; }
    void fromRect(const AXRect &rc){ x1 = rc.left, y1 = rc.top, x2 = rc.right, y2 = rc.bottom, flag = TRUE; }
};

struct RECTANDSIZE
{
    int x1,y1,x2,y2,w,h;

    void set(const AXRect &rc)
    {
        x1 = rc.left, y1 = rc.top, x2 = rc.right, y2 = rc.bottom;
        w = rc.right - rc.left + 1, h = rc.bottom - rc.top + 1;
    }
    void inflate(int n)
    {
        x1 -= n; y1 -= n; x2 += n; y2 += n;
        w += n * 2; h += n * 2;
    }
    void toRect(AXRect *prc)
    {
        prc->left = x1, prc->top = y1, prc->right = x2, prc->bottom = y2;
    }
};

//! レイヤ情報

struct LAYERINFO
{
    int nTileXCnt,nTileYCnt,
        nOffX,nOffY,
        nTileTopX,nTileTopY;
};

//! ブラシ描画用パラメータ

struct BRUSHDRAWPARAM
{
    double  dMaxSize,       //!< 最大サイズ (px)
            dMaxVal,        //!< 最大濃度
            dSizeMin,       //!< サイズ最小
            dValMin,        //!< 濃度最小
            dInterval,      //!< 間隔
            dGammaSize,     //!< 筆圧ガンマ値
            dGammaVal,
            dRandSizeMin,   //!< ランダムサイズ最小
            dRandPosLen;    //!< ランダム位置距離
    int     nRotAngle,      //!< 回転角度(360度)
            nRotRandom,     //!< 回転ランダム
            nRotAngleRes,   //!< 最終的な回転角度（作業用）
            nHoseiType,     //!< 手ぶれ補正 タイプ
            nHoseiStr;      //!< 手ぶれ補正 過去の参照点の数
    BYTE    bAnti,          //!< アンチエイリアス
            bCurve,         //!< 曲線補間
            btRandFlag,     //!< ランダムフラグ（[0bit]サイズ [1bit]位置 [2bit]位置・矩形）
            btRotFlag,      //!< 回転フラグ([0bit]進行方向 [1bit]ランダム)
            btPressFlag;    //!< 筆圧処理するか（[0bit] サイズ [1bit]濃度）

    double  dPressSt,       //!< 開始点の筆圧
            dPressEd,       //!< 終了点の筆圧
            dT,
            dBkX, dBkY,     //!< 前回の位置
            dBkPress;       //!< 前回の筆圧
};

//! レイヤ描画用

struct DRAWLAYERINFO
{
    int     nImgW,nImgH,    //!< イメージサイズ
            nColSetFunc;    //!< 色演算番号

    void (CLayerImg::*funcDrawPixel)(int,int,BYTE); //!< 点描画関数

    BRUSHDRAWPARAM  bparam;     //!< ブラシパラメータ

    CImage8     *pTexture,      //!< テクスチャ（NULLでなし）
                *pBrush;        //!< ブラシ画像

    CLayerImg   *pimgMask,      //!< レイヤマスク(NULLでなし)
                *pimgSel,       //!< 選択イメージ（NULLでなし）
                *pimgUndo;      //!< アンドゥ用イメージ

    FLAGRECT    rcfDraw;        //!< 描画されたイメージ範囲 (各値はマイナスの場合もあり)
    int         fUndoErr;       //!< アンドゥ時のエラー（0:成功,0bit:イメージタイル確保失敗,1bit:アンドゥ確保失敗）
};

#endif
