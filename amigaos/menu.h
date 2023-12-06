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


//extern void attach_menu(struct Window *window);
//extern void detach_menu(struct Window *window);
//extern void menu_events( struct IntuiMessage *IntuiMsg);

//extern struct NewMenu amiga_menu[];

#ifndef __MPLAYER_MENU_H__
#define __MPLAYER_MENU_H__


enum { // used in spawn_died()
	TYPE_UNUSED,
	TYPE_ASL,
	TYPE_REQ,
	TYPE_ABOUT
};


// let the compiler "know" such structs exists
struct pmpMessage;


void createMenuTranslation(void);
void makeMenuClass(void);
void attach_menu(struct Window *window);
void detach_menu(struct Window *window);
void open_menu(void);
void spawn_died(int32 ret, int32 proc_enum);
struct Process *spawn(void *fn, char *name, int32 proc_enum);
void seek_start(void);
static int32 PlayFile_proc(STRPTR args, int32 length, APTR execbase);
static int32 open_file_proc(STRPTR args, int32 length, APTR execbase);
void cmd_open_file(void);
int32 PlayFile_async(const char *FileName);
//void cmd_open_dvd(void);
//void cmd_open_dvdnav(void);
//void cmd_open_vcd(void);
//void cmd_open_network(void);
void cmd_play(void);
void cmd_stop(void);
BOOL record_on(void);
void cmd_record(void);
BOOL loop_on(void);
void cmd_loop(void);
static int32 open_subtitles_proc(STRPTR args, int32 length, APTR execbase);
void cmd_open_subtitles(void);
void menu_project(ULONG menucode);
void menu_play(ULONG menucode);
void menu_video(ULONG menucode);
void menu_audio(ULONG menucode);
void menu_settings(ULONG menucode);
void processMenusClass(uint32 MenuClass_id);
void menu_events( struct IntuiMessage *IntuiMsg);
void open_AboutInfowindow(struct pmpMessage *pmp);
void ShowAbout(void);
void PrintMsg(CONST_STRPTR text, int REQ_TYPE, int REQ_IMAGE, CONST_STRPTR title);
void add_file_to_que(char *FileName);
void lastsong(void);
void prevfile(void);
void nextfile(void);
struct Screen *FrontMostScr(void);
BOOL LaunchCommand(char *command);
void open_InfoWindow_Properties(STRPTR vid_str, STRPTR aud_str, STRPTR clip_str);
void FileProperties(void);
void cmd_open_dev(STRPTR wintitle, void *open_fn);


#endif
