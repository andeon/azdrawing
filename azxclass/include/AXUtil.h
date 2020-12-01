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

#ifndef _AX_UTIL_H
#define _AX_UTIL_H

#include "AXDef.h"

class AXString;

int AXDoubleToInt(double val);
int AXDPMtoDPI(int dpm);
int AXDPItoDPM(int dpi);

void AXGetWORDLE(const LPBYTE pBuf,LPVOID pVal);
void AXGetDWORDLE(const LPBYTE pBuf,LPVOID pVal);
void AXGetWORDBE(const LPBYTE pBuf,LPVOID pVal);
void AXGetDWORDBE(const LPBYTE pBuf,LPVOID pVal);

void AXSetWORDLE(LPVOID pBuf,WORD val);
void AXSetDWORDLE(LPVOID pBuf,DWORD val);
void AXSetWORDBE(LPVOID pBuf,WORD val);
void AXSetDWORDBE(LPVOID pBuf,DWORD val);

int AXGetFirstOnBit(DWORD val);
int AXGetFirstOffBit(DWORD val);

int AXSetVariableLenVal(LPVOID pBuf,UINT val);
UINT AXGetVariableLenVal(const void *pBuf,void **ppRet=NULL);

BOOL AXExeBrowser(const AXString &url);

int AXGetBase64EncSize(int size);
void AXEncodeBase64(LPVOID pDst,const void *pSrc,int size);
int AXDecodeBase64(LPVOID pDst,LPCSTR pSrc,int bufsize);

#endif
