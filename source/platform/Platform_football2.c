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

#include "football2.h"
#include "football2_screen_png.h"
#include "football2_chargestart_raw.h"
#include "football2_charge_raw.h"
#include "football2_tick_raw.h"
#include "football2_runback_raw.h"
#include "football2_firstdown_raw.h"
#include "football2_endplay_raw.h"
#include "football2_endpossession_raw.h"
#include "football2_endquarter_raw.h"
#include "football2_touchdown_raw.h"
#include "football2_safety_raw.h"
	
// images
static GRRLIB_texImg *bmpScreen;

static Sound_t tcWaveRes[FOOTBALL2_SOUND_NSOUNDS];
static Blip_t blip[FOOTBALL2_BLIP_COLUMNS][FOOTBALL2_BLIP_ROWS];
static Digit_t digit[7] = { 
	{football2_digit_x, football2_digit_y, -1},
	{football2_digit_x + football2_digit_spacing, football2_digit_y, -1, -1},
	{football2_digit_x + (4 * football2_digit_spacing), football2_digit_y, -1, -1},
	{football2_digit_x + (5 * football2_digit_spacing), football2_digit_y, -1, -1},
	{football2_digit_x + (6 * football2_digit_spacing), football2_digit_y, -1, -1},
	{football2_digit_x + (9 * football2_digit_spacing), football2_digit_y, -1, -1},
	{football2_digit_x + (10 * football2_digit_spacing), football2_digit_y, -1, -1}
};

static Help_t help[] = {
	{ WK_2, { 72, 230} },
	{ WK_1, { 15, 267} },
	{ WK_MINUS, { 15, 230} },
	{ WK_PLUS, { 15, 192} },
	{ WK_BLR, { 91, 253 } },
	{ WK_DPAD, { 163, 225} }
};
//----------------------------------------------------------------------------
//
//
void Football2_Help()
{
	Platform_Help(help, sizeof(help)/sizeof(*help));
}

static BOOL bInited = FALSE;

void Football2_Init()
{
	int x, y;
	
	if (bInited) 
		return;
	
	// Init sounds
	Sound_set(&tcWaveRes[FOOTBALL2_SOUND_CHARGESTART], football2_chargestart_raw, football2_chargestart_raw_size, 865 );
	Sound_set(&tcWaveRes[FOOTBALL2_SOUND_CHARGE], football2_charge_raw, football2_charge_raw_size, 946 );
	Sound_set(&tcWaveRes[FOOTBALL2_SOUND_TICK], football2_tick_raw, football2_tick_raw_size, 12 );
	Sound_set(&tcWaveRes[FOOTBALL2_SOUND_RUNBACK], football2_runback_raw, football2_runback_raw_size, 260 );
	Sound_set(&tcWaveRes[FOOTBALL2_SOUND_FIRSTDOWN], football2_firstdown_raw, football2_firstdown_raw_size, 138 );
	Sound_set(&tcWaveRes[FOOTBALL2_SOUND_ENDPLAY], football2_endplay_raw, football2_endplay_raw_size, 334 );
	Sound_set(&tcWaveRes[FOOTBALL2_SOUND_ENDPOSSESSION], football2_endpossession_raw, football2_endpossession_raw_size, 700 );
	Sound_set(&tcWaveRes[FOOTBALL2_SOUND_ENDQUARTER], football2_endquarter_raw, football2_endquarter_raw_size, 995 );
	Sound_set(&tcWaveRes[FOOTBALL2_SOUND_TOUCHDOWN], football2_touchdown_raw, football2_touchdown_raw_size, 2566 );
	Sound_set(&tcWaveRes[FOOTBALL2_SOUND_SAFETY], football2_safety_raw, football2_safety_raw_size, 726 );

	// load images
	bmpScreen = GRRLIB_LoadTexture(football2_screen_png);
	
	for (y = 0; y < FOOTBALL2_BLIP_ROWS; y++){
		for (x = 0; x < FOOTBALL2_BLIP_COLUMNS; x++){
			Blip_t *pblip = &blip[x][y];
			
			pblip->x = (int)((x * ((float)football2_blip_xspacing/100)) + football2_blip_x);
			pblip->y = (int)((y * ((float)football2_blip_yspacing/100)) + football2_blip_y);
			pblip->status = -1;
		}
	}
	
	// clean digit 
	for (x=0; x<7; x++) {
		digit[x].val = -1;
		digit[x].type = DIGIT_TYPE_NORMAL;
	}
	
	PlatformSetInput(0);
	// turn off the game
	Football2_SetSkill(0);
	Football2_PowerOff();

	bInited = TRUE;

}

void Football2_StopSound()
{
	// stop all sounds...
	Platform_StopSound();
}

void Football2_DeInit()
{
	// stop all sounds...
	Football2_StopSound();
	Football2_PowerOff();
	GRRLIB_FreeTexture(bmpScreen);
	bInited = FALSE;
}

void Football2_Paint()
{
	int x, y, extra;
	BOOL power = Football2_GetPower();
	int skill = Football2_GetSkill();
	int p_switch;
	p_switch = Platform_GetPowerSwitchPlus(ONOFF_OFF12, &extra);
	if(power) {
		if(p_switch == -1) {
			if(skill%2 == 0)
				Football2_PowerOff();
			if(skill%2 == 1) {
				Football2_PowerOn();
				Football2_SetSkill((extra == 0) ? LVL_ROOKIESLOW : LVL_PROSLOW);
			}		
		} 
		if(p_switch == 1) {
			if(skill%2 == 0) {
				Football2_PowerOn();
				Football2_SetSkill((extra == 0) ? LVL_ROOKIEFAST : LVL_PROFAST);
			}
		} 
	}
	else {
		if(p_switch == 1) {
			Football2_PowerOn();
			Football2_SetSkill((extra == 0) ? LVL_ROOKIESLOW : LVL_PROSLOW);
		} 
	}
//	if (gMainWndP == NULL){ return; }

	// paint the backdrop
	GRRLIB_DrawImg(realx(0), realy(0), bmpScreen, 0, SCALE_X, SCALE_Y, 0xFFFFFFFF);
	
	// visualize the control states
	if (power){
		if (skill == 0){
			draw_switch(SWITCH_TYPE_1, football2_pro_1_x, football2_pro_1_y, SWITCH_POS_PRO1);
		} else {
			draw_switch(SWITCH_TYPE_1, football2_pro_1_x, football2_pro_1_y, SWITCH_POS_PRO2);
		}

		for (y = 0; y < FOOTBALL2_BLIP_ROWS; y++){
			for (x = 0; x < FOOTBALL2_BLIP_COLUMNS; x++){
				if(blip[x][y].status != -1)
					draw_oblip(blip[x][y].x, blip[x][y].y, blip[x][y].status);
			}
		}
		for(x = 0; x < 7; x++) {
			draw_digit_f(digit[x].x, digit[x].y, digit[x].val, digit[x].type);
		}	
	} 
	else {
		draw_switch(SWITCH_TYPE_1, football2_pro_1_x, football2_pro_1_y, SWITCH_POS_OFF);
	}
	
}

void Football2_ClearScreen()
{
	int x, y;
	
	Platform_StartDraw();

	// erase the blips
	for (y = 0; y < FOOTBALL2_BLIP_ROWS; y++){
		for (x = 0; x < FOOTBALL2_BLIP_COLUMNS; x++){
			Football2_DrawBlip(BLIP_OFF, x, y);
		}
	}
	// erase the stat display
	Football2_DrawScores(-1, -1);
	Football2_DrawTime(-1);

	Platform_EndDraw();
}

void Football2_PlaySound(int nSound, unsigned int nFlags)
{
	Platform_PlaySound(&tcWaveRes[nSound], nFlags);
}

//----------------------------------------------------------------------------
// local fcn's
//

void Football2_DrawBlip(int nBright, int x, int y)
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

void Football2_DrawScores(int nHScore, int nVScore)
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

void Football2_DrawDown(int nDown, int nYards)
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

void Football2_DrawTime(float fTime)
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

void Football2_DrawYard(int nYard)
{
	static int lastsign = 1;
	int sign;
	int val, idx;

	// convert display to proper format
	if (nYard > 50){
		sign = 1;
		lastsign = sign;
		val = 100 - nYard;
	} else if (nYard < 50){
		sign = -1;
		lastsign = sign;
		val = nYard;
	} else {
		// if ball is on the 50 yardline, use sign from last time
		sign = lastsign;
		val = nYard;
	}

	// draw yard
	idx = 2;

	if (sign == 1){
		// draw direction on left
		digit[idx].val = 0;
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
		digit[idx].val = 1;
		digit[idx].type = DIGIT_TYPE_SPECIAL;
	}
}

BOOL Football2_GetInputKICK(BOOL *pChange)
{
	static BOOL bLast = FALSE;

	if (pChange){ *pChange = FALSE; }

	// check the keys
	if (Platform_GetInput1())
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

BOOL Football2_GetInputSTATUS(BOOL *pChange)
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

BOOL Football2_GetInputSCORE(BOOL *pChange)
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

BOOL Football2_GetInputPASS(BOOL *pChange)
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

BOOL Football2_GetInputLEFT(BOOL *pChange)
{
	static BOOL bLast = FALSE;

	if (pChange){ *pChange = FALSE; }

	// check the keys
	if (Platform_GetInputLEFT())
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

BOOL Football2_GetInputUP(BOOL *pChange)
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

BOOL Football2_GetInputRIGHT(BOOL *pChange)
{
	static BOOL bLast = FALSE;

	if (pChange){ *pChange = FALSE; }

	// check the keys
	if (Platform_GetInputRIGHT())
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

BOOL Football2_GetInputDOWN(BOOL *pChange)
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

BOOL Football2_TestForMovement()
{
	// check the keys
	if (Platform_GetInputLEFT()
		|| Platform_GetInputUP()
		|| Platform_GetInputRIGHT()
		|| Platform_GetInputDOWN())
	{
		return TRUE;
	}

	return FALSE;
}

void Football2_GetSize(int *w, int *h)
{
	*w = bmpScreen->w;
	*h = bmpScreen->h;
}

