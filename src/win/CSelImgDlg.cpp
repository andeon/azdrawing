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

#include "CSelImgDlg.h"

#include "CImgPrev.h"
#include "CImage8.h"
#include "CImage32.h"
#include "CConfig.h"

#include "AXLayout.h"
#include "AXButton.h"
#include "AXListBox.h"
#include "AXDir.h"
#include "AXFileStat.h"
#include "AXListBoxItem.h"
#include "AXApp.h"
#include "AXUtilFile.h"
#include "AXTrSysID.h"

#include "defGlobal.h"
#include "defStrID.h"


//------------------

#define WID_LIST            100
#define WID_BTT_NONE        101
#define WID_BTT_USEOPT      102
#define WID_BTT_FORCENONE   103
#define WID_BTT_DEFAULT     104

#define FLIST_BACK      0
#define FLIST_DIR       1
#define FLIST_FILE      2
#define FLIST_USERDIR   8
#define FLIST_TYPEMASK  7

//------------------


/*!
    @class CSelImgDlg
    @brief テクスチャ・ブラシ画像選択ダイアログ
*/



CSelImgDlg::CSelImgDlg(AXWindow *pOwner,int type,const AXString &strDefName)
    : AXDialog(pOwner, WS_DIALOG_NORMAL | WS_BK_FACE | WS_DISABLE_IM)
{
    AXLayout *plTop,*pl;
    AXWindow *pwin;

    m_nType = type;

    //============ ウィジェット

    _trgroup(STRGID_DLG_SELIMG);

    setTitle(STRID_SELIMGDLG_TITLE);

    //

    setLayout(plTop = new AXLayoutHorz(0, 8));
    plTop->setSpacing(8);

    //リスト

    plTop->addItem(m_pList = new AXListBox(this,
                                AXScrollView::SVS_SUNKEN|AXScrollView::SVS_VERT,
                                LF_EXPAND_WH, WID_LIST, 0));

    //------- 右

    plTop->addItem(pl = new AXLayoutVert);

    //プレビュー

    pl->addItem(m_pPrev = new CImgPrev(this, 0, 0, MAKE_DW4(0,0,0,8), 100, 100));

    m_pPrev->getImg()->clear(0x808080);

    //ボタン

    switch(type)
    {
        //なし
        case TYPE_OPT_TEXTURE:
            pl->addItem(new AXButton(this, 0, LF_EXPAND_W, WID_BTT_NONE, MAKE_DW4(0,0,0,2), STRID_SELIMGDLG_NONE));
            break;
        //オプション指定、なし（強制）
        case TYPE_BRUSH_TEXTURE:
            pl->addItem(new AXButton(this, 0, LF_EXPAND_W, WID_BTT_USEOPT, MAKE_DW4(0,0,0,2), STRID_SELIMGDLG_USEOPT));
            pl->addItem(new AXButton(this, 0, LF_EXPAND_W, WID_BTT_FORCENONE, MAKE_DW4(0,0,0,2), STRID_SELIMGDLG_FORCE_NONE));
            break;
        //デフォルト
        case TYPE_BRUSH_BRUSH:
            pl->addItem(new AXButton(this, 0, LF_EXPAND_W, WID_BTT_DEFAULT, MAKE_DW4(0,0,0,2), STRID_SELIMGDLG_DEFAULT));
            break;
    }

    //OKキャンセル

    pl->addItem(pwin = new AXButton(this, WS_DISABLE, LF_EXPAND_W, 1, MAKE_DW4(0,8,0,2), TRSYS_GROUPID, TRSYS_OK));
    pl->addItem(new AXButton(this, 0, LF_EXPAND_W, 2, 0, TRSYS_GROUPID, TRSYS_CANCEL));

    setDefaultButton(pwin);

    //---------

    _setInitDir(strDefName);

    //---------

    calcDefSize();
    resize(m_nDefW + 160, m_nDefH + 50);
    show();
}

//! 実行

BOOL CSelImgDlg::run(AXString *pstrPath)
{
    BOOL ret;

    ret = AXDialog::runDialog(FALSE);

    if(ret)
        *pstrPath = m_strPath;

    delete this;
    axapp->sync();

    return ret;
}

//! 通知

BOOL CSelImgDlg::onNotify(AXWindow *pwin,UINT uNotify,ULONG lParam)
{
    switch(pwin->getItemID())
    {
        //リスト
        case WID_LIST:
            if(uNotify == AXListBox::LBN_SELCHANGE)
                _changeSel();
            else if(uNotify == AXListBox::LBN_DBLCLK)
                _dblClk();
            break;
        //なし・デフォルト
        case WID_BTT_NONE:
        case WID_BTT_FORCENONE:
        case WID_BTT_DEFAULT:
            m_strPath.empty();
            endDialog(TRUE);
            break;
        //オプション指定を使う
        case WID_BTT_USEOPT:
            m_strPath = '?';
            endDialog(TRUE);
            break;

        //OK
        case 1:
            _dblClk();

            endDialog(TRUE);
            break;
        //キャンセル
        case 2:
            endDialog(FALSE);
            break;
    }

    return TRUE;
}


//========================
//サブ
//========================


//! 初期ディレクトリセット

void CSelImgDlg::_setInitDir(const AXString &strDefName)
{
    AXString str,strPath;
    BOOL bUser = TRUE;

    //strPath にパスをセット（空でトップ）

    strPath.path_removeFileName(strDefName);

    if(strPath.isNoEmpty())
    {
        //ユーザーディレクトリで存在するか

        _getRootDir(&str, TRUE);
        str.path_add(strPath);

        if(!AXIsExistFile(str, TRUE))
        {
            //システムディレクトリで存在するか

            _getRootDir(&str, FALSE);
            str.path_add(strPath);

            if(AXIsExistFile(str, TRUE))
                bUser = FALSE;
            else
                strPath.empty();
        }
    }

    //セット

    if(strPath.isEmpty())
        _setList(FALSE);
    else
    {
        m_strPath = strPath;
        m_strPath += '/';

        _setList(bUser);
    }
}

//! プレビューイメージセット

BOOL CSelImgDlg::_setPreview(const AXString &path,BOOL bUserDir)
{
    CImage8 img8;
    CImage32 img32;
    CImage32::LOADINFO info;
    AXString str;

    _getRootDir(&str, bUserDir);
    str.path_add(path);

    //32bit読み込み

    if(!img32.loadImage(str, &info))
        return FALSE;

    //8bit変換＆プレビューセット

    if(m_nType == TYPE_BRUSH_BRUSH)
    {
        //ブラシ

        if(!img8.createFrom32bit_brush(img32, info.bAlpha))
            return FALSE;

        img8.drawBrushPrev(m_pPrev->getImg());
    }
    else
    {
        //テクスチャ

        if(!img8.createFrom32bit(img32, info.bAlpha))
            return FALSE;

        img8.drawTexturePrev(m_pPrev->getImg());
    }

    return TRUE;
}

//! リスト選択変更時

void CSelImgDlg::_changeSel()
{
    int no,param;
    AXString str,name;
    CImage8 img;

    no = m_pList->getCurSel();
    if(no == -1) return;

    param = m_pList->getItemParam(no);

    if((param & FLIST_TYPEMASK) == FLIST_FILE)
    {
        //------- ファイル

        //ファイル名

        m_pList->getItemText(no, &name);

        str = m_strPath;
        str += name;

        //プレビュー

        if(_setPreview(str, param & FLIST_USERDIR))
            enableWidget(1, TRUE);
        else
            m_pPrev->getImg()->clear(0x808080);

        m_pPrev->redraw();
    }
    else
    {
        //------- ディレクトリ

        enableWidget(1, FALSE);
    }
}

//! ダブルクリック時

void CSelImgDlg::_dblClk()
{
    int no,param;
    AXString name;

    no = m_pList->getCurSel();
    param = m_pList->getItemParam(no);

    switch(param & FLIST_TYPEMASK)
    {
        //一つ上に移動
        case FLIST_BACK:
            m_strPath.path_removeEndSlash();
            m_strPath.path_removeFileName();

            if(m_strPath.isNoEmpty()) m_strPath += '/';

            _setList(param & FLIST_USERDIR);
            break;
        //ディレクトリ
        case FLIST_DIR:
            m_pList->getItemText(no, &name);

            //'<' '>' を取り除いてセット
            m_strPath += name.at(1);
            m_strPath.removeEndChar();
            m_strPath += '/';

            _setList(param & FLIST_USERDIR);
            break;
        //ファイル（m_strPath にセットして終了）
        case FLIST_FILE:
            m_pList->getItemText(no, &name);
            m_strPath += name;

            endDialog(TRUE);
            break;
    }
}

//! 画像のルートディレクトリ取得

void CSelImgDlg::_getRootDir(AXString *pStr,BOOL bUser)
{
    if(m_nType == TYPE_BRUSH_BRUSH)
    {
        if(bUser)
            *pStr = g_pconf->strBrushDir;
        else
            axapp->getResourcePath(pStr, "brush");
    }
    else
    {
        if(bUser)
            *pStr = g_pconf->strTextureDir;
        else
            axapp->getResourcePath(pStr, "texture");
    }
}

//! ソート関数

int CSelImgDlg_funcSort(AXListItem *pitem1,AXListItem *pitem2,ULONG lParam)
{
    AXListBoxItem *p1 = (AXListBoxItem *)pitem1;
    AXListBoxItem *p2 = (AXListBoxItem *)pitem2;

    if(p1->m_lParam < p2->m_lParam)
        return -1;
    else if(p1->m_lParam > p2->m_lParam)
        return 1;
    else
        return p1->m_strText.compare(p2->m_strText);
}

//! ファイルリストセット
/*!
    パスは m_strPath。
*/

void CSelImgDlg::_setList(BOOL bUserDir)
{
    AXString str;
    BOOL bTop;

    //トップディレクトリか

    bTop = m_strPath.isEmpty();

    //リスト削除

    m_pList->deleteItemAll();

    //上に戻る

    if(!bTop)
        m_pList->addItem("<..Back>", FLIST_BACK + (bUserDir? FLIST_USERDIR: 0));

    //検索

    if(bTop)
    {
        //トップディレクトリの場合は、システム/ユーザー両方のディレクトリから検索

        _getRootDir(&str, FALSE);
        _searchFile(str, FALSE);

        _getRootDir(&str, TRUE);
        _searchFile(str, TRUE);
    }
    else
    {
        //トップ以外の場合は指定ディレクトリ

        _getRootDir(&str, bUserDir);
        str.path_add(m_strPath);

        _searchFile(str, bUserDir);
    }

    //ソート

    m_pList->sortItem(CSelImgDlg_funcSort, 0);
}

//! ファイル検索

void CSelImgDlg::_searchFile(const AXString &strDir,BOOL bUserDir)
{
    AXDir dir;
    AXString str,filter;
    AXFILESTAT stat;
    int add;

    add = (bUserDir)? FLIST_USERDIR: 0;

    //

    if(!dir.open(strDir)) return;

    filter = "*.bmp;*.png;*.jpg;*.jpeg;*.gif";

    while(dir.read())
    {
        if(!dir.getFileStat(&stat)) continue;

        if(stat.isDirectory())
        {
            //ディレクトリ

            if(dir.isSpecPath()) continue;

            str = '<';
            str += dir.getCurName();
            str += '>';

            m_pList->addItem(str, FLIST_DIR + add);
        }
        else
        {
            //ファイル

            str = dir.getCurName();

            if(!str.compareFilter(filter, ';', TRUE))
                continue;

            m_pList->addItem(str, FLIST_FILE + add);
        }
    }

    dir.close();
}
