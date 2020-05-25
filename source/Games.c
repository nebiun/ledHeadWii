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

#include "LED_Handled.h"
#include "games.h"

#include "autorace.h"
#include "spacealert.h"
#include "basketball.h"
#include "basketball2.h"
#include "football.h"
#include "football2.h"
#include "hockey.h"
#include "hockeyca.h"
#include "soccer.h"
#include "soccer2.h"
#include "armorbattle.h"
#include "baseball.h"
#include "subchase.h"
#include "armorbattle_screen_png.h"
#include "autorace_screen_png.h"
#include "baseball_screen_png.h"
#include "basketball_screen_png.h"
#include "basketball2_screen_png.h"
#include "football_screen_png.h"
#include "football2_screen_png.h"
#include "hockey_screen_png.h"
#include "hockeyca_screen_png.h"
#include "skislalom_screen_png.h"
#include "soccer_screen_png.h"
#include "soccer2_screen_png.h"
#include "spacealert_screen_png.h"
#include "missileattack_screen_png.h"
#include "subchase_screen_png.h"

//#define DEEP_DEBUG	1

GAMECONTEXT gGameContext[NUM_GAMES] = 
{
	// armor battle
	{
		GAME_ARMORBATTLE,
		150,		
		"Armor Battle (1978)",
		ArmorBattle_Init,
		ArmorBattle_DeInit,
		ArmorBattle_Run,
		ArmorBattle_SetSkill,
		ArmorBattle_GetSkill,
		ArmorBattle_PowerOn,
		ArmorBattle_PowerOff,
		ArmorBattle_GetPower,
		ArmorBattle_Paint,
		ArmorBattle_GetSize,
		armorbattle_screen_png,
		ArmorBattle_Help,
		NULL
	},
	// auto race
	{
		GAME_AUTORACE,
		100,//65
		"Auto Race (1976)",
		AutoRace_Init,
		AutoRace_DeInit,
		AutoRace_Run,
		AutoRace_SetSkill,
		AutoRace_GetSkill,
		AutoRace_PowerOn,
		AutoRace_PowerOff,
		AutoRace_GetPower,
		AutoRace_Paint,
		AutoRace_GetSize,
		autorace_screen_png,
		AutoRace_Help,
		NULL
	},
	// baseball
	{
		GAME_BASEBALL,
		1,
		"Baseball (1978)",
		Baseball_Init,
		Baseball_DeInit,
		Baseball_Run,
		Baseball_SetSkill,
		Baseball_GetSkill,
		Baseball_PowerOn,
		Baseball_PowerOff,
		Baseball_GetPower,
		Baseball_Paint,
		Baseball_GetSize,
		baseball_screen_png,
		Baseball_Help,
#ifdef DEEP_DEBUG
		Baseball_Debug
#else
		NULL
#endif
	},
	// basketball
	{
		GAME_BASKETBALL,
		60,
		"Basketball (1978)",
		Basketball_Init,
		Basketball_DeInit,
		Basketball_Run,
		Basketball_SetSkill,
		Basketball_GetSkill,
		Basketball_PowerOn,
		Basketball_PowerOff,
		Basketball_GetPower,
		Basketball_Paint,
		Basketball_GetSize,
		basketball_screen_png,
		Basketball_Help,
		NULL
	},
	// basketball 2
	{
		GAME_BASKETBALL2,
		60,
		"Basketball 2 (1979)",
		Basketball2_Init,
		Basketball2_DeInit,
		Basketball2_Run,
		Basketball2_SetSkill,
		Basketball2_GetSkill,
		Basketball2_PowerOn,
		Basketball2_PowerOff,
		Basketball2_GetPower,
		Basketball2_Paint,
		Basketball2_GetSize,
		basketball2_screen_png,
		Basketball2_Help,
#ifdef DEEP_DEBUG
		Basketball2_Debug
#else
		NULL
#endif
	},
	// football
	{
		GAME_FOOTBALL,
		55,
		"Football (1977)",
		Football_Init,
		Football_DeInit,
		Football_Run,
		Football_SetSkill,
		Football_GetSkill,
		Football_PowerOn,
		Football_PowerOff,
		Football_GetPower,
		Football_Paint,
		Football_GetSize,
		football_screen_png,
		Football_Help,
		NULL
	},
	// football 2
	{
		GAME_FOOTBALL2,
		50,
		"Football II (1978)",
		Football2_Init,
		Football2_DeInit,
		Football2_Run,
		Football2_SetSkill,
		Football2_GetSkill,
		Football2_PowerOn,
		Football2_PowerOff,
		Football2_GetPower,
		Football2_Paint,
		Football2_GetSize,
		football2_screen_png,
		Football2_Help,
#ifdef DEEP_DEBUG
		Football2_Debug
#else
		NULL
#endif		
	},
	// hockey
	{
		GAME_HOCKEY,
		60,
		"Hockey (1978)",
		Hockey_Init,
		Hockey_DeInit,
		Hockey_Run,
		Hockey_SetSkill,
		Hockey_GetSkill,
		Hockey_PowerOn,
		Hockey_PowerOff,
		Hockey_GetPower,
		Hockey_Paint,
		Hockey_GetSize,
		hockey_screen_png,
		Hockey_Help,
		NULL
	},
	// hockey (canadian)
	{
		GAME_HOCKEYCA,
		60,
		"Hockey (Canadian) (1978)",
		HockeyCa_Init,
		HockeyCa_DeInit,
		HockeyCa_Run,
		HockeyCa_SetSkill,
		HockeyCa_GetSkill,
		HockeyCa_PowerOn,
		HockeyCa_PowerOff,
		HockeyCa_GetPower,
		HockeyCa_Paint,
		HockeyCa_GetSize,
		hockeyca_screen_png,
		HockeyCa_Help,
		NULL
	},
	// ski slalom
	{
		GAME_SKISLALOM,
		65,
		"Ski Slalom (1980)",
		SkiSlalom_Init,
		AutoRace_DeInit,
		AutoRace_Run,
		AutoRace_SetSkill,
		AutoRace_GetSkill,
		AutoRace_PowerOn,
		AutoRace_PowerOff,
		AutoRace_GetPower,
		AutoRace_Paint,
		AutoRace_GetSize,
		skislalom_screen_png,
		SkiSlalom_Help,
		NULL
	},
	// soccer
	{
		GAME_SOCCER,
		60,
		"Soccer (1978)",
		Soccer_Init,
		Soccer_DeInit,
		Soccer_Run,
		Soccer_SetSkill,
		Soccer_GetSkill,
		Soccer_PowerOn,
		Soccer_PowerOff,
		Soccer_GetPower,
		Soccer_Paint,
		Soccer_GetSize,
		soccer_screen_png,
		Soccer_Help,
		NULL
	},
	// soccer 2
	{
		GAME_SOCCER2,
		60,
		"Soccer 2 (1979)",
		Soccer2_Init,
		Soccer2_DeInit,
		Soccer2_Run,
		Soccer2_SetSkill,
		Soccer2_GetSkill,
		Soccer2_PowerOn,
		Soccer2_PowerOff,
		Soccer2_GetPower,
		Soccer2_Paint,
		Soccer2_GetSize,
		soccer2_screen_png,
		Soccer2_Help,
#ifdef DEEP_DEBUG
		Soccer2_Debug
#else
		NULL
#endif
	},
	// space alert
	{
		GAME_SPACEALERT,
		110,
		"Space Alert (1978)",
		SpaceAlert_Init,
		SpaceAlert_DeInit,
		SpaceAlert_Run,
		NULL,
		NULL,
		SpaceAlert_PowerOn,
		SpaceAlert_PowerOff,
		SpaceAlert_GetPower,
		SpaceAlert_Paint,
		SpaceAlert_GetSize,
		spacealert_screen_png,
		SpaceAlert_Help,
#ifdef DEEP_DEBUG
		SpaceAlert_Debug
#else
		NULL
#endif
	},
	// missile attack
	{
		GAME_MISSILEATTACK,
		110,
		"Missile Attack (1977)",
		MissileAttack_Init,
		SpaceAlert_DeInit,
		SpaceAlert_Run,
		NULL,
		NULL,
		SpaceAlert_PowerOn,
		SpaceAlert_PowerOff,
		SpaceAlert_GetPower,
		SpaceAlert_Paint,
		SpaceAlert_GetSize,
		missileattack_screen_png,
		MissileAttack_Help,
#ifdef DEEP_DEBUG
		SpaceAlert_Debug
#else
		NULL
#endif
	},
	// subchase
	{
		GAME_SUBCHASE,
		60,
		"Sub Chase (1978)",
		SubChase_Init,
		SubChase_DeInit,
		SubChase_Run,
		NULL,
		NULL,
		SubChase_PowerOn,
		SubChase_PowerOff,
		SubChase_GetPower,
		SubChase_Paint,
		SubChase_GetSize,
		subchase_screen_png,
		SubChase_Help,
		NULL
	}
};

GAMECONTEXT *gCurrentGame = NULL;
