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

#include "Soccer2.h"
#include "Games.h"

// constants

#define NUM_DEFENSEPLAYERS	5
#define TIME_STATSDISPLAY	5
#define TIME_BALLMOVE		5
#define GAMETIME			45.9
#define TIME_TICKTIMER		10
#define BALL_LOWKICK		(3 + Platform_Random(2))
#define BALL_HIGHKICK		(5 + Platform_Random(4))

#define IS_GOALAREA()		((nSector == 1) || (nSector == 9))
#define IS_ATTACKAREA()		((nSector == 1 && bHomeTeam) || (nSector == 9 && !bHomeTeam))

typedef struct {
	int state;
} BLIP;

typedef struct {
	Coord_t player;
	Coord_t receiver;
	Coord_t defense[NUM_DEFENSEPLAYERS];
} SCHEME_t;

static BLIP Blips[SOCCER2_BLIP_COLUMNS][SOCCER2_BLIP_ROWS];

// debug
static int trc_debug;

// game variables
static BOOL bGameOver;
static BOOL bHomeTeam;
static BOOL bInFrame = FALSE;
static BOOL bPower;
static BOOL bBounce = FALSE;
static BOOL bTeamMate = FALSE;
static BOOL bTeamMateSet = FALSE;
static BOOL bKickKey = FALSE;

static int nLevel = -1;
static int nHScore;
static int nVScore;
static int fGameTime;
static int nHalf;
static int nTimerTickTimer;
static int nSector;
static int nBallDirectionX;
static int nBallDirectionY;
static int nTimerBallMove;
static int nStatsIndex;
static int nTimerStatsDisplay;
static int nBallStep;
static int nBallMax;

typedef struct {
	int x;
	int y;
	int side;
}BALL_OUT;
static BALL_OUT ball_out;

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
static PLAYER goalkeeper;
static PLAYER defense[NUM_DEFENSEPLAYERS];

// macros for dealing with the players

#define SETPLAYERBRIGHTNESS(p,b)	{ \
	(p)->nBright = b; \
}

#define NOTECURRENTPLAYERPOSITION(p) { \
	(p)->nColumnOld = (p)->nColumn; \
	(p)->nRowOld = (p)->nRow; \
}

#define SETPLAYER(p,x,y,b)	{ \
	NOTECURRENTPLAYERPOSITION(p); \
	if (x >= 0) { (p)->nColumn = (x) % SOCCER2_BLIP_COLUMNS; } \
	if (y >= 0) { (p)->nRow = (y) % SOCCER2_BLIP_ROWS; } \
	if (b >= 0) { (p)->nBright = b; }\
}

#define UNSETPLAYER(p)	{ \
	(p)->nBright = BLIP_OFF; \
}

#define ISPLAYERMOVED(p)	(((p)->nColumn != (p)->nColumnOld) || ((p)->nRow != (p)->nRowOld))

#define UNMOVEPLAYER(p) { \
	(p)->nRow = (p)->nRowOld; \
	(p)->nColumn = (p)->nColumnOld; \
}

#define ISPLAYEROFFSCREEN(p) 	(!VALID_COLUMN((p)->nColumn) || !VALID_ROW((p)->nRow))

#define ISPLAYERENABLED(p) \
	((p)->nBright)

#define ISBALL(x,y)			generic_check(&ball, x, y)
#define ISPLAYER(x,y)		generic_check(&player, x, y)
#define ISRECEIVER(x,y)		generic_check(&receiver, x, y)
#define ISGOALKEEPER(x,y)	generic_check(&goalkeeper, x, y)

#define SET_BALLOUT(p)		{ ball_out.x = (p)->nColumnOld; ball_out.y = (p)->nRowOld; ball_out.side = Platform_Random(2); }

static BOOL generic_check(PLAYER *p, int x, int y)
{
	if ((p->nColumn == x)
		&& (p->nRow == y)
		&& (p->nBright)){
		return TRUE;
	}
	return FALSE;
}

static BOOL ISDEFENSE(int x, int y)
{
	for (int i=0; i<NUM_DEFENSEPLAYERS; i++){
		if(generic_check(&defense[i],x,y) == TRUE) {
			return TRUE;
		}
	}
	return FALSE;
}

static PLAYER *GETPLAYERAT(int x, int y)
{
	if (generic_check(&ball, x, y))
		return &ball;
	
	if (generic_check(&player, x, y))
		return &player;
	
	if(generic_check(&receiver, x, y))
		return &receiver;
	
	if (generic_check(&goalkeeper, x, y))
		return &goalkeeper;
	
	for (int i=0; i<NUM_DEFENSEPLAYERS; i++) {
		if(generic_check(&defense[i],x,y)) {
			return &defense[i];
		}
	}

	return (PLAYER *)NULL;
}

static BOOL ISOCCUPIED(int x, int y)
{
	PLAYER *p;
	
	p = GETPLAYERAT(x, y);
	if( p == NULL || p == &ball)
		return FALSE;

	return TRUE;
}
/* Not used (for now)
static int move_torow(PLAYER *p, int row, int flag)
{
	int v = 0;
	int c, rtn;
	
	if(row == (p->nRow + 1)) {
		v = 1;
	}
	else if(row == (p->nRow - 1)) {
		v = -1;
	}
	else {
		v = 0;
	}
	
	c = (flag) ? -1 : 1;
	
	if(v != 0 && !ISOCCUPIED(p->nColumn,p->nRow+v)) {
		rtn = p->nColumn;
	}
	else if(VALID_COLUMN(p->nColumn+c) && !ISOCCUPIED(p->nColumn+c,p->nRow+v)) {
		rtn = p->nColumn+c;
	}
	else if(VALID_COLUMN(p->nColumn-c) && !ISOCCUPIED(p->nColumn-c,p->nRow+v)) {
		rtn = p->nColumn-c;
	}
	else {
		rtn = -1;
	}

	return rtn;	
}
*/
static int move_tocol(PLAYER *p, int col, int flag)
{
	int v = 0;
	int r, rtn;
	
	if(col == (p->nColumn + 1)) {
		v = 1;
	}
	else if(col == (p->nColumn - 1)) {
		v = -1;
	}
	else {
		v = 0;
	}
	
	r = (flag) ? -1 : 1;
	
	if(v != 0 && !ISOCCUPIED(p->nColumn+v,p->nRow)) {
		rtn = p->nRow;
	}
	else if(VALID_ROW(p->nRow+r) && !ISOCCUPIED(p->nColumn+v,p->nRow+r)) {
		rtn = p->nRow+r;
	}
	else if(VALID_ROW(p->nRow-r) && !ISOCCUPIED(p->nColumn+v,p->nRow-r)) {
		rtn = p->nRow-r;
	}
	else {
		rtn = -1;
	}
	return rtn;	
}

static BOOL move_around(PLAYER *p)
{
	int x, y;
	int x1, y1;
	
	for(x=0; x<SOCCER2_BLIP_COLUMNS; x++) {
		x1 = (x + p->nColumn) % SOCCER2_BLIP_COLUMNS;
		for(y=0; y<SOCCER2_BLIP_ROWS; y++) {
			y1 = (y + p->nRow) % SOCCER2_BLIP_ROWS;
			if(!ISOCCUPIED(x1,y1)) {
				SETPLAYER(p, x1, y1, -1);
				return TRUE;
			}
		}
	}
	
	return FALSE;
}

static BOOL set_goalkeeper()
{
	PLAYER *p;
	int x;
	
	if(nSector == 1 && bHomeTeam)
		x = 0;
	else if(nSector == 9 && !bHomeTeam)
		x = 6;
	else {
		UNSETPLAYER(&goalkeeper);
		return FALSE;
	}
	if( (p = GETPLAYERAT(x, 2)) != NULL ) {
		move_around(p);
	}
	SETPLAYER(&goalkeeper,x, 2, BLIP_DIM);
	return TRUE;
}

// finite state machine stuff
#define FSM_RUNNING(x)	((x == FSM_INPLAY) || (x == FSM_SHOOTING) || (x == FSM_GOAL) || (x == FSM_PASSING))
#define next_fsm(x)		{ if(FSM_RUNNING(x)) {fsm = x; nextFsm = FSM_INVALID; } else { fsm = FSM_WAITSCOREPRESS; nextFsm = x; } }

static void fsmKickoff();
static void fsmTurnover();
static void fsmThrowin();
static void fsmGoalKick();
static void fsmCornerKick();
static void fsmInPlay();
static void fsmPassing();
static void fsmShooting();
static void fsmGameOver();
static void fsmWaitScorePress();
static void fsmGoal();

enum FSM {
	FSM_INVALID = -1,
	FSM_KICKOFF = 0,
	FSM_TURNOVER,
	FSM_THROWIN,
	FSM_GOALKICK,
	FSM_CORNERKICK,
	FSM_INPLAY,
	FSM_PASSING,
	FSM_SHOOTING,
	FSM_GAMEOVER,
	FSM_WAITSCOREPRESS,
	FSM_GOAL
};

static enum FSM fsm, nextFsm;

typedef void (*FSMFCN)();

static FSMFCN fsmfcn[] = {
	fsmKickoff,
	fsmTurnover,
	fsmThrowin,
	fsmGoalKick,
	fsmCornerKick,
	fsmInPlay,
	fsmPassing,
	fsmShooting,
	fsmGameOver,
	fsmWaitScorePress,
	fsmGoal
};

static int probability(int value)
{
	int rtn;
	
	rtn = (Platform_Random(100) <= ((value/4 * (nLevel + 1)) - Platform_Random(4 - nLevel))) ? 1 : 0;

	return rtn;
}

static void lost_possession()
{
	bHomeTeam = !bHomeTeam;
	PlatformSetInput(bHomeTeam);
}

static void InitGame()
{
	bHomeTeam = FALSE;
	PlatformSetInput(bHomeTeam);
	nHScore = 0;
	nVScore = 0;
	fGameTime = GAMETIME;
	nHalf = 0;
	bGameOver = FALSE;
	nSector = 5;
	bTeamMate = FALSE;
	bTeamMateSet = FALSE;
	bKickKey = FALSE;
	
	next_fsm(FSM_INVALID);
}

static void DrawBlips()
{
	static BOOL bBlink = FALSE;
	int x, y, nBright;

	for (int i=0; i<NUM_DEFENSEPLAYERS; i++){
		if (defense[i].nBright != BLIP_OFF){
			x = defense[i].nColumn;
			y = defense[i].nRow;
			nBright = defense[i].nBright;
			Blips[x][y].state = nBright;
		}
	}
	/* Player & receiver can overlap, then */
	/* print receiver first, then player   */
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

	if (goalkeeper.nBright != BLIP_OFF){
		x = goalkeeper.nColumn;
		y = goalkeeper.nRow;
		nBright = goalkeeper.nBright;
		Blips[x][y].state = nBright;
	}
	/* Ball must be last */
	if (ball.nBright != BLIP_OFF){
		x = ball.nColumn;
		y = ball.nRow;
		nBright = ball.nBright;
		Blips[x][y].state = nBright;
	}
	
	// draw the blips field
	for (y = 0; y < SOCCER2_BLIP_ROWS; y++){
		for (x = 0; x < SOCCER2_BLIP_COLUMNS; x++){
			switch(Blips[x][y].state){
			case BLIP_OFF:
			case BLIP_DIM:
			case BLIP_BRIGHT:
				Soccer2_DrawBlip(Blips[x][y].state, x, y);
				break;
			case BLIP_DIMBLINK:
				if (!bBlink){
					Soccer2_DrawBlip(BLIP_DIM, x, y);
				} else {
					Soccer2_DrawBlip(BLIP_OFF, x, y);
				}
				break;
			case BLIP_BRIGHTBLINK:
				if (!bBlink){
					Soccer2_DrawBlip(BLIP_BRIGHT, x, y);
				} else {
					Soccer2_DrawBlip(BLIP_OFF, x, y);
				}
				break;
			}
		}
	}
	
	Soccer2_DrawGoalareaFlag(IS_GOALAREA() ? TRUE : FALSE);

	bBlink = !bBlink;
}

static void EraseBlips()
{
	int i;
	
	// turn off all players
	UNSETPLAYER(&ball);
	UNSETPLAYER(&goalkeeper);
	UNSETPLAYER(&player);
	UNSETPLAYER(&receiver);
	for(i=0; i<NUM_DEFENSEPLAYERS; i++)
		UNSETPLAYER(&defense[i]);	
}

static void swap_player(PLAYER *p1, PLAYER *p2)
{
	PLAYER tmp;
	
	tmp = *p1;
	*p1 = *p2;
	*p2 = tmp;
	
	bTeamMate = FALSE;
	bTeamMateSet = FALSE;
	bKickKey = FALSE;
}

static void kickoff_scheme()
{
	static SCHEME_t ko[2] = {
		{ {3,2}, {3,3}, {{4,0}, {5,1}, {5,2}, {5,3}, {4,4}} },
		{ {3,2}, {3,1}, {{2,0}, {1,1}, {1,2}, {1,3}, {2,4}} }
	};
	SCHEME_t *ps;
	int i;
	
	ps = &ko[bHomeTeam];
	
	UNSETPLAYER(&ball);
	UNSETPLAYER(&goalkeeper);
	
	SETPLAYER(&player, ps->player.x, ps->player.y, BLIP_BRIGHT);
	SETPLAYER(&receiver, ps->receiver.x, ps->receiver.y, BLIP_DIMBLINK);
	for(i=0; i<NUM_DEFENSEPLAYERS; i++)
		SETPLAYER(&defense[i], ps->defense[i].x, ps->defense[i].y, BLIP_DIM);
}

static void corner_scheme()
{
	static SCHEME_t cl[2] = {
		{ {6,0}, {6,1}, {{4,0}, {4,1}, {3,2}, {4,3}, {4,4}} },
		{ {0,4}, {0,3}, {{2,0}, {2,1}, {3,2}, {2,3}, {2,4}} }
	};
	static SCHEME_t cr[2] = {
		{ {6,4}, {6,3}, {{4,0}, {4,1}, {3,2}, {4,3}, {4,4}} },
		{ {0,0}, {0,1}, {{2,0}, {2,1}, {3,2}, {2,3}, {2,4}} }

	};
	SCHEME_t *ps;
	int i;
	
	if(ball_out.x != 0 && ball_out.x != 6)
		return;
	
	if(ball_out.x == 0) {
		if(ball_out.y < 3)
			ps = &cr[1];
		else if(ball_out.y > 3)
			ps = &cl[1];
		else
			ps = ball_out.side ? &cr[1] : &cl[1];
	}
	else { /* x == 6 */
		if(ball_out.y < 3)
			ps = &cl[0];
		else if(ball_out.y < 3)
			ps = &cr[0];
		else
			ps = ball_out.side ? &cr[0] : &cl[0];
	}
		
	UNSETPLAYER(&ball);
	
	SETPLAYER(&player, ps->player.x, ps->player.y, BLIP_BRIGHT);
	SETPLAYER(&receiver, ps->receiver.x, ps->receiver.y, BLIP_DIMBLINK);
	for(i=0; i<NUM_DEFENSEPLAYERS; i++)
		SETPLAYER(&defense[i], ps->defense[i].x, ps->defense[i].y, BLIP_DIM);
	
	set_goalkeeper();
}

static void throwin_scheme()
{
	static SCHEME_t p[] = {
		/* player	receiver	0		1		2		3		4 */
		{ {3,4},    {3,3},     {{1,3},  {2,2},  {3,2},  {4,2},  {5,3}} },
		{ {2,4},    {2,3},     {{0,3},  {1,2},  {2,2},  {3,2},  {4,3}} },
		{ {1,4},    {1,3},     {{0,2},  {1,2},  {2,2},  {3,3},  {3,4}} },
		{ {0,4},    {0,3},     {{0,2},  {1,2},  {2,2},  {2,3},  {2,4}} },
		{ {3,0},    {3,1},     {{1,1},  {2,2},  {3,2},  {4,2},  {5,1}} },
		{ {2,0},    {2,1},     {{0,1},  {1,2},  {2,2},  {3,2},  {4,1}} },
		{ {1,0},    {1,1},     {{0,2},  {1,2},  {2,2},  {3,1},  {3,0}} },
		{ {0,0},    {0,1},     {{0,2},  {1,2},  {2,2},  {2,1},  {2,0}} },
		{ {6,4},    {6,3},     {{6,2},  {5,2},  {4,2},  {4,3},  {4,4}} },
		{ {5,4},    {5,3},     {{6,2},  {5,2},  {4,2},  {3,3},  {3,4}} },
		{ {4,4},    {4,3},     {{6,3},  {5,2},  {4,2},  {3,2},  {2,3}} },
		{ {6,0},    {6,1},     {{6,2},  {5,2},  {4,2},  {4,1},  {4,0}} },
		{ {5,0},    {5,1},     {{6,2},  {5,2},  {4,2},  {3,1},  {3,0}} },
		{ {4,0},    {4,1},     {{6,1},  {5,2},  {4,2},  {3,2},  {2,1}} },
	};
	int i;
	
	UNSETPLAYER(&ball);
	
	for(i=0; i<sizeof(p)/sizeof(*p); i++) {
		if(p[i].player.x == ball_out.x && p[i].player.y == ball_out.y) {
			int j;
			
			SETPLAYER(&player, p[i].player.x, p[i].player.y, BLIP_BRIGHT);
			SETPLAYER(&receiver,p[i].receiver.x , p[i].receiver.y, BLIP_DIMBLINK);
			for(j=0; j<NUM_DEFENSEPLAYERS; j++)
				SETPLAYER(&defense[j], p[i].defense[j].x, p[i].defense[j].y, BLIP_DIM);
		
			set_goalkeeper();
			break;
		}
	}
}

static void goalkick_scheme()
{
	static SCHEME_t ko[2] = {
		{ {0,2}, {2,2}, {{2,0}, {2,1}, {3,2}, {2,3}, {2,4}} },
		{ {6,2}, {4,2}, {{4,0}, {4,1}, {3,2}, {4,3}, {4,4}} }
	};
	SCHEME_t *ps;
	int i;
	
	ps = &ko[bHomeTeam];
	
	UNSETPLAYER(&ball);
	UNSETPLAYER(&goalkeeper);
	
	SETPLAYER(&player, ps->player.x, ps->player.y, BLIP_BRIGHT);
	SETPLAYER(&receiver, ps->receiver.x, ps->receiver.y, BLIP_DIMBLINK);
	for(i=0; i<NUM_DEFENSEPLAYERS; i++)
		SETPLAYER(&defense[i], ps->defense[i].x, ps->defense[i].y, BLIP_DIM);	
}

static void turnover_scheme()
{
	int i;
	
	UNSETPLAYER(&ball);
	
	/* Swap blips specularly to middle line */
	SETPLAYER(&player, SOCCER2_BLIP_COLUMNS - 1 - player.nColumn, player.nRow, BLIP_BRIGHT);
	SETPLAYER(&receiver, SOCCER2_BLIP_COLUMNS - 1 - receiver.nColumn, receiver.nRow, BLIP_DIMBLINK);
	for(i=0; i<NUM_DEFENSEPLAYERS; i++)
		SETPLAYER(&defense[i], SOCCER2_BLIP_COLUMNS - 1 - defense[i].nColumn, defense[i].nRow, BLIP_DIM);
}

/* PLAYER *p;		Player that make switch
 * int x;			Player column in the new scheme
 * int y;			Player line in the new scheme
 */
static void build_newsector(PLAYER *p, int x, int y)
{
	if(IS_GOALAREA())
		Soccer2_PlaySound(SOCCER2_SOUND_BEEP, PLAYSOUNDFLAGS_PRIORITY);	
	
	SETPLAYER(p, x, y, -1);
	set_goalkeeper();
}

static BOOL MOVEAT(const PLAYER *p, int dx, int dy, int *nx, int *ny)
{
	int x = 0;
	int y = 0;
	
	if(p->nColumn != dx) { 
		if(p->nColumn > dx && p->nColumn > 0)
			x--;
		else if(p->nColumn < dx && p->nColumn < SOCCER2_BLIP_COLUMNS-1)
			x++;
	}	
	
	if(p->nRow != dy) {
		if(p->nRow > dy && p->nRow > 0)
			y--;
		else if(p->nRow < dy && p->nRow < SOCCER2_BLIP_ROWS-1)
			y++;
	}
	
	*nx = p->nColumn + x;
	*ny = p->nRow + y;

	return TRUE;
}

static BOOL IS_ALIGNED(const PLAYER *p1, const PLAYER *p2, int *dx, int *dy)
{
	int x, y;
	
	x = p2->nColumn - p1->nColumn;
	y = p2->nRow - p1->nRow;
	
	if(dx != NULL)
		*dx = x;
	if(dy != NULL)
		*dy = y;
	
	if((x == 0) || (y == 0) || (x == y)) 
		return TRUE;
	
	return FALSE;
}

static int moveCounterInit()
{
	int rtn;
	
	switch(nLevel) {
	case LVL_ROOKIEFAST:
		rtn = 8 + Platform_Random(12);
		break;
	case LVL_PROSLOW:
		rtn = 4 + Platform_Random(8);
		break;
	case LVL_PROFAST:
		rtn = Platform_Random(4);
		break;
	case LVL_ROOKIESLOW:
	default:
		rtn = 12 + Platform_Random(16);
		break;
	}
	return rtn;
}

static enum FSM move_player(enum FSM curFsm)
{
	enum FSM newFsm = FSM_INVALID;
	BOOL bChange;
	BOOL moved = FALSE;
	BOOL newsect = FALSE;
	int col, row;
	PLAYER *p;
	
	p = (bTeamMate || curFsm == FSM_PASSING) ? &receiver : &player;
	
	NOTECURRENTPLAYERPOSITION(p);
	col = p->nColumn;
	row = p->nRow;
	
	if (Soccer2_GetInputLEFTUP(&bChange)) {
		if (bChange) {
			col -= 1;
			row -= 1;
			moved = TRUE;
		}	
	}
	else if (Soccer2_GetInputLEFTDOWN(&bChange)) {
		if (bChange) {
			col -= 1;
			row += 1;
			moved = TRUE;
		}	
	}
	else if (Soccer2_GetInputRIGHTUP(&bChange)) {
		if (bChange) {
			col += 1;
			row -= 1;
			moved = TRUE;
		}	
	}	
	else if (Soccer2_GetInputRIGHTDOWN(&bChange)) {
		if (bChange) {
			col += 1;
			row += 1;
			moved = TRUE;	
		}		
	}	
	else if (Soccer2_GetInputLEFT(&bChange)) {
		if (bChange) {
			col -= 1;
			moved = TRUE;
		}
	}
	else if (Soccer2_GetInputRIGHT(&bChange)) {
		if (bChange) {
			col += 1;
			moved = TRUE;
		}
	}
	else if (Soccer2_GetInputUP(&bChange)) {
		if (bChange) {
			row -= 1;
			moved = TRUE;
		}
	}
	else if (Soccer2_GetInputDOWN(&bChange)) {
		if (bChange) {
			row += 1;
			moved = TRUE;
		}
	}
	
	if(moved == FALSE)
		return newFsm;
	
	if(!VALID_COLUMN(col)) {
		moved = FALSE;
		
		/* Moves between sectors only if 'p' is the player and has the ball */
		if (!ISPLAYERENABLED(&ball) && (p == &player)) {
			if(col < 0) {
				if(nSector == 1) {
					SET_BALLOUT(p);
					newFsm = (!bHomeTeam) ? FSM_CORNERKICK : FSM_GOALKICK;
					lost_possession();
				}
				else if(nSector > 1) {
					nSector--;
					newsect = TRUE;
					col = SOCCER2_BLIP_COLUMNS - 1;
					moved = TRUE;
				}
			}
			else if(col >= SOCCER2_BLIP_COLUMNS) {
				if(nSector == 9) {
					SET_BALLOUT(p);
					newFsm = (!bHomeTeam) ? FSM_GOALKICK : FSM_CORNERKICK;
					lost_possession();
				}
				else if(nSector < 9) {
					nSector++;
					newsect = TRUE;
					col = 0;
					moved = TRUE;
				}
			}
		}
	}
	if (!VALID_ROW(row)) {
		moved = FALSE; 	/* Invalid move */
	}

	if(moved) {
		if(newsect)
			build_newsector(p, col, row);
		else if(!ISOCCUPIED(col, row) || 
				ISPLAYER(col,row) || ISRECEIVER(col,row)) {
			SETPLAYER(p, col, row, -1);
			if(bTeamMate) {
				bTeamMateSet = TRUE;
			}
		}
		else if(ISBALL(col,row)) {
			UNSETPLAYER(&ball);
			Soccer2_PlaySound(SOCCER2_SOUND_BOUNCE, PLAYSOUNDFLAGS_PRIORITY);
			if(p == &receiver) {
				swap_player(&player, &receiver);
			}
			SETPLAYER(&player, col, row, BLIP_BRIGHT);
			SETPLAYERBRIGHTNESS(&receiver, BLIP_DIMBLINK);
		}
		newFsm = curFsm;
	}
	
	return newFsm;
}

static BOOL count_down()
{
	// count down the clock
	if (Platform_IsNewSecond()){
		// check for end of quarter
		if (fGameTime > 0.0){
			fGameTime -= (int).1;
			if (fGameTime < 0.1){

				// end of quarter
				++nHalf;
				if (nHalf >= 2)
				{
					// game over
					bGameOver = TRUE;
					Soccer2_ClearScreen();
					Soccer2_PlaySound(SOCCER2_SOUND_ENDGAME, PLAYSOUNDFLAGS_PRIORITY);
					next_fsm(FSM_GAMEOVER);
					return FALSE;
				}

				bHomeTeam = (nHalf % 2) ? FALSE : TRUE;
				PlatformSetInput(bHomeTeam);

				fGameTime = GAMETIME;
				Soccer2_ClearScreen();
				Soccer2_PlaySound(SOCCER2_SOUND_ENDHALF, PLAYSOUNDFLAGS_PRIORITY);
				next_fsm(FSM_KICKOFF);
				return FALSE;
			}
		}
	}

	Soccer2_DrawSect(nSector, bHomeTeam);

	// make tick sounds
	if (fGameTime >= .1){
		if (--nTimerTickTimer <= 0){
			Soccer2_PlaySound(SOCCER2_SOUND_TICK, PLAYSOUNDFLAGS_ASYNC);
			nTimerTickTimer = TIME_TICKTIMER;
		}
	}
	return TRUE;
}

static BOOL move_receiver()
{
	int x, y;
	int dx, dy;
	
	if(!ISPLAYERENABLED(&receiver)) {
		return FALSE;
	}
	
	if(bTeamMate || bTeamMateSet) {
		return FALSE;
	}
	
	if(IS_ALIGNED(&receiver, &player, &dx, &dy)) {
		return FALSE;
	}
	
	if(receiver.nMoveNow-- > 0) {
		return FALSE;
	}
	
	x = receiver.nColumn;
	y = receiver.nRow;
	if(!bHomeTeam) {
		if(dx < 0 && x < (SOCCER2_BLIP_COLUMNS - 1))
			x++;
		else if(x < (SOCCER2_BLIP_COLUMNS - 1))
			x++;
	}
	else {
		if(dx > 0 && x > 0)
			x--;
		else if(x > 0)
			x--;
	}

	if(ISOCCUPIED(x, y)) {
		int newy;
		
		newy = move_tocol(&receiver, x, Platform_Random(2));
		if(newy >= 0) {
			y = newy;
		}
		else {	/* Try move only on y axis */
			x = receiver.nColumn;
			newy = move_tocol(&receiver, x, Platform_Random(2));
			if(newy >= 0) {
				y = newy;
			}
		}
	}
	
	if(!ISOCCUPIED(x, y)) {
		SETPLAYER(&receiver, x, y,  BLIP_DIMBLINK);
		receiver.nMoveNow = moveCounterInit();
		return TRUE;
	}
	
	receiver.nMoveNow = 0;	/* Try to move next time */
	return FALSE;
}

static void initDefense()
{
	int i;
	
	for(i=0; i < NUM_DEFENSEPLAYERS; i++) {
		defense[i].nMoveNow = moveCounterInit();
	}
}

static BOOL passing()
{
	int x, y;
	
	if (ISPLAYERENABLED(&ball)) 
		return FALSE;

	Soccer2_PlaySound((bKickKey) ? SOCCER2_SOUND_LOWKICK : SOCCER2_SOUND_HIGHKICK, PLAYSOUNDFLAGS_PRIORITY);
	
	y = player.nRow - receiver.nRow;
	x = player.nColumn - receiver.nColumn;
	
	if(abs(x) == abs(y)) {
		if(y > 0)
			nBallDirectionY = -1;
		else if(y < 0)
			nBallDirectionY = 1;
		else
			nBallDirectionY = 0;

		if(x > 0)
			nBallDirectionX = -1;
		else if(x < 0)
			nBallDirectionX = 1;
		else
			nBallDirectionX = 0;
	}
	else if(abs(x) > abs(y)) {
		nBallDirectionY = 0;
		if(x > 0)
			nBallDirectionX = -1;
		else if(x < 0)
			nBallDirectionX = 1;
	}
	else {
		nBallDirectionX = 0;
		if(y > 0)
			nBallDirectionY = -1;
		else if(y < 0)
			nBallDirectionY = 1;
	}
	
	SETPLAYER(&ball, player.nColumn, player.nRow, BLIP_BRIGHT);
	UNSETPLAYER(&player);
	
	nBallStep = 0;
	nBallMax = (bKickKey) ? BALL_HIGHKICK : BALL_LOWKICK;
	nTimerBallMove = TIME_BALLMOVE;
	return TRUE;
}

static BOOL shooting()
{
	if (ISPLAYERENABLED(&ball))
		return FALSE;
		
	Soccer2_PlaySound((bKickKey) ? SOCCER2_SOUND_LOWKICK : SOCCER2_SOUND_HIGHKICK, PLAYSOUNDFLAGS_PRIORITY);
	
	nBallDirectionX = (!bHomeTeam) ? 1 : -1;
	if(IS_ATTACKAREA()) {
		if(player.nRow > 2)
			nBallDirectionY = -1;
		else if(player.nRow < 2)
			nBallDirectionY = 1;
		else
			nBallDirectionY = 0;
	}
	else {
		nBallDirectionY = 0;
	}
	
	SETPLAYER(&ball, player.nColumn, player.nRow, BLIP_BRIGHT);
	
	UNSETPLAYER(&receiver);
	SETPLAYERBRIGHTNESS(&player, BLIP_DIMBLINK);
	
	nBallStep = 0;
	nBallMax = (bKickKey) ? BALL_HIGHKICK : BALL_LOWKICK;
	nTimerBallMove = TIME_BALLMOVE;
	return TRUE;
}

static int bounce(enum FSM curFsm)
{
	bBounce = TRUE;
	
	/* On sector 1 or 9, on shoot or passage it can be corner kick */
	if( IS_ATTACKAREA() ) {
		if( (curFsm == FSM_SHOOTING) && probability(50) ) {
			nBallDirectionX = (bHomeTeam) ? -1 : 1;
		}
		else if((curFsm == FSM_PASSING) && probability(25)) {
			nBallDirectionX = (bHomeTeam) ? -1 : 1;
		}
		else {
			nBallDirectionX = (bHomeTeam) ? 1 : -1;
		}
	}
	else {
		if((curFsm == FSM_PASSING) && probability(15)) 
			nBallDirectionX = (bHomeTeam) ? -1 : 1;
		else
			nBallDirectionX = (bHomeTeam) ? 1 : -1;
	}
	
	if(probability(20))
		nBallDirectionY = 0;
	else if(probability(40))
		nBallDirectionY = -1;
	else if(probability(40))
		nBallDirectionY = 1;
	
	nBallStep = 0;
	nBallMax = (Platform_Random(2)) ? BALL_HIGHKICK : BALL_LOWKICK;
	Soccer2_PlaySound(SOCCER2_SOUND_BOUNCE, PLAYSOUNDFLAGS_PRIORITY);
	
	SETPLAYERBRIGHTNESS(&player, BLIP_DIMBLINK);
	SETPLAYERBRIGHTNESS(&receiver, BLIP_DIMBLINK);
	nTimerBallMove = TIME_BALLMOVE;
	return 0;
}

static enum FSM move_defense(enum FSM curFsm)
{
	Coord_t gk = {0, 0};
	Coord_t df[NUM_DEFENSEPLAYERS];
	enum FSM newFsm = curFsm; 
	int i, nx, ny;
	
	gk.x = goalkeeper.nColumn;
	gk.y = goalkeeper.nRow;
	for(i=0; i<NUM_DEFENSEPLAYERS; i++) {
			df[i].x = defense[i].nColumn;
			df[i].y = defense[i].nRow;
	}
	
	if (ISPLAYERENABLED(&ball)) {
		if(ISPLAYERENABLED(&goalkeeper)) {
			/* if the ball is near goalkeeper, he takes it */
			if( ((!bHomeTeam && ball.nColumn == 6) || (bHomeTeam && ball.nColumn == 0)) &&
				( ball.nRow > 0 && ball.nRow < 4)) {
				gk.y = ball.nRow;
			}
		}
		for(i=0; i<NUM_DEFENSEPLAYERS; i++) {
			if(defense[i].nMoveNow-- <= 0) {
				if(MOVEAT(&defense[i], ball.nColumn, ball.nRow, &nx, &ny) == TRUE) {
					df[i].x = nx;
					df[i].y = ny;
				}
				defense[i].nMoveNow = moveCounterInit();
			}
		}
	}
	else {
		if(ISPLAYERENABLED(&goalkeeper)) {
			if(goalkeeper.nMoveNow-- <= 0) {
				goalkeeper.nMoveNow = moveCounterInit();
				if((!bHomeTeam && player.nColumn == 6) || (bHomeTeam && player.nColumn == 0)) {
					if(player.nRow < gk.y)
						gk.y--;
					else if(player.nRow > gk.y)
						gk.y++;
				}
				else if(player.nRow > 0 && player.nRow < 4) {			
					if((!bHomeTeam && player.nColumn == 5) || (bHomeTeam && player.nColumn == 1)) {	
						gk.x = player.nColumn;
						gk.y = player.nRow; 
					}
					else {
						gk.x = goalkeeper.nColumn;
						gk.y = player.nRow; 
					}	
				}
				else {
					if(player.nRow == 2)
						gk.y = 2;
					else
						gk.y = 2 + (player.nRow < 2) ? -1 : 1;
				}	
			}
		}	
		for(i=0; i<NUM_DEFENSEPLAYERS; i++) {
			if(defense[i].nMoveNow-- <= 0) {
				if(MOVEAT(&defense[i], player.nColumn, player.nRow, &nx, &ny) == TRUE) {
					df[i].x = nx;
					df[i].y = ny;
				}
				defense[i].nMoveNow = moveCounterInit();
			}
		}
	}
	
	if(ISPLAYERENABLED(&goalkeeper)) {
		if(ISBALL(gk.x, gk.y)) {
			SETPLAYER(&goalkeeper,gk.x, gk.y, BLIP_DIM);
			if(probability(75)) {
				UNSETPLAYER(&ball);
				lost_possession();
				newFsm = FSM_GOALKICK;
			}
			else {
				bounce(curFsm);
				newFsm = FSM_INPLAY;
			}
		}
		else if(!ISPLAYERENABLED(&ball) && ISPLAYER(gk.x,gk.y)) {
			if(probability(60)) {
				lost_possession();
				newFsm = FSM_GOALKICK;
			}
		}
		if(!ISOCCUPIED(gk.x, gk.y)) {
			SETPLAYER(&goalkeeper,gk.x, gk.y, BLIP_DIM);
		}
	}
	
	for(i=0; i<NUM_DEFENSEPLAYERS; i++) {
		if(ISBALL(df[i].x, df[i].y)) {
			SETPLAYER(&defense[i], df[i].x, df[i].y, BLIP_DIM);	
			if(probability(75)) {
				lost_possession();
				newFsm = FSM_TURNOVER;
			}
			else {
				bounce(curFsm);
				newFsm = FSM_INPLAY;
			}
		}
		/* a defensor can thieft the ball to the the captain? */  
		else if(IS_LVLPRO(nLevel) && !ISPLAYERENABLED(&ball) && ISPLAYER(df[i].x, df[i].y)) {
			if(probability(40)) {
				bounce(curFsm);
				newFsm = FSM_INPLAY;
			}
		}
		else if(!ISOCCUPIED(df[i].x, df[i].y))
			SETPLAYER(&defense[i], df[i].x, df[i].y, BLIP_DIM);	
	}
	
	return newFsm;
}

static enum FSM move_ball(enum FSM curFsm)
{
	if (ISPLAYERENABLED(&ball)) {	
		if (--nTimerBallMove <= 0) {
			Soccer2_PlaySound(SOCCER2_SOUND_LOOSEBALL, PLAYSOUNDFLAGS_ASYNC);
			nTimerBallMove = TIME_BALLMOVE;
		
			// ball stay
			if(IS_LVLPRO(nLevel) && (nBallStep >= nBallMax)) {
				SETPLAYERBRIGHTNESS(&player, BLIP_DIMBLINK);
				SETPLAYERBRIGHTNESS(&receiver, BLIP_OFF);
				bBounce = FALSE;
				return FSM_INPLAY;
			}
			
			// move the ball
			NOTECURRENTPLAYERPOSITION(&ball);
			ball.nColumn += nBallDirectionX;
			ball.nRow += nBallDirectionY;
			nBallStep++;

			/* Check if is off-screen */
			if (ISPLAYEROFFSCREEN(&ball)) {
				/* Choose throwin or goalkick */
				if(!VALID_COLUMN(ball.nColumn)) {
					if(nSector > 1 && nBallDirectionX < 0) {
						if(nBallDirectionY == 0) {
							nSector--;
							build_newsector(&ball, SOCCER2_BLIP_COLUMNS - 1, ball.nRow);
						}
						else {
							nBallStep = nBallMax;
							ball.nColumn = 0;
							ball.nRow = ball.nRowOld;
						}
					}
					else if(nSector < 9 && nBallDirectionX > 0) {
						if(nBallDirectionY == 0) {
							nSector++;
							build_newsector(&ball, 0, ball.nRow);
						}
						else {
							nBallStep = nBallMax;
							ball.nColumn = 6;
							ball.nRow = ball.nRowOld;
						}
					}
					else {
						SET_BALLOUT(&ball);
						UNSETPLAYER(&ball);					
						if(!bBounce) {
							lost_possession();
							return FSM_GOALKICK;
						}
						bBounce = FALSE;
						return FSM_CORNERKICK;
					}		
				}
				if(!VALID_ROW(ball.nRow)) {
					SET_BALLOUT(&ball);
					UNSETPLAYER(&ball);
					if(!bBounce) {
						lost_possession();
					}
					else {
						bBounce = FALSE;
					}
					return FSM_THROWIN;
				}
			}
			/* Check if goal */
			if(((fsm == FSM_SHOOTING) || (fsm == FSM_PASSING)) && 
			   ((nSector == 1 && ball.nColumn == 0 && ball.nRow == 2) ||
				(nSector == 9 && ball.nColumn == 6 && ball.nRow == 2))) {
				bBounce = FALSE;
				if(bKickKey == TRUE) {
					if(ISGOALKEEPER(ball.nColumn,ball.nRow)) {
						if(probability(70)) {
							SET_BALLOUT(&ball);
							lost_possession();
							return FSM_GOALKICK;
						}
					}
					else if(ISDEFENSE(ball.nColumn,ball.nRow)) {
						if(probability(20)) {
							bounce(curFsm);
							return FSM_INPLAY;
						}
						return FSM_GOAL;
					}
					else if(probability(30)) {
						SET_BALLOUT(&ball);
						bBounce = FALSE;
						lost_possession();
						return FSM_GOALKICK;
					}
					return FSM_GOAL;
				}
				if (ISGOALKEEPER(ball.nColumn, ball.nRow)) {
					lost_possession();
					return FSM_GOALKICK;
				}
				return FSM_GOAL;	
			}
	
			/* non goal */
			if( (bKickKey == FALSE) ||
			    (bKickKey == TRUE && (nBallStep == 1 || nBallStep == nBallMax)) ) {	
				/* Check if bounced by defender or goalkeeper */
				if (ISGOALKEEPER(ball.nColumn, ball.nRow)) {
					if(curFsm == FSM_PASSING || probability(50)) {
						SET_BALLOUT(&ball);
						bBounce = FALSE;
						lost_possession();
						return FSM_GOALKICK;
					}
					bounce(curFsm);
					return FSM_INPLAY;
				}
				if (ISDEFENSE(ball.nColumn, ball.nRow)) {				
					if(curFsm == FSM_PASSING) {
						bounce(curFsm);
						return FSM_INPLAY;
					}
					if(probability(50)) {
						lost_possession();
						return FSM_TURNOVER;
					}
					bounce(curFsm);
					return FSM_INPLAY;
				}
				/* Check if receiver has gained possession */
				if (ISRECEIVER(ball.nColumn, ball.nRow)) {				
					UNSETPLAYER(&ball);
					bBounce = FALSE;
					Soccer2_PlaySound(SOCCER2_SOUND_BOUNCE, PLAYSOUNDFLAGS_PRIORITY);
					/* Swap player and receiver */
					swap_player(&player, &receiver);
					
					SETPLAYERBRIGHTNESS(&player, BLIP_BRIGHT);
					SETPLAYERBRIGHTNESS(&receiver, BLIP_DIMBLINK);
					return FSM_INPLAY;	
				}
			}
			
			/* Check if player has regained possession */
			if (ISPLAYER(ball.nColumn, ball.nRow)) {
				UNSETPLAYER(&ball);
				bBounce = FALSE;
				Soccer2_PlaySound(SOCCER2_SOUND_BOUNCE, PLAYSOUNDFLAGS_PRIORITY);
				SETPLAYERBRIGHTNESS(&player, BLIP_BRIGHT);
				SETPLAYERBRIGHTNESS(&receiver, BLIP_DIMBLINK);
				return FSM_INPLAY;	
			}
		}
	}
	else {
		bBounce = FALSE;
	}
	return curFsm;
}

int Soccer2_GetPower()
{
	return (bPower ? TRUE : FALSE);
}

void Soccer2_PowerOn()
{
	if (!bPower)
	{
		InitGame();
		bPower = TRUE;
	}
}

void Soccer2_PowerOff()
{
	bPower = FALSE;
	Soccer2_StopSound();
}

void Soccer2_SetSkill(int i)
{
	nLevel = i;
}

int Soccer2_GetSkill()
{
	return nLevel;
}

void Soccer2_Run(int tu)
{
	int x, y;
	
	// prevent reentrancy
	if (bInFrame){ 
		return; 
	}
	bInFrame = TRUE;

	// turn off all blips
	for (y = 0; y < SOCCER2_BLIP_ROWS; y++){
		for (x = 0; x < SOCCER2_BLIP_COLUMNS; x++){
			Blips[x][y].state = BLIP_OFF;
		}
	}

	if (!bPower){
		Soccer2_ClearScreen();
		bInFrame = FALSE;
		return;
	}

	Platform_StartDraw();

	// run the game
	(fsmfcn[fsm])();

	DrawBlips();
	
	Platform_EndDraw();

	bInFrame = FALSE;
}

// FINITE STATE MACHINE STUFF 
static void fsmWaitScorePress()
{
	static BOOL pressed = FALSE;
	BOOL bChange;
	
	if(nextFsm == FSM_INVALID) {
		kickoff_scheme();
		nextFsm = FSM_KICKOFF;
	}
	
	if(pressed == FALSE) {
		Soccer2_DrawSect(nSector, bHomeTeam);
		if (Soccer2_GetInputSCORE(&bChange)) {
			if (bChange) {
				pressed = TRUE;
				nStatsIndex = 0;
				nTimerStatsDisplay = TIME_STATSDISPLAY;
				if(nextFsm != FSM_KICKOFF)
					EraseBlips();
			}
		}
	}
	else {
		if (--nTimerStatsDisplay <= 0)
		{
			++nStatsIndex;
			nTimerStatsDisplay = TIME_STATSDISPLAY;
		}
		switch(nStatsIndex) {
		case 0:
			Soccer2_DrawStat(-1);
			break;
		case 1:
			Soccer2_DrawStat(nVScore);
			break;
		case 2:
			Soccer2_DrawStat(-1);
			break;
		case 3:
			Soccer2_DrawStat(nHScore);
			break;
		case 4:
			Soccer2_DrawStat(-1);
			break;
		case 5:
			Soccer2_DrawStat((int)fGameTime);
			break;
		case 6:
			Soccer2_DrawStat(-1);
			break;			
		case 7:
			Soccer2_DrawSect(nSector, bHomeTeam);
			break;
		default:
			initDefense();
			receiver.nMoveNow = moveCounterInit();
			pressed = FALSE;
			bTeamMate = FALSE;
			bTeamMateSet = FALSE;
			bKickKey = FALSE;
			bBounce = FALSE;
			fsm = nextFsm;
			nextFsm = FSM_INVALID;
			switch(fsm) {
			case FSM_KICKOFF:
				kickoff_scheme();
				break;
			case FSM_CORNERKICK:
				corner_scheme();
				break;
			case FSM_GOALKICK:
				goalkick_scheme();
				break;
			case FSM_THROWIN:
				throwin_scheme();
				break;
			case FSM_TURNOVER:
				turnover_scheme();
			default:
				break;
			}
			break;
		}
	}
}

static void fsmKickoff()
{
	BOOL bChange;
	
	kickoff_scheme();

	if(Soccer2_GetInputPASS(&bChange)) {
		if (bChange) {
			Platform_IsNewSecond();
			if(passing() == TRUE) {
				next_fsm(FSM_PASSING);
			}
		}
	}
	
	Soccer2_DrawSect(nSector, bHomeTeam);
}

static void fsmTurnover()
{
	enum FSM newFsm;
		
	newFsm = move_player(fsm);
	if(newFsm != FSM_INVALID) {
		Platform_IsNewSecond();
		next_fsm(FSM_INPLAY);
	}
}

static void fsmThrowin()
{				
	BOOL bChange;
		
	if(Soccer2_GetInputPASS(&bChange)) {
		if (bChange) {
			Platform_IsNewSecond();
			if(passing() == TRUE) {
				next_fsm(FSM_PASSING);
			}
		}
	}
	if(IS_LVLPRO(nLevel)) {
		if (Soccer2_GetInputTEAM(&bChange)) {
			if (bChange) {
				bTeamMate = !bTeamMate;
				if(bTeamMate == TRUE)
					Soccer2_PlaySound(SOCCER2_SOUND_BEEP, PLAYSOUNDFLAGS_PRIORITY);
			}
		}
		if (Soccer2_GetInputSCORE(&bChange)) {
			if (bChange) {
				bTeamMateSet = FALSE;
			}
		}
		/* Can move the receiver */
		if(bTeamMate == TRUE) {
			move_player(FSM_THROWIN);
		}
	}	
	Soccer2_DrawSect(nSector, bHomeTeam);
}

static void fsmGoalKick()
{
	BOOL bChange;
		
	if(Soccer2_GetInputPASS(&bChange)) {
		if (bChange) {
			Platform_IsNewSecond();
			if(passing() == TRUE) {
				next_fsm(FSM_PASSING);
			}
		}
	}
	
	Soccer2_DrawSect(nSector, bHomeTeam);
}

static void fsmCornerKick()
{
	BOOL bChange;
		
	if(Soccer2_GetInputPASS(&bChange)) {
		if (bChange) {
			Platform_IsNewSecond();
			if(passing() == TRUE) {
				next_fsm(FSM_PASSING);
			}
		}
	}
	if(IS_LVLPRO(nLevel)) {
		if (Soccer2_GetInputTEAM(&bChange)) {
			if (bChange) {
				bTeamMate = !bTeamMate;
				if(bTeamMate == TRUE)
					Soccer2_PlaySound(SOCCER2_SOUND_BEEP, PLAYSOUNDFLAGS_PRIORITY);
			}
		}
		if (Soccer2_GetInputSCORE(&bChange)) {
			if (bChange) {
				bTeamMateSet = FALSE;
			}
		}
		/* Can move the receiver */
		if(bTeamMate == TRUE) {
			move_player(FSM_CORNERKICK);
		}
	}
	
	Soccer2_DrawSect(nSector, bHomeTeam);
}

static void fsmInPlay()
{
	enum FSM newFsm;
	BOOL bChange;
	
	newFsm = move_player(fsm);
	if(newFsm == FSM_INVALID) {
		if (Soccer2_GetInputSHOOT(&bChange)) {
			if (bChange) {
				if(shooting() == TRUE) {
					next_fsm(FSM_SHOOTING);
					return;
				}
			}
		}
		
		if (Soccer2_GetInputPASS(&bChange)) {
			if (bChange) {
				if(passing() == TRUE) {
					next_fsm(FSM_PASSING);
					return;
				}
			}
		}
		
		if(IS_LVLPRO(nLevel)) {
			if (Soccer2_GetInputTEAM(&bChange)) {
				if (bChange) {
					bTeamMate = !bTeamMate;
					if(bTeamMate == TRUE)
						Soccer2_PlaySound(SOCCER2_SOUND_BEEP, PLAYSOUNDFLAGS_PRIORITY);
				}
			}
			if (Soccer2_GetInputSCORE(&bChange)) {
				if (bChange) {
					bTeamMateSet = FALSE;
				}
			}
			if (Soccer2_GetInputKICK(&bChange)) {
				if (bChange) {
					bKickKey = !bKickKey;
					Soccer2_PlaySound((bKickKey) ? SOCCER2_SOUND_LOWKICK : SOCCER2_SOUND_HIGHKICK, PLAYSOUNDFLAGS_PRIORITY);
				}
			}
		}
	}
	else if(newFsm != FSM_INPLAY) {
		if(!FSM_RUNNING(newFsm))
			Soccer2_PlaySound(SOCCER2_SOUND_BALLOUT, PLAYSOUNDFLAGS_PRIORITY);
		next_fsm(newFsm);
		return;
	}
	
	// move the receiver
	move_receiver();
		
	// move defenders
	newFsm = move_defense(fsm);
	if(!FSM_RUNNING(newFsm))
		Soccer2_PlaySound(SOCCER2_SOUND_BALLOUT, PLAYSOUNDFLAGS_PRIORITY);
	next_fsm(newFsm);
	
	// count down the clock
	count_down();
}

static void fsmPassing()
{
	enum FSM newFsm;
	BOOL bChange;
	
	newFsm = move_player(fsm);
	if(newFsm == FSM_INVALID) {
		if (Soccer2_GetInputKICK(&bChange) && IS_LVLPRO(nLevel)) {
			if (bChange) {
				bKickKey = !bKickKey;
				Soccer2_PlaySound((bKickKey) ? SOCCER2_SOUND_LOWKICK : SOCCER2_SOUND_HIGHKICK, PLAYSOUNDFLAGS_PRIORITY);
			}
		}
	}
	else if(newFsm != FSM_PASSING) {
		if(!FSM_RUNNING(newFsm))
			Soccer2_PlaySound(SOCCER2_SOUND_BALLOUT, PLAYSOUNDFLAGS_PRIORITY);
		next_fsm(newFsm);
		return;
	}
	// move defenders
	if(bKickKey) {
		newFsm = move_defense(fsm);
		if(!FSM_RUNNING(newFsm))
			Soccer2_PlaySound(SOCCER2_SOUND_BALLOUT, PLAYSOUNDFLAGS_PRIORITY);
		next_fsm(newFsm);
	}
	// move the ball
	if(fsm == FSM_PASSING) {
		newFsm = move_ball(fsm);
		if(!FSM_RUNNING(newFsm))
			Soccer2_PlaySound(SOCCER2_SOUND_BALLOUT, PLAYSOUNDFLAGS_PRIORITY);
		next_fsm(newFsm);
	}
	
	// count down the clock
	count_down();
}

static void fsmShooting()
{
	enum FSM newFsm;
	BOOL bChange;
	
	newFsm = move_player(fsm);
	if(newFsm == FSM_INVALID) {
		if (Soccer2_GetInputKICK(&bChange) && IS_LVLPRO(nLevel)) {
			if (bChange) {
				bKickKey = !bKickKey;
				Soccer2_PlaySound((bKickKey) ? SOCCER2_SOUND_LOWKICK : SOCCER2_SOUND_HIGHKICK, PLAYSOUNDFLAGS_PRIORITY);
			}
		}	
	}
	else if(newFsm != FSM_SHOOTING) {
		if(!FSM_RUNNING(newFsm))
			Soccer2_PlaySound(SOCCER2_SOUND_BALLOUT, PLAYSOUNDFLAGS_PRIORITY);
		next_fsm(newFsm);
		return;
	}
	
	// move defenders
	if(bKickKey) {
		newFsm = move_defense(fsm);
		if(!FSM_RUNNING(newFsm))
			Soccer2_PlaySound(SOCCER2_SOUND_BALLOUT, PLAYSOUNDFLAGS_PRIORITY);
		next_fsm(newFsm);
	}
	// move the ball
	if(fsm == FSM_SHOOTING) {
		newFsm = move_ball(fsm);
		if(!FSM_RUNNING(newFsm))
			Soccer2_PlaySound(SOCCER2_SOUND_BALLOUT, PLAYSOUNDFLAGS_PRIORITY);	
		next_fsm(newFsm);
	}

	// count down the clock
	count_down();
}

static void fsmGameOver() 
{
	next_fsm(FSM_GAMEOVER);
}

static void fsmGoal()
{
	Soccer2_PlaySound(SOCCER2_SOUND_GOAL, PLAYSOUNDFLAGS_PRIORITY);
	if(!bHomeTeam)
		nVScore++;
	else
		nHScore++;
	nSector = 5;
	lost_possession();
	
	next_fsm(FSM_KICKOFF);
}

#define LINE_STEP	20
void Soccer2_Debug(int f)
{
	int w, h;
	int y = 0;
	Soccer2_GetSize(&w, &h);
	
	debugPrintf(realx(w)+10, realy(y), 0xFFFFFFFF, "level  =%d", nLevel);
	y += LINE_STEP;
	debugPrintf(realx(w)+10, realy(y), 0xFFFFFFFF, "fsm    =%d (%d)", fsm, FSM_RUNNING(fsm));
	y += LINE_STEP;
	debugPrintf(realx(w)+10, realy(y), 0xFFFFFFFF, "nx fsm =%d (%d)", nextFsm, FSM_RUNNING(nextFsm));
	y += LINE_STEP;
	debugPrintf(realx(w)+10, realy(y), 0xFFFFFFFF, "bnc=%d kk=%d", 
		bBounce, bKickKey);
	y += LINE_STEP;
	debugPrintf(realx(w)+10, realy(y), 0xFFFFFFFF, "tm=%d tms=%d", 
		bTeamMate, bTeamMateSet);
	y += LINE_STEP;
	debugPrintf(realx(w)+10, realy(y), 0xFFFFFFFF, "trc =%d", trc_debug);
	y += LINE_STEP;
	debugPrintf(realx(w)+10, realy(y), 0xFFFFFFFF, "ball =%d (%d,%d)(%d,%d)", 
		ball.nBright, ball.nColumn, ball.nRow, ball.nColumnOld, ball.nRowOld);
	y += LINE_STEP;
	debugPrintf(realx(w)+10, realy(y), 0xFFFFFFFF, "play =%d (%d,%d)(%d,%d)", 
		player.nBright, player.nColumn, player.nRow, player.nColumnOld, player.nRowOld);
	y += LINE_STEP;
	debugPrintf(realx(w)+10, realy(y), 0xFFFFFFFF, "recv =%d (%d,%d)(%d,%d)", 
		receiver.nBright, receiver.nColumn, receiver.nRow, receiver.nColumnOld, receiver.nRowOld);
	y += LINE_STEP;
}
