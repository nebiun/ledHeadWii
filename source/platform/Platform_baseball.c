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
#include "baseball_poweroff_png.h"
#include "baseball_pro1_png.h"
#include "baseball_pro2_png.h"
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

// images
static GRRLIB_texImg *bmpScreen;
static GRRLIB_texImg *bmpBlip;
static GRRLIB_texImg *bmpDimDigits;
static GRRLIB_texImg *bmpBrightDigits;
static GRRLIB_texImg *bmpPowerOff;
static GRRLIB_texImg *bmpPro1;
static GRRLIB_texImg *bmpPro2;

static Sound_t tcWaveRes[6];

static Digit_t digit[4] = {
	{ baseball_digit1_x, baseball_digit_y, 0, DIGIT_TYPE_OFF},
	{ baseball_digit2_x, baseball_digit_y, 0, DIGIT_TYPE_OFF},
	{ baseball_digit3_x, baseball_digit_y, 0, DIGIT_TYPE_OFF},
	{ baseball_digit4_x, baseball_digit_y, 0, DIGIT_TYPE_OFF}
};

// blips for normal pitches
static Blip_t blip_pitch[9] = {
	{ baseball_blip_pitch_1_x, baseball_blip_pitch_1_y, BLIP_OFF },
	{ baseball_blip_pitch_2_x, baseball_blip_pitch_2_y, BLIP_OFF },
	{ baseball_blip_pitch_3_x, baseball_blip_pitch_3_y, BLIP_OFF },
	{ baseball_blip_pitch_4_x, baseball_blip_pitch_4_y, BLIP_OFF },
	{ baseball_blip_pitch_5_x, baseball_blip_pitch_5_y, BLIP_OFF },
	{ baseball_blip_pitch_6_x, baseball_blip_pitch_6_y, BLIP_OFF },
	{ baseball_blip_pitch_7_x, baseball_blip_pitch_7_y, BLIP_OFF },
	{ baseball_blip_pitch_8_x, baseball_blip_pitch_8_y, BLIP_OFF },
	{ baseball_blip_pitch_9_x, baseball_blip_pitch_9_y, BLIP_OFF }
};
// blips for curve balls
static Blip_t blip_pitchcurve[4] = {
	{ baseball_blip_pitchcurve_2_x, baseball_blip_pitchcurve_2_y, BLIP_OFF },
	{ baseball_blip_pitchcurve_3_x, baseball_blip_pitchcurve_3_y, BLIP_OFF },
	{ baseball_blip_pitchcurve_4_x, baseball_blip_pitchcurve_4_y, BLIP_OFF },
	{ baseball_blip_pitchcurve_5_x, baseball_blip_pitchcurve_5_y, BLIP_OFF }
};
// blips bases
static Blip_t blip_base[4][3] = {
	// blips for home plate -> 1st base
	{{ baseball_blip_base_0_x,baseball_blip_base_0_y, BLIP_OFF },
	 { baseball_blip_base_0A_x,baseball_blip_base_0A_y, BLIP_OFF },
	 { baseball_blip_base_0B_x,baseball_blip_base_0B_y, BLIP_OFF }},
	// blips for 1st base -> 2nd base 
	{{ baseball_blip_base_1_x,baseball_blip_base_1_y, BLIP_OFF },
	 { baseball_blip_base_1A_x,baseball_blip_base_1A_y, BLIP_OFF },
	 { baseball_blip_base_1B_x,baseball_blip_base_1B_y, BLIP_OFF }},
	{{ baseball_blip_base_2_x,baseball_blip_base_2_y, BLIP_OFF },
	// blips for 2nd base -> 3rd base
	 { baseball_blip_base_2A_x,baseball_blip_base_2A_y, BLIP_OFF },
	 { baseball_blip_base_2B_x,baseball_blip_base_2B_y, BLIP_OFF }},
	// blips for 3rd base -> home plate
	{{ baseball_blip_base_3_x,baseball_blip_base_3_y, BLIP_OFF },
	 { baseball_blip_base_3A_x,baseball_blip_base_3A_y, BLIP_OFF },
	 { baseball_blip_base_3B_x,baseball_blip_base_3B_y, BLIP_OFF }}
};
// blips deep
static Blip_t blip_deep[2] = {
	// blip for deep 1st
	{ baseball_blip_deep1st_x,baseball_blip_deep1st_y, BLIP_OFF },
	// blip for deep 3rd
	{ baseball_blip_deep3rd_x,baseball_blip_deep3rd_y, BLIP_OFF }
};
// blips for outfield
static Blip_t blip_outfield[3] = {
	{ baseball_blip_outfield_left_x,baseball_blip_outfield_left_y, BLIP_OFF },
	{ baseball_blip_outfield_center_x,baseball_blip_outfield_center_y, BLIP_OFF },
	{ baseball_blip_outfield_right_x,baseball_blip_outfield_right_y, BLIP_OFF }
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
void Baseball_Help()
{
	Platform_Help(help, sizeof(help)/sizeof(*help));
}

static BOOL bInited = FALSE;
	
void Baseball_Init()
{
	if (bInited) return;

	// Init sounds
	Sound_set(&tcWaveRes[0], baseball_hit_raw, baseball_hit_raw_size, 82 );
	Sound_set(&tcWaveRes[1], baseball_out_raw, baseball_out_raw_size, 392 );
	Sound_set(&tcWaveRes[2], baseball_run_raw, baseball_run_raw_size, 423 );
	Sound_set(&tcWaveRes[3], baseball_strike_raw, baseball_strike_raw_size, 22 );
	Sound_set(&tcWaveRes[4], baseball_endpossession_raw, baseball_endpossession_raw_size, 560 );
	Sound_set(&tcWaveRes[5], baseball_endgame_raw, baseball_endgame_raw_size, 712 );

	// load images
	bmpScreen = GRRLIB_LoadTexture(baseball_screen_png);
	bmpBlip = GRRLIB_LoadTexture(baseball_blip_png);
	bmpDimDigits = GRRLIB_LoadTexture(baseball_dimdigits_png);
	GRRLIB_InitTileSet(bmpDimDigits, DIGIT_W, DIGIT_H, 0);
	bmpBrightDigits = GRRLIB_LoadTexture(baseball_brightdigits_png);
	GRRLIB_InitTileSet(bmpBrightDigits, DIGIT_W, DIGIT_H, 0);

	// set up the power switch images
	bmpPowerOff = GRRLIB_LoadTexture(baseball_poweroff_png);
	bmpPro1 = GRRLIB_LoadTexture(baseball_pro1_png);
	bmpPro2 = GRRLIB_LoadTexture(baseball_pro2_png);
	
	PlatformSetInput(0);
	// turn on the game
	Baseball_SetSkill(0);
	Baseball_PowerOn();

	bInited = TRUE;
}

void Baseball_DeInit()
{
	// stop all sounds...
	Platform_StopSound();
	GRRLIB_FreeTexture(bmpScreen);
	GRRLIB_FreeTexture(bmpBlip);
	GRRLIB_FreeTexture(bmpDimDigits);
	GRRLIB_FreeTexture(bmpBrightDigits);
	GRRLIB_FreeTexture(bmpPowerOff);
	GRRLIB_FreeTexture(bmpPro1);
	GRRLIB_FreeTexture(bmpPro2);
	
	bInited = FALSE;
}

void Baseball_Paint()
{
	int i;
	BOOL power = Baseball_GetPower();
	BOOL skill = Baseball_GetSkill();
	int p_switch;
	p_switch = Platform_GetPowerSwitch(ONOFF_OFF12);
	if(power) {
		if(p_switch == -1) {
			if(skill == 0)
				Baseball_PowerOff();
			if(skill == 1) {
				Baseball_PowerOn();
				Baseball_SetSkill(0);
			}		
		} 
		if(p_switch == 1) {
			if(skill == 0) {
				Baseball_PowerOn();
				Baseball_SetSkill(1);
			}
		} 
	}
	else {
		if(p_switch == 1) {
			Baseball_PowerOn();
			Baseball_SetSkill(0);
		} 
	}
	
	// paint the backdrop
	GRRLIB_DrawImg(realx(0), realy(0), bmpScreen, 0, 1, 1, 0xFFFFFFFF);
	
	if (power)
	{
		if (skill == 0)
		{
			GRRLIB_DrawImg(realx(baseball_pro_1_x), realy(baseball_pro_1_y), bmpPro1, 0, 1, 1, 0xFFFFFFFF);
		} 
		else {
			GRRLIB_DrawImg(realx(baseball_pro_2_x), realy(baseball_pro_2_y), bmpPro2, 0, 1, 1, 0xFFFFFFFF);
		}
		// show blips
		for(i=0; i<9; i++) {
			Blip_t *pblip = &blip_pitch[i];
			if(pblip->status != BLIP_OFF)
				GRRLIB_DrawImg(realx(pblip->x), realy(pblip->y), bmpBlip, 0, 1, 1, 0xFFFFFFFF);
		}
		for(i=0; i<4; i++) {
			Blip_t *pblip = &blip_pitchcurve[i];
			if(pblip->status != BLIP_OFF)
				GRRLIB_DrawImg(realx(pblip->x), realy(pblip->y), bmpBlip, 0, 1, 1, 0xFFFFFFFF);
		}
		for(i=0; i<4; i++) {
			int j;		
			for(j=0; j<3; j++) {
				Blip_t *pblip = &blip_base[i][j];
				if(pblip->status != BLIP_OFF)
					GRRLIB_DrawImg(realx(pblip->x), realy(pblip->y), bmpBlip, 0, 1, 1, 0xFFFFFFFF);
			}
		}
		for(i=0; i<2; i++) {
			Blip_t *pblip = &blip_deep[i];
			if(pblip->status != BLIP_OFF)
				GRRLIB_DrawImg(realx(pblip->x), realy(pblip->y), bmpBlip, 0, 1, 1, 0xFFFFFFFF);
		}
		for(i=0; i<3; i++) {
			Blip_t *pblip = &blip_outfield[i];
			if(pblip->status != BLIP_OFF)
				GRRLIB_DrawImg(realx(pblip->x), realy(pblip->y), bmpBlip, 0, 1, 1, 0xFFFFFFFF);
		}

		// show digits
		for(i=0; i<4; i++) {
			Digit_t *pdigit = &digit[i];
			
			switch(pdigit->type) {
			case DIGIT_TYPE_DIM:
				GRRLIB_DrawTile(realx(pdigit->x), realy(pdigit->y), bmpDimDigits, 0, 1, 1, 0xFFFFFFFF, pdigit->val + 1);
				break;
			case DIGIT_TYPE_BRIGHT:
				GRRLIB_DrawTile(realx(pdigit->x), realy(pdigit->y), bmpBrightDigits, 0, 1, 1, 0xFFFFFFFF, pdigit->val + 1);
				break;
			case DIGIT_TYPE_OFF:
			default:
				break;
			}
		}
	}
	else
	{
		GRRLIB_DrawImg(realx(baseball_power_off_x), realy(baseball_power_off_y), bmpPowerOff, 0, 1, 1, 0xFFFFFFFF);
	}
}

void Baseball_PlaySound(int nSound, unsigned int nFlags)
{
	Platform_PlaySound(&tcWaveRes[nSound], nFlags);
}

void Baseball_StopSound()
{
	// stop all sounds...
	Platform_StopSound();
}


//----------------------------------------------------------------------------
// local fcn's
//

void Baseball_DrawPitchBlip(BOOL state, int index, BOOL curve)
{
	Blip_t *pblip = NULL;
	
	switch(index)
	{
	case 0:
	default:
		pblip = &blip_pitch[0];
		break;
	case 1:
		if (curve)
			pblip = &blip_pitchcurve[0];
		else
			pblip = &blip_pitch[1];
		break;
	case 2:
		if (curve)
			pblip = &blip_pitchcurve[1];
		else
			pblip = &blip_pitch[2];
		break;
	case 3:
		if (curve)
			pblip = &blip_pitchcurve[2];
		else
			pblip = &blip_pitch[3];
		break;
	case 4:
		if (curve)
			pblip = &blip_pitchcurve[3];
		else
			pblip = &blip_pitch[4];
		break;
	case 5:
		pblip = &blip_pitch[5];
		break;
	case 6:
		pblip = &blip_pitch[6];
		break;
	case 7:
		pblip = &blip_pitch[7];
		break;
	case 8:
		pblip = &blip_pitch[8];
		break;
	}

	if (pblip != NULL )
	{
		pblip->status = (state) ? BLIP_DIM : BLIP_OFF;
	}
}

void Baseball_DrawBaseBlip(BOOL state, int index)
{
	Blip_t *pblip = NULL;
	
	switch(index)
	{
	case 12:
	case 0: // home plate
		pblip = &blip_base[0][0];
		break;
	case 1:
		pblip = &blip_base[0][1];
		break;
	case 2:
		pblip = &blip_base[0][2];
		break;
	case 3: // 1st base
		pblip = &blip_base[1][0];
		break;
	case 4:
		pblip = &blip_base[1][1];
		break;
	case 5:
		pblip = &blip_base[1][2];
		break;
	case 6: // 2nd base
		pblip = &blip_base[2][0];
		break;
	case 7:
		pblip = &blip_base[2][1];
		break;
	case 8:
		pblip = &blip_base[2][2];
		break;
	case 9: // 3rd
		pblip = &blip_base[3][0];
		break;
	case 10:
		pblip = &blip_base[3][1];
		break;
	case 11:
		pblip = &blip_base[3][2];
		break;
	}
	if (pblip != NULL )
	{
		pblip->status = (state) ? BLIP_DIM : BLIP_OFF;
	}
}

void Baseball_DrawOutfieldBlip(BOOL state, int index)
{
	Blip_t *pblip = &blip_outfield[index];

	pblip->status = (state) ? BLIP_DIM : BLIP_OFF;
}

void Baseball_DrawDeepBlip(BOOL state, int index)
{
	Blip_t *pblip = &blip_deep[index];

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
		digit[i].val = -1;
		digit[i].type = (i == 1) ? DIGIT_TYPE_BRIGHT : DIGIT_TYPE_DIM;
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

void Baseball_GetSize(int *w, int *h)
{
	*w = bmpScreen->w;
	*h = bmpScreen->h;
}

