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



#include "SpaceAlert.h"
#include "Games.h"


// constants

#define TIME_ATTACKWAVE			20
#define TIME_SHOWHIT			10

#define RAIDER_COUNT			20
//#define MISSILE_LIMIT			(-10)
#define MISSILE_LIMIT			(-7)

typedef int BLIP;

static BLIP Blips[SPACEALERT_BLIP_COLUMNS][SPACEALERT_BLIP_ROWS];

typedef struct tagRaider {
	int nColumn;
	int nRow;

	int nSlow;

	BOOL bAttacking;

	BOOL bLeftColumn;
	BOOL bMidColumn;
	BOOL bRightColumn;
}RAIDER;

static RAIDER sRaiderA, sRaiderB;


// game variables
static int nColumnSelector;
static int nCurrentMissileRow;
static int nIndexAttackWave;
static int nIndexAttackFrame;
static int nTimerAttackWave;
static int nRaiderCount;
static int nPoints;
static BOOL bGameOver;
static BOOL bWin;
static BOOL bInFrame = FALSE;
static BOOL bPower;

static void InitGame();
static void InitAttack();
static void DoMissileUpdate();
static void DoRaidersUpdate();
static void DoHitTest();
static void UpdateBlips();
static void PaintGame();


BOOL SpaceAlert_GetPower()
{
	return (bPower ? TRUE : FALSE);
}

void SpaceAlert_PowerOn()
{
	InitGame();
	bPower = TRUE;
}

void SpaceAlert_PowerOff()
{
	bPower = FALSE;
	SpaceAlert_StopSound();
	SpaceAlert_StopRaiderSound();
}

void InitGame()
{

	nColumnSelector = 1;
	nCurrentMissileRow = SPACEALERT_BLIP_ROWS - 1;

	nRaiderCount = RAIDER_COUNT;
	nIndexAttackWave = 0;
	nIndexAttackFrame = 0;

	nTimerAttackWave = 1;

	nPoints = 0;

	bGameOver = FALSE;
	bWin = TRUE;

	sRaiderA.bAttacking = FALSE;

}


void SpaceAlert_Run(int tu)
{
	int x, y;

	// prevent reentrancy
	if (bInFrame){ return; }
	bInFrame = TRUE;

	// init the blips field
	for (y = 0; y < SPACEALERT_BLIP_ROWS; y++){
		for (x = 0; x < SPACEALERT_BLIP_COLUMNS; x++){
			Blips[x][y] = BLIP_OFF;
		}
	}
	if (bPower){
		if (bGameOver){
			// handle game over
			static BOOL bFlash = 0;
			if (bWin){
				// win
				Blips[0][SPACEALERT_BLIP_ROWS-2] = BLIP_BRIGHT;
				Blips[1][SPACEALERT_BLIP_ROWS-2] = BLIP_BRIGHT;
				Blips[2][SPACEALERT_BLIP_ROWS-2] = BLIP_BRIGHT;
			} else {
				// lose
				if (bFlash){
					bFlash = FALSE;
					Blips[1][SPACEALERT_BLIP_ROWS-1] = BLIP_BRIGHT;
				} else {
					bFlash = TRUE;
				}
			}

			// allow stick and fire button to be moved
			SpaceAlert_GetInputFIRE(NULL);
			SpaceAlert_GetInputSTICK();

			// draw the frame
			PaintGame();
			bInFrame = FALSE;
			return;
		}
	} else {

		// allow stick and fire button to be moved
		SpaceAlert_GetInputFIRE(NULL);
		SpaceAlert_GetInputSTICK();

		SpaceAlert_ClearScreen();
		bInFrame = FALSE;
		return;
	}

	// get the current stick position
	nColumnSelector = SpaceAlert_GetInputSTICK();

	{
		DoHitTest();

		DoRaidersUpdate();

		UpdateBlips();
		PaintGame();

		DoHitTest();

		DoMissileUpdate();
	}

	bInFrame = FALSE;

}


void DoMissileUpdate()
{
	// update the missiles
	if (SpaceAlert_GetInputFIRE(NULL))
	{
		// fire
		if (nCurrentMissileRow == (SPACEALERT_BLIP_ROWS - 1))
		{
			SpaceAlert_PlaySound(SPACEALERT_SOUND_FIRE, PLAYSOUNDFLAGS_ASYNC | PLAYSOUNDFLAGS_PRIORITY);
			--nCurrentMissileRow;
			return;
		}
	}

	// move any active missiles
	if (nCurrentMissileRow < (SPACEALERT_BLIP_ROWS - 1))
	{
		if (nCurrentMissileRow >= MISSILE_LIMIT){
			// move missile up the screen
			--nCurrentMissileRow;
		} else {
			// missile has hit its limit offscreen -- show new missile
			nCurrentMissileRow = SPACEALERT_BLIP_ROWS - 1;
		}
	}
}

void DoRaidersUpdate()
{

	// update the attack
	if (nTimerAttackWave > 0){
		--nTimerAttackWave;
		if (nTimerAttackWave == 0){
			// start a new attack wave
			nIndexAttackWave = 0;
			InitAttack();
		}
	} else {
		nTimerAttackWave = 0;
	}

	// handle the raiders
	for (int i=0; i<2; i++){

		RAIDER *sRaiderP = i ? &sRaiderB : &sRaiderA;
		RAIDER *sOtherRaiderP = i ? &sRaiderA : &sRaiderB;

		if (sRaiderP->bAttacking){

			// raider is attacking

			if (sRaiderP->nSlow){
				// handle the slow raiders
				if (sRaiderP->nSlow == 1){
					sRaiderP->nSlow = 2;
				} else {
					sRaiderP->nSlow = 1;
				}
			}

			if (sRaiderP->nSlow <= 1){

				// advance the raider
				sRaiderP->nRow++;

				if (sRaiderP->nRow >= SPACEALERT_BLIP_ROWS){

					// miss
					sRaiderP->bAttacking = FALSE;
					--nRaiderCount;

					if (sRaiderP->nColumn == 1){

						// battlestart hit -- lose
						// draw a blank screen during the tune
						sRaiderA.bAttacking = FALSE;
						sRaiderB.bAttacking = FALSE;
						nCurrentMissileRow = -1;
						SpaceAlert_ClearScreen();
						SpaceAlert_StopRaiderSound();
						SpaceAlert_PlaySound(SPACEALERT_SOUND_LOSE, PLAYSOUNDFLAGS_PRIORITY);
						bGameOver = TRUE;
						bWin = FALSE;
						return;

					}
					else if (nRaiderCount <= 0){

						// no more raiders -- win
						// draw a blank screen during the tune
						sRaiderA.bAttacking = FALSE;
						sRaiderB.bAttacking = FALSE;
						nCurrentMissileRow = -1;
						SpaceAlert_ClearScreen();
						SpaceAlert_StopRaiderSound();
						SpaceAlert_PlaySound(SPACEALERT_SOUND_WIN, PLAYSOUNDFLAGS_PRIORITY);
						bGameOver = TRUE;
						bWin = TRUE;
						return;

					} else {
						// set up next wave
						nTimerAttackWave = TIME_ATTACKWAVE;
					}

				}

				// change lanes randomly (don't change on last rows)
				if (sRaiderP->nRow < SPACEALERT_BLIP_ROWS - 2){
					if (Platform_Random(7) == 0){
						// can only be in each column once per attack
						switch(sRaiderP->nColumn){
							case 0:
								sRaiderP->bLeftColumn = TRUE;
								if ((!sRaiderP->bMidColumn)
									&& (sOtherRaiderP->nColumn != 1)
									&& (sRaiderP->nRow != sOtherRaiderP->nRow))
								{
									sRaiderP->nColumn = 1;
								}
								break;
							case 1:
								sRaiderP->bMidColumn = TRUE;
								if (Platform_Random(2))
								{
									if ((!sRaiderP->bLeftColumn)
										&& (sOtherRaiderP->nColumn != 0)
										&& (sRaiderP->nRow != sOtherRaiderP->nRow))
									{
										sRaiderP->nColumn = 0;
									}
								} else {
									if ((!sRaiderP->bRightColumn)
										&& (sOtherRaiderP->nColumn != 2)
										&& (sRaiderP->nRow != sOtherRaiderP->nRow))
									{
										sRaiderP->nColumn = 2;
									}
								}
								break;
							case 2:
								sRaiderP->bRightColumn = TRUE;
								if ((!sRaiderP->bMidColumn)
									&& (sOtherRaiderP->nColumn != 1)
									&& (sRaiderP->nRow != sOtherRaiderP->nRow))
								{
									sRaiderP->nColumn = 1;
								}
								break;
						}
					}
				}
			}
		}
	}

	// enable/disable raiders sound
	if ((sRaiderA.bAttacking && (sRaiderA.nRow >= 0) && (sRaiderA.nRow < SPACEALERT_BLIP_ROWS))
		|| (sRaiderB.bAttacking && (sRaiderB.nRow >= 0) && (sRaiderB.nRow < SPACEALERT_BLIP_ROWS))){
		SpaceAlert_PlayRaiderSound();
	} else {
		SpaceAlert_StopRaiderSound();
	}

}

void DoHitTest()
{
	if (nCurrentMissileRow < 0)
	{
		// missile is off screen
		return;
	}

	for (int i=0; i<2; i++){
		RAIDER *sRaiderP = i ? &sRaiderB : &sRaiderA;
		if (sRaiderP->bAttacking){
			if (((nCurrentMissileRow) == sRaiderP->nRow)
				&& (nColumnSelector == sRaiderP->nColumn)){

				if ((nCurrentMissileRow == (SPACEALERT_BLIP_ROWS-1))
					&& (nColumnSelector == 1)){
					// don't count 0 point hits to center column if not fired
				} else {

					// hit a raider
					sRaiderP->bAttacking = FALSE;
					UpdateBlips();

					// draw the blips field (minus the score)
					{
						SpaceAlert_ClearScreen();
						Platform_StartDraw();

						for (int y = 0; y < SPACEALERT_BLIP_ROWS; y++){
							for (int x = 0; x < SPACEALERT_BLIP_COLUMNS; x++){
								SpaceAlert_DrawBlip(Blips[x][y], x, y);
							}
						}

						Platform_EndDraw();
					}


					SpaceAlert_PlaySound(SPACEALERT_SOUND_HIT, PLAYSOUNDFLAGS_PRIORITY);

					// add the score
					nPoints += (SPACEALERT_BLIP_ROWS-1) - nCurrentMissileRow;

					// recharge the missile
					nCurrentMissileRow = SPACEALERT_BLIP_ROWS - 1;

					--nRaiderCount;

					if (nRaiderCount <= 0){
						// no more raiders -- win
						// draw a blank screen during the tune
						sRaiderA.bAttacking = FALSE;
						sRaiderB.bAttacking = FALSE;
						nCurrentMissileRow = -1;
						SpaceAlert_ClearScreen();
						SpaceAlert_StopRaiderSound();
						SpaceAlert_PlaySound(SPACEALERT_SOUND_WIN, PLAYSOUNDFLAGS_PRIORITY);
						bGameOver = TRUE;
						bWin = TRUE;
						return;
					} else {
						// set up next wave
						nTimerAttackWave = TIME_ATTACKWAVE - ((Platform_Random(4) == 0) ? (TIME_ATTACKWAVE / 2) : 0);
					}


				}

			}
		}
	}
}


void UpdateBlips()
{
	// draw the raider blips
	for (int i=0; i<2; i++){
		RAIDER *sRaiderP = i ? &sRaiderB : &sRaiderA;
		if (sRaiderP->bAttacking){
			if ((sRaiderP->nColumn >= 0) && (sRaiderP->nColumn < SPACEALERT_BLIP_COLUMNS)
				&& (sRaiderP->nRow >= 0) && (sRaiderP->nRow < SPACEALERT_BLIP_ROWS)){
				Blips[sRaiderP->nColumn][sRaiderP->nRow] = BLIP_DIM;
			}
		}
	}

	// draw the player missile blip
	if ((nCurrentMissileRow <= (SPACEALERT_BLIP_ROWS - 1))
		&& (nCurrentMissileRow >= 0)){
		Blips[nColumnSelector][nCurrentMissileRow] = BLIP_BRIGHT;
	}

}

void PaintGame()
{
	Platform_StartDraw();

	// draw the blips field
	for (int y = 0; y < SPACEALERT_BLIP_ROWS; y++){
		for (int x = 0; x < SPACEALERT_BLIP_COLUMNS; x++){
			SpaceAlert_DrawBlip(Blips[x][y], x, y);
		}
	}

	// draw the score
	if (bPower){
		SpaceAlert_DrawScore(nPoints);
	}

	Platform_EndDraw();
}


void InitAttack()
{
	sRaiderA.nRow = -2;
	sRaiderA.nSlow = Platform_Random(3);
	sRaiderA.nColumn = Platform_Random(3);
	sRaiderA.bAttacking = TRUE;
	sRaiderA.bLeftColumn = FALSE;
	sRaiderA.bMidColumn = FALSE;
	sRaiderA.bRightColumn = FALSE;

	if ((Platform_Random(2) == 0) && (nRaiderCount > 1)){

		// dual attack

		if (Platform_Random(3) == 0){
			// stagger the attack
			sRaiderB.nRow = -(Platform_Random(5) + 2);
			sRaiderB.nSlow = 1;
		} else {
			// attack in parallel
			sRaiderB.nRow = -2;
			sRaiderB.nSlow = Platform_Random(3);
		}

		// make sure the raiders don't start on the same column
		do {
			sRaiderB.nColumn = Platform_Random(3);
		} while (sRaiderB.nColumn == sRaiderA.nColumn);

		sRaiderB.bAttacking = TRUE;
		sRaiderB.bLeftColumn = FALSE;
		sRaiderB.bMidColumn = FALSE;
		sRaiderB.bRightColumn = FALSE;
	}
}

