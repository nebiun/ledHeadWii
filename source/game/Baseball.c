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

Email : peter@peterhirschberg.comWebsite : http://www.peterhirschberg.com

*/

#include "Baseball.h"
#include "Games.h"

// constants
#define FIRSTBASE	3
#define SECONDBASE	6
#define THIRDBASE	9
#define HOMEPLATE	12


// game variables
static BOOL bHomeTeam;
static BOOL bInFrame = FALSE;
static BOOL bPower;
static BOOL bPro2 = FALSE;

static int nHRuns;
static int nVRuns;
static int nInnings;
static int nOuts;
static int nBalls;
static int nStrikes;

static int nTimerPitchWait;

static int nPitchSpeed;
static BOOL bCurveball;
static int nEraseIndex;
static int nPitchIndex;
static int nPendingRuns;

static int nTimerEndPlayWait;

static int nTimerRunnerMove;
static int nRunnerSpeed;
static int nTimerFielding;

static int nCurrentRunnerIndex;

static int nDefenseBlip;

static BOOL bCaught;
static int fireWorks;

// finite state machine stuff

static void fsmIdle(int tu);
static void fsmPitchWait(int tu);
static void fsmPitching(int tu);
static void fsmEndPlayWait(int tu);
static void fsmRun(int tu);
static void fsmWalk(int tu);
static void fsmOut(int tu);
static void fsmHomeRun(int tu);
static void fsmEndPossession(int tu);
static void fsmGameOver(int tu);

static enum FSM {
	FSM_IDLE,
	FSM_PITCHWAIT,
	FSM_PITCHING,
	FSM_ENDPLAYWAIT,
	FSM_RUN,
	FSM_WALK,
	FSM_OUT,
	FSM_HOMERUN,
	FSM_ENDPOSSESSION,
	FSM_GAMEOVER
}fsm;

typedef void (*FSMFCN)(int);

static FSMFCN fsmfcn[] = {
	fsmIdle,
	fsmPitchWait,
	fsmPitching,
	fsmEndPlayWait,
	fsmRun,
	fsmWalk,
	fsmOut,
	fsmHomeRun,
	fsmEndPossession,
	fsmGameOver
};

typedef struct RUNNER
{
	int baseindex;
	BOOL enabled;
}RUNNER;

#define MAX_RUNNERS		4
RUNNER runners[MAX_RUNNERS];


// proto's
static void InitGame();
static void ClearAllDisplay();
static void RestoreAllDisplay();
static void InsertRunner();
static BOOL MoveRunners();
static void ResetOffBaseRunners();
static void DrawDefenseBlip(int index);
static void DrawBaseBlips(BOOL bState);
static void ErasePitchBlips();
static void HitBall(int nPosition);
static void DoOut();

BOOL Baseball_GetPower()
{
	return (bPower ? TRUE : FALSE);
}

void Baseball_PowerOn()
{
	InitGame();
	bPower = TRUE;
}

void Baseball_PowerOff()
{
	bPower = FALSE;
	Baseball_StopSound();
}

void Baseball_SetSkill(int i)
{
	if (i == 0)
	{
		bPro2 = FALSE;
	} 
	else {
		bPro2 = TRUE;
	}
}

int Baseball_GetSkill()
{
	return bPro2 ? 1 : 0;
}

void InitGame()
{
	bHomeTeam = FALSE;
	PlatformSetInput(bHomeTeam);
	
	nHRuns = 0;
	nVRuns = 0;
	nInnings = 0;
	nStrikes = 0;
	nOuts = 0;
	nBalls = 0;

	for (int i=0; i<MAX_RUNNERS; i++)
	{
		runners[i].enabled = FALSE;
	}

	fsm = FSM_IDLE;
}

void Baseball_Run(int tu)
{
	// prevent reentrancy
	if (bInFrame){ return; }
	bInFrame = TRUE;

	if (!bPower)
	{
		// power is off
		Platform_StartDraw();
		Baseball_DrawScore(-1, -1);
		Platform_EndDraw();
		bInFrame = FALSE;
		return;
	}

	Platform_StartDraw();

	// run the game
	(fsmfcn[fsm])(tu);

	Platform_EndDraw();

	bInFrame = FALSE;

}

static void ErasePitchBlips()
{
	for (int i=0; i<=8; i++)
	{
		// erase both possible sets of pitch blips
		Baseball_DrawPitchBlip(FALSE, i, FALSE);
		Baseball_DrawPitchBlip(FALSE, i, TRUE);
	}
}

static void ClearAllDisplay()
{
	Baseball_DrawScore(-1, -1);
	ErasePitchBlips();
	DrawBaseBlips(FALSE);
	DrawDefenseBlip(-1);
}

static void RestoreAllDisplay()
{
	Baseball_DrawStats(nInnings+1, nOuts, nBalls, nStrikes, bHomeTeam);
	if (nEraseIndex > 0)
	{
		Baseball_DrawPitchBlip(TRUE, nEraseIndex, bCurveball);
	}
	DrawBaseBlips(TRUE);
	if (nDefenseBlip != -1)
	{
		DrawDefenseBlip(nDefenseBlip);
	}
}

static void DrawDefenseBlip(int index)
{
	Baseball_DrawDeepBlip((index == 0) ? TRUE:FALSE, 0);
	Baseball_DrawOutfieldBlip((index == 1) ? TRUE:FALSE, 0);
	Baseball_DrawOutfieldBlip((index == 2) ? TRUE:FALSE, 1);
	Baseball_DrawOutfieldBlip((index == 3) ? TRUE:FALSE, 2);
	Baseball_DrawDeepBlip((index == 4) ? TRUE:FALSE, 1);
}

static void DrawBaseBlips(BOOL bState)
{
	if (bState)
	{
		for (int i=0; i<MAX_RUNNERS; i++)
		{
			if (runners[i].enabled)
			{
				Baseball_DrawBaseBlip(TRUE, runners[i].baseindex);
			}
		}
	}
	else
	{
		for (int i=0; i<=HOMEPLATE; i++)
		{
			Baseball_DrawBaseBlip(FALSE, i);
		}
	}
}

static void InsertRunner()
{
	for (int i=0; i<MAX_RUNNERS; i++)
	{
		if (!runners[i].enabled)
		{
			runners[i].baseindex=0;
			runners[i].enabled=TRUE;
			nCurrentRunnerIndex = i;
			return;
		}
	}
}

// returns TRUE if move resulted in a run
static BOOL MoveRunners()
{
	BOOL bRun = FALSE;

	for (int i=0; i<MAX_RUNNERS; i++)
	{
		if (runners[i].enabled)
		{
			if ((runners[i].baseindex >= 0) && (runners[i].baseindex < HOMEPLATE))
			{
				// erase previous blip position
				Baseball_DrawBaseBlip(FALSE, runners[i].baseindex);
			}
			runners[i].baseindex++;
			if (runners[i].baseindex >= HOMEPLATE)
			{
				// got a run
				runners[i].baseindex=-1;
				runners[i].enabled=FALSE;
				if (i == nCurrentRunnerIndex)
				{
					nCurrentRunnerIndex = -1;
				}
				bRun=TRUE;
			}
		}
	}

	return bRun;
}

static void ResetOffBaseRunners()
{
	BOOL bOffBase = FALSE;
	for (int i=0; i<MAX_RUNNERS; i++)
	{
		if (runners[i].enabled)
		{
			if ((runners[i].baseindex != FIRSTBASE)
				&& (runners[i].baseindex != SECONDBASE)
				&& (runners[i].baseindex != THIRDBASE)
				&& (runners[i].baseindex != HOMEPLATE))
			{
				// yes, there is at least one runner and he is off-base
				bOffBase = TRUE;
				break;
			}
		}
	}
	if (bOffBase)
	{
		//
		// The ball has now been fielded and there are
		// currently runners off-base -- figure out who
		// is out.
		//
		// If the batter did not make it to first, he is out and
		// the remaining runners all move back to the previous base.
		// If the batter made it past first, the lead runner is out
		// and the remaining runners move either to the next base or
		// to the previous base, depending on how far they made it.
		//

		// see if the batter made it to first
		BOOL bFirst=TRUE;
		int i;
		for (i=0; i<MAX_RUNNERS; i++)
		{
			if (runners[i].enabled)
			{
				if (runners[i].baseindex < FIRSTBASE)
				{
					// the batter didn't make it to first - he's out
					runners[i].enabled = FALSE;
					bFirst = FALSE;
					break;
				}
			}
		}
		if (bFirst)
		{
			// look for the lead runner
			int nRunnerOut = -1;
			for (i=0; i<MAX_RUNNERS; i++)
			{
				if (runners[i].enabled)
				{
					if (nRunnerOut == -1)
					{
						nRunnerOut = i;
					}
					else
					{
						if (runners[i].baseindex > runners[nRunnerOut].baseindex)
						{
							nRunnerOut = i;
						}
					}
				}
			}
			// mark the lead runner out
			if (nRunnerOut != -1)
			{
				runners[nRunnerOut].enabled = FALSE;
			}

			// figure out if the remaining runners should go to the previous or next base
			for (i=0; i<MAX_RUNNERS; i++)
			{
				if (runners[i].enabled)
				{
					int howfar = (runners[i].baseindex) % 3;
					if (howfar <= 1)
					{
						// go back
						runners[i].baseindex--;
					}
					else
					{
						// go forward
						runners[i].baseindex++;
					}
				}
			}

		}
		else
		{
			// the batter is out
			for (i=0; i<MAX_RUNNERS; i++)
			{
				if (runners[i].enabled)
				{
					if (runners[i].baseindex < FIRSTBASE)
					{
						runners[i].enabled = FALSE;
						break;
					}
				}
			}
			// move the remaining runners back to the previous base
			for (i=0; i<MAX_RUNNERS; i++)
			{
				if (runners[i].enabled)
				{
					runners[i].baseindex -= (runners[i].baseindex) % 3;
				}
			}
		}
	}
}

static void DoOut()
{
	// clear the screen
	ClearAllDisplay();

	Baseball_PlaySound(BASEBALL_SOUND_OUT, PLAYSOUNDFLAGS_ASYNC|PLAYSOUNDFLAGS_PRIORITY);

	Platform_Pause(500);
	fsm = FSM_OUT;
}

static void HitBall(int nPosition)
{
	// I am not positive of the logic for this, so I'm
	// localizing it in this function for clarity and
	// to make it easier to tweak until it seems right.
	//
	// There are 3 positions where you can hit the ball:
	//  - right in front of the plate
	//  - directly over the plate
	//  - right behind the plate
	//
	// In addition, each position is sampled 3 times:
	//  - early
	//  - dead on
	//  - late
	//
	// Finally, there are 5 catchers that can field the ball:
	//  - left and right infield
	//  - left, center and right outfield
	// 
	// nPosition represents a value from 0 to 8, with 0
	// representing the earliest possible swing, 8 being
	// the latest possible swing, and 4 being dead on.
	//
	//                  |...^...|
	//                  012345678
	//
	// 0 will be fielded by left infield
	//  1 will be fielded by either left infield or left outfield
	//   2 will be fielded by left outfield
	//    3 will be fielded by either left or center outfield
	//     4 will be a home run or fielded by center outfield
	//    5 will be fielded by either right or center outfield
	//   6 will be fielded by right outfield
	//  7 will be fielded by either right infield or right outfield
	// 8 will be fielded by right infield
	//
	// The ball will either be fielded normally or it will be
	// a fly out and will be counted as an out. If the ball is
	// fielded normally, we pick a random fielding time (1,2 or 3
	// beeps), and a random runner speed.

	nDefenseBlip = -1;

	BOOL bOut = (Platform_Random(4) == 0) ? TRUE:FALSE;
	if (bOut)
	{
		bCaught = TRUE;

		// pop fly - pick a fielder
		switch (nPosition)
		{
		case 0:
			// left infield
			nDefenseBlip = 0;
			break;
		case 1:
			// either left infield or left outfield
			nDefenseBlip = Platform_Random(2) ? 0:1;
			break;
		case 2:
			// left outfield
			nDefenseBlip = 1;
			break;
		case 3:
			// either left or center outfield
			nDefenseBlip = Platform_Random(2) ? 1:2;
			break;
		default:
		case 4:
			// center outfield
			nDefenseBlip = 2;
			break;
		case 5:
			// either right or center outfield
			nDefenseBlip = Platform_Random(2) ? 2:3;
			break;
		case 6:
			// right outfield
			nDefenseBlip = 3;
			break;
		case 7:
			// either right infield or right outfield
			nDefenseBlip = Platform_Random(2) ? 3:4;
			break;
		case 8:
			// right infield
			nDefenseBlip = 4;
			break;
		}

		DoOut();
		return;
	}
	else
	{
		BOOL bHomeRun = FALSE;
		switch (nPosition)
		{
		case 0:
			// left infield
			nDefenseBlip = 0;
			break;
		case 1:
			// either left infield or left outfield
			nDefenseBlip = Platform_Random(2) ? 0:1;
			break;
		case 2:
			// left outfield
			nDefenseBlip = 1;
			break;
		case 3:
			// either left or center outfield
			nDefenseBlip = Platform_Random(2) ? 1:2;
			break;
		default:
		case 4:
			// center outfield (or home run)
			if (Platform_Random(4)==0)
			{
				bHomeRun = TRUE;
			}
			else
			{
				nDefenseBlip = 2;
			}
			break;
		case 5:
			// either right or center outfield
			nDefenseBlip = Platform_Random(2) ? 2:3;
			break;
		case 6:
			// right outfield
			nDefenseBlip = 3;
			break;
		case 7:
			// either right infield or right outfield
			nDefenseBlip = Platform_Random(2) ? 3:4;
			break;
		case 8:
			// right infield
			nDefenseBlip = 4;
			break;
		}

		if (bHomeRun)
		{

			fireWorks = 1;
			fsm = FSM_HOMERUN;
		}
		else
		{
			// light up the defense player that is fielding the ball
			DrawDefenseBlip(nDefenseBlip);

			// pick a random fielding speed and play
			// the correct number of base tones
			int bases = Platform_Random(3)+1;
			nTimerFielding = (50 * bases); // emperically derived magic number
			for (int i=0; i<bases; i++)
			{
				Baseball_PlaySound(BASEBALL_SOUND_HIT, PLAYSOUNDFLAGS_ASYNC|PLAYSOUNDFLAGS_PRIORITY);
				Platform_Pause(320);
			}

			// pick a random runner speed
			nTimerRunnerMove = nRunnerSpeed = 10 + Platform_Random(10);

			// insert a runner
			InsertRunner();

			fsm = FSM_RUN;
		}
	}
}

static void fsmOut(int tu)
{
	++nOuts;
	if (nOuts == 3)
	{
		// end of possession

		// but first play any pending run sounds
		while(nPendingRuns > 0)
		{
			Baseball_PlaySound(BASEBALL_SOUND_RUN, PLAYSOUNDFLAGS_ASYNC|PLAYSOUNDFLAGS_PRIORITY);
			Platform_Pause(600);
			--nPendingRuns;
		}

		nOuts = 0;
		bHomeTeam = !bHomeTeam;
		if (!bHomeTeam)
		{
			++nInnings;
			if (nInnings == 5)
			{
				// ************************
				// ****** GAME OVER *******
				// ************************
				Baseball_PlaySound(BASEBALL_SOUND_ENDGAME, PLAYSOUNDFLAGS_ASYNC|PLAYSOUNDFLAGS_PRIORITY);
				Platform_Pause(200);
				fsm = FSM_GAMEOVER;
				return;
			}
		}
		PlatformSetInput(bHomeTeam);
		Baseball_PlaySound(BASEBALL_SOUND_ENDPOSSESSION, PLAYSOUNDFLAGS_PRIORITY);	
		RestoreAllDisplay();
		Baseball_DrawStats(-1, -1, nBalls, nStrikes, bHomeTeam);
		fsm = FSM_ENDPOSSESSION;
	}
	else
	{
		RestoreAllDisplay();
		Baseball_DrawStats(-1, -1, nBalls, nStrikes, bHomeTeam);
		nTimerEndPlayWait = (nDefenseBlip == -1) ? (1500/50) : (3000/50);
		fsm = FSM_ENDPLAYWAIT;
	}
	nStrikes = 0;
	nBalls = 0;
}


// FINITE STATE MACHINE STUFF

static void fsmIdle(int tu)
{
	// play the run sounds
	while(nPendingRuns > 0)
	{
		Baseball_PlaySound(BASEBALL_SOUND_RUN, PLAYSOUNDFLAGS_ASYNC|PLAYSOUNDFLAGS_PRIORITY);
		Platform_Pause(600);
		--nPendingRuns;
	}

	nCurrentRunnerIndex = -1;
	nDefenseBlip = -1;
	bCaught = FALSE;

	if (Baseball_GetInputSCORE(NULL))
	{
		Baseball_DrawScore(nVRuns, nHRuns);
	}
	else
	{
		Baseball_DrawStats(nInnings+1, nOuts, nBalls, nStrikes, bHomeTeam);
	}

	// wait for pitch button
	if (Baseball_GetInputPITCH(NULL))
	{
		// delay from 1.5 to 2.5 seconds (according to manual)
		// before pitching the ball

		// access of game context info from here is probably not too cool
		nTimerPitchWait = (Platform_Random(1000) + 1500) / gGameContext[GAME_BASEBALL].tu;
		fsm = FSM_PITCHWAIT;
	}
	Baseball_DrawBaseBlip(FALSE, 0);
	DrawBaseBlips(TRUE);
}

static void fsmPitchWait(int tu)
{
	Baseball_DrawScore(-1, -1);

	if (nTimerPitchWait > 0)
	{
		--nTimerPitchWait;
	}

	if (nTimerPitchWait <= 0)
	{
		// pick a ball speed and throw style
		// and pitch the ball
		bCurveball = (Platform_Random(3) == 0) ? TRUE : FALSE;
		if (bCurveball)
		{
			nPitchSpeed = Platform_Random(2)+1; // curve balls are a little slower
		}
		else
		{
			nPitchSpeed = Platform_Random(3);
		}

#ifdef __PALMOS__
		nPitchSpeed += 7; // PALMOS ONLY
#endif
		
		nPitchIndex = -1;
		fsm = FSM_PITCHING;
	}
}

static void fsmPitching(int tu)
{
	BOOL bStrike = FALSE;
	static int nswing;	// swing early=0, center=1, late=2
	int nMissChance = 5;

	if(nPitchIndex == -1) {
		Baseball_DrawScore(-1, -1);
		nEraseIndex = -1;
		nPitchIndex = 0;
		nswing = 0;
	}
	
	if(nPitchIndex <= 8)
	{
		if (nEraseIndex != -1)
		{
			Baseball_DrawPitchBlip(FALSE, nEraseIndex, bCurveball);
		}

		Baseball_DrawPitchBlip(TRUE, nPitchIndex, bCurveball);
		nEraseIndex = nPitchIndex;

		// check for swing at 3 different points
		// we really only need to check 3 times
		// when the ball is in the strike zone, but
		// we'll do it anyway to keep the timing consistant

		// see HitBall() function for detailed description
		// of what it does and what the parameter means

		if (Baseball_GetInputHIT(NULL))
		{
			// swing early
			switch (nPitchIndex)
			{
			case 5:
				if (Platform_Random(nMissChance)==0)
				{
					// missed
					bStrike = TRUE;
					break;
				}
				else
				{
					HitBall(0+nswing);
					return;
				}
			case 6:
				if (Platform_Random(nMissChance)==0)
				{
					// missed
					bStrike = TRUE;
					break;
				}
				else
				{
					HitBall(3+nswing);
					return;
				}
			case 7:
				if (Platform_Random(nMissChance)==0)
				{
					// missed
					bStrike = TRUE;
					break;
				}
				else
				{
					HitBall(6+nswing);
					return;
				}
			default:
				// strike
				bStrike = TRUE;
				break;
			}
		}
		
		if (!bStrike) {
			int	nDelay = ((nPitchSpeed * 4) / 3) - tu;
			
			if(nDelay > 0)
				Platform_Pause(nDelay);
			nswing++;
			if(nswing == 3) {
				nPitchIndex++;
				nswing = 0;
			}
		}
		else {
			nPitchIndex = 9;
		}
	}
	
	if(nPitchIndex >= 9) {
		Platform_Pause(200);

		// if player didn't swing, randomly pick strike or ball
		if (Platform_Random(2) && !bStrike)
		{
			++nBalls;
		}
		else
		{
			++nStrikes;
		}

		if (nBalls == 4)
		{
			// walk
			Baseball_DrawStats(-1, -1, nBalls, nStrikes, bHomeTeam);
			Baseball_PlaySound(BASEBALL_SOUND_STRIKE, PLAYSOUNDFLAGS_PRIORITY);
			nTimerEndPlayWait = 1500/50;
			nTimerRunnerMove = nRunnerSpeed = 10; // walk is always a set speed
			InsertRunner();
			fsm = FSM_WALK;
		}
		else if (nStrikes == 3)
		{
			// struck out
			DoOut();
		}
		else
		{
			Baseball_DrawStats(-1, -1, nBalls, nStrikes, bHomeTeam);
			Baseball_PlaySound(BASEBALL_SOUND_STRIKE, PLAYSOUNDFLAGS_PRIORITY);
			nTimerEndPlayWait = 1500/50;
			fsm = FSM_ENDPLAYWAIT;
		}
	}
}

static void fsmEndPlayWait(int tu)
{
	if (nTimerEndPlayWait > 0)
	{
		--nTimerEndPlayWait;
		if (nDefenseBlip != -1)
		{
			// blink fielder if one caught a ball
			if (bCaught)
			{
				static BOOL blink=FALSE;
				static int blinkcnt=0;
				--blinkcnt;
				if (blinkcnt<0)
				{
					blinkcnt=2;
					blink=!blink;
				}
				if (blink)
				{
					DrawDefenseBlip(nDefenseBlip);
				}
				else
				{
					DrawDefenseBlip(-1);
				}
			}
			else
			{
				DrawDefenseBlip(nDefenseBlip);
			}
		}
	}
	else
	{
		// erase the old blips
		Baseball_DrawPitchBlip(FALSE, nEraseIndex, bCurveball);
		DrawDefenseBlip(-1);
		ErasePitchBlips();

		// reset the runners
		ResetOffBaseRunners();
		DrawBaseBlips(FALSE);

		fsm = FSM_IDLE;
	}
}

static void fsmRun(int tu)
{
	DrawBaseBlips(TRUE);

	// RUN does not take effect until you release HIT
	if (Baseball_GetInputRUN(NULL) && !Baseball_GetInputHIT(NULL))
	{
		if (nTimerRunnerMove > 0)
		{
			--nTimerRunnerMove;
		}
		if (nTimerRunnerMove == 0)
		{
			nTimerRunnerMove = nRunnerSpeed;
			if (MoveRunners())
			{
				// record runs
				if (bHomeTeam)
				{
					++nHRuns;
				}
				else
				{
					++nVRuns;
				}
				// also note the run for later (for the sounds)
				++nPendingRuns;
			}
		}
	}

	if (nTimerFielding > 0)
	{
		--nTimerFielding;
	}
	if (nTimerFielding == 0)
	{
		// ball is finished being fielded

		// erase the fielder's blip
//		nDefenseBlip = -1;
//		DrawDefenseBlip(-1);

		// ball has been fielded - see if any runners are off base
		BOOL bOffBase = FALSE;
		for (int i=0; i<MAX_RUNNERS; i++)
		{
			if (runners[i].enabled)
			{
				if ((runners[i].baseindex != FIRSTBASE)
					&& (runners[i].baseindex != SECONDBASE)
					&& (runners[i].baseindex != THIRDBASE)
					&& (runners[i].baseindex != HOMEPLATE))
				{
					bOffBase = TRUE;
					break;
				}
			}
		}
		if (bOffBase)
		{
			// somebody is out -- go through the 'out' theatrics
			DoOut();
		}
		else
		{
			// all runners are safe
			Baseball_DrawStats(-1, -1, nBalls, nStrikes, bHomeTeam);
			nBalls=0;
			nStrikes=0;
			nTimerEndPlayWait = 1500/50;
			fsm = FSM_ENDPLAYWAIT;
		}
	}
}

static void fsmWalk(int tu)
{
	DrawBaseBlips(TRUE);

	if (Baseball_GetInputRUN(NULL))
	{
		if (nTimerRunnerMove > 0)
		{
			--nTimerRunnerMove;
		}
		if (nTimerRunnerMove == 0)
		{
			nTimerRunnerMove = nRunnerSpeed;
			if (MoveRunners())
			{
				// record runs
				if (bHomeTeam)
				{
					++nHRuns;
				}
				else
				{
					++nVRuns;
				}
				// also note the run for later (for the sounds)
				++nPendingRuns;
			}

			// when the current runner gets on first, we're done
			if (runners[nCurrentRunnerIndex].baseindex == FIRSTBASE)
			{
				// got someone on first - all done
				nStrikes = 0;
				nBalls = 0;
				Baseball_DrawPitchBlip(FALSE, 8, FALSE);
				DrawDefenseBlip(-1);
				ErasePitchBlips();
				fsm = FSM_IDLE;
			}
		}
	}
}

static void fsmHomeRun(int tu)
{
	static int loop = 0;
	
	trace = fireWorks;
	if(fireWorks > 0) {
		//
		// do fireworks display
		//
		Platform_Pause(40);
		ClearAllDisplay();		
		
		switch(fireWorks) {
		case 1:	// base and fielder blips
			Baseball_PlaySound(BASEBALL_SOUND_RUN, PLAYSOUNDFLAGS_PRIORITY|PLAYSOUNDFLAGS_ASYNC);
			Baseball_DrawBaseBlip(TRUE,1);
			break;
		case 2:
			Baseball_DrawBaseBlip(TRUE,1);
			Baseball_DrawBaseBlip(TRUE,2);
			break;
		case 3:
			Baseball_DrawBaseBlip(TRUE,1);
			Baseball_DrawBaseBlip(TRUE,2);
			Baseball_DrawDeepBlip(TRUE,0);
			break;
		case 4:
			Baseball_DrawBaseBlip(TRUE,2);
			Baseball_DrawDeepBlip(TRUE,0);
			Baseball_DrawBaseBlip(TRUE,4);
			break;
		case 5:
			Baseball_DrawDeepBlip(TRUE,0);
			Baseball_DrawBaseBlip(TRUE,4);
			Baseball_DrawBaseBlip(TRUE,5);
			break;
		case 6:
			Baseball_DrawBaseBlip(TRUE,4);
			Baseball_DrawBaseBlip(TRUE,5);
			Baseball_DrawDeepBlip(TRUE,1);
			break;		
		case 7:
			Baseball_DrawBaseBlip(TRUE,5);
			Baseball_DrawDeepBlip(TRUE,1);
			Baseball_DrawBaseBlip(TRUE,8);
			break;
		case 8:
			Baseball_DrawDeepBlip(TRUE,1);
			Baseball_DrawBaseBlip(TRUE,8);	
			Baseball_DrawBaseBlip(TRUE,7);
			break;			
		case 9:
			Baseball_DrawBaseBlip(TRUE,8);	
			Baseball_DrawBaseBlip(TRUE,7);
			Baseball_DrawOutfieldBlip(TRUE,0);
			break;		
		case 10:	
			Baseball_DrawBaseBlip(TRUE,7);
			Baseball_DrawOutfieldBlip(TRUE,0);
			Baseball_DrawOutfieldBlip(TRUE,1);
			break;		
		case 11:	
			Baseball_DrawOutfieldBlip(TRUE,0);
			Baseball_DrawOutfieldBlip(TRUE,1);
			Baseball_DrawFireWorks();
			break;
		case 12:
			Baseball_DrawOutfieldBlip(TRUE,1);
			Baseball_DrawFireWorks();
			Baseball_DrawBaseBlip(TRUE,10);
			break;
		case 13:
			Baseball_DrawFireWorks();
			Baseball_DrawBaseBlip(TRUE,10);
			Baseball_DrawBaseBlip(TRUE,11);
			break;
		case 14:
			Baseball_DrawBaseBlip(TRUE,10);
			Baseball_DrawBaseBlip(TRUE,11);
			Baseball_DrawPitchBlip(TRUE, 8, FALSE);
			break;
		case 15:
			Baseball_DrawPitchBlip(TRUE, 8, FALSE);
			Baseball_DrawPitchBlip(TRUE, 7, FALSE);
			break;
		case 16:
			Baseball_DrawPitchBlip(TRUE, 8, FALSE);
			Baseball_DrawPitchBlip(TRUE, 7, FALSE);
			Baseball_DrawPitchBlip(TRUE, 6, FALSE);
			break;
		case 17:
			Baseball_DrawPitchBlip(TRUE, 7, FALSE);
			Baseball_DrawPitchBlip(TRUE, 6, FALSE);
			Baseball_DrawPitchBlip(TRUE, 5, FALSE);
			break;
		case 18:
			Baseball_DrawPitchBlip(TRUE, 6, FALSE);
			Baseball_DrawPitchBlip(TRUE, 5, FALSE);
			Baseball_DrawPitchBlip(TRUE, 4, TRUE);
			break;
		case 19:
			Baseball_DrawPitchBlip(TRUE, 5, FALSE);
			Baseball_DrawPitchBlip(TRUE, 3, TRUE);
			Baseball_DrawPitchBlip(TRUE, 2, TRUE);
			break;
		case 20:
			Baseball_DrawPitchBlip(TRUE, 3, TRUE);
			Baseball_DrawPitchBlip(TRUE, 2, TRUE);
			Baseball_DrawPitchBlip(TRUE, 1, TRUE);
			break;
		case 21:
			Baseball_DrawPitchBlip(TRUE, 2, TRUE);
			Baseball_DrawPitchBlip(TRUE, 1, TRUE);
			Baseball_DrawPitchBlip(TRUE, 1, FALSE);
			break;		
		case 22:
			Baseball_DrawPitchBlip(TRUE, 1, TRUE);
			Baseball_DrawPitchBlip(TRUE, 1, FALSE);
			Baseball_DrawPitchBlip(TRUE, 2, FALSE);
			break;	
		case 23:
			Baseball_DrawPitchBlip(TRUE, 1, FALSE);
			Baseball_DrawPitchBlip(TRUE, 2, FALSE);
			Baseball_DrawPitchBlip(TRUE, 3, FALSE);
			break;
		case 24:
			Baseball_DrawPitchBlip(TRUE, 2, FALSE);
			Baseball_DrawPitchBlip(TRUE, 3, FALSE);
			Baseball_DrawPitchBlip(TRUE, 4, FALSE);
			break;
		case 25:
			Baseball_DrawPitchBlip(TRUE, 3, FALSE);
			Baseball_DrawPitchBlip(TRUE, 4, FALSE);
			break;								
		case 26:
			break;
		}
		
		fireWorks++;
		if(fireWorks > 26) {
			loop++;
			if(loop == 4) {
				loop = 0;
				fireWorks = 0;
				// insert a runner
				InsertRunner();
				nTimerRunnerMove = nRunnerSpeed = 10; // fixed run speed
			}
			else {
				fireWorks = 1;
			}
		}
		return;
	}

	DrawBaseBlips(TRUE);

	Baseball_DrawBaseBlip(TRUE, 0);

	if (Baseball_GetInputRUN(NULL))
	{
		if (nTimerRunnerMove > 0)
		{
			--nTimerRunnerMove;
		}
		if (nTimerRunnerMove == 0)
		{
			nTimerRunnerMove = nRunnerSpeed;
			if (MoveRunners())
			{
				// record runs
				if (bHomeTeam)
				{
					++nHRuns;
				}
				else
				{
					++nVRuns;
				}
				// also note the run for later (for the sounds)
				++nPendingRuns;

				// see if anyone is still running the bases
				BOOL bManOnBase = FALSE;
				for (int i=0; i<MAX_RUNNERS; i++)
				{
					if (runners[i].enabled)
					{
						bManOnBase = TRUE;
						break;
					}
				}

				if (!bManOnBase)
				{
					// nobody on base - all done
					nStrikes = 0;
					nBalls = 0;
					Baseball_DrawPitchBlip(FALSE, 8, FALSE);
					DrawDefenseBlip(-1);
					ErasePitchBlips();
					fsm = FSM_IDLE;
				}
			}
		}
	}
}

static void fsmEndPossession(int tu)
{
	// wait for SCORE key
	if (Baseball_GetInputSCORE(NULL))
	{
		nOuts = 0;
		nStrikes = 0;
		nBalls = 0;

		nEraseIndex = -1;

		// remove all runners
		for (int i=0; i<MAX_RUNNERS; i++)
		{
			runners[i].enabled = FALSE;
		}

		DrawBaseBlips(FALSE);
		Baseball_DrawStats(nInnings+1, nOuts, nBalls, nStrikes, bHomeTeam);
		DrawDefenseBlip(-1);
		ErasePitchBlips();

		fsm = FSM_IDLE;
	}
	else
	{
		if (nDefenseBlip != -1)
		{
			// blink fielder if one caught a ball
			if (bCaught)
			{
				static BOOL blink=FALSE;
				static int blinkcnt=0;
				--blinkcnt;
				if (blinkcnt<0)
				{
					blinkcnt=2;
					blink=!blink;
				}
				if (blink)
				{
					DrawDefenseBlip(nDefenseBlip);
				}
				else
				{
					DrawDefenseBlip(-1);
				}
			}
			else
			{
				DrawDefenseBlip(nDefenseBlip);
			}
		}
	}
}

static void fsmGameOver(int tu)
{
	Baseball_DrawScore(nVRuns, nHRuns);
}
