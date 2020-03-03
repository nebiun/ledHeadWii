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


#include "ArmorBattle.h"
#include "Games.h"


// constants

typedef int BLIP;

static BLIP Blips[ARMORBATTLE_BLIP_COLUMNS][ARMORBATTLE_BLIP_ROWS];


// game variables
static BOOL bGameOver;
static BOOL bInFrame = FALSE;
static BOOL bPower;
static BOOL bGameStarted;

static int nGameTimer;
static int nScore;
static int nHits;
static int nDamageCount;

static int nPlayerStartX;
static int nPlayerStartY;

static void PaintGame(int nDisplay);
static void InitGame();
static void PositionPlayers();
static void PlantMines();

typedef struct OBJECT
{
	int x;
	int y;
}OBJECT;

static OBJECT player;
static OBJECT enemy;
static OBJECT mines[4];

BOOL ArmorBattle_GetPower()
{
	return (bPower ? TRUE : FALSE);
}

void ArmorBattle_PowerOn()
{
	InitGame();
	bPower = TRUE;
}

void ArmorBattle_PowerOff()
{
	bPower = FALSE;
}

int ArmorBattle_GetSkill()
{
	return 0;
}

void ArmorBattle_SetSkill(int i)
{
}

void InitGame()
{
	bGameOver = FALSE;
	nGameTimer = 0;
	bGameStarted = FALSE;
	nGameTimer = 0;
	nScore = 0;
	nHits = 0;
	nDamageCount = 0;

	nPlayerStartX = 4;
	nPlayerStartY = 3;

	player.x = nPlayerStartX;
	player.y = nPlayerStartY;

	enemy.x = 1;
	enemy.y = 1;

	PlantMines();

	Platform_IsNewSecond();
}


void ArmorBattle_Run(int tu)
{
	int x, y;

	// prevent reentrancy
	if (bInFrame){ return; }
	bInFrame = TRUE;

	// init the blips field
	for (y = 0; y < ARMORBATTLE_BLIP_ROWS; y++){
		for (x = 0; x < ARMORBATTLE_BLIP_COLUMNS; x++){
			Blips[x][y] = BLIP_OFF;
		}
	}

	// update the game timer
	if (Platform_IsNewSecond())
	{
		if (!bGameOver && bPower && bGameStarted)
		{
			++nGameTimer;
			if (nGameTimer > 99)
			{
				// time's up -- game over!
				bGameOver = TRUE;

				ArmorBattle_StopMineSound();
				ArmorBattle_ClearScreen();
				ArmorBattle_PlaySound(ARMORBATTLE_SOUND_ENDGAME, PLAYSOUNDFLAGS_PRIORITY);

				// put the tanks in their starting positions
				player.x = nPlayerStartX;
				player.y = nPlayerStartY;
			}
			else
			{
				// check to see if player tank is in range for enemy tank to fire
				if ( ((player.x-1 == enemy.x) && (player.y == enemy.y))
					|| ((player.x+1 == enemy.x) && (player.y == enemy.y))

					|| ((player.x == enemy.x) && (player.y-1 == enemy.y))
					|| ((player.x == enemy.x) && (player.y+1 == enemy.y))

					|| ((player.x-1 == enemy.x) && (player.y-1 == enemy.y))
					|| ((player.x+1 == enemy.x) && (player.y+1 == enemy.y))

					|| ((player.x-1 == enemy.x) && (player.y+1 == enemy.y))
					|| ((player.x+1 == enemy.x) && (player.y-1 == enemy.y)))
				{
					// in range
					if (nDamageCount > 0)
					{
						ArmorBattle_ClearScreen();
						ArmorBattle_StopMineSound();
						ArmorBattle_PlaySound(ARMORBATTLE_SOUND_ENEMY, PLAYSOUNDFLAGS_PRIORITY);
					}
					++nDamageCount;
					if (nDamageCount > 3)
					{
						// we got blowed up
						Blips[enemy.x][enemy.y] = BLIP_DIM;
						Blips[player.x][player.y] = BLIP_BRIGHT;

						ArmorBattle_StopSound();
						PaintGame(nScore);
						Platform_Pause(500);

						ArmorBattle_ClearScreen();
						ArmorBattle_PlaySound(ARMORBATTLE_SOUND_HIT, PLAYSOUNDFLAGS_PRIORITY);
						player.x = nPlayerStartX;
						player.y = nPlayerStartY;

						if (++nHits >= 10)
						{
							// too many hits - game over!

							ArmorBattle_PlaySound(ARMORBATTLE_SOUND_ENDGAME, PLAYSOUNDFLAGS_PRIORITY);
							bGameOver = TRUE;

							// put the tanks in their starting positions
							player.x = nPlayerStartX;
							player.y = nPlayerStartY;
						}

						bInFrame = FALSE;
						return;
					}
				}
				else
				{
					nDamageCount = 0;
				}
			}
		}
	}

	// handle power off, and game over states
	if (bPower){
		if (bGameOver)
		{
			Blips[player.x][player.y] = BLIP_BRIGHT;
			Blips[enemy.x][enemy.y] = BLIP_DIM;
			PaintGame(nScore);
			bInFrame = FALSE;
			return;
		}
	} else {
		ArmorBattle_ClearScreen();
		bInFrame = FALSE;
		return;
	}

	// wait for fire button before starting
	if (!bGameStarted)
	{
		// wait to start

		Blips[player.x][player.y] = BLIP_BRIGHT;
		Blips[enemy.x][enemy.y] = BLIP_DIMBLINK;

		BOOL bChange;
		if (ArmorBattle_GetInputFIRE(&bChange))
		{
			if (bChange)
			{
				bGameStarted = TRUE;
			}
		}
	}
	else
	{
		// run the game

		BOOL bHit = FALSE;

		BOOL bChange;
		if (ArmorBattle_GetInputLEFT(&bChange))
		{
			if (bChange)
			{
				if (ArmorBattle_GetInputFIRE(NULL))
				{
					// fire left
					Blips[player.x][player.y] = BLIP_BRIGHT;
					Blips[enemy.x][enemy.y] = BLIP_OFF;
					PaintGame(-1);
					ArmorBattle_PlaySound(ARMORBATTLE_SOUND_FIRE, PLAYSOUNDFLAGS_PRIORITY);

					// test for a hit
					if ((enemy.x == player.x - 1) && (enemy.y == player.y))
					{
						bHit = TRUE;
					}

				}
				else
				{
					// move left
					if (player.x > 0)
					{
						player.x--;
					}
				}
			}
		}
		else if (ArmorBattle_GetInputRIGHT(&bChange))
		{
			if (bChange)
			{
				if (ArmorBattle_GetInputFIRE(NULL))
				{
					// fire right
					Blips[player.x][player.y] = BLIP_BRIGHT;
					Blips[enemy.x][enemy.y] = BLIP_OFF;
					PaintGame(-1);
					ArmorBattle_PlaySound(ARMORBATTLE_SOUND_FIRE, PLAYSOUNDFLAGS_PRIORITY);

					// test for a hit
					if ((enemy.x == player.x + 1) && (enemy.y == player.y))
					{
						bHit = TRUE;
					}
				}
				else
				{
					// move right
					if (player.x < (ARMORBATTLE_BLIP_COLUMNS-1))
					{
						player.x++;
					}
				}
			}
		}
		else if (ArmorBattle_GetInputUP(&bChange))
		{
			if (bChange)
			{
				if (ArmorBattle_GetInputFIRE(NULL))
				{
					// fire up
					Blips[player.x][player.y] = BLIP_BRIGHT;
					Blips[enemy.x][enemy.y] = BLIP_OFF;
					PaintGame(-1);
					ArmorBattle_PlaySound(ARMORBATTLE_SOUND_FIRE, PLAYSOUNDFLAGS_PRIORITY);

					// test for a hit
					if ((enemy.x == player.x) && (enemy.y == player.y - 1))
					{
						bHit = TRUE;
					}
				}
				else
				{
					// move up
					if (player.y > 0)
					{
						player.y--;
					}
				}
			}
		}
		else if (ArmorBattle_GetInputDOWN(&bChange))
		{
			if (bChange)
			{
				if (ArmorBattle_GetInputFIRE(NULL))
				{
					// fire down
					Blips[player.x][player.y] = BLIP_BRIGHT;
					Blips[enemy.x][enemy.y] = BLIP_OFF;
					PaintGame(-1);
					ArmorBattle_PlaySound(ARMORBATTLE_SOUND_FIRE, PLAYSOUNDFLAGS_PRIORITY);

					// test for a hit
					if ((enemy.x == player.x) && (enemy.y == player.y + 1))
					{
						bHit = TRUE;
					}
				}
				else
				{
					// move down
					if (player.y < (ARMORBATTLE_BLIP_ROWS-1))
					{
						player.y++;
					}
				}
			}
		}

		if (bHit)
		{
			// hit an enemy tank
			ArmorBattle_StopMineSound();
			Blips[player.x][player.y] = BLIP_BRIGHT;
			Blips[enemy.x][enemy.y] = BLIP_DIM;
			PaintGame(nScore);

			Platform_Pause(250);

			ArmorBattle_ClearScreen();
			ArmorBattle_PlaySound(ARMORBATTLE_SOUND_SCORE, PLAYSOUNDFLAGS_PRIORITY);
			++nScore;

			PositionPlayers();
			PlantMines();

			bGameStarted = FALSE;

			bInFrame = FALSE;
			return;
		}

		// check for being next to a mine
		{
			BOOL bNear = FALSE;
			for (int i=0; i<3; i++)
			{
				if (((mines[i].x+1 == player.x) && (mines[i].y == player.y))
					|| ((mines[i].x-1 == player.x) && (mines[i].y == player.y))
					|| ((mines[i].x == player.x) && (mines[i].y+1 == player.y))
					|| ((mines[i].x == player.x) && (mines[i].y-1 == player.y)))
				{
					bNear = TRUE;
					break;
				}
			}
			if (bNear)
			{
				ArmorBattle_PlayMineSound();
			}
			else
			{
				ArmorBattle_StopMineSound();
			}

		}

		// check for landing on a mine
		{
			for (int i=0; i<3; i++)
			{
				if (((mines[i].x == player.x) && (mines[i].y == player.y))
					|| ((player.x == enemy.x) && (player.y == enemy.y)))
				{
					// hit a mine or enemy tank!
					Blips[player.x][player.y] = BLIP_BRIGHT;

					ArmorBattle_StopSound();
					PaintGame(nScore);
					Platform_Pause(500);

					ArmorBattle_StopMineSound();
					ArmorBattle_ClearScreen();
					ArmorBattle_PlaySound(ARMORBATTLE_SOUND_HIT, PLAYSOUNDFLAGS_PRIORITY);
					player.x = nPlayerStartX;
					player.y = nPlayerStartY;

					if (++nHits >= 10)
					{
						// too many hits - game over!
						ArmorBattle_ClearScreen();
						ArmorBattle_PlaySound(ARMORBATTLE_SOUND_ENDGAME, PLAYSOUNDFLAGS_PRIORITY);
						bGameOver = TRUE;

						// put the tanks in their starting positions
						player.x = nPlayerStartX;
						player.y = nPlayerStartY;
					}

					bInFrame = FALSE;
					return;
				}
			}
		}

		// tick sound
		ArmorBattle_PlaySound(ARMORBATTLE_SOUND_TICK, PLAYSOUNDFLAGS_ASYNC);

		// update the blips
		Blips[player.x][player.y] = BLIP_BRIGHT;
		Blips[enemy.x][enemy.y] = BLIP_DIM;
	}

	if (bGameStarted)
	{
		PaintGame(nGameTimer);
	}
	else
	{
		PaintGame(nScore);
	}

	bInFrame = FALSE;
}

void PaintGame(int nDisplay)
{
	static int nBlinkCounter = 0;

	Platform_StartDraw();

	// draw the blips field
	for (int y = 0; y < ARMORBATTLE_BLIP_ROWS; y++){
		for (int x = 0; x < ARMORBATTLE_BLIP_COLUMNS; x++)
		{
			switch(Blips[x][y]){
				case BLIP_OFF:
				case BLIP_DIM:
				case BLIP_BRIGHT:
					ArmorBattle_DrawBlip(Blips[x][y], x, y);
					break;
				case BLIP_DIMBLINK:
					if ((nBlinkCounter % 6 == 1) || (nBlinkCounter % 6 == 3)){
						ArmorBattle_DrawBlip(BLIP_DIM, x, y);
					} else {
						ArmorBattle_DrawBlip(BLIP_OFF, x, y);
					}
					break;
			}
		}
	}

	// draw the display
	ArmorBattle_DrawTime(nDisplay);

	Platform_EndDraw();

	++nBlinkCounter;

}


void PositionPlayers()
{
	// the player is always positioned somewhere on the edge
	// pick a random edge
	do {
		switch(Platform_Random(4))
		{
			case 0:
				// left side
				player.x = 0;
				player.y = Platform_Random(ARMORBATTLE_BLIP_ROWS);
				break;
			case 1:
				// top side
				player.x = Platform_Random(ARMORBATTLE_BLIP_COLUMNS);
				player.y = 0;
				break;
			case 2:
				// right side
				player.x = ARMORBATTLE_BLIP_COLUMNS-1;
				player.y = Platform_Random(ARMORBATTLE_BLIP_ROWS);
				break;
			case 3:
				// bottom side
				player.x = Platform_Random(ARMORBATTLE_BLIP_COLUMNS);
				player.y = ARMORBATTLE_BLIP_ROWS-1;
				break;
		}
	} while (player.x == 2);

	// position enemy in opposite quadrant
	do {
		if (player.x < 2)
		{
			enemy.x = Platform_Random(3) + 2;
		}
		else if (player.x > 2)
		{
			enemy.x = Platform_Random(3);
		}

		if (player.y < 2)
		{
			enemy.y = Platform_Random(2) + 2;
		}
		else if (player.y >= 2)
		{
			enemy.y = Platform_Random(2);
		}
		// keep out of corners
	} while (((enemy.x==0) && (enemy.y==0))
		|| ((enemy.x==0) && (enemy.y==3))
		|| ((enemy.x==4) && (enemy.y==0))
		|| ((enemy.x==4) && (enemy.y==3)));

	nPlayerStartX = player.x;
	nPlayerStartY = player.y;

}

void PlantMines()
{
	int i;

	// clear the mines
	for (i=0; i<4; i++)
	{
		mines[i].x = -1;
		mines[i].y = -1;
	}

	// pick new random positions
	BOOL bTooClose;
	do {

		bTooClose = FALSE;

		// scatter mines, keeping off of existing mines, player, and enemy
		do {
			mines[0].x = Platform_Random(ARMORBATTLE_BLIP_COLUMNS);
			mines[0].y = Platform_Random(ARMORBATTLE_BLIP_ROWS);
		} while (((mines[0].x == player.x) && (mines[0].y == player.y))
			|| ((mines[0].x == enemy.x) && (mines[0].y == enemy.y))
			|| ((mines[0].x == mines[1].x) && (mines[0].y == mines[1].y))
			|| ((mines[0].x == mines[2].x) && (mines[0].y == mines[2].y))
			|| ((mines[0].x == mines[3].x) && (mines[0].y == mines[3].y)));

		do {
			mines[1].x = Platform_Random(ARMORBATTLE_BLIP_COLUMNS);
			mines[1].y = Platform_Random(ARMORBATTLE_BLIP_ROWS);
		} while (((mines[1].x == player.x) && (mines[1].y == player.y))
			|| ((mines[1].x == enemy.x) && (mines[1].y == enemy.y))
			|| ((mines[1].x == mines[0].x) && (mines[1].y == mines[0].y))
			|| ((mines[1].x == mines[2].x) && (mines[1].y == mines[2].y))
			|| ((mines[1].x == mines[3].x) && (mines[1].y == mines[3].y)));

		do {
			mines[2].x = Platform_Random(ARMORBATTLE_BLIP_COLUMNS);
			mines[2].y = Platform_Random(ARMORBATTLE_BLIP_ROWS);
		} while (((mines[2].x == player.x) && (mines[2].y == player.y))
			|| ((mines[2].x == enemy.x) && (mines[2].y == enemy.y))
			|| ((mines[2].x == mines[0].x) && (mines[2].y == mines[0].y))
			|| ((mines[2].x == mines[1].x) && (mines[2].y == mines[1].y))
			|| ((mines[2].x == mines[3].x) && (mines[2].y == mines[3].y)));

		// make sure there is not a mine right next to the player's
		// starting position
		for (i=0; i<4; i++)
		{
			if (((mines[i].x+1 == player.x) && (mines[i].y == player.y))
				|| ((mines[i].x-1 == player.x) && (mines[i].y == player.y))
				|| ((mines[i].x == player.x) && (mines[i].y+1 == player.y))
				|| ((mines[i].x == player.x) && (mines[i].y-1 == player.y)))
			{
				bTooClose = TRUE;
			}
		}

	} while (bTooClose);

}

