/*
 * This file is part of MPlayer.
 *
 * MPlayer is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * MPlayer is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with MPlayer; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */


#ifndef __MPLAYER_AREXX_H__
#define __MPLAYER_AREXX_H__

#include <inttypes.h>
#include <exec/libraries.h>
#include <classes/arexx.h>

// ARexx commands
enum ArexxID
{
	RXID_SEEK=1,
	RXID_SPEED_SET,
	RXID_SPEED_INCR,
	RXID_SPEED_MULT,
	RXID_EDL_MARK,
	RXID_AUDIO_DELAY,
	RXID_QUIT,
	RXID_PAUSE,
	RXID_FRAME_STEP,
	RXID_GRAB_FRAMES,
	RXID_PT_STEP,
	RXID_PT_UP_STEP,
	RXID_ALT_SRC_STEP,
	RXID_SUB_DELAY,
	RXID_SUB_STEP,
	RXID_SUB_LOAD,
	RXID_SUB_REMOVE,
	RXID_OSD,
	RXID_OSD_SHOW_TEXT,
	RXID_VOLUME,
	RXID_USE_MASTER,
	RXID_MUTE,
	RXID_SWITCH_AUDIO,
	RXID_CONTRAST,
	RXID_GAMMA,
	RXID_BRIGHTNESS,
	RXID_HUE,
	RXID_SATURATION,
	RXID_FRAME_DROP,
	RXID_SUB_POS,
	RXID_SUB_ALIGNMENT,
	RXID_SUB_VISIBILITY,
	RXID_GET_SUB_VISIBILITY,
	RXID_SUB_SELECT,
	RXID_GET_PERCENT_POS,
	RXID_GET_TIME_POS,
	RXID_GET_TIME_LENGTH,
	RXID_VO_FULLSCREEN,
	RXID_GET_VO_FULLSCREEN,
	RXID_VO_ONTOP,
	RXID_VO_ROOTWIN,
	RXID_SWITCH_VSYNC,
	RXID_SWITCH_RATIO,
	RXID_PANSCAN,
	RXID_LOADFILE,
	RXID_LOADLIST,
	RXID_CHANGE_RECTANGLE,
	RXID_DVDNAV,
	RXID_DVDNAV_EVENT,
	RXID_FORCED_SUBS_ONLY,
	RXID_DVB_SET_CHANNEL,
	RXID_SCREENSHOT,
	RXID_MENU,
	RXID_SET_MENU,
	RXID_HELP,
	RXID_EXIT,
	RXID_HIDE
};

// InitArexx error codes
#define IA_ERR_NONE 0
#define IA_ERR_OBJECT 1
#define IA_ERR_IFACE 2
#define IA_ERR_CLASS 3
#define IA_ERR_MEM 4

typedef struct ArexxHandle
{
  struct Library *ArexxBase;
  struct ARexxIFace *IARexx;
  Object *rxObject;
  uint32_t sigmask; // This is public and should be used in the main loop's Wait().
} ArexxHandle;


void StartArexx();
void StopArexx();

void put_command0(int cmd);
void put_icommand1(int cmd,int v);
void put_icommand2(int cmd,int v,int v2);
void put_fcommand2(int cmd,float v,int v2);
void put_scommand1(int cmd,char *v);
void put_scommand2(int cmd,char *v,int v2);

int get_icommand(CONST_STRPTR cmd);

// Input: Pointer to int to store an error code. Is optional and may be NULL
// Returns: ArexxHandle, or NULL for failure
ArexxHandle* InitArexx();

// Input: ARexxHandle to close. Accepts NULL as input.
void EndArexx(ArexxHandle *ro);

// Used by mplayer.c to set a value to be returned through ARexx.
void set_arexx_rc2(long v);
void rxFunc0(struct ARexxCmd *cmd, struct RexxMsg *rm);
void rxFunc1(struct ARexxCmd *cmd, struct RexxMsg *rm);
void rxFunc2(struct ARexxCmd *cmd, struct RexxMsg *rm);
void rxFunc2sw(struct ARexxCmd *cmd, struct RexxMsg *rm);
void rxFunc(struct ARexxCmd *cmd, long value, long value2, char *str, char *str2, char isval, char isval2);

#endif
