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


#ifndef __PLATFORM_BASKETBALL_H__
#define __PLATFORM_BASKETBALL_H__

// [general]
#define basketball_digit_spacing 12
#define basketball_digit_x 110
#define basketball_digit_y 197
#define basketball_digit_w 8
#define basketball_digit_h 9
#define basketball_blip_xspacing 3500
#define basketball_blip_yspacing 3400
#define basketball_blip_x 50
#define basketball_blip_y 72
#define basketball_power_off_x 27
#define basketball_power_off_y 274
#define basketball_pro_1_x 27
#define basketball_pro_1_y 274
#define basketball_pro_2_x 27
#define basketball_pro_2_y 274
#define basketball_basket_x 120
#define basketball_basket_y 51

// interface that the platform must provide for this game

// functions exported to the game context
void Basketball_Init();
void Basketball_Help();
void Basketball_DeInit();
void Basketball_Paint();
void Basketball_ClearScreen();
void Basketball_PlaySound(int nSound, unsigned int nFlags);
void Basketball_StopSound();
void Basketball_GetSize(int *w, int *h);

// "private" functions not exported to game context
void Basketball_DrawBlip(int nBright, int x, int y);
void Basketball_DrawStat(int nStat);
void Basketball_DrawBasket(BOOL bBasket);

BOOL Basketball_GetInputLEFT(BOOL *pChange);
BOOL Basketball_GetInputUP(BOOL *pChange);
BOOL Basketball_GetInputRIGHT(BOOL *pChange);
BOOL Basketball_GetInputDOWN(BOOL *pChange);
BOOL Basketball_GetInputTHROW(BOOL *pChange);
BOOL Basketball_TestForMovement();

#endif
