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

#include <time.h>
#include "asndlib.h"
#include "Platform.h"
#include "Random.h"

static int bMute = 0;

//////////////////////////////////////////////////////
// OS dependant support
//////////////////////////////////////////////////////
void Platform_Init()
{
	// init the random number generator
	InitRandom();
}

void Platform_DeInit()
{
	Platform_EndDraw();

	// kill the random number generator
	DeinitRandom();
}

void Platform_StartDraw()
{
	
}

void Platform_EndDraw()
{

}

int Platform_SoundIsOff()
{
	return bMute ? 1 : 0;
}

int Platform_Random(int nRange)
{
	return (Random(nRange));
}

BOOL Platform_IsNewSecond()
{
	static time_t lastsec = -1;
	
	time_t newsec = time(NULL);

	if (newsec != lastsec){
		lastsec = newsec;
		return TRUE;
	}
	return FALSE;
}

/* Pause for n millisecs */
void Platform_Pause(int n)
{
	usleep(1000 * n);
}

static Sound_t *loop = NULL;
void Platform_PlaySound(Sound_t *sound, unsigned int nFlags)
{	
	if(bMute) {
		if (!(nFlags & PLAYSOUNDFLAGS_ASYNC))
			usleep(1000 * sound->msec);
		return;
	}
	
	if (nFlags & PLAYSOUNDFLAGS_LOOP)
	{
		if (nFlags & PLAYSOUNDFLAGS_PRIORITY)
			ASND_StopVoice(1);

		ASND_SetInfiniteVoice(1, VOICE_MONO_8BIT, 11025, 0,
				 (void *) sound->stream, sound->len, MAX_VOLUME, MAX_VOLUME);
		loop = sound;
	}
	else {
		if (nFlags & PLAYSOUNDFLAGS_PRIORITY)
			ASND_StopVoice(0);

		ASND_SetVoice(0, VOICE_MONO_8BIT, 11025, 0,
					 (void *) sound->stream, sound->len, MAX_VOLUME, MAX_VOLUME, NULL);
		if (!(nFlags & PLAYSOUNDFLAGS_ASYNC))
			while(ASND_StatusVoice(0) == SND_WORKING);
	}
}

void Platform_StopSound()
{
	if(loop != NULL)
		ASND_StopVoice(1);
	ASND_StopVoice(0);
}

static u32 btn;
static int wpad = 0;

void Platform_Input()
{
	static WPADData data;
	WPADData *Data;
	
	WPAD_ScanPads();
	
	Data = WPAD_Data(wpad);
	data = *Data;
	
	btn = data.btns_h;	
}

int PlatformSetInput(int pad)
{
	u32 type;
	
	if(pad < 0 || pad > 1)
		return -1;
	
	if(pad == wpad)
		return 0;
	
	if(WPAD_Probe(pad, &type) == WPAD_ERR_NONE) {
		WPAD_Rumble(pad, 1);
		sleep(1);
		WPAD_Rumble(pad, 0);
		wpad = pad;
		return 0;
	}

	return -1;
}

int PlatformGetInput()
{
	return wpad;
}

BOOL Platform_GetInputUP()
{
	if(btn & WPAD_BUTTON_B)
		return 0;
	return (btn & WPAD_BUTTON_RIGHT);
}

BOOL Platform_GetInputDOWN()
{
	if(btn & WPAD_BUTTON_B)
		return 0;
	return (btn & WPAD_BUTTON_LEFT);
}

BOOL Platform_GetInputLEFT()
{
	if(btn & WPAD_BUTTON_B)
		return 0;
	return (btn & WPAD_BUTTON_UP);
}

BOOL Platform_GetInputRIGHT()
{
	if(btn & WPAD_BUTTON_B)
		return 0;
	return (btn & WPAD_BUTTON_DOWN);
}

BOOL Platform_GetInput2()
{
	if(btn & WPAD_BUTTON_B)
		return 0;
	return (btn & WPAD_BUTTON_2);
}

BOOL Platform_GetInput1()
{
	if(btn & WPAD_BUTTON_B)
		return 0;
	return (btn & WPAD_BUTTON_1);
}

BOOL Platform_GetInputPLUS()
{
	if(btn & WPAD_BUTTON_B)
		return 0;	
	return (btn & WPAD_BUTTON_PLUS);
}

BOOL Platform_GetInputMINUS()
{
	if(btn & WPAD_BUTTON_B)
		return 0;
	return (btn & WPAD_BUTTON_MINUS);
}

BOOL Platform_GetInputMouseDown()
{
	if(btn & WPAD_BUTTON_B)
		return 0;
	return (btn & WPAD_BUTTON_A);
}

int Platform_GetPowerSwitch(int type)
{
	static u32 last_btn = 0;	
	
	if(btn != last_btn) {
		last_btn = btn;

		if((btn & WPAD_BUTTON_A) == WPAD_BUTTON_A) {
			bMute = 1 - bMute;
		}
		else if((btn & WPAD_BUTTON_B) == WPAD_BUTTON_B) {	
			switch(type) {
			case ONOFF_1OFF2:
			case ONOFF_OFF12:
				if(btn & WPAD_BUTTON_UP) {
					btn &= ~WPAD_BUTTON_UP;
					return -1;
				}
				if(btn & WPAD_BUTTON_DOWN) {
					btn &= ~WPAD_BUTTON_DOWN;
					return 1;
				}
				break;
			case ONOFF_SWITCH:
			default:
				if(btn & WPAD_BUTTON_LEFT) {
					btn &= ~WPAD_BUTTON_LEFT;
					return -1;
				}
				if(btn & WPAD_BUTTON_RIGHT) {
					btn &= ~WPAD_BUTTON_RIGHT;
					return 1;
				}
				break;
			}
		}
	}
	return 0;
}

BOOL Platform_CloseGame()
{
	return (btn & WPAD_BUTTON_HOME);
}

BOOL Platform_GetRealTimeInput1()
{
	Platform_Input();
	return Platform_GetInput1();
}

BOOL Platform_GetRealTimeInput2()
{
	Platform_Input();
	return Platform_GetInput2();
}

BOOL Platform_GetRealTimeInputPLUS()
{
	Platform_Input();
	return Platform_GetInputPLUS();
}

BOOL Platform_GetRealTimeInputMINUS()
{
	Platform_Input();
	return Platform_GetInputMINUS();
}
