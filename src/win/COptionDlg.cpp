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

#include "COptionDlg.h"

#include "CConfig.h"
#include "CDraw.h"

#include "CImage32.h"
#include "CImgPrev.h"

#include "AXLayout.h"
#include "AXSpacerItem.h"
#include "AXLabel.h"
#include "AXListBox.h"
#include "AXLineEdit.h"
#include "AXButton.h"
#include "AXComboBox.h"
#include "AXCheckButton.h"
#include "AXColorButton.h"
#include "AXFileDialog.h"
#include "AXFontDialog.h"
#include "AXApp.h"

#include "defGlobal.h"
#include "defStrID.h"

#include "COptionDlg_win.h"

//-----------------------

#define WID_LB_MENU     100

//-----------------------

/*!
    @class COptionDlg
    @brief 環境設定ダイアログ
*/


COptionDlg::COptionDlg(AXWindow *pOwner)
    : AXDialog(pOwner, WS_DIALOG_NORMAL | WS_BK_FACE)
{
    AXLayout *plTop;

    m_pwin = NULL;

    //

    _trgroup(STRGID_DLG_OPTION);

    setTitle(STRID_OPTDLG_TITLE);

    //

    setLayout(plTop = new AXLayoutVert(0, 15));
    plTop->setSpacing(10);

    //

    plTop->addItem(m_playout = new AXLayoutHorz(LF_EXPAND_WH, 10));

    m_playout->addItem(m_plbMenu = new AXListBox(this, AXScrollView::SVS_SUNKEN, LF_EXPAND_H, WID_LB_MENU, 0));

    //メニュー

    m_plbMenu->addItemTrMul(1, 5);
    m_plbMenu->setAutoWidth(TRUE);
    m_plbMenu->setAutoHeight();
    m_plbMenu->setCurSel(0);

    //ボタン

    plTop->addItem(createOKCancelButton());

    //

    _getConf();

    _setWidget(TRUE);

    //

    calcDefSize();
    resize((m_nDefW < 300)? 300: m_nDefW, m_nDefH);

    show();
}

//! 通知

BOOL COptionDlg::onNotify(AXWindow *pwin,UINT uNotify,ULONG lParam)
{
    BOOL ret;

    switch(pwin->getItemID())
    {
        //メニューリスト
        case WID_LB_MENU:
            if(uNotify == AXListBox::LBN_SELCHANGE)
                _setWidget(FALSE);
            break;

        //OK
        case 1:
            if(m_pwin) m_pwin->getDat(&m_dat);

            ret = _setConf();

            endDialog((ret)? 2: 1);
            break;
        //キャンセル
        case 2:
            endDialog(FALSE);
            break;
    }

    return TRUE;
}

//! 設定データからコピー

void COptionDlg::_getConf()
{
    m_dat.nInitW        = g_pconf->nInitImgW;
    m_dat.nInitH        = g_pconf->nInitImgH;
    m_dat.nUndoCnt      = g_pconf->nUndoCnt;
    m_dat.dwCanvasCol   = g_pconf->dwCanvasBkCol;
    m_dat.strSubWinFont = g_pconf->strSubWinFont;
    m_dat.nToolboxType	= g_pconf->btToolboxType;

    m_dat.uFlags        = g_pconf->uFlags;

    m_dat.nStepScaleD   = g_pconf->nScaleWidthDown;
    m_dat.nStepScaleU   = g_pconf->nScaleWidthUp;
    m_dat.nStepRotate   = g_pconf->nRotateWidth;
    m_dat.nStepBrushSize = g_pconf->sDragBrushSizeW;

    m_dat.strTextureDir = g_pconf->strTextureDir;
    m_dat.strBrushDir   = g_pconf->strBrushDir;

    m_dat.memDrawCursor.copy(g_pconf->memDrawCursor);
}

//! 設定データへセット
/*!
    @return キャンバスを更新するか
*/

BOOL COptionDlg::_setConf()
{
    BOOL bUpdate;

    //設定1

    bUpdate = (g_pconf->dwCanvasBkCol != m_dat.dwCanvasCol);

    g_pconf->nInitImgW      = m_dat.nInitW;
    g_pconf->nInitImgH      = m_dat.nInitH;
    g_pconf->nUndoCnt       = m_dat.nUndoCnt;
    g_pconf->dwCanvasBkCol  = m_dat.dwCanvasCol;
    g_pconf->strSubWinFont  = m_dat.strSubWinFont;
    g_pconf->btToolboxType	= m_dat.nToolboxType;

    g_pdraw->setUndoMaxCnt(m_dat.nUndoCnt);

    //設定2

    g_pconf->uFlags = m_dat.uFlags;

    //増減幅

    g_pconf->nScaleWidthDown    = m_dat.nStepScaleD;
    g_pconf->nScaleWidthUp      = m_dat.nStepScaleU;
    g_pconf->nRotateWidth       = m_dat.nStepRotate;
    g_pconf->sDragBrushSizeW    = m_dat.nStepBrushSize;

    //ディレクトリ

    g_pconf->strTextureDir  = m_dat.strTextureDir;
    g_pconf->strBrushDir    = m_dat.strBrushDir;

    //カーソル

    g_pconf->memDrawCursor.copy(m_dat.memDrawCursor);

    g_pdraw->setCursorTool();

    return bUpdate;
}

//! 設定項目ウィンドウセット

void COptionDlg::_setWidget(BOOL bInit)
{
    //現在のウィンドウ削除

    if(m_pwin)
    {
        m_pwin->getDat(&m_dat);

        m_pwin->removeFromLayout();
        delete m_pwin;

        m_pwin = NULL;
    }

    //作成

    switch(m_plbMenu->getCurSel())
    {
        case 0:
            m_pwin = new COptDlg_opt1(this, &m_dat);
            break;
        case 1:
            m_pwin = new COptDlg_opt2(this, &m_dat);
            break;
        case 2:
            m_pwin = new COptDlg_optstep(this, &m_dat);
            break;
        case 3:
            m_pwin = new COptDlg_optdir(this, &m_dat);
            break;
        case 4:
            m_pwin = new COptDlg_optcursor(this, &m_dat);
            break;
   }

    //セット

    if(m_pwin)
    {
        m_playout->addItem(m_pwin);

        if(!bInit)
        {
            //再レイアウト

            calcDefSize();

            if(m_nDefW > m_nW || m_nDefH > m_nH)
                resize((m_nDefW > m_nW)? m_nDefW: m_nW, (m_nDefH > m_nH)? m_nDefH: m_nH);
            else
                layout();
        }
    }
}



//*********************************
// COptDlg_base
//*********************************


COptDlg_base::COptDlg_base(AXWindow *pParent)
    : AXWindow(pParent, WS_BK_FACE, LF_EXPAND_WH)
{

}


//*********************************
// COptDlg_opt1 : 設定1
//*********************************


COptDlg_opt1::COptDlg_opt1(AXWindow *pParent,COptionDlg::OPTIONDAT *p)
    : COptDlg_base(pParent)
{
    AXLayout *plTop,*pl;

    _trgroup(STRGID_DLG_OPTION);

    setLayout(plTop = new AXLayoutVert(0, 12));

    //--------

    plTop->addItem(pl = new AXLayoutMatrix(2));

    //起動時のイメージ幅

    pl->addItem(new AXLabel(this, 0, LF_CENTER_Y, MAKE_DW4(0,0,6,6), STRID_OPTDLG_INITW));
    pl->addItem(m_peditInitW = new AXLineEdit(this, AXLineEdit::ES_SPIN, 0, 0, MAKE_DW4(0,0,0,6)));

    m_peditInitW->setInit(6, 1, CDraw::IMGSIZE_MAX, p->nInitW);

    //起動時のイメージ高さ

    pl->addItem(new AXLabel(this, 0, LF_CENTER_Y, MAKE_DW4(0,0,6,6), STRID_OPTDLG_INITH));
    pl->addItem(m_peditInitH = new AXLineEdit(this, AXLineEdit::ES_SPIN, 0, 0, MAKE_DW4(0,0,0,6)));

    m_peditInitH->setInit(6, 1, CDraw::IMGSIZE_MAX, p->nInitH);

    //アンドゥ回数

    pl->addItem(new AXLabel(this, 0, LF_CENTER_Y, MAKE_DW4(0,0,6,6), STRID_OPTDLG_UNDOCNT));
    pl->addItem(m_peditUndo = new AXLineEdit(this, AXLineEdit::ES_SPIN, 0, 0, MAKE_DW4(0,0,0,6)));

    m_peditUndo->setInit(6, 2, 400, p->nUndoCnt);

    //キャンバス背景色

    pl->addItem(new AXLabel(this, 0, LF_CENTER_Y, MAKE_DW4(0,0,6,6), STRID_OPTDLG_CANVASCOL));
    pl->addItem(m_pbtCanvasCol = new AXColorButton(this, AXColorButton::CBTS_CHOOSE, LF_CENTER_Y, 0, MAKE_DW4(0,0,0,6)));

    m_pbtCanvasCol->setColor(p->dwCanvasCol);
    
    //サブウィンドウタイプ
    
    pl->addItem(new AXLabel(this, 0, LF_CENTER_Y, MAKE_DW4(0,0,6,0), STRID_OPTDLG_SUBWINTYPE));
    pl->addItem(m_pcbWinType = new AXComboBox(this, 0, LF_CENTER_Y));
    
    m_pcbWinType->addItemMulTr(STRID_OPTDLG_SUBWINTYPE_TOP, 2);
    m_pcbWinType->setAutoWidth();
    m_pcbWinType->setCurSel(p->nToolboxType);

    //------- フォント

    plTop->addItem(pl = new AXLayoutVert(0, 7));

    pl->addItem(new AXButton(this, 0, 0, WID_BTT_SUBWINFONT, 0, STRID_OPTDLG_SUBWINFONT));
    pl->addItem(new AXLabel(this, 0, 0, 0, STRID_OPTDLG_RESTART_HINT));

    m_strSubWinFont = p->strSubWinFont;
}

//! 値取得

void COptDlg_opt1::getDat(COptionDlg::OPTIONDAT *p)
{
    p->nInitW       = m_peditInitW->getVal();
    p->nInitH       = m_peditInitH->getVal();
    p->nUndoCnt     = m_peditUndo->getVal();
    p->dwCanvasCol  = m_pbtCanvasCol->getColor();
    p->nToolboxType = m_pcbWinType->getCurSel();
    p->strSubWinFont = m_strSubWinFont;
}

//! 通知

BOOL COptDlg_opt1::onNotify(AXWindow *pwin,UINT uNotify,ULONG lParam)
{
    if(pwin->getItemID() == WID_BTT_SUBWINFONT)
        //フォント選択
        AXFontDialog::getFontInfo(m_pTopLevel, &m_strSubWinFont);

    return TRUE;
}


//*********************************
// COptDlg_opt2 : 設定2
//*********************************


COptDlg_opt2::COptDlg_opt2(AXWindow *pParent,COptionDlg::OPTIONDAT *p)
    : COptDlg_base(pParent)
{
    AXLayout *pl;
    int i;

    _trgroup(STRGID_DLG_OPTION);

    setLayout(pl = new AXLayoutVert(0, 5));

    for(i = 0; i < 5; i++)
    {
        pl->addItem(m_pCheck[i] = new AXCheckButton(this, 0, 0, 0, 0, STRID_OPTDLG_FLAGS_TOP + i,
                                                    p->uFlags & (1 << i)));
    }
}

//! 値取得

void COptDlg_opt2::getDat(COptionDlg::OPTIONDAT *p)
{
    int i;

    p->uFlags = 0;

    for(i = 0; i < 5; i++)
    {
        if(m_pCheck[i]->isChecked())
            p->uFlags |= (1 << i);
    }
}


//*********************************
// COptDlg_optstep : 増減幅
//*********************************


COptDlg_optstep::COptDlg_optstep(AXWindow *pParent,COptionDlg::OPTIONDAT *p)
    : COptDlg_base(pParent)
{
    AXLayout *pl;
    int i;

    _trgroup(STRGID_DLG_OPTION);

    setLayout(pl = new AXLayoutMatrix(2));

    for(i = 0; i < 4; i++)
    {
        pl->addItem(new AXLabel(this, 0, LF_CENTER_Y, MAKE_DW4(0,0,6,6), STRID_OPTDLG_STEP_SCALED + i));
        pl->addItem(m_pedit[i] = new AXLineEdit(this, AXLineEdit::ES_SPIN, 0, 0, MAKE_DW4(0,0,0,6)));
    }

    m_pedit[0]->setInit(5, 1, 100, p->nStepScaleD);
    m_pedit[1]->setInit(5, 1, 800, p->nStepScaleU);
    m_pedit[2]->setInit(5, 1, 180, p->nStepRotate);
    m_pedit[3]->setInit(5, 1, 1000, 1, p->nStepBrushSize);
}

//! 値取得

void COptDlg_optstep::getDat(COptionDlg::OPTIONDAT *p)
{
    p->nStepScaleD  = m_pedit[0]->getVal();
    p->nStepScaleU  = m_pedit[1]->getVal();
    p->nStepRotate  = m_pedit[2]->getVal();
    p->nStepBrushSize = m_pedit[3]->getVal();
}


//*********************************
// COptDlg_optdir : ディレクトリ
//*********************************


COptDlg_optdir::COptDlg_optdir(AXWindow *pParent,COptionDlg::OPTIONDAT *p)
    : COptDlg_base(pParent)
{
    AXLayout *pl;
    int i;

    _trgroup(STRGID_DLG_OPTION);

    setLayout(pl = new AXLayoutMatrix(2));

    for(i = 0; i < 2; i++)
    {
        pl->addItem(new AXLabel(this, 0, 0, MAKE_DW4(0,0,0,4), STRID_OPTDLG_TEXTUREDIR + i));
        pl->addItem(new AXSpacerItem);

        pl->addItem(m_pedit[i] = new AXLineEdit(this, 0, LF_EXPAND_W, 0, MAKE_DW4(0,0,3,6)));
        pl->addItem(new AXButton(this, AXButton::BS_REAL_WH, LF_CENTER_Y, WID_BUTTON + i, MAKE_DW4(0,0,0,6), "..."));
    }

    m_pedit[0]->setText(p->strTextureDir);
    m_pedit[1]->setText(p->strBrushDir);
}

//! 値取得

void COptDlg_optdir::getDat(COptionDlg::OPTIONDAT *p)
{
    m_pedit[0]->getText(&p->strTextureDir);
    m_pedit[1]->getText(&p->strBrushDir);
}

//! 通知

BOOL COptDlg_optdir::onNotify(AXWindow *pwin,UINT uNotify,ULONG lParam)
{
    UINT id = pwin->getItemID();

    if(id >= WID_BUTTON && id < WID_BUTTON + 2)
    {
        AXString str;

        id -= WID_BUTTON;

        m_pedit[id]->getText(&str);

        if(AXFileDialog::openDir(m_pTopLevel, str, 0, &str))
            m_pedit[id]->setText(str);
    }

    return TRUE;
}


//*********************************
// COptDlg_optcursor : カーソル
//*********************************
/*
    m_memDrawCursor : 空でデフォルト
*/


COptDlg_optcursor::COptDlg_optcursor(AXWindow *pParent,COptionDlg::OPTIONDAT *p)
    : COptDlg_base(pParent)
{
    AXLayout *plTop,*plh,*plv,*plm;
    LPBYTE pDat;

    m_memDrawCur.copy(p->memDrawCursor);

    pDat = m_memDrawCur;

    //

    _trgroup(STRGID_DLG_OPTION);

    setLayout(plTop = new AXLayoutVert(0, 10));

    //------

    plTop->addItem(plh = new AXLayoutHorz(0, 15));

    //ホットスポット位置

    plh->addItem(plv = new AXLayoutVert(0, 5));

    plv->addItem(new AXLabel(this, 0, 0, 0, STRID_OPTDLG_HOTSPOT));
    plv->addItem(plm = new AXLayoutMatrix(2));

    plm->addItem(new AXLabel(this, 0, LF_CENTER_Y, MAKE_DW4(0,0,6,5), "X"));
    plm->addItem(m_peditX = new AXLineEdit(this, AXLineEdit::ES_SPIN, 0, 0, MAKE_DW4(0,0,0,5)));

    plm->addItem(new AXLabel(this, 0, LF_CENTER_Y, MAKE_DW4(0,0,6,0), "Y"));
    plm->addItem(m_peditY = new AXLineEdit(this, AXLineEdit::ES_SPIN, 0));

    m_peditX->setInit(4, 0, 31, (pDat)? pDat[2]: 0);
    m_peditY->setInit(4, 0, 31, (pDat)? pDat[3]: 0);

    //プレビュー

    plh->addItem(m_pPrev = new CImgPrev(this, 0, 0, 0, 32, 32));

    _drawPrev();

    //-----

    //画像から読み込み/デフォルト

    plTop->addItem(plh = new AXLayoutHorz(0, 5));

    plh->addItem(new AXButton(this, 0, 0, WID_BTT_LOADIMG, 0, STRID_OPTDLG_CURSOR_LOADIMG));
    plh->addItem(new AXButton(this, 0, 0, WID_BTT_DEFAULT, 0, STRID_OPTDLG_CURSOR_DEFAULT));

    //ヘルプ

    plTop->addItem(new AXLabel(this, AXLabel::LS_BORDER, LF_EXPAND_W, 0, STRID_OPTDLG_CURSOR_HELP));
}

//! 値取得

void COptDlg_optcursor::getDat(COptionDlg::OPTIONDAT *p)
{
    LPBYTE pCur = m_memDrawCur;

    //ホットスポット位置

    if(pCur)
    {
        pCur[2] = m_peditX->getVal();
        pCur[3] = m_peditY->getVal();

        if(pCur[2] > pCur[0]) pCur[2] = pCur[0] - 1;
        if(pCur[3] > pCur[1]) pCur[3] = pCur[1] - 1;
    }

    p->memDrawCursor.copy(m_memDrawCur);
}

//! 通知

BOOL COptDlg_optcursor::onNotify(AXWindow *pwin,UINT uNotify,ULONG lParam)
{
    switch(pwin->getItemID())
    {
        //画像から読み込み
        case WID_BTT_LOADIMG:
            _loadImg();
            break;
        //デフォルト
        case WID_BTT_DEFAULT:
            m_memDrawCur.free();

            _drawPrev();
            break;
    }

    return TRUE;
}

//! 画像から読み込み

void COptDlg_optcursor::_loadImg()
{
    AXString str,filter;
    CImage32 img;

    //ファイル名

    filter = "Image File (BMP/PNG/GIF)\t*.bmp;*.png;*.gif\t";

    if(!AXFileDialog::openFile(m_pTopLevel, filter, 0, NULL, 0, &str))
        return;

    //作成

    if(!img.createCursor(str, &m_memDrawCur))
        ((AXTopWindow *)m_pTopLevel)->errMes(STRGID_MESSAGE, STRID_MES_ERR_LOAD);
    else
        _drawPrev();
}

//! カーソルプレビュー描画

void COptDlg_optcursor::_drawPrev()
{
    AXImage *pimg;
    LPBYTE pCur,pCurCol,pCurMask;
    int w,h,ix,iy,pitchCur,xpos,f;

    pimg = m_pPrev->getImg();

    pimg->clear(0xcccccc);

    //カーソルデータ

    pCur = m_memDrawCur;
    if(!pCur) pCur = g_pdraw->getCursorDat(CDraw::CURSOR_DRAW);

    w = pCur[0];
    h = pCur[1];
    pitchCur = (w + 7) >> 3;

    //描画

    pCurCol  = pCur + 4;
    pCurMask = pCur + 4 + pitchCur * h;

    for(iy = 0; iy < h; iy++)
    {
        for(ix = 0, f = 1, xpos = 0; ix < w; ix++)
        {
            if(pCurMask[xpos] & f)
                pimg->setPixel(ix, iy, (pCurCol[xpos] & f)? 0: 0xffffff);

            f <<= 1;
            if(f == 256) f = 1, xpos++;
        }

        pCurCol  += pitchCur;
        pCurMask += pitchCur;
    }

    m_pPrev->redraw();
}
