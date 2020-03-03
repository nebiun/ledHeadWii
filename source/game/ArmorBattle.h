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

#ifndef __ARMORBATTLE_H__
#define __ARMORBATTLE_H__

#include "Platform.h"
#include "Platform_armorbattle.h"


#define ARMORBATTLE_BLIP_ROWS			4
#define ARMORBATTLE_BLIP_COLUMNS		5

#define ARMORBATTLE_SOUND_TICK			0
#define ARMORBATTLE_SOUND_NEAR			1
#define ARMORBATTLE_SOUND_ENEMY			2
#define ARMORBATTLE_SOUND_FIRE			3
#define ARMORBATTLE_SOUND_HIT			4
#define ARMORBATTLE_SOUND_SCORE			5
#define ARMORBATTLE_SOUND_ENDGAME		6

void ArmorBattle_Run();
void ArmorBattle_SetSkill(int i);
int ArmorBattle_GetSkill();
void ArmorBattle_PowerOn();
void ArmorBattle_PowerOff();
BOOL ArmorBattle_GetPower();


#endif





