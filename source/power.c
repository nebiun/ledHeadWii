/*
 * LEDhead for Wii
 * Copyright (C) 2017-2020 Nebiun
 *
 * Based on the handheld electronic games by Mattel Electronics.
 * All trademarks copyrighted by their respective owners. This
 * program is not affiliated or endorsed by Mattel Electronics.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
 
#include "LED_Handled.h"
#include "switch1_png.h"
#include "switch2_png.h"
#include "switch3_png.h"
#include "switch4_png.h"

static GRRLIB_texImg *_switch1 = NULL;
static GRRLIB_texImg *_switch2 = NULL;
static GRRLIB_texImg *_switch3 = NULL;
static GRRLIB_texImg *_switch4 = NULL;

static int draw_switch4(int x, int y, int pos)
{
	int r;
	
	if(_switch4 == NULL) {
		_switch4 = GRRLIB_LoadTexture(switch4_png);
		if(_switch4 == NULL)
			return -1;
	}

	switch(pos) {
	case SWITCH_POS_PRO1:
		r = 16;
		break;
	case SWITCH_POS_PRO2:
		r = 28;
		break;
	case SWITCH_POS_OFF:
	default:
		r = 3;
		break;
	}
	GRRLIB_DrawImg(realx(x+r), realy(y+1), _switch4, 0, SCALE_X, SCALE_Y, 0xFFFFFFFF);

	return 0;
}

static int draw_switch3(int x, int y, int pos)
{
	int r;
	
	if(_switch3 == NULL) {
		_switch3 = GRRLIB_LoadTexture(switch3_png);
		if(_switch3 == NULL)
			return -1;
	}

	switch(pos) {
	case SWITCH_POS_PRO1:
		r = 2;
		break;
	case SWITCH_POS_PRO2:
		r = 59;
		break;
	case SWITCH_POS_OFF:
	default:
		r = 32;
		break;
	}
	GRRLIB_DrawImg(realx(x+r), realy(y), _switch3, 0, SCALE_X, SCALE_Y, 0xFFFFFFFF);

	return 0;
}

static int draw_switch2(int x, int y, int pos)
{
	int r;
	
	if(_switch2 == NULL) {
		_switch2 = GRRLIB_LoadTexture(switch2_png);
		if(_switch2 == NULL)
			return -1;
	}

	switch(pos) {
	case SWITCH_POS_MODE1:
		r = 2;
		break;
	case SWITCH_POS_MODE2:
		r = 35;
		break;
	case SWITCH_POS_OFF:
	default:
		r = 17;
		break;
	}
	GRRLIB_DrawImg(realx(x+r), realy(y), _switch2, 0, SCALE_X, SCALE_Y, 0xFFFFFFFF);

	return 0;
}

static int draw_switch1(int x, int y, int position)
{
	int r;
	
	if(_switch1 == NULL) {
		_switch1 = GRRLIB_LoadTexture(switch1_png);
		if(_switch1 == NULL)
			return -1;
	}	
	
	switch(position) {
	case SWITCH_POS_PRO1:
		r = 23;
		break;
	case SWITCH_POS_PRO2:
		r = 48;
		break;
	case SWITCH_POS_OFF:
	default:
		r = 7;
		break;
	}
	GRRLIB_DrawImg(realx(x+r), realy(y+3), _switch1, 0, SCALE_X, SCALE_Y, 0xFFFFFFFF);
	
	return 0;
}

void destroy_switch(void)
{
	if(_switch1 != NULL) {
		GRRLIB_FreeTexture(_switch1);
		_switch1 = NULL;
	}
	if(_switch2 != NULL) {
		GRRLIB_FreeTexture(_switch2);
		_switch2 = NULL;
	}
	if(_switch3 != NULL) {
		GRRLIB_FreeTexture(_switch3);
		_switch3 = NULL;
	}
	if(_switch4 != NULL) {
		GRRLIB_FreeTexture(_switch4);
		_switch4 = NULL;
	}
}

int draw_switch(int type, int x, int y, int pos)
{
	int rtn = -1;
	
	switch(type) {
	case SWITCH_TYPE_1:
		rtn = draw_switch1(x, y, pos);
		break;
	case SWITCH_TYPE_2:
		rtn = draw_switch2(x, y, pos);
		break;
	case SWITCH_TYPE_3:
		rtn = draw_switch3(x, y, pos);
		break;
	case SWITCH_TYPE_4:
		rtn = draw_switch4(x, y, pos);
		break;
	}
	
	return rtn;
}


