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

#include "AutoRace.h"
#include "Games.h"

// constants

#define TIMER_CARADVANCE		10

typedef int BLIP;

static BLIP Blips[AUTORACE_BLIP_COLUMNS][AUTORACE_BLIP_ROWS];

typedef struct ONCOMINGCAR{
	int nRow;
	int nColumn;
	BOOL bActive;
}ONCOMINGCAR;

static ONCOMINGCAR sOncomingCarA, sOncomingCarB;


// game variables
static int nLaneSelector;
static int nGear;
static int nCarRow;
static int nGameTimer;
static int nLaps;
static BOOL bGameEnd;		// 1 = loose, 2 = win
static BOOL bInFrame = FALSE;
static BOOL bPower;
static int nCarAdvanceTimer;
static int nGearCounter;

static void	fsmPlayStartWait();
static void	fsmInPlay();
static void fsmCrash();
static void	fsmEndLap();
static void	fsmEndGame();

static enum FSM {
	FSM_PLAYSTARTWAIT=0,
	FSM_INPLAY,
	FSM_CRASH,
	FSM_ENDLAP,
	FSM_ENDGAME
}fsm;

typedef void (*FSMFCN)();

static FSMFCN fsmfcn[] = {
	fsmPlayStartWait,
	fsmInPlay,
	fsmCrash,
	fsmEndLap,
	fsmEndGame
};

static void InitGame();
static void UpdateOurCar();
static void UpdateOncomingCars();
static void DoHitTest();
static void UpdateBlips();
static void StartSecondCar();

BOOL AutoRace_GetPower()
{
	return (bPower ? TRUE : FALSE);
}

void AutoRace_PowerOn()
{
	InitGame();
	bPower = TRUE;
}

void AutoRace_PowerOff()
{
	bPower = FALSE;
	AutoRace_StopSound();
	AutoRace_StopEngineSound();
}

int AutoRace_GetSkill()
{
	return nGear;
}

void AutoRace_SetSkill(int i)
{
	if (i > 3) { 
		i = 3; 
	}
	else if (i < 0) { 
		i = 0; 
	}
	nGear = i;
}

void InitGame()
{
	// set our car's position
	nLaneSelector = 1;
	nCarRow = AUTORACE_BLIP_ROWS - 1;

	// init the other cars
	sOncomingCarA.bActive = FALSE;
	sOncomingCarB.bActive = FALSE;

	nGameTimer = 0;
	bGameEnd = 0;
	nLaps = 0;

	nCarAdvanceTimer = TIMER_CARADVANCE;
	nGearCounter = 999;
	fsm = FSM_PLAYSTARTWAIT;
}

void AutoRace_Run(int tu)
{
	int x, y;

	// prevent reentrancy
	if (bInFrame){ return; }
	bInFrame = TRUE;

	// init the blips field
	for (y = 0; y < AUTORACE_BLIP_ROWS; y++){
		for (x = 0; x < AUTORACE_BLIP_COLUMNS; x++){
			Blips[x][y] = BLIP_OFF;
		}
	}
	if (!bPower){
		fsm = FSM_PLAYSTARTWAIT;
	}
	Platform_StartDraw();

	(fsmfcn[fsm])();

	UpdateBlips();

	// draw the blips field
	for (int y = 0; y < AUTORACE_BLIP_ROWS; y++){
		for (int x = 0; x < AUTORACE_BLIP_COLUMNS; x++){
			AutoRace_DrawBlip(Blips[x][y], x, y);
		}
	}

	// draw the score
	AutoRace_DrawTime(nGameTimer);

	Platform_EndDraw();

	bInFrame = FALSE;
}

void fsmInPlay()
{
	BOOL bMoveCars = FALSE;
	
	if (Platform_IsNewSecond()){
		// update the game timer
		++nGameTimer;
		if (nGameTimer == 99){
			// time's up -- game over!
			bGameEnd = 1;
			fsm = FSM_ENDGAME;
			return;
		}
	}

	// get the current stick position
	nLaneSelector = AutoRace_GetInputSTICK();

	// check for gear changes
	nGear = AutoRace_GetInputGEAR(NULL);

	// move the cars at the correct speed per gear selection
	AutoRace_PlayEngineSound();

	switch(nGear){
	case 0:
		if (nGearCounter > 3){
			nGearCounter = 0;
			bMoveCars = TRUE;
		}
		break;
	case 1:
		if (nGearCounter > 2){
			nGearCounter = 0;
			bMoveCars = TRUE;
		}
		break;
	case 2:
		if (nGearCounter > 1){
			nGearCounter = 0;
			bMoveCars = TRUE;
		}
		break;
	case 3:
		// move every frame
		bMoveCars = TRUE;
		nGearCounter = 0;
		break;
	}
	++nGearCounter;

	if (bMoveCars){
		UpdateOncomingCars();
		DoHitTest();
		UpdateOurCar();
		DoHitTest();
	}
}

void UpdateOurCar()
{
	if (nCarAdvanceTimer){
		--nCarAdvanceTimer;
	}
	if (nCarAdvanceTimer == 0){

		// move our car up
		--nCarRow;
		if (nCarRow <= 0){
			// lap completed!
			fsm = FSM_ENDLAP;
		}
		nCarAdvanceTimer = TIMER_CARADVANCE;
	}
}

void UpdateOncomingCars()
{

	// move the oncoming cars down
	for (int i=0; i<2; i++){

		ONCOMINGCAR *pOncomingCar = i ? &sOncomingCarB : &sOncomingCarA;

		if (pOncomingCar->bActive){
			pOncomingCar->nRow++;
			if (pOncomingCar->nRow >= AUTORACE_BLIP_ROWS){
				// car has left the screen
				pOncomingCar->bActive = FALSE;
			}
		}

	}

	// count the number of oncoming cars
	// should be at least 1 car at all times
	// maximum 2 oncoming cars at any given time

	if (!sOncomingCarA.bActive && !sOncomingCarB.bActive){

		// no cars are approaching, start at least one
		sOncomingCarA.bActive = TRUE;
		sOncomingCarA.nRow = -2;
		sOncomingCarA.nColumn = Platform_Random(3);

		// and perhaps start a second car
		if (Platform_Random(5) == 0){
			StartSecondCar();
		}

	}

	if (!sOncomingCarA.bActive || !sOncomingCarB.bActive){
		// only 1 car is active -- randomly try to start a second car
		if (Platform_Random(10) == 0){
			StartSecondCar();
		}
	}
}

void StartSecondCar()
{
	if (sOncomingCarA.bActive && sOncomingCarB.bActive){
		// both cars are already active
		return;
	}

	ONCOMINGCAR *pNewOncomingCar, *pOtherOncomingCar;
	if (sOncomingCarA.bActive){
		pNewOncomingCar = &sOncomingCarB;
		pOtherOncomingCar = &sOncomingCarA;
	} else {
		pNewOncomingCar = &sOncomingCarA;
		pOtherOncomingCar = &sOncomingCarB;
	}

	pNewOncomingCar->bActive = TRUE;
	pNewOncomingCar->nRow = -2;
	pNewOncomingCar->nColumn = Platform_Random(3);

	if ((pNewOncomingCar->nColumn == pOtherOncomingCar->nColumn)
		&& (pNewOncomingCar->nRow == pOtherOncomingCar->nRow)
		&& pOtherOncomingCar->bActive){
		// car is on top of other car -- reposition the car behind the other car
		pNewOncomingCar->nRow = pNewOncomingCar->nRow-1;
	}

}

void DoHitTest()
{
	for (int i=0; i<2; i++){
		ONCOMINGCAR *pOncomingCar = i ? &sOncomingCarB : &sOncomingCarA;

		if (pOncomingCar->bActive){
			if ((pOncomingCar->nRow == nCarRow)
				&& (pOncomingCar->nColumn == nLaneSelector)){

				// hit an oncoming car!
				fsm = FSM_CRASH;
				if (nCarRow < (AUTORACE_BLIP_ROWS - 1)){
					++nCarRow;
				} 
				else {
					// if oncoming car is in the last row and hits us
					// delete it to keep it from hitting us every frame
					// (since our car can't be moved out of the way)
					pOncomingCar->bActive = FALSE;
				}
			}
		}
	}
}

void UpdateBlips()
{
	// draw the oncoming car blips
	for (int i=0; i<2; i++){
		ONCOMINGCAR *pOncomingCar = i ? &sOncomingCarB : &sOncomingCarA;
		if (pOncomingCar->bActive){
			if ((pOncomingCar->nColumn >= 0) && (pOncomingCar->nColumn < AUTORACE_BLIP_COLUMNS)
				&& (pOncomingCar->nRow >= 0) && (pOncomingCar->nRow < AUTORACE_BLIP_ROWS)){
				Blips[pOncomingCar->nColumn][pOncomingCar->nRow] = (fsm == FSM_CRASH) ? BLIP_BRIGHT : BLIP_DIM;
			} 
			else if ((pOncomingCar->nColumn >= 0) && (pOncomingCar->nColumn < AUTORACE_BLIP_COLUMNS)
				&& (pOncomingCar->nRow == -1)){
				// oncoming car blips display on-screen for an extra frame when they first appear
				// (to give you a better chance of avoiding them when our car is near the top)
				// put them on-screen even if they are just off-screen
				Blips[pOncomingCar->nColumn][0] = (fsm == FSM_CRASH) ? BLIP_BRIGHT : BLIP_DIM;
			}
		}
	}

	// draw the player's car blip
	if ((nCarRow <= (AUTORACE_BLIP_ROWS - 1))
		&& (nCarRow >= 0)){
		Blips[nLaneSelector][nCarRow] = BLIP_BRIGHT;
	}

}

void fsmPlayStartWait()
{
	nLaneSelector = AutoRace_GetInputSTICK();

	// check for gear changes
	nGear = AutoRace_GetInputGEAR(NULL);

	if (bPower){
		Blips[nLaneSelector][nCarRow] = BLIP_BRIGHT;
	} 
	else {
		AutoRace_ClearScreen();
	}
	
	// wait for 1st gear before starting
	if (nGear == 0){		
		Platform_IsNewSecond();
		fsm = FSM_INPLAY;
	}
}

void fsmCrash()
{
	AutoRace_PlaySound(AUTORACE_SOUND_HIT, PLAYSOUNDFLAGS_PRIORITY);	
	fsm = FSM_INPLAY;
}

void fsmEndLap()
{
	// lap completed!
	AutoRace_StopEngineSound();
	Platform_Pause(300);
	nCarRow = AUTORACE_BLIP_ROWS - 1;
	++nLaps;
	if (nLaps >= 4){
		// finished race -- game over!
		bGameEnd = 2;
		fsm = FSM_ENDGAME;
	} 
	else {
		AutoRace_PlayEngineSound();
		fsm = FSM_INPLAY;
	}
}

void fsmEndGame()
{
	AutoRace_ClearScreen();
	AutoRace_StopEngineSound();
	
	switch(bGameEnd) {
	case 1:
		AutoRace_PlaySound(AUTORACE_SOUND_TIME, PLAYSOUNDFLAGS_PRIORITY);
		break;
	case 2:
		AutoRace_PlaySound(AUTORACE_SOUND_WIN, PLAYSOUNDFLAGS_PRIORITY);
		break;
	default:
		break;
	}
	bGameEnd = 0;
}