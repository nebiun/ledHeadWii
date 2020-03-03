/*
 * LEDhead for Wii
 * Copyright (C) 2017-2020 Nebiun
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
#include "digits_f_png.h"

#define DIGIT_W		8
#define DIGIT_H		12

int draw_digit_f(int x, int y, int val, int flag)
{
	static GRRLIB_texImg *digit = NULL;
	int idx = -1;
	
	if(digit == NULL) {
		digit = GRRLIB_LoadTexture(digits_f_png);
		if(digit == NULL)
			return -1;
		GRRLIB_InitTileSet(digit, DIGIT_W, DIGIT_H, 0);
	}
	
	switch(flag) {
	case DIGIT_TYPE_NORMAL:
		if(val == -1)
			idx = 0;
		else
			idx = 1 + val%10;
		break;
	case DIGIT_TYPE_FLOAT:
		if(val == -1)
			idx = 11;
		else
			idx = 12 + val%10;
		break;
	case DIGIT_TYPE_SPECIAL:
		idx = 23 + val%2;
		break;
	}
	if(idx < 0)
		return -1;
	
	GRRLIB_DrawTile(realx(x), realy(y), digit, 0, 1, 1, 0xFFFFFFFF, idx);
	return 0;
}

int draw_digit(int x, int y, int val)
{
	return draw_digit_f(x, y, val, DIGIT_TYPE_NORMAL);
}

int print_digit(int x, int y, int pass, int val, int max)
{
	char n[16];
	register int i;
	register int pos = 0;
	
	if(max >= sizeof(n))
		return -1;
	if(val == -1) {
		for(i=0; i<max; i++) {
			draw_digit(x + pos, y, -1);
			pos += (DIGIT_W + pass);
		}		
	}
	else {
		snprintf(n,sizeof(n),"%0*d",max,val);
		for(i=0; i<max; i++) {
			draw_digit(x + pos, y,  n[i] - '0' + 1);
			pos += (DIGIT_W + pass);
		}
	}
	return pos;
}
