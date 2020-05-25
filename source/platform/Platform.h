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

#ifndef __PLATFORM_H__
#define __PLATFORM_H__
#include <stdlib.h>
#include <unistd.h>
#include "LED_Handled.h"

#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif
#ifndef BOOL
#define BOOL int
#endif

// on-off switch types
#define ONOFF_1OFF2			1
#define ONOFF_OFF12			2
#define ONOFF_SWITCH		0

// common game defines
#define BLIP_OFF			0
#define BLIP_DIM			1
#define BLIP_BRIGHT			2
#define BLIP_DIMBLINK		3
#define BLIP_BRIGHTBLINK	4
#define BLIP_DIMFLICKER		5

// xxx_PlaySound() flags
#define PLAYSOUNDFLAGS_PRIORITY		0x00001
#define PLAYSOUNDFLAGS_LOOP			0x00002
#define PLAYSOUNDFLAGS_ASYNC		0x00004

typedef enum {
	WK_1,
	WK_2,
	WK_A,
	WK_B,
	WK_BUD,
	WK_BLR,
	WK_BLR1,
	WK_PLUS,
	WK_MINUS,
	WK_HOME,
	WK_DPAD,
	WK_U,
	WK_D,
	WK_L,
	WK_R,
	WK_LR,
	WK_UD,
	WK_NUMKEY
} wiikey_t ;

typedef struct {
	const void *stream;
	int len;
	int msec;
} Sound_t;

typedef struct {
	int x;
	int y;
	int status;
} Blip_t;

typedef struct {
	int x;
	int y;
	int val;
} Stat_t;

typedef struct {
	int x;
	int y;
	int val;
	int type;
} Digit_t;

typedef struct {
	int x;
	int y;
} Coord_t;

typedef struct {
	wiikey_t val;
	Coord_t pos;
} Help_t;

#define Sound_set(a, mp3, sz, ms) \
	{ ((Sound_t *)a)->stream = mp3; ((Sound_t *)a)->len = sz; ((Sound_t *)a)->msec = ms; }

// interface that the platform must provide to the games

void Platform_Init();
void Platform_Help(Help_t *vect, int n);
void Platform_KeyShow(int x, int y, int val);
void Platform_DeInit();

void Platform_StartDraw();
void Platform_EndDraw();
int Platform_Random(int nRange);
BOOL Platform_IsNewSecond();
void Platform_Pause(int nMS);

void Platform_PlaySound(Sound_t *sound, unsigned int nFlags);
void Platform_StopSound();
int Platform_SoundIsOff();

void Platform_Input();
int PlatformSetInput(int pad);
int PlatformGetInput();

BOOL Platform_GetInputUP();
BOOL Platform_GetInputDOWN();
BOOL Platform_GetInputLEFT();
BOOL Platform_GetInputLEFTUP();
BOOL Platform_GetInputLEFTDOWN();
BOOL Platform_GetInputRIGHT();
BOOL Platform_GetInputRIGHTUP();
BOOL Platform_GetInputRIGHTDOWN();
BOOL Platform_GetInput1();
BOOL Platform_GetInput2();
BOOL Platform_GetInputPLUS();
BOOL Platform_GetInputMINUS();

BOOL Platform_GetRealTimeInput1();
BOOL Platform_GetRealTimeInput2();
BOOL Platform_GetRealTimeInputPLUS();
BOOL Platform_GetRealTimeInputMINUS();

int Platform_GetPowerSwitch(int type);
int Platform_GetPowerSwitchPlus(int type, int *extra);
BOOL Platform_CloseGame();

#endif
