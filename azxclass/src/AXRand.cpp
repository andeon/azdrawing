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
#include <limits.h>

#include "AXRand.h"


/*!
    @class AXRand
    @brief 乱数生成クラス (XorShift)

    @ingroup etc
*/


AXRand::AXRand()
{
    init(::time(NULL));
}

//! 初期化

void AXRand::init(unsigned int seed)
{
    m_x = 1812433253U * (seed ^ (seed >> 30)) + 1;
    m_y = 1812433253U * (m_x ^ (m_x >> 30)) + 2;
    m_z = 1812433253U * (m_y ^ (m_y >> 30)) + 3;
    m_w = 1812433253U * (m_z ^ (m_z >> 30)) + 4;
}

//! DWORD値取得

unsigned int AXRand::getDWORD()
{
    unsigned int t;

    t   = m_x ^ (m_x << 11);
    m_x = m_y;
    m_y = m_z;
    m_z = m_w;
    m_w = (m_w ^ (m_w >> 19)) ^ (t ^ (t >> 8));

    return m_w;
}

//! 範囲指定取得

int AXRand::getRange(int min,int max)
{
    if(min == max)
        return min;
    else
        return (int)((double)getDWORD() / (UINT_MAX + 1.0) * (max - min + 1)) + min;
}

//! double(0以上1未満)取得

double AXRand::getDouble()
{
    return (double)getDWORD() / (UINT_MAX + 1.0);
}
