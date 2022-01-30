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

#include "basketball.h"
#include "basketball_screen_png.h"
#include "basketball_tick_raw.h"
#include "basketball_bounce_raw.h"
#include "basketball_score_raw.h"
#include "basketball_endplay_raw.h"
#include "basketball_endquarter_raw.h"
#include "basketball_endgame_raw.h"

// images
static GRRLIB_texImg *bmpScreen;

static Sound_t tcWaveRes[BASKETBALL_SOUND_NSOUNDS];
static Blip_t blip[BASKETBALL_BLIP_COLUMNS][BASKETBALL_BLIP_ROWS];
static Blip_t basket;
static Stat_t digit[2];
static Help_t help[] = {
	{ WK_1, { 34, 232 } },
	{ WK_2, { 183, 232} },
	{ WK_BLR, {27, 275} },
	{ WK_DPAD, { 105, 242} }
};
//----------------------------------------------------------------------------
//
//
void Basketball_Help()
{
	Platform_Help(help, sizeof(help)/sizeof(*help));
}

static BOOL bInited = FALSE;

void Basketball_Init()
{
	int x, y;
	
	if (bInited) 
		return;

	// Init sounds
	Sound_set(&tcWaveRes[BASKETBALL_SOUND_TICK], basketball_tick_raw, basketball_tick_raw_size, 27);
	Sound_set(&tcWaveRes[BASKETBALL_SOUND_BOUNCE], basketball_bounce_raw, basketball_bounce_raw_size, 50);
	Sound_set(&tcWaveRes[BASKETBALL_SOUND_SCORE], basketball_score_raw, basketball_score_raw_size, 799);
	Sound_set(&tcWaveRes[BASKETBALL_SOUND_ENDPLAY], basketball_endplay_raw, basketball_endplay_raw_size, 721);
	Sound_set(&tcWaveRes[BASKETBALL_SOUND_ENDQUARTER], basketball_endquarter_raw, basketball_endquarter_raw_size, 1878);
	Sound_set(&tcWaveRes[BASKETBALL_SOUND_ENDGAME], basketball_endgame_raw, basketball_endgame_raw_size, 3753);
	
	// load images
	bmpScreen = GRRLIB_LoadTexture(basketball_screen_png);

	// set blips 
	for (y = 0; y < BASKETBALL_BLIP_ROWS; y++){
		for (x = 0; x < BASKETBALL_BLIP_COLUMNS; x++){
			Blip_t *pblip = &blip[x][y];
			
			pblip->x = (int)((x * ((float)basketball_blip_xspacing/100)) + basketball_blip_x);
			pblip->y = (int)((y * ((float)basketball_blip_yspacing/100)) + basketball_blip_y);
			pblip->status = -1;
		}
	}
	basket.x = basketball_basket_x;
	basket.y = basketball_basket_y;
	basket.status = -1;
	
	// set digits
	for(x = 0; x < 2; x++) {
		digit[x].x = basketball_digit_x + x * basketball_digit_spacing;
		digit[x].y = basketball_digit_y;
	}
	
	PlatformSetInput(0);
	// turn on the game
	Basketball_SetSkill(0);
	Basketball_PowerOn();

	bInited = TRUE;
}

void Basketball_StopSound()
{
	// stop all sounds...
	Platform_StopSound();
}

void Basketball_DeInit()
{
	// stop all sounds...
	Basketball_StopSound();
	Basketball_PowerOff();
	GRRLIB_FreeTexture(bmpScreen);
	bInited = FALSE;
}

void Basketball_Paint()
{
	int x, y;
	BOOL power = Basketball_GetPower();
	BOOL skill = Basketball_GetSkill();
	int p_switch;
	p_switch = Platform_GetPowerSwitch(ONOFF_1OFF2);
	if(p_switch == -1) {
		if(!power) {
			Basketball_PowerOn();
			Basketball_SetSkill(0);
		}
		else if(power && skill == 1) {
			Basketball_PowerOff();
		}
	}
	else if(p_switch == 1) {
		if(!power) {
			Basketball_PowerOn();
			Basketball_SetSkill(1);
		}
		else if(power && skill == 0) {
			Basketball_PowerOff();
		}
	}
	
	// paint the backdrop
	GRRLIB_DrawImg(realx(0), realy(0), bmpScreen, 0, SCALE_X, SCALE_Y, 0xFFFFFFFF);

	// visualize the control states
	if (power){
		if (skill == 0){
			draw_switch(SWITCH_TYPE_2,basketball_pro_1_x, basketball_pro_1_y, SWITCH_POS_MODE1);
		} else {
			draw_switch(SWITCH_TYPE_2,basketball_pro_1_x, basketball_pro_1_y, SWITCH_POS_MODE2);
		}
 
		for (y = 0; y < BASKETBALL_BLIP_ROWS; y++){
			for (x = 0; x < BASKETBALL_BLIP_COLUMNS; x++){
				Blip_t *pblip = &blip[x][y];
				
				if(pblip->status != -1)
					draw_vblip(pblip->x, pblip->y, pblip->status);
			}
		}	
		if(basket.status != -1)
			draw_vblip(basket.x, basket.y, basket.status);
		for(x = 0; x < 2; x++)
			draw_digit(digit[x].x, digit[x].y, digit[x].val);
	}
	else {
		draw_switch(SWITCH_TYPE_2,basketball_pro_1_x, basketball_pro_1_y, SWITCH_POS_OFF);
	}
}

void Basketball_ClearScreen()
{
	Platform_StartDraw();

	// erase the blips
	for (int y = 0; y < BASKETBALL_BLIP_ROWS; y++){
		for (int x = 0; x < BASKETBALL_BLIP_COLUMNS; x++){
			Basketball_DrawBlip(BLIP_OFF, x, y);
		}
	}

	Basketball_DrawBasket(FALSE);

	// erase the stat display
	Basketball_DrawStat(-1);

	Platform_EndDraw();
}

void Basketball_PlaySound(int nSound, unsigned int nFlags)
{
	Platform_PlaySound(&tcWaveRes[nSound], nFlags);
}

//----------------------------------------------------------------------------
// local fcn's
//

void Basketball_DrawBlip(int nBright, int x, int y)
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

void Basketball_DrawStat(int nStat)
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

void Basketball_DrawBasket(BOOL bBasket)
{
	Platform_StartDraw();

	if (bBasket)
	{
		int x, y;

		// erase the blips
		for (y = 0; y < BASKETBALL_BLIP_ROWS; y++){
			for (x = 0; x < BASKETBALL_BLIP_COLUMNS; x++){
				Basketball_DrawBlip(BLIP_OFF, x, y);
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

BOOL Basketball_GetInputLEFT(BOOL *pChange)
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

BOOL Basketball_GetInputUP(BOOL *pChange)
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

BOOL Basketball_GetInputRIGHT(BOOL *pChange)
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

BOOL Basketball_GetInputDOWN(BOOL *pChange)
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

BOOL Basketball_GetInputTHROW(BOOL *pChange)
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

BOOL Basketball_TestForMovement()
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

void Basketball_GetSize(int *w, int *h)
{
	*w = bmpScreen->w;
	*h = bmpScreen->h;
}
