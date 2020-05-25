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
#include "Basketball2_screen_png.h"
#include "Basketball2_3secs_raw.h"
#include "Basketball2_bounce_raw.h"
#include "Basketball2_endgame_raw.h"
#include "Basketball2_endplay_raw.h"
#include "Basketball2_endquarter_raw.h"
#include "Basketball2_foul_raw.h"
#include "Basketball2_freethrow_raw.h"
#include "Basketball2_overtime_raw.h"
#include "Basketball2_score_raw.h"
#include "Basketball2_score3_raw.h"
#include "Basketball2_tick_raw.h"

// images
static GRRLIB_texImg *bmpScreen;

static Sound_t tcWaveRes[BASKETBALL2_SOUND_NSOUNDS];
static Blip_t blip[BASKETBALL2_BLIP_COLUMNS][BASKETBALL2_BLIP_ROWS];
static Blip_t basket;
static Stat_t digit[2];
static Help_t help[] = {	
	{ WK_U, { 33, 179} },
	{ WK_L, { 33, 205} },
	{ WK_R, { 33, 232} },
	{ WK_1, { 87, 175} },
	{ WK_2, { 175, 175} },
	{ WK_BLR1, {10, 268} },
	{ WK_DPAD, { 128, 231} }
};
//----------------------------------------------------------------------------
//
//
void Basketball2_Help()
{
	Platform_Help(help, sizeof(help)/sizeof(*help));
}

static BOOL bInited = FALSE;

void Basketball2_Init()
{
	int x, y;
	
	if (bInited) 
		return;

	// Init sounds
	Sound_set(&tcWaveRes[BASKETBALL2_SOUND_TICK], basketball2_tick_raw, basketball2_tick_raw_size, 8);
	Sound_set(&tcWaveRes[BASKETBALL2_SOUND_BOUNCE], basketball2_bounce_raw, basketball2_bounce_raw_size, 37);
	Sound_set(&tcWaveRes[BASKETBALL2_SOUND_SCORE], basketball2_score_raw, basketball2_score_raw_size, 1153);
	Sound_set(&tcWaveRes[BASKETBALL2_SOUND_ENDPLAY], basketball2_endplay_raw, basketball2_endplay_raw_size, 1061);
	Sound_set(&tcWaveRes[BASKETBALL2_SOUND_ENDQUARTER], basketball2_endquarter_raw, basketball2_endquarter_raw_size, 1400);
	Sound_set(&tcWaveRes[BASKETBALL2_SOUND_ENDGAME], basketball2_endgame_raw, basketball2_endgame_raw_size, 2823);
	Sound_set(&tcWaveRes[BASKETBALL2_SOUND_SCORE3], basketball2_score3_raw, basketball2_score3_raw_size, 2003);
	Sound_set(&tcWaveRes[BASKETBALL2_SOUND_FOUL], basketball2_foul_raw, basketball2_foul_raw_size, 0);
	Sound_set(&tcWaveRes[BASKETBALL2_SOUND_3SECS], basketball2_3secs_raw, basketball2_3secs_raw_size, 687);
	Sound_set(&tcWaveRes[BASKETBALL2_SOUND_FREETHROW], basketball2_freethrow_raw, basketball2_freethrow_raw_size, 608);
	Sound_set(&tcWaveRes[BASKETBALL2_SOUND_OVERTIME], basketball2_overtime_raw, basketball2_overtime_raw_size, 3007);
	
	// load images
	bmpScreen = GRRLIB_LoadTexture(basketball2_screen_png);

	// set blips 
	for (y = 0; y < BASKETBALL2_BLIP_ROWS; y++){
		for (x = 0; x < BASKETBALL2_BLIP_COLUMNS; x++){
			Blip_t *pblip = &blip[x][y];
			
			pblip->x = (x * basketball2_blip_xspacing) + basketball2_blip_x;
			pblip->y = (y * basketball2_blip_yspacing) + basketball2_blip_y;
			pblip->status = -1;
		}
	}
	basket.x = basketball2_basket_x;
	basket.y = basketball2_basket_y;
	basket.status = -1;
	
	// set digits
	for(x = 0; x < 2; x++) {
		digit[x].x = basketball2_digit_x + x * basketball2_digit_spacing;
		digit[x].y = basketball2_digit_y;
	}
	
	PlatformSetInput(0);
	// turn on the game
	Basketball2_SetSkill(LVL_HIGHSCHOOL);
	Basketball2_PowerOn();

	bInited = TRUE;
}

void Basketball2_StopSound()
{
	// stop all sounds...
	Platform_StopSound();
}

void Basketball2_DeInit()
{
	// stop all sounds...
	Basketball2_StopSound();
	Basketball2_PowerOff();
	GRRLIB_FreeTexture(bmpScreen);
	bInited = FALSE;
}

void Basketball2_Paint()
{
	int x, y, extra;
	BOOL power = Basketball2_GetPower();
	int skill = Basketball2_GetSkill();
	int p_switch;
	p_switch = Platform_GetPowerSwitchPlus(ONOFF_OFF12, &extra);
	if(power) {
		if(p_switch == -1) {
			if(skill%2 == 0)
				Basketball2_PowerOff();
			if(skill%2 == 1) {
				Basketball2_PowerOn();
				Basketball2_SetSkill((extra == 0) ? LVL_HIGHSCHOOL : LVL_ALLSTAR);
			}		
		} 
		if(p_switch == 1) {
			if(skill%2 == 0) {
				Basketball2_PowerOn();
				Basketball2_SetSkill((extra == 0) ? LVL_COLLAGE : LVL_PROFESSIONAL);
			}
		} 
	}
	else {
		if(p_switch == 1) {
			Basketball2_PowerOn();
			Basketball2_SetSkill((extra == 0) ? LVL_HIGHSCHOOL : LVL_ALLSTAR);
		} 
	}
	
	// paint the backdrop
	GRRLIB_DrawImg(realx(0), realy(0), bmpScreen, 0, SCALE_X, SCALE_Y, 0xFFFFFFFF);

	// visualize the control states
	if (power){
		if (skill%2 == 0){
			draw_switch(SWITCH_TYPE_4, basketball2_power_off_x, basketball2_power_off_y, SWITCH_POS_PRO1);
		} else {
			draw_switch(SWITCH_TYPE_4, basketball2_power_off_x, basketball2_power_off_y, SWITCH_POS_PRO2);
		}
 
		for (y = 0; y < BASKETBALL2_BLIP_ROWS; y++){
			for (x = 0; x < BASKETBALL2_BLIP_COLUMNS; x++){
				Blip_t *pblip = &blip[x][y];
				
				if(pblip->status != -1)
					draw_oblip(pblip->x, pblip->y, pblip->status);			
			}
		}	
		if(basket.status != -1)
			draw_oblip(basket.x, basket.y, basket.status);
		for(x = 0; x < 2; x++)
			draw_digit(digit[x].x, digit[x].y, digit[x].val);
	}
	else {
		draw_switch(SWITCH_TYPE_4, basketball2_power_off_x, basketball2_power_off_y, SWITCH_POS_OFF);
	}
}

void Basketball2_ClearScreen()
{
	Platform_StartDraw();

	// erase the blips
	for (int y = 0; y < BASKETBALL2_BLIP_ROWS; y++){
		for (int x = 0; x < BASKETBALL2_BLIP_COLUMNS; x++){
			Basketball2_DrawBlip(BLIP_OFF, x, y);
		}
	}

	Basketball2_DrawBasket(FALSE);

	// erase the stat display
	Basketball2_DrawStat(-1);

	Platform_EndDraw();
}

void Basketball2_PlaySound(int nSound, unsigned int nFlags)
{
	Platform_PlaySound(&tcWaveRes[nSound], nFlags);
}

//----------------------------------------------------------------------------
// local fcn's
//

void Basketball2_DrawBlip(int nBright, int x, int y)
{
	switch(nBright){
	case BLIP_DIM:
		blip[x][y].status  = BLIP_TYPE_NORMAL;
		break;
	case BLIP_BRIGHT:
		blip[x][y].status  = BLIP_TYPE_BRIGHT;
		break;
	case BLIP_OFF:
	default:
		blip[x][y].status  = -1;
		break;
	}
}

void Basketball2_DrawStat(int nStat)
{
	if (nStat == -1){
		// erase the display
		digit[0].val = -1;
		digit[1].val = -1;
	} else {
		// draw 10s place
		digit[0].val = nStat/10;
		// draw 1s place
		digit[1].val = nStat%10;		
	}
}

void Basketball2_DrawBasket(BOOL bBasket)
{
	Platform_StartDraw();

	if (bBasket)
	{
		int x, y;

		// erase the blips
		for (y = 0; y < BASKETBALL2_BLIP_ROWS; y++){
			for (x = 0; x < BASKETBALL2_BLIP_COLUMNS; x++){
				Basketball2_DrawBlip(BLIP_OFF, x, y);
			}
		}
		// draw the basket
		basket.status = BLIP_TYPE_BRIGHT;
	}
	else
	{
		// erase the basket
		basket.status = -1;
	}

	Platform_EndDraw();
}

BOOL Basketball2_GetInputLEFTUP(BOOL *pChange)
{
	static BOOL bLast = FALSE;

	if (pChange){ *pChange = FALSE; }

	// check the keys
	if (Platform_GetInputLEFTUP())
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

BOOL Basketball2_GetInputLEFTDOWN(BOOL *pChange)
{
	static BOOL bLast = FALSE;

	if (pChange){ *pChange = FALSE; }

	// check the keys
	if (Platform_GetInputLEFTDOWN())
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

BOOL Basketball2_GetInputLEFT(BOOL *pChange)
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

BOOL Basketball2_GetInputRIGHTUP(BOOL *pChange)
{
	static BOOL bLast = FALSE;

	if (pChange){ *pChange = FALSE; }

	// check the keys
	if (Platform_GetInputRIGHTUP())
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

BOOL Basketball2_GetInputRIGHTDOWN(BOOL *pChange)
{
	static BOOL bLast = FALSE;

	if (pChange){ *pChange = FALSE; }

	// check the keys
	if (Platform_GetInputRIGHTDOWN())
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

BOOL Basketball2_GetInputRIGHT(BOOL *pChange)
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

BOOL Basketball2_GetInputUP(BOOL *pChange)
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

BOOL Basketball2_GetInputDOWN(BOOL *pChange)
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

BOOL Basketball2_GetInputSHOOT(BOOL *pChange)
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

BOOL Basketball2_GetInputPASS(BOOL *pChange)
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

BOOL Basketball2_TestForMovement()
{
	// check the keys
	if (
	/*	Platform_GetInputLEFTUP() ||
		Platform_GetInputLEFTDOWN() || 
		Platform_GetInputRIGHTUP() ||
		Platform_GetInputRIGHTDOWN() || */
		Platform_GetInputLEFT() ||
		Platform_GetInputUP() ||
		Platform_GetInputRIGHT() ||
		Platform_GetInputDOWN() ||
		Platform_GetInput2() ||
		Platform_GetInput1())
	{
		return TRUE;
	}

	return FALSE;
}

void Basketball2_GetSize(int *w, int *h)
{
	*w = bmpScreen->w;
	*h = bmpScreen->h;
}
