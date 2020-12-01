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
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#include "AXDef.h"


//! stderr にエラー表示

void AXError(LPCSTR szFormat,...)
{
    va_list arg;

    ::va_start(arg, szFormat);
    ::vfprintf(stderr, szFormat, arg);
    ::fflush(stderr);
    ::va_end(arg);
}

//! メモリをゼロクリア

void AXMemZero(void *pBuf,DWORD size)
{
    ::memset(pBuf, 0, size);
}

//! メモリ確保

void *AXMalloc(DWORD dwSize)
{
    return ::malloc(dwSize);
}

//! 再確保

BOOL AXRealloc(void **ppBuf,DWORD dwSize)
{
    LPVOID pNew;

    pNew = ::realloc(*ppBuf, dwSize);
    if(!pNew) return FALSE;

    *ppBuf = pNew;

    return TRUE;
}

//! メモリ解放
/*!
    解放するだけ
*/

void AXFreeNormal(void *pBuf)
{
    if(pBuf) ::free(pBuf);
}

//! メモリ解放
/*!
    解放後、ポインタには NULL が入る
*/

void AXFree(void **ppBuf)
{
    if(*ppBuf)
    {
        ::free(*ppBuf);

        *ppBuf = NULL;
    }
}

