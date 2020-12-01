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

#include <string.h>

#include "AXImage8.h"


/*!
    @class AXImage8
    @brief 独自8BITイメージ（パレット付き）

    - 横1列は4バイト単位。

    @ingroup draw
*/


AXImage8::AXImage8()
{
    m_pBuf = NULL;
    m_pPal = NULL;
    m_nWidth = m_nHeight = m_nPalCnt = 0;
}

AXImage8::~AXImage8()
{
    free();
}

//! 解放

void AXImage8::free()
{
    AXFree((void **)&m_pBuf);
    AXFree((void **)&m_pPal);

    m_nWidth = m_nHeight = m_nPalCnt = 0;
}

//! イメージ作成（パレットは作成しない）

BOOL AXImage8::create(int w,int h)
{
    free();

    m_nPitch = (w + 3) & (~3);

    m_pBuf = (LPBYTE)AXMalloc(m_nPitch * h);
    if(!m_pBuf) return FALSE;

    m_nWidth  = w;
    m_nHeight = h;

    return TRUE;
}

//! 作成（パレットも同時に）
/*!
    @param pPal パレットデータ（RGB）
    @param palcnt パレット数
*/

BOOL AXImage8::create(int w,int h,LPDWORD pPal,int palcnt)
{
    if(!pPal || palcnt <= 0) return FALSE;

    if(!create(w, h)) return FALSE;

    //パレット

    m_pPal = (LPDWORD)AXMalloc(palcnt * sizeof(DWORD));
    if(!m_pPal) return FALSE;

    ::memcpy(m_pPal, pPal, palcnt * sizeof(DWORD));

    m_nPalCnt = palcnt;

    return TRUE;
}



//===========================
//
//===========================


//! RGB値からパレット値を検索
//! @return -1で見つからなかった

int AXImage8::findPalFromRGB(DWORD col) const
{
    int i;

    col &= 0xffffff;

    for(i = 0; i < m_nPalCnt; i++)
    {
        if((m_pPal[i] & 0xffffff) == col) return i;
    }

    return -1;
}

//! クリア

void AXImage8::clear(int pal)
{
    DWORD c;
    int cnt = m_nPitch * m_nHeight >> 2;

    c = ((DWORD)pal << 24) | (pal << 16) | (pal << 8) | pal;

#ifdef _AX_OPT_ASM_X86
    ASMV(
        "rep stosl\n\t"
        ::"D"(m_pBuf),"a"(c),"c"(cnt)
    );
#else

    LPDWORD p = (LPDWORD)m_pBuf;

    for(; cnt > 0; cnt--)
        *(p++) = c;

#endif
}


