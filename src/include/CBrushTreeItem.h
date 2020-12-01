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

#ifndef _AZDRAW_BRUSHTREEITEM_H_
#define _AZDRAW_BRUSHTREEITEM_H_

#include "AXTreeViewItem.h"

class CImage8;

class CBrushTreeItem:public AXTreeViewItem
{
public:
    enum
    {
        BRUSHSIZE_MIN   = 5,
        BRUSHSIZE_MAX   = 6000,
        PRESS_MIN       = 1,
        PRESS_MAX       = 600,
        HOSEISTR_MAX    = 14,

        WORDBASE_NUM    = 11,
        BYTEBASE_NUM    = 7,

        PARAM_TOPITEM   = 0,
        PARAM_FOLDER    = 1,
        PARAM_BRUSH     = 2,

        ICONNO_BRUSH    = 1,
        ICONNO_REGBRUSH_TOP = 2,

        DATTYPE_LINK    = 0,
        DATTYPE_MANUAL  = 1,

        PIXTYPE_BLEND       = 0,
        PIXTYPE_COMPARE     = 1,
        PIXTYPE_OVERWRITE   = 2,
        PIXTYPE_OVERWRITE2  = 3,
        PIXTYPE_ERASE       = 4,
        PIXTYPE_BLUR        = 5,

        BFLAG_ANTIALIAS   = 1,
        BFLAG_CURVE       = 2,
        BFLAG_RANDPOS_BOX = 4
    };

public:
    AXString    m_strBrushImg,  //空:デフォルト
                m_strTexImg;    //空:なし（強制）、"?":オプション指定を使う

    WORD    m_wSize,            //ブラシサイズ (1.0=10)
            m_wSizeCtlMax,      //最大サイズ値
            m_wMinSize,         //最小サイズ(%) (1.0=10)
            m_wMinVal,          //最小濃度(%) (1.0=10)
            m_wInterval,        //間隔 (1.00=100)
            m_wRotAngle,        //回転角度
            m_wRotRandom,       //回転ランダム
            m_wRan_sizeMin,     //ランダム・サイズ最小 (1.0=10)
            m_wRan_posLen,      //ランダム・位置最大距離 (1.00=100)
            m_wPressSize,       //筆圧・サイズ (1.00=100)
            m_wPressVal;        //筆圧・濃度 (1.00=100)
    BYTE    m_btDatType,        //データタイプ ([0]連動保存 [1]手動保存)
            m_btVal,            //濃度 (0-255)
            m_btPixType,        //点の塗りタイプ
            m_btHoseiType,      //補正タイプ
            m_btHoseiStr,       //補正強さ (0[1]〜)
            m_btRotType,        //回転タイプ
            m_btFlag,           //フラグ
            m_bLoadImg;         //イメージが読み込まれたか

    CImage8 *m_pimgBrush,
            *m_pimgTex;

protected:
    void _textformat_str(AXString *pDst,const AXString &str);
    void _pasteText_getStr(AXString *pDst,LPCUSTR pc);
    BOOL _pasteText_win(const AXString &str);

public:
    CBrushTreeItem();

    void setNew();
    void copyFrom(const CBrushTreeItem *p);
    void getDatArray(LPWORD pWord,LPBYTE pByte);
    void setDatArray(LPWORD pWord,LPBYTE pByte);

    void getTextFormat(AXString *pstr);
    BOOL setFromTextFormat(const AXString &strText);
};

#endif
