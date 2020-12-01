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

#ifndef _AX_UTIL_FILE_H
#define _AX_UTIL_FILE_H

#include "AXDef.h"

class AXString;
struct AXFILESTAT;

BOOL AXIsExistFile(const AXString &filename,BOOL bDir=FALSE);
LONGLONG AXGetFileSize(const AXString &filename);
BOOL AXCreateDir(const AXString &dirpath,int perm=0755);
BOOL AXCreateDirHome(LPCSTR szPathAdd,int perm=0755);
BOOL AXDeleteFile(const AXString &filename);
BOOL AXDeleteDir(const AXString &dirpath);
void AXDeleteDirAndFiles(const AXString &dirpath);
BOOL AXGetFILESTAT(const AXString &filename,AXFILESTAT *pdst);
void AXStatToFILESTAT(AXFILESTAT *pdst,const void *psrc);
BOOL AXSetFileTimestamp(const AXString &filename,LONGLONG timeAccess,LONGLONG timeMod);

#endif
