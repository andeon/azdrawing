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

#ifndef _AX_UTIL_STR_H
#define _AX_UTIL_STR_H

#include "AXDef.h"

class AXString;

void AXAddRecentString(AXString *pstrArray,int nArrayCnt,const AXString &strAdd);
void AXURIListToString(AXString *pstr,LPCSTR szURI,BOOL bLocalFile);

int AXGetUTF8CharWidth(LPCSTR szText);

int AXUnicodeToUTF8Char(LPSTR szBuf,UINT code);
UINT AXUTF8toUnicodeChar(LPCSTR szText,char **ppRet);

int AXUStrLen(LPCUSTR pBuf);
int AXUStrCompare(LPCUSTR pBuf1,LPCUSTR pBuf2);
int AXUStrCompare(LPCUSTR pBuf1,LPCSTR pBuf2);
int AXUStrCompare(LPCUSTR pBuf1,LPCUSTR pBuf2,int len);
int AXUStrCompareCharEnd(LPCUSTR pBuf1,LPCUSTR pBuf2,UNICHAR codeEnd);
int AXUStrCompareCase(LPCUSTR pBuf1,LPCUSTR pBuf2);
int AXUStrCompareCase(LPCUSTR pBuf1,LPCSTR pBuf2);
BOOL AXUStrCompareMatch(LPCUSTR pSrc,LPCUSTR pPattern,BOOL bCase=FALSE);
int AXUStrToInt(LPCUSTR pBuf);
DWORD AXUStrHexToVal(LPCUSTR pBuf,int maxlen,UNICHAR **ppRet=NULL);
double AXUStrToDouble(LPCUSTR pBuf);

void AXStrRev(LPSTR szBuf,int len);
int AXIntToStr(LPSTR szBuf,int nVal);
int AXIntToFloatStr(LPSTR szBuf,int val,int dig);
int AXIntToStrDig(LPSTR szBuf,int val,int dig);
int AXValToHexStr(LPSTR szBuf,DWORD val);
int AXValToHexStrDig(LPSTR szBuf,DWORD val,int dig);

#endif
