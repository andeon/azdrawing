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

#define _AX_X11_XI2
#include "AXX11.h"

#include <stdlib.h>

#include "CApp.h"

#include "CDevList.h"
#include "CConfig.h"
#include "CDraw.h"
#include "CCanvasWin.h"
#include "defGlobal.h"

#include "AXCursor.h"
#include "AXUtilX.h"
#include "AXUtilFile.h"


//-------------------------

#define DISP    ((Display *)m_pDisp)

//-------------------------

/*!
    @class CApp
    @brief アプリケーションクラス（XInput2対応）
*/


CApp::CApp()
{
    m_nGrabDevID = 0;
    m_XIdbcWin  = 0;
    m_XIdbcTime = 0;
    m_XIdbcX = m_XIdbcY = 0;
    m_dLastPressure = 0;
}

//! 初期化2

void CApp::init2()
{
    char path[] = "/tmp/azdw_XXXXXX";

    //一時ディレクトリ作成

    if(::mkdtemp(path))
        m_strTmpDir = path;
}

//! 終了

void CApp::end2()
{
    //一時ディレクトリ削除

    if(m_strTmpDir.isNoEmpty())
        AXDeleteDir(m_strTmpDir);
}

//! XI、通常のグラブ共通解除

void CApp::ungrabBoth()
{
#ifdef _AX_OPT_XI2
    ungrabXI();
#endif

    CANVASAREA->ungrabPointer();
}

//! XI初期化

void CApp::initXI(AXWindow *pwin)
{
#ifdef _AX_OPT_XI2

    XIEventMask em;
    BYTE mask[1] = {0};

    em.deviceid = 2;
    em.mask_len = sizeof(mask);
    em.mask     = mask;

    XISetMask(mask, XI_ButtonPress);
    XISetMask(mask, XI_ButtonRelease);
    XISetMask(mask, XI_Motion);

    XISelectEvents(DISP, pwin->getid(), &em, 1);

#endif
}

//! 設定読込前に、現在存在するデバイスの情報をセット

void CApp::setDevice(CDevList *pDev)
{
#ifdef _AX_OPT_XI2

    XIDeviceInfo *pinfo;
    XIAnyClassInfo *pclass;
    XIValuatorClassInfo *pvc;
    int cnt,i,j,pressno,bttnum;
    double pressmin,pressmax;
    AXString strDevName;
    Atom label_abs_pressure;

    pinfo = XIQueryDevice(DISP, XIAllDevices, &cnt);
    if(!pinfo) return;

    label_abs_pressure = getAtom("Abs Pressure");

    for(i = 0; i < cnt; i++)
    {
        strDevName.setLocal(pinfo[i].name);

        //ポインタデバイスのみ

        if(pinfo[i].use != XIMasterPointer && pinfo[i].use != XISlavePointer)
            continue;

        //マスタポインタの場合、名前を空に

        if(pinfo[i].use == XIMasterPointer)
            strDevName.empty();

        //スレイブの仮想デバイスは除く

        if(pinfo[i].use == XISlavePointer && strDevName.compareMatch("Virtual core*", TRUE))
            continue;

        //ボタンや筆圧の情報

        bttnum   = 0;
        pressno  = CDevItem::PRESS_NONE;
        pressmin = pressmax = 1;

        for(j = 0; j < pinfo[i].num_classes; j++)
        {
            pclass = pinfo[i].classes[j];

            if(pclass->type == XIButtonClass)
            {
                //ボタン数

                bttnum = ((XIButtonClassInfo *)pclass)->num_buttons;

                if(bttnum > CDevItem::BUTTON_MAXCNT)
                    bttnum = CDevItem::BUTTON_MAXCNT;
            }
            else if(pclass->type == XIValuatorClass)
            {
                //筆圧

                pvc = (XIValuatorClassInfo *)pclass;

                if(pvc->label == label_abs_pressure)
                {
					pressno  = pvc->number;
					pressmin = pvc->min;
					pressmax = pvc->max;
                }
            }
        }

        //

        pDev->addDeviceInit(pinfo[i].deviceid, strDevName, bttnum, pressno, pressmin, pressmax);
    }

    XIFreeDeviceInfo(pinfo);

#else

    AXString str;

    pDev->addDeviceInit(2, str, CDevItem::PRESS_NONE, 5, 1, 1);

#endif
}


//================================


//! グラブ中か

BOOL CApp::isGrab()
{
    return (AXApp::isGrab() || m_nGrabDevID);
}

//! グラブ

void CApp::grabXI(ULONG idWin,int srcid)
{
#ifdef _AX_OPT_XI2

    if(m_nGrabDevID) return;

    XIEventMask em;
    BYTE mask[1] = {0};

    em.deviceid = 2;
    em.mask_len = 1;
    em.mask     = mask;

    XISetMask(mask, XI_ButtonPress);
    XISetMask(mask, XI_ButtonRelease);
    XISetMask(mask, XI_Motion);

    XIGrabDevice(DISP, 2, idWin, CurrentTime, 0,
                 GrabModeAsync, GrabModeAsync, FALSE, &em);

    //

    m_nGrabDevID = srcid;

#endif
}

//! グラブ解放

void CApp::ungrabXI()
{
#ifdef _AX_OPT_XI2

    if(m_nGrabDevID)
    {
        XIUngrabDevice(DISP, 2, CurrentTime);
        m_nGrabDevID = 0;

        AXApp::sync();
    }

#endif
}

//! 筆圧取得

double CApp::_getPress(LPVOID pEvent)
{
#ifdef _AX_OPT_XI2

    XIDeviceEvent *pev = (XIDeviceEvent *)pEvent;
    CDevItem *pdi;
    int i,pos,valpos;
    double press = 1;

    pdi = (g_pconf->pDev)->getDevFromID(pev->sourceid);

    if(pdi && pdi->getPressNo() != CDevItem::PRESS_NONE)
    {
        valpos = pdi->getPressNo();

        /* 筆圧の値がない時は最後の値。
         * デバイスによっては、前回の値と同じ場合は valuators で値が来ない場合がある */

        if(!XIMaskIsSet(pev->valuators.mask, valpos))
			press = m_dLastPressure;
		else
		{
			for(i = pos = 0; i < valpos; i++)
			{
				if(XIMaskIsSet(pev->valuators.mask, i)) pos++;
			}

			press = pdi->getPress(pev->valuators.values[pos]);
		}
    }

    return press;

#else
    return 1;
#endif
}


//============================
// イベント
//============================


//! XInput2イベント

void CApp::onInputEvent(AXWindow *pwin,LPVOID pEvent,BOOL bSkip)
{
#ifdef _AX_OPT_XI2

    XIDeviceEvent *pev = (XIDeviceEvent *)pEvent;
    CDraw::DRAWPOINT pt;
    UINT btt;

    //ダイアログ表示中

    if(bSkip)
    {
        //左クリック

        if(pev->evtype == XI_ButtonPress && pev->detail == 1)
            g_pdraw->onLDownInDlg((int)pev->event_x, (int)pev->event_y);

        return;
    }

    //

    switch(pev->evtype)
    {
        case XI_ButtonPress:
        case XI_ButtonRelease:
        case XI_Motion:
            /* グラブ時は、押された時のデバイスと異なる場合は無視。
               マスタデバイスをグラブしているので、グラブ中もソースデバイスは変化するため */

            if(m_nGrabDevID && m_nGrabDevID != pev->sourceid) return;

            //位置と筆圧

            pt.x = pev->event_x;
            pt.y = pev->event_y;
            pt.press = _getPress(pev);

            //筆圧は常に記録しておく

            m_dLastPressure = pt.press;

            //

            if(pev->evtype == XI_Motion)
                //移動時
                g_pdraw->onMove(pt, pev->mods.base & ControlMask, pev->mods.base & ShiftMask);
            else if(pev->evtype == XI_ButtonRelease)
            {
                //離し時

                if(g_pdraw->onUp(pt, pev->detail))
                    ungrabXI();
            }
            else if(pev->evtype == XI_ButtonPress)
            {
                //押し時

                //-------- 左ダブルクリック判定

                if(pev->event == m_XIdbcWin && pev->detail == 1 && pev->time < m_XIdbcTime + 350 &&
                        ::abs((int)pev->root_x - m_XIdbcX) < 3 && ::abs((int)pev->root_y - m_XIdbcY) < 3)
                {
                    m_XIdbcWin = 0;

                    if(g_pdraw->onLDblClk())
                    {
                        ungrabXI();
                        break;
                    }
                }

                m_XIdbcWin  = pev->event;
                m_XIdbcTime = pev->time;
                m_XIdbcX    = (int)pev->root_x;
                m_XIdbcY    = (int)pev->root_y;

                //--------- 通常クリック

                btt = (g_pconf->pDev)->getButtonAction(pev->sourceid, pev->detail,
                                pev->mods.base & ControlMask, pev->mods.base & ShiftMask, pev->mods.base & Mod1Mask);

                if(g_pdraw->onDown(pt, btt))
                    grabXI(pev->event, pev->sourceid);
            }

            break;
    }

#endif
}

