/*
 * LEDhead for Wii
 * Copyright (C) 2017-2020 Nebiun
 *
 * Based on the handheld electronic games by Mattel Electronics.
 * All trademarks copyrighted by their respective owners. This
 * program is not affiliated or endorsed by Mattel Electronics.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef __SOCCER2_H__
#define __SOCCER2_H__

#include "Platform.h"
#include "Platform_Soccer2.h"

#define SOCCER2_BLIP_ROWS		5
#define SOCCER2_BLIP_COLUMNS	7
#define VALID_COLUMN(x)			( ((x)>=0) && ((x) < SOCCER2_BLIP_COLUMNS) )
#define VALID_ROW(y)			( ((y)>=0) && ((y) < SOCCER2_BLIP_ROWS) )

#define SOCCER2_SOUND_TICK			0	// Time
#define SOCCER2_SOUND_LOOSEBALL		1	// Loose ball
#define SOCCER2_SOUND_BALLOUT		2	// Defense stole ball or ball out
#define SOCCER2_SOUND_GOAL			3	// Goal Scored
#define SOCCER2_SOUND_BEEP			4	// Offense in sec.1 ore Teammate controlled
#define SOCCER2_SOUND_HIGHKICK		5	// High shot & high pass
#define SOCCER2_SOUND_LOWKICK		6 	// Low shot & low pass or defense kick
#define SOCCER2_SOUND_ENDHALF		7	// End of the half 
#define SOCCER2_SOUND_ENDGAME		8	// End of the game
#define SOCCER2_SOUND_BOUNCE		9	// Bounce (or ball is caught or bounced by anyone)
#define SOCCER2_SOUND_NSOUNDS		10	// Total sounds

void Soccer2_Run(int tu);
void Soccer2_SetSkill(int i);
int Soccer2_GetSkill();
void Soccer2_PowerOn();
void Soccer2_PowerOff();
int Soccer2_GetPower();
void Soccer2_Debug(int f);

#endif
