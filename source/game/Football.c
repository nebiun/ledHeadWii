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


#include "Football.h"
#include "Games.h"

// constants

#define NUM_DEFENSEPLAYERS	5
#define MAX_YARD			100

typedef int BLIP;

static BLIP Blips[FOOTBALL_BLIP_COLUMNS][FOOTBALL_BLIP_ROWS];

// game variables
static BOOL bHomeTeam;
static BOOL bInFrame = FALSE;
static BOOL bPower;
static BOOL bPro2 = FALSE;
static BOOL bGotFirstDown;

static BOOL bDisplayScore;
static BOOL bDisplayTime;
static BOOL bDisplayYard;
static BOOL bDisplayDown;
static BOOL bDisplayBlips;

static int nHScore;
static int nVScore;
static float fGameTime;
static int nFirstDownYard;
static int nDown;
static int nQuarter;
static int nCurrentYardline;

typedef struct PLAYER {
	int nYard;
	int nColumn;
	int nLane;
	int nBright;

	int nColumnOld;
	int nLaneOld;
	int nYardOld;
}PLAYER;

static PLAYER ball;
static PLAYER player[NUM_DEFENSEPLAYERS];

// macros for dealing with the players

#define TRANSLATE_COLUMN(x)	\
	(!bHomeTeam ? x : (FOOTBALL_BLIP_COLUMNS - 1) - x)

#define UNTRANSLATE_COLUMN(x)	\
	(bHomeTeam ? x : (FOOTBALL_BLIP_COLUMNS - 1) - x)

#define TRANSLATE_YARD(x) \
	(!bHomeTeam ? x : (MAX_YARD - x))

#define SETPLAYERBRIGHTNESS(p,b)	{ \
	p.nBright = b; \
}

#define NOTECURRENTPLAYERPOSITION(p) { \
	p.nColumnOld = p.nColumn; \
	p.nLaneOld = p.nLane; \
	p.nYardOld = p.nYard; \
}

#define SETPLAYER(p,a,x,y,b)	{ \
	NOTECURRENTPLAYERPOSITION(p); \
	p.nYard = a; \
	p.nColumn = TRANSLATE_COLUMN(x); \
	p.nLane = y; \
	p.nBright = b; \
}

#define UNSETPLAYER(p)	{ \
	NOTECURRENTPLAYERPOSITION(p); \
	p.nYard = -1; \
	p.nColumn = -1; \
	p.nLane = -1; \
	p.nBright = BLIP_OFF; \
}

#define MOVEPLAYERUP(p)		{ \
	NOTECURRENTPLAYERPOSITION(p); \
	p.nLane--; \
}
#define MOVEPLAYERDOWN(p)	{ \
	NOTECURRENTPLAYERPOSITION(p); \
	p.nLane++; \
}
#define MOVEPLAYERLEFT(p)	{ \
	NOTECURRENTPLAYERPOSITION(p); \
	p.nColumn--; \
	if (p.nColumn < 0){ \
		p.nColumn = FOOTBALL_BLIP_COLUMNS-1; \
	} \
	if (!bHomeTeam){ \
	  p.nYard--; \
	} else { \
	  p.nYard++; \
	} \
}
#define MOVEPLAYERRIGHT(p)	{ \
	NOTECURRENTPLAYERPOSITION(p); \
	p.nColumn++; \
	if (p.nColumn >= FOOTBALL_BLIP_COLUMNS){ \
		p.nColumn = 0; \
	} \
	if (!bHomeTeam){ \
		p.nYard++; \
	} else { \
	  p.nYard--; \
	} \
}
#define MOVEPLAYERDOWNFIELD(p)	{ \
	NOTECURRENTPLAYERPOSITION(p); \
	p.nYard--; \
	if (bHomeTeam){ \
		p.nColumn++; \
		if (p.nColumn >= FOOTBALL_BLIP_COLUMNS){ \
			p.nColumn = 0; \
		} \
	} else { \
		p.nColumn--; \
		if (p.nColumn < 0){ \
			p.nColumn = FOOTBALL_BLIP_COLUMNS-1; \
		} \
	} \
}
#define MOVEPLAYERUPFIELD(p)	{ \
	NOTECURRENTPLAYERPOSITION(p); \
	p.nYard++; \
	if (!bHomeTeam){ \
		p.nColumn++; \
		if (p.nColumn >= FOOTBALL_BLIP_COLUMNS){ \
			p.nColumn = 0; \
		} \
	} else { \
		p.nColumn--; \
		if (p.nColumn < 0){ \
			p.nColumn = FOOTBALL_BLIP_COLUMNS-1; \
		} \
	} \
}

static BOOL ISBALL(int x, int y);
static BOOL ISBALL(int x, int y)
{
	if ((ball.nColumn == x)
		&& (ball.nLane == y)
		&& (ball.nBright)){
		return TRUE;
	}
	return FALSE;
}

static BOOL ISDEFENSE(int x, int y);
static BOOL ISDEFENSE(int x, int y)
{
	for (int i=0; i<NUM_DEFENSEPLAYERS; i++){
		if ((player[i].nColumn == x)
			&& (player[i].nLane == y)
			&& (player[i].nBright)){
			return TRUE;
		}
	}
	return FALSE;
}

static BOOL ISOCCUPIED(int x, int y);
static BOOL ISOCCUPIED(int x, int y)
{
	if (ISBALL(x,y)){
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
		if ((player[i].nColumn == x)
			&& (player[i].nLane == y)
			&& (player[i].nBright)){
			return i;
		}
	}
	return -1;
}

#define UNMOVEPLAYER(p) { \
	p.nLane = p.nLaneOld; \
	p.nColumn = p.nColumnOld; \
	p.nYard = p.nYardOld; \
}

#define ISPLAYEROFFSCREEN(p) \
	((p.nColumn > FOOTBALL_BLIP_COLUMNS-1) || (p.nColumn < 0))



// finite state machine stuff

static void fsmFormation();
static void fsmInPlay();
static void fsmPlayEnded();
static void fsmGameOver();

static enum FSM {
	FSM_FORMATION,
	FSM_INPLAY,
	FSM_PLAYENDED,
	FSM_GAMEOVER
}fsm;

typedef void (*FSMFCN)();

static FSMFCN fsmfcn[] = {
	fsmFormation,
	fsmInPlay,
	fsmPlayEnded,
	fsmGameOver
};


// proto's
static void InitGame();
static void DrawBlips();
static void EraseBlips();


BOOL Football_GetPower()
{
	return (bPower ? TRUE : FALSE);
}

void Football_PowerOn()
{
	InitGame();
	bPower = TRUE;
}

void Football_PowerOff()
{
	bPower = FALSE;
	Football_StopSound();
}

void Football_SetSkill(int i){
	if (i == 0){
		bPro2 = FALSE;
	} else {
		bPro2 = TRUE;
	}
}

int Football_GetSkill(){
	return bPro2 ? 1 : 0;
}

void InitGame()
{
	bHomeTeam = TRUE;
	PlatformSetInput(bHomeTeam);
	
	nHScore = 0;
	nVScore = 0;
	fGameTime = 15.0;
	nDown = 0;
	nQuarter = 0;
	nCurrentYardline = 100 - 20;
	nFirstDownYard = nCurrentYardline - 10;

	bDisplayScore = FALSE;
	bDisplayTime = FALSE;
	bDisplayYard = FALSE;
	bDisplayDown = FALSE;
	bDisplayBlips = TRUE;
	
	fsm = FSM_FORMATION;
}

void Football_Run(int tu)
{
	int x, y;

	// prevent reentrancy
	if (bInFrame){ return; }
	bInFrame = TRUE;

	// init the blips field
	for (y = 0; y < FOOTBALL_BLIP_ROWS; y++){
		for (x = 0; x < FOOTBALL_BLIP_COLUMNS; x++){
			Blips[x][y] = BLIP_OFF;
		}
	}

	if (!bPower){
		Football_ClearScreen();
		bInFrame = FALSE;
		return;
	}

	Platform_StartDraw();

	// run the game
	(fsmfcn[fsm])();

	// update the display
	if (bDisplayBlips){
		DrawBlips();
	} else {
		EraseBlips();
	}
	
	if (bDisplayScore){
		Football_DrawScores(nHScore, nVScore);
	}
	
	if (bDisplayYard){
		Football_DrawYard(TRANSLATE_YARD(nCurrentYardline));
	} else if (bDisplayTime){
		Football_DrawTime(fGameTime);
	}

	if (bDisplayDown){
		int nYardsToGo = nCurrentYardline - nFirstDownYard;
		if (nYardsToGo > ball.nYard){
			// don't show yards to go if more than distance to goal
			Football_DrawDown(nDown, -1);
		} else {
			Football_DrawDown(nDown, nYardsToGo);
		}
	}

	if (!bDisplayDown && !bDisplayScore){
		Football_DrawScores(-1, -1);
	}

	if (!bDisplayTime && !bDisplayYard){
		Football_DrawTime(-1);
	}

	Platform_EndDraw();

	bInFrame = FALSE;

}

void DrawBlips()
{
	int x, y, nBright;
	static int nBlinkTimer = 0;
	static BOOL bBlink = FALSE;

	for (int i=0; i<NUM_DEFENSEPLAYERS; i++){
		if (player[i].nBright != BLIP_OFF){
			x = player[i].nColumn % 10;
			y = player[i].nLane;
			nBright = player[i].nBright;
			Blips[x][y] = nBright;
		}
	}
	if (ball.nBright != BLIP_OFF){
		x = ball.nColumn % 10;
		y = ball.nLane;
		nBright = ball.nBright;
		Blips[x][y] = nBright;
	}

	// draw the blips field
	for (y = 0; y < FOOTBALL_BLIP_ROWS; y++){
		for (x = 0; x < FOOTBALL_BLIP_COLUMNS; x++){

			switch(Blips[x][y]){
				case BLIP_OFF:
				case BLIP_DIM:
				case BLIP_BRIGHT:
					Football_DrawBlip(Blips[x][y], x, y);
					break;
				case BLIP_DIMBLINK:
					if (!bBlink){
						Football_DrawBlip(BLIP_DIM, x, y);
					} else {
						Football_DrawBlip(BLIP_OFF, x, y);
					}
					break;
				case BLIP_BRIGHTBLINK:
					if (bBlink){
						Football_DrawBlip(BLIP_BRIGHT, x, y);
					} else {
						Football_DrawBlip(BLIP_OFF, x, y);
					}
					break;
			}

		}
	}

	if (--nBlinkTimer <= 0){
		nBlinkTimer = 4;
		bBlink = !bBlink;
	}
}

void EraseBlips()
{
	// erase the blips field
	for (int y = 0; y < FOOTBALL_BLIP_ROWS; y++){
		for (int x = 0; x < FOOTBALL_BLIP_COLUMNS; x++){
			Football_DrawBlip(BLIP_OFF, x, y);
		}
	}
}

// FINITE STATE MACHINE STUFF

static void fsmFormation()
{
	bDisplayTime = FALSE;
	bDisplayScore = FALSE;
	bDisplayYard = FALSE;
	bDisplayDown = FALSE;
	bDisplayBlips = FALSE;

	if (Football_GetInputSCORE(NULL)){
		bDisplayScore = TRUE;
		bDisplayTime = TRUE;
	} else if (Football_GetInputSTATUS(NULL)){
		bDisplayYard = TRUE;
		bDisplayDown = TRUE;
	} else {
		bDisplayBlips = TRUE;
	}

	if (bGotFirstDown){
		nFirstDownYard = nCurrentYardline - 10;
		nDown = 0;
		bGotFirstDown = FALSE;
	}

	// set up the players in formation

	SETPLAYER(ball, nCurrentYardline+0, 8, 1, BLIP_BRIGHT);

	SETPLAYER(player[0], 0, 5, 0, BLIP_DIM);
	SETPLAYER(player[1], 0, 5, 1, BLIP_DIM);
	SETPLAYER(player[2], 0, 5, 2, BLIP_DIM);
	SETPLAYER(player[3], 0, 0, 1, BLIP_DIM);
	SETPLAYER(player[4], 0, 3, 1, BLIP_DIM);

	// wait for kickoff or movement, then start play
	if (Football_GetInputKICK(NULL) && (nDown == 3)){

		// if 4th down player can attempt fieldgoal/punt

		// calculate where ball will land
		int nDistance = Platform_Random(64) + 1;
		if ((ball.nYard - nDistance) < 0){

			// make it a field goal
			if (bHomeTeam){
				nHScore += 3;
			} else {
				nVScore += 3;
			}

			Football_ClearScreen();
			Football_PlaySound(FOOTBALL_SOUND_SCORE, PLAYSOUNDFLAGS_PRIORITY);

			// give the ball to the other team
			bHomeTeam = !bHomeTeam;
			PlatformSetInput(bHomeTeam);
			nDown = 0;
			ball.nYard = 20;
			nCurrentYardline = ball.nYard;
			nFirstDownYard = nCurrentYardline - 10;
			fsm = FSM_FORMATION;
			return;

		} else {
			// make it a punt
			// give the ball to the other team
			bHomeTeam = !bHomeTeam;
			PlatformSetInput(bHomeTeam);
			nDown = 0;
			ball.nYard = MAX_YARD - (ball.nYard - nDistance);
			nCurrentYardline = ball.nYard;
			nFirstDownYard = ball.nYard - 10;

			Football_ClearScreen();
			Football_PlaySound(FOOTBALL_SOUND_ENDPOSSESSION, PLAYSOUNDFLAGS_PRIORITY);
			fsm = FSM_FORMATION;
			return;
		}


	}
	else if (Football_TestForMovement())
	{
		Platform_IsNewSecond();
		fsm = FSM_INPLAY;
		fsmInPlay();
		return;
	} 
}

static void fsmInPlay()
{
	bDisplayTime = FALSE;
	bDisplayScore = FALSE;
	bDisplayYard = FALSE;
	bDisplayDown = FALSE;
	bDisplayBlips = TRUE;

	BOOL bChange;
	if (Football_GetInputRUN(&bChange))
	{
		if (bChange)
		{
			MOVEPLAYERDOWNFIELD(ball);
		}
	}
	else if (Football_GetInputUP(&bChange))
	{
		if (bChange)
		{
			if (ball.nLane){
				MOVEPLAYERUP(ball);
			}
		}
	}
	else if (Football_GetInputDOWN(&bChange))
	{
		if (bChange)
		{
			if (ball.nLane < 2){
				MOVEPLAYERDOWN(ball);
			}
		}
	}

	int nYardsToGo = ball.nYard - nFirstDownYard;

	// 1st down
	if (nYardsToGo <= 0){
		if (!bGotFirstDown){
			bGotFirstDown = TRUE;
		}
	} else {
		bGotFirstDown = FALSE;
	}


	// check for touchdown
	if (ball.nYard < 0){
		// touchdown!!
		if (bHomeTeam){
			nHScore += 7;
		} else {
			nVScore += 7;
		}
		Football_ClearScreen();
		Football_PlaySound(FOOTBALL_SOUND_SCORE, PLAYSOUNDFLAGS_PRIORITY);
		Platform_IsNewSecond();
		bHomeTeam = !bHomeTeam;
		PlatformSetInput(bHomeTeam);
		ball.nYard = 100 - 20;
		nCurrentYardline = ball.nYard;
		nFirstDownYard = nCurrentYardline - 10;
		nDown = 0;
		fsm = FSM_PLAYENDED;
		return;
	}

	// check for collisions
	if (ISDEFENSE(ball.nColumn, ball.nLane)){
		// tackled!
		Football_ClearScreen();

		int i = GETPLAYERAT(ball.nColumn, ball.nLane);
		if (i != -1){
			SETPLAYERBRIGHTNESS(player[i], BLIP_DIMBLINK);
		}

		UNMOVEPLAYER(ball);

		if ((++nDown >= 4) && (!bGotFirstDown)){

			// give the ball to the other team
			bHomeTeam = !bHomeTeam;
			PlatformSetInput(bHomeTeam);
			nDown = 0;
			ball.nYard = MAX_YARD - ball.nYard;
			nFirstDownYard = ball.nYard - 10;

			Football_PlaySound(FOOTBALL_SOUND_ENDPOSSESSION, PLAYSOUNDFLAGS_PRIORITY);
			fsm = FSM_PLAYENDED;
			return;

		} else {
			Football_PlaySound(FOOTBALL_SOUND_ENDPLAY, PLAYSOUNDFLAGS_PRIORITY);
			fsm = FSM_PLAYENDED;
			return;
		}

		return;
	}


	// move the defense randomly towards the ball
	if (Platform_Random(100) < ((bPro2) ? 20 : 10)){
		int i = Platform_Random(NUM_DEFENSEPLAYERS);
		if (player[i].nBright){
			// pick horizontal or vertical movement toward the ball
			if (Platform_Random(2) == 0){
				if (player[i].nColumn < ball.nColumn){
					if (!ISDEFENSE(player[i].nColumn+1, player[i].nLane)){
						NOTECURRENTPLAYERPOSITION(player[i]);
						player[i].nColumn++;
					}
				} else if (player[i].nColumn > ball.nColumn){
					if (!ISDEFENSE(player[i].nColumn-1, player[i].nLane)){
						NOTECURRENTPLAYERPOSITION(player[i]);
						player[i].nColumn--;
					}
				}
			} else {
				if (player[i].nLane < ball.nLane){
					if (!ISDEFENSE(player[i].nColumn, player[i].nLane+1)){
						NOTECURRENTPLAYERPOSITION(player[i]);
						player[i].nLane++;
					}
				} else if (player[i].nLane > ball.nLane){
					if (!ISDEFENSE(player[i].nColumn, player[i].nLane-1)){
						NOTECURRENTPLAYERPOSITION(player[i]);
						player[i].nLane--;
					}
				}
			}
		}
	}


	// check for collisions again
	if (ISDEFENSE(ball.nColumn, ball.nLane)){
		// tackled!
		Football_ClearScreen();

		int i = GETPLAYERAT(ball.nColumn, ball.nLane);
		if (i != -1){
			UNMOVEPLAYER(player[i]);
			SETPLAYERBRIGHTNESS(player[i], BLIP_DIMBLINK);
		}

		if ((++nDown >= 4) && (!bGotFirstDown)){

			// give the ball to the other team
			bHomeTeam = !bHomeTeam;
			PlatformSetInput(bHomeTeam);
			nDown = 0;
			ball.nYard = MAX_YARD - ball.nYard;
			nFirstDownYard = ball.nYard - 10;

			Football_PlaySound(FOOTBALL_SOUND_ENDPOSSESSION, PLAYSOUNDFLAGS_PRIORITY);
			fsm = FSM_PLAYENDED;
			return;

		} else {
			Football_PlaySound(FOOTBALL_SOUND_ENDPLAY, PLAYSOUNDFLAGS_PRIORITY);
			fsm = FSM_PLAYENDED;
			return;
		}

		return;
	}


	// count down the clock
	if (Platform_IsNewSecond()){
		if (fGameTime > 0.0){
			fGameTime -= (float).1;
			if (fGameTime > 0.0){
				Football_PlaySound(FOOTBALL_SOUND_TICK, PLAYSOUNDFLAGS_ASYNC);
			} else {
				// end of quarter!
				fGameTime = 0.0;
			}
		}
	}

}


static void fsmPlayEnded()
{
	bDisplayTime = FALSE;
	bDisplayScore = FALSE;
	bDisplayYard = FALSE;
	bDisplayDown = FALSE;
	bDisplayBlips = TRUE;

	BOOL bEndOfQuarter = (fGameTime < 0.1) ? TRUE : FALSE;

	// check for game over
	if (bEndOfQuarter && (nQuarter == 3)){
		fGameTime = 0.0;
		bDisplayBlips = FALSE;
		fsm = FSM_GAMEOVER;
		return;
	}

	// display '00' if clock ran out
	if (bEndOfQuarter){
		fGameTime = 0.0;
		bDisplayTime = TRUE;
		bDisplayBlips = FALSE;
	}

	if ((Football_GetInputSCORE(NULL) && !bEndOfQuarter) // end of quarter must press STATUS to set up next play
		|| (Football_GetInputSTATUS(NULL))){

		Football_StopSound();

		// if clock ran down in previous play
		if (bEndOfQuarter){
			++nQuarter;
			if (nQuarter == 2){
				// halftime - force kickoff
				// after halftime, visitor team gets possession
				bHomeTeam = FALSE;
				PlatformSetInput(bHomeTeam);
				ball.nYard = 100 - 20;
				nCurrentYardline = ball.nYard;
				nFirstDownYard = nCurrentYardline - 10;
				nDown = 0;

				fGameTime = 15.0;
				fsm = FSM_FORMATION;
				return;
			} else {
				fGameTime = 15.0;
			}
		}

		nCurrentYardline = ball.nYard;

		fsm = FSM_FORMATION;
//		fsmFormation();
		return;
	}

}


static void fsmGameOver()
{
	bDisplayTime = TRUE;
	bDisplayScore = TRUE;
	bDisplayYard = FALSE;
	bDisplayDown = FALSE;
	bDisplayBlips = FALSE;
}





