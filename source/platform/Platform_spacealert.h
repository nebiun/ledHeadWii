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


#ifndef __PLATFORM_SPACEALERT_H__
#define __PLATFORM_SPACEALERT_H__

// [general]
#define spacealert_digit_spacing 12
#define spacealert_digit_x 163
#define spacealert_digit_y 86
#define spacealert_digit_w 13
#define spacealert_digit_h 14
#define spacealert_blip_xspacing 1300
#define spacealert_blip_yspacing 1600
#define spacealert_blip_x 158
#define spacealert_blip_y 117
#define spacealert_slider_x 142
#define spacealert_slider_y 271
#define spacealert_power_x 87
#define spacealert_power_y 231

// [general]
#define missileattack_digit_spacing 12
#define missileattack_digit_x 172
#define missileattack_digit_y 65
#define missileattack_digit_w 13
#define missileattack_digit_h 14
#define missileattack_blip_xspacing 1300
#define missileattack_blip_yspacing 1700
#define missileattack_blip_x 166
#define missileattack_blip_y 95
#define missileattack_slider_x 151
#define missileattack_slider_y 292
#define missileattack_power_x 78
#define missileattack_power_y 188

// interface that the platform must provide for this game

// functions exported to the game context
void SpaceAlert_Init();
void MissileAttack_Init();
void SpaceAlert_Help();
void MissileAttack_Help();
void SpaceAlert_DeInit();
void SpaceAlert_Paint();
void SpaceAlert_ClearScreen();
void SpaceAlert_PlaySound(int nSound, unsigned int nFlags);
void SpaceAlert_StopSound();
void SpaceAlert_GetSize(int *w, int *h);

// "private" functions not exported to game context
void SpaceAlert_PlayRaiderSound();
void SpaceAlert_StopRaiderSound();
void SpaceAlert_DrawBlip(int nBright, int x, int y);
void SpaceAlert_DrawScore(int nScore);

int SpaceAlert_GetInputSTICK();
BOOL SpaceAlert_GetInputFIRE(BOOL *pChange);

#endif
