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

#include "Basketball.h"
#include "Games.h"


// constants

#define NUM_DEFENSEPLAYERS	5

#define TIME_TICKTIMERPRO1	7
#define TIME_TICKTIMERPRO2	4
#define TIME_STATSDISPLAY	5
#define TIME_BALLMOVE		5
#define TIME_DEFLECT		2


typedef int BLIP;

static BLIP Blips[BASKETBALL_BLIP_COLUMNS][BASKETBALL_BLIP_ROWS];

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
static int nQuarter;
static int nTimerTickTimer;

static BOOL bShoot;
static int nBallDirectionX;
static int nBallDirectionY;
static int nTimerBallMove;

static BOOL bDeflect;
static BOOL nTimerDeflect;

static int nStatsIndex;
static int nTimerStatsDisplay;

typedef struct PLAYER {
	int nColumn;
	int nRow;
	int nBright;
	int nColumnOld;
	int nRowOld;
}PLAYER;

static PLAYER ball;
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

static BOOL ISBALL(int x, int y)
{
	if ((ball.nColumn == x)
		&& (ball.nRow == y)
		&& (ball.nBright)){
		return TRUE;
	}
	return FALSE;
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
	((p.nColumn > BASKETBALL_BLIP_COLUMNS-1) \
	|| (p.nColumn < 0) \
	|| (p.nRow > BASKETBALL_BLIP_ROWS-1) \
	|| (p.nRow < 0))


#define ISPLAYERENABLED(p) \
	(p.nBright)

// finite state machine stuff

static void fsmPlayStartWait();
static void fsmShowStats();
static void fsmInPlay();
static void fsmDunk();
static void fsmGameOver();


static enum FSM {
	FSM_PLAYSTARTWAIT=0,
	FSM_SHOWSTATS,
	FSM_INPLAY,
	FSM_DUNK,
	FSM_GAMEOVER
}fsm;

typedef void (*FSMFCN)();

static FSMFCN fsmfcn[] = {
	fsmPlayStartWait,
	fsmShowStats,
	fsmInPlay,
	fsmDunk,
	fsmGameOver
};

static void InitGame()
{
	bHomeTeam = FALSE;
	PlatformSetInput(bHomeTeam);
	
	nHScore = 0;
	nVScore = 0;
	fGameTime = 12.0;
	nQuarter = 0;
	bGameOver = FALSE;
	bDeflect = FALSE;

	fsm = FSM_PLAYSTARTWAIT;
}

static void DrawBlips()
{
	int x, y, nBright;
	static BOOL bBlink = FALSE;

	if(fsm != FSM_DUNK) {
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
		if (ball.nBright != BLIP_OFF){
			x = ball.nColumn;
			y = ball.nRow;
			nBright = ball.nBright;
			Blips[x][y] = nBright;
		}
	}
	// draw the blips field
	for (y = 0; y < BASKETBALL_BLIP_ROWS; y++){
		for (x = 0; x < BASKETBALL_BLIP_COLUMNS; x++){

			switch(Blips[x][y]){
				case BLIP_OFF:
				case BLIP_DIM:
				case BLIP_BRIGHT:
					Basketball_DrawBlip(Blips[x][y], x, y);
					break;
				case BLIP_DIMBLINK:
					if (!bBlink){
						Basketball_DrawBlip(BLIP_DIM, x, y);
					} else {
						Basketball_DrawBlip(BLIP_OFF, x, y);
					}
					break;
				case BLIP_BRIGHTBLINK:
					if (bBlink){
						Basketball_DrawBlip(BLIP_BRIGHT, x, y);
					} else {
						Basketball_DrawBlip(BLIP_OFF, x, y);
					}
					break;
			}

		}
	}

	bBlink = !bBlink;
}

BOOL Basketball_GetPower()
{
	return (bPower ? TRUE : FALSE);
}

void Basketball_PowerOn()
{
	InitGame();
	bPower = TRUE;
}

void Basketball_PowerOff()
{
	bPower = FALSE;
	Basketball_StopSound();
}

void Basketball_SetSkill(int i){
	if (i == 0){
		bPro2 = FALSE;
	} else {
		bPro2 = TRUE;
	}
}

int Basketball_GetSkill(){
	return bPro2 ? 1 : 0;
}

void Basketball_Run(int tu)
{
	int x, y;

	// prevent reentrancy
	if (bInFrame){ return; }
	bInFrame = TRUE;

	// init the blips field
	for (y = 0; y < BASKETBALL_BLIP_ROWS; y++){
		for (x = 0; x < BASKETBALL_BLIP_COLUMNS; x++){
			Blips[x][y] = BLIP_OFF;
		}
	}

	if (!bPower){
		Basketball_ClearScreen();
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
		UNSETPLAYER(ball);
		SETPLAYER(defense[0], 0, 0, BLIP_DIM);
		SETPLAYER(defense[1], 4, 0, BLIP_DIM);
		SETPLAYER(defense[2], 1, 1, BLIP_DIM);
		SETPLAYER(defense[3], 2, 1, BLIP_DIM);
		SETPLAYER(defense[4], 3, 1, BLIP_DIM);
	}

	bShoot = FALSE;
	nPlayTime = 24;
	nTimerTickTimer = bPro2 ? TIME_TICKTIMERPRO2 : TIME_TICKTIMERPRO1;
	bDeflect = FALSE;
	nTimerDeflect = 0;

	// show stats if pressed 'down'
	// else, wait for player to move then advance to next state
	BOOL bChange;
	if (Basketball_GetInputDOWN(&bChange))
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
	else if (Basketball_TestForMovement())
	{
		Platform_IsNewSecond();
		fsmInPlay();
		fsm = FSM_INPLAY;
		return;
	}

	Basketball_DrawStat((int)fGameTime);

}

void fsmShowStats()
{
	// show formation
	if (!bGameOver)
	{
		SETPLAYER(player, ((bHomeTeam) ? 0 : 4), 3, BLIP_BRIGHT);
		UNSETPLAYER(ball);
		SETPLAYER(defense[0], 0, 0, BLIP_DIM);
		SETPLAYER(defense[1], 4, 0, BLIP_DIM);
		SETPLAYER(defense[2], 1, 1, BLIP_DIM);
		SETPLAYER(defense[3], 2, 1, BLIP_DIM);
		SETPLAYER(defense[4], 3, 1, BLIP_DIM);
	}

	if (--nTimerStatsDisplay <= 0)
	{
		++nStatsIndex;
		nTimerStatsDisplay = TIME_STATSDISPLAY;
	}
	switch(nStatsIndex)
	{
		case 0:
			Basketball_DrawStat(-1);
			break;
		case 1:
			Basketball_DrawStat(nVScore);
			break;
		case 2:
			Basketball_DrawStat(-1);
			break;
		case 3:
			Basketball_DrawStat(nHScore);
			break;
		case 4:
			Basketball_DrawStat(-1);
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
	if (Basketball_GetInputLEFT(&bChange))
	{
		if (bChange)
		{
			if ((player.nColumn > 0)
				&& (!ISOCCUPIED(player.nColumn-1, player.nRow))){
				MOVEPLAYERLEFT(player);
			}
		}
	}
	else if (Basketball_GetInputRIGHT(&bChange))
	{
		if (bChange)
		{
			if ((player.nColumn < (BASKETBALL_BLIP_COLUMNS-1))
				&& (!ISOCCUPIED(player.nColumn+1, player.nRow))){
				MOVEPLAYERRIGHT(player);
			}
		}
	}
	else if (Basketball_GetInputUP(&bChange))
	{
		if (bChange)
		{
			if ((player.nRow > 0)
				&& (!ISOCCUPIED(player.nColumn, player.nRow-1))){
				MOVEPLAYERUP(player);
			}
		}
	}
	else if (Basketball_GetInputDOWN(&bChange))
	{
		if (bChange)
		{
			if ((player.nRow < (BASKETBALL_BLIP_ROWS-1))
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

	if (Basketball_GetInputTHROW(&bChange))
	{
		if (bChange)
		{
			if (!ISPLAYERENABLED(ball))
			{
				// shoot!
				bShoot = TRUE;
				nTimerBallMove = TIME_BALLMOVE;
				SETPLAYERBRIGHTNESS(player, BLIP_DIMBLINK);
				SETPLAYER(ball, player.nColumn, player.nRow, BLIP_BRIGHT);

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
			int x = defense[0].nColumn;
			int y = defense[0].nRow;
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
			if (!ISOCCUPIED(x,y)){
				SETPLAYER(defense[0], x, y, BLIP_DIM);
			}
		}
		if (Platform_Random(bPro2 ? 10 : 20) == 0){
			// move horizontally or vertically
			int x = defense[1].nColumn;
			int y = defense[1].nRow;
			if (Platform_Random(2)){
				if (x == (BASKETBALL_BLIP_COLUMNS-1)){
					x = (BASKETBALL_BLIP_COLUMNS-2);
				} else {
					x = (BASKETBALL_BLIP_COLUMNS-1);
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
				SETPLAYER(defense[1], x, y, BLIP_DIM);
			}
		}
	}

	// there is a peculiar behavior with the game where if
	// the player is anywhere in the 2nd to last row, the
	// middle defenders line up in their formation positions
	// don't know if this was intentional or not, but this
	// behavior is reproduced here

	if (player.nRow == (BASKETBALL_BLIP_ROWS - 2))
	{
		if (Platform_Random(bPro2 ? 1 : 3) == 0)
		{
			// pick a random defender
			static int nDefenderLast = -1;
			int nDefender = Platform_Random(3);

			if (nDefender != nDefenderLast)
			{
				nDefenderLast = nDefender;

				PLAYER *pDefender;
				int ox, oy;
				switch(nDefender)
				{
					case 0:
						pDefender = &defense[2];
						ox = 1;
						oy = 1;
						break;
					case 1:
						pDefender = &defense[3];
						ox = 2;
						oy = 1;
						break;
					case 2:
						pDefender = &defense[4];
						ox = 3;
						oy = 1;
						break;
				}

				int dx = pDefender->nColumn;
				int dy = pDefender->nRow;

				// randomly move the defender towards its formation position
				if (Platform_Random(2))
				{
					if (dx < ox)
					{
						++dx;
					}
					else if (dx > ox)
					{
						--dx;
					}
				}
				else
				{
					if (dy < oy)
					{
						++dy;
					}
					else if (dy > oy)
					{
						--dy;
					}
				}

				// make sure the defender does not step on anybody,
				// and keep him confined to the proper area
				if (!ISOCCUPIED(dx, dy)
					&& (dx > 0)
					&& (dx < (BASKETBALL_BLIP_COLUMNS - 1))
					&& (dy >= 0)
					&& (dy < (BASKETBALL_BLIP_ROWS - 1)))
				{
					SETPLAYER((*pDefender), dx, dy, BLIP_DIM);
				}


			}
		}
	}
	else
	{
		// middle defenders try to roughly occupy the
		// midpoint between the ball and the basket
		int midx;
		int midy;
		if (ISPLAYERENABLED(ball))
		{
			midx = (ball.nColumn + 2) / 2;
			midy = (ball.nRow + 1) / 2;
		}
		else
		{
			midx = (player.nColumn + 2) / 2;
			midy = (player.nRow + 1) / 2;
		}

		// decide if a defender will move this frame -
		if (Platform_Random(bPro2 ? 2 : 5) == 0)
		{
			// pick a random defender
			static int nDefenderLast = -1;
			int nDefender = Platform_Random(3);

			if (nDefender != nDefenderLast)
			{
				nDefenderLast = nDefender;

				PLAYER *pDefender;
				switch(nDefender)
				{
					case 0:
						pDefender = &defense[2];
						break;
					case 1:
						pDefender = &defense[3];
						break;
					case 2:
						pDefender = &defense[4];
						break;
				}

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
					&& (dx < (BASKETBALL_BLIP_COLUMNS - 1))
					&& (dy >= 0)
					&& (dy < (BASKETBALL_BLIP_ROWS - 1)))
				{
					SETPLAYER((*pDefender), dx, dy, BLIP_DIM);
				}


			}
		}
	}

	// move the ball
	if (ISPLAYERENABLED(ball))
	{
		// the puck hit a defender last frame - play the sound
		if (ISDEFENSE(ball.nColumn, ball.nRow) && !bDeflect)
		{
			bDeflect = TRUE;
			Basketball_PlaySound(BASKETBALL_SOUND_BOUNCE, PLAYSOUNDFLAGS_PRIORITY);
			nTimerDeflect = TIME_DEFLECT;
			return;
		}

		if (--nTimerBallMove <= 0)
		{
			nTimerBallMove = TIME_BALLMOVE;

			// check for a basket
			if ((ball.nColumn == 2) && (ball.nRow == 0)
				&& !ISDEFENSE(ball.nColumn, ball.nRow)) // <-- is this right?
			{
				// basket!
				ball.nRow = -1;
				Basketball_DrawBasket(TRUE);
				fsm = FSM_DUNK;
				return;
			}

			// bounce the ball off the far wall
			if (!ISDEFENSE(ball.nColumn, ball.nRow))
			{
				if ((ball.nRow <= 0) && (nBallDirectionY == -1))
				{
					ball.nRow = 0;
					nBallDirectionY = 1;
					Basketball_PlaySound(BASKETBALL_SOUND_BOUNCE, PLAYSOUNDFLAGS_PRIORITY);
					nTimerDeflect = TIME_DEFLECT;
					bDeflect = FALSE;
					return;
				}
			}

			// bounce the ball off the defenders
			if (bDeflect)
			{
				nBallDirectionX = Platform_Random(3) - 1;
				nBallDirectionY = 1;
				bDeflect = FALSE;
			}

			// see if player has regained possession
			if (ISPLAYER(ball.nColumn, ball.nRow) && !bShoot)
			{
				UNSETPLAYER(ball);
				SETPLAYERBRIGHTNESS(player, BLIP_BRIGHT);
			}
			else
			{
				// move the ball
				ball.nColumn += nBallDirectionX;
				ball.nRow += nBallDirectionY;
				bShoot = FALSE;

				// see if the ball bounced off the screen
				if (ISPLAYEROFFSCREEN(ball) &&  ISPLAYERENABLED(ball))
				{
					// off the screen - get rid of it and end the play
					bShoot = FALSE;
					UNSETPLAYER(ball);
					SETPLAYERBRIGHTNESS(player, BLIP_BRIGHT);
					Basketball_ClearScreen();
					Basketball_PlaySound(BASKETBALL_SOUND_ENDPLAY, PLAYSOUNDFLAGS_PRIORITY);
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
		// check for end of quarter
		if (fGameTime > 0.0){
			fGameTime -= (float).1;
			if (fGameTime < 0.1){

				// end of quarter

				++nQuarter;
				if (nQuarter == 4)
				{
					// game over
					bGameOver = TRUE;

					// show formation
					{
						SETPLAYER(player, 4, 3, BLIP_BRIGHT);
						UNSETPLAYER(ball);
						SETPLAYER(defense[0], 0, 0, BLIP_DIM);
						SETPLAYER(defense[1], 4, 0, BLIP_DIM);
						SETPLAYER(defense[2], 1, 1, BLIP_DIM);
						SETPLAYER(defense[3], 2, 1, BLIP_DIM);
						SETPLAYER(defense[4], 3, 1, BLIP_DIM);
					}

					Basketball_ClearScreen();
					Basketball_PlaySound(BASKETBALL_SOUND_ENDGAME, PLAYSOUNDFLAGS_PRIORITY);
					nStatsIndex = 0;
					nTimerStatsDisplay = TIME_STATSDISPLAY;
					fsm = FSM_SHOWSTATS;
					return;
				}
				else
				{
					// VERIFY THIS AGAINST THE INSTRUCTION MANUAL
					bHomeTeam = (nQuarter % 2) ? FALSE : TRUE;
					PlatformSetInput(bHomeTeam);
				}

				fGameTime = 12.0;
				Basketball_ClearScreen();
				Basketball_PlaySound(BASKETBALL_SOUND_ENDQUARTER, PLAYSOUNDFLAGS_PRIORITY);
				nStatsIndex = 0;
				nTimerStatsDisplay = TIME_STATSDISPLAY;
				fsm = FSM_SHOWSTATS;
				return;
			}
		}
		// check for end of play timer
		if (--nPlayTime < 0){
			Basketball_ClearScreen();
			Basketball_PlaySound(BASKETBALL_SOUND_ENDPLAY, PLAYSOUNDFLAGS_PRIORITY);
			bHomeTeam = !bHomeTeam;
			PlatformSetInput(bHomeTeam);
			nStatsIndex = 0;
			nTimerStatsDisplay = TIME_STATSDISPLAY;
			fsm = FSM_SHOWSTATS;
			return;
		}
	}

	Basketball_DrawStat(nPlayTime);

	// make tick sounds
	if (fGameTime >= .1){
		if (--nTimerTickTimer <= 0){
			Basketball_PlaySound(BASKETBALL_SOUND_TICK, PLAYSOUNDFLAGS_ASYNC);
			nTimerTickTimer = bPro2 ? TIME_TICKTIMERPRO2 : TIME_TICKTIMERPRO1;
		}
	}
}
				
void fsmDunk()
{
	Basketball_PlaySound(BASKETBALL_SOUND_SCORE, PLAYSOUNDFLAGS_PRIORITY);
	Basketball_DrawBasket(FALSE);
	if (bHomeTeam) 
	{
		nHScore += 2;
	}
	else
	{
		nVScore += 2;
	}
	// reset formation
	{
		SETPLAYER(player, ((bHomeTeam) ? 0 : 4), 3, BLIP_BRIGHT);
		UNSETPLAYER(ball);
		SETPLAYER(defense[0], 0, 0, BLIP_DIM);
		SETPLAYER(defense[1], 4, 0, BLIP_DIM);
		SETPLAYER(defense[2], 1, 1, BLIP_DIM);
		SETPLAYER(defense[3], 2, 1, BLIP_DIM);
		SETPLAYER(defense[4], 3, 1, BLIP_DIM);
	}
	Platform_IsNewSecond();
	fsm = FSM_INPLAY;
}

void fsmGameOver()
{
	// show stats if pressed 'down'
	BOOL bChange;
	if (Basketball_GetInputDOWN(&bChange))
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

	Basketball_DrawStat(0);
}

