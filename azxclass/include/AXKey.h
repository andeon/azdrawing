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

#ifndef _AX_KEYDEF_H
#define _AX_KEYDEF_H

enum KEYDEF
{
    KEY_BACKSPACE   = 0xff08,
    KEY_TAB         = 0xff09,
    KEY_ENTER       = 0xff0d,
    KEY_PAUSE       = 0xff13,
    KEY_SCROLL_LOCK = 0xff14,
    KEY_PRINTSCREEN = 0xff15,
    KEY_ESCAPE      = 0xff1b,
    KEY_DELETE      = 0xffff,

    KEY_KANJI       = 0xff21,
    KEY_MUHENKAN    = 0xff22,
    KEY_HENKAN      = 0xff23,
    KEY_ROMAJI      = 0xff24,
    KEY_HIRAGANA_KATAKANA = 0xff27,
    KEY_ZENKAKU_HANKAKU = 0xff2a,
    KEY_EISU        = 0xff30,

    KEY_HOME        = 0xff50,
    KEY_LEFT        = 0xff51,
    KEY_UP          = 0xff52,
    KEY_RIGHT       = 0xff53,
    KEY_DOWN        = 0xff54,
    KEY_PAGEUP      = 0xff55,
    KEY_PAGEDOWN    = 0xff56,
    KEY_END         = 0xff57,
    KEY_BEGIN       = 0xff58,

    KEY_INSERT      = 0xff63,
    KEY_MENU        = 0xff67,
    KEY_NUM_LOCK    = 0xff7f,

    KEY_NUM_SPACE   = 0xff80,
    KEY_NUM_TAB     = 0xff89,
    KEY_NUM_ENTER   = 0xff8d,
    KEY_NUM_F1      = 0xff92,
    KEY_NUM_F2      = 0xff93,
    KEY_NUM_F3      = 0xff94,
    KEY_NUM_HOME    = 0xff95,
    KEY_NUM_LEFT    = 0xff96,
    KEY_NUM_UP      = 0xff97,
    KEY_NUM_RIGHT   = 0xff98,
    KEY_NUM_DOWN    = 0xff99,
    KEY_NUM_PAGEUP  = 0xff9a,
    KEY_NUM_PAGEDOWN = 0xff9b,
    KEY_NUM_END     = 0xff9c,
    KEY_NUM_BEGIN   = 0xff9d,
    KEY_NUM_INSERT  = 0xff9e,
    KEY_NUM_DELETE  = 0xff9f,
    KEY_NUM_EQUAL   = 0xffbd,
    KEY_NUM_MUL     = 0xffaa,
    KEY_NUM_ADD     = 0xffab,
    KEY_NUM_SUB     = 0xffad,
    KEY_NUM_PERIOD  = 0xffae,
    KEY_NUM_DIV     = 0xffaf,
    KEY_NUM_0       = 0xffb0,
    KEY_NUM_1       = 0xffb1,
    KEY_NUM_2       = 0xffb2,
    KEY_NUM_3       = 0xffb3,
    KEY_NUM_4       = 0xffb4,
    KEY_NUM_5       = 0xffb5,
    KEY_NUM_6       = 0xffb6,
    KEY_NUM_7       = 0xffb7,
    KEY_NUM_8       = 0xffb8,
    KEY_NUM_9       = 0xffb9,

    KEY_F1          = 0xffbe,
    KEY_F2          = 0xffbf,
    KEY_F3          = 0xffc0,
    KEY_F4          = 0xffc1,
    KEY_F5          = 0xffc2,
    KEY_F6          = 0xffc3,
    KEY_F7          = 0xffc4,
    KEY_F8          = 0xffc5,
    KEY_F9          = 0xffc6,
    KEY_F10         = 0xffc7,
    KEY_F11         = 0xffc8,
    KEY_F12         = 0xffc9,

    KEY_SHIFT_L     = 0xffe1,
    KEY_SHIFT_R     = 0xffe2,
    KEY_CTRL_L      = 0xffe3,
    KEY_CTRL_R      = 0xffe4,
    KEY_CAPS_LOCK   = 0xffe5,
    KEY_SHIFT_LOCK  = 0xffe6,
    KEY_ALT_L       = 0xffe9,
    KEY_ALT_R       = 0xffea,
    KEY_SUPER_L     = 0xffeb,
    KEY_SUPER_R     = 0xffec,

    KEY_SPACE       = 0x20,

    KEY_VOID        = 0xffffff
};

#define ISKEY_ENTER(k)      (k == KEY_ENTER || k == KEY_NUM_ENTER)
#define ISKEY_SPACE(k)      (k == ' ' || k == KEY_NUM_SPACE)
#define ISKEY_TAB(k)        (k == KEY_TAB || k == KEY_NUM_TAB)
#define ISKEY_UP(k)         (k == KEY_UP || k == KEY_NUM_UP)
#define ISKEY_DOWN(k)       (k == KEY_DOWN || k == KEY_NUM_DOWN)
#define ISKEY_LEFT(k)       (k == KEY_LEFT || k == KEY_NUM_LEFT)
#define ISKEY_RIGHT(k)      (k == KEY_RIGHT || k == KEY_NUM_RIGHT)
#define ISKEY_PAGEUP(k)     (k == KEY_PAGEUP || k == KEY_NUM_PAGEUP)
#define ISKEY_PAGEDOWN(k)   (k == KEY_PAGEDOWN || k == KEY_NUM_PAGEDOWN)
#define ISKEY_HOME(k)       (k == KEY_HOME || k == KEY_NUM_HOME)
#define ISKEY_END(k)        (k == KEY_END || k == KEY_NUM_END)

#endif
