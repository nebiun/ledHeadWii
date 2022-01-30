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

#include "hockey.h"
#include "hockey_screen_png.h"
#include "hockey_tick_raw.h"
#include "hockey_deflect_raw.h"
#include "hockey_bump_raw.h"
#include "hockey_poke_raw.h"
#include "hockey_score_raw.h"
#include "hockey_penalty_raw.h"
#include "hockey_steal_raw.h"
#include "hockey_endperiod_raw.h"
#include "hockey_endgame_raw.h"
	
// images
static GRRLIB_texImg *bmpScreen;

static Sound_t tcWaveRes[HOCKEY_SOUND_NSOUNDS];
static Blip_t blip[HOCKEY_BLIP_COLUMNS][HOCKEY_BLIP_ROWS];
static Stat_t digit[2];
static Help_t help[] = {
	{ WK_1, { 34, 243} },
	{ WK_2, { 181, 243} },
	{ WK_BLR, { 24, 286} },
	{ WK_DPAD, { 104, 252} }
};
//----------------------------------------------------------------------------
//
//
void Hockey_Help()
{
	Platform_Help(help, sizeof(help)/sizeof(*help));
}

static BOOL bInited = FALSE;

void Hockey_Init()
{
	int x, y;
	
	if (bInited) 
		return;
	
	// Init sounds
	Sound_set(&tcWaveRes[HOCKEY_SOUND_TICK], hockey_tick_raw, hockey_tick_raw_size, 14 );
	Sound_set(&tcWaveRes[HOCKEY_SOUND_DEFLECT], hockey_deflect_raw, hockey_deflect_raw_size, 16 );
	Sound_set(&tcWaveRes[HOCKEY_SOUND_BUMP], hockey_bump_raw, hockey_bump_raw_size, 35 );
	Sound_set(&tcWaveRes[HOCKEY_SOUND_POKE], hockey_poke_raw, hockey_poke_raw_size, 57 );
	Sound_set(&tcWaveRes[HOCKEY_SOUND_SCORE], hockey_score_raw, hockey_score_raw_size, 1575 );
	Sound_set(&tcWaveRes[HOCKEY_SOUND_PENALTY], hockey_penalty_raw, hockey_penalty_raw_size, 1196 );
	Sound_set(&tcWaveRes[HOCKEY_SOUND_STEAL], hockey_steal_raw, hockey_steal_raw_size, 531 );
	Sound_set(&tcWaveRes[HOCKEY_SOUND_ENDPERIOD], hockey_endperiod_raw, hockey_endperiod_raw_size, 1205 );
	Sound_set(&tcWaveRes[HOCKEY_SOUND_ENDGAME], hockey_endgame_raw, hockey_endgame_raw_size, 2382 );

	// load images
	bmpScreen = GRRLIB_LoadTexture(hockey_screen_png);

	// set blips 
	for (y = 0; y < HOCKEY_BLIP_ROWS; y++){
		for (x = 0; x < HOCKEY_BLIP_COLUMNS; x++){
			Blip_t *pblip = &blip[x][y];
			
			pblip->x = (int)((x * ((float)hockey_blip_xspacing/100)) + hockey_blip_x);
			if(y == 0)
				pblip->y = (int)(hockey_blip_y - ((hockey_blip_yspacing/100)/2));
			else
				pblip->y = (int)(((y - 1)  * ((float)hockey_blip_yspacing/100)) + hockey_blip_y);
			pblip->status = -1;
		}
	}
	// set digits
	for(x = 0; x < 2; x++) {
		digit[x].x = hockey_digit_x + x * hockey_digit_spacing;
		digit[x].y = hockey_digit_y;
	}
	
	PlatformSetInput(0);
	// turn on the game
	Hockey_SetSkill(0);
	Hockey_PowerOn();

	bInited = TRUE;
}

void Hockey_StopSound()
{
	// stop all sounds...
	Platform_StopSound();
}

void Hockey_DeInit()
{
	// stop all sounds...
	Hockey_StopSound();
	Hockey_PowerOff();
	GRRLIB_FreeTexture(bmpScreen);
	bInited = FALSE;
}

void Hockey_Paint()
{
	int x, y;
	BOOL power = Hockey_GetPower();
	BOOL skill = Hockey_GetSkill();
	int p_switch;
	p_switch = Platform_GetPowerSwitch(ONOFF_1OFF2);
	if(p_switch == -1) {
		if(!power) {
			Hockey_PowerOn();
			Hockey_SetSkill(0);
		}
		else if(power && skill == 1) {
			Hockey_PowerOff();
		}
	}
	else if(p_switch == 1) {
		if(!power) {
			Hockey_PowerOn();
			Hockey_SetSkill(1);
		}
		else if(power && skill == 0) {
			Hockey_PowerOff();
		}
	}	
	// paint the backdrop
	GRRLIB_DrawImg(realx(0), realy(0), bmpScreen, 0, SCALE_X, SCALE_Y, 0xFFFFFFFF);
	
	// visualize the control states
	if (power){
		if (skill == 0){
			draw_switch(SWITCH_TYPE_2, hockey_pro_1_x, hockey_pro_1_y, SWITCH_POS_MODE1);
		} else {
			draw_switch(SWITCH_TYPE_2, hockey_pro_1_x, hockey_pro_1_y, SWITCH_POS_MODE2);
		}
		
		for (y = 0; y < HOCKEY_BLIP_ROWS; y++){
			for (x = 0; x < HOCKEY_BLIP_COLUMNS; x++){
				if(blip[x][y].status != -1)
					draw_vblip(blip[x][y].x, blip[x][y].y, blip[x][y].status);
			}
		}	

		for(x = 0; x < 2; x++)
			draw_digit(digit[x].x, digit[x].y, digit[x].val);
	} 
	else {
		draw_switch(SWITCH_TYPE_2, hockey_pro_1_x, hockey_pro_1_y, SWITCH_POS_OFF);
	}
}

void Hockey_ClearScreen()
{
	Platform_StartDraw();

	// erase the blips
	for (int y = 0; y < HOCKEY_BLIP_ROWS; y++){
		for (int x = 0; x < HOCKEY_BLIP_COLUMNS; x++){
			Hockey_DrawBlip(BLIP_OFF, x, y);
		}
	}

	// erase the stat display
	Hockey_DrawStat(-1);

	Platform_EndDraw();
}

void Hockey_PlaySound(int nSound, unsigned int nFlags)
{
	Platform_PlaySound(&tcWaveRes[nSound], nFlags);
}

//----------------------------------------------------------------------------
// local fcn's
//

void Hockey_DrawBlip(int nBright, int x, int y)
{
	int new_status;
	
	switch(nBright){
	case BLIP_DIM:
		new_status = BLIP_TYPE_NORMAL;
		break;
	case BLIP_BRIGHT:
		new_status = BLIP_TYPE_BRIGHT;
		break;
	case BLIP_OFF:
	default:
		new_status = -1;
		break;
	}
	blip[x][y].status = new_status;
}

void Hockey_DrawStat(int nStat)
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

BOOL Hockey_GetInputLEFT(BOOL *pChange)
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

BOOL Hockey_GetInputUP(BOOL *pChange)
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

BOOL Hockey_GetInputRIGHT(BOOL *pChange)
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

BOOL Hockey_GetInputDOWN(BOOL *pChange)
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

BOOL Hockey_GetInputTHROW(BOOL *pChange)
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

BOOL Hockey_TestForMovement()
{
	// check the keys
	if (Platform_GetInputLEFT()
		|| Platform_GetInputUP()
		|| Platform_GetInputRIGHT()
		|| Platform_GetInputDOWN()
		|| Platform_GetInput2())
	{
		return TRUE;
	}

	return FALSE;
}

void Hockey_GetSize(int *w, int *h)
{
	*w = bmpScreen->w;
	*h = bmpScreen->h;
}

