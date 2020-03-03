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

#include "subchase.h"
#include "subchase_screen_png.h"
#include "subchase_blipdim_png.h"
#include "subchase_blipbright_png.h"
#include "subchase_digits_png.h"
#include "subchase_poweroff_png.h"
#include "subchase_poweron_png.h"
#include "subchase_sonar_raw.h"
#include "subchase_charge_raw.h"
#include "subchase_hit_raw.h"
#include "subchase_sink_raw.h"

#define DIGIT_W	24
#define DIGIT_H	28

// images
static GRRLIB_texImg *bmpScreen;
static GRRLIB_texImg *bmpBlipDim;
static GRRLIB_texImg *bmpBlipBright;
static GRRLIB_texImg *bmpDigits;
static GRRLIB_texImg *bmpPowerOff;
static GRRLIB_texImg *bmpPowerOn;

static Sound_t tcWaveRes[4];
static Blip_t blip[SUBCHASE_BLIP_COLUMNS][SUBCHASE_BLIP_ROWS];
static Stat_t digit[2];
static Help_t help[] = {
	{ WK_2, {1, 115} },
	{ WK_BUD, { 73, 242 } },
	{ WK_DPAD, { 148, 257 } }
};
//----------------------------------------------------------------------------
//
//
void SubChase_Help()
{
	Platform_Help(help, sizeof(help)/sizeof(*help));
}

static BOOL bInited = FALSE;

void SubChase_Init()
{
	int x, y;

	// Init sounds
	Sound_set(&tcWaveRes[0], subchase_sonar_raw, subchase_sonar_raw_size, 119);
	Sound_set(&tcWaveRes[1], subchase_charge_raw, subchase_charge_raw_size, 915);
	Sound_set(&tcWaveRes[2], subchase_hit_raw, subchase_hit_raw_size, 1190);
	Sound_set(&tcWaveRes[3], subchase_sink_raw, subchase_sink_raw_size, 1654);

	// load images
	bmpScreen = GRRLIB_LoadTexture(subchase_screen_png);
	bmpBlipDim = GRRLIB_LoadTexture(subchase_blipdim_png);
	bmpBlipBright = GRRLIB_LoadTexture(subchase_blipbright_png);
	bmpDigits = GRRLIB_LoadTexture(subchase_digits_png);
	GRRLIB_InitTileSet(bmpDigits, DIGIT_W, DIGIT_H, 0);
	
	bmpPowerOff = GRRLIB_LoadTexture(subchase_poweroff_png);
	bmpPowerOn = GRRLIB_LoadTexture(subchase_poweron_png);

	// set blips 
	for (y = 0; y < SUBCHASE_BLIP_ROWS; y++){
		for (x = 0; x < SUBCHASE_BLIP_COLUMNS; x++){
			Blip_t *pblip = &blip[x][y];
			
			pblip->x = (int)((x * ((float)subchase_blip_xspacing/100)) + subchase_blip_x);
			pblip->y = (int)((y * ((float)subchase_blip_yspacing/100)) + subchase_blip_y);
			pblip->status = -1;
		}
	}
	// set digits
	for(x = 0; x < 2; x++) {
		digit[x].x = subchase_digit_x + x * subchase_digit_spacing;
		digit[x].y = subchase_digit_y;
	}
	
	if (!bInited)
	{
		PlatformSetInput(0);	
		// turn game on
		SubChase_PowerOn();

		bInited = TRUE;
	}
}

void SubChase_DeInit()
{
	// stop all sounds...
	Platform_StopSound();
	
	GRRLIB_FreeTexture(bmpScreen);
	GRRLIB_FreeTexture(bmpBlipDim);
	GRRLIB_FreeTexture(bmpBlipBright);
	GRRLIB_FreeTexture(bmpDigits);
	GRRLIB_FreeTexture(bmpPowerOff);
	GRRLIB_FreeTexture(bmpPowerOn);

	bInited = FALSE;
}

void SubChase_Paint()
{
	int x, y;
	BOOL power = SubChase_GetPower();
	int p_switch;
	p_switch = Platform_GetPowerSwitch(ONOFF_SWITCH);
	if(p_switch == -1 && power == TRUE) {
		SubChase_PowerOff();
	}
	if(p_switch == 1 && power == FALSE) {
		SubChase_PowerOn();
	}
	
	// paint the backdrop
	GRRLIB_DrawImg(realx(0), realy(0), bmpScreen, 0, 1, 1, 0xFFFFFFFF);
	
	// visualize the control states
	if (power)
	{
		GRRLIB_DrawImg(realx(subchase_power_x), realy(subchase_power_y), bmpPowerOn, 0, 1, 1, 0xFFFFFFFF);
	
		for (y = 0; y < SUBCHASE_BLIP_ROWS; y++){
			for (x = 0; x < SUBCHASE_BLIP_COLUMNS; x++){
				Blip_t *pblip = &blip[x][y];
				
				switch(pblip->status) {
				case BLIP_DIM:
					GRRLIB_DrawImg(realx(pblip->x), realy(pblip->y), bmpBlipDim, 0, 1, 1, 0xFFFFFFFF);
					break;
				case BLIP_BRIGHT:
					GRRLIB_DrawImg(realx(pblip->x), realy(pblip->y), bmpBlipBright, 0, 1, 1, 0xFFFFFFFF);
					break;
				case BLIP_OFF:
				default:
					break;
				}
			}
		}	
		
		for(x = 0; x < 2; x++) {
			GRRLIB_DrawTile(realx(digit[x].x), realy(digit[x].y), bmpDigits, 0, 1, 1, 0xFFFFFFFF, digit[x].val);
		}
	}
	else
	{
		GRRLIB_DrawImg(realx(subchase_power_x), realy(subchase_power_y), bmpPowerOff, 0, 1, 1, 0xFFFFFFFF);
	}
}

void SubChase_ClearScreen()
{
	Platform_StartDraw();

	// erase the blips
	for (int y = 0; y < SUBCHASE_BLIP_ROWS; y++){
		for (int x = 0; x < SUBCHASE_BLIP_COLUMNS; x++){
			SubChase_DrawBlip(BLIP_OFF, x, y);
		}
	}

	Platform_EndDraw();
}

void SubChase_PlaySound(int nSound, unsigned int nFlags)
{
	if (nFlags & PLAYSOUNDFLAGS_PRIORITY)
	{
		// stop any playing sounds first
		Platform_StopSound();
	}

	Platform_PlaySound(&tcWaveRes[nSound], nFlags);
}

void SubChase_StopSound()
{
	// stop all sounds...
	Platform_StopSound();
}


//----------------------------------------------------------------------------
// local fcn's
//

void SubChase_DrawBlip(int nBright, int x, int y)
{
	switch(nBright){
	case BLIP_OFF:
		blip[x][y].status = -1;
		break;
	case BLIP_DIM:
		blip[x][y].status = BLIP_DIM;
		break;
	case BLIP_BRIGHT:
		blip[x][y].status = BLIP_BRIGHT;
		break;
	}
}

void SubChase_DrawScore(int nScore)
{
	if (nScore == -1){
		// erase the display
		digit[0].val = 0;
		digit[1].val = 0;
	} else {
		// draw 10s place
		if (nScore >= 10){
			digit[0].val = (nScore/10) + 1;
		} else {
			digit[0].val = 0;
		}
		// draw 1s place
		digit[1].val = (nScore%10) + 1;
	}
}

BOOL SubChase_GetInputFIRE(BOOL *pChange)
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

BOOL SubChase_GetInputLEFT(BOOL *pChange)
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

BOOL SubChase_GetInputUP(BOOL *pChange)
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

BOOL SubChase_GetInputRIGHT(BOOL *pChange)
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

BOOL SubChase_GetInputDOWN(BOOL *pChange)
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

void SubChase_GetSize(int *w, int *h)
{
	*w = bmpScreen->w;
	*h = bmpScreen->h;
}

