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

#ifndef _AZDRAW_GETFILENAMEDLG_H_
#define _AZDRAW_GETFILENAMEDLG_H_

#include "AXFileDialog.h"

class CGetFileNameDlg:public AXFileDialog
{
protected:
    AXLineEdit  *m_peditPNG,
                *m_peditJPEG;
    AXComboBox  *m_pcbJPEGSamp;

protected:
    void _init_ex();

public:
    CGetFileNameDlg(AXWindow *pOwner,int type,LPCUSTR pstrFilter,int deftype,LPCUSTR pstrInitDir,UINT uFlags,AXString *pstrRet,LPINT pRetType);

    virtual BOOL onNotifyFileDialog(int notify);

    static BOOL getSaveFile(AXWindow *pOwner,LPCUSTR pstrFilter,int deftype,LPCUSTR pstrInitDir,UINT uFlags,AXString *pstrRet,LPINT pRetType=NULL);
};

#endif
