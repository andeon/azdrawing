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

#include "CNewDlg.h"

#include "CConfig.h"

#include "AXLayout.h"
#include "AXSpacerItem.h"
#include "AXLabel.h"
#include "AXLineEdit.h"
#include "AXButton.h"
#include "AXMenu.h"
#include "AXApp.h"
#include "AXUtilStr.h"

#include "defGlobal.h"
#include "defStrID.h"


//-----------------------

#define WID_EDIT_WPX    100
#define WID_EDIT_HPX    101
#define WID_EDIT_WCM    102
#define WID_EDIT_HCM    103
#define WID_EDIT_DPI    104
#define WID_REGLIST     110
#define WID_RECENTLIST  111
#define WID_DEFINEDLIST 112

//-----------------------

/*!
    @class CNewDlg
    @brief 新規作成ダイアログ
*/


CNewDlg::CNewDlg(AXWindow *pOwner)
    : AXDialog(pOwner, WS_DIALOG_NORMAL | WS_BK_FACE)
{
    AXLayout *plTop,*plh,*pl;

    _trgroup(STRGID_DLG_NEW);

    setTitle(STRID_NEWDLG_TITLE);

    setLayout(plTop = new AXLayoutVert);
    plTop->setSpacing(8);

    plTop->addItem(plh = new AXLayoutHorz(0, 15));

    //------- 値

    plh->addItem(pl = new AXLayoutMatrix(3));

    pl->addItem(new AXSpacerItem);
    pl->addItem(new AXLabel(this, 0, LF_CENTER_X, MAKE_DW4(0,0,6,6), _string(STRGID_WORD, STRID_WORD_WIDTH)));
    pl->addItem(new AXLabel(this, 0, LF_CENTER_X, MAKE_DW4(0,0,0,6), _string(STRGID_WORD, STRID_WORD_HEIGHT)));

    pl->addItem(new AXLabel(this, 0, LF_RIGHT|LF_CENTER_Y, MAKE_DW4(0,0,6,6), "px"));
    pl->addItem(m_peditWpx = new AXLineEdit(this, AXLineEdit::ES_SPIN, 0, WID_EDIT_WPX, MAKE_DW4(0,0,6,6)));
    pl->addItem(m_peditHpx = new AXLineEdit(this, AXLineEdit::ES_SPIN, 0, WID_EDIT_HPX, MAKE_DW4(0,0,0,6)));

    pl->addItem(new AXLabel(this, 0, LF_RIGHT|LF_CENTER_Y, MAKE_DW4(0,0,6,6), "cm"));
    pl->addItem(m_peditWcm = new AXLineEdit(this, AXLineEdit::ES_SPIN, 0, WID_EDIT_WCM, MAKE_DW4(0,0,6,6)));
    pl->addItem(m_peditHcm = new AXLineEdit(this, AXLineEdit::ES_SPIN, 0, WID_EDIT_HCM, MAKE_DW4(0,0,0,6)));

    pl->addItem(new AXLabel(this, 0, LF_RIGHT|LF_CENTER_Y, MAKE_DW4(0,0,6,0), "DPI"));
    pl->addItem(m_peditDPI = new AXLineEdit(this, AXLineEdit::ES_SPIN, 0, WID_EDIT_DPI, 0));

    m_peditWpx->setInit(7, 1, 9999, g_pconf->dwNewRecentSize[0] >> 16);
    m_peditHpx->setInit(7, 1, 9999, g_pconf->dwNewRecentSize[0] & 0xffff);
    m_peditDPI->setInit(7, 1, 2000, g_pconf->wNewRecentDPI[0]);
    m_peditWcm->setWidthFromLen(7);
    m_peditHcm->setWidthFromLen(7);
    m_peditWcm->setValStatus(1, 100000, 2);
    m_peditHcm->setValStatus(1, 100000, 2);

    //px と DPI から cm セット
    _changeVal(0);
    _changeVal(1);

    //-------- リスト

    plh->addItem(pl = new AXLayoutVert(0, 2));
    pl->setPaddingTop(15);

    pl->addItem(new AXButton(this, 0, LF_EXPAND_W, WID_REGLIST, 0, STRID_NEWDLG_REGIST));
    pl->addItem(new AXButton(this, 0, LF_EXPAND_W, WID_RECENTLIST, 0, STRID_NEWDLG_RECENT));
    pl->addItem(new AXButton(this, 0, LF_EXPAND_W, WID_DEFINEDLIST, 0, STRID_NEWDLG_DEFINED));

    //-------

    plTop->addItem(pl = createOKCancelButton());
    pl->setPaddingTop(15);

    //

    calcDefSize();
    resize(m_nDefW, m_nDefH);

    show();
}

//! 通知

BOOL CNewDlg::onNotify(AXWindow *pwin,UINT uNotify,ULONG lParam)
{
    UINT id = pwin->getItemID();

    switch(id)
    {
        //値を連動
        case WID_EDIT_WPX:
        case WID_EDIT_HPX:
        case WID_EDIT_WCM:
        case WID_EDIT_HCM:
            _changeVal(id - WID_EDIT_WPX);
            break;
        //DPI
        case WID_EDIT_DPI:
            _changeVal(2);
            _changeVal(3);
            break;

        //登録リスト
        case WID_REGLIST:
            _registList();
            break;
        //履歴リスト
        case WID_RECENTLIST:
            _recentList();
            break;
        //規定リスト
        case WID_DEFINEDLIST:
            _definedList();
            break;

        //OK
        case 1:
            //履歴の先頭にセットさせる

            g_pconf->addNewRecent(m_peditWpx->getVal(),
                                  m_peditHpx->getVal(),
                                  m_peditDPI->getVal());

            endDialog(TRUE);
            break;
        //キャンセル
        case 2:
            endDialog(FALSE);
            break;
    }

    return TRUE;
}


//============================
//
//============================


//! 値変更時

void CNewDlg::_changeVal(int no)
{
    double dpi;

    dpi = m_peditDPI->getVal();

    switch(no)
    {
        //px(W) -> cm(W)
        case 0:
            m_peditWcm->setVal((int)(m_peditWpx->getVal() * 2.54 * 100 / dpi + 0.5));
            break;
        //px(H) -> cm(H)
        case 1:
            m_peditHcm->setVal((int)(m_peditHpx->getVal() * 2.54 * 100 / dpi + 0.5));
            break;
        //cm(W) -> px(W)
        case 2:
            m_peditWpx->setVal((int)(m_peditWcm->getVal() * dpi / (2.54 * 100) + 0.5));
            break;
        //cm(H) -> px(H)
        case 3:
            m_peditHpx->setVal((int)(m_peditHcm->getVal() * dpi / (2.54 * 100) + 0.5));
            break;
    }
}

//! pxサイズとDPI値をセット

void CNewDlg::_setPXandDPI(int w,int h,int dpi)
{
    m_peditWpx->setVal(w);
    m_peditHpx->setVal(h);
    m_peditDPI->setVal(dpi);

    _changeVal(0);
    _changeVal(1);
}

//! 履歴リストメニュー

void CNewDlg::_recentList()
{
    AXMenu *pm;
    int i;
    AXString str;
    AXRectSize rcs;

    pm = new AXMenu;

    for(i = 0; i < CConfig::NEWRECENT_NUM; i++)
    {
        str.setInt(g_pconf->dwNewRecentSize[i] >> 16);
        str += 'x';
        str += (int)(g_pconf->dwNewRecentSize[i] & 0xffff);
        str += "px  ";
        str += (int)g_pconf->wNewRecentDPI[i];
        str += "DPI";

        pm->add(i, str);
    }

    getWidget(WID_RECENTLIST)->getWindowRect(&rcs);

    i = (int)pm->popup(NULL, rcs.x, rcs.y + rcs.h, 0);

    delete pm;

    //

    if(i != -1)
    {
        _setPXandDPI(g_pconf->dwNewRecentSize[i] >> 16,
                     g_pconf->dwNewRecentSize[i] & 0xffff,
                     g_pconf->wNewRecentDPI[i]);
    }
}

//! 登録リスト

void CNewDlg::_registList()
{
    AXMenu *pm;
    int i;
    AXString str;
    AXRectSize rcs;

    pm = new AXMenu;

    for(i = 0; i < CConfig::NEWREG_NUM; i++)
    {
        str = '[';
        str += (i + 1);
        str += "] ";
        str += (int)(g_pconf->dwNewRegSize[i] >> 16);
        str += 'x';
        str += (int)(g_pconf->dwNewRegSize[i] & 0xffff);
        str += "px ";
        str += (int)g_pconf->wNewRegDPI[i];
        str += "DPI";

        pm->add(i, str);
    }

    pm->addSep();

    for(i = 0; i < CConfig::NEWREG_NUM; i++)
    {
        str = "set -> [";
        str += (i + 1);
        str += ']';

        pm->add(1000 + i, str);
    }

    //

    getWidget(WID_REGLIST)->getWindowRect(&rcs);

    i = (int)pm->popup(NULL, rcs.x, rcs.y + rcs.h, 0);

    delete pm;

    //

    if(i != -1)
    {
        if(i < 1000)
        {
            //呼び出し

            _setPXandDPI(g_pconf->dwNewRegSize[i] >> 16,
                         g_pconf->dwNewRegSize[i] & 0xffff,
                         g_pconf->wNewRegDPI[i]);
        }
        else
        {
            //登録

            i -= 1000;

            g_pconf->dwNewRegSize[i] = ((DWORD)m_peditWpx->getVal() << 16) | m_peditHpx->getVal();
            g_pconf->wNewRegDPI[i]   = m_peditDPI->getVal();
        }
    }
}

//! 規定リスト

void CNewDlg::_definedList()
{
    AXMenu *pm;
    int i;
    AXString str;
    AXRectSize rcs;
    DWORD size;
    //cm単位。1.0 = 100
    DWORD a[7] = {
        (5940 << 16) | 8410, (4200 << 16) | 5940, (2970 << 16) | 4200,
        (2100 << 16) | 2970, (1480 << 16) | 2100, (1050 << 16) | 1480,
        (740 << 16) | 1050
    };
    DWORD b[7] = {
        (7280 << 16) | 10300, (5150 << 16) | 7280, (3640 << 16) | 5150,
        (2570 << 16) | 3640, (1820 << 16) | 2570, (1280 << 16) | 1820,
        (910 << 16) | 1280
    };

    //

    pm = new AXMenu;

    for(i = 0; i < 7; i++)
    {
        _setDefinedName(&str, 'A', i + 1, a[i]);
        pm->add(i, str);
    }

    pm->addSep();

    for(i = 0; i < 7; i++)
    {
        _setDefinedName(&str, 'B', i + 1, b[i]);
        pm->add(1000 + i, str);
    }

    //

    getWidget(WID_DEFINEDLIST)->getWindowRect(&rcs);

    i = (int)pm->popup(NULL, rcs.x, rcs.y + rcs.h, 0);

    delete pm;

    //

    if(i != -1)
    {
        if(i < 1000)
            size = a[i];
        else
            size = b[i - 1000];

        m_peditWcm->setVal(size >> 16);
        m_peditHcm->setVal(size & 0xffff);
        _changeVal(2);
        _changeVal(3);
    }
}

//! 規定名セット

void CNewDlg::_setDefinedName(AXString *pstr,char c,int no,DWORD size)
{
    char m[12];

    *pstr = c;
    *pstr += no;
    *pstr += " (";
    AXIntToFloatStr(m, (size >> 16) / 10, 1);
    *pstr += m;
    *pstr += 'x';
    AXIntToFloatStr(m, (size & 0xffff) / 10, 1);
    *pstr += m;
    *pstr += "cm)";
}
