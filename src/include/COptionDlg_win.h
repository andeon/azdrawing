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
    オプションダイアログ、各ウィンドウ
    (COptionDlg.cpp)
*/


class COptDlg_base:public AXWindow
{
public:
    COptDlg_base(AXWindow *pParent);

    virtual void getDat(COptionDlg::OPTIONDAT *p) = 0;
};

//! 設定1

class COptDlg_opt1:public COptDlg_base
{
protected:
    AXLineEdit  *m_peditInitW,
                *m_peditInitH,
                *m_peditUndo;
    AXColorButton   *m_pbtCanvasCol;
    AXComboBox	*m_pcbWinType;

    AXString    m_strSubWinFont;

    enum
    {
        WID_BTT_SUBWINFONT = 100
    };

public:
    COptDlg_opt1(AXWindow *pParent,COptionDlg::OPTIONDAT *p);

    virtual void getDat(COptionDlg::OPTIONDAT *p);
    virtual BOOL onNotify(AXWindow *pwin,UINT uNotify,ULONG lParam);
};

//! 設定2

class COptDlg_opt2:public COptDlg_base
{
protected:
    AXCheckButton   *m_pCheck[5];

public:
    COptDlg_opt2(AXWindow *pParent,COptionDlg::OPTIONDAT *p);

    virtual void getDat(COptionDlg::OPTIONDAT *p);
};

//! 増減幅

class COptDlg_optstep:public COptDlg_base
{
protected:
    AXLineEdit  *m_pedit[4];

public:
    COptDlg_optstep(AXWindow *pParent,COptionDlg::OPTIONDAT *p);

    virtual void getDat(COptionDlg::OPTIONDAT *p);
};

//! ディレクトリ

class COptDlg_optdir:public COptDlg_base
{
protected:
    AXLineEdit  *m_pedit[2];

    enum
    {
        WID_BUTTON = 100
    };

public:
    COptDlg_optdir(AXWindow *pParent,COptionDlg::OPTIONDAT *p);

    virtual void getDat(COptionDlg::OPTIONDAT *p);
    virtual BOOL onNotify(AXWindow *pwin,UINT uNotify,ULONG lParam);
};

//! 描画用カーソル

class COptDlg_optcursor:public COptDlg_base
{
protected:
    AXLineEdit  *m_peditX,
                *m_peditY;
    CImgPrev    *m_pPrev;

    AXMem   m_memDrawCur;

    enum
    {
        WID_BTT_LOADIMG = 100,
        WID_BTT_DEFAULT
    };

    void _drawPrev();
    void _loadImg();

public:
    COptDlg_optcursor(AXWindow *pParent,COptionDlg::OPTIONDAT *p);

    virtual void getDat(COptionDlg::OPTIONDAT *p);
    virtual BOOL onNotify(AXWindow *pwin,UINT uNotify,ULONG lParam);
};
