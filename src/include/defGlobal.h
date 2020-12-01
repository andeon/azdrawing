/************************************************************************
*  Copyright (C) 2013-2015 Azel.
*
*  This file is part of AzDrawing.
*
*  AzDrawing is free software: you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation, either version 3 of the License, or
*  (at your option) any later version.
*
*  AzDrawing is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*************************************************************************/

#ifndef _AZDRAW_DEFGLOBAL_H_
#define _AZDRAW_DEFGLOBAL_H_

#ifdef _DEF_NOEXTERN
    #define EXTERN
#else
    #define EXTERN	extern
#endif

class CConfig;
class CDraw;
class CResource;

EXTERN CConfig   *g_pconf;
EXTERN CDraw     *g_pdraw;
EXTERN CResource *g_pres;

#undef EXTERN

#endif
