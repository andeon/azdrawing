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

#include "AXFileDialog.h"

#include "AXLayout.h"
#include "AXFileListView.h"
#include "AXButton.h"
#include "AXLineEdit.h"
#include "AXCheckButton.h"
#include "AXComboBox.h"
#include "AXLabel.h"

#include "AXTrSysID.h"
#include "AXApp.h"
#include "AXUtilStr.h"
#include "AXUtilFile.h"
#include "AXMessageBox.h"


//-------------------------

#define WID_ROOT    100
#define WID_HOME    101
#define WID_SHOWDOT 102
#define WID_OK      103
#define WID_CANCEL  104

//-------------------------

/*!
    @class AXFileDialog
    @brief ファイル選択ダイアログ

    - 独自の処理を行いたい場合は、onNotifyFileDialog() で処理。@n
      トップレイアウトは水平または垂直で作成され、その中にさらにメインのレイアウトが入っている。@n
      独自のウィジェットを配置したい場合は、トップレイアウトに追加する。

    @ingroup dialog
*/

/*!
    @var AXFileDialog::FDF_MULTISEL
    @brief ファイル開く時、複数選択を有効に
    @var AXFileDialog::FDF_SHOW_DOT
    @brief 先頭に '.' が付く隠しファイルを表示する
    @var AXFileDialog::FDF_NO_OVERWRITE_MES
    @brief ファイル保存時、上書き確認メッセージを表示しない
    @var AXFileDialog::FDF_CUSTOM_HORZ
    @brief カスタム時、全体のレイアウトを水平で作成する（デフォルトは垂直）
*/
/*!
    @enum AXFileDialog::FILEDIALOGCUSTOMNOTIFY
    @brief onNotifyFileDialog() 時の通知

    @var AXFileDialog::FDCN_OK
    @brief 決定された時（FALSEを返すと処理をキャンセル）
    @var AXFileDialog::FDCN_CANCEL
    @brief キャンセルされた時（FALSEを返すと処理をキャンセル）
    @var AXFileDialog::FDCN_SELFILE
    @brief ファイルの選択が変更された
*/


AXFileDialog::~AXFileDialog()
{

}

AXFileDialog::AXFileDialog(AXWindow *pOwner,int type,LPCUSTR pstrFilter,
                           int deftype,LPCUSTR pstrInitDir,UINT uFlags,AXString *pstrRet,LPINT pRetType)
    : AXDialog(pOwner, WS_DIALOG_NORMAL | WS_BK_FACE)
{
    m_nDlgType  = type;
    m_uDlgFlags = uFlags;
    m_pstrRet   = pstrRet;
    m_pRetType  = pRetType;

    if(pstrFilter)
        m_strFilter.setSplitCharNULL(pstrFilter, '\t');

    _init(pstrInitDir, deftype);
}

//! 実行

BOOL AXFileDialog::run(int w,int h)
{
    resize(w, h);
    show();

    return runDialog();
}


//==============================
//関数
//==============================


//! ファイル開く
/*!
    @param pstrFilter   フィルタ文字列。NULLですべてのファイル（例： "画像ファイル\\t*.gif;*.png\\tすべてのファイル\\t*\\t"）
    @param deftype      デフォルトのフィルタタイプ（インデックス番号。0〜）
    @param pstrInitDir  初期ディレクトリ（NULL でホームディレクトリ）
    @param uFlags       AXFileDialog::FILEDIALOGFLAGS
    @param pstrRet      結果のファイル名が返る
*/

BOOL AXFileDialog::openFile(AXWindow *pOwner,LPCUSTR pstrFilter,int deftype,
                            LPCUSTR pstrInitDir,UINT uFlags,AXString *pstrRet)
{
    AXFileDialog *pdlg = new AXFileDialog(pOwner, DTYPE_OPEN, pstrFilter, deftype, pstrInitDir, uFlags, pstrRet, NULL);

    return pdlg->run(450, 400);
}

//! ファイル保存
/*!
    @param pRetType 選択されたファイル（フィルタ）タイプが返る（0〜）。NULL で取得しない。
*/

BOOL AXFileDialog::saveFile(AXWindow *pOwner,LPCUSTR pstrFilter,int deftype,
              LPCUSTR pstrInitDir,UINT uFlags,AXString *pstrRet,LPINT pRetType)
{
    AXFileDialog *pdlg = new AXFileDialog(pOwner, DTYPE_SAVE, pstrFilter, deftype, pstrInitDir, uFlags, pstrRet, pRetType);

    return pdlg->run(450, 400);
}

//! ディレクトリ選択

BOOL AXFileDialog::openDir(AXWindow *pOwner,LPCUSTR pstrInitDir,UINT uFlags,AXString *pstrRet)
{
    AXFileDialog *pdlg = new AXFileDialog(pOwner, DTYPE_DIR, NULL, 0, pstrInitDir, uFlags, pstrRet, NULL);

    return pdlg->run(450, 400);
}


//==============================
//サブ
//==============================


//! 初期化

void AXFileDialog::_init(LPCUSTR pstrInitDir,int deftype)
{
    AXString strDir;
    int n;

    _trgroup(TRSYS_GROUPID);

    //初期ディレクトリ

    if(!pstrInitDir)
        strDir.path_setHomePath();
    else
    {
        strDir = pstrInitDir;

        if(!AXIsExistFile(strDir, TRUE))
            strDir.path_setHomePath();
    }

    //タイトル

    if(m_nDlgType == DTYPE_OPEN)
        n = TRSYS_TITLE_OPENFILE;
    else if(m_nDlgType == DTYPE_SAVE)
        n = TRSYS_TITLE_SAVEFILE;
    else
        n = TRSYS_TITLE_SELECTDIR;

    setTitle(n);

    //ウィンドウ作成

    _initWin();

    //初期状態

    m_peditDir->setText(strDir);

    if(m_nDlgType == DTYPE_SAVE)
    {
        m_peditName->setFocus();
        m_peditName->selectAll();
    }
    else
        m_pList->setFocus();

    if(m_nDlgType != DTYPE_DIR)
    {
        _setTypeCombo(deftype);
        m_pList->setFilter((LPWORD)m_pcbType->getItemParam(-1));
    }

    m_pList->setDirectory(strDir);
}

//! ウィンドウ作成

void AXFileDialog::_initWin()
{
    AXLayout *pltop,*plm,*pl;
    AXButton *pbt;
    UINT flag;

    //全体レイアウト

    if(m_uDlgFlags & FDF_CUSTOM_HORZ)
        pltop = new AXLayoutHorz;
    else
        pltop = new AXLayoutVert;

    pltop->setSpacing(8);

    setLayout(pltop);

    //================ メイン部分

    pltop->addItem(plm = new AXLayoutVert(LF_EXPAND_WH, 6));

    //------ フォルダパス部分（ディレクトリ名＋ROOT+HOME）

    plm->addItem(pl = new AXLayoutHorz(LF_EXPAND_W, 3));

    pl->addItem(m_peditDir = new AXLineEdit(this, AXLineEdit::ES_READONLY, LF_EXPAND_W | LF_CENTER_Y));
    pl->addItem(new AXButton(this, AXButton::BS_REAL_W, 0, WID_ROOT, 0, "root"));
    pl->addItem(new AXButton(this, AXButton::BS_REAL_W, 0, WID_HOME, 0, "home"));

    //------- リスト

    flag = AXScrollView::SVS_SUNKEN | AXListView::LVS_COLLINE;
    if(m_nDlgType == DTYPE_DIR) flag |= AXFileListView::FLVS_ONLY_DIR;
    if(m_nDlgType == DTYPE_OPEN && (m_uDlgFlags & FDF_MULTISEL)) flag |= AXListView::LVS_MULTISEL;
    if(!(m_uDlgFlags & FDF_SHOW_DOT)) flag |= AXFileListView::FLVS_HIDE_DOT;

    m_pList = new AXFileListView(this, flag, LF_EXPAND_WH);
    m_pList->setListColumnWidth(226, 70, 120);

    plm->addItem(m_pList);

    //-------- ファイル名部分

    pl = new AXLayoutMatrix(2);
    pl->setLayoutFlags(LF_EXPAND_W);
    plm->addItem(pl);

    //ファイル名（保存時のみ）

    if(m_nDlgType == DTYPE_SAVE)
    {
        pl->addItem(new AXLabel(this, 0, LF_RIGHT | LF_CENTER_Y, MAKE_DW4(0,0,5,6), TRSYS_FILENAME));
        pl->addItem(m_peditName = new AXLineEdit(this, 0, LF_EXPAND_W, 0, MAKE_DW4(0,0,0,6)));

        m_peditName->setText(*m_pstrRet);
    }
    else
        m_peditName = NULL;

    //種類（ディレクトリ時は除く）

    if(m_nDlgType != DTYPE_DIR)
    {
        pl->addItem(new AXLabel(this, 0, LF_RIGHT | LF_CENTER_Y, MAKE_DW4(0,0,5,0), TRSYS_TYPE));
        pl->addItem(m_pcbType = new AXComboBox(this, 0, LF_EXPAND_W));
    }
    else
        m_pcbType = NULL;

    //-------- 下

    pl = new AXLayoutHorz(LF_EXPAND_W, 4);
    pl->setPaddingTop(8);
    plm->addItem(pl);

    //隠しファイル表示

    pl->addItem(new AXCheckButton(this, 0, LF_EXPAND_X | LF_CENTER_Y,
                        WID_SHOWDOT, MAKE_DW4(0,0,8,0),
                        TRSYS_SHOW_HIDDENFILES, m_uDlgFlags & FDF_SHOW_DOT));

    //開く/保存

    pl->addItem(pbt = new AXButton(this, 0, 0, WID_OK, 0, (m_nDlgType == DTYPE_SAVE)? TRSYS_SAVE: TRSYS_OPEN));

    setDefaultButton(pbt);

    //キャンセル

    pl->addItem(new AXButton(this, 0, 0, WID_CANCEL, 0, TRSYS_CANCEL));
}

//! 種類のコンボボックスセット

void AXFileDialog::_setTypeCombo(int defno)
{
    LPCUSTR p,p1;

    for(p = m_strFilter; *p; )
    {
        //フィルタ文字列
        p1 = p + AXUStrLen(p) + 1;
        if(*p1 == 0) break;

        m_pcbType->addItem(p, (ULONG)p1);

        p = p1 + AXUStrLen(p1) + 1;
    }

    m_pcbType->setCurSel(defno);
}

//! ディレクトリ変更時

void AXFileDialog::_changeDir()
{
    m_peditDir->setText(m_pList->getNowDir());
}

//! 選択ファイルが変更された時

void AXFileDialog::_changeSelFile()
{
    int type;
    AXString str;

    type = m_pList->getFocusFileName(&str, FALSE);

    if(type == AXFileListView::FILETYPE_FILE)
    {
        if(m_nDlgType == DTYPE_SAVE)
            m_peditName->setText(str);

        onNotifyFileDialog(FDCN_SELFILE);
    }
}

//! 終了

void AXFileDialog::_end(BOOL bOK)
{
    int n;
    AXString str;

    if(bOK)
    {
        //--------- 開く/保存

        switch(m_nDlgType)
        {
            //ディレクトリ
            case DTYPE_DIR:
                *m_pstrRet = m_pList->getNowDir();
                break;
            //開く
            case DTYPE_OPEN:
                if(m_uDlgFlags & FDF_MULTISEL)
                {
                    n = m_pList->getMultiFileName(&str);
                    if(n == 0) return;
                }
                else
                {
                    n = m_pList->getFocusFileName(&str, TRUE);

                    if(n != AXFileListView::FILETYPE_FILE)
                        return;
                }

                *m_pstrRet = str;
                break;
            //保存
            case DTYPE_SAVE:
                m_peditName->getText(&str);
                if(str.isEmpty()) return;

                *m_pstrRet = m_pList->getNowDir();
                m_pstrRet->path_add(str);

                //上書き確認

                if(!(m_uDlgFlags & FDF_NO_OVERWRITE_MES) && AXIsExistFile(*m_pstrRet, FALSE))
                {
                    if(AXMessageBox::message(this, NULL, _string(TRSYS_GROUPID, TRSYS_MES_OVERWRITE),
                                    AXMessageBox::YES | AXMessageBox::NO,
                                    AXMessageBox::NO) == AXMessageBox::NO)
                        return;
                }

                //ファイルタイプ

                if(m_pRetType) *m_pRetType = m_pcbType->getCurSel();
                break;
        }

        if(!onNotifyFileDialog(FDCN_OK)) return;
        endDialog(1);
    }
    else
    {
        //-------- キャンセル

        if(!onNotifyFileDialog(FDCN_CANCEL)) return;
        endDialog(0);
    }
}


//==============================
//ハンドラ
//==============================


//! カスタム時の通知

BOOL AXFileDialog::onNotifyFileDialog(int notify)
{
    return TRUE;
}

//! 閉じる（キャンセル）

BOOL AXFileDialog::onClose()
{
    _end(FALSE);
    return TRUE;
}

//! 通知

BOOL AXFileDialog::onNotify(AXWindow *pwin,UINT uNotify,ULONG lParam)
{
    if(pwin == m_pList)
    {
        //リスト

        switch(uNotify)
        {
            //ファイル選択変更
            case AXFileListView::FLVN_SELFILE:
                _changeSelFile();
                break;
            //ディレクトリ移動
            case AXFileListView::FLVN_MOVEDIR:
                _changeDir();
                break;
            //ダブルクリック
            case AXFileListView::FLVN_DBLCLK_FILE:
                _end(TRUE);
                break;
        }
    }
    else if(pwin == m_pcbType)
    {
        //種類変更

        if(uNotify == AXComboBox::CBN_SELCHANGE)
        {
            m_pList->setFilter((LPWORD)m_pcbType->getItemParam(-1));
            m_pList->updateList();
        }
    }
    else
    {
        switch(pwin->getItemID())
        {
            //ルート
            case WID_ROOT:
                m_pList->setDirectoryRoot();
                _changeDir();
                break;
            //ホーム
            case WID_HOME:
                m_pList->setDirectoryHome();
                _changeDir();
                break;
            //隠しファイル
            case WID_SHOWDOT:
                m_pList->setHideDot(!lParam);
                break;
            //開く/保存
            case WID_OK:
                _end(TRUE);
                break;
            //キャンセル
            case WID_CANCEL:
                _end(FALSE);
                break;
        }
    }

    return TRUE;
}
