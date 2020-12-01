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

#include <stdlib.h>
#include <pthread.h>

#include "AXThread.h"


//***********************************
// AXThread
//***********************************

/*!
    @class AXThread
    @brief スレッドクラス

    @ingroup thread
*/


AXThread::AXThread()
{
    m_pID = 0;
}

//! 開始
/*!
    @attention スレッド関数が終了してもメモリは解放されないので、必ず join() もしくはデタッチを行うこと
*/

int AXThread::create(void *(*func)(void *),LPVOID pParam)
{
    pthread_t id;
    int ret;

    ret = ::pthread_create(&id, NULL, func, pParam);

    m_pID = (LPVOID)id;

    return ret;
}

//! スレッドが終了するまで待ち、リソースを解放

BOOL AXThread::join()
{
    if(m_pID && ::pthread_join((pthread_t)m_pID, NULL) == 0)
    {
        m_pID = 0;
        return TRUE;
    }

    return FALSE;
}

//! スレッドを強制終了する

BOOL AXThread::cancel()
{
    pthread_t id = (pthread_t)m_pID;

    if(id && ::pthread_cancel(id) == 0)
    {
        //pthread_cancel() はスレッドが終了したかどうかに関わらずすぐ返るので、join で待つ

        ::pthread_join(id, NULL);

        m_pID = 0;

        return TRUE;
    }

    return FALSE;
}

//! スレッドをデタッチ状態にする
/*!
    スレッド終了時、リソースを解放させるようにする。@n
    create() 後にすぐに実行しておけば、join() で待つ必要がなくなる。@n
    ただし、デタッチ後は join() によって終了を待つことはできない。
*/

BOOL AXThread::detach()
{
    if(!m_pID) return FALSE;

    return (::pthread_detach((pthread_t)m_pID) == 0);
}


//***********************************
// AXMutex
//***********************************

/*!
    @class AXMutex
    @brief ミューテクスクラス

    - スレッドの同期に使う。
    - 同期させたい複数のスレッドで１つのミューテクスを使い、他のスレッドを実行させたくない部分を
      lock() 〜 unlock() ではさむ。
    - ロックされている間は、他のスレッドは実行されない。
    - ロックされた時に他のスレッドの実行が止まるわけではなく、ロックされている間に他のスレッドでロックが
      実行されたら、そのスレッドではロックが解除されるまで待たされる。

    @ingroup thread
*/


AXMutex::AXMutex()
{
    m_pDat = NULL;
}

AXMutex::~AXMutex()
{
    destroy();
}

//! 破棄

void AXMutex::destroy()
{
    if(m_pDat)
    {
        ::pthread_mutex_destroy((pthread_mutex_t *)m_pDat);
        delete (pthread_mutex_t *)m_pDat;

        m_pDat = NULL;
    }
}

//! 初期化

void AXMutex::init()
{
    if(!m_pDat)
    {
        m_pDat = new pthread_mutex_t;

        ::pthread_mutex_init((pthread_mutex_t *)m_pDat, NULL);
    }
}

//! 再帰ありで初期化

void AXMutex::initRecursive()
{
    if(!m_pDat)
    {
        pthread_mutexattr_t mutex;

        m_pDat = new pthread_mutex_t;

        ::pthread_mutexattr_init(&mutex);
        ::pthread_mutexattr_settype(&mutex, PTHREAD_MUTEX_RECURSIVE_NP);
        ::pthread_mutex_init((pthread_mutex_t *)m_pDat, &mutex);
        ::pthread_mutexattr_destroy(&mutex);    //※linuxでは何もしない
    }
}

//! ロックする

void AXMutex::lock()
{
    if(m_pDat)
        ::pthread_mutex_lock((pthread_mutex_t *)m_pDat);
}

//! アンロック

void AXMutex::unlock()
{
    if(m_pDat)
        ::pthread_mutex_unlock((pthread_mutex_t *)m_pDat);
}
