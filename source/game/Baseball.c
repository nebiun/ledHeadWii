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

#include <string.h>
#include <sys/time.h>
#include "Baseball.h"
#include "Games.h"

// constants
#define FIRSTBASE	3
#define SECONDBASE	6
#define THIRDBASE	9
#define HOMEPLATE	12

#define MAGIC_NUMBER	64		// emperically derived magic number
#define SPEED_BASE		(1 + (MAGIC_NUMBER/16)) 

#define CENTER_FIELD	0
#define LEFT_FIELD		1
#define RIGHT_FIELD		2

// game variables
static BOOL bHomeTeam;
static BOOL bInFrame = FALSE;

static int nHRuns;
static int nVRuns;
static int nInnings;
static int nOuts;
static int nBalls;
static int nStrikes;

static int nTimerPitchWait;

static int nPitchSpeed;
static BOOL bCurveball;
static BOOL bStrike = FALSE;
static int nPitchIndex;
static int nPendingRuns;

static int nTimerEndPlayWait;

static int nTimerRunnerMove;
static int nRunnerSpeed;
static int nTimerFielding;
static int nCurrentRunnerIndex;

static int nDefenseBlip;
static int nBallBlip;

static BOOL bCaught;
static BOOL bDisplayOff;
static int fireWorks;
static int nSwingSlot;

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
static void fsmGoOut(int tu);

enum FSM {
	FSM_IDLE,
	FSM_PITCHWAIT,
	FSM_PITCHING,
	FSM_ENDPLAYWAIT,
	FSM_RUN,
	FSM_WALK,
	FSM_OUT,
	FSM_HOMERUN,
	FSM_ENDPOSSESSION,
	FSM_GAMEOVER,
	FSM_GOOUT
};
static enum FSM fsm;

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
	fsmGameOver,
	fsmGoOut
};

typedef struct
{
	int baseindex;
	BOOL enabled;
} RUNNER;

#define MAX_RUNNERS		4
RUNNER runners[MAX_RUNNERS];

typedef struct {
	int seq[3];
	int n;
} fireworks_t;

static fireworks_t fw_frames[] = {
	{ {DIAMOND(1)}, 1},
	{ {DIAMOND(1), DIAMOND(2)}, 2},
	{ {DIAMOND(1), DIAMOND(2), DEEP_1ST}, 3},
	{ {DIAMOND(2), DEEP_1ST, DIAMOND(4)}, 3},
	{ {DEEP_1ST, DIAMOND(4), DIAMOND(5)}, 3},
	{ {DIAMOND(4), DIAMOND(5), DEEP_3RD}, 3},
	{ {DIAMOND(5), DEEP_3RD, DIAMOND(8)}, 3},
	{ {DEEP_3RD, DIAMOND(8), DIAMOND(7)}, 3},
	{ {DIAMOND(8), DIAMOND(7), OUT_LEFT}, 3},
	{ {DIAMOND(7), OUT_LEFT, OUT_CENTER}, 3},
	{ {OUT_LEFT, OUT_CENTER, -1}, 3},
	{ {OUT_CENTER, -1, DIAMOND(10)}, 3},
	{ {-1, DIAMOND(10), DIAMOND(11)}, 3},
	{ {DIAMOND(10), DIAMOND(11), NORMAL_BALL(8)}, 3},
	{ {NORMAL_BALL(8), NORMAL_BALL(7)}, 2},
	{ {NORMAL_BALL(8), NORMAL_BALL(7), NORMAL_BALL(6)}, 3},
	{ {NORMAL_BALL(7), NORMAL_BALL(6), NORMAL_BALL(5)}, 3},
	{ {NORMAL_BALL(6), NORMAL_BALL(5), CURVE_BALL(4)}, 3},
	{ {NORMAL_BALL(5), CURVE_BALL(3), CURVE_BALL(2)}, 3},
	{ {CURVE_BALL(3), CURVE_BALL(2), CURVE_BALL(1)}, 3},
	{ {CURVE_BALL(2), CURVE_BALL(1), NORMAL_BALL(1)}, 3},
	{ {CURVE_BALL(1), NORMAL_BALL(1), NORMAL_BALL(2)}, 3},
	{ {NORMAL_BALL(1), NORMAL_BALL(2), NORMAL_BALL(3)}, 3},
	{ {NORMAL_BALL(2), NORMAL_BALL(3), NORMAL_BALL(4)}, 3},
	{ {NORMAL_BALL(3), NORMAL_BALL(4)}, 2},
	{ {}, 0}
};

static void ClearBlips()
{
	int i;
	
	if(fireWorks) {
		fireworks_t *fw=&fw_frames[fireWorks-1];
		
		for(i=0; i<fw->n; i++) {
			if(fw->seq[i] != -1)
				Baseball_DrawBlip(FALSE,fw->seq[i]);
		}
	}
	else {
		if(nBallBlip != -1) {
			Baseball_DrawBlip(FALSE, nBallBlip);
		}
		for (i=0; i<MAX_RUNNERS; i++) {
			if (runners[i].enabled) {
				Baseball_DrawBlip(FALSE, DIAMOND(runners[i].baseindex));
			}
		}
		if (nDefenseBlip != -1) {
			Baseball_DrawBlip(FALSE, nDefenseBlip);
		}
	}
}

static void DrawBlips()
{
	int i;
	
	if(fireWorks > 0) {
		fireworks_t *fw=&fw_frames[fireWorks-1];

		Baseball_DrawScore(-1, -1);	// Clean fireworks
		for(i=0; i<fw->n; i++) {
			if(fw->seq[i] == -1)
				Baseball_DrawFireWorks();
			else
				Baseball_DrawBlip(TRUE,fw->seq[i]);
		}
	}
	else {
		if(nBallBlip != -1) {
			Baseball_DrawBlip(TRUE, nBallBlip);
		}
		for (i=0; i<MAX_RUNNERS; i++) {
			if (runners[i].enabled) {
				Baseball_DrawBlip(TRUE, DIAMOND(runners[i].baseindex));
			}
		}
		if (nDefenseBlip != -1) {
			if (bCaught) {
				static BOOL blink=FALSE;
				static int blinkcnt=0;
	
				--blinkcnt;
				if (blinkcnt<0) {
					blinkcnt = 2;
					blink = !blink;
				}
				if (blink) {
					Baseball_DrawBlip(TRUE, nDefenseBlip);
				}
			}
			else 
				Baseball_DrawBlip(TRUE, nDefenseBlip);
		}
	}
}

static void InitGame()
{
	bHomeTeam = FALSE;
	PlatformSetInput(bHomeTeam);
	
	nHRuns = 0;
	nVRuns = 0;
	nInnings = 0;
	nStrikes = 0;
	nOuts = 0;
	nBalls = 0;
	nBallBlip = -1;
	nDefenseBlip = -1;
	for (int i=0; i<MAX_RUNNERS; i++) {
		runners[i].enabled = FALSE;
	}

	fsm = FSM_IDLE;
}

void Baseball_Run(int tu)
{
	// prevent reentrancy
	if (bInFrame)
		return;
	
	bInFrame = TRUE;

	if (Baseball_GetPower() == FALSE) {
		InitGame();
		// power is off
		Platform_StartDraw();
		Baseball_DrawScore(-1, -1);
		Platform_EndDraw();
		bInFrame = FALSE;
		return;
	}

	Platform_StartDraw();
	
	ClearBlips();
	
	// run the game
	(fsmfcn[fsm])(tu);

	if(!bDisplayOff) {
		DrawBlips();
	}
	else {
		Baseball_DrawScore(-1, -1);
	}
	
	Platform_EndDraw();

	bInFrame = FALSE;

}

static void InsertRunner()
{
	for (int i=0; i<MAX_RUNNERS; i++) {
		if (!runners[i].enabled) {
			runners[i].baseindex = 0;
			runners[i].enabled = TRUE;
			nCurrentRunnerIndex = i;
			return;
		}
	}
}

// returns TRUE if move resulted in a run
static BOOL MoveRunners()
{
	BOOL bRun = FALSE;

	for (int i=0; i<MAX_RUNNERS; i++) {
		if (runners[i].enabled) {
			runners[i].baseindex++;
			if (runners[i].baseindex >= HOMEPLATE) {
				// got a run
				runners[i].baseindex = -1;
				runners[i].enabled = FALSE;
				if (i == nCurrentRunnerIndex) {
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
	for (int i=0; i<MAX_RUNNERS; i++) {
		if (runners[i].enabled) {
			if ((runners[i].baseindex != FIRSTBASE)
				&& (runners[i].baseindex != SECONDBASE)
				&& (runners[i].baseindex != THIRDBASE)
				&& (runners[i].baseindex != HOMEPLATE)) {
				// yes, there is at least one runner and he is off-base
				bOffBase = TRUE;
				break;
			}
		}
	}
	if (bOffBase) {
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
		for (i=0; i<MAX_RUNNERS; i++) {
			if (runners[i].enabled) {
				if (runners[i].baseindex < FIRSTBASE) {
					// the batter didn't make it to first - he's out
					runners[i].enabled = FALSE;
					bFirst = FALSE;
					break;
				}
			}
		}
		if (bFirst) {
			// look for the lead runner
			int nRunnerOut = -1;
			for (i=0; i<MAX_RUNNERS; i++) {
				if (runners[i].enabled) {
					if (nRunnerOut == -1) {
						nRunnerOut = i;
					}
					else {
						if (runners[i].baseindex > runners[nRunnerOut].baseindex) {
							nRunnerOut = i;
						}
					}
				}
			}
			// mark the lead runner out
			if (nRunnerOut != -1) {
				runners[nRunnerOut].enabled = FALSE;
			}

			// figure out if the remaining runners should go to the previous or next base
			for (i=0; i<MAX_RUNNERS; i++) {
				if (runners[i].enabled) {
					int howfar = (runners[i].baseindex) % 3;
					if (howfar <= 1) {
						// go back
						runners[i].baseindex--;
					}
					else {
						// go forward
						runners[i].baseindex++;
					}
				}
			}
		}
		else {
			// the batter is out
			for (i=0; i<MAX_RUNNERS; i++) {
				if (runners[i].enabled) {
					if (runners[i].baseindex < FIRSTBASE) {
						runners[i].enabled = FALSE;
						break;
					}
				}
			}
			// move the remaining runners back to the previous base
			for (i=0; i<MAX_RUNNERS; i++) {
				if (runners[i].enabled) {
					runners[i].baseindex -= (runners[i].baseindex) % 3;
				}
			}
		}
	}
}

static enum FSM HitBall()
{
	// This routine was totally rewritten by Nebiun
	//
	// There are 3 positions where you can hit the ball:
	//  - right in front of the plate
	//  - directly over the plate
	//  - right behind the plate
	//
	// In addition, each position is sampled more times related 
	// to the ball speed (random and depending from game level) 
	//
	// Finally, there are 5 catchers that can field the ball:
	//  - left and right infield
	//  - left, center and right outfield
	// 
	// The ball will either be fielded normally or it will be
	// a fly out and will be counted as an out. If the ball is
	// fielded normally, we pick a random fielding time (1,2 or 3
	// beeps), and a random runner speed.
	int side, force;
	
	if(nPitchIndex == 5) {
		if(Platform_Random(10) == 0)	// left-handed batter
			side = RIGHT_FIELD;		
		else
			side = LEFT_FIELD;
	}
	else if(nPitchIndex == 7) {
		if(Platform_Random(10) == 0)	// left-handed batter
			side = LEFT_FIELD;		
		else
			side = RIGHT_FIELD;
	}
	else {
		switch(Platform_Random(5)) {
		case 0:
			side = LEFT_FIELD;
			break;
		case 1:
			side = RIGHT_FIELD;
			break;
		default:
			side = CENTER_FIELD;
			break;
		}
	}

	if((nPitchSpeed <= 1) || (nSwingSlot < nPitchSpeed)) {
		force = 1;
	}
	else {
		force = 0;
	}
	
	switch(side) {
	case LEFT_FIELD:
		if((force == 0) || (Platform_Random(5) == 0))
			nDefenseBlip = DEEP_3RD;
		else
			nDefenseBlip = Platform_Random(2) ? DEEP_3RD : OUT_LEFT;
		break;
	case RIGHT_FIELD:
		if((force == 0) || (Platform_Random(5) == 0))
			nDefenseBlip = DEEP_1ST;
		else
			nDefenseBlip = Platform_Random(2) ? DEEP_1ST : OUT_RIGHT;
		break;
	default:
		if((force == 0) || (Platform_Random(5) == 0))
			nDefenseBlip = Platform_Random(2) ? OUT_LEFT : OUT_RIGHT;
		else 
			nDefenseBlip = OUT_CENTER;	
		break;
	}

	// Out ?
	if (Platform_Random(4) == 0) {	
		bCaught = TRUE;
		return FSM_GOOUT;
	}

	// Homerun
	if ((force == 1) && 
	    (((side == CENTER_FIELD) && (Platform_Random(4) == 0)) || (Platform_Random(25) == 0))) {
		nDefenseBlip = -1;
		fireWorks = 1;
		return FSM_HOMERUN;
	}
	
	Baseball_DrawStats(-1, -1, nBalls, nStrikes, bHomeTeam);
	
	// pick a random fielding speed and play
	// the correct number of base tones
	int level = Baseball_GetSkill();
	int bases = Platform_Random(3)+1;
	nTimerFielding = (MAGIC_NUMBER * bases) - (level * Platform_Random(10)); 
	for (int i=0; i<bases; i++) {
		Baseball_PlaySound(BASEBALL_SOUND_HIT, PLAYSOUNDFLAGS_PRIORITY);
		Platform_Pause(200);
	}

	// pick a random runner speed:
	// slow or medium or fast 
	nTimerRunnerMove = nRunnerSpeed = SPEED_BASE * (1 + Platform_Random(3));
	
	// insert a runner
	InsertRunner();
	return FSM_RUN;
}

static long long Baseball_Clock(int f)
{
	static struct timeval tv_start;
	struct timeval tv, tv_delta;
	long long rtn;
	
	if(f == 1) {
		gettimeofday(&tv_start, NULL);
		return 0;
	}
	gettimeofday(&tv, NULL);
	timersub(&tv, &tv_start, &tv_delta);
    
	rtn = tv_delta.tv_sec * 1000000 + tv_delta.tv_usec; 
	return rtn;
}

static BOOL run()
{
	if (--nTimerRunnerMove <= 0) {
		nTimerRunnerMove = nRunnerSpeed;
		if (MoveRunners()) {
			// record runs
			if (bHomeTeam) {
				++nHRuns;
			}
			else {
				++nVRuns;
			}
			// also note the run for later (for the sounds)
			++nPendingRuns;
		}
		return TRUE;
	}
	return FALSE;
}

static void pendigRunNotify()
{		
	while(nPendingRuns > 0) {
		Baseball_PlaySound(BASEBALL_SOUND_RUN, PLAYSOUNDFLAGS_PRIORITY);
		Platform_Pause(200);
		--nPendingRuns;
	}
	bDisplayOff = (nPendingRuns > 0) ? TRUE : FALSE;
}

// FINITE STATE MACHINE STUFF
static void fsmGoOut(int tu)
{
	static int delay = 0;
	
	if(delay == 0) {
		delay = 1000 * 600;
		bDisplayOff = TRUE;
		Baseball_PlaySound(BASEBALL_SOUND_OUT, PLAYSOUNDFLAGS_ASYNC|PLAYSOUNDFLAGS_PRIORITY);
		Baseball_Clock(1);
	}
	else if(Baseball_Clock(0) >= delay) {
		delay = 0;
		bDisplayOff = FALSE;
		fsm = FSM_OUT;	
	}	
}

static void fsmOut(int tu)
{
	++nOuts;
	if (nOuts == 3) {
		// end of possession

		// but first play any pending run sounds
		if(nPendingRuns > 0)
			pendigRunNotify();

		nOuts = 0;
		bHomeTeam = !bHomeTeam;
		if (!bHomeTeam) {
			++nInnings;
			if (nInnings == 5) {
				// ************************
				// ****** GAME OVER *******
				// ************************
				Baseball_PlaySound(BASEBALL_SOUND_ENDGAME, PLAYSOUNDFLAGS_ASYNC|PLAYSOUNDFLAGS_PRIORITY);
				fsm = FSM_GAMEOVER;
				return;
			}
		}
		PlatformSetInput(bHomeTeam);
		Baseball_PlaySound(BASEBALL_SOUND_ENDPOSSESSION, PLAYSOUNDFLAGS_PRIORITY|PLAYSOUNDFLAGS_ASYNC);	
		Baseball_DrawStats(-1, -1, nBalls, nStrikes, bHomeTeam);
		fsm = FSM_ENDPOSSESSION;
	}
	else {
		Baseball_DrawStats(-1, -1, nBalls, nStrikes, bHomeTeam);
		nStrikes = 0;
		nBalls = 0;
		nTimerEndPlayWait = (nDefenseBlip == -1) ? 1500 : 3000;
		Baseball_Clock(1);
		fsm = FSM_ENDPLAYWAIT;
	}
}

static void fsmIdle(int tu)
{
	// play the run sounds
	if(nPendingRuns > 0) 
		pendigRunNotify();

	nCurrentRunnerIndex = -1;
	nDefenseBlip = -1;
	bCaught = FALSE;
	nBallBlip = -1;
	
	if (Baseball_GetInputSCORE(NULL)) {
		Baseball_DrawScore(nVRuns, nHRuns);
	}
	else {
		Baseball_DrawStats(nInnings+1, nOuts, nBalls, nStrikes, bHomeTeam);
	}

	// wait for pitch button
	if (Baseball_GetInputPITCH(NULL)) {
		// delay from 1.5 to 2.5 seconds (according to manual)
		// before pitching the ball
		nTimerPitchWait = 1000 * (1500 + Platform_Random(1000)); // Microsecs
		Baseball_Clock(1);
		fsm = FSM_PITCHWAIT;
	}
}

static void fsmPitchWait(int tu)
{
	Baseball_DrawScore(-1, -1);
	
	if (Baseball_Clock(0) >= nTimerPitchWait) {
		int level = Baseball_GetSkill();
		
		// pick a ball speed and throw style
		// and pitch the ball
		bCurveball = (Platform_Random(3 - level) == 0) ? TRUE : FALSE;
		if (bCurveball) {
			nPitchSpeed = (3 - level) + Platform_Random(2 - level); // curve balls are a little slower
		}
		else {
			nPitchSpeed = (2 - level) + Platform_Random(3 - level);
		}
	
		nSwingSlot = nPitchSpeed;	
		nPitchIndex = 0;
		nBallBlip = (bCurveball) ? CURVE_BALL(nPitchIndex) : NORMAL_BALL(nPitchIndex);

		bStrike = FALSE;
		fsm = FSM_PITCHING;
	}
}

static void fsmPitching(int tu)
{
	const int nMissChance = 5;
	
	if(nPitchIndex <= 8) {
		if(Baseball_GetInputHIT(NULL)) {			
			if((nPitchIndex >= 5) && (nPitchIndex <= 7) && (Platform_Random(nMissChance) != 0)) {
				fsm = HitBall();
			}
			else {
				bStrike = TRUE;
				nPitchIndex = 9;
			}
		}
		else {
			if(--nSwingSlot <= 0) {	
				nSwingSlot = nPitchSpeed;			
				++nPitchIndex;
			}
		}
	}
	else {
		// if player didn't swing, randomly pick strike or ball
		if (Platform_Random(2) && !bStrike) {
			++nBalls;
		}
		else {
			++nStrikes;
		}

		if (nBalls == 4) {
			// walk
			Baseball_DrawStats(-1, -1, nBalls, nStrikes, bHomeTeam);
			Baseball_PlaySound(BASEBALL_SOUND_STRIKE, PLAYSOUNDFLAGS_PRIORITY|PLAYSOUNDFLAGS_ASYNC);
			nTimerRunnerMove = nRunnerSpeed = 2 * SPEED_BASE; // walk is always a fixed speed
			InsertRunner();
			fsm = FSM_WALK;
		}
		else if (nStrikes == 3) {
			// struck out
			fsm = FSM_GOOUT;
		}
		else {
			Baseball_DrawStats(-1, -1, nBalls, nStrikes, bHomeTeam);
			Baseball_PlaySound(BASEBALL_SOUND_STRIKE, PLAYSOUNDFLAGS_PRIORITY|PLAYSOUNDFLAGS_ASYNC);
			nTimerEndPlayWait = 1500;
			fsm = FSM_ENDPLAYWAIT;
		}
	}
	
	if(nPitchIndex <= 8)
		nBallBlip = (bCurveball) ? CURVE_BALL(nPitchIndex) : NORMAL_BALL(nPitchIndex);
}

static void fsmEndPlayWait(int tu)
{
	static int delay = 0;
	
	if(delay == 0) {
		delay = 1000 * nTimerEndPlayWait;
		Baseball_Clock(1);
	}
	else if(Baseball_Clock(0) >= delay) {
		delay = 0;
		// reset the runners
		ResetOffBaseRunners();
		fsm = FSM_IDLE;
	}
}

static void fsmRun(int tu)
{
	// RUN does not take effect until you release HIT
	if (Baseball_GetInputRUN(NULL) && !Baseball_GetInputHIT(NULL)) {
		run();
	}

	if (--nTimerFielding <= 0) {
		// ball is finished being fielded

		// ball has been fielded - see if any runners are off base
		BOOL bOffBase = FALSE;
		for (int i=0; i<MAX_RUNNERS; i++) {
			if (runners[i].enabled) {
				if ((runners[i].baseindex != FIRSTBASE)
					&& (runners[i].baseindex != SECONDBASE)
					&& (runners[i].baseindex != THIRDBASE)
					&& (runners[i].baseindex != HOMEPLATE)) {
					bOffBase = TRUE;
					break;
				}
			}
		}
		if (bOffBase) {
			// somebody is out -- go through the 'out' theatrics
			fsm = FSM_GOOUT;
		}
		else {
			// all runners are safe
			Baseball_DrawStats(-1, -1, nBalls, nStrikes, bHomeTeam);
			nBalls=0;
			nStrikes=0;
			nTimerEndPlayWait = 1500;
			fsm = FSM_ENDPLAYWAIT;
		}
	}
}

static void fsmWalk(int tu)
{
	if (Baseball_GetInputRUN(NULL)) {
		if(run() == TRUE) {
			// when the current runner gets on first, we're done
			if (runners[nCurrentRunnerIndex].baseindex == FIRSTBASE) {
				// got someone on first - all done
				nStrikes = 0;
				nBalls = 0;
				nTimerEndPlayWait = 1500;
				fsm = FSM_ENDPLAYWAIT;
			}
		}
	}
}

static void fsmHomeRun(int tu)
{
	static int loop = 0;
	
	if(fireWorks > 0) {
		//
		// do fireworks display
		//	
		
		if(fireWorks == 1)
			Baseball_PlaySound(BASEBALL_SOUND_RUN, PLAYSOUNDFLAGS_PRIORITY|PLAYSOUNDFLAGS_ASYNC);
		Platform_Pause(40);
		
		fireWorks++;
		if(fireWorks > 26) {
			loop++;
			if(loop == 4) {
				loop = 0;
				fireWorks = 0;
				// insert a runner
				InsertRunner();
				nTimerRunnerMove = nRunnerSpeed = 2 * SPEED_BASE; // fixed run speed
			}
			else {
				fireWorks = 1;
			}
		}
		return;
	}

	if (Baseball_GetInputRUN(NULL)) {
		if (run() == TRUE) {
			// see if anyone is still running the bases
			BOOL bManOnBase = FALSE;
			for (int i=0; i<MAX_RUNNERS; i++) {
				if (runners[i].enabled) {
					bManOnBase = TRUE;
					break;
				}
			}
			if (!bManOnBase) {
				// nobody on base - all done
				nStrikes = 0;
				nBalls = 0;
				fsm = FSM_IDLE;
			}
		}
	}
}

static void fsmEndPossession(int tu)
{
	// wait for SCORE key
	if (Baseball_GetInputSCORE(NULL)) {
		nOuts = 0;
		nStrikes = 0;
		nBalls = 0;

		// remove all runners
		for (int i=0; i<MAX_RUNNERS; i++) {
			runners[i].enabled = FALSE;
		}

		Baseball_DrawStats(nInnings+1, nOuts, nBalls, nStrikes, bHomeTeam);
		fsm = FSM_IDLE;
	}
}

static void fsmGameOver(int tu)
{
	Baseball_DrawScore(nVRuns, nHRuns);
}

#define LINE_STEP	20
void Baseball_Debug(int f)
{

	int w, h;
	int y = 0;
	
	Baseball_GetSize(&w, &h);
	
	debugPrintf(realx(w)+10, realy(y), 0xFFFFFFFF, "fsm =%d", fsm);
	y += LINE_STEP;
	debugPrintf(realx(w)+10, realy(y), 0xFFFFFFFF, "pitch i=%d s=%d", nPitchIndex, nPitchSpeed);
	y += LINE_STEP;
	debugPrintf(realx(w)+10, realy(y), 0xFFFFFFFF, "pitch w=%d", nTimerPitchWait/1000);
	y += LINE_STEP;
	debugPrintf(realx(w)+10, realy(y), 0xFFFFFFFF, "curve =%d", bCurveball);
	y += LINE_STEP;
	debugPrintf(realx(w)+10, realy(y), 0xFFFFFFFF, "Play tim =%d", nTimerEndPlayWait);
	y += LINE_STEP;
	debugPrintf(realx(w)+10, realy(y), 0xFFFFFFFF, "Field tim =%d",  nTimerFielding);
	y += LINE_STEP;
	debugPrintf(realx(w)+10, realy(y), 0xFFFFFFFF, "Swing =%d", nSwingSlot);
	y += LINE_STEP;
	debugPrintf(realx(w)+10, realy(y), 0xFFFFFFFF, "Speed =%d", nRunnerSpeed);
	y += LINE_STEP;
}
