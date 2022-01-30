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

#include "hockeyca.h"
#include "hockeyca_screen_png.h"
#include "hockeyca_tick_raw.h"
#include "hockeyca_bounce_raw.h"
#include "hockeyca_score_raw.h"
#include "hockeyca_endplay_raw.h"
#include "hockeyca_endperiod_raw.h"
#include "hockeyca_endgame_raw.h"
	
// images
static GRRLIB_texImg *bmpScreen;

static Sound_t tcWaveRes[HOCKEYCA_SOUND_NSOUNDS];
static Blip_t blip[HOCKEYCA_BLIP_COLUMNS][HOCKEYCA_BLIP_ROWS];
static Blip_t goal;
static Stat_t digit[2];
static Help_t help[] = {
	{ WK_1, { 34, 243} },
	{ WK_2, { 181, 243} },
	{ WK_BLR, { 25, 286 } },
	{ WK_DPAD, { 104, 251} }
};
//----------------------------------------------------------------------------
//
//
void HockeyCa_Help()
{
	Platform_Help(help, sizeof(help)/sizeof(*help));
}

static BOOL bInited = FALSE;

void HockeyCa_Init()
{	
	int x, y;
	
	if (bInited) 
		return;
	
	// Init sounds
	Sound_set(&tcWaveRes[HOCKEYCA_SOUND_TICK], hockeyca_tick_raw, hockeyca_tick_raw_size, 13 );
	Sound_set(&tcWaveRes[HOCKEYCA_SOUND_BOUNCE], hockeyca_bounce_raw, hockeyca_bounce_raw_size, 33 );
	Sound_set(&tcWaveRes[HOCKEYCA_SOUND_SCORE], hockeyca_score_raw, hockeyca_score_raw_size, 3307 );
	Sound_set(&tcWaveRes[HOCKEYCA_SOUND_ENDPLAY], hockeyca_endplay_raw, hockeyca_endplay_raw_size, 762 );
	Sound_set(&tcWaveRes[HOCKEYCA_SOUND_ENDQUARTER], hockeyca_endperiod_raw, hockeyca_endperiod_raw_size, 1654 );
	Sound_set(&tcWaveRes[HOCKEYCA_SOUND_ENDGAME], hockeyca_endgame_raw, hockeyca_endgame_raw_size, 2543 );

	// load images
	bmpScreen = GRRLIB_LoadTexture(hockeyca_screen_png);

	// set blips 
	for (y = 0; y < HOCKEYCA_BLIP_ROWS; y++){
		for (x = 0; x < HOCKEYCA_BLIP_COLUMNS; x++){
			Blip_t *pblip = &blip[x][y];
			
			pblip->x = (int)((x * ((float)hockeyca_blip_xspacing/100)) + hockeyca_blip_x);
			pblip->y = (int)((y * ((float)hockeyca_blip_yspacing/100)) + hockeyca_blip_y);
			pblip->status = -1;
		}
	}
	goal.x = hockeyca_goal_x;
	goal.y = hockeyca_goal_y;
	goal.status = -1;
	
	// set digits
	for(x = 0; x < 2; x++) {
		digit[x].x = hockeyca_digit_x + x * hockeyca_digit_spacing;
		digit[x].y = hockeyca_digit_y;
	}
	PlatformSetInput(0);
	// turn on the game
	HockeyCa_SetSkill(0);
	HockeyCa_PowerOn();

	bInited = TRUE;
}

void HockeyCa_StopSound()
{
	// stop all sounds...
	Platform_StopSound();
}

void HockeyCa_DeInit()
{
	// stop all sounds...
	HockeyCa_StopSound();
	HockeyCa_PowerOff();
	GRRLIB_FreeTexture(bmpScreen);
	bInited = FALSE;
}

void HockeyCa_Paint()
{
	int x, y;
	BOOL power = HockeyCa_GetPower();
	BOOL skill = HockeyCa_GetSkill();
	int p_switch;
	p_switch = Platform_GetPowerSwitch(ONOFF_1OFF2);
	if(p_switch == -1) {
		if(!power) {
			HockeyCa_PowerOn();
			HockeyCa_SetSkill(0);
		}
		else if(power && skill == 1) {
			HockeyCa_PowerOff();
		}
	}
	else if(p_switch == 1) {
		if(!power) {
			HockeyCa_PowerOn();
			HockeyCa_SetSkill(1);
		}
		else if(power && skill == 0) {
			HockeyCa_PowerOff();
		}
	}	
	// paint the backdrop
	GRRLIB_DrawImg(realx(0), realy(0), bmpScreen, 0, SCALE_X, SCALE_Y, 0xFFFFFFFF);
	
	// visualize the control states
	if (power){
		if (skill == 0){
			draw_switch(SWITCH_TYPE_2,hockeyca_pro_1_x, hockeyca_pro_1_y, SWITCH_POS_MODE1);
		} else {
			draw_switch(SWITCH_TYPE_2,hockeyca_pro_1_x, hockeyca_pro_1_y, SWITCH_POS_MODE2);
		}
		
		for (y = 0; y < HOCKEYCA_BLIP_ROWS; y++){
			for (x = 0; x < HOCKEYCA_BLIP_COLUMNS; x++){
				if(blip[x][y].status != -1)
					draw_vblip(blip[x][y].x, blip[x][y].y, blip[x][y].status);
			}
		}
		
		if(goal.status != -1)
			draw_vblip(goal.x, goal.y, goal.status);
		
		for(x = 0; x < sizeof(digit)/sizeof(*digit); x++)
			draw_digit(digit[x].x, digit[x].y, digit[x].val);	
	} 
	else {
		draw_switch(SWITCH_TYPE_2,hockeyca_pro_1_x, hockeyca_pro_1_y, SWITCH_POS_OFF);
	}
}

void HockeyCa_ClearScreen()
{
	Platform_StartDraw();

	// erase the blips
	for (int y = 0; y < HOCKEYCA_BLIP_ROWS; y++){
		for (int x = 0; x < HOCKEYCA_BLIP_COLUMNS; x++){
			HockeyCa_DrawBlip(BLIP_OFF, x, y);
		}
	}
	HockeyCa_DrawGoal(FALSE);

	// erase the stat display
	Platform_EndDraw();
}

void HockeyCa_PlaySound(int nSound, unsigned int nFlags)
{
	Platform_PlaySound(&tcWaveRes[nSound], nFlags);
}

//----------------------------------------------------------------------------
// local fcn's
//

void HockeyCa_DrawBlip(int nBright, int x, int y)
{
	int new_status;

	switch(nBright){
	case BLIP_DIM:
		new_status = BLIP_TYPE_NORMAL;
		break;
	case BLIP_BRIGHT:
		new_status = BLIP_TYPE_BRIGHT;
		break;
	case BLIP_DIMFLICKER:
		new_status = BLIP_TYPE_FLICKER;
		break;
	case BLIP_OFF:
	default:
		new_status = -1;
		break;
	}
	blip[x][y].status = new_status;
}

void HockeyCa_DrawStat(int nStat)
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

void HockeyCa_DrawGoal(BOOL bBasket)
{
	Platform_StartDraw();

	if (bBasket)
	{
		int x, y;

		// erase the blips
		for (y = 0; y < HOCKEYCA_BLIP_ROWS; y++){
			for (x = 0; x < HOCKEYCA_BLIP_COLUMNS; x++){
				HockeyCa_DrawBlip(BLIP_OFF, x, y);
			}
		}
		// draw the basket
		goal.status = BLIP_TYPE_BRIGHT;
	}
	else
	{
		// erase the goal
		goal.status = -1;
	}

	Platform_EndDraw();
}

BOOL HockeyCa_GetInputLEFT(BOOL *pChange)
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

BOOL HockeyCa_GetInputUP(BOOL *pChange)
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

BOOL HockeyCa_GetInputRIGHT(BOOL *pChange)
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

BOOL HockeyCa_GetInputDOWN(BOOL *pChange)
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

BOOL HockeyCa_GetInputTHROW(BOOL *pChange)
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

BOOL HockeyCa_TestForMovement()
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

void HockeyCa_GetSize(int *w, int *h)
{
	*w = bmpScreen->w;
	*h = bmpScreen->h;
}

