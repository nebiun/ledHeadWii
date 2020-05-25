/*
 * LEDhead for Wii
 * Copyright (C) 2017-2020 Nebiun
 *
 * Based on the handheld electronic games by Mattel Electronics.
 * All trademarks copyrighted by their respective owners. This
 * program is not affiliated or endorsed by Mattel Electronics.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "Basketball2.h"
#include "Games.h"

// constants

#define GAMETIME	12.0
#define OVERTIME	5.0
#define PLAYTIME	24
#define PLAYTIME2	14
#define NUM_DEFENSEPLAYERS	5

#define TIME_TICKTIMER		10

#define TIME_STATSDISPLAY	5
#define TIME_BALLMOVE		5
#define TIME_WAITPASSAGE	(10 * (4 - nLevel + 1))
#define TIME_DEFLECT		2

#define DMODE_MAN			0
#define DMODE_ZONE			1
#define DMODE_PRESS			2

typedef struct {
	int state;
	const int value;
} BLIP;

static BLIP Blips[BASKETBALL2_BLIP_COLUMNS][BASKETBALL2_BLIP_ROWS] = {
	{ {0, 3}, {0, 0}, {0, 0}, {0, 3}, {0, 0} },
	{ {0, 2}, {0, 0}, {0, 2}, {0, 0}, {0, 0} },
	{ {0, 2}, {0, 2}, {0, 0}, {0, 0}, {0, 0} },
	{ {0, 2}, {0, 2}, {0, 2}, {0, 2}, {0, 3} },
	{ {0, 2}, {0, 2}, {0, 0}, {0, 0}, {0, 0} },
	{ {0, 2}, {0, 0}, {0, 2}, {0, 0}, {0, 0} },
	{ {0, 3}, {0, 0}, {0, 0}, {0, 3}, {0, 0} }
};

// game variables
static BOOL bGameOver;
static BOOL bHomeTeam;
static BOOL bInFrame = FALSE;
static BOOL bPower;
static int nLevel = -1;
static int nPoints;

static int nHScore;
static int nVScore;
static float fGameTime;
static int nPlayTime;
static int nQuarter;
static int nTimerTickTimer;
static int n3secTime;
static int nFreeShoot;

static BOOL bShoot;
static BOOL bPass;
static BOOL bPassed;
static BOOL bInPass;
static BOOL bFoul;
static int nBallDirectionX;
static int nBallDirectionY;
static int nTimerBallMove;
static int nDefenseMode = -1;

static BOOL bDeflect;
static BOOL nTimerDeflect;

static int nStatsIndex;
static int nTimerStatsDisplay;

typedef struct {
	int nColumn;
	int nRow;
	int nBright;
	int nColumnOld;
	int nRowOld;
	int nMoveNow;
}PLAYER;

static PLAYER ball;
static PLAYER player;
static PLAYER receiver;
static PLAYER defense[NUM_DEFENSEPLAYERS];

typedef struct {
	int min_x;
	int min_y;
	int max_x;
	int max_y;
} ZONE;

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
#define MOVEPLAYERLEFTUP(p)	{ \
	NOTECURRENTPLAYERPOSITION(p); \
	p.nColumn--; \
	p.nRow--;\
}
#define MOVEPLAYERLEFTDOWN(p)	{ \
	NOTECURRENTPLAYERPOSITION(p); \
	p.nColumn--; \
	p.nRow++; \
}
#define MOVEPLAYERRIGHT(p)	{ \
	NOTECURRENTPLAYERPOSITION(p); \
	p.nColumn++; \
}
#define MOVEPLAYERRIGHTUP(p)	{ \
	NOTECURRENTPLAYERPOSITION(p); \
	p.nColumn++; \
	p.nRow--; \
}
#define MOVEPLAYERRIGHTDOWN(p)	{ \
	NOTECURRENTPLAYERPOSITION(p); \
	p.nColumn++; \
	p.nRow++; \
}

#define IN_ZONE(x, y, z) \
	((x >= z->min_x && x <= z->max_x) && (y >= z->min_y && ny <= z->max_y))
	
#define IS_SHOOT_POSITION(x,y) ((Blips[x][y].value >= 2) ? 1 : 0)

#define IS_3SEC_ZONE(x,y)		((x >= 2 && x <= 4) && (y >= 0 && y <= 2))

static BOOL ISPLAYER(int x, int y)
{
	if ((player.nColumn == x)
		&& (player.nRow == y)
		&& (player.nBright)){
		return TRUE;
	}
	return FALSE;
}

static BOOL ISRECEIVER(int x, int y)
{
	if ((receiver.nColumn == x)
		&& (receiver.nRow == y)
		&& (receiver.nBright)){
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
	if (ISRECEIVER(x,y)){
		return TRUE;
	}
	if (ISDEFENSE(x,y)){
		return TRUE;
	}
	return FALSE;
}

static BOOL MOVEAT(int sx, int sy, int dx, int dy, int *nx, int *ny)
{
	int x = 0;
	int y = 0;
	
	if(sx != dx) { 
		if(sx > dx && sx > 0)
			x--;
		else if(sx < dx && sx < BASKETBALL2_BLIP_COLUMNS-1)
			x++;
	}	
	
	if(sy != dy) {
		if(sy > dy && sy > 0)
			y--;
		else if(sy < dy && sy < BASKETBALL2_BLIP_ROWS-1)
			y++;
	}
	
	if(x != 0 || y != 0) {
		if (ISPLAYER(sx+x,sy+y) && IS_SHOOT_POSITION(sx+x,sx+y)){
			if(y > 0)
				y = 0;
		}
		if(ISOCCUPIED(sx+x,sy+y)) {
			if(!ISOCCUPIED(sx,sy+y))
				x = 0;
			else if (!ISOCCUPIED(sx+x,sy))
				y = 0;
			else {
				x = 0;
				y = 0;
			}		
		}
		*nx = sx + x;
		*ny = sy + y;
		if(x == 0 && y == 0)
			return FALSE;
		return TRUE;
	} 
	
	*nx = sx;
	*ny = sy;
	return FALSE;
}

static BOOL ISAGOODPASS()
{
	int x, y;
	
	x = player.nColumn + nBallDirectionX;
	y = player.nRow + nBallDirectionY;
	
	if(nBallDirectionX == 0) {
		while(y != receiver.nRow) {
			y += nBallDirectionY;
			if(ISDEFENSE(x,y)) {
				return 0;
			}
		}			
		return 1;
	}
	if(nBallDirectionY == 0) {
		while(x != receiver.nColumn) {
			x += nBallDirectionX;
			if(ISDEFENSE(x,y)) {
				return 0;
			}
		}			
		return 1;
	}	
	
	while(x != receiver.nColumn && y != receiver.nRow) {
		x += nBallDirectionX;
		y += nBallDirectionY;
		if(ISDEFENSE(x,y)) {
			return 0;
		}
	}
	
	return 1;
}

static BOOL probability(int value)
{
	int rtn;
	
	rtn = (Platform_Random(100) <= ((value/4 * (nLevel + 1)) - Platform_Random(4 - nLevel))) ? TRUE : FALSE;

	return rtn;
}

static BOOL ISFOUL()
{
	int v = 0;
	int d = 0;
	
	if(ISDEFENSE(player.nColumn,player.nRow-1))
		d++;
	else if(ISDEFENSE(player.nColumn,player.nRow+1))
		d++;
	else if(ISDEFENSE(player.nColumn-1,player.nRow))
		d++;
	else if(ISDEFENSE(player.nColumn+1,player.nRow))
		d++;
	
	if(d == 0)
		return FALSE;

	if((player.nRow == 0) && (player.nColumn == 3))
		v = 80;
	else if((player.nRow >= 0 && player.nRow <= 1) && (player.nColumn >= 2 && player.nColumn <= 4))
		v = 40;
	else if((player.nRow >= 0 && player.nRow <= 2) && (player.nColumn >= 1 && player.nColumn <= 5))
		v = 20;
	else if(player.nRow >= 0 && player.nRow <= 3)
		v = 10;

	if(v == 0)
		return FALSE;
	
	if(probability(v))
		return TRUE;
	
	return FALSE;
}

static void freethrow_scheme()
{
	SETPLAYER(receiver, ((bHomeTeam) ? 1 : 5), 1, BLIP_DIMBLINK);
	SETPLAYER(player, 3, 3, BLIP_BRIGHT);
	UNSETPLAYER(ball);
	SETPLAYER(defense[0], 1, 0, BLIP_DIM);
	SETPLAYER(defense[1], ((bHomeTeam) ? 5 : 1), 1, BLIP_DIM);
	SETPLAYER(defense[2], 5, 0, BLIP_DIM);
	SETPLAYER(defense[3], 1, 2, BLIP_DIM);
	SETPLAYER(defense[4], 5, 2, BLIP_DIM);	
}

static void show_formation()
{
	int i;
	
	SETPLAYER(receiver, ((bHomeTeam) ? 6 : 0), 4, BLIP_DIMBLINK);
	SETPLAYER(player, 3, 4, BLIP_BRIGHT);
	UNSETPLAYER(ball);
	SETPLAYER(defense[0], 2, 0, BLIP_DIM);
	SETPLAYER(defense[1], 4, 0, BLIP_DIM);
	SETPLAYER(defense[2], 1, 2, BLIP_DIM);
	SETPLAYER(defense[3], 5, 2, BLIP_DIM);
	SETPLAYER(defense[4], 3, 3, BLIP_DIM);
	
	for (i=0; i<NUM_DEFENSEPLAYERS; i++)
		defense[i].nMoveNow = 0;
	receiver.nMoveNow = 0;	
}

#define ISPLAYERMOVED(p)	((p.nColumn != p.nColumnOld) || (p.nRow != p.nRowOld))

#define UNMOVEPLAYER(p) { \
	p.nRow = p.nRowOld; \
	p.nColumn = p.nColumnOld; \
}

#define ISPLAYEROFFSCREEN(p) \
	((p.nColumn > BASKETBALL2_BLIP_COLUMNS-1) \
	|| (p.nColumn < 0) \
	|| (p.nRow > BASKETBALL2_BLIP_ROWS-1) \
	|| (p.nRow < 0))


#define ISPLAYERENABLED(p) \
	(p.nBright)

// finite state machine stuff

static void fsmPlayStartWait();
static void fsmShowStats();
static void fsmDefenseWait();
static void fsmInPlay();
static void fsmDunk();
static void fsmGameOver();
static void fsmFreeThrow();
static void fsmFoul();

static enum FSM {
	FSM_PLAYSTARTWAIT=0,
	FSM_SHOWSTATS,
	FSM_DEFENSEWAIT,
	FSM_INPLAY,
	FSM_DUNK,
	FSM_GAMEOVER,
	FSM_FREETHROW,
	FSM_FOUL
}fsm;

typedef void (*FSMFCN)();

static FSMFCN fsmfcn[] = {
	fsmPlayStartWait,
	fsmShowStats,
	fsmDefenseWait,
	fsmInPlay,
	fsmDunk,
	fsmGameOver,
	fsmFreeThrow,
	fsmFoul
};

static void InitGame()
{
	bHomeTeam = FALSE;
	PlatformSetInput(bHomeTeam);
	
	nHScore = 0;
	nVScore = 0;
	fGameTime = GAMETIME;
	nQuarter = 0;
	bGameOver = FALSE;
	bDeflect = FALSE;

	fsm = FSM_DEFENSEWAIT;
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
				Blips[x][y].state = nBright;
			}
		}
		if (receiver.nBright != BLIP_OFF){
			x = receiver.nColumn;
			y = receiver.nRow;
			nBright = receiver.nBright;
			Blips[x][y].state = nBright;
		}
		if (player.nBright != BLIP_OFF){
			x = player.nColumn;
			y = player.nRow;
			nBright = player.nBright;
			Blips[x][y].state = nBright;
		}
		if (ball.nBright != BLIP_OFF){
			x = ball.nColumn;
			y = ball.nRow;
			nBright = ball.nBright;
			Blips[x][y].state = nBright;
		}
	}
	// draw the blips field
	for (y = 0; y < BASKETBALL2_BLIP_ROWS; y++){
		for (x = 0; x < BASKETBALL2_BLIP_COLUMNS; x++){
			switch(Blips[x][y].state){
			case BLIP_OFF:
			case BLIP_DIM:
			case BLIP_BRIGHT:
				Basketball2_DrawBlip(Blips[x][y].state, x, y);
				break;
			case BLIP_DIMBLINK:
				if (!bBlink){
					Basketball2_DrawBlip(BLIP_DIM, x, y);
				} else {
					Basketball2_DrawBlip(BLIP_OFF, x, y);
				}
				break;
			case BLIP_BRIGHTBLINK:
				if (!bBlink){
					Basketball2_DrawBlip(BLIP_BRIGHT, x, y);
				} else {
					Basketball2_DrawBlip(BLIP_OFF, x, y);
				}
				break;
			}
		}
	}

	bBlink = !bBlink;
}

BOOL Basketball2_GetPower()
{
	return (bPower ? TRUE : FALSE);
}

void Basketball2_PowerOn()
{
	InitGame();
	bPower = TRUE;
}

void Basketball2_PowerOff()
{
	bPower = FALSE;
	Basketball2_StopSound();
}

void Basketball2_SetSkill(int i)
{
	nLevel = i;
}

int Basketball2_GetSkill()
{
	return nLevel;
}


void Basketball2_Run(int tu)
{
	int x, y;

	// prevent reentrancy
	if (bInFrame){ return; }
	bInFrame = TRUE;

	// init the blips field
	for (y = 0; y < BASKETBALL2_BLIP_ROWS; y++){
		for (x = 0; x < BASKETBALL2_BLIP_COLUMNS; x++){
			Blips[x][y].state = BLIP_OFF;
		}
	}

	if (!bPower){
		Basketball2_ClearScreen();
		bInFrame = FALSE;
		return;
	}

	Platform_StartDraw();

	(fsmfcn[fsm])();

	DrawBlips();

	Platform_EndDraw();

	bInFrame = FALSE;

}

static int moveCounterInit()
{
	int rtn;
	
	switch(nLevel) {
	case LVL_COLLAGE:
		rtn = Platform_Random(12);
		break;
	case LVL_ALLSTAR:
		rtn = Platform_Random(8);
		break;
	case LVL_PROFESSIONAL:
		rtn = Platform_Random(4);
		break;
	case LVL_HIGHSCHOOL:
	default:
		rtn = Platform_Random(16);
		break;
	}
	return rtn;
}

static void initDefense()
{
	int i;
	
	for(i=0; i < NUM_DEFENSEPLAYERS; i++) {
		defense[i].nMoveNow = moveCounterInit();
	}
}

static void manDefense()
{
	static ZONE zone[NUM_DEFENSEPLAYERS] = {
		{1, 0, 3, 2}, {3, 0, 5, 2}, {1, 1, 3, 3}, {3, 1, 5, 3}, {2, 1, 4, 3}
	};
	int i, d = 0;
	
	for(i=0; i < NUM_DEFENSEPLAYERS; i++) {
		int nx, ny;
		ZONE *z = &zone[i];
		
		nx = defense[i].nColumn;
		ny = defense[i].nRow;	
		if(defense[i].nMoveNow == 0) {
			if(d < 2 && IN_ZONE(player.nColumn, player.nRow, z)) {
				d++;
				MOVEAT(defense[i].nColumn, defense[i].nRow, player.nColumn, player.nRow, &nx, &ny);
			}
			else {
				MOVEAT(defense[i].nColumn, defense[i].nRow, player.nColumn, player.nRow, &nx, &ny);
			}	
			if(!IN_ZONE(nx,ny,z)) {
				nx = defense[i].nColumn;
				ny = defense[i].nRow;			
			}
			defense[i].nMoveNow = moveCounterInit();
		}
		else {
			defense[i].nMoveNow--;
		}
		SETPLAYER(defense[i], nx, ny, BLIP_DIM);
	}		
}

static void zoneDefense()
{
	static ZONE zone[NUM_DEFENSEPLAYERS] = {
		{0, 0, 4, 1}, {2, 0, 5, 2}, {0, 0, 2, 4}, {4, 0, 6, 4}, {0, 2, 6, 4}
	};
	int i;
	
	for(i=0; i < NUM_DEFENSEPLAYERS; i++) {
		int nx, ny;
		ZONE *z = &zone[i];
		
		nx = defense[i].nColumn;
		ny = defense[i].nRow;	
		if(defense[i].nMoveNow == 0) {
			MOVEAT(defense[i].nColumn, defense[i].nRow, player.nColumn, player.nRow, &nx, &ny);
			if(!IN_ZONE(nx, ny, z)) {
				nx = defense[i].nColumn;
				ny = defense[i].nRow;
			}
			defense[i].nMoveNow = moveCounterInit();
		}
		else {
			defense[i].nMoveNow--;
		}
		SETPLAYER(defense[i], nx, ny, BLIP_DIM);
	}
}

static void pressDefense()
{
	static ZONE zone[NUM_DEFENSEPLAYERS] = {
		{1, 0, 3, 3}, {3, 0, 5, 3}, {1, 1, 5, 3}, {1, 1, 5, 3}, {1, 2, 5, 3}
	};
	int i, d = 0;
	
	for(i=0; i < NUM_DEFENSEPLAYERS; i++) {
		int nx, ny;
		ZONE *z = &zone[i];
		
		nx = defense[i].nColumn;
		ny = defense[i].nRow;	
		if(defense[i].nMoveNow == 0) {
			if(IN_ZONE(receiver.nColumn, receiver.nRow, z)) {
				d++;
				MOVEAT(defense[i].nColumn, defense[i].nRow, receiver.nColumn, receiver.nRow, &nx, &ny);
			}
			else {
				if(d == 0 && i == 4) {
					d++;
					MOVEAT(defense[i].nColumn, defense[i].nRow, receiver.nColumn, receiver.nRow, &nx, &ny);
				}
				else {
					MOVEAT(defense[i].nColumn, defense[i].nRow, player.nColumn, player.nRow, &nx, &ny);
				}
			}
			if(!IN_ZONE(nx,ny,z)) {
				nx = defense[i].nColumn;
				ny = defense[i].nRow;
			}
			defense[i].nMoveNow = moveCounterInit();
		}
		else {
			defense[i].nMoveNow--;	
		}
		SETPLAYER(defense[i], nx, ny, BLIP_DIM);
	}
}

static void moveReceiver()
{
	static int objx, objy;
	int nx, ny;	
	
	nx = receiver.nColumn;
	ny = receiver.nRow;

	if(receiver.nMoveNow == 0) {	
		if((receiver.nColumn == 0 || receiver.nColumn == 6) && receiver.nRow == 4) {
			objx = 3;
			objy = 0;
		}
		else if(receiver.nColumn == objx && receiver.nRow == objy) {
			do {
				objx = Platform_Random(BASKETBALL2_BLIP_COLUMNS);
				objy = Platform_Random(BASKETBALL2_BLIP_ROWS);
			} while(receiver.nColumn == objx && receiver.nRow == objy);
		}
/*
		trc_x = objx;
		trc_y = objy;
		trc_x1 = receiver.nColumn;
		trc_y1 = receiver.nRow;
*/		
		if(MOVEAT(receiver.nColumn, receiver.nRow, objx, objy, &nx, &ny) != TRUE) {
			int nobjx, nobjy;

			do {
				nobjx = Platform_Random(BASKETBALL2_BLIP_COLUMNS);
				nobjy = Platform_Random(BASKETBALL2_BLIP_ROWS);
			} while(nobjx == objx && nobjy == objy);
			objx = nobjx;
			objy = nobjy;
			nx = receiver.nColumn;
			ny = receiver.nRow;
		}
		
		receiver.nMoveNow = 5 + Platform_Random(TIME_WAITPASSAGE);
		if(IS_SHOOT_POSITION(nx,ny))
			receiver.nMoveNow += 10;
	}
	else {
		receiver.nMoveNow--;
	}
	SETPLAYER(receiver, nx, ny, BLIP_DIMBLINK);
}

// FINITE STATE MACHINE STUFF

static void fsmPlayStartWait()
{
	bShoot = FALSE;
	bPass = FALSE;
	nPlayTime = PLAYTIME;
	n3secTime = -1;
	nTimerTickTimer = TIME_TICKTIMER;
	bDeflect = FALSE;
	nTimerDeflect = 0;

	// show stats if pressed 'down'
	// else, wait for player to move then advance to next state
	BOOL bChange;
	if (Basketball2_GetInputDOWN(&bChange))
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
	else if (Basketball2_TestForMovement())
	{
		Platform_IsNewSecond();
		fsmInPlay();
		fsm = FSM_INPLAY;
		return;
	}

	Basketball2_DrawStat((int)fGameTime);
}

static void fsmShowStats()
{
	// show formation
	if (!bGameOver)
	{
		show_formation();
	}

	if (--nTimerStatsDisplay <= 0)
	{
		++nStatsIndex;
		nTimerStatsDisplay = TIME_STATSDISPLAY;
	}
	switch(nStatsIndex)
	{
		case 0:
			Basketball2_DrawStat(-1);
			break;
		case 1:
			Basketball2_DrawStat(nVScore);
			break;
		case 2:
			Basketball2_DrawStat(-1);
			break;
		case 3:
			Basketball2_DrawStat(nHScore);
			break;
		case 4:
			Basketball2_DrawStat(-1);
			break;
		default:
		case 5:
			if (bGameOver) {
				fsm = FSM_GAMEOVER;
			}
			else {
				fsm = FSM_DEFENSEWAIT;
			}
			break;
	}
}

static void fsmDefenseWait()
{	
	BOOL bChange;

	show_formation();
	nDefenseMode = -1;
	
	if (Basketball2_GetInputUP(&bChange)) {
		if (bChange)
			nDefenseMode = DMODE_MAN;
	}
	else if (Basketball2_GetInputLEFT(&bChange))	{
		if (bChange)
			nDefenseMode = DMODE_ZONE;
	}
	else if (Basketball2_GetInputRIGHT(&bChange)) {
		if (bChange)
			nDefenseMode = DMODE_PRESS;	
	}
	else if (Basketball2_GetInputDOWN(&bChange))
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
	Basketball2_DrawStat((int)fGameTime);
	if (nDefenseMode != -1) {
/*		trace_bug = nDefenseMode; */
		initDefense();
		fsm = FSM_PLAYSTARTWAIT;
	}
}

static void fsmInPlay()
{
	BOOL bChange;
	
	if (Basketball2_GetInputLEFTUP(&bChange)) {
		if (bChange)
		{
			if ((player.nColumn > 0) && (player.nRow > 0)
				&& (!ISOCCUPIED(player.nColumn-1, player.nRow-1))){
				MOVEPLAYERLEFTUP(player);
			}
		}	
	}
	else if (Basketball2_GetInputLEFTDOWN(&bChange)) {
		if (bChange)
		{
			if ((player.nColumn > 0) && (player.nRow < (BASKETBALL2_BLIP_ROWS-1))
				&& (!ISOCCUPIED(player.nColumn-1, player.nRow+1))){
				MOVEPLAYERLEFTDOWN(player);
			}
		}	
	}
	else if (Basketball2_GetInputRIGHTUP(&bChange)) {
		if (bChange)
		{
			if ((player.nColumn < (BASKETBALL2_BLIP_COLUMNS-1)) && (player.nRow > 0)
				&& (!ISOCCUPIED(player.nColumn+1, player.nRow-1))){
				MOVEPLAYERRIGHTUP(player);
			}
		}	
	}	
	else if (Basketball2_GetInputRIGHTDOWN(&bChange)) {
		if (bChange)
		{
			if ((player.nColumn < (BASKETBALL2_BLIP_COLUMNS-1)) && (player.nRow < (BASKETBALL2_BLIP_ROWS-1))
				&& (!ISOCCUPIED(player.nColumn+1, player.nRow+1))){
				MOVEPLAYERRIGHTDOWN(player);
			}
		}		
	}	
	else if (Basketball2_GetInputLEFT(&bChange))
	{
		if (bChange)
		{
			if ((player.nColumn > 0)
				&& (!ISOCCUPIED(player.nColumn-1, player.nRow))){
				MOVEPLAYERLEFT(player);
			}
		}
	}
	else if (Basketball2_GetInputRIGHT(&bChange))
	{
		if (bChange)
		{
			if ((player.nColumn < (BASKETBALL2_BLIP_COLUMNS-1))
				&& (!ISOCCUPIED(player.nColumn+1, player.nRow))){
				MOVEPLAYERRIGHT(player);
			}
		}
	}
	else if (Basketball2_GetInputUP(&bChange))
	{
		if (bChange)
		{
			if ((player.nRow > 0)
				&& (!ISOCCUPIED(player.nColumn, player.nRow-1))){
				MOVEPLAYERUP(player);
			}
		}
	}
	else if (Basketball2_GetInputDOWN(&bChange))
	{
		if (bChange)
		{
			if ((player.nRow < (BASKETBALL2_BLIP_ROWS-1))
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

	if (Basketball2_GetInputSHOOT(&bChange))
	{
		if (bChange)
		{
			if (!ISPLAYERENABLED(ball))
			{
				// shoot!
				bShoot = TRUE;
				bPassed = FALSE;
				bInPass = FALSE;
				nTimerBallMove = TIME_BALLMOVE;
				SETPLAYERBRIGHTNESS(player, BLIP_DIMBLINK);
				SETPLAYER(ball, player.nColumn, player.nRow, BLIP_BRIGHT);
				nPoints = Blips[player.nColumn][player.nRow].value;
				
				if (player.nRow > 0)
					nBallDirectionY = -1;	 
				else
					nBallDirectionY = 0;

				if (player.nColumn < 3)
					nBallDirectionX = 1;
				else if(player.nColumn > 3)
					nBallDirectionX = -1;
				else // player.nColumn == 3
					nBallDirectionX = 0;
					
				bFoul = ISFOUL();
			}
		}
	}
	
	if (Basketball2_GetInputPASS(&bChange))
	{
		if (bChange)
		{
			if (!ISPLAYERENABLED(ball))
			{
				int x, y;
				
				// Pass!
				bPass = TRUE;
				bInPass = TRUE;
				nTimerBallMove = TIME_BALLMOVE;
				SETPLAYERBRIGHTNESS(player, BLIP_DIMBLINK);
				SETPLAYER(ball, player.nColumn, player.nRow, BLIP_BRIGHT);
				
				y = player.nRow - receiver.nRow;
				x = player.nColumn - receiver.nColumn;
				
				if(x > 0)
					nBallDirectionX = -1;
				else if(x < 0)
					nBallDirectionX = 1;
				else
					nBallDirectionX = 0;
			
				if(y > 0)
					nBallDirectionY = -1;
				else if(y < 0)
					nBallDirectionY = 1;
				else
					nBallDirectionY = 0;
				
				bPassed = ISAGOODPASS();
			}
		}
	}
	// move the receiver
	moveReceiver();

	// move defenders
	switch(nDefenseMode) {
	case DMODE_MAN:
		manDefense();
		break;
	case DMODE_ZONE:
		zoneDefense();
		break;
	case DMODE_PRESS:
		pressDefense();
		break;
	}

	// move the ball
	if (ISPLAYERENABLED(ball))
	{
		n3secTime = -1;
		
		// the puck hit a defender last frame - play the sound
		if (ISDEFENSE(ball.nColumn, ball.nRow) && !bDeflect &&  !bPassed)
		{
			bDeflect = TRUE;
			Basketball2_PlaySound(BASKETBALL2_SOUND_BOUNCE, PLAYSOUNDFLAGS_PRIORITY);
			nTimerDeflect = TIME_DEFLECT;
			return;
		}

		if (--nTimerBallMove <= 0)
		{
			nTimerBallMove = TIME_BALLMOVE;
			if (bFoul) {
				Basketball2_PlaySound(BASKETBALL2_SOUND_FOUL, PLAYSOUNDFLAGS_PRIORITY);
			}
			
			// check for a basket
			if ((ball.nColumn == 3) && (ball.nRow == 0) && !bInPass
				&& !ISDEFENSE(ball.nColumn, ball.nRow)) // <-- is this right?
			{
				// basket!
				nFreeShoot = (bFoul) ? 1 : 0;
				ball.nRow = -1;
				Basketball2_DrawBasket(TRUE);
				fsm = FSM_DUNK;
				return;
			}
			
			// check the foul
			if (bFoul) {
				nFreeShoot = nPoints;
				fsm = FSM_FOUL;
				return;
			}

			// bounce the ball off the far wall
			if (!ISDEFENSE(ball.nColumn, ball.nRow) && !ISRECEIVER(ball.nColumn, ball.nRow))
			{
				if ((ball.nRow <= 0) && (ball.nColumn >= 2 && ball.nColumn <= 4) && (nBallDirectionY == -1))
				{
					ball.nRow = 0;
					nBallDirectionY = 1;
					Basketball2_PlaySound(BASKETBALL2_SOUND_BOUNCE, PLAYSOUNDFLAGS_PRIORITY);
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
			if (ISPLAYER(ball.nColumn, ball.nRow) && !bShoot && !bPass)
			{
				UNSETPLAYER(ball);
				SETPLAYERBRIGHTNESS(player, BLIP_BRIGHT);
			}
			else if (ISRECEIVER(ball.nColumn, ball.nRow)) {
				PLAYER tmp;
				
				bPassed = FALSE;
				UNSETPLAYER(ball);
				tmp = player;
				player = receiver;
				receiver = tmp;
				SETPLAYERBRIGHTNESS(player, BLIP_BRIGHT);
			}
			else
			{
				// move the ball
				ball.nColumn += nBallDirectionX;
				ball.nRow += nBallDirectionY;
				bShoot = FALSE;	
				bPass = FALSE;	

				// see if the ball bounced off the screen
				if (ISPLAYEROFFSCREEN(ball) &&  ISPLAYERENABLED(ball))
				{
					// off the screen - get rid of it and end the play
					UNSETPLAYER(ball);
					SETPLAYERBRIGHTNESS(player, BLIP_BRIGHT);
					Basketball2_ClearScreen();
					Basketball2_PlaySound(BASKETBALL2_SOUND_ENDPLAY, PLAYSOUNDFLAGS_PRIORITY);
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
	
	// 3sec time rule
	if(IS_3SEC_ZONE(player.nColumn, player.nRow) && !ISPLAYERENABLED(ball)) {
		if(n3secTime == -1)
			n3secTime = 3;
	}
	else 
		n3secTime = -1;

	// count down the clock
	if (Platform_IsNewSecond()){
		// check for end of quarter
		if (fGameTime > 0.0){
			fGameTime -= (float).1;
			if (fGameTime < 0.1){

				// end of quarter
				++nQuarter;
				
				// Overtime needed if nHScore == nVScore
				if ((nQuarter >= 4) && (nHScore != nVScore))
				{
					// game over
					bGameOver = TRUE;

					show_formation();

					Basketball2_ClearScreen();
					Basketball2_PlaySound(BASKETBALL2_SOUND_ENDGAME, PLAYSOUNDFLAGS_PRIORITY);
					nStatsIndex = 0;
					nTimerStatsDisplay = TIME_STATSDISPLAY;
					fsm = FSM_SHOWSTATS;
					return;
				}

				bHomeTeam = (nQuarter % 2) ? FALSE : TRUE;
				PlatformSetInput(bHomeTeam);

				fGameTime = (nQuarter < 4) ? GAMETIME : OVERTIME;
				Basketball2_ClearScreen();
				if(fGameTime == GAMETIME)
					Basketball2_PlaySound(BASKETBALL2_SOUND_ENDQUARTER, PLAYSOUNDFLAGS_PRIORITY);
				else
					Basketball2_PlaySound(BASKETBALL2_SOUND_OVERTIME, PLAYSOUNDFLAGS_PRIORITY);
				nStatsIndex = 0;
				nTimerStatsDisplay = TIME_STATSDISPLAY;
				fsm = FSM_SHOWSTATS;
				return;
			}
		}
		// check for 3sec time
		if(n3secTime != -1) {
			n3secTime--;
			if(n3secTime == 0) {
				Basketball2_ClearScreen();
				Basketball2_PlaySound(BASKETBALL2_SOUND_3SECS, PLAYSOUNDFLAGS_PRIORITY);
				bHomeTeam = !bHomeTeam;
				PlatformSetInput(bHomeTeam);
				nStatsIndex = 0;
				nTimerStatsDisplay = TIME_STATSDISPLAY;
				fsm = FSM_SHOWSTATS;
				return;	
			}
		}
		
		// check for end of play timer
		if (--nPlayTime < 0){
			Basketball2_ClearScreen();
			Basketball2_PlaySound(BASKETBALL2_SOUND_ENDPLAY, PLAYSOUNDFLAGS_PRIORITY);
			bHomeTeam = !bHomeTeam;
			PlatformSetInput(bHomeTeam);
			nStatsIndex = 0;
			nTimerStatsDisplay = TIME_STATSDISPLAY;
			fsm = FSM_SHOWSTATS;
			return;
		}
	}

	Basketball2_DrawStat(nPlayTime);

	// make tick sounds
	if (fGameTime >= .1){
		if (--nTimerTickTimer <= 0){
			Basketball2_PlaySound(BASKETBALL2_SOUND_TICK, PLAYSOUNDFLAGS_ASYNC);
			nTimerTickTimer = TIME_TICKTIMER;
		}
	}
}
				
static void fsmDunk()
{
	switch(nPoints) {
	case 1:
		Basketball2_PlaySound(BASKETBALL2_SOUND_FREETHROW, PLAYSOUNDFLAGS_PRIORITY);
		break;
	case 3:
		Basketball2_PlaySound(BASKETBALL2_SOUND_SCORE3, PLAYSOUNDFLAGS_PRIORITY);
		break;
	default:
		Basketball2_PlaySound(BASKETBALL2_SOUND_SCORE, PLAYSOUNDFLAGS_PRIORITY);
		break;
	}	
	Basketball2_DrawBasket(FALSE);
	if (bHomeTeam) 
		nHScore += nPoints;
	else
		nVScore += nPoints;
	
	if(!bFoul) {
		show_formation();
		Platform_IsNewSecond();
		fsm = FSM_INPLAY;
	}
	else {
		fsm = FSM_FOUL;
	}
}

static void fsmFoul()
{
	Basketball2_ClearScreen();
	freethrow_scheme();
	fsm = FSM_FREETHROW;
}

static void fsmFreeThrow()
{
	BOOL bChange;
	if (Basketball2_GetInputSHOOT(&bChange)) {
		if (bChange) {
			nFreeShoot--;
			nTimerBallMove = TIME_BALLMOVE;
			SETPLAYERBRIGHTNESS(player, BLIP_DIMBLINK);
			SETPLAYER(ball, player.nColumn, player.nRow, BLIP_BRIGHT);
		}
	}
	
	if (--nTimerBallMove <= 0) {
		nTimerBallMove = TIME_BALLMOVE;
		
		// check for a basket
		if (ball.nRow == 0) {
			if(nFreeShoot == 0)
				bFoul = 0;
			// basket!
			if(probability(100)) {
				nPoints = 1;
				ball.nRow = -1;
				Basketball2_DrawBasket(TRUE);
				fsm = FSM_DUNK;
			}
			else {
				ball.nRow = 1;
				nBallDirectionY = 1;
				nBallDirectionX = Platform_Random(3) - 1;
				Basketball2_PlaySound(BASKETBALL2_SOUND_BOUNCE, PLAYSOUNDFLAGS_PRIORITY);
				nTimerDeflect = TIME_DEFLECT;
				bDeflect = FALSE;
				nPlayTime = PLAYTIME2;
				Platform_IsNewSecond();
				fsm = FSM_INPLAY;
			}
			return;
		}
		// move the ball
		ball.nRow -= 1;
	}	
}

static void fsmGameOver()
{
	// show stats if pressed 'down'
	BOOL bChange;
	if (Basketball2_GetInputDOWN(&bChange))
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

	Basketball2_DrawStat(0);
}

void Basketball2_Debug(int f)
{
	
}
