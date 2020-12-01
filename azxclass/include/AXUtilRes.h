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

#ifndef _AX_UTILRES_H
#define _AX_UTILRES_H

#include "AXDef.h"

class AXMem;
class AXImageList;
class AXImage32;
class AXTarBall;

BOOL AXLoadResAppIcon(AXMem *pmem,LPCSTR szFileName);
void AXLoadResImageList(AXImageList *pimg,LPCSTR szFileName,int onew,int colMask,BOOL bDisable);
void AXLoadResImage32(AXImage32 *pimg,LPCSTR szFileName);

void AXLoadTarResImageList(AXTarBall *ptar,AXImageList *pimg,LPCSTR szFileName,int onew,int colMask,BOOL bDisable);
void AXLoadTarResImage32(AXTarBall *ptar,AXImage32 *pimg,LPCSTR szFileName);

#endif
