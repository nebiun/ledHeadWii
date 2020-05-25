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

#include "soccer2.h"
#include "soccer2_screen_png.h"
#include "soccer2_ballout_raw.h"  
#include "soccer2_highkick_raw.h" 
#include "soccer2_beep_raw.h"  
#include "soccer2_looseball_raw.h" 
#include "soccer2_endgame_raw.h" 
#include "soccer2_lowkick_raw.h" 
#include "soccer2_endhalf_raw.h" 
#include "soccer2_tick_raw.h" 
#include "soccer2_goal_raw.h"
#include "soccer2_bounce_raw.h" 

// images
static GRRLIB_texImg *bmpScreen;

static Sound_t tcWaveRes[SOCCER2_SOUND_NSOUNDS];
static Blip_t blip[SOCCER2_BLIP_COLUMNS][SOCCER2_BLIP_ROWS];
static Blip_t goalarea;
static Digit_t digit[2];
static Help_t help[] = {	
	{ WK_PLUS, { 38, 181} },
	{ WK_U, { 34, 206} },
	{ WK_MINUS, { 38, 237} },
	{ WK_1, { 87, 175} },
	{ WK_2, { 175, 175} },
	{ WK_BLR1, {10, 268} },
	{ WK_DPAD, { 128, 231} }
};

//----------------------------------------------------------------------------
//
//
void Soccer2_Help()
{
	Platform_Help(help, sizeof(help)/sizeof(*help));
}

static BOOL bInited = FALSE;

void Soccer2_Init()
{
	int x, y;
	
	if (bInited) 
		return;
	
	// Init sounds
	Sound_set(&tcWaveRes[SOCCER2_SOUND_TICK], soccer2_tick_raw, soccer2_tick_raw_size, 8);
	Sound_set(&tcWaveRes[SOCCER2_SOUND_LOOSEBALL], soccer2_looseball_raw, soccer2_looseball_raw_size,54);
	Sound_set(&tcWaveRes[SOCCER2_SOUND_BALLOUT], soccer2_ballout_raw, soccer2_ballout_raw_size,461);
	Sound_set(&tcWaveRes[SOCCER2_SOUND_GOAL], soccer2_goal_raw, soccer2_goal_raw_size, 1225);
	Sound_set(&tcWaveRes[SOCCER2_SOUND_BEEP], soccer2_beep_raw, soccer2_beep_raw_size, 86);
	Sound_set(&tcWaveRes[SOCCER2_SOUND_HIGHKICK], soccer2_highkick_raw, soccer2_highkick_raw_size, 79);
	Sound_set(&tcWaveRes[SOCCER2_SOUND_LOWKICK], soccer2_lowkick_raw, soccer2_lowkick_raw_size, 93);
	Sound_set(&tcWaveRes[SOCCER2_SOUND_ENDHALF], soccer2_endhalf_raw, soccer2_endhalf_raw_size, 1335);
	Sound_set(&tcWaveRes[SOCCER2_SOUND_ENDGAME], soccer2_endgame_raw, soccer2_endgame_raw_size, 2234);
	Sound_set(&tcWaveRes[SOCCER2_SOUND_BOUNCE], soccer2_bounce_raw, soccer2_bounce_raw_size, 144);
	
	// load images
	bmpScreen = GRRLIB_LoadTexture(soccer2_screen_png);
	
	for (y = 0; y < SOCCER2_BLIP_ROWS; y++){
		for (x = 0; x < SOCCER2_BLIP_COLUMNS; x++){
			Blip_t *pblip = &blip[x][y];
			
			pblip->x = (x * soccer2_blip_xspacing) + soccer2_blip_x;
			pblip->y = (y * soccer2_blip_yspacing) + soccer2_blip_y;
			pblip->status = -1;
		}
	}
	goalarea.x = soccer2_goalarea_x;
	goalarea.y = soccer2_goalarea_y;
	goalarea.status = -1;
	
	// clean digit 
	// set digits
	for(x = 0; x < 2; x++) {
		digit[x].x = soccer2_digit_x + x * soccer2_digit_spacing;
		digit[x].y = soccer2_digit_y;
		digit[x].val = -1;
		digit[x].type = DIGIT_TYPE_NORMAL;
	}
	
	PlatformSetInput(0);
	// turn on the game
	Soccer2_SetSkill(0);
	Soccer2_PowerOn();

	bInited = TRUE;
}

void Soccer2_StopSound()
{
	// stop all sounds...
	Platform_StopSound();
}

void Soccer2_DeInit()
{
	// stop all sounds...
	Soccer2_StopSound();
	Soccer2_PowerOff();
	GRRLIB_FreeTexture(bmpScreen);
	bInited = FALSE;
}

void Soccer2_Paint()
{
	int x, y, extra;
	BOOL power = Soccer2_GetPower();
	int skill = Soccer2_GetSkill();
	int p_switch;
	p_switch = Platform_GetPowerSwitchPlus(ONOFF_OFF12, &extra);
	if(power) {
		if(p_switch == -1) {
			if(skill%2 == 0)
				Soccer2_PowerOff();
			if(skill%2 == 1) {
				Soccer2_PowerOn();
				Soccer2_SetSkill((extra == 0) ? LVL_ROOKIESLOW : LVL_PROSLOW);
			}		
		} 
		if(p_switch == 1) {
			if(skill%2 == 0) {
				Soccer2_PowerOn();
				Soccer2_SetSkill((extra == 0) ? LVL_ROOKIEFAST : LVL_PROFAST);
			}
		} 
	}
	else {
		if(p_switch == 1) {
			Soccer2_PowerOn();
			Soccer2_SetSkill((extra == 0) ? LVL_ROOKIESLOW : LVL_PROSLOW);
		} 
	}
	
//	if (gMainWndP == NULL){ return; }

	// paint the backdrop
	GRRLIB_DrawImg(realx(0), realy(0), bmpScreen, 0, SCALE_X, SCALE_Y, 0xFFFFFFFF);
	
	// visualize the control states
	if (power){
		if (skill%2 == 0){
			draw_switch(SWITCH_TYPE_4, soccer2_power_off_x, soccer2_power_off_y, SWITCH_POS_PRO1);
		} else {
			draw_switch(SWITCH_TYPE_4, soccer2_power_off_x, soccer2_power_off_y, SWITCH_POS_PRO2);
		}
 
		for (y = 0; y < SOCCER2_BLIP_ROWS; y++){
			for (x = 0; x < SOCCER2_BLIP_COLUMNS; x++){
				Blip_t *pblip = &blip[x][y];
				
				if(pblip->status != -1)
					draw_oblip(pblip->x, pblip->y, pblip->status);			
			}
		}
		if(goalarea.status != -1)
			draw_oblip(goalarea.x, goalarea.y, goalarea.status);
		
		for(x = 0; x < 2; x++)
			draw_digit_f(digit[x].x, digit[x].y, digit[x].val, digit[x].type);
	}
	else {
		draw_switch(SWITCH_TYPE_4, soccer2_power_off_x, soccer2_power_off_y, SWITCH_POS_OFF);
	}
}

void Soccer2_ClearScreen()
{
	int x, y;
	
	Platform_StartDraw();

	// erase the blips
	for (y = 0; y < SOCCER2_BLIP_ROWS; y++){
		for (x = 0; x < SOCCER2_BLIP_COLUMNS; x++){
			Soccer2_DrawBlip(BLIP_OFF, x, y);
		}
	}
	// erase the stat display
	Soccer2_DrawStat(-1);

	Platform_EndDraw();
}

void Soccer2_PlaySound(int nSound, unsigned int nFlags)
{
	if(tcWaveRes[nSound].stream != NULL)
		Platform_PlaySound(&tcWaveRes[nSound], nFlags);
}

//----------------------------------------------------------------------------
// local fcn's
//

void Soccer2_DrawBlip(int nBright, int x, int y)
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

void Soccer2_DrawGoalareaFlag(BOOL status)
{
	if (status) {
		// goalarea flag on
		goalarea.status = BLIP_TYPE_NORMAL;
	}
	else {
		// goalarea flag off
		goalarea.status = -1;
	}
}

void Soccer2_DrawStat(int nStat)
{
	digit[0].type = DIGIT_TYPE_NORMAL;
	digit[1].type = DIGIT_TYPE_NORMAL;
	
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

void Soccer2_DrawSect(int nSect, BOOL side)
{
	digit[0].type = DIGIT_TYPE_NORMAL;
	digit[0].val = (side == TRUE) ? nSect : (10 - nSect);
	digit[1].type = DIGIT_TYPE_SPECIAL;
	digit[1].val = (side == FALSE) ? 2 : 3;	// Print U or H
}

BOOL Soccer2_GetInputPASS(BOOL *pChange)
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

BOOL Soccer2_GetInputSHOOT(BOOL *pChange)
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

BOOL Soccer2_GetInputKICK(BOOL *pChange)
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

BOOL Soccer2_GetInputSCORE(BOOL *pChange)
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

BOOL Soccer2_GetInputTEAM(BOOL *pChange)
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

BOOL Soccer2_GetInputLEFTUP(BOOL *pChange)
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

BOOL Soccer2_GetInputLEFTDOWN(BOOL *pChange)
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

BOOL Soccer2_GetInputLEFT(BOOL *pChange)
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

BOOL Soccer2_GetInputRIGHTUP(BOOL *pChange)
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

BOOL Soccer2_GetInputRIGHTDOWN(BOOL *pChange)
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

BOOL Soccer2_GetInputRIGHT(BOOL *pChange)
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

BOOL Soccer2_GetInputUP(BOOL *pChange)
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

BOOL Soccer2_GetInputDOWN(BOOL *pChange)
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

BOOL Soccer2_TestForMovement()
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

void Soccer2_GetSize(int *w, int *h)
{
	*w = bmpScreen->w;
	*h = bmpScreen->h;
}

