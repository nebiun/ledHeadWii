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
#include "v_blip_png.h"
#include "o_blip_png.h"

#define V_BLIP_H	4
#define V_BLIP_W	4
#define O_BLIP_H	4
#define O_BLIP_W	4

static inline int _drawBlip(int x, int y, int type, GRRLIB_texImg *blip)
{
	int idx;
	
	switch(type) {
	case BLIP_TYPE_BRIGHT:
		idx = 1;
		break;
	case BLIP_TYPE_FLICKER:
		idx = 2;
		break;
	case BLIP_TYPE_NORMAL:
	default:
		idx = 0;
		break;
	}
	GRRLIB_DrawTile(realx(x), realy(y), blip, 0, SCALE_X, SCALE_Y, 0xFFFFFFFF, idx);

	return idx;
}

int draw_vblip(int x, int y, int type)
{
	static GRRLIB_texImg *blip = NULL;
	
	if(blip == NULL) {
		blip = GRRLIB_LoadTexture(v_blip_png);
		if(blip == NULL)
			return -1;
		GRRLIB_InitTileSet(blip, V_BLIP_W, V_BLIP_H, 0);
	}

	return _drawBlip(x, y, type, blip);
}

int draw_oblip(int x, int y, int type)
{
	static GRRLIB_texImg *blip = NULL;
	
	if(blip == NULL) {
		blip = GRRLIB_LoadTexture(o_blip_png);
		if(blip == NULL)
			return -1;
		GRRLIB_InitTileSet(blip, O_BLIP_W, O_BLIP_H, 0);
	}

	return _drawBlip(x, y, type, blip);
}
