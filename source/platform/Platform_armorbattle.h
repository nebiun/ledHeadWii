/*

LEDhead
Copyright 2001, Peter Hirschberg
Author: Peter Hirschberg

The current version of this SOURCE CODE as well as the official
versions of the LEDHEAD APPLICATION are available from my website
at: http://www.peterhirschberg.com

Based on the handheld electronic games by Mattel Electronics.
All trademarks copyrighted by their respective owners. This
program is not affiliated or endorsed by Mattel Electronics.

License agreement:

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program (license.txt); if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

Email : peter@peterhirschberg.com
Website : http://www.peterhirschberg.com

*/

#ifndef __PLATFORM_ARMORBATTLE_H__
#define __PLATFORM_ARMORBATTLE_H__

// [general]
#define armorbattle_digit_spacing 24
#define armorbattle_digit_x 101
#define armorbattle_digit_y 30
#define armorbattle_digit_w 24
#define armorbattle_digit_h 33
#define armorbattle_blip_xspacing 3600
#define armorbattle_blip_yspacing 3600
#define armorbattle_blip_x 49
#define armorbattle_blip_y 78
#define armorbattle_power_x 42
#define armorbattle_power_y 241

// interface that the platform must provide for this game

// Input remapping
#define ARMORBATTLE_KEY_UP
#define ARMORBATTLE_KEY_DOWN
#define ARMORBATTLE_KEY_UP
#define ARMORBATTLE_KEY_UP

// functions exported to the game context
void ArmorBattle_Init();
void ArmorBattle_Help();
void ArmorBattle_DeInit();
void ArmorBattle_Paint();
void ArmorBattle_ClearScreen();
void ArmorBattle_PlaySound(int nSound, unsigned int nFlags);
void ArmorBattle_StopSound();
void ArmorBattle_GetSize(int *w, int *h);

// "private" functions not exported to game context
void ArmorBattle_PlayMineSound();
void ArmorBattle_StopMineSound();
void ArmorBattle_DrawBlip(int nBright, int x, int y);
void ArmorBattle_DrawTime(int nTime);

int ArmorBattle_GetInputLEFT(BOOL *pChange);
int ArmorBattle_GetInputUP(BOOL *pChange);
int ArmorBattle_GetInputRIGHT(BOOL *pChange);
int ArmorBattle_GetInputDOWN(BOOL *pChange);
int ArmorBattle_GetInputFIRE(BOOL *pChange);

#endif
