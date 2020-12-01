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

#ifndef _AZDRAW_SELIMGDLG_H_
#define _AZDRAW_SELIMGDLG_H_

#include "AXDialog.h"
#include "AXString.h"

class AXListBox;
class CImgPrev;
class CImage8;

class CSelImgDlg:public AXDialog
{
public:
    enum
    {
        TYPE_OPT_TEXTURE,
        TYPE_BRUSH_TEXTURE,
        TYPE_BRUSH_BRUSH
    };

protected:
    int         m_nType;
    AXString    m_strPath;   //!< ルートディレクトリ以下のパス（空でルート）

    AXListBox   *m_pList;
    CImgPrev    *m_pPrev;

protected:
    void _setInitDir(const AXString &strDefName);
    void _changeSel();
    void _dblClk();
    void _getRootDir(AXString *pStr,BOOL bUser);
    void _setList(BOOL bUserDir);
    BOOL _setPreview(const AXString &path,BOOL bUserDir);
    void _searchFile(const AXString &strDir,BOOL bUserDir);

public:
    CSelImgDlg(AXWindow *pOwner,int type,const AXString &strDefName);

    BOOL run(AXString *pstrPath);

    virtual BOOL onNotify(AXWindow *pwin,UINT uNotify,ULONG lParam);
};

#endif
