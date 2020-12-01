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

#ifndef _AX_UTIL_ZLIB_H
#define _AX_UTIL_ZLIB_H

#include "AXDef.h"

class AXFile;
class AXMem;
class AXMemAuto;

BOOL AXZlib_DecodeFromFile(LPBYTE pDstBuf,DWORD dwDstSize,AXFile *pFile,DWORD dwSrcSize,DWORD dwBufSize=8192);
BOOL AXZlib_DecodeFromBuf(LPBYTE pDstBuf,DWORD dwDstSize,LPBYTE pSrcBuf,DWORD dwSrcSize);

DWORD AXZlib_EncodeToFile(AXFile *pFile,LPBYTE pSrcBuf,DWORD dwSrcSize,int nLevel=6,DWORD dwBufSize=8192);
DWORD AXZlib_EncodeToMemAuto(AXMemAuto *pmem,LPBYTE pSrcBuf,DWORD dwSrcSize,int nLevel=6,DWORD dwBufSize=8192);

#endif
