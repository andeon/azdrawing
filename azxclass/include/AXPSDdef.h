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

#ifndef _AX_PSDDEF_H
#define _AX_PSDDEF_H

namespace axpsd
{
    enum COLMODE
    {
        COLMODE_BITMAP      = 0,
        COLMODE_GRAYSCALE   = 1,
        COLMODE_INDEXED     = 2,
        COLMODE_RGB         = 3,
        COLMODE_CMYK        = 4,
        COLMODE_MULTI       = 7,
        COLMODE_DUOTONE     = 8,
        COLMODE_LAB         = 9
    };

    enum PSDBLEND
    {
        BLEND_NORMAL        = ('n'<<24)|('o'<<16)|('r'<<8)|'m',
        BLEND_DISSOLVE      = ('d'<<24)|('i'<<16)|('s'<<8)|'s',
        BLEND_DARKEN        = ('d'<<24)|('a'<<16)|('r'<<8)|'k',
        BLEND_MULTIPLY      = ('m'<<24)|('u'<<16)|('l'<<8)|' ',
        BLEND_BURN          = ('i'<<24)|('d'<<16)|('i'<<8)|'v',
        BLEND_LINEAR_BURN   = ('l'<<24)|('b'<<16)|('r'<<8)|'n',
        BLEND_DARKER_COL    = ('d'<<24)|('k'<<16)|('C'<<8)|'l',
        BLEND_LIGHTEN       = ('l'<<24)|('i'<<16)|('t'<<8)|'e',
        BLEND_SCREEN        = ('s'<<24)|('c'<<16)|('r'<<8)|'n',
        BLEND_DODGE         = ('d'<<24)|('i'<<16)|('v'<<8)|' ',
        BLEND_LINEAR_DODGE  = ('l'<<24)|('d'<<16)|('d'<<8)|'g',
        BLEND_LIGHTER_COL   = ('l'<<24)|('g'<<16)|('C'<<8)|'l',
        BLEND_OVERLAY       = ('o'<<24)|('v'<<16)|('e'<<8)|'r',
        BLEND_SOFT_LIGHT    = ('s'<<24)|('L'<<16)|('i'<<8)|'t',
        BLEND_HARD_LIGHT    = ('h'<<24)|('L'<<16)|('i'<<8)|'t',
        BLEND_VIVID_LIGHT   = ('v'<<24)|('L'<<16)|('i'<<8)|'t',
        BLEND_LINEAR_LIGHT  = ('l'<<24)|('L'<<16)|('i'<<8)|'t',
        BLEND_PIN_LIGHT     = ('p'<<24)|('L'<<16)|('i'<<8)|'t',
        BLEND_HARD_MIX      = ('h'<<24)|('M'<<16)|('i'<<8)|'x',
        BLEND_DIFFERENCE    = ('d'<<24)|('i'<<16)|('f'<<8)|'f',
        BLEND_EXCLUSION     = ('s'<<24)|('m'<<16)|('u'<<8)|'d',
        BLEND_SUBTRACT      = ('f'<<24)|('s'<<16)|('u'<<8)|'b',
        BLEND_DIVIDE        = ('f'<<24)|('d'<<16)|('i'<<8)|'v',
        BLEND_HUE           = ('h'<<24)|('u'<<16)|('e'<<8)|' ',
        BLEND_SATURATION    = ('s'<<24)|('a'<<16)|('t'<<8)|' ',
        BLEND_COLOR         = ('c'<<24)|('o'<<16)|('l'<<8)|'r',
        BLEND_LUMINOSITY    = ('l'<<24)|('u'<<16)|('m'<<8)|' '
    };
};

#endif
