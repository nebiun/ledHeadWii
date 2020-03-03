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


#ifndef __PLATFORM_BASEBALL_H__
#define __PLATFORM_BASEBALL_H__

// [general]
#define baseball_power_off_x 91
#define baseball_power_off_y 300
#define baseball_pro_1_x 91
#define baseball_pro_1_y 300
#define baseball_pro_2_x 91
#define baseball_pro_2_y 300
#define baseball_digit1_x 73
#define baseball_digit2_x 102
#define baseball_digit3_x 131
#define baseball_digit4_x 160
#define baseball_digit_y 34
#define baseball_digit_w 8
#define baseball_digit_h 9

// blips for normal pitches
#define baseball_blip_pitch_1_x 118
#define baseball_blip_pitch_1_y 151
#define baseball_blip_pitch_2_x 118
#define baseball_blip_pitch_2_y 167
#define baseball_blip_pitch_3_x 118
#define baseball_blip_pitch_3_y 182
#define baseball_blip_pitch_4_x 118
#define baseball_blip_pitch_4_y 197
#define baseball_blip_pitch_5_x 118
#define baseball_blip_pitch_5_y 211
#define baseball_blip_pitch_6_x 118
#define baseball_blip_pitch_6_y 225
#define baseball_blip_pitch_7_x 118
#define baseball_blip_pitch_7_y 236
#define baseball_blip_pitch_8_x 118
#define baseball_blip_pitch_8_y 248
#define baseball_blip_pitch_9_x 118
#define baseball_blip_pitch_9_y 256

// blips for curve balls
#define baseball_blip_pitchcurve_2_x 123
#define baseball_blip_pitchcurve_2_y 166
#define baseball_blip_pitchcurve_3_x 125
#define baseball_blip_pitchcurve_3_y 181
#define baseball_blip_pitchcurve_4_x 125
#define baseball_blip_pitchcurve_4_y 196
#define baseball_blip_pitchcurve_5_x 123
#define baseball_blip_pitchcurve_5_y 210

// home plate -> 1st base
#define baseball_blip_base_0_x 118
#define baseball_blip_base_0_y 236
#define baseball_blip_base_0A_x 138
#define baseball_blip_base_0A_y 215
#define baseball_blip_base_0B_x 160
#define baseball_blip_base_0B_y 192

// 1st base -> 2nd base
#define baseball_blip_base_1_x 184
#define baseball_blip_base_1_y 168
#define baseball_blip_base_1A_x 165
#define baseball_blip_base_1A_y 145
#define baseball_blip_base_1B_x 142
#define baseball_blip_base_1B_y 121

// 2nd base -> 3rd base
#define baseball_blip_base_2_x 118
#define baseball_blip_base_2_y 99
#define baseball_blip_base_2A_x 93
#define baseball_blip_base_2A_y 121
#define baseball_blip_base_2B_x 72
#define baseball_blip_base_2B_y 144

// 3rd base -> home plate
#define baseball_blip_base_3_x 52
#define baseball_blip_base_3_y 167
#define baseball_blip_base_3A_x 76
#define baseball_blip_base_3A_y 192
#define baseball_blip_base_3B_x 98
#define baseball_blip_base_3B_y 215

// deep 1st
#define baseball_blip_deep1st_x 191
#define baseball_blip_deep1st_y 148

// deep 3rd
#define baseball_blip_deep3rd_x 45
#define baseball_blip_deep3rd_y 148

// outfield
#define baseball_blip_outfield_left_x 53
#define baseball_blip_outfield_left_y 85
#define baseball_blip_outfield_center_x 119
#define baseball_blip_outfield_center_y 65
#define baseball_blip_outfield_right_x 184
#define baseball_blip_outfield_right_y 85

// interface that the platform must provide for this game

// functions exported to the game context
void Baseball_Init();
void Baseball_Help();
void Baseball_DeInit();
void Baseball_Paint();
void Baseball_PlaySound(int nSound, unsigned int nFlags);
void Baseball_StopSound();
void Baseball_GetSize(int *w, int *h);

// "private" functions not exported to game context
void Baseball_DrawPitchBlip(BOOL state, int index, BOOL curve);
void Baseball_DrawBaseBlip(BOOL state, int index);
void Baseball_DrawOutfieldBlip(BOOL state, int index);
void Baseball_DrawDeepBlip(BOOL state, int index);

void Baseball_DrawStats(int innings, int outs, int balls, int strikes, BOOL bHomeTeam);
void Baseball_DrawScore(int vruns, int hruns);
void Baseball_DrawFireWorks(void);

BOOL Baseball_GetInputSCORE(BOOL *pChange);
BOOL Baseball_GetInputPITCH(BOOL *pChange);
BOOL Baseball_GetInputHIT(BOOL *pChange);
BOOL Baseball_GetInputRUN(BOOL *pChange);

#endif
