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

#include "baseball.h"
#include "baseball_screen_png.h"
#include "baseball_blip_png.h"
#include "baseball_dimdigits_png.h"
#include "baseball_brightdigits_png.h"
#include "baseball_hit_raw.h"
#include "baseball_out_raw.h"
#include "baseball_run_raw.h"
#include "baseball_strike_raw.h"
#include "baseball_endpossession_raw.h"
#include "baseball_endgame_raw.h"

#define DIGIT_W		8
#define DIGIT_H		12

#define DIGIT_TYPE_OFF		0	
#define DIGIT_TYPE_DIM		1
#define DIGIT_TYPE_BRIGHT	2
int Baseball_errno;

static BOOL bPower = FALSE;
static BOOL bPro2 = FALSE;
static BOOL bInited = FALSE;

// images
static GRRLIB_texImg *bmpScreen;
static GRRLIB_texImg *bmpBlip;
static GRRLIB_texImg *bmpDimDigits;
static GRRLIB_texImg *bmpBrightDigits;

static Sound_t tcWaveRes[BASEBALL_SOUND_NSOUNDS];

static Digit_t digit[4] = {
	{ baseball_digit1_x, baseball_digit_y, 0, DIGIT_TYPE_OFF},
	{ baseball_digit2_x, baseball_digit_y, 0, DIGIT_TYPE_OFF},
	{ baseball_digit3_x, baseball_digit_y, 0, DIGIT_TYPE_OFF},
	{ baseball_digit4_x, baseball_digit_y, 0, DIGIT_TYPE_OFF}
};

#define N_BLIPS	(sizeof(blips)/sizeof(*blips))
static Blip_t blips[BASEBALL_NBLIPS] = {
	{ baseball_blip_pitch_1_x, baseball_blip_pitch_1_y, BLIP_OFF },				// 0 blips for normal pitches	
	{ baseball_blip_pitch_2_x, baseball_blip_pitch_2_y, BLIP_OFF },
	{ baseball_blip_pitch_3_x, baseball_blip_pitch_3_y, BLIP_OFF },
	{ baseball_blip_pitch_4_x, baseball_blip_pitch_4_y, BLIP_OFF },
	{ baseball_blip_pitch_5_x, baseball_blip_pitch_5_y, BLIP_OFF },
	{ baseball_blip_pitch_6_x, baseball_blip_pitch_6_y, BLIP_OFF },
	{ baseball_blip_pitch_7_x, baseball_blip_pitch_7_y, BLIP_OFF },
	{ baseball_blip_pitch_8_x, baseball_blip_pitch_8_y, BLIP_OFF },
	{ baseball_blip_pitch_9_x, baseball_blip_pitch_9_y, BLIP_OFF },
	{ baseball_blip_pitchcurve_2_x, baseball_blip_pitchcurve_2_y, BLIP_OFF },	// 9 blips for curve balls
	{ baseball_blip_pitchcurve_3_x, baseball_blip_pitchcurve_3_y, BLIP_OFF },
	{ baseball_blip_pitchcurve_4_x, baseball_blip_pitchcurve_4_y, BLIP_OFF },
	{ baseball_blip_pitchcurve_5_x, baseball_blip_pitchcurve_5_y, BLIP_OFF },
	{ baseball_blip_base_0_x,baseball_blip_base_0_y, BLIP_OFF },				// 13 blips for home plate -> 1st base
	{ baseball_blip_base_0A_x,baseball_blip_base_0A_y, BLIP_OFF },
	{ baseball_blip_base_0B_x,baseball_blip_base_0B_y, BLIP_OFF },
	{ baseball_blip_base_1_x,baseball_blip_base_1_y, BLIP_OFF },				// 16 blips for 1st base -> 2nd base 
	{ baseball_blip_base_1A_x,baseball_blip_base_1A_y, BLIP_OFF },
	{ baseball_blip_base_1B_x,baseball_blip_base_1B_y, BLIP_OFF },
	{ baseball_blip_base_2_x,baseball_blip_base_2_y, BLIP_OFF },				// 19 blips for 2nd base -> 3rd base
	{ baseball_blip_base_2A_x,baseball_blip_base_2A_y, BLIP_OFF },
	{ baseball_blip_base_2B_x,baseball_blip_base_2B_y, BLIP_OFF },
	{ baseball_blip_base_3_x,baseball_blip_base_3_y, BLIP_OFF },				// 22 blips for 3rd base -> home plate
	{ baseball_blip_base_3A_x,baseball_blip_base_3A_y, BLIP_OFF },
	{ baseball_blip_base_3B_x,baseball_blip_base_3B_y, BLIP_OFF },
	{ baseball_blip_deep1st_x,baseball_blip_deep1st_y, BLIP_OFF },				// 25 blip for deep 1st	
	{ baseball_blip_deep3rd_x,baseball_blip_deep3rd_y, BLIP_OFF },				// 26 blip for deep 3rd
	{ baseball_blip_outfield_left_x,baseball_blip_outfield_left_y, BLIP_OFF },	// 27 blips for outfield left
	{ baseball_blip_outfield_center_x,baseball_blip_outfield_center_y, BLIP_OFF },
	{ baseball_blip_outfield_right_x,baseball_blip_outfield_right_y, BLIP_OFF }	// 29 blips for outfield right
};

static Help_t help[] = {
	{ WK_1, {195, 247} },
	{ WK_2, {195, 280} },
	{ WK_PLUS, {22, 247} },
	{ WK_MINUS, {22, 280} },
	{ WK_BLR, {94, 258} }
};
//----------------------------------------------------------------------------
//
//
static void Baseball_StopSound()
{
	// stop all sounds...
	Platform_StopSound();
}

void Baseball_Help()
{
	Platform_Help(help, sizeof(help)/sizeof(*help));
}

BOOL Baseball_GetPower()
{
	return bPower;
}

void Baseball_PowerOn()
{
	bPower = TRUE;
}

void Baseball_PowerOff()
{
	bPower = FALSE;
	Baseball_StopSound();
}

void Baseball_SetSkill(int v)
{
	bPro2 = (v == 0) ? FALSE : TRUE;
}

int Baseball_GetSkill()
{
	return (bPro2 == TRUE) ? 1 : 0;
}
	
void Baseball_Init()
{
	if (bInited) 
		return;

	// Init sounds
	Sound_set(&tcWaveRes[BASEBALL_SOUND_HIT], baseball_hit_raw, baseball_hit_raw_size, 82 );
	Sound_set(&tcWaveRes[BASEBALL_SOUND_OUT], baseball_out_raw, baseball_out_raw_size, 392 );
	Sound_set(&tcWaveRes[BASEBALL_SOUND_RUN], baseball_run_raw, baseball_run_raw_size, 423 );
	Sound_set(&tcWaveRes[BASEBALL_SOUND_STRIKE], baseball_strike_raw, baseball_strike_raw_size, 22 );
	Sound_set(&tcWaveRes[BASEBALL_SOUND_ENDPOSSESSION], baseball_endpossession_raw, baseball_endpossession_raw_size, 560 );
	Sound_set(&tcWaveRes[BASEBALL_SOUND_ENDGAME], baseball_endgame_raw, baseball_endgame_raw_size, 712 );

	// load images
	bmpScreen = GRRLIB_LoadTexture(baseball_screen_png);
	bmpBlip = GRRLIB_LoadTexture(baseball_blip_png);
	bmpDimDigits = GRRLIB_LoadTexture(baseball_dimdigits_png);
	GRRLIB_InitTileSet(bmpDimDigits, DIGIT_W, DIGIT_H, 0);
	bmpBrightDigits = GRRLIB_LoadTexture(baseball_brightdigits_png);
	GRRLIB_InitTileSet(bmpBrightDigits, DIGIT_W, DIGIT_H, 0);

	PlatformSetInput(0);
	// turn on the game
	bPro2 = FALSE;
	bPower = FALSE;

	bInited = TRUE;
}

void Baseball_DeInit()
{
	// stop all sounds...
	Baseball_StopSound();
	bPower = FALSE;
	GRRLIB_FreeTexture(bmpScreen);
	GRRLIB_FreeTexture(bmpBlip);
	GRRLIB_FreeTexture(bmpDimDigits);
	GRRLIB_FreeTexture(bmpBrightDigits);
	bInited = FALSE;
}

void Baseball_Paint()
{
	int p_switch;
	p_switch = Platform_GetPowerSwitch(ONOFF_OFF12);
	if(bPower) {
		if(p_switch == -1) {
			if(bPro2 == FALSE)
				bPower = FALSE;
			if(bPro2 == TRUE) {
				bPower = TRUE;
				bPro2 = FALSE;
			}		
		} 
		if(p_switch == 1) {
			if(bPro2 == 0) {
				bPower = TRUE;
				bPro2 = TRUE;
			}
		} 
	}
	else {
		if(p_switch == 1) {
			bPower = TRUE;
			bPro2 = FALSE;
		} 
	}
	
	// paint the backdrop
	GRRLIB_DrawImg(realx(0), realy(0), bmpScreen, 0, SCALE_X, SCALE_Y, 0xFFFFFFFF);
	
	if (bPower == TRUE) {
		int i;
		if (bPro2 == FALSE) {
			draw_switch(SWITCH_TYPE_1, baseball_pro_1_x, baseball_pro_1_y, SWITCH_POS_PRO1);
		} 
		else {
			draw_switch(SWITCH_TYPE_1, baseball_pro_1_x, baseball_pro_1_y, SWITCH_POS_PRO2);
		}
		// show blips
		for(i=0; i< BASEBALL_NBLIPS; i++) {
			Blip_t *pblip = &blips[i];
			if(pblip->status != BLIP_OFF)
				GRRLIB_DrawImg(realx(pblip->x), realy(pblip->y), bmpBlip, 0, SCALE_X, SCALE_Y, 0xFFFFFFFF);
		}
		
		// show digits
		for(i=0; i<4; i++) {
			Digit_t *pdigit = &digit[i];
			
			switch(pdigit->type) {
			case DIGIT_TYPE_DIM:
				GRRLIB_DrawTile(realx(pdigit->x), realy(pdigit->y), bmpDimDigits, 0, SCALE_X, SCALE_Y, 0xFFFFFFFF, pdigit->val + 1);
				break;
			case DIGIT_TYPE_BRIGHT:
				GRRLIB_DrawTile(realx(pdigit->x), realy(pdigit->y), bmpBrightDigits, 0, SCALE_X, SCALE_Y, 0xFFFFFFFF, pdigit->val + 1);
				break;
			case DIGIT_TYPE_OFF:
			default:
				break;
			}
		}
	}
	else {
		Baseball_StopSound();
		draw_switch(SWITCH_TYPE_1, baseball_pro_1_x, baseball_pro_1_y, SWITCH_POS_OFF);
	}
}

void Baseball_PlaySound(int nSound, unsigned int nFlags)
{
	Platform_PlaySound(&tcWaveRes[nSound], nFlags);
}

//----------------------------------------------------------------------------
// local fcn's
//
void Baseball_DrawBlip(BOOL state, int index)
{
	Blip_t *pblip = &blips[index];

	pblip->status = (state) ? BLIP_DIM : BLIP_OFF;
}

void Baseball_DrawStats(int innings, int outs, int balls, int strikes, BOOL bHomeTeam)
{
	Digit_t *pdigit = &digit[0];
	
	// innings
	if (innings == -1)
	{
		pdigit->val = -1;
		pdigit->type = DIGIT_TYPE_DIM;
	}
	else
	{
		pdigit->val = innings;
		pdigit->type = DIGIT_TYPE_DIM;
	}

	// outs
	pdigit++;
	if (outs == -1)
	{
		pdigit->val = -1;
		pdigit->type = DIGIT_TYPE_DIM;
	}
	else
	{
		if (bHomeTeam)
		{
			pdigit->val = outs;
			pdigit->type = DIGIT_TYPE_DIM;
		}
		else
		{
			// draw decimal
			pdigit->val = outs+11;
			pdigit->type = DIGIT_TYPE_DIM;
		}
	}

	// balls
	pdigit++;
	pdigit->val = balls;
	pdigit->type = DIGIT_TYPE_DIM;

	// strikes
	pdigit++;
	if (bHomeTeam)
	{
		// draw decimal
		pdigit->val = strikes+11;
		pdigit->type = DIGIT_TYPE_DIM;
	}
	else
	{
		pdigit->val = strikes;
		pdigit->type = DIGIT_TYPE_DIM;
	}
}

void Baseball_DrawScore(int vruns, int hruns)
{
	if ((vruns==-1) && (hruns==-1))
	{
		// erase the digits display
		int i;
		for(i=0; i <4; i++) {
			digit[i].val = -1;
			digit[i].type = DIGIT_TYPE_BRIGHT;
		}
	}
	else
	{
		int i;
		for(i=0; i <4; i++) {
			digit[i].type = DIGIT_TYPE_BRIGHT;
		}
		// draw visitors 10s place
		digit[0].val = vruns/10;

		// draw visitors 1s place
		digit[1].val = vruns%10;

		// draw home 10s place
		digit[2].val =  hruns/10;

		// draw home 1s place
		digit[3].val = hruns%10;
	}
}

void Baseball_DrawFireWorks(void)
{
	int i;
	
	for(i=0; i <4; i++) {
		digit[i].val = (i == 1) ? 10 : -1;
		digit[i].type = DIGIT_TYPE_DIM;
	}
}

BOOL Baseball_GetInputSCORE(BOOL *pChange)
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

BOOL Baseball_GetInputPITCH(BOOL *pChange)
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

BOOL Baseball_GetInputHIT(BOOL *pChange)
{
	static BOOL bLast = FALSE;

	if (pChange){ *pChange = FALSE; }

	// check the keys
	if (Platform_GetRealTimeInput2()) // NOTE: gets realtime key state
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

BOOL Baseball_GetInputRUN(BOOL *pChange)
{
	static BOOL bLast = FALSE;

	if (pChange){ *pChange = FALSE; }

	// check the keys
	if (Platform_GetRealTimeInput1()) // NOTE: gets realtime key state
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

void Baseball_GetSize(int *w, int *h)
{
	*w = bmpScreen->w;
	*h = bmpScreen->h;
}

