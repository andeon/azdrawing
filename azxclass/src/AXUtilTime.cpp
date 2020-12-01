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

#include "AXUtilTime.h"

/*!
    @defgroup axutiltime AXUtilTime
    @brief 時間関連ユーティリティ

    ※ librt のリンクが必要 (-lrt)

    @ingroup util
    @{
*/


//! 時間取得

void AXGetTime(TIMENANO *pdst)
{
    struct timespec ts;

    ::clock_gettime(CLOCK_MONOTONIC, &ts);

    pdst->sec   = ts.tv_sec;
    pdst->nsec  = ts.tv_nsec;
}

//! 時間に指定ナノ秒分追加

void AXAddTime(TIMENANO *psrc,ULONGLONG nanosec)
{
    nanosec += psrc->nsec;

    psrc->sec   += nanosec / (1000 * 1000 * 1000);
    psrc->nsec  = nanosec % (1000 * 1000 * 1000);
}

//! 時間の差分 (p1 - p2) を pdst に取得
/*!
    ※p1の方が値が大きい場合のみ
    @return FALSEでp2の方が小さい
*/

BOOL AXDiffTime(TIMENANO *pdst,const TIMENANO *p1,const TIMENANO *p2)
{
    if(AXCompareTime(p1, p2) < 0)
        return FALSE;
    else
    {
        pdst->sec = p1->sec - p2->sec;

        if(p1->nsec >= p2->nsec)
            pdst->nsec = p1->nsec - p2->nsec;
        else
        {
            pdst->sec--;
            pdst->nsec = (int)((ULONGLONG)p1->nsec + 1000 * 1000 * 1000 - p2->nsec);
        }

        return TRUE;
    }
}

//! 時間比較
/*!
    @return [0] 同じ [-1] p1がp2より小さい [1] p1がp2より大きい
*/

int AXCompareTime(const TIMENANO *p1,const TIMENANO *p2)
{
    if(p1->sec < p2->sec)
        return -1;
    else if(p1->sec > p2->sec)
        return 1;
    else
    {
        //秒が同じ

        if(p1->nsec < p2->nsec)
            return -1;
        else if(p1->nsec > p2->nsec)
            return 1;
        else
            return 0;
    }
}

//@}
