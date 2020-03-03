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
#include "poweroff_a_png.h"

#define POWEROFF_H	8
#define POWEROFF_W	48

int draw_poweroff_a(int x, int y, int val)
{
	static GRRLIB_texImg *power = NULL;
	int idx;
	
	if(power == NULL) {
		power = GRRLIB_LoadTexture(poweroff_a_png);
		if(power == NULL)
			return -1;
		GRRLIB_InitTileSet(power, POWEROFF_W, POWEROFF_H, 0);
	}

	switch(val) {
	case POWER_POS_MODE1:
		idx = 1;
		break;
	case POWER_POS_MODE2:
		idx = 2;
		break;
	case POWER_POS_OFF:
	default:
		idx = 0;
		break;
	}
	GRRLIB_DrawTile(realx(x), realy(y), power, 0, 1, 1, 0xFFFFFFFF, idx);

	return idx;
}
