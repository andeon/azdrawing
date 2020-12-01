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
    オプションウィンドウの各タブ内容
*/



//! ツール（ドットペン）

class COptWin_tool_dot:public AXWindow
{
public:
    enum
    {
        WID_1PX = 100,
        WID_2PX,
        WID_3PX,
        WID_THIN
    };

    COptWin_tool_dot();
    virtual BOOL onNotify(AXWindow *pwin,UINT uNotify,ULONG lParam);
};

//! ツール（図形塗りつぶし）

class COptWin_tool_polygon:public AXWindow
{
    UINT        *m_pOptVal;
    CValBar2    *m_pbarVal;
    AXComboBox  *m_pcbPixel;

    enum
    {
        WID_VAL = 100,
        WID_PIXEL,
        WID_BRUSHVAL,
        WID_ANTI
    };

public:
    COptWin_tool_polygon();
    virtual BOOL onNotify(AXWindow *pwin,UINT uNotify,ULONG lParam);
};

//! ツール（塗りつぶし）

class COptWin_tool_paint:public AXWindow
{
    CValBar2    *m_pbarVal,
                *m_pbarValCmp;
    AXComboBox  *m_pcbType,
                *m_pcbPixel;

    enum
    {
        WID_VAL = 100,
        WID_TYPE,
        WID_VALCMP,
        WID_PIXEL,
        WID_USEBRUSHVAL,
        WID_NOREF
    };

public:
    COptWin_tool_paint();
    virtual BOOL onNotify(AXWindow *pwin,UINT uNotify,ULONG lParam);
};

//! ツール（閉領域）

class COptWin_tool_paintclose:public AXWindow
{
    CValBar2    *m_pbarVal;
    AXComboBox  *m_pcbPixel;

    enum
    {
        WID_VAL = 100,
        WID_PIXEL,
        WID_USEBRUSHVAL,
        WID_NOREF
    };

public:
    COptWin_tool_paintclose();
    virtual BOOL onNotify(AXWindow *pwin,UINT uNotify,ULONG lParam);
};

//! ツール（不透明消し）

class COptWin_tool_painterase:public AXWindow
{
    enum { WID_NOREF = 100 };

public:
    COptWin_tool_painterase();
    virtual BOOL onNotify(AXWindow *pwin,UINT uNotify,ULONG lParam);
};

//! ツール（移動）

class COptWin_tool_move:public AXWindow
{
    enum
    {
        WID_TYPE1 = 100,
        WID_TYPE2,
        WID_TYPE3
    };

public:
    COptWin_tool_move();
    virtual BOOL onNotify(AXWindow *pwin,UINT uNotify,ULONG lParam);
};

//! グラデーション

class COptWin_tool_grad:public AXWindow
{
    enum
    {
        WID_CB_SEL = 100,
        WID_BTT_LISTADD,
        WID_BTT_LISTDEL,
        WID_BTT_LISTEDIT,
        WID_BAR_VAL,
        WID_CB_PIX,
        WID_CK_REVERSE,
        WID_CK_LOOP
    };

    AXComboBox  *m_pcbSel,
                *m_pcbPix;
    CValBar2    *m_pbarVal;
    CImgPrev    *m_pPrev;

    void _changeSel();

public:
    COptWin_tool_grad();
    virtual BOOL onNotify(AXWindow *pwin,UINT uNotify,ULONG lParam);
};

//! 選択範囲

class COptWin_tool_sel:public AXWindow
{
    enum
    {
        WID_BT_COL = 100,
        WID_BAR_OPACITY
    };

    AXColorButton   *m_pbtCol;
    CValBar2        *m_pbarOc;

public:
    COptWin_tool_sel();
    virtual BOOL onNotify(AXWindow *pwin,UINT uNotify,ULONG lParam);
};


//-------------------


//! 定規

class COptWin_rule_type:public AXWindow
{
    AXImage   m_img;

    void setImage();

public:
    COptWin_rule_type(AXWindow *pParent);
    virtual BOOL onPaint(AXHD_PAINT *phd);
    virtual BOOL onButtonDown(AXHD_MOUSE *phd);
};

class COptWin_rule:public AXWindow
{
    enum
    {
        WID_BTT_ANGLE = 100,
        WID_BTT_POS
    };

    COptWin_rule_type   *m_pwidType;

	void _enableBtt();
    void menuAngle();
    void menuPos();

public:
    COptWin_rule();
    virtual BOOL onNotify(AXWindow *pwin,UINT uNotify,ULONG lParam);
};

//! テクスチャ

class COptWin_texture:public AXWindow
{
    CSelImgBar  *m_pSel;
    CImgPrev    *m_pPrev;

    void _changeFile(BOOL bLoad);

public:
    COptWin_texture();
    virtual BOOL onNotify(AXWindow *pwin,UINT uNotify,ULONG lParam);
};

//! 入り抜き

class COptWin_headtail:public AXWindow
{
    enum
    {
        WID_CK_LINE = 100,
        WID_CK_BEZIER,
        WID_EDIT_HEAD,
        WID_EDIT_TAIL,
        WID_BT_SET,
        WID_LIST,
    };

    AXListBox   *m_pList;
    AXLineEdit  *m_peditHead,
                *m_peditTail;

    void _setVal(WORD val);
    void _setListVal();

public:
    COptWin_headtail();
    virtual BOOL onNotify(AXWindow *pwin,UINT uNotify,ULONG lParam);
};

//! 設定

class COptWin_option:public AXWindow
{
    enum
    {
        WID_CB_IMGBK = 100,
        WID_BTT_IMGBK
    };

    AXComboBox  *m_pcbImgBk;

public:
    COptWin_option();
    virtual BOOL onNotify(AXWindow *pwin,UINT uNotify,ULONG lParam);
    virtual BOOL onOwnerDraw(AXDrawable *pdraw,AXHD_OWNERDRAW *phd);
};
