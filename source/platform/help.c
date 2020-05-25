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
#include "Platform.h"
#include "WK1_png.h"
#include "WK2_png.h"
#include "WKA_png.h"
#include "WKB_png.h"
#include "WKBUD_png.h"
#include "WKBLR_png.h"
#include "WKBLR1_png.h"
#include "WKPLUS_png.h"
#include "WKMINUS_png.h"
#include "WKHOME_png.h"
#include "WKDPAD_png.h"
#include "WKU_png.h"
#include "WKD_png.h"
#include "WKL_png.h"
#include "WKR_png.h"
#include "WKUD_png.h"
#include "WKLR_png.h"

static GRRLIB_texImg *key_img[WK_NUMKEY];

static void _key_init(void)
{
	if(key_img[0] == NULL) {
		key_img[WK_1] = GRRLIB_LoadTexture(WK1_png);
		key_img[WK_2] = GRRLIB_LoadTexture(WK2_png);
		key_img[WK_A] = GRRLIB_LoadTexture(WKA_png);
		key_img[WK_B] = GRRLIB_LoadTexture(WKB_png);
		key_img[WK_BUD] = GRRLIB_LoadTexture(WKBUD_png);
		key_img[WK_BLR] = GRRLIB_LoadTexture(WKBLR_png);
		key_img[WK_BLR1] = GRRLIB_LoadTexture(WKBLR1_png);
		key_img[WK_PLUS] = GRRLIB_LoadTexture(WKPLUS_png);
		key_img[WK_MINUS] = GRRLIB_LoadTexture(WKMINUS_png);
		key_img[WK_HOME] = GRRLIB_LoadTexture(WKHOME_png);
		key_img[WK_DPAD] = GRRLIB_LoadTexture(WKDPAD_png);
		key_img[WK_U] = GRRLIB_LoadTexture(WKU_png);
		key_img[WK_D] = GRRLIB_LoadTexture(WKD_png);
		key_img[WK_L] = GRRLIB_LoadTexture(WKL_png);
		key_img[WK_R] = GRRLIB_LoadTexture(WKR_png);
		key_img[WK_UD] = GRRLIB_LoadTexture(WKUD_png);
		key_img[WK_LR] = GRRLIB_LoadTexture(WKLR_png);
	}	
}

void Platform_Help(Help_t *vect, int n)
{
	int i;
	if(key_img[0] == NULL)
		_key_init();
	
	for(i=0; i<n; i++) {
		GRRLIB_DrawImg(realx(vect[i].pos.x),realy(vect[i].pos.y),key_img[vect[i].val],0,SCALE_X,SCALE_Y, 0xFFFFFFFF);
	//	debugPrintf(300, 60+30*i, 0xFFFFFFFF, "x %d, y %d, img[%d] %p",vect[i].pos.x,vect[i].pos.y,vect[i].val, key_img[vect[i].val]);
	}
}

void Platform_KeyShow(int x, int y, int val)
{
	if(val < 0 || val >= WK_NUMKEY)
		return;
	
	if(key_img[0] == NULL)
		_key_init();
	
	GRRLIB_DrawImg(x,y,key_img[val],0,SCALE_X,SCALE_Y, 0xFFFFFFFF);
}
