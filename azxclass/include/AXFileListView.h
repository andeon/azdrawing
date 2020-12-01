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

#ifndef _AX_FILELISTVIEW_H
#define _AX_FILELISTVIEW_H

#include "AXListView.h"
#include "AXString.h"

class AXImageList;

class AXFileListView:public AXListView
{
public:
    enum FILELISTVIEWSTYLE
    {
        FLVS_HIDE_DOT   = LVS_EXTRA,
        FLVS_ONLY_DIR   = LVS_EXTRA << 1
    };

    enum FILELISTVIEWNOTIFY
    {
        FLVN_SELFILE     = 100,
        FLVN_MOVEDIR     = 101,
        FLVN_DBLCLK_FILE = 102
    };

    enum FILETYPE
    {
        FILETYPE_NONE,
        FILETYPE_FILE,
        FILETYPE_DIR
    };

protected:
    AXString    m_strDir,
                m_strFilter;
    AXImageList *m_pimgIcon;

protected:
    void _createFileListView();
    void _setFileList();
    void _dblclk();

    static int funcSort(AXListItem *p1,AXListItem *p2,ULONG lParam);

public:
    virtual ~AXFileListView();

    AXFileListView(AXWindow *pParent,UINT uStyle,UINT uLayoutFlags);
    AXFileListView(AXWindow *pParent,UINT uStyle,UINT uLayoutFlags,UINT uItemID,DWORD dwPadding);

    virtual AXWindow *getNotify();

    const AXString& getNowDir() const { return m_strDir; }

    void setListColumnWidth(int namew,int sizew,int datew);
    void setDirectory(const AXString &str);
    void setDirectoryRoot();
    void setDirectoryHome();
    void setFilter(LPCUSTR pText);
    void setHideDot(BOOL bHide);
    void updateList();

    int getFocusFileName(AXString *pstr,BOOL bFullPath);
    int getMultiFileName(AXString *pstr);

    virtual BOOL onNotify(AXWindow *pwin,UINT uNotify,ULONG lParam);
};

#endif
