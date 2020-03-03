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


#ifndef __PLATFORM_AUTORACE_H__
#define __PLATFORM_AUTORACE_H__

// [general]
#define autorace_digit_spacing 10
#define autorace_digit_x 161
#define autorace_digit_y 75
#define autorace_digit_w 8
#define autorace_digit_h 9
#define autorace_blip_xspacing 1600
#define autorace_blip_yspacing 1900
#define autorace_blip_x 153
#define autorace_blip_y 114
#define autorace_slider_x 141
#define autorace_slider_y 265
#define autorace_power_x 91
#define autorace_power_y 229
#define autorace_gear_x 91
#define autorace_gear_y 94

// [general]
#define skislalom_digit_spacing 10
#define skislalom_digit_x 161
#define skislalom_digit_y 95
#define skislalom_digit_w 8
#define skislalom_digit_h 9
#define skislalom_blip_xspacing 1600
#define skislalom_blip_yspacing 1900
#define skislalom_blip_x 153
#define skislalom_blip_y 134
#define skislalom_slider_x 141
#define skislalom_slider_y 285
#define skislalom_power_x 91
#define skislalom_power_y 229
#define skislalom_gear_x 91
#define skislalom_gear_y 94

// interface that the platform must provide for this game

// functions exported to the game context
void SkiSlalom_Init();
void SkiSlalom_Help();

void AutoRace_Init();
void AutoRace_Help();
void AutoRace_DeInit();
void AutoRace_Paint();
void AutoRace_ClearScreen();
void AutoRace_PlaySound(int nSound, unsigned int nFlags);
void AutoRace_StopSound();
void AutoRace_GetSize(int *w, int *h);

// "private" functions not exported to game context
void AutoRace_PlayEngineSound();
void AutoRace_StopEngineSound();
void AutoRace_DrawBlip(int nBright, int x, int y);
void AutoRace_DrawTime(int nTime);

int AutoRace_GetInputSTICK();
int AutoRace_GetInputGEAR(BOOL *pChange);

#endif
