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
along with this program (license.txt.h" if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

Email : peter@peterhirschberg.com
Website : http://www.peterhirschberg.com

*/

#include "autorace.h"
#include "slide1_png.h"
#include "skislalom_screen_png.h"
#include "skislalom_slide_png.h"
#include "autorace_screen_png.h"
#include "autorace_slide_png.h"
#include "autorace_gear1_raw.h"
#include "autorace_gear2_raw.h"
#include "autorace_gear3_raw.h"
#include "autorace_gear4_raw.h"
#include "autorace_hit_raw.h"
#include "autorace_time_raw.h"
#include "autorace_win_raw.h"
#include "skislalom_gear1_raw.h"
#include "skislalom_gear2_raw.h"
#include "skislalom_gear3_raw.h"
#include "skislalom_gear4_raw.h"
#include "skislalom_hit_raw.h"
#include "skislalom_time_raw.h"
#include "skislalom_win_raw.h"	
// images
static GRRLIB_texImg *bmpScreen;
static GRRLIB_texImg *bmpPowerSlide;
static GRRLIB_texImg *bmpSlide;

static int nStick = 1;

static BOOL bEngineSound = FALSE;
static BOOL bEngineSoundPlaying = FALSE;

// **************************************************
// Ski Slalom is the exact same game as Auto Race,
// except the screen is flipped. I'm going to cheat
// and reuse as much of the Auto Race code as I can.
// This flag is used to toggle between the two games.
static BOOL bSkiSlalom = FALSE;
// **************************************************

static void StartEngineSound();
static void StopEngineSound();

static Sound_t tcWaveRes[AUTORACE_SOUND_NSOUNDS];
static Blip_t blip[AUTORACE_BLIP_COLUMNS][AUTORACE_BLIP_ROWS];
static Stat_t digit[2];
static Coord_t i_slider, i_power, i_gear;
static Help_t autorace_help[] = {
	{ WK_1, { 87, 90} },
	{ WK_2, { 87, 144} },
	{ WK_BUD, {89, 220} },
	{ WK_LR, { 153, 264} }
};

static Help_t skislalom_help[] = {
	{ WK_1, { 87, 74} },
	{ WK_2, { 87, 148} },
	{ WK_BUD, {86, 205} },
	{ WK_LR, { 154, 285} }
};
//----------------------------------------------------------------------------
//
//
void AutoRace_Help()
{
	Platform_Help(autorace_help, sizeof(autorace_help)/sizeof(*autorace_help));
}

void SkiSlalom_Help()
{
	Platform_Help(skislalom_help, sizeof(skislalom_help)/sizeof(*skislalom_help));
}

static BOOL bInited = FALSE;

void SkiSlalom_Init()
{
	int x, y;

	bSkiSlalom = TRUE;

	// Init sounds
	Sound_set(&tcWaveRes[AUTORACE_SOUND_GEAR0], skislalom_gear1_raw, skislalom_gear1_raw_size, 1324);
	Sound_set(&tcWaveRes[AUTORACE_SOUND_GEAR1], skislalom_gear2_raw, skislalom_gear2_raw_size, 1051);
	Sound_set(&tcWaveRes[AUTORACE_SOUND_GEAR2], skislalom_gear3_raw, skislalom_gear3_raw_size, 1045);
	Sound_set(&tcWaveRes[AUTORACE_SOUND_GEAR3], skislalom_gear4_raw, skislalom_gear4_raw_size, 852);
	Sound_set(&tcWaveRes[AUTORACE_SOUND_HIT], skislalom_hit_raw, skislalom_hit_raw_size, 137);
	Sound_set(&tcWaveRes[AUTORACE_SOUND_TIME], skislalom_time_raw, skislalom_time_raw_size, 937);
	Sound_set(&tcWaveRes[AUTORACE_SOUND_WIN], skislalom_win_raw, skislalom_win_raw_size, 1275);

	// load images
	bmpScreen = GRRLIB_LoadTexture(skislalom_screen_png);
	bmpPowerSlide = GRRLIB_LoadTexture(slide1_png);
	bmpSlide = GRRLIB_LoadTexture(skislalom_slide_png);
	
	// images position
	i_slider.x = skislalom_slider_x;
	i_slider.y = skislalom_slider_y;
	i_power.x = skislalom_power_x;
	i_power.y = skislalom_power_y;
	i_gear.x = skislalom_gear_x;
	i_gear.y = skislalom_gear_y;
	
	// set blips 
	for (y = 0; y < AUTORACE_BLIP_ROWS; y++){
		for (x = 0; x < AUTORACE_BLIP_COLUMNS; x++){
			Blip_t *pblip = &blip[x][y];
			
			pblip->x = (int)((x * ((float)skislalom_blip_xspacing/100)) + skislalom_blip_x);
			pblip->y = (int)((y * ((float)skislalom_blip_yspacing/100)) + skislalom_blip_y);
			pblip->status = -1;
		}
	}
	
	// set digits
	for(x = 0; x < 2; x++) {
		digit[x].x = skislalom_digit_x + x * skislalom_digit_spacing;
		digit[x].y = skislalom_digit_y;
	}

	if (!bInited)
	{
		PlatformSetInput(0);
		
		// turn on the game
		AutoRace_SetSkill(0);

		// start with game off
		AutoRace_PowerOff();

		bInited = TRUE;
	}
}

void AutoRace_Init()
{
	int x, y;

	bSkiSlalom = FALSE;
	
	// Init sounds
	Sound_set(&tcWaveRes[0], autorace_gear1_raw, autorace_gear1_raw_size, 1324);
	Sound_set(&tcWaveRes[1], autorace_gear2_raw, autorace_gear2_raw_size, 1051);
	Sound_set(&tcWaveRes[2], autorace_gear3_raw, autorace_gear3_raw_size, 1045);
	Sound_set(&tcWaveRes[3], autorace_gear4_raw, autorace_gear4_raw_size, 852);
	Sound_set(&tcWaveRes[4], autorace_hit_raw, autorace_hit_raw_size, 137);
	Sound_set(&tcWaveRes[5], autorace_time_raw, autorace_time_raw_size, 937);
	Sound_set(&tcWaveRes[6], autorace_win_raw, autorace_win_raw_size, 1275);

	// load images
	bmpScreen = GRRLIB_LoadTexture(autorace_screen_png);
	bmpPowerSlide = GRRLIB_LoadTexture(slide1_png);
	bmpSlide = GRRLIB_LoadTexture(autorace_slide_png);
	
	// images position
	i_slider.x = autorace_slider_x;
	i_slider.y = autorace_slider_y;
	i_power.x = autorace_power_x;
	i_power.y = autorace_power_y;
	i_gear.x = autorace_gear_x;
	i_gear.y = autorace_gear_y;
	
	// set blips 
	for (y = 0; y < AUTORACE_BLIP_ROWS; y++){
		for (x = 0; x < AUTORACE_BLIP_COLUMNS; x++){
			Blip_t *pblip = &blip[x][y];
			
			pblip->x = (int)((x * ((float)autorace_blip_xspacing/100)) + autorace_blip_x);
			pblip->y = (int)((y * ((float)autorace_blip_yspacing/100)) + autorace_blip_y);
			pblip->status = -1;
		}
	}
	
	// set digits
	for(x = 0; x < 2; x++) {
		digit[x].x = autorace_digit_x + x * autorace_digit_spacing;
		digit[x].y = autorace_digit_y;
	}
	
	if (!bInited)
	{
		PlatformSetInput(0);
		
		// turn on the game
		AutoRace_SetSkill(0);

		// start with game off
		AutoRace_PowerOff();

		bInited = TRUE;
	}
}

void AutoRace_StopSound()
{
	bEngineSoundPlaying = FALSE;
	bEngineSound = FALSE;

	// stop all sounds...
	Platform_StopSound();
}

void AutoRace_DeInit()
{
	// stop all sounds...
	AutoRace_StopSound();
	AutoRace_PowerOff();
	GRRLIB_FreeTexture(bmpScreen);
	GRRLIB_FreeTexture(bmpPowerSlide);
	GRRLIB_FreeTexture(bmpSlide);
	destroy_gear();
	bInited = FALSE;
}

void AutoRace_Paint()
{
	int x, y;
	BOOL power = AutoRace_GetPower();
	int p_switch;
	p_switch = Platform_GetPowerSwitch(ONOFF_SWITCH);
	if(p_switch == -1 && power == TRUE) {
		AutoRace_PowerOff();
	}
	if(p_switch == 1 && power == FALSE) {
		AutoRace_PowerOn();
	}	
	// paint the backdrop
	GRRLIB_DrawImg(realx(0), realy(0), bmpScreen, 0, SCALE_X, SCALE_Y, 0xFFFFFFFF);
	
	// draw gear
	draw_gear(i_gear.x, i_gear.y, 4 - AutoRace_GetSkill());
	
	// Draw stick
	switch (nStick) {
	case 0:
		x = 2;
		break;
	case 2:
		x = 27;
		break;
	case 1:
	default:
		x = 15;
		break;
	}
	GRRLIB_DrawImg(realx(i_slider.x+x), realy(i_slider.y), bmpSlide, 0, SCALE_X, SCALE_Y, 0xFFFFFFFF);
		
	// Draw power	
	if (power) {
		GRRLIB_DrawImg(realx(i_power.x), realy(i_power.y+3), bmpPowerSlide, 0, SCALE_X, SCALE_Y, 0xFFFFFFFF);
		for (y = 0; y < AUTORACE_BLIP_ROWS; y++){
			for (x = 0; x < AUTORACE_BLIP_COLUMNS; x++){
				if(blip[x][y].status != -1)
					draw_vblip(blip[x][y].x, blip[x][y].y, blip[x][y].status);
			}
		}	
		for(x = 0; x < sizeof(digit)/sizeof(*digit); x++)
			draw_digit(digit[x].x, digit[x].y, digit[x].val);
	}
	else {
		GRRLIB_DrawImg(realx(i_power.x), realy(i_power.y+16), bmpPowerSlide, 0, SCALE_X, SCALE_Y, 0xFFFFFFFF);
	}

}

void AutoRace_ClearScreen()
{
	Platform_StartDraw();

	// erase the blips
	for (int y = 0; y < AUTORACE_BLIP_ROWS; y++){
		for (int x = 0; x < AUTORACE_BLIP_COLUMNS; x++){
			AutoRace_DrawBlip(BLIP_OFF, x, y);
		}
	}

	// erase the score display
	AutoRace_DrawTime(-1);

	Platform_EndDraw();
}

void AutoRace_PlaySound(int nSound, unsigned int nFlags)
{
	if ((nFlags & PLAYSOUNDFLAGS_PRIORITY) || bEngineSoundPlaying)
	{
		// stop any playing sounds first
		Platform_StopSound();
	}

	// this sound will cut off any looping sounds
	// note this so we can restart them later
	bEngineSoundPlaying = FALSE;

	Platform_PlaySound(&tcWaveRes[nSound], nFlags);
}

//----------------------------------------------------------------------------
// local fcn's
//
static void StartEngineSound()
{
	// if gear has changed, switch sounds
	static int nOldGear = -1;
	int nGear = AutoRace_GetSkill();
	if (nGear != nOldGear)
	{
		AutoRace_StopEngineSound();
		nOldGear = nGear;
	}

	if (!bEngineSoundPlaying)
	{
		int nSound = AutoRace_GetSkill();

		unsigned int nSoundFlags = PLAYSOUNDFLAGS_LOOP | PLAYSOUNDFLAGS_ASYNC;
		Platform_PlaySound(&tcWaveRes[nSound], nSoundFlags);
		// mark the sound as playing
		bEngineSoundPlaying = TRUE;
	}
}

static void StopEngineSound()
{
	if (bEngineSoundPlaying)
	{
		Platform_StopSound();
		bEngineSoundPlaying = FALSE;
	}
}

void AutoRace_PlayEngineSound()
{
	if (!bEngineSound)
	{
		bEngineSound = TRUE;
		StartEngineSound();
	}
}

void AutoRace_StopEngineSound()
{
	if (bEngineSound)
	{
		bEngineSound = FALSE;
		StopEngineSound();
	}
}

void AutoRace_DrawBlip(int nBright, int x, int y)
{
	if (bSkiSlalom)
	{
		// flip the screen for ski slalom
		y = (AUTORACE_BLIP_ROWS - 1) - y;
	}

	switch(nBright){
	case BLIP_DIM:
		blip[x][y].status = BLIP_TYPE_NORMAL;
		break;
	case BLIP_BRIGHT:
		blip[x][y].status = BLIP_TYPE_BRIGHT;
		break;
	case BLIP_OFF:
	default:
		blip[x][y].status = -1;
		break;
	}
	
	// update the looped engine sound
	// this is not a good place for this
	// should be in some sort of draw frame function
	// maybe need to add that to the game structure
	if (bEngineSound){
		StartEngineSound();
	} else {
		StopEngineSound();
	}

}

void AutoRace_DrawTime(int nTime)
{
	if (nTime == -1){
		// erase the display
		digit[0].val = -1;
		digit[1].val = -1;
	} else {
		// draw 10s place
		if(nTime >= 10)
			digit[0].val = nTime/10;
		else
			digit[0].val = -1;
		// draw 1s place
		digit[1].val = nTime%10 ;		
	}
}

int AutoRace_GetInputSTICK()
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

	// leave it where it was
//	return nStick;
	if (nStick != 1)
	{
		nStick = 1;
	}
	return nStick;
}

int AutoRace_GetInputGEAR(BOOL *pChange)
{
	static BOOL bLastB = FALSE;
	static BOOL bLastC = FALSE;

	if (pChange){ *pChange = FALSE; }

	int gear = AutoRace_GetSkill();

	// check the keys
	if (Platform_GetInput1())
	{
		if (!bLastB)
		{
			// shift up
			if (gear < 3)
			{
				++gear;
				AutoRace_SetSkill(gear);
				if (pChange)
				{
					*pChange = TRUE;
				}
			}
			bLastB = TRUE;
		}
	}
	else
	{
		bLastB = FALSE;
	}

	if (Platform_GetInput2())
	{
		if (!bLastC)
		{
			// shift down
			if (gear > 0)
			{
				--gear;
				AutoRace_SetSkill(gear);
				if (pChange)
				{
					*pChange = TRUE;
				}
			}
			bLastC = TRUE;
		}
	}
	else
	{
		bLastC = FALSE;
	}

	return gear;
}

void AutoRace_GetSize(int *w, int *h)
{
	*w = bmpScreen->w;
	*h = bmpScreen->h;
}
