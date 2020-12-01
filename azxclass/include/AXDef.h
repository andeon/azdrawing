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

#ifndef _AX_DEF_H
#define _AX_DEF_H

typedef int             BOOL;
typedef unsigned char   BYTE;
typedef unsigned short  UNICHAR;
typedef wchar_t         WCHAR;
typedef unsigned short  WORD;
typedef unsigned int    DWORD;
typedef unsigned int    UINT;
typedef unsigned long   ULONG;
typedef long long int   LONGLONG;
typedef unsigned long long  ULONGLONG;

typedef void*               LPVOID;
typedef char*               LPSTR;
typedef const char*         LPCSTR;
typedef unsigned short*     LPUSTR;
typedef const unsigned short*   LPCUSTR;
typedef unsigned char*      LPBYTE;
typedef int*                LPINT;
typedef unsigned short*     LPWORD;
typedef unsigned int*       LPDWORD;
typedef wchar_t*            LPWSTR;
typedef const wchar_t*      LPCWSTR;

#undef  NULL
#undef  TRUE
#undef  FALSE

#define NULL    (0)
#define NULLP   ((void*)0)
#define TRUE    (1)
#define FALSE   (0)

#define MAKE_DW2(hi,low)    ((DWORD)(hi) << 16) | (low))
#define MAKE_DW4(a,b,c,d)   (((DWORD)(a) << 24) | ((b) << 16) | ((c) << 8) | (d))
#define _DTOI(d)            ((int)((d) + 0.5))
#define _RGB(r,g,b)         (((r) << 16) | ((g) << 8) | (b))
#define _RGBA(r,g,b,a)      (((DWORD)(a) << 24) | ((r) << 16) | ((g) << 8) | (b))
#define _GETR(c)            (((c) >> 16) & 0xff)
#define _GETG(c)            (((c) >> 8) & 0xff)
#define _GETB(c)            ((c) & 0xff)
#define _GETA(c)            ((c) >> 24)

#define ASMV                __asm__ __volatile__
#define _DELETE(p)          if(p) { delete (p); (p) = 0; }

//---------------

void AXError(LPCSTR szFormat,...);

void AXMemZero(void *pBuf,DWORD size);

void *AXMalloc(DWORD dwSize);
BOOL AXRealloc(void **ppBuf,DWORD dwSize);
void AXFreeNormal(void *pBuf);
void AXFree(void **ppBuf);

//----------------

class AXPoint
{
public:
    int x,y;

    void set(int xx,int yy) { x = xx; y = yy; }
    void zero() { x = y = 0; }
    BOOL operator ==(const AXPoint &c) const { return (x == c.x && y == c.y); }
    BOOL operator !=(const AXPoint &c) const { return (x != c.x || y != c.y); }
    void operator +=(const AXPoint &c) { x += c.x; y += c.y; }
    void operator -=(const AXPoint &c) { x -= c.x; y -= c.y; }
};

class AXSize
{
public:
    int w,h;

    void set(int ww,int hh) { w = ww; h = hh; }
    void zero() { w = h = 0; }
    BOOL operator ==(const AXSize &c) const { return (w == c.w && h == c.h); }
};

#endif
