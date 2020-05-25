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

#ifndef __BASKETBALL2_H__
#define __BASKETBALL2_H__

#include "Platform.h"
#include "Platform_Basketball2.h"

#define BASKETBALL2_BLIP_ROWS			5
#define BASKETBALL2_BLIP_COLUMNS		7

#define BASKETBALL2_SOUND_TICK			0
#define BASKETBALL2_SOUND_BOUNCE		1
#define BASKETBALL2_SOUND_SCORE			2	// Four-note fanfare
#define BASKETBALL2_SOUND_ENDPLAY		3	// Two whistles
#define BASKETBALL2_SOUND_ENDQUARTER	4	// Buzzer
#define BASKETBALL2_SOUND_ENDGAME		5	// Long buzzer
#define BASKETBALL2_SOUND_SCORE3		6	// Double fanfare
#define BASKETBALL2_SOUND_FOUL			7	// One whistle
#define BASKETBALL2_SOUND_3SECS			8	// Three whistles
#define BASKETBALL2_SOUND_FREETHROW		9	// Two-note fanfare
#define BASKETBALL2_SOUND_OVERTIME		10	// Two buzzers
#define BASKETBALL2_SOUND_NSOUNDS		11	


void Basketball2_Run(int tu);
void Basketball2_SetSkill(int i);
int Basketball2_GetSkill();
void Basketball2_PowerOn();
void Basketball2_PowerOff();
BOOL Basketball2_GetPower();
void Basketball2_Debug(int f);

#endif

