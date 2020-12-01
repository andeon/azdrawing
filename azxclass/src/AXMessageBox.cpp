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

#include "AXMessageBox.h"

#include "AXLayout.h"
#include "AXLabel.h"
#include "AXButton.h"
#include "AXCheckButton.h"
#include "AXApp.h"
#include "AXTrSysID.h"


/*!
    @class AXMessageBox
    @brief メッセージボックスダイアログ

    - 関数で使う場合は、@n
      @code
ret = AXMessageBox::message(...);
if(ret == AXMessageBox::OK) ...
@endcode
    - NOTSHOW はチェックボタン。押されたボタンと OR されたフラグで返る。

    @ingroup dialog
*/


AXMessageBox::AXMessageBox(AXWindow *pOwner,LPCUSTR pTitle,LPCUSTR pMessage,UINT uBttFlags,UINT uDefBtt)
    : AXDialog(pOwner,
               WS_TITLE | WS_CLOSE | WS_BORDER | WS_MENUBTT | WS_TABMOVE |
               WS_HIDE | WS_TRANSIENT_FOR | WS_BK_FACE | WS_DISABLE_IM)
{
    m_uBttFlags = uBttFlags;
    m_uDownBtt  = 0;
    m_uOrFlags  = 0;

    if(pTitle) setTitle(pTitle);

    _createMessageBox(pMessage, uDefBtt);

    calcDefSize();
    resize(m_nDefW, m_nDefH);
}

//! 関数
/*!
    @param pTitle       ウィンドウタイトル。NULLでなし。
    @param uBttFlags    置くボタンのフラグ。 AXMessageBox::MBBUTTONS
    @param uDefBtt      デフォルトボタン。 AXMessageBox::MBBUTTONS
    @return 押されたボタンのフラグ。 AXMessageBox::MBBUTTONS
*/

UINT AXMessageBox::message(AXWindow *pOwner,LPCUSTR pTitle,LPCUSTR pMessage,UINT uBttFlags,UINT uDefBtt)
{
    AXMessageBox *p = new AXMessageBox(pOwner, pTitle, pMessage, uBttFlags, uDefBtt);
    return p->run();
}

//! エラーメッセージ関数

void AXMessageBox::error(AXWindow *pOwner,LPCUSTR pMessage)
{
    AXString str("error");
    AXMessageBox *p = new AXMessageBox(pOwner, str, pMessage, OK, OK);
    p->run();
}

//! 実行

UINT AXMessageBox::run()
{
    show();

    return runDialog();
}

//! ウィジェット作成

void AXMessageBox::_createMessageBox(LPCUSTR pMessage,UINT uDefBtt)
{
    AXLayout *plv,*plb;

    setLayout(plv = new AXLayoutVert(0, 10));
    plv->setSpacing(10);

    //メッセージ

    plv->addItem(new AXLabel(this, 0, LF_EXPAND_X | LF_EXPAND_Y | LF_CENTER_X | LF_CENTER_Y, 0, pMessage));

    //"このメッセージを表示しない"

    if(m_uBttFlags & NOTSHOW)
    {
        plv->addItem(new AXCheckButton(this, 0, LF_CENTER_X, NOTSHOW, 0,
                                   _string(TRSYS_GROUPID, TRSYS_NOTSHOW_MESSAGE), FALSE));
    }

    //---------- ボタン

    plv->addItem(plb = new AXLayoutHorz(LF_EXPAND_X | LF_CENTER_X, 4));

    plb->setPaddingTop(6);

    if(m_uBttFlags & SAVE) _btt(plb, SAVE, TRSYS_SAVE, "Save", 'S');
    if(m_uBttFlags & SAVENO) _btt(plb, SAVENO, TRSYS_SAVENO, "NoSave", 'U');
    if(m_uBttFlags & YES) _btt(plb, YES, TRSYS_YES, "Yes", 'Y');
    if(m_uBttFlags & NO) _btt(plb, NO, TRSYS_NO, "No", 'N');
    if(m_uBttFlags & OK) _btt(plb, OK, TRSYS_OK, "OK", 'O');
    if(m_uBttFlags & CANCEL) _btt(plb, CANCEL, TRSYS_CANCEL, "Cancel", 'C');
    if(m_uBttFlags & ABORT) _btt(plb, ABORT, TRSYS_ABORT, "Abort", 'A');

    //デフォルトボタン

    AXWindow *pdef = getWidget(uDefBtt);
    if(pdef)
    {
        setDefaultButton(pdef);
        pdef->setFocus();
    }
}

//! ボタン作成

void AXMessageBox::_btt(AXLayout *pl,UINT btt,WORD wStrID,LPCSTR szDef,char key)
{
    AXString str;
    LPCUSTR pstr = axapp->getTrStringRaw(TRSYS_GROUPID, wStrID);

    if(pstr)
        str = pstr;
    else
        str = szDef;

    //ショートカットキー表記

    str += '(';
    str += key;
    str += ')';

    //

    pl->addItem(new AXButton(this, 0, 0, btt, 0, str));
}


//============================
//ハンドラ
//============================


//! 通知

BOOL AXMessageBox::onNotify(AXWindow *pwin,UINT uNotify,ULONG lParam)
{
    UINT id = pwin->getItemID();

    if(id == NOTSHOW)
        m_uOrFlags ^= NOTSHOW;
    else
        endDialog(id | m_uOrFlags);

    return TRUE;
}

//! 閉じる

BOOL AXMessageBox::onClose()
{
    int ret;

    if(m_uBttFlags & CANCEL)
        ret = CANCEL;
    else if(m_uBttFlags & NO)
        ret = NO;
    else if(m_uBttFlags & SAVENO)
        ret = SAVENO;
    else
        ret = OK;

    endDialog(ret | m_uOrFlags);

    return TRUE;
}

//! キー押し

BOOL AXMessageBox::onKeyDown(AXHD_KEY *phd)
{
    int c;
    UINT bflag;

    //デフォルト処理

    if(AXDialog::onKeyDown(phd)) return TRUE;

    //ショートカットキー

    if(phd->keysym >= 'A' && phd->keysym <= 'Z')
        c = phd->keysym;
    else if(phd->keysym >= 'a' && phd->keysym <= 'z')
        c = phd->keysym - 0x20;
    else
        c = -1;

    if(c != -1 && m_uDownBtt == 0)
    {
        if(c == 'O') bflag = OK;
        else if(c == 'C') bflag = CANCEL;
        else if(c == 'Y') bflag = YES;
        else if(c == 'N') bflag = NO;
        else if(c == 'S') bflag = SAVE;
        else if(c == 'U') bflag = SAVENO;
        else if(c == 'A') bflag = ABORT;
        else bflag = 0;

        if(m_uBttFlags & bflag)
        {
            m_uDownBtt = bflag;

            ((AXButton *)getWidget(bflag))->press();

            return TRUE;
        }
    }

    return FALSE;
}

//! キー離し

BOOL AXMessageBox::onKeyUp(AXHD_KEY *phd)
{
    //デフォルト処理

    if(AXDialog::onKeyUp(phd)) return TRUE;

    //ショートカットキー

    if(m_uDownBtt)
        endDialog(m_uDownBtt | m_uOrFlags);

    return TRUE;
}
