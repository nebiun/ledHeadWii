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


#ifndef __PLATFORM_SUBCHASE_H__
#define __PLATFORM_SUBCHASE_H__
// [general]
#define subchase_digit_spacing 21
#define subchase_digit_x 103
#define subchase_digit_y 46
#define subchase_digit_w 21
#define subchase_digit_h 27
#define subchase_blip_xspacing 2400
#define subchase_blip_yspacing 3400
#define subchase_blip_x 49
#define subchase_blip_y 90
#define subchase_power_x 42
#define subchase_power_y 251

// [areas]
#define subchase_power_off_x 38
#define subchase_power_off_y 268
#define subchase_power_off_w 22
#define subchase_power_off_h 16
#define subchase_power_on_x 38
#define subchase_power_on_y 252
#define subchase_power_on_w 22
#define subchase_power_on_h 16
#define subchase_up_x 145
#define subchase_up_y 227
#define subchase_up_w 25
#define subchase_up_h 22
#define subchase_down_x 145
#define subchase_down_y 274
#define subchase_down_w 25
#define subchase_down_h 22
#define subchase_left_x 125
#define subchase_left_y 249
#define subchase_left_w 22
#define subchase_left_h 25
#define subchase_right_x 168
#define subchase_right_y 249
#define subchase_right_w 22
#define subchase_right_h 25
#define subchase_fire_x 0
#define subchase_fire_y 0
#define subchase_fire_w 0
#define subchase_fire_h 0


// interface that the platform must provide for this game

// functions exported to the game context
void SubChase_Init();
void SubChase_Help();
void SubChase_DeInit();
void SubChase_Paint();
void SubChase_ClearScreen();
void SubChase_PlaySound(int nSound, unsigned int nFlags);
void SubChase_StopSound();
void SubChase_OnClick(int x, int y);
int SubChase_OnMouseMove(int x, int y);
void SubChase_GetSize(int *w, int *h);

// "private" functions not exported to game context
void SubChase_DrawBlip(int nBright, int x, int y);
void SubChase_DrawScore(int nScore);

int SubChase_GetInputLEFT(BOOL *pChange);
int SubChase_GetInputUP(BOOL *pChange);
int SubChase_GetInputRIGHT(BOOL *pChange);
int SubChase_GetInputDOWN(BOOL *pChange);
int SubChase_GetInputFIRE(BOOL *pChange);

#endif
