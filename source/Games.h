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


#ifndef __GAMES_H__
#define __GAMES_H__

// game ids
#define GAME_ARMORBATTLE		0
#define GAME_AUTORACE			1
#define GAME_BASEBALL			2
#define GAME_BASKETBALL			3
#define GAME_BASKETBALL2		4
#define GAME_FOOTBALL			5
#define GAME_FOOTBALL2			6
#define GAME_HOCKEY				7
#define GAME_HOCKEYCA			8
#define GAME_SKISLALOM			9
#define GAME_SOCCER				10
#define GAME_SOCCER2			11
#define GAME_SPACEALERT			12
#define GAME_SUBCHASE			13

#define NUM_GAMES				14	

typedef struct GAMECONTEXT
{
	int id;

	// is the interval in milliseconds that the game's
	// 'run' function should be called
	int tu;

	char *szName;
	char *szDir;

	void (*Init)(void);
	void (*DeInit)(void);
	void (*Run)(int tu);
	void (*SetSkill)(int i);
	int (*GetSkill)(void);
	void (*PowerOn)(void);
	void (*PowerOff)(void);
	int (*GetPower)(void);
	void (*Paint)(void);
	void (*GetSize)(int *w, int *h);
	const u8 *screen;
	void (*Help)(void);
}GAMECONTEXT;

extern GAMECONTEXT gGameContext[NUM_GAMES];
extern GAMECONTEXT *gCurrentGame;


#endif
