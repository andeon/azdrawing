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

#ifndef _AX_FILEDIALOG_H
#define _AX_FILEDIALOG_H

#include "AXDialog.h"
#include "AXString.h"

class AXFileListView;
class AXLineEdit;
class AXComboBox;

class AXFileDialog:public AXDialog
{
public:
    enum FILEDIALOGFLAGS
    {
        FDF_MULTISEL         = 1,
        FDF_SHOW_DOT         = 2,
        FDF_NO_OVERWRITE_MES = 4,
        FDF_CUSTOM_HORZ      = 8
    };

    enum FILEDIALOGCUSTOMNOTIFY
    {
        FDCN_OK,
        FDCN_CANCEL,
        FDCN_SELFILE
    };

protected:
    AXFileListView  *m_pList;
    AXLineEdit      *m_peditDir,
                    *m_peditName;
    AXComboBox      *m_pcbType;

    int         m_nDlgType;
    UINT        m_uDlgFlags;
    AXString    m_strFilter;
    LPINT       m_pRetType;
    AXString    *m_pstrRet;

    enum FILEDIALOGTYPE
    {
        DTYPE_OPEN,
        DTYPE_SAVE,
        DTYPE_DIR
    };

protected:
    void _init(LPCUSTR pstrInitDir,int deftype);
    void _initWin();
    void _changeDir();
    void _changeSelFile();
    void _setTypeCombo(int defno);
    void _end(BOOL bOK);

public:
    virtual ~AXFileDialog();
    AXFileDialog(AXWindow *pOwner,int type,LPCUSTR pstrFilter,int deftype,LPCUSTR pstrInitDir,UINT uFlags,AXString *pstrRet,LPINT pRetType);

    BOOL run(int w,int h);

    virtual BOOL onNotifyFileDialog(int notify);
    virtual BOOL onClose();
    virtual BOOL onNotify(AXWindow *pwin,UINT uNotify,ULONG lParam);

    //

    static BOOL openFile(AXWindow *pOwner,LPCUSTR pstrFilter,int deftype,LPCUSTR pstrInitDir,UINT uFlags,AXString *pstrRet);
    static BOOL saveFile(AXWindow *pOwner,LPCUSTR pstrFilter,int deftype,LPCUSTR pstrInitDir,UINT uFlags,AXString *pstrRet,LPINT pRetType=NULL);
    static BOOL openDir(AXWindow *pOwner,LPCUSTR pstrInitDir,UINT uFlags,AXString *pstrRet);
};

#endif
