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
#include "football.h"
#include "football2.h"
#include "hockey.h"
#include "hockeyca.h"
#include "soccer.h"
#include "armorbattle.h"
#include "baseball.h"
#include "subchase.h"
#include "armorbattle_screen_png.h"
#include "autorace_screen_png.h"
#include "baseball_screen_png.h"
#include "basketball_screen_png.h"
#include "football_screen_png.h"
#include "football2_screen_png.h"
#include "hockey_screen_png.h"
#include "hockeyca_screen_png.h"
#include "skislalom_screen_png.h"
#include "soccer_screen_png.h"
#include "spacealert_screen_png.h"
#include "subchase_screen_png.h"
#include "nodisp_screen_png.h"

GAMECONTEXT gGameContext[NUM_GAMES] = 
{
	// armor battle
	{
		GAME_ARMORBATTLE,
		150,		
		"Armor Battle",
		"armorbattle",
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
	},
	// auto race
	{
		GAME_AUTORACE,
		100,//65
		"Auto Race",
		"autorace",
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
		AutoRace_Help
	},
	// baseball
	{
		GAME_BASEBALL,
		10,
		"Baseball",
		"baseball",
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
		Baseball_Help
	},
	// basketball
	{
		GAME_BASKETBALL,
		60,
		"Basketball",
		"basketball",
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
		Basketball_Help
	},
	// basketball 2
	{
		GAME_BASKETBALL2,
		0,
		"Basketball II",
		"basketball2",
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		nodisp_screen_png,
		NULL
	},
	// football
	{
		GAME_FOOTBALL,
		55,
		"Football",
		"football",
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
	},
	// football 2
	{
		GAME_FOOTBALL2,
		50,
		"Football II",
		"football2",
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
	},
	// hockey
	{
		GAME_HOCKEY,
		60,
		"Hockey",
		"hockey",
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
		Hockey_Help
	},
	// hockey (canadian)
	{
		GAME_HOCKEYCA,
		60,
		"Hockey (Canadian)",
		"hockeyca",
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
		HockeyCa_Help
	},
	// ski slalom
	{
		GAME_SKISLALOM,
		65,
		"Ski Slalom",
		"skislalom",
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
		SkiSlalom_Help
	},
	// soccer
	{
		GAME_SOCCER,
		60,
		"Soccer",
		"soccer",
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
		Soccer_Help
	},
	// soccer 2
	{
		GAME_SOCCER2,
		0,
		"Soccer II",
		"soccer2",
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		nodisp_screen_png,
		NULL
	},
	// space alert
	{
		GAME_SPACEALERT,
		110,
		"Space Alert",
		"spacealert",
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
		SpaceAlert_Help
	},
	// subchase
	{
		GAME_SUBCHASE,
		60,
		"Sub Chase",
		"subchase",
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
		SubChase_Help
	}
};

GAMECONTEXT *gCurrentGame = NULL;
