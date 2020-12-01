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
    [ブラシウィンドウ] 各タブのウィンドウクラス
*/


//! 基底クラス

class CBrushWinTab_base:public AXWindow
{
public:
    CBrushWinTab_base(AXWindow *pParent);

    virtual void setValue(CBrushTreeItem *p) = 0;
};

//! 最小＆間隔

class CBrushWinTab_min:public CBrushWinTab_base
{
    enum
    {
        WID_BAR_SIZEMIN = 100,
        WID_BAR_VALMIN,
        WID_BAR_INTERVAL
    };

    CValBar2    *m_pbarSize,
                *m_pbarVal,
                *m_pbarInter;

public:
    CBrushWinTab_min(AXWindow *pParent);

    virtual void setValue(CBrushTreeItem *p);
    virtual BOOL onNotify(AXWindow *pwin,UINT uNotify,ULONG lParam);
};

//! ランダム

class CBrushWinTab_random:public CBrushWinTab_base
{
    enum
    {
        WID_BAR_RANSIZE = 100,
        WID_BAR_RANPOS,
        WID_CK_RANCIRCLE,
        WID_CK_RANBOX
    };

    CValBar2    *m_pbarSize,
                *m_pbarPos;
    AXCheckButton   *m_pckCir,
                    *m_pckBox;

public:
    CBrushWinTab_random(AXWindow *pParent);

    virtual void setValue(CBrushTreeItem *p);
    virtual BOOL onNotify(AXWindow *pwin,UINT uNotify,ULONG lParam);
};

//! 筆圧

class CBrushWinTab_press:public CBrushWinTab_base
{
    enum
    {
        WID_PRESS_SIZE = 100,
        WID_PRESS_VAL,
        WID_EDIT_SIZE,
        WID_EDIT_VAL
    };

    CPressCurveView *m_pSize,
                    *m_pVal;
    AXLineEdit      *m_peditSize,
                    *m_peditVal;

public:
    CBrushWinTab_press(AXWindow *pParent);

    virtual void setValue(CBrushTreeItem *p);
    virtual BOOL onNotify(AXWindow *pwin,UINT uNotify,ULONG lParam);
};

//! ブラシ形状

class CBrushWinTab_style:public CBrushWinTab_base
{
    enum
    {
        WID_BRUSHIMG = 100,
        WID_CB_ROTTYPE,
        WID_BAR_ANGLE,
        WID_BAR_RANDOM
    };

    CSelImgBar  *m_pselImg;
    AXComboBox  *m_pcbRotType;
    CValBar2    *m_pbarAngle,
                *m_pbarRand;

    void _setSelImg();

public:
    CBrushWinTab_style(AXWindow *pParent);

    virtual void setValue(CBrushTreeItem *p);
    virtual BOOL onNotify(AXWindow *pwin,UINT uNotify,ULONG lParam);
};

//! 他

class CBrushWinTab_etc:public CBrushWinTab_base
{
    enum
    {
        WID_TEXTUREIMG = 100,
        WID_CK_ANTIALIAS,
        WID_CK_CURVE
    };

    CSelImgBar      *m_pselImg;
    AXCheckButton   *m_pckAnti,
                    *m_pckCurve;

    void _setSelImg();

public:
    CBrushWinTab_etc(AXWindow *pParent);

    virtual void setValue(CBrushTreeItem *p);
    virtual BOOL onNotify(AXWindow *pwin,UINT uNotify,ULONG lParam);
};
