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

#include "armorbattle.h"
#include "armorbattle_screen_png.h"
#include "armorbattle_blipdim_png.h"
#include "armorbattle_blipbright_png.h"
#include "armorbattle_digits_png.h"
#include "armorbattle_poweroff_png.h"
#include "armorbattle_poweron_png.h"
#include "armorbattle_tick_raw.h"
#include "armorbattle_near_raw.h"
#include "armorbattle_enemy_raw.h"
#include "armorbattle_fire_raw.h"
#include "armorbattle_hit_raw.h"
#include "armorbattle_score_raw.h"
#include "armorbattle_endgame_raw.h"

#define DIGIT_W	24
#define DIGIT_H 32
	
// images
static GRRLIB_texImg *bmpScreen;
static GRRLIB_texImg *bmpBlipDim;
static GRRLIB_texImg *bmpBlipBright;
static GRRLIB_texImg *bmpDigits;
static GRRLIB_texImg *bmpPowerOff;
static GRRLIB_texImg *bmpPowerOn;

static BOOL bMineSound = FALSE;
static BOOL bMineSoundPlaying = FALSE;

static Sound_t tcWaveRes[ARMORBATTLE_SOUND_NSOUNDS];
static Blip_t blip[ARMORBATTLE_BLIP_COLUMNS][ARMORBATTLE_BLIP_ROWS];
static Stat_t digit[2];
static Help_t help[] = {
	{ WK_2, {1, 131} },
	{ WK_BUD, {74, 232} },
	{ WK_DPAD, { 147, 247} }
};
//----------------------------------------------------------------------------
//
//
void ArmorBattle_Help()
{
	Platform_Help(help, sizeof(help)/sizeof(*help));
}

static BOOL bInited = FALSE;

void ArmorBattle_Init()
{
	int x, y;

	// Init sounds
	Sound_set(&tcWaveRes[ARMORBATTLE_SOUND_TICK], armorbattle_tick_raw, armorbattle_tick_raw_size, 17 );
	Sound_set(&tcWaveRes[ARMORBATTLE_SOUND_NEAR], armorbattle_near_raw, armorbattle_near_raw_size, 165 );
	Sound_set(&tcWaveRes[ARMORBATTLE_SOUND_ENEMY], armorbattle_enemy_raw, armorbattle_enemy_raw_size, 124 );
	Sound_set(&tcWaveRes[ARMORBATTLE_SOUND_FIRE], armorbattle_fire_raw, armorbattle_fire_raw_size, 660 );
	Sound_set(&tcWaveRes[ARMORBATTLE_SOUND_HIT], armorbattle_hit_raw, armorbattle_hit_raw_size, 524 );
	Sound_set(&tcWaveRes[ARMORBATTLE_SOUND_SCORE], armorbattle_score_raw, armorbattle_score_raw_size, 1542 );
	Sound_set(&tcWaveRes[ARMORBATTLE_SOUND_ENDGAME], armorbattle_endgame_raw, armorbattle_endgame_raw_size, 5244 );

	// load images
	bmpScreen = GRRLIB_LoadTexture(armorbattle_screen_png);
	bmpDigits = GRRLIB_LoadTexture(armorbattle_digits_png);
	GRRLIB_InitTileSet(bmpDigits, DIGIT_W, DIGIT_H, 0);
	bmpPowerOff = GRRLIB_LoadTexture(armorbattle_poweroff_png);
	bmpPowerOn = GRRLIB_LoadTexture(armorbattle_poweron_png);
	bmpBlipDim = GRRLIB_LoadTexture(armorbattle_blipdim_png);
	bmpBlipBright = GRRLIB_LoadTexture(armorbattle_blipbright_png);

	// grab some other values from the skin file
	for (y = 0; y < ARMORBATTLE_BLIP_ROWS; y++){
		for (x = 0; x < ARMORBATTLE_BLIP_COLUMNS; x++){
			Blip_t *pblip = &blip[x][y];
			pblip->x = (int)((x * ((float)armorbattle_blip_xspacing/100)) + armorbattle_blip_x);
			pblip->y = (int)((y * ((float)armorbattle_blip_yspacing/100)) + armorbattle_blip_y);
			pblip->status = -1;
		}
	}
	
	// set digits
	for(x = 0; x < 2; x++) {
		digit[x].x = armorbattle_digit_x + x * armorbattle_digit_spacing;
		digit[x].y = armorbattle_digit_y;
	}
	
	if (!bInited)
	{
		PlatformSetInput(0);
		
		// turn game on
		ArmorBattle_PowerOn();

		bInited = TRUE;
	}
}

void ArmorBattle_StopSound()
{
	bMineSoundPlaying = FALSE;
	bMineSound = FALSE;

	// stop all sounds...
	Platform_StopSound();
}

void ArmorBattle_DeInit()
{
	// stop all sounds...
	ArmorBattle_StopSound();
	ArmorBattle_PowerOff();
	GRRLIB_FreeTexture(bmpScreen);
	GRRLIB_FreeTexture(bmpBlipDim);
	GRRLIB_FreeTexture(bmpBlipBright);
	GRRLIB_FreeTexture(bmpDigits);
	GRRLIB_FreeTexture(bmpPowerOff);
	GRRLIB_FreeTexture(bmpPowerOn);
	
	bInited = FALSE;
}

void ArmorBattle_Paint()
{
	int x, y;
	BOOL power = ArmorBattle_GetPower();
	int p_switch;

	p_switch = Platform_GetPowerSwitch(ONOFF_SWITCH);
	if(p_switch == -1 && power == TRUE) {
		ArmorBattle_PowerOff();
	}
	if(p_switch == 1 && power == FALSE) {
		ArmorBattle_PowerOn();
	}
	
	GRRLIB_DrawImg (realx(0), realy(0), bmpScreen, 0, SCALE_X, SCALE_Y, 0xFFFFFFFF);
	
	// visualize the control states
	if (power)
	{
		GRRLIB_DrawImg(realx(armorbattle_power_x), realy(armorbattle_power_y), bmpPowerOn, 0, SCALE_X, SCALE_Y, 0xFFFFFFFF);
		
		// show blips 
		for (y = 0; y < ARMORBATTLE_BLIP_ROWS; y++){
			for (x = 0; x < ARMORBATTLE_BLIP_COLUMNS; x++){
				Blip_t *pblip = &blip[x][y];
				switch(pblip->status) {
				case BLIP_OFF:
					break;
				case BLIP_DIM:
					GRRLIB_DrawImg (realx(pblip->x), realy(pblip->y), bmpBlipDim, 0, SCALE_X, SCALE_Y, 0xFFFFFFFF);
					break;
				case BLIP_BRIGHT:
					GRRLIB_DrawImg (realx(pblip->x), realy(pblip->y), bmpBlipBright, 0, SCALE_X, SCALE_Y, 0xFFFFFFFF);
					break;
				}
			}
		}

		// Show time
		for(x = 0; x < sizeof(digit)/sizeof(*digit); x++) {
			if(digit[x].val == -1)
				GRRLIB_DrawTile(realx(digit[x].x), realy(digit[x].y), bmpDigits, 0, SCALE_X, SCALE_Y, 0xFFFFFFFF, 0);	
			else
				GRRLIB_DrawTile(realx(digit[x].x), realy(digit[x].y), bmpDigits, 0, SCALE_X, SCALE_Y, 0xFFFFFFFF, digit[x].val);
		}
	}
	else
	{
		GRRLIB_DrawImg(realx(armorbattle_power_x), realy(armorbattle_power_y), bmpPowerOff, 0, 1, 1, 0xFFFFFFFF);
	}
	

}

void ArmorBattle_ClearScreen()
{
	Platform_StartDraw();

	// erase the blips
	for (int y = 0; y < ARMORBATTLE_BLIP_ROWS; y++){
		for (int x = 0; x < ARMORBATTLE_BLIP_COLUMNS; x++){
			ArmorBattle_DrawBlip(BLIP_OFF, x, y);
		}
	}

	// erase the score display
	ArmorBattle_DrawTime(-1);

	Platform_EndDraw();
}

void ArmorBattle_PlaySound(int nSound, unsigned int nFlags)
{
	if (bMineSoundPlaying && !(nFlags & PLAYSOUNDFLAGS_PRIORITY))
	{
		// go away
		return;
	}

	if ((nFlags & PLAYSOUNDFLAGS_PRIORITY) || bMineSoundPlaying)
	{
		// stop any playing sounds first
		Platform_StopSound();
	}

	// this sound will cut off any looping sounds
	// note this so we can restart them later
	bMineSoundPlaying = FALSE;

	Platform_PlaySound(&tcWaveRes[nSound], nFlags);
}

//----------------------------------------------------------------------------
// local fcn's
//

static void StartMineSound()
{
	if (!bMineSoundPlaying)
	{
		Platform_PlaySound(&tcWaveRes[ARMORBATTLE_SOUND_NEAR ], PLAYSOUNDFLAGS_LOOP | PLAYSOUNDFLAGS_ASYNC);
		// mark the sound as playing
		bMineSoundPlaying = TRUE;
	}
}

static void StopMineSound()
{
	if (bMineSoundPlaying)
	{
		Platform_StopSound();
		bMineSoundPlaying = FALSE;
	}
}

void ArmorBattle_PlayMineSound()
{
	if (!bMineSound)
	{
		bMineSound = TRUE;
		StartMineSound();
	}
}

void ArmorBattle_StopMineSound()
{
	if (bMineSound)
	{
		bMineSound = FALSE;
		StopMineSound();
	}
}

void ArmorBattle_DrawBlip(int nBright, int x, int y)
{
	blip[x][y].status = nBright;

	// update the looped mine sound
	// this is not a good place for this
	// should be in some sort of draw frame function
	// maybe need to add that to the game structure
	if (bMineSound){
		StartMineSound();
	} else {
		StopMineSound();
	}
}

void ArmorBattle_DrawTime(int nTime)
{
	if (nTime == -1){
		// erase the display
		digit[0].val = -1;
		digit[1].val = -1;
	} else {
		// draw 10s place
		if (nTime >= 10) 
			digit[0].val = nTime/10 + 1;
		else
			digit[0].val = -1;
		// draw 1s place
		digit[1].val = nTime%10 + 1;		
	}
}

BOOL ArmorBattle_GetInputFIRE(BOOL *pChange)
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

BOOL ArmorBattle_GetInputLEFT(BOOL *pChange)
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

BOOL ArmorBattle_GetInputUP(BOOL *pChange)
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

BOOL ArmorBattle_GetInputRIGHT(BOOL *pChange)
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

BOOL ArmorBattle_GetInputDOWN(BOOL *pChange)
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

void ArmorBattle_GetSize(int *w, int *h)
{
	*w = bmpScreen->w;
	*h = bmpScreen->h;
}

