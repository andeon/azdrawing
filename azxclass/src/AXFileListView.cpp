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

#include <time.h>

#include "AXFileListView.h"

#include "AXDir.h"
#include "AXApp.h"
#include "AXTrSysID.h"
#include "AXFileStat.h"
#include "AXImageList.h"

#include "img_iconfile.h"


//--------------------------

#define FLIST_BACK  0
#define FLIST_DIR   1
#define FLIST_FILE  2

/*!
    @class AXFileListView
    @brief ファイルリストビューウィジェット

    @ingroup widget
*/
/*!
    @var AXFileListView::FLVS_HIDE_DOT
    @brief 先頭がドットの隠しファイルを隠す
    @var AXFileListView::FLVS_ONLY_DIR
    @brief ディレクトリのみ表示

    @var AXFileListView::FLVN_SELFILE
    @brief ファイルが選択された
    @var AXFileListView::FLVN_MOVEDIR
    @brief ディレクトリが移動した
    @var AXFileListView::FLVN_DBLCLK_FILE
    @brief ファイルがダブルクリックされた
*/


AXFileListView::~AXFileListView()
{
    delete m_pimgIcon;
}

AXFileListView::AXFileListView(AXWindow *pParent,UINT uStyle,UINT uLayoutFlags)
    : AXListView(pParent, uStyle | SVS_HORZVERT, uLayoutFlags)
{
    _createFileListView();
}

AXFileListView::AXFileListView(AXWindow *pParent,UINT uStyle,UINT uLayoutFlags,UINT uItemID,DWORD dwPadding)
    : AXListView(pParent, uStyle | SVS_HORZVERT, uLayoutFlags, uItemID, dwPadding)
{
    _createFileListView();
}

//! 作成

void AXFileListView::_createFileListView()
{
    m_uType   = TYPE_FILELISTVIEW;

    m_strDir    = '/';
    m_strFilter = '*';

    //アイコン

    m_pimgIcon = new AXImageList;
    m_pimgIcon->loadPNG(g_icon_file, sizeof(g_icon_file), 16, 0x00ff00);

    setImageList(m_pimgIcon);

    //

    _trgroup(TRSYS_GROUPID);

    //ヘッダ

    addColumn(_str(TRSYS_FILENAME), 230, 0);
    addColumn(_str(TRSYS_SIZE), 70, CF_RIGHT);
    addColumn(_str(TRSYS_MODIFIED_DATE), 120, 0);
}

//! 子ウィンドウからの通知を自身へ送る

AXWindow *AXFileListView::getNotify()
{
    return this;
}


//==============================
//
//==============================


//! ヘッダの各幅セット

void AXFileListView::setListColumnWidth(int namew,int sizew,int datew)
{
    setColumnWidth(0, namew);
    setColumnWidth(1, sizew);
    setColumnWidth(2, datew);
}

//! ディレクトリセット

void AXFileListView::setDirectory(const AXString &str)
{
    m_strDir = str;
    m_strDir.path_removeEndSlash();

    _setFileList();
}

//! ルートディレクトリにセット

void AXFileListView::setDirectoryRoot()
{
    m_strDir = '/';

    _setFileList();
}

//! ホームディレクトリにセット

void AXFileListView::setDirectoryHome()
{
    m_strDir.path_setHomePath();

    _setFileList();
}

//! フィルタ文字列セット
/*!
    ワイルドカード有効。複数指定する場合は ';' で区切る。NULL ですべて。
*/

void AXFileListView::setFilter(LPCUSTR pText)
{
    if(pText)
        m_strFilter = pText;
    else
        m_strFilter = '*';
}

//! 隠しファイルを隠すか

void AXFileListView::setHideDot(BOOL bHide)
{
    changeStyle(FLVS_HIDE_DOT, bHide);
    _setFileList();
}

//! ファイルリスト更新

void AXFileListView::updateList()
{
    _setFileList();
}

//! フォーカス位置のファイル名を取得
//! @return ファイルタイプ(AXFileListView::FILETYPE)

int AXFileListView::getFocusFileName(AXString *pstr,BOOL bFullPath)
{
    AXListViewItem *p = getFocusItem();
    AXString str;

    pstr->empty();

    if(!p) return FILETYPE_NONE;
    if(p->m_lParam == FLIST_BACK) return FILETYPE_NONE;

    //

    p->m_strText.getSplitPosStr(&str, 0, '\t');

    if(bFullPath)
    {
        *pstr = m_strDir;
        pstr->path_add(str);
    }
    else
        *pstr = str;

    return (p->m_lParam == FLIST_FILE)? FILETYPE_FILE: FILETYPE_DIR;
}

//! 複数選択時、複数ファイル名を取得
/*!
    ディレクトリ名\\tファイル名\\tファイル名...\\t

    @return ファイル数
*/

int AXFileListView::getMultiFileName(AXString *pstr)
{
    AXListViewItem *p;
    AXString str;
    int cnt = 0;

    *pstr = m_strDir;
    *pstr += '\t';

    for(p = getSelNext(); p; p = getSelNext(p))
    {
        if(p->m_lParam == FLIST_FILE)
        {
            p->m_strText.getSplitPosStr(&str, 0, '\t');

            *pstr += str;
            *pstr += '\t';

            cnt++;
        }
    }

    return cnt;
}


//==============================
//サブ
//==============================


//! ファイルリストセット

void AXFileListView::_setFileList()
{
    AXDir dir;
    AXString name,str;
    AXFILESTAT fs;
    time_t timet;
    struct tm *ptm;
    char m[32];

    deleteAllItem();

    //一つ戻る

    if(m_strDir != "/")
    {
        name = "..";
        addItem(name, FLIST_BACK, 0, FLIST_BACK);
    }

    //

    if(!dir.open(m_strDir)) return;

    while(dir.read())
    {
        if(!dir.getFileStat(&fs)) continue;

        name = dir.getCurName();

        if(fs.isDirectory())
        {
            //======= ディレクトリ

            if(dir.isSpecPath())
                continue;
            else
            {
                //通常ディレクトリ

                if((m_uStyle & FLVS_HIDE_DOT) && name[0] == '.')
                    continue;

                addItem(name, FLIST_DIR, 0, FLIST_DIR);
            }
        }
        else
        {
            //======== ファイル

            //隠しファイル

            if((m_uStyle & FLVS_HIDE_DOT) && name[0] == '.')
                continue;

            //ディレクトリのみ時

            if(m_uStyle & FLVS_ONLY_DIR) continue;

            //フィルタ比較

            if(!name.compareFilter(m_strFilter, ';', TRUE)) continue;

            //追加

            str.setFileSize(fs.lFileSize);

            name += '\t';
            name += str;
            name += '\t';

            timet = fs.timeModify;
            ptm = ::localtime(&timet);

            ::strftime(m, 31, "%Y/%m/%d %H:%M", ptm);
            name += m;

            addItem(name, FLIST_FILE, 0, FLIST_FILE);
        }
    }

    dir.close();

    //ソート

    sortItem(funcSort, 0);
}

//! ソート関数

int AXFileListView::funcSort(AXListItem *pitem1,AXListItem *pitem2,ULONG lParam)
{
    AXListViewItem *p1 = (AXListViewItem *)pitem1;
    AXListViewItem *p2 = (AXListViewItem *)pitem2;

    if(p1->m_lParam < p2->m_lParam)
        return -1;
    else if(p1->m_lParam > p2->m_lParam)
        return 1;
    else
        return p1->m_strText.compareCharEnd(p2->m_strText, '\t');
}

//! ダブルクリック処理

void AXFileListView::_dblclk()
{
    AXListViewItem *pi = getFocusItem();

    if(!pi) return;

    switch(pi->m_lParam)
    {
        //戻る
        case FLIST_BACK:
            m_strDir.path_removeFileName();
            if(m_strDir.isEmpty()) m_strDir = '/';

            _setFileList();

            m_pNotify->onNotify(this, FLVN_MOVEDIR, 0);
            break;
        //ディレクトリ
        case FLIST_DIR:
            m_strDir.path_add(pi->m_strText);

            _setFileList();

            m_pNotify->onNotify(this, FLVN_MOVEDIR, 0);
            break;
        //ファイル
        case FLIST_FILE:
            m_pNotify->onNotify(this, FLVN_DBLCLK_FILE, 0);
            break;
    }
}


//==============================
//ハンドラ
//==============================


//! 通知

BOOL AXFileListView::onNotify(AXWindow *pwin,UINT uNotify,ULONG lParam)
{
    if(AXListView::onNotify(pwin, uNotify, lParam))
        return TRUE;

    //

    if(pwin == this)
    {
        AXListViewItem *pi;

        switch(uNotify)
        {
            //フォーカス変更
            case LVN_CHANGEFOCUS:
                pi = getFocusItem();

                if(pi && pi->m_lParam == FLIST_FILE)
                    m_pNotify->onNotify(this, FLVN_SELFILE, 0);
                break;
            //ダブルクリック
            case LVN_DBLCLK:
                _dblclk();
                break;
        }
    }

    return TRUE;
}
