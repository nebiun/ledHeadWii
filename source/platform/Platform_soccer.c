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

#include "soccer.h"
#include "soccer_screen_png.h"
#include "soccer_tick_raw.h"
#include "soccer_bounce_raw.h"
#include "soccer_score_raw.h"
#include "soccer_endplay_raw.h"
#include "soccer_endperiod_raw.h"
#include "soccer_endgame_raw.h"

// images
static GRRLIB_texImg *bmpScreen;

static Sound_t tcWaveRes[6];
static Blip_t blip[SOCCER_BLIP_COLUMNS][SOCCER_BLIP_ROWS];
static Blip_t goal;
static Stat_t digit[2];
static Help_t help[] = {
	{ WK_2, { 180, 243} },
	{ WK_BLR, { 25, 285 } },
	{ WK_DPAD, { 105, 252} }
};
//----------------------------------------------------------------------------
//
//
void Soccer_Help()
{
	Platform_Help(help, sizeof(help)/sizeof(*help));
}

static BOOL bInited = FALSE;

void Soccer_Init()
{
	int x, y;
	
	if (bInited) return;
	
	// Init sounds
	Sound_set(&tcWaveRes[0], soccer_tick_raw, soccer_tick_raw_size, 21 );
	Sound_set(&tcWaveRes[1], soccer_bounce_raw, soccer_bounce_raw_size, 47 );
	Sound_set(&tcWaveRes[2], soccer_score_raw, soccer_score_raw_size, 1637 );
	Sound_set(&tcWaveRes[3], soccer_endplay_raw, soccer_endplay_raw_size, 793 );
	Sound_set(&tcWaveRes[4], soccer_endperiod_raw, soccer_endperiod_raw_size, 1746 );
	Sound_set(&tcWaveRes[5], soccer_endgame_raw, soccer_endgame_raw_size, 2687 );

	// load images
	bmpScreen = GRRLIB_LoadTexture(soccer_screen_png);

	// set blips 
	for (y = 0; y < SOCCER_BLIP_ROWS; y++){
		for (x = 0; x < SOCCER_BLIP_COLUMNS; x++){
			Blip_t *pblip = &blip[x][y];
			
			pblip->x = (int)((x * ((float)soccer_blip_xspacing/100)) + soccer_blip_x);
			pblip->y = (int)((y * ((float)soccer_blip_yspacing/100)) + soccer_blip_y);
			pblip->status = -1;
		}
	}
	goal.x = soccer_goal_x;
	goal.y = soccer_goal_y;
	goal.status = -1;
	
	// set digits
	for(x = 0; x < 2; x++) {
		digit[x].x = soccer_digit_x + x * soccer_digit_spacing;
		digit[x].y = soccer_digit_y;
	}
	PlatformSetInput(0);
	// turn on the game
	Soccer_SetSkill(0);
	Soccer_PowerOn();

	bInited = TRUE;
}

void Soccer_DeInit()
{
	// stop all sounds...
	Platform_StopSound();
	GRRLIB_FreeTexture(bmpScreen);
	bInited = FALSE;
}

void Soccer_Paint()
{
	int x, y;
	BOOL power = Soccer_GetPower();
	BOOL skill = Soccer_GetSkill();
	int p_switch;
	p_switch = Platform_GetPowerSwitch(ONOFF_1OFF2);
	if(p_switch == -1) {
		if(!power) {
			Soccer_PowerOn();
			Soccer_SetSkill(0);
		}
		else if(power && skill == 1) {
			Soccer_PowerOff();
		}
	}
	else if(p_switch == 1) {
		if(!power) {
			Soccer_PowerOn();
			Soccer_SetSkill(1);
		}
		else if(power && skill == 0) {
			Soccer_PowerOff();
		}
	}
	
	// paint the backdrop
	GRRLIB_DrawImg(realx(0), realy(0), bmpScreen, 0, 1, 1, 0xFFFFFFFF);
	
	// visualize the control states
	if (power){
		if (skill == 0){
			draw_poweroff_a(soccer_power_off_x, soccer_power_off_y, POWER_POS_MODE1);
		} else {
			draw_poweroff_a(soccer_power_off_x, soccer_power_off_y, POWER_POS_MODE2);
		}	
		for (y = 0; y < SOCCER_BLIP_ROWS; y++){
			for (x = 0; x < SOCCER_BLIP_COLUMNS; x++){
				Blip_t *pblip = &blip[x][y];
				if(pblip->status != -1)
					draw_vblip(pblip->x, pblip->y, pblip->status);
			}
		}	
		if(goal.status != -1)
			draw_vblip(goal.x, goal.y, goal.status);
		for(x = 0; x < sizeof(digit)/sizeof(*digit); x++)
			draw_digit(digit[x].x, digit[x].y, digit[x].val);	
	} 
	else {
		draw_poweroff_a(soccer_power_off_x, soccer_power_off_y, POWER_POS_OFF);
	}
}

void Soccer_ClearScreen()
{
	Platform_StartDraw();
	
	// erase the blips
	for (int y = 0; y < SOCCER_BLIP_ROWS; y++){
		for (int x = 0; x < SOCCER_BLIP_COLUMNS; x++){
			Soccer_DrawBlip(BLIP_OFF, x, y);
		}
	}

	Soccer_DrawGoal(FALSE);

	// erase the stat display
	Soccer_DrawStat(-1);

	Platform_EndDraw();
}

void Soccer_PlaySound(int nSound, unsigned int nFlags)
{
	Platform_PlaySound(&tcWaveRes[nSound], nFlags);
}

void Soccer_StopSound()
{
	// stop all sounds...
	Platform_StopSound();
}


//----------------------------------------------------------------------------
// local fcn's
//

void Soccer_DrawBlip(int nBright, int x, int y)
{
	Blip_t *pblip = &blip[x][y];

	switch(nBright){
	case BLIP_DIM:
		pblip->status = BLIP_TYPE_NORMAL;
		break;
	case BLIP_BRIGHT:
		pblip->status = BLIP_TYPE_BRIGHT;
		break;
	case BLIP_DIMFLICKER:
		pblip->status = BLIP_TYPE_FLICKER;
		break;
	case BLIP_OFF:
	default:
		pblip->status = -1;
		break;
	}
}

void Soccer_DrawStat(int nStat)
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

void Soccer_DrawGoal(BOOL bBasket)
{
	Platform_StartDraw();

	if (bBasket)
	{
		int x, y;

		// erase the blips
		for (y = 0; y < SOCCER_BLIP_ROWS; y++){
			for (x = 0; x < SOCCER_BLIP_COLUMNS; x++){
				Soccer_DrawBlip(BLIP_OFF, x, y);
			}
		}
		// draw the goal
		goal.status = BLIP_TYPE_BRIGHT;
	}
	else
	{
		// erase the goal
		goal.status = -1;
	}

	Platform_EndDraw();
}

BOOL Soccer_GetInputLEFT(BOOL *pChange)
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

BOOL Soccer_GetInputUP(BOOL *pChange)
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

BOOL Soccer_GetInputRIGHT(BOOL *pChange)
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

BOOL Soccer_GetInputDOWN(BOOL *pChange)
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

BOOL Soccer_GetInputTHROW(BOOL *pChange)
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

BOOL Soccer_TestForMovement()
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

void Soccer_GetSize(int *w, int *h)
{
	*w = bmpScreen->w;
	*h = bmpScreen->h;
}

