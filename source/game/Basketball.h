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


#ifndef __BASKETBALL_H__
#define __BASKETBALL_H__

#include "Platform.h"
#include "Platform_Basketball.h"

#define BASKETBALL_BLIP_ROWS			4
#define BASKETBALL_BLIP_COLUMNS			5

#define BASKETBALL_SOUND_TICK			0
#define BASKETBALL_SOUND_BOUNCE			1
#define BASKETBALL_SOUND_SCORE			2
#define BASKETBALL_SOUND_ENDPLAY		3
#define BASKETBALL_SOUND_ENDQUARTER		4
#define BASKETBALL_SOUND_ENDGAME		5
#define BASKETBALL_SOUND_NSOUNDS		6

void Basketball_Run(int tu);
void Basketball_SetSkill(int i);
int Basketball_GetSkill();
void Basketball_PowerOn();
void Basketball_PowerOff();
BOOL Basketball_GetPower();


#endif

