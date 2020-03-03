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

#include "spacealert.h"
#include "spacealert_screen_png.h"
#include "spacealert_poweron_png.h"
#include "spacealert_poweroff_png.h"
#include "spacealert_aimleft_png.h"
#include "spacealert_aimcenter_png.h"
#include "spacealert_aimright_png.h"
#include "spacealert_fire_raw.h"
#include "spacealert_hit_raw.h"
#include "spacealert_lose_raw.h"
#include "spacealert_win_raw.h"
#include "spacealert_raider_raw.h"

// images
static GRRLIB_texImg *bmpScreen;
static GRRLIB_texImg *bmpPowerOn;
static GRRLIB_texImg *bmpPowerOff;
static GRRLIB_texImg *bmpAimLeft;
static GRRLIB_texImg *bmpAimCenter;
static GRRLIB_texImg *bmpAimRight;

static int nStick = 1;

static BOOL bRaiderSound = FALSE;
static BOOL bRaiderSoundPlaying = FALSE;

static void StartRaiderSound();
static void StopRaiderSound();

static Sound_t tcWaveRes[5];
static Blip_t blip[SPACEALERT_BLIP_COLUMNS][SPACEALERT_BLIP_ROWS];
static Stat_t digit[2];
static Help_t help[] = {
	{ WK_2, { 86, 122 } },
	{ WK_BUD, { 88, 224 } },
	{ WK_LR, { 156, 265 } }
};
//----------------------------------------------------------------------------
//
//
void SpaceAlert_Help()
{
	Platform_Help(help, sizeof(help)/sizeof(*help));
}

static BOOL bInited = FALSE;

void SpaceAlert_Init()
{
	int x, y;
	if (bInited) return;

	// Init sounds
	Sound_set(&tcWaveRes[0], spacealert_fire_raw, spacealert_fire_raw_size, 109);
	Sound_set(&tcWaveRes[1], spacealert_hit_raw, spacealert_hit_raw_size, 284);
	Sound_set(&tcWaveRes[2], spacealert_lose_raw, spacealert_lose_raw_size, 1243);
	Sound_set(&tcWaveRes[3], spacealert_win_raw, spacealert_win_raw_size, 850);
	Sound_set(&tcWaveRes[4], spacealert_raider_raw, spacealert_raider_raw_size, 3902);
	
	// load images
	bmpScreen = GRRLIB_LoadTexture(spacealert_screen_png);
	bmpPowerOn = GRRLIB_LoadTexture(spacealert_poweron_png);
	bmpPowerOff = GRRLIB_LoadTexture(spacealert_poweroff_png);
	bmpAimLeft = GRRLIB_LoadTexture(spacealert_aimleft_png);
	bmpAimCenter = GRRLIB_LoadTexture(spacealert_aimcenter_png);
	bmpAimRight = GRRLIB_LoadTexture(spacealert_aimright_png);

	// set blips 
	for (y = 0; y < SPACEALERT_BLIP_ROWS; y++){
		for (x = 0; x < SPACEALERT_BLIP_COLUMNS; x++){
			Blip_t *pblip = &blip[x][y];
	
			pblip->x = (int)((x * ((float)spacealert_blip_xspacing/100)) + spacealert_blip_x);
			pblip->y = (int)((y * ((float)spacealert_blip_yspacing/100)) + spacealert_blip_y);
			pblip->status = -1;
		}
	}
	// set digits
	for(x = 0; x < 2; x++) {
		digit[x].x = spacealert_digit_x + x * spacealert_digit_spacing;
		digit[x].y = spacealert_digit_y;
	}
	PlatformSetInput(0);
	// start with the game off
	SpaceAlert_PowerOff();

	bInited = TRUE;
}

void SpaceAlert_DeInit()
{
	// stop all sounds...
	Platform_StopSound();
	bRaiderSoundPlaying = FALSE;
	
	GRRLIB_FreeTexture(bmpScreen);
	GRRLIB_FreeTexture(bmpPowerOff);
	GRRLIB_FreeTexture(bmpPowerOn);
	GRRLIB_FreeTexture(bmpAimLeft);
	GRRLIB_FreeTexture(bmpAimCenter);
	GRRLIB_FreeTexture(bmpAimRight);

	bInited = FALSE;
}

void SpaceAlert_Paint()
{
	int x, y;
	BOOL power = SpaceAlert_GetPower();
	int p_switch;
	p_switch = Platform_GetPowerSwitch(ONOFF_SWITCH);
	if(p_switch == -1 && power == TRUE) {
		SpaceAlert_PowerOff();
	}
	if(p_switch == 1 && power == FALSE) {
		SpaceAlert_PowerOn();
	}
	
	// paint the backdrop
	GRRLIB_DrawImg(realx(0), realy(0), bmpScreen, 0, 1, 1, 0xFFFFFFFF);
	
	// visualize the control states
	if (power){
		GRRLIB_DrawImg(realx(spacealert_power_x), realy(spacealert_power_y), bmpPowerOn, 0, 1, 1, 0xFFFFFFFF);
	
		for (y = 0; y < SPACEALERT_BLIP_ROWS; y++){
			for (x = 0; x < SPACEALERT_BLIP_COLUMNS; x++){
				Blip_t *pblip = &blip[x][y];
				if(pblip->status != -1)
					draw_vblip(pblip->x, pblip->y, pblip->status);
			}
		}
		// Draw points
		for(x = 0; x < 2; x++)
			draw_digit(digit[x].x, digit[x].y, digit[x].val);	
		// Draw stick
		switch (nStick)
		{
		case 0:
			GRRLIB_DrawImg(realx(spacealert_slider_x), realy(spacealert_slider_y), bmpAimLeft, 0, 1, 1, 0xFFFFFFFF);
			break;
		case 1:
			GRRLIB_DrawImg(realx(spacealert_slider_x), realy(spacealert_slider_y), bmpAimCenter, 0, 1, 1, 0xFFFFFFFF);
			break;
		case 2:
			GRRLIB_DrawImg(realx(spacealert_slider_x), realy(spacealert_slider_y), bmpAimRight, 0, 1, 1, 0xFFFFFFFF);
			break;
		}
	} 
	else {
		GRRLIB_DrawImg(realx(spacealert_power_x), realy(spacealert_power_y), bmpPowerOff, 0, 1, 1, 0xFFFFFFFF);
	}

}

void SpaceAlert_ClearScreen()
{
	Platform_StartDraw();

	// erase the blips
	for (int y = 0; y < SPACEALERT_BLIP_ROWS; y++){
		for (int x = 0; x < SPACEALERT_BLIP_COLUMNS; x++){
			SpaceAlert_DrawBlip(BLIP_OFF, x, y);
		}
	}

	// erase the score display
	SpaceAlert_DrawScore(-1);

	Platform_EndDraw();
}

void SpaceAlert_PlaySound(int nSound, unsigned int nFlags)
{
	if ((nFlags & PLAYSOUNDFLAGS_PRIORITY)  || bRaiderSoundPlaying)
	{
		// stop any playing sounds first
		Platform_StopSound();
	}

	// this sound will cut off any looping sounds
	// note this so we can restart them later
	bRaiderSoundPlaying = FALSE;

	Platform_PlaySound(&tcWaveRes[nSound], nFlags);
}

void SpaceAlert_StopSound()
{
	bRaiderSoundPlaying = FALSE;
	bRaiderSound = FALSE;

	// stop all sounds...
	Platform_StopSound();
}


//----------------------------------------------------------------------------
// local fcn's
//

static void StartRaiderSound()
{
	if (!bRaiderSoundPlaying)
	{
		
		unsigned int nSoundFlags = PLAYSOUNDFLAGS_LOOP | PLAYSOUNDFLAGS_ASYNC;
		Platform_PlaySound(&tcWaveRes[SPACEALERT_SOUND_RAIDER], nSoundFlags);
		// mark the sound as playing
		bRaiderSoundPlaying = TRUE;
	}
}

static void StopRaiderSound()
{
	if (bRaiderSoundPlaying)
	{
		Platform_StopSound();
		bRaiderSoundPlaying = FALSE;
	}
}

void SpaceAlert_PlayRaiderSound()
{
	if (!bRaiderSound)
	{
		bRaiderSound = TRUE;
		StartRaiderSound();
	}
}

void SpaceAlert_StopRaiderSound()
{
	if (bRaiderSound)
	{
		bRaiderSound = FALSE;
		StopRaiderSound();
	}
}

void SpaceAlert_DrawBlip(int nBright, int x, int y)
{
	switch(nBright){
	case BLIP_OFF:
		blip[x][y].status = -1;
		break;
	case BLIP_DIM:
		blip[x][y].status = BLIP_TYPE_NORMAL;
		break;
	case BLIP_BRIGHT:
		blip[x][y].status = BLIP_TYPE_BRIGHT;
		break;
	}

	// update the looped raider sound
	// this is not a good place for this
	// should be in some sort of draw frame function
	// maybe need to add that to the game structure
	if (bRaiderSound){
		StartRaiderSound();
	} else {
		StopRaiderSound();
	}

}

void SpaceAlert_DrawScore(int nScore)
{
	if (nScore == -1){
		// erase the display
		digit[0].val = -1;
		digit[1].val = -1;
	} else {
		// draw 10s place
		digit[0].val = nScore/10;
		// draw 1s place
		digit[1].val = nScore%10;		
	}
}

int SpaceAlert_GetInputSTICK()
{
	// check the keys
	if (Platform_GetInputLEFT()
		&& !Platform_GetInputRIGHT())
	{
		if (nStick != 0)
		{
			nStick = 0;
		}
		return nStick;
	}
	if (Platform_GetInputRIGHT()
		&& !Platform_GetInputLEFT())
	{
		if (nStick != 2)
		{
			nStick = 2;
		}
		return nStick;
	}

	if (nStick != 1)
	{
		nStick = 1;
	}
	return nStick;
}

BOOL SpaceAlert_GetInputFIRE(BOOL *pChange)
{
	static BOOL bLast = FALSE;

	if (pChange){ *pChange = FALSE; }

	// check the keys
	if (Platform_GetInput2())
	{
		if (!bLast)
		{
			if (pChange)
			{
				*pChange = TRUE;
			}
		}
		bLast = TRUE;
		return TRUE;
	}

	bLast = FALSE;

	return FALSE;
}

void SpaceAlert_GetSize(int *w, int *h)
{
	*w = bmpScreen->w;
	*h = bmpScreen->h;
}


