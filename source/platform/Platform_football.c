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

#include "football.h"
#include "football_screen_png.h"
#include "football_tick_raw.h"
#include "football_endplay_raw.h"
#include "football_endpossession_raw.h"
#include "football_score_raw.h"

// images
static GRRLIB_texImg *bmpScreen;

static Sound_t tcWaveRes[FOOTBALL_SOUND_NSOUDS];
static Blip_t blip[FOOTBALL_BLIP_COLUMNS][FOOTBALL_BLIP_ROWS];
static Digit_t digit[7] = { 
	{football_digit_x, football_digit_y, -1},
	{football_digit_x + football_digit_spacing, football_digit_y, -1, -1},
	{football_digit_x + (4 * football_digit_spacing), football_digit_y, -1, -1},
	{football_digit_x + (5 * football_digit_spacing), football_digit_y, -1, -1},
	{football_digit_x + (6 * football_digit_spacing), football_digit_y, -1, -1},
	{football_digit_x + (9 * football_digit_spacing), football_digit_y, -1, -1},
	{football_digit_x + (10 * football_digit_spacing), football_digit_y, -1, -1}
};
static Help_t help[] = {
	{ WK_2, { 74, 205} },
	{ WK_MINUS, { 30, 230} },
	{ WK_PLUS, { 30, 184} },
	{ WK_BLR, { 92, 285 } },
	{ WK_UD, { 187, 201 } },
	{ WK_LR, { 140, 201 } }
};
//----------------------------------------------------------------------------
//
//
void Football_Help()
{
	Platform_Help(help, sizeof(help)/sizeof(*help));
}

static BOOL bInited = FALSE;

void Football_Init()
{
	int x, y;
	
	if (bInited) 
		return;

	// Init sounds	
	Sound_set(&tcWaveRes[FOOTBALL_SOUND_TICK], football_tick_raw, football_tick_raw_size, 25);
	Sound_set(&tcWaveRes[FOOTBALL_SOUND_ENDPLAY], football_endplay_raw, football_endplay_raw_size, 489);
	Sound_set(&tcWaveRes[FOOTBALL_SOUND_ENDPOSSESSION], football_endpossession_raw, football_endpossession_raw_size, 984);
	Sound_set(&tcWaveRes[FOOTBALL_SOUND_SCORE], football_score_raw, football_score_raw_size, 1335);

	// load images
	bmpScreen = GRRLIB_LoadTexture(football_screen_png);

	// init blips positions
	for (y = 0; y < FOOTBALL_BLIP_ROWS; y++){
		for (x = 0; x < FOOTBALL_BLIP_COLUMNS; x++){
			Blip_t *pblip = &blip[x][y];
			
			pblip->x = (int)((x * ((float)football_blip_xspacing/100)) + football_blip_x);
			pblip->y = (int)((y * ((float)football_blip_yspacing/100)) + football_blip_y);
			pblip->status = -1;
		}
	}	
	// clean digit 
	for (x=0; x<7; x++) {
		digit[x].val = -1;
		digit[x].type = DIGIT_TYPE_NORMAL;
	}

	PlatformSetInput(0);
	// turn on the game
	Football_SetSkill(0);
	Football_PowerOn();

	bInited = TRUE;
}

void Football_StopSound()
{
	// stop all sounds...
	Platform_StopSound();
}

void Football_DeInit()
{
	// stop all sounds...
	Football_StopSound();
	Football_PowerOff();
	GRRLIB_FreeTexture(bmpScreen);
	bInited = FALSE;
}

void Football_Paint()
{	
	int x, y;
	BOOL power = Football_GetPower();
	BOOL skill = Football_GetSkill();
	int p_switch;
	p_switch = Platform_GetPowerSwitch(ONOFF_1OFF2);
	if(p_switch == -1) {
		if(!power) {
			Football_PowerOn();
			Football_SetSkill(0);
		}
		else if(power && skill == 1) {
			Football_PowerOff();
		}
	}
	else if(p_switch == 1) {
		if(!power) {
			Football_PowerOn();
			Football_SetSkill(1);
		}
		else if(power && skill == 0) {
			Football_PowerOff();
		}
	}
	
	// paint the backdrop
	GRRLIB_DrawImg(realx(0), realy(0), bmpScreen, 0, SCALE_X, SCALE_Y, 0xffffffff);
	// visualize the control states
	if (power){
		if (skill == 0){
			draw_switch(SWITCH_TYPE_3, football_pro_1_x, football_pro_1_y, SWITCH_POS_PRO1);
		} else {
			draw_switch(SWITCH_TYPE_3, football_pro_1_x, football_pro_1_y, SWITCH_POS_PRO2);
		}

		for (y = 0; y < FOOTBALL_BLIP_ROWS; y++){
			for (x = 0; x < FOOTBALL_BLIP_COLUMNS; x++){
				if(blip[x][y].status != -1)
					draw_oblip(blip[x][y].x, blip[x][y].y, blip[x][y].status);
			}
		}
		for(x = 0; x < 7; x++) {
			draw_digit_f(digit[x].x, digit[x].y, digit[x].val, digit[x].type);
		}
	} 
	else {
		draw_switch(SWITCH_TYPE_3, football_pro_1_x, football_pro_1_y, SWITCH_POS_OFF);
	}
	
}

void Football_ClearScreen()
{
	int x, y;
	
	Platform_StartDraw();

	// erase the blips
	for (y = 0; y < FOOTBALL_BLIP_ROWS; y++){
		for (x = 0; x < FOOTBALL_BLIP_COLUMNS; x++){
			Football_DrawBlip(BLIP_OFF, x, y);
		}
	}
	// erase the stat display
	Football_DrawScores(-1, -1);
	Football_DrawTime(-1);

	Platform_EndDraw();
}

void Football_PlaySound(int nSound, unsigned int nFlags)
{
	Platform_PlaySound(&tcWaveRes[nSound], nFlags);
}

//----------------------------------------------------------------------------
// local fcn's
//

void Football_DrawBlip(int nBright, int x, int y)
{
	Blip_t *pblip = &blip[x][y];
	
	switch(nBright){
	case BLIP_OFF:
		pblip->status = -1;
		break;
	case BLIP_DIM:
		pblip->status = BLIP_TYPE_NORMAL;
		break;
	case BLIP_BRIGHT:
		pblip->status = BLIP_TYPE_BRIGHT;
		break;
	}
}

void Football_DrawScores(int nHScore, int nVScore)
{
	// draw home team score
	if (nHScore == -1){
		// erase the time
		digit[0].val = -1;
		digit[0].type = DIGIT_TYPE_NORMAL;
		digit[1].val = -1;
		digit[1].type = DIGIT_TYPE_NORMAL;
	} else {

		// draw 10s place
		if ((nHScore/10) != 0){
			digit[0].val = nHScore/10;
		} else {
			digit[0].val = -1;
		}
		digit[0].type = DIGIT_TYPE_NORMAL;

		// draw 1s place
		digit[1].val = nHScore%10;
		digit[1].type = DIGIT_TYPE_NORMAL;
	}

	// draw visitor team score
	if (nVScore == -1){
		// erase the time
		digit[5].val = -1;
		digit[5].type = DIGIT_TYPE_NORMAL;
		digit[6].val = -1;
		digit[6].type = DIGIT_TYPE_NORMAL;
	} else {
		// draw 10s place
		if ((nVScore/10) != 0){
			digit[5].val = nVScore/10;
		} else {
			digit[5].val = -1;
		}
		digit[5].type = DIGIT_TYPE_NORMAL;
		// draw 1s place
		digit[6].val = nVScore%10;
		digit[6].type = DIGIT_TYPE_NORMAL;
	}
}

void Football_DrawDown(int nDown, int nYards)
{
	// draw downs
	if (nDown == -1){
		// erase the down
		digit[0].val = -1;
		digit[0].type = DIGIT_TYPE_NORMAL;
		digit[1].val = -1;
		digit[1].type = DIGIT_TYPE_NORMAL;
	} else {
		++nDown;

		// draw 10s place
		if (nDown >= 10){
			digit[0].val = nDown/10;
		} else {
			digit[0].val = -1;
		}
		digit[0].type = DIGIT_TYPE_NORMAL;

		// draw 1s place
		digit[1].val = nDown%10;
		digit[1].type = DIGIT_TYPE_NORMAL;
	}

	// draw yards
	if (nYards <= 0){
		// erase the yards
		digit[5].val = -1;
		digit[5].type = DIGIT_TYPE_NORMAL;
		digit[6].val = -1;
		digit[6].type = DIGIT_TYPE_NORMAL;
	} else {
		// draw 10s place
		if (nYards >= 10){
			digit[5].val = nYards/10;
		} else {
			digit[5].val = -1;
		}
		digit[5].type = DIGIT_TYPE_NORMAL;

		// draw 1s place
		digit[6].val = nYards%10;
		digit[6].type = DIGIT_TYPE_NORMAL;
	}
}

void Football_DrawTime(float fTime)
{
	int i;
	
	// erase the time
	for(i=2; i<5; i++) {
		digit[i].val = -1;
		digit[i].type = DIGIT_TYPE_NORMAL;
	}

	// draw time display
	if (fTime == 0.0) {
		// zero time is displayed as '00'
		digit[2].val = -1;
		digit[3].val = 0;
		digit[4].val = 0;
	} 
	else if (fTime > 0.0) {

		int nTime = (int)(fTime + .1); // add .1 for rounding error

		// draw 10s place
		if ((nTime/10) != 0){
			digit[2].val = nTime/10;
		} 
		else {
			digit[2].val = -1;
		}

		// draw 1s place (and decimal)
		digit[3].val = nTime%10;
		digit[3].type = DIGIT_TYPE_FLOAT;

		// draw fractional portion
		char lpszTime[16];
		sprintf(lpszTime, "%.01f", fTime);
		for (int i=0; lpszTime[i]; i++){
			if (lpszTime[i] == '.'){
				digit[4].val = lpszTime[i+1] - '0';
				break;
			}
		}
	}
}

void Football_DrawYard(int nYard)
{
	int sign;
	int val, idx;

	// convert display to proper format
	if (nYard >= 50){
		sign = 1;
		val = 100 - nYard;
	} else {
		sign = -1;
		val = nYard;
	}

	// draw yard
	idx = 2;

	if (sign == 1){
		// draw direction on left
		digit[idx].val = 1;
		digit[idx].type = DIGIT_TYPE_SPECIAL;
		idx++;
	}

	// draw 10s place
	digit[idx].val = val/10;
	digit[idx].type = DIGIT_TYPE_NORMAL;
	idx++;
	
	digit[idx].val = val%10;
	digit[idx].type = DIGIT_TYPE_NORMAL;
	idx++;

	if (sign == -1){
		// draw direction on right
		digit[idx].val = 0;
		digit[idx].type = DIGIT_TYPE_SPECIAL;
	}
}

BOOL Football_GetInputKICK(BOOL *pChange)
{
	static BOOL bLast = FALSE;

	if (pChange){ *pChange = FALSE; }

	// check the keys
	if (Platform_GetInput2())
	{
		if (!bLast && pChange)
		{
			*pChange = TRUE;
		}
		bLast = TRUE;
		return TRUE;
	}

	bLast = FALSE;

	return FALSE;
}

BOOL Football_GetInputSTATUS(BOOL *pChange)
{
	static BOOL bLast = FALSE;

	if (pChange){ *pChange = FALSE; }

	// check the keys
	if (Platform_GetInputPLUS())
	{
		if (!bLast && pChange)
		{
			*pChange = TRUE;
		}
		bLast = TRUE;
		return TRUE;
	}

	bLast = FALSE;

	return FALSE;
}

BOOL Football_GetInputSCORE(BOOL *pChange)
{
	static BOOL bLast = FALSE;

	if (pChange){ *pChange = FALSE; }

	// check the keys
	if (Platform_GetInputMINUS())
	{
		if (!bLast && pChange)
		{
			*pChange = TRUE;
		}
		bLast = TRUE;
		return TRUE;
	}

	bLast = FALSE;

	return FALSE;
}

BOOL Football_GetInputUP(BOOL *pChange)
{
	static BOOL bLast = FALSE;

	if (pChange){ *pChange = FALSE; }

	// check the keys
	if (Platform_GetInputUP())
	{
		if (!bLast && pChange)
		{
			*pChange = TRUE;
		}
		bLast = TRUE;
		return TRUE;
	}

	bLast = FALSE;

	return FALSE;
}

BOOL Football_GetInputRUN(BOOL *pChange)
{
	static BOOL bLast = FALSE;

	if (pChange){ *pChange = FALSE; }

	// check the keys
	if (Platform_GetInputRIGHT()
		|| Platform_GetInputLEFT())
	{
		if (!bLast && pChange)
		{
			*pChange = TRUE;
		}
		bLast = TRUE;
		return TRUE;
	}

	bLast = FALSE;

	return FALSE;
}

BOOL Football_GetInputDOWN(BOOL *pChange)
{
	static BOOL bLast = FALSE;

	if (pChange){ *pChange = FALSE; }

	// check the keys
	if (Platform_GetInputDOWN())
	{
		if (!bLast && pChange)
		{
			*pChange = TRUE;
		}
		bLast = TRUE;
		return TRUE;
	}

	bLast = FALSE;

	return FALSE;
}

BOOL Football_TestForMovement()
{
	// check the keys
	if (Platform_GetInputRIGHT()
		|| Platform_GetInputLEFT()
		|| Platform_GetInputUP()
		|| Platform_GetInputDOWN())
	{
		return TRUE;
	}

	return FALSE;
}

void Football_GetSize(int *w, int *h)
{
	*w = bmpScreen->w;
	*h = bmpScreen->h;
}
