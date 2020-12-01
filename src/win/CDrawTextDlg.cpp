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

#include "CDrawTextDlg.h"

#include "CDraw.h"
#include "CConfig.h"
#include "CMainWin.h"

#include "AXLayout.h"
#include "AXLabel.h"
#include "AXMultiEdit.h"
#include "AXComboBox.h"
#include "AXLineEdit.h"
#include "AXCheckButton.h"
#include "AXApp.h"
#include "AXUtilFontConfig.h"
#include "AXKey.h"

#include "defGlobal.h"
#include "defStrID.h"


//--------------------------

#define WID_EDIT        100
#define WID_FONT        101
#define WID_STYLE       102
#define WID_SIZE        103
#define WID_CHARSPACE   104
#define WID_LINESPACE   105
#define WID_VERT        106
#define WID_2COL        107
#define WID_HINTING     108
#define WID_PREVIEW     109

//--------------------------


//*************************************
// CDrawTextDlgEdit - 複数行エディット
//*************************************


class CDrawTextDlgEdit:public AXMultiEdit
{
public:
    enum { NOTIFY_POSMOVE = 100 };

    CDrawTextDlgEdit(AXWindow *pParent);

    BOOL onKeyDown(AXHD_KEY *phd);
};


CDrawTextDlgEdit::CDrawTextDlgEdit(AXWindow *pParent)
    : AXMultiEdit(pParent, 0, LF_EXPAND_WH, WID_EDIT, 0)
{

}

BOOL CDrawTextDlgEdit::onKeyDown(AXHD_KEY *phd)
{
    int dir = -1;

    //Ctrl+矢印キーで 1px 移動

    if((phd->state & AXWindow::STATE_MODMASK) == AXWindow::STATE_CTRL)
    {
        switch(phd->keysym)
        {
            case KEY_LEFT:
            case KEY_NUM_LEFT:
                dir = 0;
                break;
            case KEY_UP:
            case KEY_NUM_UP:
                dir = 1;
                break;
            case KEY_RIGHT:
            case KEY_NUM_RIGHT:
                dir = 2;
                break;
            case KEY_DOWN:
            case KEY_NUM_DOWN:
                dir = 3;
                break;
        }
    }

    if(dir != -1)
    {
        g_pdraw->text_move1px(dir);

        getNotify()->onNotify(this, NOTIFY_POSMOVE, 0);

        return TRUE;
    }
    else
        return AXMultiEdit::onKeyDown(phd);
}


//*********************************
// CDrawTextDlg
//*********************************


/*!
    @class CDrawTextDlg
    @brief テキスト描画ダイアログ
*/


CDrawTextDlg::CDrawTextDlg()
    : AXDialog(MAINWIN, WS_DIALOG_NORMAL | WS_BK_FACE)
{
    AXLayout *plTop,*plF,*pl,*plFh;

    _trgroup(STRGID_DRAWTEXT);

    setTitle(STRID_DTEXT_TITLE);

    //メインレイアウト

    setLayout(plTop = new AXLayoutHorz(0, 10));

    plTop->setSpacing(8);

    //------- テキスト入力・プレビュー・ヒント

    plTop->addItem(pl = new AXLayoutVert(LF_EXPAND_WH, 4));

    pl->addItem(m_pEdit = new CDrawTextDlgEdit(this));
    pl->addItem(new AXCheckButton(this, 0, 0, WID_PREVIEW, 0, STRID_DTEXT_PREVIEW, g_pdraw->m_uTextFlags & CDraw::TEXTF_PREVIEW));
    pl->addItem(new AXLabel(this, AXLabel::LS_BORDER, LF_EXPAND_W, 0, STRID_DTEXT_HINT));

    //------- フォント設定

    plTop->addItem(plF = new AXLayoutVert(0, 5));

    //フォント名

    plF->addItem(m_pcbFont = new AXComboBox(this, 0, LF_EXPAND_W, WID_FONT, 0));

    //フォントスタイル

    plF->addItem(pl = new AXLayoutHorz(LF_EXPAND_W, 5));

    pl->addItem(new AXLabel(this, 0, LF_CENTER_Y, 0, STRID_DTEXT_STYLE));
    pl->addItem(m_pcbStyle = new AXComboBox(this, 0, LF_EXPAND_W, WID_STYLE, 0));

    //サイズ

    plF->addItem(plFh = new AXLayoutHorz(0, 5));
    plFh->addItem(pl = new AXLayoutMatrix(2));

    pl->addItem(new AXLabel(this, 0, LF_CENTER_Y, MAKE_DW4(0,0,4,4), STRID_DTEXT_SIZE));
    pl->addItem(m_peditSize = new AXLineEdit(this, AXLineEdit::ES_SPIN, 0, WID_SIZE, MAKE_DW4(0,0,0,4)));

    m_peditSize->setWidthFromLen(4);
    m_peditSize->setValStatus(1, 500);
    m_peditSize->setVal(g_pdraw->m_nTextSize);

    //字間

    pl->addItem(new AXLabel(this, 0, LF_CENTER_Y, MAKE_DW4(0,0,4,4), STRID_DTEXT_CHARSPACE));
    pl->addItem(m_peditCharSP = new AXLineEdit(this, AXLineEdit::ES_SPIN, 0, WID_CHARSPACE, MAKE_DW4(0,0,0,4)));

    m_peditCharSP->setWidthFromLen(4);
    m_peditCharSP->setValStatus(-100, 100);
    m_peditCharSP->setVal(g_pdraw->m_nTextCharSP);

    //行間

    pl->addItem(new AXLabel(this, 0, LF_CENTER_Y, MAKE_DW4(0,0,4,0), STRID_DTEXT_LINESPACE));
    pl->addItem(m_peditLineSP = new AXLineEdit(this, AXLineEdit::ES_SPIN, 0, WID_LINESPACE, 0));

    m_peditLineSP->setWidthFromLen(4);
    m_peditLineSP->setValStatus(-100, 100);
    m_peditLineSP->setVal(g_pdraw->m_nTextLineSP);

    //チェック

    plFh->addItem(pl = new AXLayoutVert(0, 5));

    pl->addItem(new AXCheckButton(this, 0, 0, WID_VERT, 0, STRID_DTEXT_VERT, g_pdraw->m_uTextFlags & CDraw::TEXTF_VERT));
    pl->addItem(new AXCheckButton(this, 0, 0, WID_2COL, 0, STRID_DTEXT_2COL, g_pdraw->m_uTextFlags & CDraw::TEXTF_2COL));

    //ヒンティング

    plF->addItem(pl = new AXLayoutHorz(0, 5));

    pl->addItem(new AXLabel(this, 0, LF_CENTER_Y, 0, STRID_DTEXT_HINTING));
    pl->addItem(m_pcbHinting = new AXComboBox(this, 0, 0, WID_HINTING, 0));

    m_pcbHinting->addItemMulTr(STRID_DTEXT_HT_NONE, 4);
    m_pcbHinting->setAutoWidth();
    m_pcbHinting->setCurSel(g_pdraw->m_nTextHinting);

    //ボタン

    pl = createOKCancelButton();
    pl->setPaddingTop(10);

    plF->addItem(pl);

    //---------

    //フォント名とスタイル

    _setFontFace();

    m_pcbFont->getItemText(-1, &g_pdraw->m_strFontFace);

    _setFontStyle();

    m_pcbStyle->getItemText(-1, &g_pdraw->m_strFontStyle);

    //フォント作成

    g_pdraw->text_createFontInit();

    //前回のテキストがある場合

    if(g_pdraw->m_strDrawText.isNoEmpty())
    {
        m_pEdit->setFocus();
        m_pEdit->setText(g_pdraw->m_strDrawText);
        m_pEdit->selectAll();

        if(g_pdraw->isTextPreview())
            g_pdraw->draw_textPrev();
    }

    //--------

    calcDefSize();

    AXRectSize rcs;

    rcs.set(-1, -1, (m_nDefW < 350)? 350: m_nDefW, m_nDefH);
    showInit(g_pconf->rcsTextWin, rcs, CConfig::WININITVAL, TRUE);
}

//! 閉じる

BOOL CDrawTextDlg::onClose()
{
    _end(FALSE);
    return TRUE;
}

//! 通知

BOOL CDrawTextDlg::onNotify(AXWindow *pwin,UINT uNotify,ULONG lParam)
{
    UINT id = pwin->getItemID();

    switch(id)
    {
        //エディット
        case WID_EDIT:
            if(uNotify == AXMultiEdit::MEN_CHANGE)
            {
                m_pEdit->getText(&g_pdraw->m_strDrawText);

                _prev();
            }
            else if(uNotify == CDrawTextDlgEdit::NOTIFY_POSMOVE)
                _prev();
            break;

        //フォント名
        case WID_FONT:
            if(uNotify == AXComboBox::CBN_SELCHANGE)
            {
                m_pcbFont->getItemText(-1, &g_pdraw->m_strFontFace);

                _setFontStyle();
                m_pcbStyle->getItemText(-1, &g_pdraw->m_strFontStyle);

                g_pdraw->text_createFont();
                _prev();
            }
            break;
        //フォントスタイル
        case WID_STYLE:
            if(uNotify == AXComboBox::CBN_SELCHANGE)
            {
                m_pcbStyle->getItemText(-1, &g_pdraw->m_strFontStyle);

                g_pdraw->text_createFont();
                _prev();
            }
            break;
        //サイズ
        case WID_SIZE:
            if(uNotify == AXLineEdit::EN_CHANGE)
            {
                g_pdraw->m_nTextSize = m_peditSize->getVal();

                g_pdraw->text_createFont();
                _prev();
            }
            break;
        //字間
        case WID_CHARSPACE:
            if(uNotify == AXLineEdit::EN_CHANGE)
            {
                g_pdraw->m_nTextCharSP = m_peditCharSP->getVal();
                _prev();
            }
            break;
        //行間
        case WID_LINESPACE:
            if(uNotify == AXLineEdit::EN_CHANGE)
            {
                g_pdraw->m_nTextLineSP = m_peditLineSP->getVal();
                _prev();
            }
            break;
        //縦書き
        case WID_VERT:
            g_pdraw->m_uTextFlags ^= CDraw::TEXTF_VERT;
            _prev();
            break;
        //2値
        case WID_2COL:
            g_pdraw->m_uTextFlags ^= CDraw::TEXTF_2COL;

            g_pdraw->text_clearCache();
            _prev();
            break;
        //ヒンティング
        case WID_HINTING:
            if(uNotify == AXComboBox::CBN_SELCHANGE)
            {
                g_pdraw->m_nTextHinting = m_pcbHinting->getCurSel();

                g_pdraw->text_clearCache();
                _prev();
            }
            break;
        //プレビュー
        case WID_PREVIEW:
            g_pdraw->m_uTextFlags ^= CDraw::TEXTF_PREVIEW;
            g_pdraw->draw_textPrev();
            break;

        //OK
        case 1:
            _end(TRUE);
            break;
        //キャンセル
        case 2:
            _end(FALSE);
            break;
    }

    return TRUE;
}

//! タイマー

BOOL CDrawTextDlg::onTimer(UINT uTimerID,ULONG lParam)
{
    g_pdraw->draw_textPrev();

    delTimer(uTimerID);

    return TRUE;
}


//========================
//
//========================


//! 終了

void CDrawTextDlg::_end(BOOL bOK)
{
    //タイマーが残っている

    if(isTimerExist(0))
    {
        delTimer(0);
        g_pdraw->draw_textPrev();
    }

    //位置とサイズ保存

    AXTopWindow::getTopWindowRect(&g_pconf->rcsTextWin);

    //

    endDialog(bOK);
}

//! プレビュー表示

void CDrawTextDlg::_prev()
{
    if(g_pdraw->isTextPreview())
        addTimer(0, 50);
}

//! フォント名セット

void CDrawTextDlg::_setFontFace()
{
    FcFontSet *fs;
    int i;
    AXString str;

    //コンボボックスにセット

    fs = AXFCGetFamilyList();

    if(fs)
    {
        for(i = 0; i < fs->nfont; i++)
        {
            if(AXFCGetPatternString(fs->fonts[i], FC_FAMILY, &str))
                m_pcbFont->addItemSort(str, 0);
        }

        FcFontSetDestroy(fs);
    }

    //

    m_pcbFont->setAutoWidth();

    //-------- 初期選択

    if(g_pdraw->m_strFontFace.isEmpty())
        i = 0;
    else
    {
        i = m_pcbFont->findItemText(g_pdraw->m_strFontFace);
        if(i == -1) i = 0;
    }

    m_pcbFont->setCurSel(i);
}

//! フォントスタイルリストセット

void CDrawTextDlg::_setFontStyle()
{
    FcFontSet *fs;
    AXString str;
    int i;

    //コンボボックスにセット

    m_pcbStyle->deleteItemAll();

    fs = AXFCGetStyleList(g_pdraw->m_strFontFace);

    if(fs)
    {
        for(i = 0; i < fs->nfont; i++)
        {
            if(AXFCGetPatternString(fs->fonts[i], FC_STYLE, &str))
                m_pcbStyle->addItem(str);
        }

        FcFontSetDestroy(fs);
    }

    //初期選択

    if(g_pdraw->m_strFontStyle.isEmpty())
        i = 0;
    else
    {
        i = m_pcbStyle->findItemText(g_pdraw->m_strFontStyle);
        if(i == -1) i = 0;
    }

    m_pcbStyle->setCurSel(i);
}
