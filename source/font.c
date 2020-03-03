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
#include "FreeSerif_ttf.h"
#include "xped_ttf.h"

static GRRLIB_ttfFont *gameFont = NULL;
static GRRLIB_ttfFont *debugFont = NULL;

void print_text(int x, int y, u32 rgba, const char *fmt, ...)
{
	char lbuf[1024];
	va_list args;

	va_start(args, fmt);
	vsnprintf(lbuf,sizeof(lbuf)-1,fmt,args);
	va_end(args);
	lbuf[sizeof(lbuf)-1] = '\0';   /* tappo */
	
	if(gameFont == NULL) {
		gameFont = GRRLIB_LoadTTF(xped_ttf, xped_ttf_size);
		if(gameFont == NULL) {
			return;
		}
	}
	GRRLIB_PrintfTTF(x, y, gameFont, lbuf, 20, rgba);
}

void debugDestroy(void)
{
	if(debugFont != NULL)
		GRRLIB_FreeTTF(debugFont);
}

void debugPrintf(int x, int y, u32 rgba, const char *fmt, ...)
{
	char lbuf[1024];
	va_list args;

	va_start(args, fmt);
	vsnprintf(lbuf,sizeof(lbuf)-1,fmt,args);
	va_end(args);
	lbuf[sizeof(lbuf)-1] = '\0';   /* tappo */
	
	if(debugFont == NULL) {
		debugFont = GRRLIB_LoadTTF(FreeSerif_ttf, FreeSerif_ttf_size);
		if(debugFont == NULL) {
			return;
		}
	}
	GRRLIB_PrintfTTF(x, y, debugFont, lbuf, 20, rgba);
}
