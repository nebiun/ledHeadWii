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

#ifndef __AUTORACE_H__
#define __AUTORACE_H__

#include "Platform.h"
#include "Platform_Autorace.h"


#define AUTORACE_BLIP_ROWS			7
#define AUTORACE_BLIP_COLUMNS		3

#define AUTORACE_SOUND_GEAR0		0
#define AUTORACE_SOUND_GEAR1		1
#define AUTORACE_SOUND_GEAR2		2
#define AUTORACE_SOUND_GEAR3		3
#define AUTORACE_SOUND_HIT			4
#define AUTORACE_SOUND_TIME			5
#define AUTORACE_SOUND_WIN			6
#define AUTORACE_SOUND_NSOUNDS		7

void AutoRace_Run(int tu);
void AutoRace_SetSkill(int i);
int AutoRace_GetSkill();
void AutoRace_PowerOn();
void AutoRace_PowerOff();
BOOL AutoRace_GetPower();


#endif





