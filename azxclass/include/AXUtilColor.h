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

#ifndef _AX_UTIL_COLOR_H
#define _AX_UTIL_COLOR_H

#include "AXDef.h"

void AXRGBtoHSV(int r,int g,int b,double *pHSV);
void AXRGBtoHSV(DWORD rgb,double *pHSV);
void AXRGBtoHSV(DWORD rgb,LPINT pHSV);

void AXHSVtoRGB(int h,double s,double v,LPINT pRGB);
DWORD AXHSVtoRGB(int h,double s,double v);
DWORD AXHSVtoRGB_fast(int h,int s,int v);

void AXHLStoRGB(int h,double l,double s,LPINT pRGB);
DWORD AXHLStoRGB(int h,double l,double s);
void AXRGBtoHLS(int r,int g,int b,double *pHLS);

#endif
