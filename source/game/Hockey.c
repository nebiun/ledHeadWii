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

#include "Hockey.h"
#include "Games.h"


// constants

#define NUM_DEFENSEPLAYERS	4

#define TIME_TICKTIMERPRO1	7
#define TIME_TICKTIMERPRO2	4
#define TIME_STATSDISPLAY	5
#define TIME_PUCKMOVE		3
#define TIME_PENALTY		20 // (2 simulated minutes)

typedef int BLIP;

static BLIP Blips[HOCKEY_BLIP_COLUMNS][HOCKEY_BLIP_ROWS];

// game variables
static BOOL bGameOver;
static BOOL bHomeTeam;
static BOOL bInFrame = FALSE;
static BOOL bPower;
static BOOL bPro2 = FALSE;

static int nHScore;
static int nVScore;
static float fGameTime;
static int nPeriod;
static int nTimerTickTimer;

static BOOL bShoot;
static int nPuckDirectionX;
static int nPuckDirectionY;
static int nTimerPuckMove;

static BOOL bBumped;
static int nPenalty;
static int nPenaltyTimer;

static BOOL bDeflect;
static BOOL bSteal;

static int nStatsIndex;
static int nTimerStatsDisplay;

static int nBehindGoalDirX = 0;

static int nDeflectPointX = 0;
static int nDeflectPointY = 0;

typedef struct PLAYER {
	int nColumn;
	int nRow;
	int nBright;
	int nColumnOld;
	int nRowOld;
}PLAYER;

static PLAYER puck;
static PLAYER player;
static PLAYER defense[NUM_DEFENSEPLAYERS];

// macros for dealing with the players

#define SETPLAYERBRIGHTNESS(p,b)	{ \
	p.nBright = b; \
}

#define NOTECURRENTPLAYERPOSITION(p) { \
	p.nColumnOld = p.nColumn; \
	p.nRowOld = p.nRow; \
}

#define SETPLAYER(p,x,y,b)	{ \
	NOTECURRENTPLAYERPOSITION(p); \
	p.nColumn = x; \
	p.nRow = y; \
	p.nBright = b; \
}

#define UNSETPLAYER(p)	{ \
	NOTECURRENTPLAYERPOSITION(p); \
	p.nColumn = -1; \
	p.nRow = -1; \
	p.nBright = BLIP_OFF; \
}

#define MOVEPLAYERUP(p)		{ \
	NOTECURRENTPLAYERPOSITION(p); \
	p.nRow--; \
}
#define MOVEPLAYERDOWN(p)	{ \
	NOTECURRENTPLAYERPOSITION(p); \
	p.nRow++; \
}
#define MOVEPLAYERLEFT(p)	{ \
	NOTECURRENTPLAYERPOSITION(p); \
	p.nColumn--; \
}
#define MOVEPLAYERRIGHT(p)	{ \
	NOTECURRENTPLAYERPOSITION(p); \
	p.nColumn++; \
}

static BOOL ISPUCK(int x, int y);
static BOOL ISPUCK(int x, int y)
{
	if ((puck.nColumn == x)
		&& (puck.nRow == y)
		&& (puck.nBright)){
		return TRUE;
	}
	return FALSE;
}

static BOOL ISPLAYER(int x, int y);
static BOOL ISPLAYER(int x, int y)
{
	if ((player.nColumn == x)
		&& (player.nRow == y)
		&& (player.nBright)){
		return TRUE;
	}
	return FALSE;
}

static BOOL ISDEFENSE(int x, int y);
static BOOL ISDEFENSE(int x, int y)
{
	for (int i=0; i<NUM_DEFENSEPLAYERS; i++){
		if ((defense[i].nColumn == x)
			&& (defense[i].nRow == y)
			&& (defense[i].nBright)){
			return TRUE;
		}
	}
	return FALSE;
}

static BOOL ISOCCUPIED(int x, int y);
static BOOL ISOCCUPIED(int x, int y)
{
	// easy way to keep player and defenders out of the non-playfield areas
	if (((x==2) && (y==1))		// the goal
		|| ((x==0) && (y==0))
		|| ((x==1) && (y==0))
		|| ((x==3) && (y==0))
		|| ((x==4) && (y==0))
		|| ((x==0) && (y==1))
		|| ((x==4) && (y==1)))
	{
		return TRUE;
	}

	if (ISPLAYER(x,y)){
		return TRUE;
	}
	if (ISDEFENSE(x,y)){
		return TRUE;
	}
	return FALSE;
}

static int GETPLAYERAT(int x, int y);
static int GETPLAYERAT(int x, int y){
	for (int i=0; i<NUM_DEFENSEPLAYERS; i++){
		if ((defense[i].nColumn == x)
			&& (defense[i].nRow == y)
			&& (defense[i].nBright)){
			return i;
		}
	}
	return -1;
}

#define UNMOVEPLAYER(p) { \
	p.nRow = p.nRowOld; \
	p.nColumn = p.nColumnOld; \
}

#define ISPLAYEROFFSCREEN(p) \
	((p.nColumn > HOCKEY_BLIP_COLUMNS-1) \
	|| (p.nColumn < 0) \
	|| (p.nRow > HOCKEY_BLIP_ROWS-1) \
	|| (p.nRow < 0))


#define ISPLAYERENABLED(p) \
	(p.nBright)


// finite state machine stuff

static void fsmPlayStartWait();
static void fsmShowStats();
static void fsmInPlay();
static void fsmGoal();
static void fsmGameOver();


static enum FSM {
	FSM_PLAYSTARTWAIT=0,
	FSM_SHOWSTATS,
	FSM_INPLAY,
	FSM_GOAL,
	FSM_GAMEOVER
}fsm;

typedef void (*FSMFCN)();

static FSMFCN fsmfcn[] = {
	fsmPlayStartWait,
	fsmShowStats,
	fsmInPlay,
	fsmGoal,
	fsmGameOver
};


// proto's
static void InitGame();
static void DrawBlips();
static void EraseBlips();
static BOOL PenaltyTest(int x, int y);


BOOL Hockey_GetPower()
{
	return (bPower ? TRUE : FALSE);
}

void Hockey_PowerOn()
{
	InitGame();
	bPower = TRUE;
}

void Hockey_PowerOff()
{
	bPower = FALSE;
	Hockey_StopSound();
}

void Hockey_SetSkill(int i){
	if (i == 0){
		bPro2 = FALSE;
	} else {
		bPro2 = TRUE;
	}
}

int Hockey_GetSkill(){
	return bPro2 ? 1 : 0;
}

void InitGame()
{
	bHomeTeam = FALSE;
	PlatformSetInput(bHomeTeam);
	
	nHScore = 0;
	nVScore = 0;
	fGameTime = (float)20.9;
	nPeriod = 0;
	bGameOver = FALSE;
	nBehindGoalDirX = 0;
	bBumped = FALSE;
	nPenalty = 0;
	nPenaltyTimer = 0;
	bDeflect = FALSE;
	bSteal = FALSE;

	// show stats when power is switched on
	nStatsIndex = 0;
	nTimerStatsDisplay = TIME_STATSDISPLAY;
	fsm = FSM_SHOWSTATS;
	fsmShowStats();
}

void Hockey_Run(int tu)
{
	int x, y;

	// prevent reentrancy
	if (bInFrame){ return; }
	bInFrame = TRUE;

	// init the blips field
	for (y = 0; y < HOCKEY_BLIP_ROWS; y++){
		for (x = 0; x < HOCKEY_BLIP_COLUMNS; x++){
			Blips[x][y] = BLIP_OFF;
		}
	}

	if (!bPower){
		Hockey_ClearScreen();
		bInFrame = FALSE;
		return;
	}

	Platform_StartDraw();

	(fsmfcn[fsm])();

	DrawBlips();

	Platform_EndDraw();

	bInFrame = FALSE;

}

void DrawBlips()
{
	int x, y, nBright;
	static BOOL bBlink = FALSE;

	if(fsm != FSM_GOAL) {
		for (int i=0; i<NUM_DEFENSEPLAYERS; i++){
			if (defense[i].nBright != BLIP_OFF){
				x = defense[i].nColumn;
				y = defense[i].nRow;
				nBright = defense[i].nBright;
				Blips[x][y] = nBright;
			}
		}
		if (player.nBright != BLIP_OFF){
			x = player.nColumn;
			y = player.nRow;
			nBright = player.nBright;
			Blips[x][y] = nBright;
		}
		if (puck.nBright != BLIP_OFF){
			x = puck.nColumn;
			y = puck.nRow;
			nBright = puck.nBright;
			Blips[x][y] = nBright;
		}

		// draw the penalty blips
		if (nPenalty == -1)
		{
			Blips[0][1] = BLIP_DIM;
		}
		else if (nPenalty == 1)
		{
			Blips[4][1] = BLIP_DIM;
		}
	}

	// draw the blips field
	for (y = 0; y < HOCKEY_BLIP_ROWS; y++){
		for (x = 0; x < HOCKEY_BLIP_COLUMNS; x++){

			switch(Blips[x][y]){
				case BLIP_OFF:
				case BLIP_DIM:
				case BLIP_BRIGHT:
					Hockey_DrawBlip(Blips[x][y], x, y);
					break;
				case BLIP_DIMBLINK:
					if (!bBlink){
						Hockey_DrawBlip(BLIP_DIM, x, y);
					} else {
						Hockey_DrawBlip(BLIP_OFF, x, y);
					}
					break;
				case BLIP_BRIGHTBLINK:
					if (bBlink){
						Hockey_DrawBlip(BLIP_BRIGHT, x, y);
					} else {
						Hockey_DrawBlip(BLIP_OFF, x, y);
					}
					break;
			}

		}
	}

	bBlink = !bBlink;
}

void EraseBlips()
{
	// erase the blips field
	for (int y = 1; y < HOCKEY_BLIP_ROWS; y++){
		for (int x = 0; x < HOCKEY_BLIP_COLUMNS; x++){
			Hockey_DrawBlip(BLIP_OFF, x, y);
		}
	}

	// erase the blip behind the goal
	Hockey_DrawBlip(BLIP_OFF, 2, 0);
}

BOOL PenaltyTest(int x, int y)
{
	if (ISDEFENSE(x, y))
	{
		if (!bBumped)
		{
			bBumped = TRUE;
			Hockey_PlaySound(HOCKEY_SOUND_BUMP, PLAYSOUNDFLAGS_ASYNC|PLAYSOUNDFLAGS_PRIORITY);
		}
		else
		{
			// penalty
			nPenalty = bHomeTeam ? -1 : 1;
			nPenaltyTimer = TIME_PENALTY;

			Hockey_ClearScreen();
			Hockey_PlaySound(HOCKEY_SOUND_PENALTY, PLAYSOUNDFLAGS_PRIORITY);

			UNSETPLAYER(puck);
			SETPLAYERBRIGHTNESS(player, BLIP_BRIGHT);

			bHomeTeam = !bHomeTeam;
			PlatformSetInput(bHomeTeam);

			nStatsIndex = 0;
			nTimerStatsDisplay = TIME_STATSDISPLAY;
			fsm = FSM_SHOWSTATS;

			Platform_IsNewSecond();
			bShoot = FALSE;
			return TRUE;
		}
	}

	return FALSE;
}

// FINITE STATE MACHINE STUFF

void fsmPlayStartWait()
{
	// show formation
	{
		SETPLAYER(player, ((bHomeTeam) ? 0 : 4), 4, BLIP_BRIGHT);
		UNSETPLAYER(puck);
		SETPLAYER(defense[0], 2, 2, BLIP_DIM);
		SETPLAYER(defense[1], 1, 2, BLIP_DIM);
		SETPLAYER(defense[2], 3, 2, BLIP_DIM);
		if (!nPenalty)
		{
			SETPLAYER(defense[3], 2, 3, BLIP_DIM);
		}
		else
		{
			UNSETPLAYER(defense[3]);
		}
	}

	bShoot = FALSE;
	nTimerTickTimer = bPro2 ? TIME_TICKTIMERPRO2 : TIME_TICKTIMERPRO1;
	bBumped = FALSE;
	bDeflect = FALSE;
	bSteal = FALSE;

	// show stats if pressed 'down'
	// else, wait for player to move then advance to next state
	BOOL bChange;
	if (Hockey_GetInputDOWN(&bChange))
	{
		if (bChange)
		{
			nStatsIndex = 0;
			nTimerStatsDisplay = TIME_STATSDISPLAY;
			fsm = FSM_SHOWSTATS;
			fsmShowStats();
			return;
		}
	}
	else if (Hockey_TestForMovement())
	{
		Platform_IsNewSecond();
		fsmInPlay();
		fsm = FSM_INPLAY;
		return;
	}

	Hockey_DrawStat((int)fGameTime);

}

void fsmShowStats()
{
	// show formation
	if (!bGameOver)
	{
		SETPLAYER(player, ((bHomeTeam) ? 0 : 4), 4, BLIP_BRIGHT);
		UNSETPLAYER(puck);
		SETPLAYER(defense[0], 2, 2, BLIP_DIM);
		SETPLAYER(defense[1], 1, 2, BLIP_DIM);
		SETPLAYER(defense[2], 3, 2, BLIP_DIM);
		if (!nPenalty)
		{
			SETPLAYER(defense[3], 2, 3, BLIP_DIM);
		}
		else
		{
			UNSETPLAYER(defense[3]);
		}
	}

	if (--nTimerStatsDisplay <= 0)
	{
		++nStatsIndex;
		nTimerStatsDisplay = TIME_STATSDISPLAY;
	}
	switch(nStatsIndex)
	{
		case 0:
			Hockey_DrawStat(-1);
			break;
		case 1:
			Hockey_DrawStat(nVScore);
			break;
		case 2:
			Hockey_DrawStat(-1);
			break;
		case 3:
			Hockey_DrawStat(nHScore);
			break;
		case 4:
			Hockey_DrawStat(-1);
			break;
		default:
		case 5:
			if (bGameOver)
			{
				fsm = FSM_GAMEOVER;
			}
			else
			{
				fsm = FSM_PLAYSTARTWAIT;
			}
			break;
	}
}

void fsmInPlay()
{
	BOOL bChange;
	if (Hockey_GetInputLEFT(&bChange))
	{
		if (bChange)
		{
			if (player.nColumn > 0)
			{
				// handle special behavior behind the goal
				if ((player.nRow == 1) && (player.nColumn == 1))
				{
					if (PenaltyTest(0, 2))
					{
						return;
					}
					else if (!ISOCCUPIED(0, 2))
					{
						SETPLAYER(player, 0, 2, player.nBright);
					}
				}
				else if ((player.nRow == 0) && (player.nColumn == 2))
				{
					if (PenaltyTest(1, 1))
					{
						return;
					}
					else if (!ISOCCUPIED(1, 1))
					{
						SETPLAYER(player, 1, 1, player.nBright);
					}
				}
				else
				{
					if (PenaltyTest(player.nColumn-1, player.nRow))
					{
						return;
					}
					else if (!ISOCCUPIED(player.nColumn-1, player.nRow))
					{
						MOVEPLAYERLEFT(player);
					}
				}
			}

		}
	}
	else if (Hockey_GetInputRIGHT(&bChange))
	{
		if (bChange)
		{
			if (player.nColumn < (HOCKEY_BLIP_COLUMNS-1))
			{
				// handle special behavior behind the goal
				if ((player.nRow == 1) && (player.nColumn == 3))
				{
					if (PenaltyTest(4, 2))
					{
						return;
					}
					else if (!ISOCCUPIED(4, 2))
					{
						SETPLAYER(player, 4, 2, player.nBright);
					}
				}
				else if ((player.nRow == 0) && (player.nColumn == 2))
				{
					if (PenaltyTest(3, 1))
					{
						return;
					}
					else if (!ISOCCUPIED(3, 1))
					{
						SETPLAYER(player, 3, 1, player.nBright);
					}
				}
				else
				{
					if (PenaltyTest(player.nColumn+1, player.nRow))
					{
						return;
					}
					else if (!ISOCCUPIED(player.nColumn+1, player.nRow))
					{
						MOVEPLAYERRIGHT(player);
					}
				}
			}
		}
	}
	else if (Hockey_GetInputUP(&bChange))
	{
		if (bChange)
		{
  			if (player.nRow > 2)
			{
				if (PenaltyTest(player.nColumn, player.nRow-1))
				{
					return;
				}
				else if (!ISOCCUPIED(player.nColumn, player.nRow-1))
				{
					MOVEPLAYERUP(player);
				}
			}

			// handle special behavior behind the goal 
			else if (player.nRow == 1)
			{
				if ((player.nColumn == 3) || (player.nColumn == 1))
				{
					if (PenaltyTest(2, 0))
					{
						return;
					}
					else if (!ISOCCUPIED(2, 0))
					{
						SETPLAYER(player, 2, 0, player.nBright);
					}
				}
			}
			else if (player.nRow == 2)
			{
				switch(player.nColumn)
				{
					case 0:
					case 1:
						if (PenaltyTest(1, 1))
						{
							return;
						}
						else if (!ISOCCUPIED(1, 1))
						{				
							SETPLAYER(player, 1, 1, player.nBright);
						}
						break;
					case 2:
						// the goal - can't go there
						break;
					case 3:
					case 4:
						if (PenaltyTest(3, 1))
						{
							return;
						}
						else if (!ISOCCUPIED(3, 1))
						{				
							SETPLAYER(player, 3, 1, player.nBright);
						}
						break;
				}
			}
		}
	}
	else if (Hockey_GetInputDOWN(&bChange))
	{
		if (bChange)
		{
			if (player.nRow < (HOCKEY_BLIP_ROWS-1))
			{
				if (PenaltyTest(player.nColumn, player.nRow+1))
				{
					return;
				}
				else if (!ISOCCUPIED(player.nColumn, player.nRow+1))
				{
					MOVEPLAYERDOWN(player);
				}
			}

		}
	}

	if (Hockey_GetInputTHROW(&bChange))
	{
		if (bChange)
		{
 			if (!ISPLAYERENABLED(puck))
			{
				// shoot!
				bShoot = TRUE;
				nTimerPuckMove = TIME_PUCKMOVE;
				SETPLAYERBRIGHTNESS(player, BLIP_DIMBLINK);
				SETPLAYER(puck, player.nColumn, player.nRow, BLIP_BRIGHT);

				if (player.nRow < 2)
				{
					nPuckDirectionX = 0;
					nPuckDirectionY = 0;

					if (player.nColumn < 2)
					{
						nBehindGoalDirX = 1;
					}
					else if (player.nColumn > 2)
					{
						nBehindGoalDirX = -1;
					}
					else
					{
						nBehindGoalDirX = Platform_Random(2) ? 1 : -1;
					}
				}
				else
				{
					nPuckDirectionY = -1;
					nBehindGoalDirX = 0;

					if (player.nColumn < 2)
					{
						nPuckDirectionX = 1;
					}
					else if (player.nColumn > 2)
					{
						nPuckDirectionX = -1;
					}
					else
					{
						nPuckDirectionX = 0;
					}
				}

			}
		}
	}

	// let the defenders try to pokecheck
	// it's easier to check all 4 positions around the player than
	// to check each defender separately
	BOOL bPoked = FALSE;
	if (!ISPLAYERENABLED(puck))
	{
		if (player.nRow < 2)
		{
			// handle behind the goal
			if ((((player.nColumn == 2) || (player.nRow == 0)) && ISDEFENSE(1, 1))
				|| (((player.nColumn == 2) || (player.nRow == 0)) && ISDEFENSE(3, 1))
				|| (((player.nColumn == 1) || (player.nRow == 1)) && ISDEFENSE(2, 0))
				|| (((player.nColumn == 3) || (player.nRow == 1)) && ISDEFENSE(2, 1))
				|| (((player.nColumn == 1) || (player.nRow == 1)) && ISDEFENSE(1, 2))
				|| (((player.nColumn == 3) || (player.nRow == 1)) && ISDEFENSE(3, 2))
				|| (((player.nColumn == 1) || (player.nRow == 1)) && ISDEFENSE(0, 2)) // not sure about this
				|| (((player.nColumn == 3) || (player.nRow == 1)) && ISDEFENSE(4, 2))) // not sure about this
			{
				if (Platform_Random(bPro2 ? 40 : 50) == 0)
				{
					bPoked = TRUE;
				}
			}
		}
		else
		{
			if ((player.nColumn > 0) && ISDEFENSE(player.nColumn-1, player.nRow))
			{
				// left
				if (Platform_Random(bPro2 ? 40 : 50) == 0)
				{
					bPoked = TRUE;
				}
			}
			if ((player.nColumn < (HOCKEY_BLIP_COLUMNS-1)) && ISDEFENSE(player.nColumn+1, player.nRow))
			{
				// right
				if (Platform_Random(bPro2 ? 40 : 50) == 0)
				{
					bPoked = TRUE;
				}
			}
			if ((player.nRow > 0) && ISDEFENSE(player.nColumn, player.nRow+1))
			{
				// above
				if (Platform_Random(bPro2 ? 40 : 50) == 0)
				{
					bPoked = TRUE;
				}
			}
			if ((player.nRow < (HOCKEY_BLIP_ROWS-1)) && ISDEFENSE(player.nColumn, player.nRow-1))
			{
				// below
				if (Platform_Random(bPro2 ? 40 : 50) == 0)
				{
					bPoked = TRUE;
				}
			}
		}
	}

	if (bPoked)
	{
		Hockey_PlaySound(HOCKEY_SOUND_POKE, PLAYSOUNDFLAGS_PRIORITY);

		bShoot = TRUE;
		nTimerPuckMove = TIME_PUCKMOVE;
		SETPLAYERBRIGHTNESS(player, BLIP_DIMBLINK);
		SETPLAYER(puck, player.nColumn, player.nRow, BLIP_BRIGHT);

		if (player.nRow < 2)
		{
			nPuckDirectionX = 0;
			nPuckDirectionY = 0;

			if (player.nColumn < 2)
			{
				nBehindGoalDirX = 1;
			}
			else if (player.nColumn > 2)
			{
				nBehindGoalDirX = -1;
			}
			else
			{
				nBehindGoalDirX = Platform_Random(2) ? 1 : -1;
			}
		}
		else
		{
			do
			{
				if (player.nColumn == 0)
				{
					nPuckDirectionX = Platform_Random(2);
				}
				else if (player.nColumn == (HOCKEY_BLIP_COLUMNS-1))
				{
					nPuckDirectionX = -(Platform_Random(2));
				}
				else
				{
					nPuckDirectionX = Platform_Random(3) - 1;
				}

				if (player.nRow == 0)
				{
					nPuckDirectionY = Platform_Random(2);
				}
				else if (player.nRow == (HOCKEY_BLIP_ROWS-1))
				{
					nPuckDirectionY = -(Platform_Random(2));
				}
				else
				{
					nPuckDirectionY = Platform_Random(3) - 1;
				}
			} while((nPuckDirectionX == 0) && (nPuckDirectionY == 0));

			nBehindGoalDirX = 0;
		}
	}


	// establish a target for the defense
	PLAYER *targetP = ISPLAYERENABLED(puck) ? &puck : &player;

	// move the goalie
	{
		if (Platform_Random(bPro2 ? 3 : 6) == 0)
		{
			if ((targetP->nColumn < defense[0].nColumn)
				&& (defense[0].nColumn > 1))
			{
				if (!ISOCCUPIED(defense[0].nColumn-1, defense[0].nRow))
				{
					defense[0].nColumn--;
				}
			}
			else if ((targetP->nColumn > defense[0].nColumn)
				&& (defense[0].nColumn < 3))
			{
				if (!ISOCCUPIED(defense[0].nColumn+1, defense[0].nRow))
				{
					defense[0].nColumn++;
				}
			}
		}
	}

	// move the other defenders toward the target
	{
		if (Platform_Random(bPro2 ? 5 : 10) == 0)
		{
			// pick a random defender
			static int nDefenderLast = -1;
			int nDefender = nPenalty ? (Platform_Random(2) + 1) : (Platform_Random(3) + 1);

			if (nDefender != nDefenderLast)
			{
				nDefenderLast = nDefender;

				PLAYER *pDefender = &defense[nDefender];

				int dx = pDefender->nColumn;
				int dy = pDefender->nRow;

				// randomly move the defender towards the target
				if (Platform_Random(2))
				{
					if (dx < targetP->nColumn)
					{
						++dx;
					}
					else if (dx > targetP->nColumn)
					{
						--dx;
					}
				}
				else
				{
					if (dy < targetP->nRow)
					{
						++dy;
					}
					else if (dy > targetP->nRow)
					{
						--dy;
					}
				}

				// occasionally randomly offset the defender's position
				if (Platform_Random(5) == 0)
				{
					if (Platform_Random(2))
					{
						dx += (Platform_Random(2)) ? 1 : -1;
					}
					else 
					{
						dy += (Platform_Random(2)) ? 1 : -1;
					}
				}

				// need to allow defenders to move around behind goal, etc
				if ((dx == 2) && (dy == 1))
				{
					if ((pDefender->nColumn == 1) && (pDefender->nRow == 1))
					{
						dx = 2;
						dy = 0;	
					}
					else if ((pDefender->nColumn == 2) && (pDefender->nRow == 0))
					{
						dx = 2;
						dy = 0;	
					}
					else if ((pDefender->nColumn == 3) && (pDefender->nRow == 1))
					{
						dx = 2;
						dy = 0;	
					}
				}
				else if ((dx == 1) && (dy == 0))
				{
					dx = 1;
					dy = 1;	
				}
				else if ((dx == 3) && (dy == 0))
				{
					dx = 3;
					dy = 1;	
				}
				else if ((dx == 0) && (dy == 1))
				{
					dx = 0;
					dy = 2;	
				}
				else if ((dx == 4) && (dy == 1))
				{
					dx = 4;
					dy = 2;	
				}

				// make sure the defender does not step on anybody,
				// and keep him confined to the proper area
				if (!ISOCCUPIED(dx, dy)
					&& (dx >= 0)
					&& (dx <= (HOCKEY_BLIP_COLUMNS - 1))
					&& (dy >= 0)
					&& (dy <= (HOCKEY_BLIP_ROWS - 1)))
				{
					SETPLAYER((*pDefender), dx, dy, BLIP_DIM);
				}

			}
		}
	}

	// move the puck
	if (ISPLAYERENABLED(puck))
	{
		// the puck hit a defender last frame - do something
		if (ISDEFENSE(puck.nColumn, puck.nRow) && !bDeflect)
		{
			bDeflect = TRUE;
			nDeflectPointX = puck.nColumn;
			nDeflectPointY = puck.nRow;
			if (Platform_Random(10) == 0)
			{
				bSteal = TRUE;
			}
			else
			{
				Hockey_PlaySound(HOCKEY_SOUND_DEFLECT, PLAYSOUNDFLAGS_PRIORITY);
				Platform_Pause(120);
			}
		}

		if (--nTimerPuckMove <= 0)
		{
			nTimerPuckMove = TIME_PUCKMOVE;

			// handle the puck behind the goal

			if ((puck.nRow < 2)
				|| ((puck.nRow == 2) && (puck.nColumn == 0))
				|| ((puck.nRow == 2) && (puck.nColumn == 4)))
			{
				if ((puck.nColumn == 2) && (puck.nRow == 1))
				{
					// ** goal! **
					nPenalty = 0;
					nPenaltyTimer = 0;
					nBehindGoalDirX = 0;
					nPuckDirectionX = 0;
					nPuckDirectionY = 0;

					Hockey_ClearScreen();
					fsm = FSM_GOAL;
					return;
				}

				if ((nPuckDirectionY == -1)
					&& !nBehindGoalDirX)
				{
					if (puck.nColumn < 2)
					{
						nBehindGoalDirX = 1;
					}
					else if (puck.nColumn > 2)
					{
						nBehindGoalDirX = -1;
					}
				}
				
				if (nBehindGoalDirX == 1)
				{
					if ((puck.nColumn == 0) && (puck.nRow == 2))
					{
						puck.nColumn = 1;
						puck.nRow = 1;
					}
					else if ((puck.nColumn == 1) && (puck.nRow == 1))
					{
						puck.nColumn = 2;
						puck.nRow = 0;
					}
					else if ((puck.nColumn == 2) && (puck.nRow == 0))
					{
						puck.nColumn = 3;
						puck.nRow = 1;
					}
					else if ((puck.nColumn == 3) && (puck.nRow == 1))
					{
						puck.nColumn = 4;
						puck.nRow = 1;// should be 2 but gets incremented below;
						nPuckDirectionX = 0;
						nPuckDirectionY = 1;
						nBehindGoalDirX = 0;
					}
				}
				else if (nBehindGoalDirX == -1)
				{
					if ((puck.nColumn == 4) && (puck.nRow == 2))
					{
						puck.nColumn = 3;
						puck.nRow = 1;
					}
					else if ((puck.nColumn == 3) && (puck.nRow == 1))
					{
						puck.nColumn = 2;
						puck.nRow = 0;
					}
					else if ((puck.nColumn == 2) && (puck.nRow == 0))
					{
						puck.nColumn = 1;
						puck.nRow = 1;
					}
					else if ((puck.nColumn == 1) && (puck.nRow == 1))
					{
						puck.nColumn = 0;
						puck.nRow = 1;// should be 2 but gets incremented below
						nPuckDirectionX = 0;
						nPuckDirectionY = 1;
						nBehindGoalDirX = 0;
					}
				}

			}

			// handle the puck hitting the walls
			
			if ((puck.nColumn <= 0) && (nPuckDirectionX == -1))
			{
				// bounce off left wall
				puck.nColumn = 0;
				do {
					// pick a new direction
					nPuckDirectionX = Platform_Random(2);
					switch(Platform_Random(4))
					{
						case 0:
							nPuckDirectionY = -1;
							break;
						case 1:
							nPuckDirectionY = 0;
							break;
						default:
							// puck is more likely to go up
							nPuckDirectionY = 1;
					}
				} while ((nPuckDirectionX==0) && (nPuckDirectionY==0));
			}
			else if ((puck.nColumn >= (HOCKEY_BLIP_COLUMNS-1)) && (nPuckDirectionX == 1))
			{
				// bounce off right wall
				puck.nColumn = (HOCKEY_BLIP_COLUMNS-1);
				do {
					// pick a new direction
					nPuckDirectionX = -(Platform_Random(2));
					switch(Platform_Random(4))
					{
						case 0:
							nPuckDirectionY = -1;
							break;
						case 1:
							nPuckDirectionY = 0;
							break;
						default:
							// puck is more likely to go up
							nPuckDirectionY = 1;
					}
				} while ((nPuckDirectionX==0) && (nPuckDirectionY==0));
			}

			if ((puck.nRow >= (HOCKEY_BLIP_ROWS-1)) && (nPuckDirectionY == 1))
			{
				// bounce off rear wall
				puck.nRow = (HOCKEY_BLIP_ROWS-1);
				if ((nPuckDirectionX == 0)
					&& ((puck.nColumn == 0) || (puck.nColumn == (HOCKEY_BLIP_COLUMNS-1))))
				{
					// puck follows the sides
					nPuckDirectionY = - 1;
				}
				else {
					do {
						// pick a new direction
						if (puck.nColumn <= 0)
						{
							nPuckDirectionX = Platform_Random(2);
						}
						else if (puck.nColumn >= (HOCKEY_BLIP_COLUMNS-1))
						{
							nPuckDirectionX = -(Platform_Random(2));
						}
						else
						{
							nPuckDirectionX = Platform_Random(3) - 1;
						}
						nPuckDirectionY = -(Platform_Random(2));
					} while ((nPuckDirectionX==0) && (nPuckDirectionY==0));
				}
			}

			// the puck has hit a defender
//			if (ISDEFENSE(puck.nColumn, puck.nRow))
			if (bDeflect)
			{
				// let the defenders try to steal the puck
				//if (Platform_Random(10) == 0)
				if (bSteal)
				{
					nPenalty = 0;
					nPenaltyTimer = 0;
					nBehindGoalDirX = 0;
					nPuckDirectionX = 0;
					nPuckDirectionY = 0;

					Hockey_ClearScreen();
					Hockey_PlaySound(HOCKEY_SOUND_STEAL, PLAYSOUNDFLAGS_PRIORITY);

					UNSETPLAYER(puck);
					SETPLAYERBRIGHTNESS(player, BLIP_BRIGHT);

					bHomeTeam = !bHomeTeam;
					PlatformSetInput(bHomeTeam);

					nStatsIndex = 0;
					nTimerStatsDisplay = TIME_STATSDISPLAY;
					fsm = FSM_SHOWSTATS;

					Platform_IsNewSecond();
					bShoot = FALSE;
					bDeflect = FALSE;
					bSteal = FALSE;
					return;
				}
				else
				{
					// bounce the puck off the defender
					if (GETPLAYERAT(puck.nColumn, puck.nRow) == 0)
					{
						// goalie always hits the puck down (i think)
						nPuckDirectionY = 1;
						nPuckDirectionX = Platform_Random(3)-1;
					}
					else
					{
						if (nDeflectPointY >= 2)
						{
							do {
								// pick a new direction
								if (puck.nColumn <= 0)
								{
									nPuckDirectionX = Platform_Random(2);
								}
								else if (puck.nColumn >= (HOCKEY_BLIP_COLUMNS-1))
								{
									nPuckDirectionX = -(Platform_Random(2));
								}
								else
								{
									nPuckDirectionX = Platform_Random(3) - 1;
								}

								if (puck.nRow <= 0)
								{
									nPuckDirectionY = Platform_Random(2);
								}
								else if (puck.nRow >= (HOCKEY_BLIP_ROWS-1))
								{
									nPuckDirectionY = -(Platform_Random(2));
								}
								else
								{
									nPuckDirectionY = Platform_Random(3) - 1;
								}
							} while ((nPuckDirectionX==0) && (nPuckDirectionY==0));
						}
					}
				}
			}
			// see if player has regained possession
			else if (ISPLAYER(puck.nColumn, puck.nRow) && !bShoot)
			{
				UNSETPLAYER(puck);
				SETPLAYERBRIGHTNESS(player, BLIP_BRIGHT);
			}

			// move the puck
			if (!nBehindGoalDirX)
			{
				puck.nColumn += nPuckDirectionX;
				puck.nRow += nPuckDirectionY;

				// special check for hitting the corners on the curve
				if ((puck.nColumn == 0) && (puck.nRow == 1))
				{
					puck.nColumn = 1;
					puck.nRow = 1;
					nBehindGoalDirX = 1;
				}
				else if ((puck.nColumn == 4) && (puck.nRow == 1))
				{
					puck.nColumn = 3;
					puck.nRow = 1;
					nBehindGoalDirX = -1;
				}
			}

			bDeflect = FALSE;
			bSteal = FALSE;
			bShoot = FALSE;
		}
	}

	// count down the clock
	if (Platform_IsNewSecond()){
		// check for end of quarter
		if (fGameTime > 0.0){
			fGameTime -= (float).1;
			if (fGameTime < 0.1)
			{
				// end of period
				nPenalty = 0;
				nPenaltyTimer = 0;

				fGameTime = 0.0;

				++nPeriod;
				if (nPeriod == 3)
				{
					// game over
					bGameOver = TRUE;

					// show formation
					{
						SETPLAYER(player, 4, 4, BLIP_BRIGHT);
						UNSETPLAYER(puck);
						SETPLAYER(defense[0], 2, 2, BLIP_DIM);
						SETPLAYER(defense[1], 1, 2, BLIP_DIM);
						SETPLAYER(defense[2], 3, 2, BLIP_DIM);
						SETPLAYER(defense[3], 2, 3, BLIP_DIM);
					}

					Hockey_ClearScreen();
					Hockey_PlaySound(HOCKEY_SOUND_ENDGAME, PLAYSOUNDFLAGS_PRIORITY);
					nStatsIndex = 0;
					nTimerStatsDisplay = TIME_STATSDISPLAY;
					fsm = FSM_SHOWSTATS;
					return;
				}
				else
				{
					// random possession
					bHomeTeam = Platform_Random(2) == 0 ? TRUE : FALSE;
					PlatformSetInput(bHomeTeam);
				}

				fGameTime = 20.0;
				Hockey_ClearScreen();
				Hockey_PlaySound(HOCKEY_SOUND_ENDPERIOD, PLAYSOUNDFLAGS_PRIORITY);
				nStatsIndex = 0;
				nTimerStatsDisplay = TIME_STATSDISPLAY;
				fsm = FSM_SHOWSTATS;
				return;
			}
			else
			{
				// see if its time for the penalty to be over
				if (nPenalty && (nPenaltyTimer > 0))
				{
					--nPenaltyTimer;
					if (nPenaltyTimer <= 0)
					{
						// take the player out of penalty
						int x, y;
						do 
						{
							x = Platform_Random(HOCKEY_BLIP_COLUMNS);
							y = Platform_Random(HOCKEY_BLIP_ROWS);
						} while (ISOCCUPIED(x, y));

						SETPLAYER(defense[3], x, y, BLIP_DIM);
						nPenalty = 0;
						nPenaltyTimer = 0;
					}
				}
			}
		}
	}

	Hockey_DrawStat((int)fGameTime);

	// make tick sounds
	if (fGameTime >= .1){
		if (--nTimerTickTimer <= 0){
			Hockey_PlaySound(HOCKEY_SOUND_TICK, PLAYSOUNDFLAGS_ASYNC);
			nTimerTickTimer = bPro2 ? TIME_TICKTIMERPRO2 : TIME_TICKTIMERPRO1;
		}
	}
}

void fsmGoal()
{
	Hockey_PlaySound(HOCKEY_SOUND_SCORE, PLAYSOUNDFLAGS_PRIORITY);

	UNSETPLAYER(puck);
	SETPLAYERBRIGHTNESS(player, BLIP_BRIGHT);

	if (bHomeTeam)
	{
		nHScore += 1;
	}
	else
	{
		nVScore += 1;
	}
	bHomeTeam = !bHomeTeam;
	PlatformSetInput(bHomeTeam);

	nStatsIndex = 0;
	nTimerStatsDisplay = TIME_STATSDISPLAY;
	fsm = FSM_SHOWSTATS;

	Platform_IsNewSecond();
	bShoot = FALSE;
}

void fsmGameOver()
{
	// show stats if pressed 'down'
	BOOL bChange;
	if (Hockey_GetInputDOWN(&bChange))
	{
		if (bChange)
		{
			nStatsIndex = 0;
			nTimerStatsDisplay = TIME_STATSDISPLAY;
			fsm = FSM_SHOWSTATS;
			fsmShowStats();
			return;
		}
	}

	Hockey_DrawStat(0);
}

