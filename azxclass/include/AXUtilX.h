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

#ifndef _AX_UTIL_X_H
#define _AX_UTIL_X_H

#include "AXDef.h"

class AXMem;
class AXMemAuto;
class AXString;


ULONG AXGetAtom(LPCSTR szName);
void AXGetAtomName(AXString *pstr,ULONG atom);

void AXSetPropertyAtom(ULONG idWin,ULONG atomProp,ULONG *pAtom,int num);
void AXSetPropertyCARDINAL(ULONG idWin,ULONG atomProp,long *pVal,int num);
void AXSetPropertyCompoundText(ULONG idWin,ULONG atomProp,const AXString &str);
void AXSetProperty8(ULONG idWin,ULONG atomProp,ULONG atomType,const void *pBuf,UINT uSize,BOOL bAdd=FALSE);

BOOL AXGetProperty8(ULONG idWin,ULONG atomProp,AXMem *pmem);
BOOL AXGetProperty32Array(ULONG idWin,ULONG atomProp,ULONG atomType,LPVOID pBuf,int cnt);
BOOL AXGetProperty32(ULONG idWin,ULONG atomProp,ULONG atomType,AXMem *pmem,LPINT pCnt);

ULONG AXGetSelectionTargetType(ULONG idWin,ULONG atomSelection,ULONG *pAtom,int cnt);
BOOL AXGetSelectionData(ULONG idWin,ULONG atomSelection,ULONG atomType,AXMem *pmem);
BOOL AXGetSelectionCompoundText(ULONG idWin,ULONG atomSelection,AXString *pstr);

void AXSetEvClientMessage(LPVOID pEvent,ULONG idWin,ULONG atomMes);
BOOL AXRecvEventTimeout(int evtype,int timeoutms,LPVOID pEvent);

void AXSendWMEvent(ULONG idWin,ULONG atom,long data1,long data2=0,long data3=0,long data4=0);
void AXSendNetWMState(ULONG idWin,int action,ULONG atomFirst,ULONG atomSecond=0);
BOOL AXIsExistNetWMState(ULONG idWin,ULONG atomFirst,ULONG atomSecond=0);

ULONG AXGetTopFrameWindow(ULONG idWin);
ULONG AXGetActiveWindow();
ULONG AXGetChildWindowPt(ULONG idWin,int rootx,int rooty);
BOOL AXGetKeysymString(AXString *pstr,UINT keysym);
void AXUngrabPointer();

#endif
