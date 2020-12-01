/*****************************************************************************
 *  Copyright (C) 2012-2015 Azel.
 *
 *  This file is part of AzXClass.
 *
 *  This library is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this library.  If not, see <http://www.gnu.org/licenses/>.
******************************************************************************/

#include "AXFontDialog.h"

#include "AXLayout.h"
#include "AXLabel.h"
#include "AXListBox.h"
#include "AXLineEdit.h"
#include "AXListBoxItem.h"
#include "AXTrSysID.h"
#include "AXApp.h"
#include "AXUtilStr.h"
#include "AXUtilFontConfig.h"


//-------------------

#define WID_LB_NAME     100
#define WID_LB_STYLE    101
#define WID_EDIT_SIZE   102

//-------------------


/*!
    @class AXFontDialog
    @brief フォントダイアログ

    @ingroup dialog
*/


AXFontDialog::AXFontDialog(AXWindow *pOwner,FONTINFO *pinfo,UINT uFlags)
    : AXDialog(pOwner,
               WS_TITLE | WS_CLOSE | WS_BORDER | WS_MENUBTT | WS_TABMOVE |
               WS_HIDE | WS_TRANSIENT_FOR | WS_BK_FACE | WS_DISABLE_IM)
{
    AXLayout *plTop,*plh,*plv;

    m_pInfo = pinfo;

    _trgroup(TRSYS_GROUPID);

    setTitle(TRSYS_TITLE_FONTDIALOG);

    //

    setLayout(plTop = new AXLayoutVert(0, 15));
    plTop->setSpacing(10);

    plTop->addItem(plh = new AXLayoutHorz(LF_EXPAND_WH, 8));

    //フォント名

    plh->addItem(plv = new AXLayoutVert(LF_EXPAND_H, 5));

    plv->addItem(new AXLabel(this, 0, 0, 0, TRSYS_FONTNAME));

    plv->addItem(m_plbName = new AXListBox(this,
                                AXScrollView::SVS_SUNKEN | AXScrollView::SVS_VERT | AXScrollView::SVS_VERT_FIX,
                                LF_EXPAND_H, WID_LB_NAME, 0));

    //------ サイズ・スタイル

    plh->addItem(plv = new AXLayoutVert(LF_EXPAND_WH, 5));

    //サイズ

    if(uFlags & FDFLAG_NOSIZE)
        m_peditSize = NULL;
    else
    {
        plv->addItem(new AXLabel(this, 0, 0, 0, TRSYS_FONTSIZE));
        plv->addItem(m_peditSize = new AXLineEdit(this, AXLineEdit::ES_SPIN, 0, WID_EDIT_SIZE, MAKE_DW4(0,0,0,5)));

        m_peditSize->setInit(6, 10, 5000, 1, (m_pInfo->strName.isEmpty())? 100: (int)(m_pInfo->dSize * 10));
    }

    //スタイル

    plv->addItem(new AXLabel(this, 0, 0, 0, TRSYS_FONTSTYLE));
    plv->addItem(m_plbStyle = new AXListBox(this,
                                  AXScrollView::SVS_SUNKEN|AXScrollView::SVS_VERT,
                                  LF_EXPAND_WH, WID_LB_STYLE, 0));

    m_plbStyle->setMinWidth(150);

    //----------

    //ボタン

    plTop->addItem(createOKCancelButton());

    //初期化

    _setFontFace();
    _setFontStyle(TRUE);

    //

    calcDefSize();
    resize(m_nDefW, (m_nDefH < 300)? 300: m_nDefH);

    show();
}

//! フォント名セット

void AXFontDialog::_setFontFace()
{
    FcFontSet *fs;
    AXString str;
    AXListBoxItem *pSel = NULL;
    int i;

    //セット

    fs = AXFCGetFamilyList();

    if(fs)
    {
        for(i = 0; i < fs->nfont; i++)
        {
            if(AXFCGetPatternString(fs->fonts[i], FC_FAMILY, &str))
            {
                m_plbName->addItem(str);

                if(str == m_pInfo->strName)
                    pSel = m_plbName->getBottomItem();
            }
        }

        FcFontSetDestroy(fs);
    }

    //

    m_plbName->sortItem(NULL, 0);
    m_plbName->setAutoWidth(TRUE);
    m_plbName->setCurSel((pSel)? pSel: m_plbName->getTopItem());
    if(pSel) m_plbName->setScrollItem(pSel, 0);
}

//! フォントスタイルリストセット

void AXFontDialog::_setFontStyle(BOOL bInit)
{
    FcFontSet *fs;
    AXString str;
    int i;
    AXListBoxItem *pSel = NULL;

    m_plbStyle->deleteItemAll();

    //セット

    m_plbName->getItemText(-1, &str);

    fs = AXFCGetStyleList(str);

    if(fs)
    {
        for(i = 0; i < fs->nfont; i++)
        {
            if(AXFCGetPatternString(fs->fonts[i], FC_STYLE, &str))
            {
                m_plbStyle->addItem(str);

                if(bInit)
                {
                    if(str == m_pInfo->strStyle)
                        pSel = m_plbStyle->getBottomItem();
                }
            }
        }

        FcFontSetDestroy(fs);
    }

    //選択

    m_plbStyle->setCurSel((pSel)? pSel: m_plbStyle->getTopItem());
}

//! 通知

BOOL AXFontDialog::onNotify(AXWindow *pwin,UINT uNotify,ULONG lParam)
{
    switch(pwin->getItemID())
    {
        //フォント名
        case WID_LB_NAME:
            if(uNotify == AXListBox::LBN_SELCHANGE)
                _setFontStyle(FALSE);
            break;

        //OK
        case 1:
            m_plbName->getItemText(-1, &m_pInfo->strName);
            m_plbStyle->getItemText(-1, &m_pInfo->strStyle);

            if(m_peditSize)
                m_pInfo->dSize = m_peditSize->getVal() * 0.1;

            endDialog(TRUE);
            break;
        case 2:
            endDialog(FALSE);
            break;
    }

    return TRUE;
}


//===========================
// 関数
//===========================


//! ダイアログ表示関数

BOOL AXFontDialog::getFontInfo(AXWindow *pOwner,FONTINFO *pinfo,UINT uFlags)
{
    AXFontDialog *pdlg = new AXFontDialog(pOwner, pinfo, uFlags);

    return pdlg->runDialog();
}

//! ダイアログ表示関数(文字列形式で)

BOOL AXFontDialog::getFontInfo(AXWindow *pOwner,AXString *pstr,UINT uFlags)
{
    AXFontDialog *pdlg;
    AXFontDialog::FONTINFO info;

    AXFontDialog::xftFormatToFontInfo(&info, *pstr);

    pdlg = new AXFontDialog(pOwner, &info, uFlags);

    if(pdlg->runDialog())
    {
        AXFontDialog::fontInfoToXftFormat(pstr, info);
        return TRUE;
    }
    else
        return FALSE;
}

//! xftフォーマットの文字列からフォント情報取得
/*!
    "フォント名:size=*:style=*"
*/

void AXFontDialog::xftFormatToFontInfo(FONTINFO *pinfo,const AXString &strText)
{
    AXString str,strType;
    LPUSTR p,p2;

    pinfo->strStyle.empty();
    pinfo->dSize = 10;

    //

    str.setSplitCharNULL(strText, ':');

    p = str;

    //最初はフォント名

    pinfo->strName = p;

    p += AXUStrLen(p) + 1;

    //以降

    while(*p)
    {
        for(p2 = p; *p2 && *p2 != '='; p2++);

        if(*p2 == '=')
        {
            *p2 = 0;
            p2++;

            //タイプ

            strType = p;

            if(strType == "size")
                pinfo->dSize = AXUStrToDouble(p2);
            else if(strType == "style")
                pinfo->strStyle = p2;
        }

        //次へ

        for(p = p2; *p; p++);

        p++;
    }
}

//! フォント情報からXft形式テキストを取得

void AXFontDialog::fontInfoToXftFormat(AXString *pstr,const FONTINFO &info)
{
    char m[32];

    AXIntToFloatStr(m, (int)(info.dSize * 10), 1);

    *pstr = info.strName;
    *pstr += ":size=";
    *pstr += m;
    *pstr += ":style=";
    *pstr += info.strStyle;
}
