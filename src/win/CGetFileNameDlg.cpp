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

#include "CGetFileNameDlg.h"

#include "CConfig.h"

#include "AXLayout.h"
#include "AXLabel.h"
#include "AXLineEdit.h"
#include "AXComboBox.h"
#include "AXApp.h"

#include "defGlobal.h"
#include "defStrID.h"


/*!
    @class CGetFileNameDlg
    @brief ファイル名取得ダイアログ（カスタム）
*/


CGetFileNameDlg::CGetFileNameDlg(AXWindow *pOwner,int type,LPCUSTR pstrFilter,int deftype,
                LPCUSTR pstrInitDir,UINT uFlags,AXString *pstrRet,LPINT pRetType)
    : AXFileDialog(pOwner, type, pstrFilter, deftype, pstrInitDir, uFlags, pstrRet, pRetType)
{
    AXLayout *pl,*plv;

    _trgroup(STRGID_FILEDIALOG);

    getLayout()->addItem(plv = new AXLayoutVert(0, 4));
    plv->setPaddingTop(8);

    //PNG圧縮率

    plv->addItem(pl = new AXLayoutHorz(0, 8));

    pl->addItem(new AXLabel(this, 0, LF_CENTER_Y, 0, STRID_FILEDLG_PNGLEVEL));
    pl->addItem(m_peditPNG = new AXLineEdit(this, AXLineEdit::ES_SPIN, 0));

    m_peditPNG->setInit(5, 0, 9, g_pconf->btPNGLevel);

    //JPEG品質・サンプリング

    plv->addItem(pl = new AXLayoutHorz(0, 6));

    pl->addItem(new AXLabel(this, 0, LF_CENTER_Y, 0, STRID_FILEDLG_JPEGQUA));
    pl->addItem(m_peditJPEG = new AXLineEdit(this, AXLineEdit::ES_SPIN, LF_CENTER_Y));
    pl->addItem(new AXLabel(this, 0, LF_CENTER_Y, 0, STRID_FILEDLG_JPEGSAMP));
    pl->addItem(m_pcbJPEGSamp = new AXComboBox(this, 0, LF_CENTER_Y));

    m_peditJPEG->setInit(5, 1, 100, g_pconf->btJPEGQua);

    m_pcbJPEGSamp->addItem("4:4:4 (HI)", 444);
    m_pcbJPEGSamp->addItem("4:2:2", 422);
    m_pcbJPEGSamp->addItem("4:1:1 (LOW)", 411);
    m_pcbJPEGSamp->setAutoWidth();
    m_pcbJPEGSamp->setCurSel_findParam(g_pconf->wJPEGSamp);
}

//! （関数）ファイル保存

BOOL CGetFileNameDlg::getSaveFile(AXWindow *pOwner,LPCUSTR pstrFilter,int deftype,
              LPCUSTR pstrInitDir,UINT uFlags,AXString *pstrRet,LPINT pRetType)
{
    CGetFileNameDlg *pdlg;

    pdlg = new CGetFileNameDlg(pOwner, DTYPE_SAVE, pstrFilter, deftype, pstrInitDir, uFlags, pstrRet, pRetType);

    return pdlg->run(450, 450);
}

//! カスタム時の通知

BOOL CGetFileNameDlg::onNotifyFileDialog(int notify)
{
    switch(notify)
    {
        //決定・キャンセル
        case AXFileDialog::FDCN_OK:
        case AXFileDialog::FDCN_CANCEL:
            g_pconf->btPNGLevel = m_peditPNG->getVal();
            g_pconf->btJPEGQua  = m_peditJPEG->getVal();
            g_pconf->wJPEGSamp  = m_pcbJPEGSamp->getItemParam(-1);
            break;
    }

    return TRUE;
}
