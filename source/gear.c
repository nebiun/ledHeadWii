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
#include "slide1_png.h"
static GRRLIB_texImg *_gear_slide = NULL;

void destroy_gear(void)
{
	if(_gear_slide != NULL) {
		GRRLIB_FreeTexture(_gear_slide);
		_gear_slide = NULL;
	}
}

int draw_gear(int x, int y, int pos)
{

	int yy;
	
	if(_gear_slide == NULL) {
		_gear_slide = GRRLIB_LoadTexture(slide1_png);
		if(_gear_slide == NULL) {
			return -1;
		}
	}	
	switch(pos) {
	case 1:
		yy = 3;
		break;
	case 2:
		yy = 19;
		break;
	case 3:
		yy = 37;
		break;
	case 4:
		yy = 53;
		break;
	default:
		yy = -1;
		break;
	}
	
	if(yy != -1)
		GRRLIB_DrawImg(realx(x), realy(y+yy), _gear_slide, 0, SCALE_X, SCALE_Y, 0xFFFFFFFF);

	return 0;
}
