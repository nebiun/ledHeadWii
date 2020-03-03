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


#ifndef __FOOTBALL2_H__
#define __FOOTBALL2_H__


#include "Platform.h"
#include "Platform_Football2.h"


#define FOOTBALL2_BLIP_ROWS		3
#define FOOTBALL2_BLIP_COLUMNS	10

#define FOOTBALL2_SOUND_CHARGESTART	0
#define FOOTBALL2_SOUND_CHARGE		1
#define FOOTBALL2_SOUND_TICK			2
#define FOOTBALL2_SOUND_RUNBACK		3
#define FOOTBALL2_SOUND_FIRSTDOWN		4
#define FOOTBALL2_SOUND_ENDPLAY		5
#define FOOTBALL2_SOUND_ENDPOSSESSION	6
#define FOOTBALL2_SOUND_ENDQUARTER	7
#define FOOTBALL2_SOUND_TOUCHDOWN		8
#define FOOTBALL2_SOUND_SAFETY		9

void Football2_Run();
void Football2_SetSkill(int i);
int Football2_GetSkill();
void Football2_PowerOn();
void Football2_PowerOff();
int Football2_GetPower();


#endif
