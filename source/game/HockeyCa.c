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

#include "HockeyCa.h"
#include "Games.h"

// constants

#define NUM_DEFENSEPLAYERS	6

#define TIME_TICKTIMERPRO1	7
#define TIME_TICKTIMERPRO2	4
#define TIME_STATSDISPLAY	5
#define TIME_PUCKMOVE		5
#define TIME_DEFLECT		2


typedef int BLIP;

static BLIP Blips[HOCKEYCA_BLIP_COLUMNS][HOCKEYCA_BLIP_ROWS];

// game variables
static BOOL bGameOver;
static BOOL bHomeTeam;
static BOOL bInFrame = FALSE;
static BOOL bPower;
static BOOL bPro2 = FALSE;

static int nHScore;
static int nVScore;
static float fGameTime;
static int nPlayTime;
static int nPeriod;
static int nTimerTickTimer;

static BOOL bShoot;
static int nBallDirectionX;
static int nBallDirectionY;
static int nTimerBallMove;

static BOOL bDeflect;
static int nTimerDeflect;

static int nStatsIndex;
static int nTimerStatsDisplay;

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

static BOOL ISPLAYER(int x, int y)
{
	if ((player.nColumn == x)
		&& (player.nRow == y)
		&& (player.nBright)){
		return TRUE;
	}
	return FALSE;
}

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

static BOOL ISOCCUPIED(int x, int y)
{
	if (ISPLAYER(x,y)){
		return TRUE;
	}
	if (ISDEFENSE(x,y)){
		return TRUE;
	}
	return FALSE;
}

#define UNMOVEPLAYER(p) { \
	p.nRow = p.nRowOld; \
	p.nColumn = p.nColumnOld; \
}

#define ISPLAYEROFFSCREEN(p) \
	((p.nColumn > HOCKEYCA_BLIP_COLUMNS-1) \
	|| (p.nColumn < 0) \
	|| (p.nRow > HOCKEYCA_BLIP_ROWS-1) \
	|| (p.nRow < 0))


#define ISPLAYERENABLED(p) \
	(p.nBright)

// evaluates to true if given position can block a shot
#define ISBLOCKINGPOS(x,y) \
	(  ((x)==2 && (y)==0) \
	|| ((x)==2 && (y)==1) \
	|| ((x)==2 && (y)==2) \
	|| ((x)==1 && (y)==1) \
	|| ((x)==3 && (y)==1) )


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

static void InitGame()
{
	bHomeTeam = FALSE;
	PlatformSetInput(bHomeTeam);
	
	nHScore = 0;
	nVScore = 0;
	fGameTime = (float)20.9;
	nPeriod = 0;
	bGameOver = FALSE;
	bDeflect = FALSE;

	fsm = FSM_PLAYSTARTWAIT;
}

static void DrawBlips()
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
	}

	// draw the blips field
	for (y = 0; y < HOCKEYCA_BLIP_ROWS; y++){
		for (x = 0; x < HOCKEYCA_BLIP_COLUMNS; x++){

			switch(Blips[x][y]){
			case BLIP_OFF:
			case BLIP_DIM:
			case BLIP_BRIGHT:
			case BLIP_DIMFLICKER:
				HockeyCa_DrawBlip(Blips[x][y], x, y);
				break;
			case BLIP_DIMBLINK:
				if (!bBlink){
					HockeyCa_DrawBlip(BLIP_DIM, x, y);
				} 
				else {
					HockeyCa_DrawBlip(BLIP_OFF, x, y);
				}
				break;
			case BLIP_BRIGHTBLINK:
				if (bBlink){
					HockeyCa_DrawBlip(BLIP_BRIGHT, x, y);
				} 
				else {
					HockeyCa_DrawBlip(BLIP_OFF, x, y);
				}
				break;
			}

		}
	}

	bBlink = !bBlink;
}

BOOL HockeyCa_GetPower()
{
	return (bPower ? TRUE : FALSE);
}

void HockeyCa_PowerOn()
{
	InitGame();
	bPower = TRUE;
}

void HockeyCa_PowerOff()
{
	bPower = FALSE;
	HockeyCa_StopSound();
}

void HockeyCa_SetSkill(int i){
	if (i == 0){
		bPro2 = FALSE;
	} else {
		bPro2 = TRUE;
	}
}

int HockeyCa_GetSkill(){
	return bPro2 ? 1 : 0;
}

void HockeyCa_Run(int tu)
{
	int x, y;

	// prevent reentrancy
	if (bInFrame){ return; }
	bInFrame = TRUE;

	// init the blips field
	for (y = 0; y < HOCKEYCA_BLIP_ROWS; y++){
		for (x = 0; x < HOCKEYCA_BLIP_COLUMNS; x++){
			Blips[x][y] = BLIP_OFF;
		}
	}

	if (!bPower){
		HockeyCa_ClearScreen();
		bInFrame = FALSE;
		return;
	}

	Platform_StartDraw();

	(fsmfcn[fsm])();

	DrawBlips();

	Platform_EndDraw();

	bInFrame = FALSE;

}


// FINITE STATE MACHINE STUFF

void fsmPlayStartWait()
{
	// show formation
	{
		SETPLAYER(player, ((bHomeTeam) ? 0 : 4), 3, BLIP_BRIGHT);
		UNSETPLAYER(puck);

		SETPLAYER(defense[0], 2, 0, BLIP_DIMFLICKER);

		SETPLAYER(defense[1], 0, 1, BLIP_DIM);
		SETPLAYER(defense[2], 4, 1, BLIP_DIM);
		SETPLAYER(defense[3], 1, 1, BLIP_DIM);
		SETPLAYER(defense[4], 3, 1, BLIP_DIM);

		SETPLAYER(defense[5], (bHomeTeam ? 3 : 1), 2, BLIP_DIM);
	}

	bShoot = FALSE;
	nPlayTime = 50;
	nTimerTickTimer = bPro2 ? TIME_TICKTIMERPRO2 : TIME_TICKTIMERPRO1;
	bDeflect = FALSE;
	nTimerDeflect = 0;

	// show stats if pressed 'down'
	// else, wait for player to move then advance to next state
	BOOL bChange;
	if (HockeyCa_GetInputDOWN(&bChange))
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
	else if (HockeyCa_TestForMovement())
	{
		Platform_IsNewSecond();
		fsmInPlay();
		fsm = FSM_INPLAY;
		return;
	}

	HockeyCa_DrawStat((int)fGameTime);

}

void fsmShowStats()
{
	// show formation
	if (!bGameOver)
	{
		SETPLAYER(player, ((bHomeTeam) ? 0 : 4), 3, BLIP_BRIGHT);
		UNSETPLAYER(puck);

		SETPLAYER(defense[0], 2, 0, BLIP_DIMFLICKER);

		SETPLAYER(defense[1], 0, 1, BLIP_DIM);
		SETPLAYER(defense[2], 4, 1, BLIP_DIM);
		SETPLAYER(defense[3], 1, 1, BLIP_DIM);
		SETPLAYER(defense[4], 3, 1, BLIP_DIM);

		SETPLAYER(defense[5], (bHomeTeam ? 3 : 1), 2, BLIP_DIM);
	}

	if (--nTimerStatsDisplay <= 0)
	{
		++nStatsIndex;
		nTimerStatsDisplay = TIME_STATSDISPLAY;
	}
	switch(nStatsIndex)
	{
		case 0:
			HockeyCa_DrawStat(-1);
			break;
		case 1:
			HockeyCa_DrawStat(nVScore);
			break;
		case 2:
			HockeyCa_DrawStat(-1);
			break;
		case 3:
			HockeyCa_DrawStat(nHScore);
			break;
		case 4:
			HockeyCa_DrawStat(-1);
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
	if (HockeyCa_GetInputLEFT(&bChange))
	{
		if (bChange)
		{
			if ((player.nColumn > 0)
				&& (!ISOCCUPIED(player.nColumn-1, player.nRow))){
				MOVEPLAYERLEFT(player);
			}
		}
	}
	else if (HockeyCa_GetInputRIGHT(&bChange))
	{
		if (bChange)
		{
			if ((player.nColumn < (HOCKEYCA_BLIP_COLUMNS-1))
				&& (!ISOCCUPIED(player.nColumn+1, player.nRow))){
				MOVEPLAYERRIGHT(player);
			}
		}
	}
	else if (HockeyCa_GetInputUP(&bChange))
	{
		if (bChange)
		{
			if ((player.nRow > 0)
				&& (!ISOCCUPIED(player.nColumn, player.nRow-1))){
				MOVEPLAYERUP(player);
			}
		}
	}
	else if (HockeyCa_GetInputDOWN(&bChange))
	{
		if (bChange)
		{
			if ((player.nRow < (HOCKEYCA_BLIP_ROWS-1))
				&& (!ISOCCUPIED(player.nColumn, player.nRow+1))){
				MOVEPLAYERDOWN(player);
			}
		}
	}

	if (nTimerDeflect)
	{
		// still in deflect
		--nTimerDeflect;
		return;
	}

	if (HockeyCa_GetInputTHROW(&bChange))
	{
		if (bChange)
		{
			if (!ISPLAYERENABLED(puck))
			{
				// shoot!
				bShoot = TRUE;
				nTimerBallMove = TIME_PUCKMOVE;
				SETPLAYERBRIGHTNESS(player, BLIP_DIMBLINK);
				SETPLAYER(puck, player.nColumn, player.nRow, BLIP_BRIGHT);

				if (player.nRow > 0)
				{
					nBallDirectionY = -1;
				}						 
				else
				{
					nBallDirectionY = 0;
				}

				if (player.nColumn < 2)
				{
					nBallDirectionX = 1;
				}
				else if (player.nColumn > 2)
				{
					nBallDirectionX = -1;
				}
				else
				{
					nBallDirectionX = 0;
				}
			}
		}
	}


	// move the two end defenders randomly
	{
		if (Platform_Random(bPro2 ? 10 : 20) == 0){
			// move horizontally or vertically
			int x = defense[1].nColumn;
			int y = defense[1].nRow;
			if (Platform_Random(2)){
				if (x == 0){
					x = 1;
				} else {
					x = 0;
				}
			} else {
				if (y == 0){
					y = 1;
				} else {
					y = 0;
				}
			}
			if (!ISOCCUPIED(x,y))
			{
				// if puck is active, only move defender
				// if it will block the shot
				if ((ISPLAYERENABLED(puck) && ISBLOCKINGPOS(x,y))
					|| (!ISPLAYERENABLED(puck)))
				{
					SETPLAYER(defense[1], x, y, BLIP_DIM);
				}
			}
		}
		if (Platform_Random(bPro2 ? 10 : 20) == 0){
			// move horizontally or vertically
			int x = defense[2].nColumn;
			int y = defense[2].nRow;
			if (Platform_Random(2)){
				if (x == (HOCKEYCA_BLIP_COLUMNS-1)){
					x = (HOCKEYCA_BLIP_COLUMNS-2);
				} else {
					x = (HOCKEYCA_BLIP_COLUMNS-1);
				}
			} else {
				if (y == 0){
					y = 1;
				} else {
					y = 0;
				}
			}
			if (!ISOCCUPIED(x,y))
			{
				// if puck is active, only move defender
				// if it will block the shot
				if ((ISPLAYERENABLED(puck) && ISBLOCKINGPOS(x,y))
					|| (!ISPLAYERENABLED(puck)))
				{
					SETPLAYER(defense[2], x, y, BLIP_DIM);
				}
			}
		}
	}

	// establish a target for the defense
	PLAYER *targetP = ISPLAYERENABLED(puck) ? &puck : &player;

	// move the goalie
	{
		// decide if the goalie will move this frame -
		if (Platform_Random(bPro2 ? 6 : 8) == 0)
		{
			// pick a direction
			if ((targetP->nRow == 1)
				&& (defense[0].nRow == 0)
				&& (defense[0].nColumn == targetP->nColumn)
				&& (Platform_Random(2) == 0))

			{
				// puck is loose - move out of goal area if we can deflect the puck
				// notice we only check here for defenders - it is intentionally
				// possible for the goalie to move on top of the player and
				// thus intercept the puck
				if (!ISDEFENSE(defense[0].nColumn, 1))
				{
					defense[0].nRow = 1;
				}
			}
			else
			{
				if (defense[0].nRow == 1)
				{
					// move back to row 0 to protect goal
					if (Platform_Random(3) == 0)
					{
						// notice we only check here for defenders - it is intentionally
						// possible for the goalie to move on top of the player and
						// thus intercept the puck
						if (!ISDEFENSE(defense[0].nColumn, 0))
						{
							defense[0].nRow = 0;
						}
					}
				}
				else
				{
					// horizontally
					if ((targetP->nColumn < defense[0].nColumn)
						&& (defense[0].nColumn > 1))
					{
						// notice we only check here for defenders - it is intentionally
						// possible for the goalie to move on top of the player and
						// thus intercept the puck
						if (!ISDEFENSE(defense[0].nColumn-1, defense[0].nRow))
						{
							defense[0].nColumn--;
						}
					}
					else if ((targetP->nColumn > defense[0].nColumn)
						&& (defense[0].nColumn < 3))
					{
						// notice we only check here for defenders - it is intentionally
						// possible for the goalie to move on top of the player and
						// thus intercept the puck
						if (!ISDEFENSE(defense[0].nColumn+1, defense[0].nRow))
						{
								defense[0].nColumn++;
						}
					}
				}
			}

			// check for goalie intercepting the puck
			if ((targetP->nColumn == defense[0].nColumn)
				&& (targetP->nRow == defense[0].nRow))
			{
				if (ISPLAYERENABLED(puck))
				{
					// deflect it - handled below by the puck move code
				}
				else
				{
					// got the puck while the player still had possession - interception
					bShoot = FALSE;
					UNSETPLAYER(puck);
					SETPLAYERBRIGHTNESS(player, BLIP_BRIGHT);
					HockeyCa_ClearScreen();
					HockeyCa_PlaySound(HOCKEYCA_SOUND_ENDPLAY, PLAYSOUNDFLAGS_PRIORITY);
					bHomeTeam = !bHomeTeam;
					PlatformSetInput(bHomeTeam);
					nStatsIndex = 0;
					nTimerStatsDisplay = TIME_STATSDISPLAY;
					fsm = FSM_SHOWSTATS;
					return;
				}
			}

		}
	}

	// middle defenders try to roughly occupy the
	// midpoint between the player and the goal
	{
		int midx = (player.nColumn + 2) / 2;
		int midy = (player.nRow + 1) / 2;

		// decide if a defender will move this frame -
		if (Platform_Random(bPro2 ? 2 : 5) == 0)
		{
			// pick a random defender
			static int nDefenderLast = -1;
			int nDefender = Platform_Random(3);

			if (nDefender != nDefenderLast)
			{
				nDefenderLast = nDefender;

				PLAYER *pDefender = &defense[3 + nDefender];
	
				int dx = pDefender->nColumn;
				int dy = pDefender->nRow;

				// randomly move the defender towards the midpoint
				if (Platform_Random(2))
				{
					if (dx < midx)
					{
						++dx;
					}
					else if (dx > midx)
					{
						--dx;
					}
				}
				else
				{
					if (dy < midy)
					{
						++dy;
					}
					else if (dy > midy)
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

				// make sure the defender does not step on anybody,
				// and keep him confined to the proper area
				if (!ISOCCUPIED(dx, dy)
					&& (dx > 0)
					&& (dx < (HOCKEYCA_BLIP_COLUMNS - 1))
					&& (dy >= 0)
					&& (dy < (HOCKEYCA_BLIP_ROWS - 1)))
				{
					// if puck is active, only move defender
					// if it will block the shot
					if ((ISPLAYERENABLED(puck) && ISBLOCKINGPOS(dx,dy))
						|| (!ISPLAYERENABLED(puck)))
					{
						SETPLAYER((*pDefender), dx, dy, BLIP_DIM);
					}
				}

			}
		}
	}

	// move the puck
	if (ISPLAYERENABLED(puck))
	{
		// the puck hit a defender last frame - play the sound
		if (ISDEFENSE(puck.nColumn, puck.nRow) && !bDeflect)
		{
			bDeflect = TRUE;
			HockeyCa_PlaySound(HOCKEYCA_SOUND_BOUNCE, PLAYSOUNDFLAGS_PRIORITY);
			nTimerDeflect = TIME_DEFLECT;
			return;
		}

		if (--nTimerBallMove <= 0)
		{
			nTimerBallMove = TIME_PUCKMOVE;

			// check for a goal
			if ((puck.nColumn == 2) && (puck.nRow == 0)
				&& !ISDEFENSE(puck.nColumn, puck.nRow)) // <-- is this right?
			{
				// goal!
				puck.nRow = -1;
				HockeyCa_DrawGoal(TRUE);
				fsm = FSM_GOAL;
				return;
			}

			// bounce the puck off the defenders
			if (bDeflect)
			{
				Platform_IsNewSecond();
				nBallDirectionX = Platform_Random(3) - 1;
				nBallDirectionY = 1;
				bDeflect = FALSE;
			}

			// see if player has regained possession
			if (ISPLAYER(puck.nColumn, puck.nRow) && !bShoot)
			{
				UNSETPLAYER(puck);
				SETPLAYERBRIGHTNESS(player, BLIP_BRIGHT);
				nPlayTime = 50; // reset play timer
			}
			else
			{
				// move the puck
				puck.nColumn += nBallDirectionX;
				puck.nRow += nBallDirectionY;
				bShoot = FALSE;

				// see if the puck bounced off the screen
				if (ISPLAYEROFFSCREEN(puck) &&  ISPLAYERENABLED(puck))
				{
					// off the screen - get rid of it and end the play
					bShoot = FALSE;
					UNSETPLAYER(puck);
					SETPLAYERBRIGHTNESS(player, BLIP_BRIGHT);
					HockeyCa_ClearScreen();
					HockeyCa_PlaySound(HOCKEYCA_SOUND_ENDPLAY, PLAYSOUNDFLAGS_PRIORITY);
					bHomeTeam = !bHomeTeam;
					PlatformSetInput(bHomeTeam);
					nStatsIndex = 0;
					nTimerStatsDisplay = TIME_STATSDISPLAY;
					fsm = FSM_SHOWSTATS;
					return;
				}
			}
		}
	}

	// count down the clock
	if (Platform_IsNewSecond()){
		// check for end of period
		if (fGameTime > 0.0){
			fGameTime -= (float).1;
			if (fGameTime < 0.1){

				// end of period

				++nPeriod;
				if (nPeriod == 3)
				{
					// game over
					bGameOver = TRUE;

					HockeyCa_ClearScreen();
					HockeyCa_PlaySound(HOCKEYCA_SOUND_ENDGAME, PLAYSOUNDFLAGS_PRIORITY);

					// show formation
					{
						SETPLAYER(player, 0, 3, BLIP_BRIGHT);
						UNSETPLAYER(puck);

						SETPLAYER(defense[0], 2, 0, BLIP_DIMFLICKER);

						SETPLAYER(defense[1], 0, 1, BLIP_DIM);
						SETPLAYER(defense[2], 4, 1, BLIP_DIM);
						SETPLAYER(defense[3], 1, 1, BLIP_DIM);
						SETPLAYER(defense[4], 3, 1, BLIP_DIM);

						SETPLAYER(defense[5], 3, 2, BLIP_DIM);
					}

					nStatsIndex = 0;
					nTimerStatsDisplay = TIME_STATSDISPLAY;
					fsm = FSM_SHOWSTATS;
					return;
				}
				else
				{
					bHomeTeam = !bHomeTeam;
					PlatformSetInput(bHomeTeam);
				}

				fGameTime = (float)20.9;
				HockeyCa_ClearScreen();
				HockeyCa_PlaySound(HOCKEYCA_SOUND_ENDQUARTER, PLAYSOUNDFLAGS_PRIORITY);
				nStatsIndex = 0;
				nTimerStatsDisplay = TIME_STATSDISPLAY;
				fsm = FSM_SHOWSTATS;
				return;
			}
		}
		// check for end of play timer
		if (--nPlayTime < 0){
			HockeyCa_ClearScreen();
			HockeyCa_PlaySound(HOCKEYCA_SOUND_ENDPLAY, PLAYSOUNDFLAGS_PRIORITY);
			bHomeTeam = !bHomeTeam;
			PlatformSetInput(bHomeTeam);
			nStatsIndex = 0;
			nTimerStatsDisplay = TIME_STATSDISPLAY;
			fsm = FSM_SHOWSTATS;
			return;
		}
	}

	HockeyCa_DrawStat((int)fGameTime);

	// make tick sounds
	if (fGameTime >= .1){
		if (--nTimerTickTimer <= 0){
			HockeyCa_PlaySound(HOCKEYCA_SOUND_TICK, PLAYSOUNDFLAGS_ASYNC);
			nTimerTickTimer = bPro2 ? TIME_TICKTIMERPRO2 : TIME_TICKTIMERPRO1;
		}
	}
}

void fsmGoal()
{
	HockeyCa_PlaySound(HOCKEYCA_SOUND_SCORE, PLAYSOUNDFLAGS_PRIORITY);
	HockeyCa_DrawGoal(FALSE);
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
}

void fsmGameOver()
{
	// show stats if pressed 'down'
	BOOL bChange;
	if (HockeyCa_GetInputDOWN(&bChange))
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

	HockeyCa_DrawStat(0);
}

