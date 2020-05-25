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


#include "SubChase.h"
#include "Games.h"


// constants

typedef int BLIP;
static BLIP Blips[SUBCHASE_BLIP_COLUMNS][SUBCHASE_BLIP_ROWS];

#define TIME_SONAR_SLOW						5
#define TIME_SONAR_FAST						2
#define TIME_ENEMY_SUB_MOVE					30
#define TIME_INITIAL_FIRE_TORPEDO_DELAY		250
#define TIME_FIRE_TORPEDO_DELAY				20
#define TIME_TORPEDO_MOVE					6
#define TIME_KILL_GAME_TIMER				80

// game variables
static BOOL bGameOver;
static BOOL bInFrame = FALSE;
static BOOL bPower;

static int nScore;
static int nCharges;
static int nChargeValue;

static int nTimerSonar;
static int nTimerEnemySubMove;
static int nTimerFireTorpedo;
static int nTimerMoveTorpedo;
static int nKillGameTimer;

static int nSubDirX;
static int nTorpedoDirX;

static BOOL bPlayerHasMoved;

static BOOL bInitialTorpedoFired = FALSE;

static void InitGame();
static void PositionEnemySub();
static void FireTorpedoes();
static void DrawBlips();
static void DrawScore(int nDisplay);

typedef struct OBJECT
{
	int x;
	int y;
	BOOL bEnable;
}OBJECT;

static OBJECT player;
static OBJECT sub;
static OBJECT torpedo[2];


BOOL SubChase_GetPower()
{
	return (bPower ? TRUE : FALSE);
}

void SubChase_PowerOn()
{
	InitGame();
	bPower = TRUE;
}

void SubChase_PowerOff()
{
	bPower = FALSE;
}

int SubChase_GetSkill()
{
	return 0;
}

void SubChase_SetSkill(int i)
{
}

void InitGame()
{
	bGameOver = FALSE;

	nScore = 0;
	nCharges = 30;
	nChargeValue = 5;

	player.x = 3;
	player.y = 1;

	torpedo[0].bEnable = FALSE;
	torpedo[1].bEnable = FALSE;

	nTorpedoDirX = 1;
	nTimerFireTorpedo = TIME_INITIAL_FIRE_TORPEDO_DELAY;
	nKillGameTimer = TIME_KILL_GAME_TIMER;

	bPlayerHasMoved = FALSE;

	nTimerSonar = 0;

	PositionEnemySub();

	Platform_IsNewSecond();
}


void SubChase_Run(int tu)
{
	int x, y;

	// prevent reentrancy
	if (bInFrame){ return; }
	bInFrame = TRUE;

	// init the blips field
	for (y = 0; y < SUBCHASE_BLIP_ROWS; y++){
		for (x = 0; x < SUBCHASE_BLIP_COLUMNS; x++){
			Blips[x][y] = BLIP_OFF;
		}
	}

	// handle power off, and game over states
	if (bPower){
		if (bGameOver)
		{
			Blips[player.x][player.y] = BLIP_BRIGHT;
			DrawBlips();
			DrawScore(nScore);
			bInFrame = FALSE;
			return;
		}
	} else {
		SubChase_ClearScreen();
		DrawScore(-1);
		bInFrame = FALSE;
		return;
	}

	// run the game
	BOOL bChange;
	if (SubChase_GetInputLEFT(&bChange))
	{
		bPlayerHasMoved = TRUE;
		if (bChange)
		{
			// move left
			if (player.x > 0)
			{
				player.x--;
			}
		}
	}
	else if (SubChase_GetInputRIGHT(&bChange))
	{
		bPlayerHasMoved = TRUE;
		if (bChange)
		{
			// move right
			if (player.x < (SUBCHASE_BLIP_COLUMNS-1))
			{
				player.x++;
			}
		}
	}
	else if (SubChase_GetInputUP(&bChange))
	{
		bPlayerHasMoved = TRUE;
		if (bChange)
		{
			// move up
			if (player.y > 0)
			{
				player.y--;
			}
		}
	}
	else if (SubChase_GetInputDOWN(&bChange))
	{
		bPlayerHasMoved = TRUE;
		if (bChange)
		{
			// move down
			if (player.y < (SUBCHASE_BLIP_ROWS-1))
			{
				player.y++;
			}
		}
	}

	if (SubChase_GetInputFIRE(NULL))
	{
		// release depth charge - check for hit
		if ((player.x == sub.x) && (player.y == sub.y))
		{
			// hit
			{
				// clear the screen except for the player's ship
				SubChase_ClearScreen();
				DrawScore(-1);
				Platform_StartDraw();
					SubChase_DrawBlip(BLIP_BRIGHT, player.x, player.y);
				Platform_EndDraw();
			}
			SubChase_PlaySound(SUBCHASE_SOUND_HIT, PLAYSOUNDFLAGS_PRIORITY);
			nScore+=nChargeValue;
			nChargeValue = 5;
			bInitialTorpedoFired = TRUE;
			PositionEnemySub();
		}
		else
		{
			// miss
			{
				// clear the screen except for the player's ship
				SubChase_ClearScreen();
				DrawScore(-1);
				Platform_StartDraw();
					SubChase_DrawBlip(BLIP_BRIGHT, player.x, player.y);
				Platform_EndDraw();
			}
			SubChase_PlaySound(SUBCHASE_SOUND_CHARGE, PLAYSOUNDFLAGS_PRIORITY);
			nTimerEnemySubMove = 0;

			// decrease charge value
			if (nChargeValue==5)
			{
				nChargeValue=3;
			}
			else if (nChargeValue==3)
			{
				nChargeValue=2;
			}
			else if (nChargeValue==2)
			{
				nChargeValue=1;
			}

			// decrease number of charges and check for game over
			--nCharges;
			if (nCharges == 0)
			{
				// GAME OVER!
				{
					// clear the screen except for the player's ship
					SubChase_ClearScreen();
					DrawScore(-1);
					Platform_StartDraw();
						SubChase_DrawBlip(BLIP_BRIGHT, player.x, player.y);
					Platform_EndDraw();
				}
				SubChase_PlaySound(SUBCHASE_SOUND_SINK, PLAYSOUNDFLAGS_PRIORITY);
				bGameOver = TRUE;
				bInFrame = FALSE;
				return;
			}
		}
	}

	// check for proximity to enemy sub
	int nSonar = 0;
	if (((sub.x == player.x) && (sub.y == player.y))
		|| ((sub.x == player.x) && (sub.y == player.y-1))
		|| ((sub.x == player.x) && (sub.y == player.y+1)))
	{
		// fast
		nSonar = 1;
	}
	else if (((sub.x == player.x-1) && (sub.y == player.y-1))
		|| ((sub.x == player.x-1) && (sub.y == player.y))
		|| ((sub.x == player.x-1) && (sub.y == player.y+1))
		|| ((sub.x == player.x+1) && (sub.y == player.y-1))
		|| ((sub.x == player.x+1) && (sub.y == player.y))
		|| ((sub.x == player.x+1) && (sub.y == player.y+1)))
	{
		// slow
		nSonar = 2;
	}

	if (nSonar)
	{
		++nTimerSonar;
		if (nSonar==2)
		{
			if (nTimerSonar > TIME_SONAR_SLOW)
			{
				SubChase_PlaySound(SUBCHASE_SOUND_SONAR, PLAYSOUNDFLAGS_ASYNC);
				nTimerSonar = 0;
			}
		}
		else
		{
			if (nTimerSonar > TIME_SONAR_FAST)
			{
				SubChase_PlaySound(SUBCHASE_SOUND_SONAR, PLAYSOUNDFLAGS_ASYNC);
				nTimerSonar = 0;
			}
		}
	}
	else
	{
		nTimerSonar = 0;
	}

	// update the blips
	Blips[player.x][player.y] = BLIP_BRIGHT;
//	Blips[sub.x][sub.y] = BLIP_DIM;
	if (torpedo[0].bEnable)
	{
		if ((torpedo[0].x >= 0) && (torpedo[0].x < SUBCHASE_BLIP_COLUMNS))
		{
			Blips[torpedo[0].x][torpedo[0].y] = BLIP_DIM;
		}
	}
	if (torpedo[1].bEnable)
	{
		if ((torpedo[1].x >= 0) && (torpedo[1].x < SUBCHASE_BLIP_COLUMNS))
		{
			Blips[torpedo[1].x][torpedo[1].y] = BLIP_DIM;
		}
	}

	// check for torpedo hit
	if (((torpedo[0].bEnable) && (torpedo[0].x == player.x) && (torpedo[0].y == player.y))
		|| ((torpedo[0].bEnable) && (torpedo[0].x == player.x) && (torpedo[0].y == player.y)))
	{
		// torpedo hit
		{
			// clear the screen except for the player's ship
			SubChase_ClearScreen();
			DrawScore(-1);
			Platform_StartDraw();
				SubChase_DrawBlip(BLIP_BRIGHT, player.x, player.y);
			Platform_EndDraw();
		}
		SubChase_PlaySound(SUBCHASE_SOUND_SINK, PLAYSOUNDFLAGS_PRIORITY);
		bGameOver = TRUE;
		bInFrame = FALSE;
		return;
	}

	DrawBlips();
	DrawScore(nScore);

	// move the enemy sub
	--nTimerEnemySubMove;
	if (nTimerEnemySubMove <= 0)
	{
		nTimerEnemySubMove = TIME_ENEMY_SUB_MOVE;
		if (nSubDirX > 0)
		{
			if (sub.x >= (SUBCHASE_BLIP_COLUMNS-1))
			{
				nSubDirX = -1;
			}
		}
		else
		{
			if (sub.x <= 0)
			{
				nSubDirX = 1;
			}
		}
		sub.x += nSubDirX;
	}

	// move the torpedoes
	--nTimerMoveTorpedo;
	if (nTimerMoveTorpedo <= 0)
	{
		nTimerMoveTorpedo = TIME_TORPEDO_MOVE;
		for (int i=0; i<2; i++)
		{
			if (torpedo[i].bEnable)
			{
				torpedo[i].x += nTorpedoDirX;
				if ((torpedo[i].x < 0) && (nTorpedoDirX < 0))
				{
					torpedo[i].bEnable = FALSE;
				}
				if ((torpedo[i].x >= SUBCHASE_BLIP_COLUMNS) && (nTorpedoDirX > 0))
				{
					torpedo[i].bEnable = FALSE;
				}
			}
		}
	}

	if (nKillGameTimer > 0)
	{
		--nKillGameTimer;
	}
	if (!bPlayerHasMoved && (nKillGameTimer <= 0))
	{
		// player is just sitting there
		// fire a torpedo at him right now
		// (as per real game)
		FireTorpedoes();
		bPlayerHasMoved = TRUE;
	}

	// randomly fire torpedoes as required
	if ((torpedo[0].bEnable == FALSE) && (torpedo[1].bEnable == FALSE))
	{
		if (nTimerFireTorpedo > 0)
		{
			--nTimerFireTorpedo;
		}
		if (Platform_Random(5) == 0)
		{
			if (nTimerFireTorpedo <= 0)
			{
				FireTorpedoes();
			}
		}
	}

	bInFrame = FALSE;
}

void DrawBlips()
{
	Platform_StartDraw();

	// draw the blips field
	for (int y = 0; y < SUBCHASE_BLIP_ROWS; y++){
		for (int x = 0; x < SUBCHASE_BLIP_COLUMNS; x++)
		{
			switch(Blips[x][y]){
				case BLIP_OFF:
				case BLIP_DIM:
				case BLIP_BRIGHT:
					SubChase_DrawBlip(Blips[x][y], x, y);
					break;
			}
		}
	}

	Platform_EndDraw();
}

void DrawScore(int nDisplay)
{
	// draw the display
	Platform_StartDraw();
	SubChase_DrawScore(nDisplay);
	Platform_EndDraw();
}

void PositionEnemySub()
{
	nTimerEnemySubMove = TIME_ENEMY_SUB_MOVE;
	nSubDirX = Platform_Random(2) ? 1 : -1;

	while (1)
	{
		sub.x = Platform_Random(SUBCHASE_BLIP_COLUMNS);
		sub.y = Platform_Random(SUBCHASE_BLIP_ROWS);
		if (((sub.x == player.x) && (sub.y == player.y))
			|| ((sub.x == player.x-1) && (sub.y == player.y-1))
			|| ((sub.x == player.x-1) && (sub.y == player.y))
			|| ((sub.x == player.x-1) && (sub.y == player.y+1))
			|| ((sub.x == player.x+1) && (sub.y == player.y-1))
			|| ((sub.x == player.x+1) && (sub.y == player.y))
			|| ((sub.x == player.x+1) && (sub.y == player.y+1))
			|| ((sub.x == player.x) && (sub.y == player.y-1))
			|| ((sub.x == player.x) && (sub.y == player.y+1)))
		{
			// try again
		}
		else
		{
			return;
		}
	}
}

static void FireTorpedoes()
{
	nTimerFireTorpedo = TIME_FIRE_TORPEDO_DELAY;
	nTimerMoveTorpedo = TIME_TORPEDO_MOVE;

	// enable 1 or more torpedoes
	torpedo[0].bEnable = TRUE;
	if (bInitialTorpedoFired)
	{
		torpedo[1].bEnable = Platform_Random(7) ? FALSE : TRUE;
	}

	// pick the direction based on where the player is
	if (player.x < 3)
	{
		nTorpedoDirX = -1;
	}
	else if (player.x > 3)
	{
		nTorpedoDirX = 1;
	}
	else
	{
		if (!bPlayerHasMoved)
		{
			// when player hasn't moved, the torpedo is always from the right
			nTorpedoDirX = -1;
		}
		else
		{
			nTorpedoDirX = (Platform_Random(2)) ? 1 : -1;
		}
	}

	if (nTorpedoDirX > 0)
	{
		if ((player.x == (SUBCHASE_BLIP_COLUMNS-1)) && Platform_Random(2))
		{
			torpedo[0].x = 3;
			torpedo[1].x = 3;
		}
		else
		{
			torpedo[0].x = player.x - Platform_Random(5) - 3;
			torpedo[1].x = player.x - Platform_Random(5) - 3;
		}
	}
	else
	{
		if ((player.x == 0) && Platform_Random(2))
		{
			torpedo[0].x = 3;
			torpedo[1].x = 3;
		}
		else
		{
			torpedo[0].x = player.x + Platform_Random(5) + 3;
			torpedo[1].x = player.x + Platform_Random(5) + 3;
		}
	}

	torpedo[0].y = player.y;
	torpedo[1].y = Platform_Random(3);

	bInitialTorpedoFired = TRUE;
}

