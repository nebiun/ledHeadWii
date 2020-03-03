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
#include <sys/time.h>
#include "LED_Handled.h"
#include "Games.h"
#include "Platform.h"
#include "cover_png.h"
#include "grrlib_logo_png.h"
#include "WKRD_png.h"
#include "WKLU_png.h"

//#define CHECK_PAINTTIME 1
//#define PRINT_TRACE     1

#ifdef  CHECK_PAINTTIME
struct timeval tv1, tv2;
long delta_usec, max_delta;
#endif

int trace = 0;
static int elapsed = 0;

static void timerProc(syswd_t alarm, void *arg)
{
	if (gCurrentGame)
	{
		elapsed++;
		// set up the next timer event
		struct timespec clock_time;
							
		clock_time.tv_sec = 0;
		clock_time.tv_nsec = gCurrentGame->tu * 1000000;
		SYS_SetAlarm(alarm,&clock_time,timerProc, NULL);	
	}
}

int main(int argc, char **argv) 
{
	GRRLIB_texImg *background;
	GRRLIB_texImg *game_screen = NULL;
	GRRLIB_texImg *pre_screen = NULL;
	GRRLIB_texImg *post_screen = NULL;
	GRRLIB_texImg *next, *prev;
	GRRLIB_texImg *logo;
	u16 btn, last_btn = -1;
	int help_on = 0;
	
	GRRLIB_Init();
	WPAD_Init();
	//set IR resolution to 640 width and 480 height
	WPAD_SetVRes(WPAD_CHAN_ALL, 640, 480);
	//return data for wii remotes should contain Button Data, Accelerometer, and IR
	WPAD_SetDataFormat(WPAD_CHAN_ALL, WPAD_FMT_BTNS_ACC_IR);
	
	// Audio
	ASND_Init();
	ASND_Pause(0);	
	
	background = GRRLIB_LoadTexture(cover_png);
	next = GRRLIB_LoadTexture(WKRD_png);
	prev = GRRLIB_LoadTexture(WKLU_png);
	Platform_Init();
	
	GRRLIB_FillScreen(RGBA(0x80,0x00,0x00,0xff));
	logo = GRRLIB_LoadTexture(grrlib_logo_png);
	GRRLIB_DrawImg((rmode->viWidth - logo->w)/2,(rmode->viHeight - logo->h)/2,logo,0,1,1, 0xFFFFFFFF);
	GRRLIB_Render();
	sleep(1);
	GRRLIB_FreeTexture(logo);
	
	GRRLIB_FillScreen(RGBA(0x00,0x00,0x00,0x00));	// black;
	GRRLIB_DrawImg(0,0,background,0,1,1, 0xFFFFFFFF);
	GRRLIB_Render();
	sleep(2);
	
	// Loop forever
    while(1) {
		int changed;
		
		WPAD_ScanPads();
		
		changed = 0;
		btn = WPAD_ButtonsHeld(0);
		
		if(last_btn == -1 || btn != last_btn) {
			last_btn = btn;

			if(btn & WPAD_BUTTON_A)	{
				if(gCurrentGame == NULL) {
					gCurrentGame = &gGameContext[0];
					changed++;
				}
				else if(gCurrentGame->Init != NULL) {					
					usleep(200000);
					/* play */
					gCurrentGame->Init();
					
					// set up the next timer event
					struct timespec clock_time;
					syswd_t timer;

					SYS_CreateAlarm(&timer);
					clock_time.tv_sec = 0;
					clock_time.tv_nsec = gCurrentGame->tu * 1000000;
					SYS_SetAlarm(timer,&clock_time,timerProc, NULL);
					
					while(1) {
						if(elapsed) {
							elapsed = 0;
							Platform_Input();
							if(Platform_CloseGame()) {
								break;
							}
#ifdef CHECK_PAINTTIME						
							gettimeofday(&tv1,NULL);
#endif							
							gCurrentGame->Run(gCurrentGame->tu);
#ifdef CHECK_PAINTTIME	
							gettimeofday(&tv2,NULL);
							delta_usec = tv2.tv_sec - tv1.tv_sec;
							delta_usec = 1000000 * delta_usec + tv2.tv_usec - tv1.tv_usec;
							if(delta_usec > max_delta)
								max_delta = delta_usec; 
							
							debugPrintf(300, 60, 0xFFFFFFFF, "run us=%d (max %d)", delta_usec, max_delta);
#endif
#ifdef PRINT_TRACE	
							debugPrintf(300, 30, 0xFFFFFFFF, "trc=%d", trace);
#endif
						}
#ifdef CHECK_PAINTTIME						
						gettimeofday(&tv1,NULL);
#endif						
						gCurrentGame->Paint();
#ifdef CHECK_PAINTTIME	
						gettimeofday(&tv2,NULL);
						delta_usec = tv2.tv_sec - tv1.tv_sec;
						delta_usec = 1000000 * delta_usec + tv2.tv_usec - tv1.tv_usec;
						
						debugPrintf(300, 90, 0xFFFFFFFF, "paint us=%d", delta_usec);
#endif											
						if(Platform_SoundIsOff())
							print_text(realx(0), realy(game_screen->h + 30), 0xFFFFFFFF, "(mute)");
						else {
							Platform_KeyShow(realx(0), realy(game_screen->h + 30), WK_A);
							print_text(realx(0 + 30), realy(game_screen->h + 30), 0xFFFFFFFF, "for mute");
						}
						Platform_KeyShow(realx(0), realy(game_screen->h + 60), WK_HOME);
						print_text(realx(0 + 30), realy(game_screen->h + 60), 0xFFFFFFFF, "exit game");
						GRRLIB_Render();
					}
					gCurrentGame->DeInit();
					SYS_CancelAlarm(timer);
					usleep(500000);
				}

			}		
			else if(((btn & WPAD_BUTTON_LEFT) || (btn & WPAD_BUTTON_DOWN))&& (gCurrentGame != NULL)) {
				if(gCurrentGame < &gGameContext[NUM_GAMES-1]) {
					gCurrentGame++;
					changed++;
				}
			}
			else if(((btn & WPAD_BUTTON_RIGHT) || (btn & WPAD_BUTTON_UP)) && (gCurrentGame != NULL)) {
				if(gCurrentGame > &gGameContext[0]) {
					gCurrentGame--;
					changed++;
				}
			}
			else if(btn & WPAD_BUTTON_B) {
				help_on = 1 - help_on;
				usleep(300000);
			}
			else if(btn & WPAD_BUTTON_HOME)	{
				break;
			}
		}
		
		if(gCurrentGame == NULL) {
			GRRLIB_DrawImg(0,0,background,0,1,1, 0xFFFFFFFF);
			print_text(215, 430, 0xFFFFFFFF, "Press       to continue");
			Platform_KeyShow(284, 430, WK_A);
		}
		else {
			if(changed) {
				if(game_screen != NULL)
					GRRLIB_FreeTexture(game_screen);
				if(pre_screen != NULL) {
					GRRLIB_FreeTexture(post_screen);
					post_screen = NULL;
				}
				if(pre_screen != NULL) {
					GRRLIB_FreeTexture(pre_screen);
					pre_screen = NULL;
				}
				game_screen = GRRLIB_LoadTexture(gCurrentGame->screen);
				if(gCurrentGame > &gGameContext[0])
					post_screen = GRRLIB_LoadTexture((gCurrentGame-1)->screen);
				if(gCurrentGame < &gGameContext[NUM_GAMES-1])
					pre_screen = GRRLIB_LoadTexture((gCurrentGame+1)->screen);
			}
			/* x e y vanno ricalcolate per posizionare lo screen */
			GRRLIB_DrawImg(realx(0),realy(0),game_screen,0,1,1, 0xFFFFFFFF);
			print_text(realx(0), realy(game_screen->h + 20), 0xFFFFFFFF, "%s", gCurrentGame->szName);
			if(post_screen != NULL) {
				GRRLIB_DrawImg(realx(0)-(post_screen->w/2)-50,realy(0)+50,post_screen,0,0.5,0.5, 0xFFFFFFFF);
				GRRLIB_DrawImg(realx(0)-(prev->w/2)-15,realy(0)+125,prev,0,0.5,0.5, 0xFFFFFFFF);
			}
			if(pre_screen != NULL) {
				GRRLIB_DrawImg(realx(0)+game_screen->w+15,realy(0)+125,next,0,0.5,0.5, 0xFFFFFFFF);
				GRRLIB_DrawImg(realx(0)+game_screen->w+50,realy(0)+50,pre_screen,0,0.5,0.5, 0xFFFFFFFF);	
			}
			Platform_KeyShow(realx(0), realy(game_screen->h + 60), WK_A);
			print_text(realx(0 + 30), realy(game_screen->h + 60), 0xFFFFFFFF, "to start the game");
			Platform_KeyShow(realx(0), realy(game_screen->h + 90), WK_B);
			print_text(realx(0 + 30), realy(game_screen->h + 90), 0xFFFFFFFF, "for keys map");
			if(help_on == 1 && gCurrentGame->Help != NULL)
				gCurrentGame->Help();
#ifdef PRINT_TRACE
			debugPrintf(300, 30, 0xFFFFFFFF, "trc=%d",trace);
#endif
		}
		GRRLIB_Render();
    }
	
	ASND_End();

//	Debug_destroy();
	GRRLIB_Exit();

    exit(0);  // Use exit() to exit a program, do not use 'return' from main()
}
