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

#include <unistd.h>

#include "CProgressDlg.h"

#include "AXLayout.h"
#include "AXProgressBar.h"
#include "AXThread.h"
#include "AXApp.h"


/*!
    @class CProgressDlg
    @brief 進行状況ダイアログ
*/


CProgressDlg::CProgressDlg(AXWindow *pOwner,void *(*func)(void *),LPVOID pParam1,LPVOID pParam2)
    : AXDialog(pOwner, WS_HIDE | WS_TITLE | WS_BORDER | WS_TRANSIENT_FOR | WS_NOT_TAKEFOCUS | WS_DISABLE_IM | WS_BK_FACE)
{
    AXLayout *pl;

    m_func    = func;
    m_pParam1 = pParam1;
    m_pParam2 = pParam2;

    //

    setTitle("progress...");

    setLayout(pl = new AXLayoutVert);

    pl->setSpacing(2);

    pl->addItem(m_pProg = new AXProgressBar(this, AXProgressBar::PBS_SIMPLE, LF_EXPAND_W));

    //

    calcDefSize();
    resize(250, m_nDefH);

    show();
    moveCenter();
}

//! 閉じる（無効）

BOOL CProgressDlg::onClose()
{
    return TRUE;
}

//! 実行

int CProgressDlg::run()
{
    AXThread thread;
    int ret;

    thread.create(m_func, this);

    axapp->runDlg(this);

    thread.join();

    ret = getResult();

    delete this;
    axapp->sync();

    return ret;
}

//! スレッド終了

void CProgressDlg::endThread(int ret)
{
    axapp->mutexLock();
    endDialog(ret);
    axapp->mutexUnlock();

    axapp->selectOut();

    ::usleep(50000);
}

//! プログレス最大値セット

void CProgressDlg::setProgMax(UINT max)
{
    axapp->mutexLock();
    m_pProg->setStatus(0, max, 0);
    axapp->mutexUnlock();
}

//! プログレス最大値セット（スレッド外時）

void CProgressDlg::setProgMaxOutThread(UINT max)
{
    m_pProg->setStatus(0, max, 0);
}

//! プログレス値+1

void CProgressDlg::incProg()
{
    axapp->mutexLock();
    m_pProg->incPos();
    axapp->mutexUnlock();
    axapp->selectOut();
}

//! プログレス値セット

void CProgressDlg::setProgPos(int pos)
{
    axapp->mutexLock();
    m_pProg->setPos(pos);
    axapp->mutexUnlock();
    axapp->selectOut();
}

//! プログレス位置加算

void CProgressDlg::addProgPos(int add)
{
    axapp->mutexLock();

    m_pProg->setPos(m_pProg->getPos() + add);

    axapp->mutexUnlock();
    axapp->selectOut();
}

//! プログレスサブ処理開始

void CProgressDlg::beginProgSub(int step,int max,BOOL bOneStep)
{
    axapp->mutexLock();

    if(bOneStep)
        m_pProg->setStatus(0, step, 0);

    m_nProgStep   = step;
    m_nProgMax    = max - 1;
    m_nProgTopPos = m_pProg->getPos();

    m_nProgPos    = 0;
    m_nProgSubPos = 0;
    m_nProgNext   = m_nProgMax / step;

    axapp->mutexUnlock();
}

//! プログレスサブ処理 +1

void CProgressDlg::incProgSub()
{
    axapp->mutexLock();

    m_nProgSubPos++;

    if(m_nProgSubPos > m_nProgMax)
    {
        //最後の場合

        m_pProg->setPos(m_nProgTopPos + m_nProgStep);
        axapp->selectOut();
    }
    else if(m_nProgMax < m_nProgStep * 2)
    {
        //最大値がステップ数よりある程度少ない場合

        m_pProg->setPos(m_nProgTopPos + m_nProgSubPos * m_nProgStep / m_nProgMax);
        axapp->selectOut();
    }
    else if(m_nProgSubPos >= m_nProgNext)
    {
        //最大値が大きい場合

        m_nProgPos++;

        m_pProg->setPos(m_nProgTopPos + m_nProgPos);

        m_nProgNext = (m_nProgPos + 1) * m_nProgMax / m_nProgStep;

        axapp->selectOut();
    }

    axapp->mutexUnlock();
}
