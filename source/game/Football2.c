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

#include "Football2.h"
#include "Games.h"

// constants

#define NUM_DEFENSEPLAYERS	6
#define MAX_YARD			100

typedef int BLIP;

static BLIP Blips[FOOTBALL2_BLIP_COLUMNS][FOOTBALL2_BLIP_ROWS];

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

static int nPassingStart;

static int nKickoffStart;
static int nKickoffDestination;

static BOOL bCanKick;
static BOOL bPunting;
static BOOL bFieldGoalAttempt;

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
static PLAYER receiver;
static PLAYER player[NUM_DEFENSEPLAYERS];

// macros for dealing with the players

#define TRANSLATE_COLUMN(x)	\
	(!bHomeTeam ? x : (FOOTBALL2_BLIP_COLUMNS - 1) - x)

#define UNTRANSLATE_COLUMN(x)	\
	(bHomeTeam ? x : (FOOTBALL2_BLIP_COLUMNS - 1) - x)

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
		p.nColumn = FOOTBALL2_BLIP_COLUMNS-1; \
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
	if (p.nColumn >= FOOTBALL2_BLIP_COLUMNS){ \
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
		if (p.nColumn >= FOOTBALL2_BLIP_COLUMNS){ \
			p.nColumn = 0; \
		} \
	} else { \
		p.nColumn--; \
		if (p.nColumn < 0){ \
			p.nColumn = FOOTBALL2_BLIP_COLUMNS-1; \
		} \
	} \
}
#define MOVEPLAYERUPFIELD(p)	{ \
	NOTECURRENTPLAYERPOSITION(p); \
	p.nYard++; \
	if (!bHomeTeam){ \
		p.nColumn++; \
		if (p.nColumn >= FOOTBALL2_BLIP_COLUMNS){ \
			p.nColumn = 0; \
		} \
	} else { \
		p.nColumn--; \
		if (p.nColumn < 0){ \
			p.nColumn = FOOTBALL2_BLIP_COLUMNS-1; \
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

static BOOL ISRECEIVER(int x, int y);
static BOOL ISRECEIVER(int x, int y)
{
	if ((receiver.nColumn == x)
		&& (receiver.nLane == y)
		&& (receiver.nBright)){
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
	if (ISRECEIVER(x,y)){
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
	((p.nColumn > FOOTBALL2_BLIP_COLUMNS-1) || (p.nColumn < 0))



// finite state machine stuff

static void fsmKickoffWait();
static void fsmKickoffCharge();
static void fsmKickoffMidair();
static void fsmKickoffRunbackWait();
static void fsmKickoffRunback();
static void fsmKickoffSafety();
static void fsmFormation();
static void fsmInPlay();
static void fsmPassing();
static void fsmSafety();
static void fsmPlayEnded();
static void fsmGameOver();

static enum FSM {
	FSM_KICKOFFWAIT=0,
	FSM_KICKOFFCHARGE,
	FSM_KICKOFFMIDAIR,
	FSM_KICKOFFRUNBACKWAIT,
	FSM_KICKOFFRUNBACK,
	FSM_KICKOFFSAFETY,
	FSM_FORMATION,
	FSM_INPLAY,
	FSM_PASSING,
	FSM_SAFETY,
	FSM_PLAYENDED,
	FSM_GAMEOVER
}fsm;

typedef void (*FSMFCN)();

static FSMFCN fsmfcn[] = {
	fsmKickoffWait,
	fsmKickoffCharge,
	fsmKickoffMidair,
	fsmKickoffRunbackWait,
	fsmKickoffRunback,
	fsmKickoffSafety,
	fsmFormation,
	fsmInPlay,
	fsmPassing,
	fsmSafety,
	fsmPlayEnded,
	fsmGameOver
};


// proto's
static void InitGame();
static void DrawBlips();
static void EraseBlips();



int Football2_GetPower()
{
	return (bPower ? TRUE : FALSE);
}

void Football2_PowerOn()
{
	if (!bPower)
	{
		InitGame();
		bPower = TRUE;
	}
}

void Football2_PowerOff()
{
	bPower = FALSE;
	Football2_StopSound();
}

void Football2_SetSkill(int i){
	if (i == 0){
		bPro2 = FALSE;
	} else {
		bPro2 = TRUE;
	}
}

int Football2_GetSkill(){
	return bPro2 ? 1 : 0;
}

void InitGame()
{
	bHomeTeam = FALSE;
	PlatformSetInput(bHomeTeam);
	
	nHScore = 0;
	nVScore = 0;
	fGameTime = 15.0;
	nDown = -1;
	nQuarter = 0;
	nCurrentYardline = 35;

	bDisplayScore = FALSE;
	bDisplayTime = TRUE;
	bDisplayYard = FALSE;
	bDisplayDown = FALSE;
	bDisplayBlips = TRUE;
	
	fsm = FSM_KICKOFFWAIT;
}

void Football2_Run(int tu)
{
	int x, y;

	// prevent reentrancy
	if (bInFrame){ return; }
	bInFrame = TRUE;

	// init the blips field
	for (y = 0; y < FOOTBALL2_BLIP_ROWS; y++){
		for (x = 0; x < FOOTBALL2_BLIP_COLUMNS; x++){
			Blips[x][y] = BLIP_OFF;
		}
	}

	if (!bPower){
		Football2_ClearScreen();
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
		Football2_DrawScores(nHScore, nVScore);
	}
	
	if (bDisplayYard){
		Football2_DrawYard(TRANSLATE_YARD(nCurrentYardline));
	} else if (bDisplayTime){
		Football2_DrawTime(fGameTime);
	}

	if (bDisplayDown){
		Football2_DrawDown(nDown, nCurrentYardline - nFirstDownYard);
	}

	if (!bDisplayDown && !bDisplayScore){
		Football2_DrawScores(-1, -1);
	}

	if (!bDisplayTime && !bDisplayYard){
		Football2_DrawTime(-1);
	}

	Platform_EndDraw();

	bInFrame = FALSE;

}

void DrawBlips()
{
	int x, y, nBright;
	static BOOL blink = FALSE;

	for (int i=0; i<NUM_DEFENSEPLAYERS; i++){
		if (player[i].nBright != BLIP_OFF){
			x = player[i].nColumn % 10;
			y = player[i].nLane;
			nBright = player[i].nBright;
			Blips[x][y] = nBright;
		}
	}
	if (receiver.nBright != BLIP_OFF){
		x = receiver.nColumn % 10;
		y = receiver.nLane;
		nBright = receiver.nBright;
		Blips[x][y] = nBright;
	}
	if (ball.nBright != BLIP_OFF){
		x = ball.nColumn % 10;
		y = ball.nLane;
		nBright = ball.nBright;
		Blips[x][y] = nBright;
	}

	// draw the blips field
	for (y = 0; y < FOOTBALL2_BLIP_ROWS; y++){
		for (x = 0; x < FOOTBALL2_BLIP_COLUMNS; x++){

			switch(Blips[x][y]){
				case BLIP_OFF:
				case BLIP_DIM:
				case BLIP_BRIGHT:
					Football2_DrawBlip(Blips[x][y], x, y);
					break;
				case BLIP_DIMBLINK:
					if (!blink){
						Football2_DrawBlip(BLIP_DIM, x, y);
					} else {
						Football2_DrawBlip(BLIP_OFF, x, y);
					}
					break;
				case BLIP_BRIGHTBLINK:
					if (blink){
						Football2_DrawBlip(BLIP_BRIGHT, x, y);
					} else {
						Football2_DrawBlip(BLIP_OFF, x, y);
					}
					break;
			}

		}
	}

	blink = !blink;
}

void EraseBlips()
{
	// erase the blips field
	for (int y = 0; y < FOOTBALL2_BLIP_ROWS; y++){
		for (int x = 0; x < FOOTBALL2_BLIP_COLUMNS; x++){
			Football2_DrawBlip(BLIP_OFF, x, y);
		}
	}
}

// FINITE STATE MACHINE STUFF

static void fsmKickoffWait()
{
	bDisplayTime = TRUE;
	bDisplayScore = FALSE;
	bDisplayYard = FALSE;
	bDisplayDown = FALSE;
	bDisplayBlips = FALSE;

	// if clock ran down in previous play
	if (fGameTime <= 0.0){
		if ((nQuarter == 3)){
			// game over
			fsm = FSM_GAMEOVER;
			return;
		} else {
			++nQuarter;
			if (nQuarter == 2){
				// halftime - force kickoff
				// after halftime, home team gets possession
				bHomeTeam = TRUE;
				PlatformSetInput(bHomeTeam);
				ball.nYard = 35;
				nCurrentYardline = ball.nYard;
				nFirstDownYard = nCurrentYardline - 10;
				fGameTime = 15.0;
			} else {
				fGameTime = 15.0;
			}
		}
	}

	SETPLAYER(ball, nCurrentYardline, 5, 1, BLIP_BRIGHTBLINK);

	UNSETPLAYER(receiver);

	SETPLAYER(player[0], 50, 0, 0, BLIP_DIM);
	SETPLAYER(player[1], 50, 1, 1, BLIP_DIMBLINK);
	SETPLAYER(player[2], 50, 0, 2, BLIP_DIM);
	UNSETPLAYER(player[3]);
	UNSETPLAYER(player[4]);
	UNSETPLAYER(player[5]);

	if (Football2_GetInputSCORE(NULL)){
		bDisplayScore = TRUE;
	} else if (Football2_GetInputSTATUS(NULL)){
		bDisplayYard = TRUE;
	} else {
		bDisplayBlips = TRUE;
		if (Football2_GetInputKICK(NULL)){

			Football2_ClearScreen();
			Football2_PlaySound(FOOTBALL2_SOUND_CHARGESTART, PLAYSOUNDFLAGS_PRIORITY);

			SETPLAYER(player[0], 50, 0, 0, BLIP_DIM);
			SETPLAYER(player[1], 50, 1, 1, BLIP_DIM);
			SETPLAYER(player[2], 50, 0, 2, BLIP_DIM);

			nDown = 0;

			fsm = FSM_KICKOFFCHARGE;
			fsmKickoffCharge();
			return;
		}
	}
}

static void fsmKickoffCharge()
{
static BOOL i = TRUE;

	bDisplayTime = FALSE;
	bDisplayScore = FALSE;
	bDisplayYard = TRUE;
	bDisplayDown = FALSE;
	bDisplayBlips = TRUE;

	if (i){
		MOVEPLAYERUPFIELD(player[0]);
		MOVEPLAYERUPFIELD(player[1]);
		MOVEPLAYERUPFIELD(player[2]);
	} else {
		if (player[1].nColumn == ball.nColumn){
			Football2_PlaySound(FOOTBALL2_SOUND_CHARGE, PLAYSOUNDFLAGS_ASYNC | PLAYSOUNDFLAGS_PRIORITY);

			UNSETPLAYER(player[0]);
			UNSETPLAYER(player[1]);
			UNSETPLAYER(player[2]);

			// calculate where ball will land
			nKickoffDestination = Platform_Random(50) + 60;

			SETPLAYERBRIGHTNESS(ball, BLIP_BRIGHTBLINK);
			Platform_IsNewSecond();
			fsm = FSM_KICKOFFMIDAIR;
			fsmKickoffMidair();
			i = TRUE;
			return;
		}
	}
	i = !i;
}

static void fsmKickoffMidair()
{
static BOOL i = TRUE;

	bDisplayTime = FALSE;
	bDisplayScore = FALSE;
	bDisplayYard = TRUE;
	bDisplayDown = FALSE;
	bDisplayBlips = TRUE;

	if (i){
		++nCurrentYardline; // for the display
		MOVEPLAYERUPFIELD(ball);
	} else {
		if (ball.nYard >= nKickoffDestination){
			// back has landed
			Football2_ClearScreen();
			SETPLAYERBRIGHTNESS(ball, BLIP_BRIGHT);
			if (bPunting){

				// put 2 receivers randomly on the screen
				for (int n=0; n<2; n++){
					int i = Platform_Random(NUM_DEFENSEPLAYERS);
					if (player[i].nBright == BLIP_OFF){
						int x, y;
						do {
							// find a random place on the screen
							// don't land on top of anyone
							x = Platform_Random(FOOTBALL2_BLIP_COLUMNS);
							y = Platform_Random(FOOTBALL2_BLIP_ROWS);
						} while (ISOCCUPIED(TRANSLATE_COLUMN(x), y));
						SETPLAYER(player[i], 0, x, y, BLIP_DIM);
					}
				}

				Football2_PlaySound(FOOTBALL2_SOUND_RUNBACK, PLAYSOUNDFLAGS_PRIORITY);
				fsm = FSM_KICKOFFRUNBACKWAIT;

			} else if (bFieldGoalAttempt){
				// field goal attempt failed
				// put ball back to where it was kicked from
				Football2_PlaySound(FOOTBALL2_SOUND_ENDPOSSESSION, PLAYSOUNDFLAGS_PRIORITY);
				ball.nYard = 100 - nKickoffStart;
				nCurrentYardline = ball.nYard;
				nFirstDownYard = ball.nYard - 10;

				SETPLAYERBRIGHTNESS(ball, BLIP_BRIGHTBLINK);
				fsm = FSM_PLAYENDED;
			} else {
				Football2_PlaySound(FOOTBALL2_SOUND_RUNBACK, PLAYSOUNDFLAGS_PRIORITY);
				fsm = FSM_KICKOFFRUNBACKWAIT;
			}
			bPunting = FALSE;
			bFieldGoalAttempt = FALSE;
			return;
		} else if (ball.nYard >= 100){

			Football2_ClearScreen();

			// ball landed in endzone
			if (bFieldGoalAttempt){
				// successful
				if (bHomeTeam){
					nVScore += 3;
				} else {
					nHScore += 3;
				}
				Football2_PlaySound(FOOTBALL2_SOUND_TOUCHDOWN, PLAYSOUNDFLAGS_PRIORITY);
				ball.nYard = 35;
				nCurrentYardline = ball.nYard;
				nFirstDownYard = nCurrentYardline - 35;
				bFieldGoalAttempt = FALSE;
				fsm = FSM_KICKOFFWAIT;
			} else if (bPunting){
				SETPLAYERBRIGHTNESS(ball, BLIP_BRIGHTBLINK);
				Football2_PlaySound(FOOTBALL2_SOUND_ENDPOSSESSION, PLAYSOUNDFLAGS_PRIORITY);
				ball.nYard = MAX_YARD - 20;
				nCurrentYardline = ball.nYard;
				nFirstDownYard = nCurrentYardline - 10;
				bPunting = FALSE;
				fsm = FSM_PLAYENDED;
			} else {
				SETPLAYERBRIGHTNESS(ball, BLIP_BRIGHTBLINK);
				Football2_PlaySound(FOOTBALL2_SOUND_ENDPOSSESSION, PLAYSOUNDFLAGS_PRIORITY);
				bPunting = FALSE;
				fsm = FSM_KICKOFFSAFETY;
			}
			return;
		}
	}
	i = !i;

	if (Platform_IsNewSecond()
		&& (fsm == FSM_KICKOFFMIDAIR)){
		Football2_PlaySound(FOOTBALL2_SOUND_TICK, PLAYSOUNDFLAGS_ASYNC);
	}
}

static void fsmKickoffRunbackWait()
{
	bDisplayTime = TRUE;
	bDisplayScore = FALSE;
	bDisplayYard = FALSE;
	bDisplayDown = FALSE;
	bDisplayBlips = FALSE;

	if (Football2_GetInputSCORE(NULL)){
		bDisplayScore = TRUE;
	} else if (Football2_GetInputSTATUS(NULL)){
		bDisplayYard = TRUE;
	} else {
		bDisplayBlips = TRUE;
	}

	if ((Football2_GetInputLEFT(NULL))
		|| (Football2_GetInputUP(NULL))
		|| (Football2_GetInputRIGHT(NULL))
		|| (Football2_GetInputDOWN(NULL))){
		Platform_IsNewSecond();
		fsm = FSM_KICKOFFRUNBACK;
		fsmKickoffRunback();
		return;
	}
}

static void fsmKickoffRunback()
{
	bDisplayTime = TRUE;
	bDisplayScore = FALSE;
	bDisplayYard = FALSE;
	bDisplayDown = FALSE;
	bDisplayBlips = TRUE;

	BOOL bChange;
	if (Football2_GetInputLEFT(&bChange))
	{
		if (bChange)
		{
			MOVEPLAYERLEFT(ball);
			if (bHomeTeam && (ball.nColumn == (FOOTBALL2_BLIP_COLUMNS-1)))
			{
				MOVEPLAYERRIGHT(ball);
			}
		}
	}
	else if (Football2_GetInputRIGHT(&bChange))
	{
		if (bChange)
		{
			MOVEPLAYERRIGHT(ball);
			if (!bHomeTeam && ((ball.nColumn % FOOTBALL2_BLIP_COLUMNS) == 0))
			{
				MOVEPLAYERLEFT(ball);
			}
		}
	}
	else if (Football2_GetInputUP(&bChange))
	{
		if (bChange)
		{
			if (ball.nLane)
			{
				MOVEPLAYERUP(ball);
			}
		}
	}
	else if (Football2_GetInputDOWN(&bChange))
	{
		if (bChange)
		{
			if (ball.nLane < 2)
			{
				MOVEPLAYERDOWN(ball);
			}
		}
	}

	// check for touchdown
	if (ball.nYard < 0){
		// touchdown!!
		if (bHomeTeam){
			nHScore += 7;
		} else {
			nVScore += 7;
		}
		Football2_ClearScreen();
		Football2_PlaySound(FOOTBALL2_SOUND_TOUCHDOWN, PLAYSOUNDFLAGS_PRIORITY);
		Platform_IsNewSecond();
		bHomeTeam = !bHomeTeam;
		PlatformSetInput(bHomeTeam);
		nCurrentYardline = 35;
		fsm = FSM_KICKOFFWAIT;
		return;
	}


	// check for collisions
	if (ISDEFENSE(ball.nColumn, ball.nLane)){
		// tackled!
		Football2_ClearScreen();
		Football2_PlaySound(FOOTBALL2_SOUND_ENDPLAY, PLAYSOUNDFLAGS_PRIORITY);

		int i = GETPLAYERAT(ball.nColumn, ball.nLane);
		if (i != -1){
			SETPLAYERBRIGHTNESS(player[i], BLIP_DIMBLINK);
		}

		UNMOVEPLAYER(ball);
		SETPLAYERBRIGHTNESS(ball, BLIP_BRIGHTBLINK);

		nFirstDownYard = ball.nYard - 10;

		Platform_IsNewSecond();
		fsm = FSM_PLAYENDED;
		return;
	}


	// move the defense randomly towards the ball
	if (Platform_Random(100) < ((bPro2) ? 25 : 15)){
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

	// randomly add defense players
	if (Platform_Random(10) < 3){
		int i = Platform_Random(NUM_DEFENSEPLAYERS);
		if (player[i].nBright == BLIP_OFF){
			int x, y;
			do {
				// find a random place on the screen
				// don't land on top of anyone
				x = Platform_Random(FOOTBALL2_BLIP_COLUMNS);
				y = Platform_Random(FOOTBALL2_BLIP_ROWS);
			} while (ISOCCUPIED(TRANSLATE_COLUMN(x), y));
			SETPLAYER(player[i], 0, x, y, BLIP_DIM);
		}
	}


	// check for collisions again
	if (ISDEFENSE(ball.nColumn, ball.nLane)){
		// tackled!
		Football2_ClearScreen();
		Football2_PlaySound(FOOTBALL2_SOUND_ENDPLAY, PLAYSOUNDFLAGS_PRIORITY);

		int i = GETPLAYERAT(ball.nColumn, ball.nLane);
		if (i != -1){
			UNMOVEPLAYER(player[i]);
			SETPLAYERBRIGHTNESS(player[i], BLIP_DIMBLINK);
		}

		SETPLAYERBRIGHTNESS(ball, BLIP_BRIGHTBLINK);

		nFirstDownYard = ball.nYard - 10;

		Platform_IsNewSecond();
		fsm = FSM_PLAYENDED;
		return;
	}


	// count down the clock
	if (Platform_IsNewSecond()){
		if (fGameTime > 0.0){
			fGameTime -= (float).1;
			if (fGameTime > 0.0){
				Football2_PlaySound(FOOTBALL2_SOUND_TICK, PLAYSOUNDFLAGS_ASYNC);
			} else {
				// end of quarter!
				fGameTime = 0.0;
				Football2_PlaySound(FOOTBALL2_SOUND_ENDQUARTER, PLAYSOUNDFLAGS_PRIORITY);
			}
		}
	}

}

static void fsmKickoffSafety()
{
	bDisplayTime = TRUE;
	bDisplayScore = FALSE;
	bDisplayYard = FALSE;
	bDisplayDown = FALSE;
	bDisplayBlips = TRUE;

	if ((Football2_GetInputLEFT(NULL))
		|| (Football2_GetInputUP(NULL))
		|| (Football2_GetInputRIGHT(NULL))
		|| (Football2_GetInputDOWN(NULL))
		|| (Football2_GetInputSCORE(NULL))
		|| (Football2_GetInputSTATUS(NULL))){
		nCurrentYardline = MAX_YARD - 20;
		nFirstDownYard = nCurrentYardline - 10;
		fsmFormation();
		fsm = FSM_FORMATION;
		return;
	}
}


static void fsmFormation()
{
	bDisplayTime = TRUE;
	bDisplayScore = FALSE;
	bDisplayYard = FALSE;
	bDisplayDown = FALSE;
	bDisplayBlips = FALSE;

	if (Football2_GetInputSCORE(NULL)){
		bDisplayScore = TRUE;
	} else if (Football2_GetInputSTATUS(NULL)){
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

	UNSETPLAYER(receiver);
	SETPLAYER(ball, nCurrentYardline+1, 7, 1, BLIP_BRIGHT);

	SETPLAYER(player[0], nCurrentYardline-6, 0, 2, BLIP_DIM);
	SETPLAYER(player[1], nCurrentYardline-4, 2, 0, BLIP_DIM);
	SETPLAYER(player[2], nCurrentYardline-2, 4, 1, BLIP_DIM);
	SETPLAYER(player[3], nCurrentYardline-0, 6, 0, BLIP_DIM);
	SETPLAYER(player[4], nCurrentYardline-0, 6, 1, BLIP_DIM);
	SETPLAYER(player[5], nCurrentYardline-0, 6, 2, BLIP_DIM);

	// wait for player to move, then start play
	BOOL bChange;
	if (Football2_GetInputPASS(&bChange))
	{
		if (bChange)
		{
			// pass
			SETPLAYERBRIGHTNESS(ball, BLIP_BRIGHTBLINK);
			nPassingStart = ball.nYard;
			fsm = FSM_PASSING;
			fsmPassing();
			return;
		}
	}
	else if (Football2_GetInputKICK(&bChange))
	{
		if (bChange)
		{
			// punt

			bHomeTeam = !bHomeTeam;
			PlatformSetInput(bHomeTeam);
			nDown = 0;
			ball.nYard = MAX_YARD - ball.nYard;
			nCurrentYardline = ball.nYard;

			UNSETPLAYER(player[0]);
			UNSETPLAYER(player[1]);
			UNSETPLAYER(player[2]);
			UNSETPLAYER(player[3]);
			UNSETPLAYER(player[4]);
			UNSETPLAYER(player[5]);

			// calculate where ball will land
			nKickoffDestination = ball.nYard + (Platform_Random(50) + 10);

			SETPLAYERBRIGHTNESS(ball, BLIP_BRIGHTBLINK);
			Platform_IsNewSecond();
			bPunting = TRUE;
			fsm = FSM_KICKOFFMIDAIR;
			return;
		}
	}
	else if (Football2_TestForMovement())
	{
		bCanKick = TRUE;

		Platform_IsNewSecond();
		if (Platform_Random(2)){
			SETPLAYER(receiver, nCurrentYardline-1, 5, 0, BLIP_DIMBLINK);
		} else {
			SETPLAYER(receiver, nCurrentYardline-1, 5, 2, BLIP_DIMBLINK);
		}

		fsm = FSM_INPLAY;
		fsmInPlay();
		return;
	}
}

static void fsmInPlay()
{
	bDisplayTime = TRUE;
	bDisplayScore = FALSE;
	bDisplayYard = FALSE;
	bDisplayDown = FALSE;
	bDisplayBlips = TRUE;

	BOOL bChange;
	if (Football2_GetInputLEFT(&bChange))
	{
		if (bChange)
		{
			MOVEPLAYERLEFT(ball);
			if (bHomeTeam && (ball.nColumn == (FOOTBALL2_BLIP_COLUMNS-1)))
			{
				MOVEPLAYERRIGHT(ball);
			}
		}
	}
	else if (Football2_GetInputRIGHT(&bChange))
	{
		if (bChange)
		{
			MOVEPLAYERRIGHT(ball);
			if (!bHomeTeam && ((ball.nColumn % FOOTBALL2_BLIP_COLUMNS) == 0))
			{
				MOVEPLAYERLEFT(ball);
			}
		}
	}
	else if (Football2_GetInputUP(&bChange))
	{
		if (bChange)
		{
			if (ball.nLane)
			{
				bCanKick = FALSE;
				MOVEPLAYERUP(ball);
			}
		}
	}
	else if (Football2_GetInputDOWN(&bChange))
	{
		if (bChange)
		{
			if (ball.nLane < 2)
			{
				bCanKick = FALSE;
				MOVEPLAYERDOWN(ball);
			}
		}
	}

	if (Football2_GetInputPASS(NULL) && (ball.nYard >= nCurrentYardline)){
		SETPLAYERBRIGHTNESS(ball, BLIP_BRIGHTBLINK);
		nPassingStart = UNTRANSLATE_COLUMN(ball.nColumn);
		fsm = FSM_PASSING;
		fsmPassing();
		return;
	}
	if (Football2_GetInputKICK(NULL) && bCanKick){

		// field goal attempt!
		bHomeTeam = !bHomeTeam;
		PlatformSetInput(bHomeTeam);
		nDown = 0;
		ball.nYard = MAX_YARD - ball.nYard;
		nKickoffStart = nCurrentYardline;
		nCurrentYardline = ball.nYard;

		UNSETPLAYER(player[0]);
		UNSETPLAYER(player[1]);
		UNSETPLAYER(player[2]);
		UNSETPLAYER(player[3]);
		UNSETPLAYER(player[4]);
		UNSETPLAYER(player[5]);
		UNSETPLAYER(receiver);

		// calculate where ball will land
		nKickoffDestination = ball.nYard + (Platform_Random(30) + 10);
		SETPLAYERBRIGHTNESS(ball, BLIP_BRIGHTBLINK);
		Platform_IsNewSecond();
		bFieldGoalAttempt = TRUE;
		fsm = FSM_KICKOFFMIDAIR;

		return;
	}

	int nYardsToGo = ball.nYard - nFirstDownYard;

	// 1st down
	if (nYardsToGo <= 0){
		if (!bGotFirstDown){
			Football2_PlaySound(FOOTBALL2_SOUND_FIRSTDOWN, PLAYSOUNDFLAGS_PRIORITY);
			bGotFirstDown = TRUE;
		}
	} else {
		bGotFirstDown = FALSE;
	}


	// get rid of the receiver once we've
	// passed the line of scrimmage
	if (ball.nYard < nCurrentYardline){
		UNSETPLAYER(receiver);
	}

	// check for touchdown
	if (ball.nYard < 0){
		// touchdown!!
		if (bHomeTeam){
			nHScore += 7;
		} else {
			nVScore += 7;
		}
		Football2_ClearScreen();
		Football2_PlaySound(FOOTBALL2_SOUND_TOUCHDOWN, PLAYSOUNDFLAGS_PRIORITY);
		Platform_IsNewSecond();
		bHomeTeam = !bHomeTeam;
		PlatformSetInput(bHomeTeam);
		nCurrentYardline = 35;
		fsm = FSM_KICKOFFWAIT;
		return;
	}

	// check for collisions
	if (ISDEFENSE(ball.nColumn, ball.nLane)){
		// tackled!
		Football2_ClearScreen();

		int i = GETPLAYERAT(ball.nColumn, ball.nLane);
		if (i != -1){
			SETPLAYERBRIGHTNESS(player[i], BLIP_DIMBLINK);
		}

		UNMOVEPLAYER(ball);
		SETPLAYERBRIGHTNESS(ball, BLIP_BRIGHTBLINK);
		UNSETPLAYER(receiver);

		if (ball.nYard  > 100){

			// safety
			Football2_ClearScreen();
			Football2_PlaySound(FOOTBALL2_SOUND_SAFETY, PLAYSOUNDFLAGS_PRIORITY);
			fsm = FSM_SAFETY;
			return;

		} else if ((++nDown >= 4) && (!bGotFirstDown)){

			// give the ball to the other team
			bHomeTeam = !bHomeTeam;
			PlatformSetInput(bHomeTeam);
			nDown = 0;
			ball.nYard = MAX_YARD - ball.nYard;
			nFirstDownYard = ball.nYard - 10;

			Football2_PlaySound(FOOTBALL2_SOUND_ENDPOSSESSION, PLAYSOUNDFLAGS_PRIORITY);
			fsm = FSM_PLAYENDED;
			return;

		} else {
			Football2_PlaySound(FOOTBALL2_SOUND_ENDPLAY, PLAYSOUNDFLAGS_PRIORITY);
			fsm = FSM_PLAYENDED;
			return;
		}

		return;
	}


	// move the defense randomly towards the ball
	if (Platform_Random(100) < ((bPro2) ? 25 : 15)){
		int i = Platform_Random(NUM_DEFENSEPLAYERS);
		if (player[i].nBright){
			// pick horizontal or vertical movement toward the ball
			if (Platform_Random(2) == 0){
				if (player[i].nColumn < ball.nColumn){
					if (!ISDEFENSE(player[i].nColumn+1, player[i].nLane)
						&& !ISRECEIVER(player[i].nColumn+1, player[i].nLane)){
						NOTECURRENTPLAYERPOSITION(player[i]);
						player[i].nColumn++;
					}
				} else if (player[i].nColumn > ball.nColumn){
					if (!ISDEFENSE(player[i].nColumn-1, player[i].nLane)
						&& !ISRECEIVER(player[i].nColumn-1, player[i].nLane)){
						NOTECURRENTPLAYERPOSITION(player[i]);
						player[i].nColumn--;
					}
				}
			} else {
				if (player[i].nLane < ball.nLane){
					if (!ISDEFENSE(player[i].nColumn, player[i].nLane+1)
						&& !ISRECEIVER(player[i].nColumn, player[i].nLane+1)){
						NOTECURRENTPLAYERPOSITION(player[i]);
						player[i].nLane++;
					}
				} else if (player[i].nLane > ball.nLane){
					if (!ISDEFENSE(player[i].nColumn, player[i].nLane-1)
						&& !ISRECEIVER(player[i].nColumn, player[i].nLane-1)){
						NOTECURRENTPLAYERPOSITION(player[i]);
						player[i].nLane--;
					}
				}
			}
		}
	}


	// move the receiver randomly
	if (Platform_Random(100) < ((bPro2) ? 15 : 5)){
		if (receiver.nBright){
			// pick horizontal or vertical movement toward the ball
			if (Platform_Random(4) == 0){
				if (receiver.nLane < ball.nLane){
					if (!ISOCCUPIED(receiver.nColumn, receiver.nLane+1)){
						NOTECURRENTPLAYERPOSITION(receiver);
						receiver.nLane++;
					}
				} else if (receiver.nLane > ball.nLane){
					if (!ISOCCUPIED(receiver.nColumn, receiver.nLane-1)){
						NOTECURRENTPLAYERPOSITION(receiver);
						receiver.nLane--;
					}
				}
			} else {
				if (Platform_Random(3) == 0){
					int dir = (bHomeTeam) ? 1 : -1;
					if (!ISOCCUPIED(receiver.nColumn + dir, receiver.nLane)){
						NOTECURRENTPLAYERPOSITION(receiver);
						receiver.nColumn += dir;
						if (ISPLAYEROFFSCREEN(receiver)){
							UNMOVEPLAYER(receiver);
						}
					}
				}
			}

		}
	}


	// check for collisions again
	if (ISDEFENSE(ball.nColumn, ball.nLane)){
		// tackled!
		Football2_ClearScreen();

		int i = GETPLAYERAT(ball.nColumn, ball.nLane);
		if (i != -1){
			UNMOVEPLAYER(player[i]);
			SETPLAYERBRIGHTNESS(player[i], BLIP_DIMBLINK);
		}

		SETPLAYERBRIGHTNESS(ball, BLIP_BRIGHTBLINK);
		UNSETPLAYER(receiver);

		if (ball.nYard  > 100){

			// safety
			Football2_ClearScreen();
			Football2_PlaySound(FOOTBALL2_SOUND_SAFETY, PLAYSOUNDFLAGS_PRIORITY);
			fsm = FSM_SAFETY;
			return;

		} else if ((++nDown >= 4) && (!bGotFirstDown)){

			// give the ball to the other team
			bHomeTeam = !bHomeTeam;
			PlatformSetInput(bHomeTeam);
			nDown = 0;
			ball.nYard = MAX_YARD - ball.nYard;
			nFirstDownYard = ball.nYard - 10;

			Football2_PlaySound(FOOTBALL2_SOUND_ENDPOSSESSION, PLAYSOUNDFLAGS_PRIORITY);
			fsm = FSM_PLAYENDED;
			return;
		} else {
			Football2_PlaySound(FOOTBALL2_SOUND_ENDPLAY, PLAYSOUNDFLAGS_PRIORITY);
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
				Football2_PlaySound(FOOTBALL2_SOUND_TICK, PLAYSOUNDFLAGS_ASYNC);
			} else {
				// end of quarter!
				fGameTime = 0.0;
				Football2_PlaySound(FOOTBALL2_SOUND_ENDQUARTER, PLAYSOUNDFLAGS_PRIORITY);
			}
		}
	}
}

static void fsmPassing()
{
static int i = 0;
BOOL bIntercepted = FALSE;

	bDisplayTime = TRUE;
	bDisplayScore = FALSE;
	bDisplayYard = FALSE;
	bDisplayDown = FALSE;
	bDisplayBlips = TRUE;

	if (i==0){

		MOVEPLAYERDOWNFIELD(ball);

		if ((ball.nColumn == receiver.nColumn)
			&& (ball.nLane == receiver.nLane)){

			// completed pass
			UNSETPLAYER(receiver);
			SETPLAYERBRIGHTNESS(ball, BLIP_BRIGHT);
			fsm = FSM_INPLAY;

		} else if ((ball.nColumn == 0)
			|| (ball.nColumn == FOOTBALL2_BLIP_COLUMNS-1)){

			// incomplete pass
			Football2_ClearScreen();
			++nDown;
			ball.nYard = nCurrentYardline;
			if ((++nDown >= 4) && (!bGotFirstDown)){

				// give the ball to the other team
				bHomeTeam = !bHomeTeam;
				PlatformSetInput(bHomeTeam);
				nDown = 0;
				ball.nYard = MAX_YARD - ball.nYard;
				nFirstDownYard = ball.nYard - 10;

				Football2_PlaySound(FOOTBALL2_SOUND_ENDPOSSESSION, PLAYSOUNDFLAGS_PRIORITY);

			} else {
				Football2_PlaySound(FOOTBALL2_SOUND_ENDPLAY, PLAYSOUNDFLAGS_PRIORITY);
			}

			Platform_IsNewSecond();
			fsm = FSM_PLAYENDED;

		} else if (ISDEFENSE(ball.nColumn, ball.nLane)){

			// ** The passed ball can only pass over defense players
			// on the offense side of the line of scrimmage. All others
			// will intercept the ball (per instruction manual)

			if (UNTRANSLATE_COLUMN(ball.nColumn) >= 3)
			{
				// intercepted!
				Football2_ClearScreen();
				Football2_PlaySound(FOOTBALL2_SOUND_ENDPOSSESSION, PLAYSOUNDFLAGS_PRIORITY);

				UNSETPLAYER(receiver);

				int n = GETPLAYERAT(ball.nColumn, ball.nLane);
				if (n != -1){
					SETPLAYERBRIGHTNESS(player[n], BLIP_DIMBLINK);
				}

				// unmove the ball
				UNMOVEPLAYER(ball);

				// give the ball to the other team
				bHomeTeam = !bHomeTeam;
				PlatformSetInput(bHomeTeam);
				nDown = 0;
				ball.nYard = MAX_YARD - ball.nYard;
				nFirstDownYard = ball.nYard - 10;

				Platform_IsNewSecond();
				fsm = FSM_PLAYENDED;
			}
		}
	}

	if ((++i == 5) || (fsm != FSM_PASSING)){
		i = 0;
	}

	// count down the clock
	if (Platform_IsNewSecond()){
		if (fGameTime > 0.0){
			fGameTime -= (float).1;
			if (fGameTime > 0.0){
				Football2_PlaySound(FOOTBALL2_SOUND_TICK, PLAYSOUNDFLAGS_ASYNC);
			} else {
				// end of quarter!
				fGameTime = 0.0;
				Football2_PlaySound(FOOTBALL2_SOUND_ENDQUARTER, PLAYSOUNDFLAGS_PRIORITY);
			}
		}
	}
}

static void fsmSafety()
{
	bDisplayTime = TRUE;
	bDisplayScore = FALSE;
	bDisplayYard = FALSE;
	bDisplayDown = FALSE;
	bDisplayBlips = TRUE;

	UNSETPLAYER(receiver);

	// check for game over
	if ((fGameTime <= 0.0) && (nQuarter == 3)){
		// add the 2 points for the safety
		if (bHomeTeam){
			nVScore += 2;
		} else {
			nHScore += 2;
		}

		fsm = FSM_GAMEOVER;
		return;
	}

	if (Football2_GetInputSCORE(NULL) || Football2_GetInputSTATUS(NULL)){
		// if clock ran down in previous play
		if (fGameTime < 0.1){
			++nQuarter;
			if (nQuarter == 2){
				// halftime - force kickoff
				bHomeTeam = !bHomeTeam;
				PlatformSetInput(bHomeTeam);
				ball.nYard = 35;
				nCurrentYardline = ball.nYard;
				nFirstDownYard = nCurrentYardline - 35;

				// add the 2 points for the safety
				if (bHomeTeam){
					nVScore += 2;
				} else {
					nHScore += 2;
				}

				fGameTime = 15.0;
				fsm = FSM_KICKOFFWAIT;
				return;
			} else {
				fGameTime = 15.0;
			}
		}

		// add 2 points and set up kickoff from 20 yardline
		if (bHomeTeam){
			nVScore += 2;
		} else {
			nHScore += 2;
		}
		bHomeTeam = !bHomeTeam;
		PlatformSetInput(bHomeTeam);
		ball.nYard = 20;
		nCurrentYardline = ball.nYard;
		nFirstDownYard = nCurrentYardline - 10;

		fsm = FSM_KICKOFFWAIT;
		fsmKickoffWait();
		return;
	}

}

static void fsmPlayEnded()
{
	bDisplayTime = TRUE;
	bDisplayScore = FALSE;
	bDisplayYard = FALSE;
	bDisplayDown = FALSE;
	bDisplayBlips = TRUE;

	// check for game over
	if ((fGameTime <= 0.0) && (nQuarter == 3)){
		fsm = FSM_GAMEOVER;
		return;
	}

	if ((Football2_GetInputSCORE(NULL))
		|| (Football2_GetInputSTATUS(NULL))){
		// if clock ran down in previous play
		if (fGameTime < 0.1){
			++nQuarter;
			if (nQuarter == 2){
				// halftime - force kickoff
				// after halftime, home team gets possession
				bHomeTeam = TRUE;
				PlatformSetInput(bHomeTeam);
				ball.nYard = 35;
				nCurrentYardline = ball.nYard;
				nFirstDownYard = nCurrentYardline - 10;

				fGameTime = 15.0;
				fsm = FSM_KICKOFFWAIT;
				return;
			} else {
				fGameTime = 15.0;
			}
		}

		nCurrentYardline = ball.nYard;

		fsm = FSM_FORMATION;
		fsmFormation();
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





