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

#include <proto/intuition.h>
#include <proto/gadtools.h>
#include <proto/dos.h>
#include <proto/asl.h>
#include <proto/requester.h>
#include <proto/exec.h>
#include <proto/wb.h>
#include <proto/utility.h>
#include <classes/requester.h>
#include <libraries/gadtools.h>
#include <intuition/menuclass.h>

#include <images/bitmap.h>
// #include <proto/bitmap.h>

#include <stdbool.h>

// #include "SDL/SDL_endian.h"

#include "mp_msg.h"
#include "mp_core.h"

#include "input/input.h"
#include "arexx.h"
#include "menu.h"
#include "../osdep/keycodes.h"
#include "../version.h"
#define FFMPEG_VERSION "6.0"

#include "debug.h"


BOOL is_ontop = FALSE;
BOOL is_mute = FALSE;	// zzd10h

extern STRPTR ARexxPortName;

extern mp_cmd_t mp_cmds[]; // static
extern struct Screen *My_Screen;
extern int stream_cache_size;	// Need to set this to 0 if you select DVDNAV.
extern uint32_t is_fullscreen;

static int32 OpenDVD(STRPTR args UNUSED, int32 length UNUSED, struct ExecBase *execbase UNUSED);
static int32 OpenDVDNAV(STRPTR args UNUSED, int32 length UNUSED, struct ExecBase *execbase UNUSED);
static int32 OpenVCD(STRPTR args UNUSED, int32 length UNUSED, struct ExecBase *execbase UNUSED);
static int32 OpenNetwork(STRPTR args UNUSED, int32 length UNUSED, struct ExecBase *execbase UNUSED);

// Not static used in drag and drop.
//int32 PlayFile_async(const char *FileName);

// Only used for menus
static UBYTE *LoadFile(const char *StartingDir, CONST_STRPTR ASL_Pattern, CONST_STRPTR ASL_WindowTitle);
static int32 PrintMsgProc(STRPTR args UNUSED, int32 length UNUSED, struct ExecBase *execbase);
//void PrintMsg(CONST_STRPTR text, int REQ_TYPE, int REQ_IMAGE, CONST_STRPTR title);


extern void make_appwindow(struct Window *win);
extern void delete_appwindow(void);


//void prevfile(void);
//void nextfile(void);

//BOOL loop_on(void);

extern BOOL choosing_flag;
extern void set_gfx_rendering_option();

extern BOOL AISS_MENU ;				// In amigaos_stuff.c zzd10h
extern BOOL AISS_MENU_TO_SCALE ;		// In amigaos_stuff.c zzd10h
extern int AISS_MENU_SIZE ;			// In amigaos_stuff.c zzd10h Setted by MenuImageSize env variable
extern BOOL MENUCLASS_SUPPORT ;		// In amigaos_stuff.c zzd10h TRUE if MenuClass menu is supported (AmigaOS 4.1 FE with intuition.library >= 54.6)
extern BOOL BANNER_DISPLAY ;			// In amigaos_stuff.c zzd10h Display banner ? SETENV SAVE Disable_Banner_Images 1 to disable them
// struct Screen *screen;

extern Class *RequesterClass, *InfoWindowClass, *BitMapClass;

#include <classes/infowindow.h>
extern struct ClassLibrary *InfoWindowBase;
#include "infowindow_banner.h" // using bin2C
#define BANNER_WIDTH  60
#define BANNER_HEIGHT 57
static void BackFillFunc(struct Hook *hook UNUSED, struct RastPort *rp, struct BackFillMessage *msg);
struct Process *About_Process, *ASL_Process, *Req_Process;


#include <datatypes/datatypes.h>
#include <datatypes/pictureclass.h>
#include <proto/datatypes.h>
#include <proto/graphics.h>
#include <graphics/rpattr.h>
// #include <graphics/composite.h>
// #include <graphics/blitattr.h>
#include <proto/layers.h>


extern struct Catalog *catalog ;
#define MPlayer_NUMBERS
#define MPlayer_STRINGS
#include "MPlayer_catalog.h"
extern STRPTR myGetCatalogStr (struct Catalog *catalog, LONG num, STRPTR def);
#define CS(id) myGetCatalogStr(catalog,id,id##_STR)

char menu_Project[64];
	char menu_Project_OpenFile[64];
	char menu_Project_OpenDVD[64];
	char menu_Project_OpenDVDNAV[64];
	char menu_Project_OpenVCD[64];
	char menu_Project_OpenNetwork[64];
	char menu_Project_AboutMPlayer[64];
	char menu_Project_Iconify[64];
	char menu_Project_Quit[64];

char menu_Play[64];
	char menu_Play_PlayPause[64];
	char menu_Play_Stop[64];
char menu_Play_Record[64];
	char menu_Play_Loop[64];
	char menu_Play_PrevFilm[64];
	char menu_Play_NextFilm[64];
	char menu_Play_Properties[64];

char menu_OptionsVideo[64];
	char menu_OptionsVideo_StayOnTop[64];
	char menu_OptionsVideo_OpenSubtitles[64];
	char menu_OptionsVideo_CycleSubtitles[64];
char menu_OptionsVideo_UnloadSubtitles[64];
char menu_OptionsVideo_SubtitlesSize[64];
char menu_OptionsVideo_SubtitlesBig[64];
char menu_OptionsVideo_SubtitlesSmall[64];
	char menu_OptionsVideo_Fullscreen[64];
	char menu_OptionsVideo_Screenshot[64];
char menu_OptionsVideo_AspectRatio[64];
char menu_OptionsVideo_AR_Original[64];

char menu_OptionsAudio[64];
	char menu_OptionsAudio_Mute[64];
	char menu_OptionsAudio_VolumeUp[64];
	char menu_OptionsAudio_VolumeDown[64];

char menu_Settings[64];
	char menu_Settings_MPlayerGUI[64];


void choosing(BOOL set)
{
	choosing_flag = set;
	set_gfx_rendering_option();
}

void blink_screen(void)
{
	struct Screen *screen = NULL;
	screen = (struct Screen*) ((struct IntuitionBase*) IntuitionBase)->FirstScreen;
	DisplayBeep(screen) ;
}


extern char *TOOL_MPGUI;
extern char *SUBEXTPAT;
extern char *EXTPATTERN;
extern MPContext *mpctx;


// MUST match struct NewMenu order
enum
{
	ID_open_file,
	ID_open_dvd,
	ID_open_dvdnav,
	ID_open_vcd,
	ID_open_network,
dummy1,
	ID_about,
	ID_iconify,
dummy2,
	ID_quit,
NUMITEMS_PROJECT
};

enum
{
	ID_play,
	ID_stop,
	ID_loop,
// ID_record,
dummy3,
	ID_prev,
	ID_next,
dummy4,
	ID_properties,
NUMITEMS_PLAY
};

enum
{
	ID_ontop,
	ID_fullscreen,
	ID_screenshot,
//	ID_aspectratio,
dummy5,
	ID_opensubtitles,
	ID_subtitles,
ID_unloadsubtitles,
ID_subtitlessize,
NUMITEMS_VIDEO
};

enum { // ID_subtitlessize SUBMENU
ID_subtitlesbig,
ID_subtitlessmall
};

enum
{
	ID_mute,
	ID_volume_up,
	ID_volume_down,
NUMITEMS_AUDIO
};

enum
{
	ID_mplayergui,
NUMITEMS_SETTINGS
};

/*
void open_menu(void);

void cmd_open_file(void);
void cmd_open_dev(STRPTR proc_name, void *open_fn);
//void cmd_open_dvd(void);
//void cmd_open_dvdnav(void);
//void cmd_open_vcd(void);
//void cmd_open_network(void);
void cmd_play(void);

void add_file_to_que(char *FileName);
void cmd_open_subtitles(void);
void cmd_properties(void);
void cmd_mplayergui(void);

void menu_project(ULONG menucode);
void menu_play(ULONG menucode);
void menu_video(ULONG menucode);
// void menu_aspect(ULONG menucode);
void menu_audio(ULONG menucode);
void menu_settings(ULONG menucode);
*/

void (*select_menu[]) (ULONG menucode) =
{
	menu_project,
	menu_play,
	menu_video,
//	menu_aspect,
	menu_audio,
menu_settings
};

struct pmpMessage {
	char text[1024];
	ULONG type;
	ULONG image;
};

struct pmpMessage errmsg =
{
	"Error",0,0
};

// int play_start_id   = 11;
// int video_start_id  = 17;
#define PLAY_START_ID         NUMITEMS_PROJECT + 1 + 1            // +MSG_Menu_Project and we are "pointing" to next item
#define VIDEO_START_ID        PLAY_START_ID + NUMITEMS_PLAY + 1   // we are "pointing" to next item
// #define ASPECTRATIO_START_ID  VIDEO_START_ID + ID_aspectratio + 1 // +MSG_Menu_Video

int spawn_count = 0;


/****************** MENU DEFINITION *******************/
static struct Menu *menu;

// MenuClass Menu [zzd10h]
enum
{
    MID_MENU_OPENFILE_MENUCLASS = 1,
    MID_MENU_OPENDVD_MENUCLASS,
    MID_MENU_OPENDVDNAV_MENUCLASS,
    MID_MENU_OPENVCD_MENUCLASS,
    MID_MENU_OPENNETWORK_MENUCLASS,
    MID_MENU_ICONIFY_MENUCLASS,
    MID_MENU_ABOUT_MENUCLASS,
    MID_MENU_QUIT_MENUCLASS,
    MID_MENU_PLAYPAUSE_MENUCLASS,
    MID_MENU_STOP_MENUCLASS,
MID_MENU_RECORD_MENUCLASS,
    MID_MENU_LOOP_MENUCLASS,
    MID_MENU_PREVFILE_MENUCLASS,
    MID_MENU_NEXTFILE_MENUCLASS,
    MID_MENU_PROPERTIES_MENUCLASS,
    MID_MENU_STAYONTOP_MENUCLASS,
    MID_MENU_OPENSUBTITLES_MENUCLASS,
    MID_MENU_CYCLESUBTITLES_MENUCLASS,
MID_MENU_UNLOADSUBTITLES_MENUCLASS,
//MID_MENU_SUBTITLESSIZE_MENUCLASS,
MID_MENU_SUBTITLESBIG_MENUCLASS,
MID_MENU_SUBTITLESSMALL_MENUCLASS,
    MID_MENU_FULLSCREEN_MENUCLASS,
    MID_MENU_SCREENSHOT_MENUCLASS,
MID_MENU_ASPECTRATIO_MENUCLASS,
MID_MENU_AR_ORIGINAL_MENUCLASS,
MID_MENU_AR_16_10_MENUCLASS,
MID_MENU_AR_16_9_MENUCLASS,
MID_MENU_AR_185_1_MENUCLASS,
MID_MENU_AR_221_1_MENUCLASS,
MID_MENU_AR_235_1_MENUCLASS,
MID_MENU_AR_239_1_MENUCLASS,
MID_MENU_AR_5_3_MENUCLASS,
MID_MENU_AR_4_3_MENUCLASS,
MID_MENU_AR_5_4_MENUCLASS,
// MID_MENU_AR_1_1_MENUCLASS,
    MID_MENU_MUTE_MENUCLASS,
    MID_MENU_VOLUMEUP_MENUCLASS,
    MID_MENU_VOLUMEDOWN_MENUCLASS,
    MID_MENU_MPLAYERGUI_MENUCLASS,

    MID_MENU_AUTO_BASE = 1000
};

Object *amiga_menuClass = NULL ;     // Menu for MenuClass of AmigaOS 4.1 FE
/*Object *menuobj_MID_MENU_OPENFILE_MENUCLASS;
Object *menuobj_MID_MENU_OPENDVD_MENUCLASS;
Object *menuobj_MID_MENU_OPENDVDNAV_MENUCLASS;
Object *menuobj_MID_MENU_OPENVCD_MENUCLASS;
Object *menuobj_MID_MENU_OPENNETWORK_MENUCLASS;
Object *menuobj_MID_MENU_ICONIFY_MENUCLASS;
Object *menuobj_MID_MENU_ABOUT_MENUCLASS;
Object *menuobj_MID_MENU_QUIT_MENUCLASS;
Object *menuobj_MID_MENU_PLAYPAUSE_MENUCLASS;
Object *menuobj_MID_MENU_STOP_MENUCLASS;
Object *menuobj_MID_MENU_LOOP_MENUCLASS;
Object *menuobj_MID_MENU_PREVFILE_MENUCLASS;
Object *menuobj_MID_MENU_NEXTFILE_MENUCLASS;
Object *menuobj_MID_MENU_STAYONTOP_MENUCLASS;
Object *menuobj_MID_MENU_FULLSCREEN_MENUCLASS;
Object *menuobj_MID_MENU_SCREENSHOT_MENUCLASS;
Object *menuobj_MID_MENU_CYCLESUBTITLES_MENUCLASS;
//	Object *menuobj_MID_MENU_ASPECTRATIO_MENUCLASS;
Object *menuobj_MID_MENU_MUTE_MENUCLASS;
Object *menuobj_MID_MENU_VOLUMEUP_MENUCLASS;
Object *menuobj_MID_MENU_VOLUMEDOWN_MENUCLASS;
Object *menuobj_MID_MENU_OPENSUBTITLES_MENUCLASS;
Object *menuobj_MID_MENU_PROPERTIES_MENUCLASS;
Object *menuobj_MID_MENU_MPLAYERGUI_MENUCLASS;*/

enum    // Root menus
{
    PROJECT_MENU,
    PLAY_MENU,
    VIDEO_OPTIONS_MENU,
    AUDIO_OPTIONS_MENU,
    SETTINGS_MENU
};

enum    // Items of PROJECT_MENU NewMenu menu
{
    MENU_OPENFILE,
    MENU_OPENDVD,
    OPENDVDNAV,
    OPENVCD,
    OPENNETWORK,
    BAR1,
    MENU_ABOUT,
    MENU_ICONIFY,
    BAR2,
    MENU_QUIT
};

enum    // Items of PLAY_MENU NewMenu menu
{
    MENU_PLAYPAUSE,
    MENU_STOP,
MENU_RECORD,
    MENU_LOOP,
BAR3,
    MENU_PREVFILE,
    MENU_NEXTFILE,
BAR4,
MENU_PROPERTIES
};

enum    // Items of VIDEO_OPTIONS_MENU NewMenu menu
{
    MENU_STAYONTOP,
    MENU_FULLSCREEN,
    MENU_SCREENSHOT,
MENU_ASPECTRATIO,
BAR5,
    MENU_OPENSUBTITLES,
    MENU_CYCLESUBTITLES,
MENU_UNLOADSUBTITLES,
MENU_SUBTITLESSIZE,
MENU_SUBTITLESBIG,
MENU_SUBTITLESSMALL
};

enum    // Items of AUDIO_OPTIONS_MENU NewMenu menu
{
    MENU_MUTE,
    MENU_VOLUMEUP,
    MENU_VOLUMEDOWN
};

enum    // Items of SETTINGS_MENU NewMenu menu
{
    MENU_MPLAYERGUI
};

struct NewMenu amiga_menu[] =
{// Type, Label, CommKey, Flags, MutualExclude, UserData
	{ NM_TITLE, menu_Project, NULL, 0, 0L, NULL },
		{ NM_ITEM,  menu_Project_OpenFile,     (STRPTR)"F", 0, 0L, NULL },
		{ NM_ITEM,  menu_Project_OpenDVD,      (STRPTR)"D", 0, 0L, NULL },
		{ NM_ITEM,  menu_Project_OpenDVDNAV,   (STRPTR)"N", 0, 0L, NULL },
		{ NM_ITEM,  menu_Project_OpenVCD,      (STRPTR)"V", 0, 0L, NULL },
		{ NM_ITEM,  menu_Project_OpenNetwork,  (STRPTR)"T", 0, 0L, NULL },
		{ NM_ITEM, NM_BARLABEL, 0, 0, 0, 0},
		{ NM_ITEM,  menu_Project_AboutMPlayer, (STRPTR)"?", 0, 0L, NULL },
		{ NM_ITEM,  menu_Project_Iconify,      (STRPTR)"I", 0, 0L, NULL },
		{ NM_ITEM, NM_BARLABEL, 0, 0, 0, 0},
		{ NM_ITEM,  menu_Project_Quit,         (STRPTR)"Q", 0, 0L, NULL },

	{ NM_TITLE, menu_Play, NULL, 0, 0L, NULL },
		{ NM_ITEM,  menu_Play_PlayPause,  (STRPTR)"P",                  0, 0L, NULL },
		{ NM_ITEM,  menu_Play_Stop,       (STRPTR)"S",                  0, 0L, NULL },
//		{ NM_ITEM,  menu_Play_Record,     (STRPTR)"R", MENUTOGGLE|CHECKIT, 0L, NULL },
		{ NM_ITEM,  menu_Play_Loop,       (STRPTR)"L", MENUTOGGLE|CHECKIT, 0L, NULL },
		{ NM_ITEM, NM_BARLABEL, NULL, 0, 0L, NULL },
		{ NM_ITEM,  menu_Play_PrevFilm,   (STRPTR)"E",                  0, 0L, NULL },
		{ NM_ITEM,  menu_Play_NextFilm,   (STRPTR)"X",                  0, 0L, NULL },
		{ NM_ITEM, NM_BARLABEL, NULL, 0, 0L, NULL },
		{ NM_ITEM,  menu_Play_Properties, (STRPTR)"O",                  0, 0L, NULL },

	{ NM_TITLE, menu_OptionsVideo, NULL, 0, 0L, NULL },
		{ NM_ITEM,  menu_OptionsVideo_StayOnTop,  NULL, MENUTOGGLE|CHECKIT, 0L, NULL },
		{ NM_ITEM,  menu_OptionsVideo_Fullscreen, NULL,                  0, 0L, NULL },
		{ NM_ITEM,  menu_OptionsVideo_Screenshot, NULL,                  0, 0L, NULL },
//		{ NM_ITEM,  (STRPTR)"Normal Dimension",  (STRPTR)"1",  0, 0L, NULL },
//		{ NM_ITEM,  (STRPTR)"Doubled Dimension", (STRPTR)"2",  0, 0L, NULL },
//		{ NM_ITEM,  (STRPTR)"Screen Dimension",  (STRPTR)"3",  0, 0L, NULL },
//		{ NM_ITEM,  (STRPTR)"Half Dimension",    (STRPTR)"4",  0, 0L, NULL },
//		{ NM_ITEM,  NM_BARLABEL, NULL, 0, 0L, NULL },
// 23
		/*{ NM_ITEM,  menu_OptionsVideo_AspectRatio, NULL, 0, 0L, NULL },
			{ NM_SUB,   menu_OptionsVideo_AR_Original,   NULL, CHECKIT|CHECKED,    ~1, NULL },
			{ NM_SUB,   (STRPTR)"16:10",                 NULL,         CHECKIT,    ~2, NULL },
			{ NM_SUB,   (STRPTR)"16:9",                  NULL,         CHECKIT,    ~4, NULL },
			{ NM_SUB,   (STRPTR)"1.85:1",                NULL,         CHECKIT,    ~8, NULL },
			{ NM_SUB,   (STRPTR)"2.21:1",                NULL,         CHECKIT,   ~16, NULL },
			{ NM_SUB,   (STRPTR)"2.35:1",                NULL,         CHECKIT,   ~32, NULL },
			{ NM_SUB,   (STRPTR)"2.39:1",                NULL,         CHECKIT,   ~64, NULL },
			{ NM_SUB,   (STRPTR)"5:3",                   NULL,         CHECKIT,  ~128, NULL },
			{ NM_SUB,   (STRPTR)"4:3",                   NULL,         CHECKIT,  ~256, NULL },
			{ NM_SUB,   (STRPTR)"5:4",                   NULL,         CHECKIT,  ~512, NULL },
//			{ NM_SUB,   (STRPTR)"1:1",                   NULL,         CHECKIT, ~1024, NULL },*/
		{ NM_ITEM, NM_BARLABEL, NULL, 0, 0L, NULL },
		{ NM_ITEM,  menu_OptionsVideo_OpenSubtitles,   NULL,               0,    0L, NULL },
		{ NM_ITEM,  menu_OptionsVideo_CycleSubtitles,  NULL,               0,    0L, NULL },
		{ NM_ITEM,  menu_OptionsVideo_UnloadSubtitles, NULL,               0,    0L, NULL },
{ NM_ITEM,  menu_OptionsVideo_SubtitlesSize, NULL,               0,    0L, NULL },
	{ NM_SUB,  menu_OptionsVideo_SubtitlesBig,   NULL,               0,    0L, NULL },
	{ NM_SUB,  menu_OptionsVideo_SubtitlesSmall, NULL,               0,    0L, NULL },

	{ NM_TITLE, menu_OptionsAudio, NULL, 0, 0L, NULL },
		{ NM_ITEM,  menu_OptionsAudio_Mute,       (STRPTR)"M", MENUTOGGLE|CHECKIT, 0L, NULL },
		{ NM_ITEM,  menu_OptionsAudio_VolumeUp,   (STRPTR)"+",                  0, 0L, NULL },
		{ NM_ITEM,  menu_OptionsAudio_VolumeDown, (STRPTR)"-",                  0, 0L, NULL },

	{ NM_TITLE, menu_Settings, NULL, 0, 0L, NULL },
		{ NM_ITEM,  menu_Settings_MPlayerGUI, (STRPTR)"G", 0, 0L, NULL },

		{ NM_END, NULL, NULL, 0, 0L, NULL }
};

void createMenuTranslation(void)
{
	if (MENUCLASS_SUPPORT) return;

	strcpy(menu_Project,CS(MSG_Menu_Project));
	strcpy(menu_Project_OpenFile,CS(MSG_Menu_Project_OpenFile));
	strcpy(menu_Project_OpenDVD,CS(MSG_Menu_Project_OpenDVD));
	strcpy(menu_Project_OpenDVDNAV,CS(MSG_Menu_Project_OpenDVDNAV));
	strcpy(menu_Project_OpenVCD,CS(MSG_Menu_Project_OpenVCD));
	strcpy(menu_Project_OpenNetwork,CS(MSG_Menu_Project_OpenNetwork));
	strcpy(menu_Project_Iconify,CS(MSG_Menu_Project_Iconify));
	strcpy(menu_Project_AboutMPlayer,CS(MSG_Menu_Project_About));
	strcpy(menu_Project_Quit,CS(MSG_Menu_Project_Quit));

	strcpy(menu_Play,CS(MSG_Menu_Play));
	strcpy(menu_Play_PlayPause,CS(MSG_Menu_Play_PlayPause));
	strcpy(menu_Play_Stop,CS(MSG_Menu_Play_Stop));
	strcpy(menu_Play_Record,CS(MSG_Menu_Play_Record));
	strcpy(menu_Play_Loop,CS(MSG_Menu_Play_Loop));
	strcpy(menu_Play_PrevFilm,CS(MSG_Menu_Play_PrevFile));
	strcpy(menu_Play_NextFilm,CS(MSG_Menu_Play_NextFile));
	strcpy(menu_Play_Properties,CS(MSG_Menu_Play_Properties));

	strcpy(menu_OptionsVideo,CS(MSG_Menu_OptionsVideo));
	strcpy(menu_OptionsVideo_StayOnTop,CS(MSG_Menu_OptionsVideo_StayOnTop));
	strcpy(menu_OptionsVideo_OpenSubtitles,CS(MSG_Menu_OptionsVideo_OpenSubtitles));
	strcpy(menu_OptionsVideo_CycleSubtitles,CS(MSG_Menu_OptionsVideo_CycleSubtitles));
strcpy(menu_OptionsVideo_UnloadSubtitles,CS(MSG_Menu_OptionsVideo_UnloadSubtitles));
strcpy(menu_OptionsVideo_SubtitlesSize,CS(MSG_Menu_OptionsVideo_SubtitlesSize));
strcpy(menu_OptionsVideo_SubtitlesBig,CS(MSG_Menu_OptionsVideo_SubtitlesBig));
strcpy(menu_OptionsVideo_SubtitlesSmall,CS(MSG_Menu_OptionsVideo_SubtitlesSmall));
	strcpy(menu_OptionsVideo_Fullscreen,CS(MSG_Menu_OptionsVideo_Fullscreen));
	strcpy(menu_OptionsVideo_Screenshot,CS(MSG_Menu_OptionsVideo_Screenshot));
strcpy(menu_OptionsVideo_AspectRatio,CS(MSG_Menu_OptionsVideo_AspectRatio));
strcpy(menu_OptionsVideo_AR_Original,CS(MSG_Menu_OptionsVideo_AR_Original));

	strcpy(menu_OptionsAudio,CS(MSG_Menu_OptionsAudio));
	strcpy(menu_OptionsAudio_Mute,CS(MSG_Menu_OptionsAudio_Mute));
	strcpy(menu_OptionsAudio_VolumeUp,CS(MSG_Menu_OptionsAudio_VolumeUp));
	strcpy(menu_OptionsAudio_VolumeDown,CS(MSG_Menu_OptionsAudio_VolumeDown));

	strcpy(menu_Settings,CS(MSG_Menu_Settings));
	strcpy(menu_Settings_MPlayerGUI,CS(MSG_Menu_Settings_MPlayerGUI));
}


/*void findMenuClassObj(void)	// zzd10h
{
	if (amiga_menuClass)
	{
		menuobj_MID_MENU_OPENFILE_MENUCLASS    = (Object *)IDoMethod(amiga_menuClass,MM_FINDID,0,  MID_MENU_OPENFILE_MENUCLASS);
		menuobj_MID_MENU_OPENDVD_MENUCLASS     = (Object *)IDoMethod(amiga_menuClass,MM_FINDID,0,  MID_MENU_OPENDVD_MENUCLASS);
		menuobj_MID_MENU_OPENDVDNAV_MENUCLASS  = (Object *)IDoMethod(amiga_menuClass,MM_FINDID,0,  MID_MENU_OPENDVDNAV_MENUCLASS);
		menuobj_MID_MENU_OPENVCD_MENUCLASS     = (Object *)IDoMethod(amiga_menuClass,MM_FINDID,0,  MID_MENU_OPENVCD_MENUCLASS);
		menuobj_MID_MENU_OPENNETWORK_MENUCLASS = (Object *)IDoMethod(amiga_menuClass,MM_FINDID,0,  MID_MENU_OPENNETWORK_MENUCLASS);
		menuobj_MID_MENU_ABOUT_MENUCLASS       = (Object *)IDoMethod(amiga_menuClass,MM_FINDID,0,  MID_MENU_ABOUT_MENUCLASS);
		menuobj_MID_MENU_ICONIFY_MENUCLASS     = (Object *)IDoMethod(amiga_menuClass,MM_FINDID,0,  MID_MENU_ICONIFY_MENUCLASS);
		menuobj_MID_MENU_QUIT_MENUCLASS        = (Object *)IDoMethod(amiga_menuClass,MM_FINDID,0,  MID_MENU_QUIT_MENUCLASS);

		menuobj_MID_MENU_PLAYPAUSE_MENUCLASS  = (Object *)IDoMethod(amiga_menuClass,MM_FINDID,0,  MID_MENU_PLAYPAUSE_MENUCLASS);
		menuobj_MID_MENU_STOP_MENUCLASS       = (Object *)IDoMethod(amiga_menuClass,MM_FINDID,0,  MID_MENU_STOP_MENUCLASS);
		menuobj_MID_MENU_LOOP_MENUCLASS       = (Object *)IDoMethod(amiga_menuClass,MM_FINDID,0,  MID_MENU_LOOP_MENUCLASS);
		menuobj_MID_MENU_PREVFILE_MENUCLASS   = (Object *)IDoMethod(amiga_menuClass,MM_FINDID,0,  MID_MENU_PREVFILE_MENUCLASS);
		menuobj_MID_MENU_NEXTFILE_MENUCLASS   = (Object *)IDoMethod(amiga_menuClass,MM_FINDID,0,  MID_MENU_NEXTFILE_MENUCLASS);
		menuobj_MID_MENU_PROPERTIES_MENUCLASS = (Object *)IDoMethod(amiga_menuClass,MM_FINDID,0,  MID_MENU_PROPERTIES_MENUCLASS);

		menuobj_MID_MENU_STAYONTOP_MENUCLASS      = (Object *)IDoMethod(amiga_menuClass,MM_FINDID,0,  MID_MENU_STAYONTOP_MENUCLASS);
		menuobj_MID_MENU_FULLSCREEN_MENUCLASS     = (Object *)IDoMethod(amiga_menuClass,MM_FINDID,0,  MID_MENU_FULLSCREEN_MENUCLASS);
		menuobj_MID_MENU_SCREENSHOT_MENUCLASS     = (Object *)IDoMethod(amiga_menuClass,MM_FINDID,0,  MID_MENU_SCREENSHOT_MENUCLASS);
		menuobj_MID_MENU_OPENSUBTITLES_MENUCLASS  = (Object *)IDoMethod(amiga_menuClass,MM_FINDID,0,  MID_MENU_OPENSUBTITLES_MENUCLASS);
		menuobj_MID_MENU_CYCLESUBTITLES_MENUCLASS = (Object *)IDoMethod(amiga_menuClass,MM_FINDID,0,  MID_MENU_CYCLESUBTITLES_MENUCLASS);
//		menuobj_MID_MENU_ASPECTRATIO_MENUCLASS 	= (Object *)IDoMethod(amiga_menuClass,MM_FINDID,0,  MID_MENU_ASPECTRATIO_MENUCLASS);

		menuobj_MID_MENU_MUTE_MENUCLASS       = (Object *)IDoMethod(amiga_menuClass,MM_FINDID,0,  MID_MENU_MUTE_MENUCLASS);
		menuobj_MID_MENU_VOLUMEUP_MENUCLASS   = (Object *)IDoMethod(amiga_menuClass,MM_FINDID,0,  MID_MENU_VOLUMEUP_MENUCLASS);
		menuobj_MID_MENU_VOLUMEDOWN_MENUCLASS = (Object *)IDoMethod(amiga_menuClass,MM_FINDID,0,  MID_MENU_VOLUMEDOWN_MENUCLASS);

		menuobj_MID_MENU_MPLAYERGUI_MENUCLASS = (Object *)IDoMethod(amiga_menuClass,MM_FINDID,0,  MID_MENU_MPLAYERGUI_MENUCLASS);
	}
}*/

static struct Image *MenuImage(STRPTR name, struct Screen *screen)
{
   struct Image *i = NULL;
   APTR prev_win;
   BPTR dir, prev_dir;
   STRPTR name_s, name_g;
   uint32 len;

  if (AISS_MENU && MENUCLASS_SUPPORT)
  {
   len = strlen(name);

   name_s = AllocVecTags(len + 3 + len + 3,TAG_END);

   if (name_s)
   {
      name_g = name_s + len + 3;

      strcpy(name_s,name);
      strcat(name_s,"_s");

      strcpy(name_g,name);
      strcat(name_g,"_g");

      prev_win = SetProcWindow((APTR)-1);		// Disable requesters

      dir = Lock("TBIMAGES:",SHARED_LOCK);
      // dir = Lock(GUIIMAGES,SHARED_LOCK);	// To lower tbimages scan

      SetProcWindow(prev_win);			// Re-enable requesters

      if (dir != ZERO)
      {
         prev_dir = SetCurrentDir(dir);

         i = (struct Image *)NewObject(BitMapClass, NULL, // "bitmap.image",
                                       BITMAP_SourceFile, name,
                                       BITMAP_SelectSourceFile, name_s,
                                       BITMAP_DisabledSourceFile, name_g,
                                       BITMAP_Screen, screen,
                                       BITMAP_Masking, TRUE,
                                       // Scale the AISS_MENU picture if AISS_MENU_TO_SCALE = TRUE (ENV:MenuImageSize > 0)
    		                       AISS_MENU_TO_SCALE ? IA_Scalable : TAG_IGNORE, TRUE,
				       AISS_MENU_TO_SCALE ? IA_Width : TAG_IGNORE , AISS_MENU_SIZE,
				       AISS_MENU_TO_SCALE ? IA_Height : TAG_IGNORE , AISS_MENU_SIZE,
				       // If AISS picture size <= screen font height, up it
				       // AISS_MENU_TO_SCALE && (AISS_MENU_SIZE <= screen->RastPort.TxHeight) ? IA_Top : TAG_IGNORE , -1,
                                      TAG_END);

         if (i && !AISS_MENU_TO_SCALE)
            SetAttrs((Object *)i,IA_Height,i->Height + 2,TAG_END);

         SetCurrentDir(prev_dir);

         UnLock(dir);
      }

      FreeVec(name_s);
   }
  } // End if AISS_MENU
   return (i);
}

void makeMenuClass(void)	// zzd10h
{
	struct Screen *screen;

	screen = FrontMostScr();
	//screen = LockPubScreen(NULL);
	if(screen)
	{
DBUG("makeMenuClass() screen=0x%08lx\n",screen);
		amiga_menuClass = MStrip,

                   MA_AddChild, MTitle(CS(MSG_Menu_Project)),	// PROJECT_MENU menu
                     MA_AddChild, MItem(CS(MSG_Menu_Project_OpenFile)),
                       MA_Key,   "F",
                       MA_Image, MenuImage("pictureload",screen),
                       MA_ID,    MID_MENU_OPENFILE_MENUCLASS,
                     MEnd,
                     MA_AddChild, MItem(CS(MSG_Menu_Project_OpenDVD)),
                       MA_Key,   "D",
                       MA_Image, MenuImage("dvd",screen),
                       MA_ID,    MID_MENU_OPENDVD_MENUCLASS,
                     MEnd,
                     MA_AddChild, MItem(CS(MSG_Menu_Project_OpenDVDNAV)),
                       MA_Key,   "N",
                       MA_Image, MenuImage("blueray",screen),
                       MA_ID,    MID_MENU_OPENDVDNAV_MENUCLASS,
                     MEnd,
                     MA_AddChild, MItem(CS(MSG_Menu_Project_OpenVCD)),
                       MA_Key,   "V",
                       MA_Image, MenuImage("cd",screen),
                       MA_ID,    MID_MENU_OPENVCD_MENUCLASS,
                     MEnd,
                     MA_AddChild, MItem(CS(MSG_Menu_Project_OpenNetwork)),
                       MA_Key,   "T",
                       MA_Image, MenuImage("download",screen),
                       MA_ID,    MID_MENU_OPENNETWORK_MENUCLASS,
                     MEnd,
                     MA_AddChild, MSeparator, MEnd,

                     MA_AddChild, MItem(CS(MSG_Menu_Project_About)),
                       MA_Key,   "?",
                       MA_Image, MenuImage("info",screen),
                       MA_ID,    MID_MENU_ABOUT_MENUCLASS,
                     MEnd,
                     MA_AddChild, MItem(CS(MSG_Menu_Project_Iconify)),
                       MA_Key,   "I",
                       MA_Image, MenuImage("iconify",screen),
                       MA_ID,    MID_MENU_ICONIFY_MENUCLASS,
                     MEnd,
                     MA_AddChild, MSeparator, MEnd,
                     MA_AddChild, MItem(CS(MSG_Menu_Project_Quit)),
                       MA_Key,   "Q",
                       MA_Image, MenuImage("quit",screen),
                       MA_ID,    MID_MENU_QUIT_MENUCLASS,
                     MEnd,
                   MEnd,	// End of PROJECT_MENU menu

                   MA_AddChild, MTitle(CS(MSG_Menu_Play)),	// PLAY_MENU menu
                     MA_AddChild, MItem(CS(MSG_Menu_Play_PlayPause)),
                       MA_Key,   "P",
                       MA_Image, MenuImage("tapeplay",screen),
                       MA_ID,    MID_MENU_PLAYPAUSE_MENUCLASS,
                     MEnd,
                     MA_AddChild, MItem(CS(MSG_Menu_Play_Stop)),
                       MA_Key,   "S",
                       MA_Image, MenuImage("tapestop",screen),
                       MA_ID,    MID_MENU_STOP_MENUCLASS,
                     MEnd,
                     MA_AddChild, MItem(CS(MSG_Menu_Play_Loop)),
                       MA_Key,      "L",
                       MA_Toggle,   TRUE,
                       MA_Selected, loop_on(),
                       MA_Image,    MenuImage("tapeloop",screen),
                       MA_ID,       MID_MENU_LOOP_MENUCLASS,
                     MEnd,
                     /*MA_AddChild, MItem(CS(MSG_Menu_Play_Record)),
                       MA_Key, "R",
                       MA_Toggle, TRUE,
                       // MA_Selected, record_on(),
                       MA_Image, MenuImage("taperec",screen),
                       MA_ID, MID_MENU_RECORD_MENUCLASS,
                     MEnd,*/
                     MA_AddChild, MSeparator, MEnd,
                     MA_AddChild, MItem(CS(MSG_Menu_Play_PrevFile)),
                       MA_Key,   "E",
                       MA_Image, MenuImage("tapelast",screen),
                       MA_ID,    MID_MENU_PREVFILE_MENUCLASS,
                     MEnd,
                     MA_AddChild, MItem(CS(MSG_Menu_Play_NextFile)),
                       MA_Key,   "X",
                       MA_Image, MenuImage("tapenext",screen),
                       MA_ID,    MID_MENU_NEXTFILE_MENUCLASS,
                     MEnd,
                     MA_AddChild, MSeparator, MEnd,
                     MA_AddChild, MItem(CS(MSG_Menu_Play_Properties)),
                       MA_Key,   "O",
                       MA_Image, MenuImage("infobubble",screen),
                       MA_ID,    MID_MENU_PROPERTIES_MENUCLASS,
                     MEnd,
                   MEnd,	// End of PLAY_MENU menu

                   MA_AddChild, MTitle(CS(MSG_Menu_OptionsVideo)),	// VIDEO_OPTIONS_MENU menu
                     MA_AddChild, MItem(CS(MSG_Menu_OptionsVideo_StayOnTop)),
                       MA_Toggle,   TRUE,
                       MA_Selected, is_ontop,
                       MA_Image,    MenuImage("listerclone",screen),
                       MA_ID,       MID_MENU_STAYONTOP_MENUCLASS,
                     MEnd,
                     MA_AddChild, MItem(CS(MSG_Menu_OptionsVideo_Fullscreen)),
                       MA_Image, MenuImage("fullscreen",screen),
                       MA_ID,    MID_MENU_FULLSCREEN_MENUCLASS,
                     MEnd,
                     MA_AddChild, MItem(CS(MSG_Menu_OptionsVideo_Screenshot)),
                       MA_Image, MenuImage("snapshot",screen),
                       MA_ID,    MID_MENU_SCREENSHOT_MENUCLASS,
                     MEnd,
                     /*MA_AddChild, MSeparator, MEnd,
                     MA_AddChild, MItem(CS(MSG_Menu_OptionsVideo_AspectRatio)),
                       MA_Image, MenuImage("blockquadrat",screen),
                       // MA_ID, MID_MENU_ASPECTRATIO_MENUCLASS,
                       MA_AddChild, MItem(CS(MSG_Menu_OptionsVideo_AR_Original)),
                         MA_ID, MID_MENU_AR_ORIGINAL_MENUCLASS,
                           MA_MX,       ~(1<<0) & 0xFFFFFFF, // POS 1st on this subitem -> ~0
                           MA_Selected, TRUE,
                         MEnd,
                         MA_AddChild, MItem("16:10"),
                           MA_ID, MID_MENU_AR_16_10_MENUCLASS,
                           MA_MX,       ~(1<<1) & 0xFFFFFFF, // POS 2nd on this subitem -> ~1
                           MA_Selected, FALSE,
                         MEnd,
  MA_AddChild, MItem("16:9"),
    MA_ID, MID_MENU_AR_16_9_MENUCLASS,
    MA_MX,       ~(1<<2) & 0xFFFFFFF, // POS 3nd on this subitem -> ~2
    MA_Selected, FALSE,
  MEnd,
  MA_AddChild, MItem("1.85:1"),
    MA_ID, MID_MENU_AR_185_1_MENUCLASS,
    MA_MX,       ~(1<<3) & 0xFFFFFFF,
    MA_Selected, FALSE,
  MEnd,
  MA_AddChild, MItem("2.21:1"),
    MA_ID, MID_MENU_AR_221_1_MENUCLASS,
    MA_MX,       ~(1<<4) & 0xFFFFFFF,
    MA_Selected, FALSE,
  MEnd,
  MA_AddChild, MItem("2.35:1"),
    MA_ID, MID_MENU_AR_235_1_MENUCLASS,
    MA_MX,       ~(1<<5) & 0xFFFFFFF,
    MA_Selected, FALSE,
  MEnd,
  MA_AddChild, MItem("2.39:1"),
    MA_ID, MID_MENU_AR_239_1_MENUCLASS,
    MA_MX,       ~(1<<6) & 0xFFFFFFF,
    MA_Selected, FALSE,
  MEnd,
  MA_AddChild, MItem("5:3"),
    MA_ID, MID_MENU_AR_5_3_MENUCLASS,
    MA_MX,       ~(1<<7) & 0xFFFFFFF,
    MA_Selected, FALSE,
  MEnd,
  MA_AddChild, MItem("4:3"),
    MA_ID, MID_MENU_AR_4_3_MENUCLASS,
    MA_MX,       ~(1<<8) & 0xFFFFFFF,
    MA_Selected, FALSE,
  MEnd,
  MA_AddChild, MItem("5:4"),
    MA_ID, MID_MENU_AR_5_4_MENUCLASS,
    MA_MX,       ~(1<<9) & 0xFFFFFFF,
    MA_Selected, FALSE,
  MEnd,
//  MA_AddChild, MItem("1:1"),
//    MA_ID, MID_MENU_AR_1_1_MENUCLASS,
//    MA_MX,       ~(1<<10) & 0xFFFFFFF,
//    MA_Selected, FALSE,
//  MEnd,
MEnd,*/
                     MA_AddChild, MSeparator, MEnd,
                     MA_AddChild, MItem(CS(MSG_Menu_OptionsVideo_OpenSubtitles)),
                       MA_Image, MenuImage("fieldsadd",screen), // "textframe"
                       MA_ID,    MID_MENU_OPENSUBTITLES_MENUCLASS,
                     MEnd,
                     MA_AddChild, MItem(CS(MSG_Menu_OptionsVideo_CycleSubtitles)),
                       MA_Image, MenuImage("refresh",screen),
                       MA_ID,    MID_MENU_CYCLESUBTITLES_MENUCLASS,
                     MEnd,
                     MA_AddChild, MItem(CS(MSG_Menu_OptionsVideo_UnloadSubtitles)),
                       MA_Image, MenuImage("fieldsremove",screen),
                       MA_ID,    MID_MENU_UNLOADSUBTITLES_MENUCLASS,
                     MEnd,
                     MA_AddChild, MItem(CS(MSG_Menu_OptionsVideo_SubtitlesSize)),
                       MA_Image, MenuImage("font_size",screen),
                       //MA_ID,    MID_MENU_SUBTITLESSIZE_MENUCLASS,
                       MA_AddChild, MItem(CS(MSG_Menu_OptionsVideo_SubtitlesBig)),
                         MA_Image, MenuImage("font_larger",screen),
                         MA_ID,    MID_MENU_SUBTITLESBIG_MENUCLASS,
                       MEnd,
                       MA_AddChild, MItem(CS(MSG_Menu_OptionsVideo_SubtitlesSmall)),
                         MA_Image, MenuImage("font_smaller",screen),
                         MA_ID,    MID_MENU_SUBTITLESSMALL_MENUCLASS,
                       MEnd,
                     MEnd,
                   MEnd,	// End of VIDEO_OPTIONS_MENU menu

                   MA_AddChild, MTitle(CS(MSG_Menu_OptionsAudio)),	// AUDIO_OPTIONS_MENU menu
                     MA_AddChild, MItem(CS(MSG_Menu_OptionsAudio_Mute)),
                       MA_Key,      "M",
                       MA_Toggle,   TRUE,
                       MA_Selected, is_mute,
                       MA_Image,    MenuImage("soundmute",screen),
                       MA_ID,       MID_MENU_MUTE_MENUCLASS,
                     MEnd,
                     MA_AddChild, MItem(CS(MSG_Menu_OptionsAudio_VolumeUp)),
                       MA_Key,   "+",
                       MA_Image, MenuImage("add",screen), // "arrowupfilled",screen), // "add"
                       MA_ID,    MID_MENU_VOLUMEUP_MENUCLASS,
                     MEnd,
                     MA_AddChild, MItem(CS(MSG_Menu_OptionsAudio_VolumeDown)),
                       MA_Key,   "-",
                       MA_Image, MenuImage("remove",screen), // "arrowdownfilled",screen), // "remove"
                       MA_ID,    MID_MENU_VOLUMEDOWN_MENUCLASS,
                     MEnd,
                   MEnd,	// End of AUDIO_OPTIONS_MENU menu

                   MA_AddChild, MTitle(CS(MSG_Menu_Settings)),	// SETTINGS_MENU menu
                     MA_AddChild, MItem(CS(MSG_Menu_Settings_MPlayerGUI)),
                       MA_Key,   "G",
                       MA_Image, MenuImage("player",screen),
                       MA_ID,    MID_MENU_MPLAYERGUI_MENUCLASS,
                     MEnd,
                   MEnd,	// End of SETTINGS_MENU menu

                 MEnd;  // End of menu

		//UnlockPubScreen(NULL,screen) ;
	}
}


struct Window *Menu_Window;

void attach_menu(struct Window *window)
{
	// struct Menu *menu;
DBUG("attach_menu() win=0x%08lx\n",window);
	// Create translation for NewMenu and MenuClass menu [zzd10h]
	createMenuTranslation() ;

	open_menu();

DBUG("  is_fullscreen=0x%08lx\n",is_fullscreen);

	if (( (menu || amiga_menuClass) ) && (window))
	{
		make_appwindow(window);

		if (!MENUCLASS_SUPPORT)
		{
			struct VisualInfo *vi;

			if (vi = GetVisualInfoA(window -> WScreen,NULL))
			{
				LayoutMenus( menu, vi, GTMN_NewLookMenus,TRUE, TAG_END );
				(amiga_menu+0)->nm_UserData = (APTR)vi;
DBUG("  amiga_menu[0]->nm_UserData = %p (vi)\n",(amiga_menu+0)->nm_UserData);
			}
			SetMenuStrip(window, menu);
		}
		else
		{
			if(is_fullscreen) {
				Object *menu_obj = (Object *)IDoMethod(amiga_menuClass, MM_FINDID, 0, MID_MENU_ICONIFY_MENUCLASS);
				SetAttrs(menu_obj, MA_Disabled,TRUE, TAG_END);
				menu_obj = (Object *)IDoMethod(amiga_menuClass, MM_FINDID, 0, MID_MENU_FULLSCREEN_MENUCLASS);
				SetAttrs(menu_obj, MA_Image,MenuImage("fullscreen_off",window->WScreen), TAG_END);
			}
			SetMenuStrip(window, amiga_menuClass);
		}

		Menu_Window = window;
	}

}


void detach_menu(struct Window *window)
{
	// struct Menu *menu;
DBUG("detach_menu() win=0x%08lx\n",window);
	if (menu)
	{
		/*while (spawn_count)
		{
			// Printf("You need to close the requester windows.\nCan't remove menu from MPlayer, yet\n");
			// Delay(1);
			PrintMsg("MPlayer's requesters still opened!", REQTYPE_INFO, REQIMAGE_ERROR, CS(MSG_Requester_Title_Error));
		}*/

		ClearMenuStrip(window);
		FreeMenus( menu );
		menu = NULL;
		FreeVisualInfo( (struct VisualInfo *)(amiga_menu+0)->nm_UserData );
DBUG(" amiga_menu[0]->nm_UserData = %p (vi))\n",(amiga_menu+0)->nm_UserData);
	}

	if (amiga_menuClass)
		DisposeObject(amiga_menuClass);

	delete_appwindow();
}

void open_menu(void)
{
	//struct Menu *menu;

	if (!MENUCLASS_SUPPORT)
	{
		UWORD i = MENUTOGGLE|CHECKIT;

DBUG("  PLAY_START_ID = %ld  (ID_loop=%ld)\n",PLAY_START_ID,ID_loop);
		if (loop_on()==TRUE) { i |= CHECKED; }
		amiga_menu[ PLAY_START_ID + ID_loop ].nm_Flags = i;

/*
		if (vi)
		{
			if (isStarted == FALSE)
			{
				isStarted = TRUE;
			}
			else
			{
				int i = 0;
*/

		i = MENUTOGGLE|CHECKIT;
DBUG("  VIDEO_START_ID = %ld  (ID_ontop=%ld)\n",VIDEO_START_ID,ID_ontop);
		if (is_ontop==1) { i |= CHECKED; }
		amiga_menu[ VIDEO_START_ID + ID_ontop ].nm_Flags = i;

		amiga_menu[ID_iconify+1].nm_Flags = is_fullscreen? NM_ITEMDISABLED : 0;

/*DBUG("ASPECTRATIO_START_ID = %ld\n",ASPECTRATIO_START_ID);
//		for (i=24;i<35;i++)
		for(i=ASPECTRATIO_START_ID; i<ASPECTRATIO_START_ID+11; ++i) {
//			amiga_menu[i].nm_Flags = MENUTOGGLE|CHECKIT;
		}*/
/*
		switch(AspectRatio)
		{
			case AR_ORIGINAL:
					nm[24].nm_Flags = MENUTOGGLE|CHECKIT|CHECKED;
					break;
			case AR_16_10:
					nm[25].nm_Flags = MENUTOGGLE|CHECKIT|CHECKED;
					break;
			case AR_16_9:
					nm[26].nm_Flags = MENUTOGGLE|CHECKIT|CHECKED;
					break;
			case AR_185_1:
					nm[27].nm_Flags = MENUTOGGLE|CHECKIT|CHECKED;
					break;
			case AR_221_1:
					nm[28].nm_Flags = MENUTOGGLE|CHECKIT|CHECKED;
					break;
			case AR_235_1:
					nm[29].nm_Flags = MENUTOGGLE|CHECKIT|CHECKED;
					break;
			case AR_239_1:
					nm[30].nm_Flags = MENUTOGGLE|CHECKIT|CHECKED;
					break;
			case AR_5_3:
					nm[31].nm_Flags = MENUTOGGLE|CHECKIT|CHECKED;
					break;
			case AR_4_3:
					nm[32].nm_Flags = MENUTOGGLE|CHECKIT|CHECKED;
					break;
			case AR_5_4:
					nm[33].nm_Flags = MENUTOGGLE|CHECKIT|CHECKED;
					break;
			case AR_1_1:
					nm[34].nm_Flags = MENUTOGGLE|CHECKIT|CHECKED;
					break;
		}

	} // END if(vi)
*/

		menu = CreateMenusA(amiga_menu,NULL);
	}
	else
	{
		// SetAttrs(window, WA_NewLookMenus,FALSE, TAG_END);
		makeMenuClass() ;
		// SetAttrs(window, WA_MenuStrip,amiga_menuClass, TAG_END);
//		findMenuClassObj() ;	// populate Menu class item Obj
	}
}

void spawn_died(int32 ret, int32 died_enum)
{
	spawn_count--;
DBUG("spawn_died(): count=%ld  TYPE_%02ld\n",spawn_count,died_enum);
	switch(died_enum) {
		case TYPE_UNUSED: break;
		case TYPE_ABOUT : About_Process = NULL; break;
		case TYPE_ASL   : ASL_Process = NULL; break;
		case TYPE_REQ   : Req_Process = NULL; break;
		default: DebugPrintF("[%s] TYPE_%02ld unknown\n",__FUNCTION__,died_enum); break;
	}
}

struct Process *spawn(void *fn, char *name, int32 died_enum)
{
	struct Process *ret = (struct Process *) CreateNewProcTags (
	                                          NP_Start,     (ULONG) fn,
	                                          NP_Name,      name,
	                                          NP_StackSize, 262144,
	                                          NP_Child,     TRUE,
	                                          NP_Priority,  0,
	                                          NP_ExitData,  IDOS,
	                                          NP_FinalCode, spawn_died,
	                                          NP_FinalData, died_enum,
	                                         TAG_DONE);

	if (ret) spawn_count ++;
DBUG("spawn(): count=%ld (new process=0x%08lx)\n",spawn_count,ret);
	return ret;
}

void seek_start(void)
{
	mp_cmd_t MPCmd;
	mp_cmd_t *c;

DBUG("MP_CMD_SEEK = %d\n",MP_CMD_SEEK);

	bzero(&MPCmd,sizeof(MPCmd));

	MPCmd.pausing = FALSE;
	MPCmd.id      = MP_CMD_SEEK;
	strcpy(MPCmd.name, "seek");
	MPCmd.nargs   = 2;
	MPCmd.args[0].type = MP_CMD_ARG_FLOAT;
	MPCmd.args[0].v.f  = 0/1000.0f;
	MPCmd.args[1].type = MP_CMD_ARG_INT;
	MPCmd.args[1].v.i  = 2;
	MPCmd.args[2].type = -1;
	MPCmd.args[2].v.i  = 0;
	if (( c = mp_cmd_clone(&MPCmd)))
	{
		if ((mp_input_queue_cmd(c)==0))
		{
			mp_cmd_free(c);
		}
		else
		{
			printf("[%s]Failed to queue\n",__FUNCTION__);
		}
	}
	else
	{
		printf("[%s]Failed to clone\n",__FUNCTION__);
	}
}


char *__tmp_file_name = NULL;

static int32 PlayFile_proc(STRPTR args, int32 length, APTR execbase)
{
	if (__tmp_file_name)
	{
		seek_start();
		Delay(8);

		add_file_to_que( __tmp_file_name );
		Delay(8);

		nextfile();

		free(__tmp_file_name);
		__tmp_file_name = NULL;
	}

	return(RETURN_OK);
}

static int32 open_file_proc(STRPTR args, int32 length, APTR execbase)
{
	char *FileName;

	if(get_icommand("pause") == 0) // 0 -> not already in pause
		put_command0(MP_CMD_FRAME_STEP); // play 1 frame and pause

	if (FileName = LoadFile(NULL,EXTPATTERN,CS(MSG_Requester_OpenFile_Video)))
	{
		__tmp_file_name = strdup(FileName);
FreeVec(FileName); FileName = NULL;
		PlayFile_proc(args,length,execbase);
	}

	if(get_icommand("pause") == 1) // 1 -> already in pause
		put_command0(MP_CMD_PAUSE); // resume pause

	return(RETURN_OK);
}

void cmd_open_file(void)
{
//	struct Process *proc;
DBUG("Req_Process=0x%08lx  ASL_Process=0x%08lx\n",Req_Process,ASL_Process);
//	if (choosing_flag == FALSE)
if(Req_Process==NULL  &&  ASL_Process==NULL)
	{
//		proc = spawn( open_file_proc, "Open file" );
//		if (!proc) PrintMsg(errmsg.text, REQTYPE_INFO, REQIMAGE_WARNING, CS(MSG_Requester_Title_Warning));
ASL_Process = spawn( open_file_proc, "Open file", TYPE_ASL );
DBUG("Req_Process=0x%08lx  ASL_Process=0x%08lx\n",Req_Process,ASL_Process);
if (!ASL_Process) PrintMsg(errmsg.text, REQTYPE_INFO, REQIMAGE_WARNING, CS(MSG_Requester_Title_Warning));
	}
	else
		blink_screen() ; // zzd10h
}

int32 PlayFile_async(const char *FileName)
{
	struct Process *proc;

	if (choosing_flag == FALSE)
	{
		__tmp_file_name = strdup(FileName);
		proc = spawn( PlayFile_proc, "MPlayer:async", TYPE_UNUSED );
		if (!proc) PrintMsg(errmsg.text, REQTYPE_INFO, REQIMAGE_WARNING, CS(MSG_Requester_Title_Warning));
	}
	else
		blink_screen() ; // zzd10h

	return RETURN_OK;
}


void cmd_open_dev(STRPTR proc_name, void *open_fn)
{
DBUG("Req_Process=0x%08lx  ASL_Process=0x%08lx\n",Req_Process,ASL_Process);
	if(ASL_Process==NULL  &&  Req_Process==NULL)
	{
		Req_Process = spawn(open_fn, proc_name, TYPE_REQ);
DBUG("Req_Process=0x%08lx  ASL_Process=0x%08lx\n",Req_Process,ASL_Process);
		if (!Req_Process) PrintMsg(errmsg.text, REQTYPE_INFO, REQIMAGE_WARNING, CS(MSG_Requester_Title_Warning));
	}
	else
		blink_screen() ; // zzd10h
}


/*void cmd_open_dvd(void)
{
//	struct Process *proc;
//	if (choosing_flag == FALSE)
//	{
//		proc = spawn( OpenDVD, "Open DVD", TYPE_REQ );
//		if (!proc) PrintMsg(errmsg.text, REQTYPE_INFO, REQIMAGE_WARNING, CS(MSG_Requester_Title_Warning));
//	}
if(Req_Process == NULL)
	{
//		proc = spawn( open_file_proc, "Open file" );
//		if (!proc) PrintMsg(errmsg.text, REQTYPE_INFO, REQIMAGE_WARNING, CS(MSG_Requester_Title_Warning));
Req_Process = spawn( OpenDVD, "Open DVD", TYPE_REQ );
if (!Req_Process) PrintMsg(errmsg.text, REQTYPE_INFO, REQIMAGE_WARNING, CS(MSG_Requester_Title_Warning));
}
	else
		blink_screen() ; // zzd10h
}*/

/*void cmd_open_dvdnav(void)
{
	struct Process *proc;
	if (choosing_flag == FALSE)
	{
		proc = spawn( OpenDVDNAV, "Open DVDNAV", TYPE_REQ );
		if (!proc) PrintMsg(errmsg.text, REQTYPE_INFO, REQIMAGE_WARNING, CS(MSG_Requester_Title_Warning));
	}
	else
		blink_screen() ; // zzd10h
}

void cmd_open_vcd(void)
{
	struct Process *proc;

	if (choosing_flag == FALSE)
	{
		proc = spawn( OpenVCD, "Open VCD", TYPE_REQ );
		if (!proc) PrintMsg(errmsg.text, REQTYPE_INFO, REQIMAGE_WARNING, CS(MSG_Requester_Title_Warning));
	}
	else
		blink_screen() ; // zzd10h
}

void cmd_open_network(void)
{
	struct Process *proc;
	if (choosing_flag == FALSE)
	{
		proc = spawn( OpenNetwork, "Open URL", TYPE_REQ );
		if (!proc) PrintMsg(errmsg.text, REQTYPE_INFO, REQIMAGE_WARNING, CS(MSG_Requester_Title_Warning));
	}
	else
		blink_screen() ; // zzd10h
}*/

void cmd_play(void)
{
	put_command0(MP_CMD_PAUSE);
}

void cmd_stop(void)
{
	put_command0(MP_CMD_STOP);
}

// samo79 - Record function WIP
BOOL record_on(void)
{
return FALSE;
//	if (mpctx->record_times < 0) return FALSE;
//
//	if (mpctx->record_times == 0) return TRUE;
}

void cmd_record(void)
{
	/*if (record_on()==FALSE)
	{
		put_icommand1(MP_CMD_RECORD,1);
	}
	else
	{
		put_icommand1(MP_CMD_RECORD,-1);
	}*/
}


BOOL loop_on(void)
{
	if (mpctx->loop_times < 0) return FALSE;

	if (mpctx->loop_times == 0) return TRUE;
}

void cmd_loop(void)
{
	if (loop_on()==FALSE)
	{
		put_icommand1(MP_CMD_LOOP,1);
	}
	else
	{
		put_icommand1(MP_CMD_LOOP,-1);
	}
}

static int32 open_subtitles_proc(STRPTR args, int32 length, APTR execbase)
{
	char *FileName;

	if(get_icommand("pause") == 0) // 0 -> not already in pause
		put_command0(MP_CMD_FRAME_STEP); // play 1 frame and pause

	if (FileName = LoadFile(NULL,SUBEXTPAT,CS(MSG_Requester_OpenFile_SubTitles)))
	{
		__tmp_file_name = strdup(FileName);
FreeVec(FileName); FileName = NULL;
DBUG("subtitles file: '%s'\n",__tmp_file_name);
		put_scommand1(MP_CMD_SUB_LOAD,__tmp_file_name);
		put_icommand1(MP_CMD_SUB_SELECT,0);
		put_icommand1(MP_CMD_SUB_VISIBILITY,1);
	}

	if(get_icommand("pause") == 1) // 1 -> already in pause
		put_command0(MP_CMD_PAUSE); // resume pause

	return(RETURN_OK);
}

void cmd_open_subtitles(void)
{
//	struct Process *proc;
DBUG("cmd_open_subtitles()\n",NULL);
if(ASL_Process == NULL)
//	if (choosing_flag == FALSE)
	{
//		proc = spawn( open_subtitles_proc, "Open subtitles" );
//		if (!proc) PrintMsg(errmsg.text, REQTYPE_INFO, REQIMAGE_WARNING, CS(MSG_Requester_Title_Warning));
ASL_Process = spawn( open_subtitles_proc, "Open subtitles", TYPE_ASL );
if (!ASL_Process) PrintMsg(errmsg.text, REQTYPE_INFO, REQIMAGE_WARNING, CS(MSG_Requester_Title_Warning));
	}
	else
		blink_screen() ; // zzd10h
}


void menu_project(ULONG menucode)
{
	// DBUG("Menu number %ld\n", ITEMNUM(menucode));
	switch (ITEMNUM(menucode))
	{
		case ID_open_file:
			cmd_open_file();
			break;
		case ID_open_dvd:
//			cmd_open_dvd();
cmd_open_dev("Open DVD ReqProcess", OpenDVD);
			break;
		case ID_open_dvdnav:
//			cmd_open_dvdnav();
cmd_open_dev("Open DVDNAV ReqProcess", OpenDVDNAV);
			break;
		case ID_open_vcd:
//			cmd_open_vcd();
cmd_open_dev("Open VCD ReqProcess", OpenVCD);
			break;
		case ID_open_network:
//			cmd_open_network();
cmd_open_dev("Open URL ReqProcess", OpenNetwork);
			break;
		case ID_iconify:
			Iconify(Menu_Window);
			break;
		case ID_about:
			ShowAbout();
			break;
		case ID_quit:
//DBUG("spawn_count = %ld\n",spawn_count);
//			if(spawn_count == 0) put_command0(MP_CMD_QUIT);
			put_command0(MP_CMD_QUIT);
			break;
	}
}

void menu_play(ULONG menucode)
{
	switch (ITEMNUM(menucode))
	{
		case ID_play:
			cmd_play();
			break;

		case ID_stop:
			cmd_stop();
			break;


		/*case ID_record:
			cmd_record();
			break;*/


		case ID_loop:
			cmd_loop();
			break;

		case ID_prev:
			prevfile();
			break;

		case ID_next:
			nextfile();
			break;

		case ID_properties:
DBUG("ID_properties\n",NULL);
			FileProperties();
			break;
	}
}

void menu_video(ULONG menucode)
{
	switch (ITEMNUM(menucode))
	{
		case ID_ontop:
			put_command0(MP_CMD_VO_ONTOP);
			break;
		case ID_opensubtitles:
			cmd_open_subtitles();
			break;
		case ID_subtitles:
			put_command0(MP_CMD_SUB_SELECT);
			break;
		case ID_unloadsubtitles:
			put_icommand1(MP_CMD_SUB_VISIBILITY,0);
			put_icommand1(MP_CMD_SUB_REMOVE,-1);
			break;
		case ID_subtitlessize:
			switch (SUBNUM(menucode))
			{
				case ID_subtitlesbig:
					put_fcommand2(MP_CMD_SUB_SCALE,0.50,0);
					break;
				case ID_subtitlessmall:
					put_fcommand2(MP_CMD_SUB_SCALE,-0.50,0);
					break;
			}
			break;
		case ID_fullscreen:
			put_command0(MP_CMD_VO_FULLSCREEN);
			break;
		case ID_screenshot:
			amigaos_screenshot();
//			put_command0(MP_CMD_SCREENSHOT);
			break;
/*
		case 5:
			p96pip_ChangeWindowSize(x1);
			break;
		case 6:
			p96pip_ChangeWindowSize(x2);
			break;
		case 7:
			p96pip_ChangeWindowSize(xF);
			break;
		case 8:
			p96pip_ChangeWindowSize(xH);
			break;
*/
//		case ID_aspectratio:
// DBUG("ID_aspectratio %ld\n",SUBNUM(menucode));
			/*switch (SUBNUM(menucode))
			{
				case 0:
					AspectRatio = AR_ORIGINAL;
					p96pip_ChangeWindowSize(AR_ORIGINAL);
					break;
				case 2:
					AspectRatio = AR_16_10;
					p96pip_ChangeWindowSize(AR_16_10);
				case 3:
					AspectRatio = AR_16_9;
					p96pip_ChangeWindowSize(AR_16_9);
					break;
				case 4:
					AspectRatio = AR_185_1;
					p96pip_ChangeWindowSize(AR_185_1);
					break;
				case 5:
					AspectRatio = AR_221_1;
					p96pip_ChangeWindowSize(AR_221_1);
					break;
				case 6:
					AspectRatio = AR_235_1;
					p96pip_ChangeWindowSize(AR_235_1);
					break;
				case 7:
					AspectRatio = AR_239_1;
					p96pip_ChangeWindowSize(AR_239_1);
					break;
				case 8:
					AspectRatio = AR_5_3;
					p96pip_ChangeWindowSize(AR_5_3);
					break;
				case 9:
					AspectRatio = AR_4_3;
					p96pip_ChangeWindowSize(AR_4_3);
					break;
				case 10:
					AspectRatio = AR_5_4;
					p96pip_ChangeWindowSize(AR_5_4);
					break;
				case 11:
					AspectRatio = AR_1_1;
					p96pip_ChangeWindowSize(AR_1_1);
					break;
			}*/
//			break;

	}
}

void menu_audio(ULONG menucode)
{
	switch (ITEMNUM(menucode))
	{
		case ID_mute:
			mp_input_queue_cmd(mp_input_parse_cmd("mute"));
			is_mute = !is_mute ;
			break;
		case ID_volume_up:
			mplayer_put_key(KEY_VOLUME_UP);
			break;
		case ID_volume_down:
			mplayer_put_key(KEY_VOLUME_DOWN);
			break;
	}
}

void menu_settings(ULONG menucode)
{
	switch (ITEMNUM(menucode))
	{
		case ID_mplayergui:
// DBUG("launching MPlayerGUI [WIP]\n",NULL);
			LaunchCommand(TOOL_MPGUI);
			break;
	}
}


void processMenusClass(uint32 MenuClass_id) // zzd10h
{
	// Object *menu_obj = (Object *)IDoMethod(amiga_menuClass, MM_FINDID, 0, MenuClass_id);

	switch (MenuClass_id)
	{
		case MID_MENU_OPENFILE_MENUCLASS:
			cmd_open_file();
			break;

		case MID_MENU_OPENDVD_MENUCLASS:
//			cmd_open_dvd();
cmd_open_dev("Open DVD ReqProcess", OpenDVD);
//OpenDVD(NULL,0,NULL);
			break;

		case MID_MENU_OPENDVDNAV_MENUCLASS:
//			cmd_open_dvdnav();
cmd_open_dev("Open DVDNAV ReqProcess", OpenDVDNAV);
			break;

		case MID_MENU_OPENVCD_MENUCLASS:
//			cmd_open_vcd();
cmd_open_dev("Open VCD ReqProcess", OpenVCD);
			break;

		case MID_MENU_OPENNETWORK_MENUCLASS:
//			cmd_open_network();
cmd_open_dev("Open URL ReqProcess", OpenNetwork);
			break;

		case MID_MENU_ICONIFY_MENUCLASS:
			Iconify(Menu_Window);
			break;

		case MID_MENU_ABOUT_MENUCLASS:
			ShowAbout();
			break;

		case MID_MENU_QUIT_MENUCLASS:
//DBUG("spawn_count = %ld\n",spawn_count);
//			if(spawn_count == 0) put_command0(MP_CMD_QUIT);
			put_command0(MP_CMD_QUIT);
			break;

		case MID_MENU_PLAYPAUSE_MENUCLASS:
			cmd_play();
			break;

		case MID_MENU_STOP_MENUCLASS:
			cmd_stop();
			break;


case MID_MENU_RECORD_MENUCLASS:
	cmd_record();
	break;


		case MID_MENU_LOOP_MENUCLASS:
			cmd_loop();
			break;

		case MID_MENU_PREVFILE_MENUCLASS:
			prevfile();
			break;

		case MID_MENU_NEXTFILE_MENUCLASS:
			nextfile();
			break;

		case MID_MENU_STAYONTOP_MENUCLASS:
			put_command0(MP_CMD_VO_ONTOP);
			break;

		case MID_MENU_OPENSUBTITLES_MENUCLASS:
// DBUG("MID_MENU_OPENSUBTITLES_MENUCLASS\n",NULL);
			cmd_open_subtitles();
			break;

		case MID_MENU_CYCLESUBTITLES_MENUCLASS:
			put_command0(MP_CMD_SUB_SELECT);
			break;

		case MID_MENU_UNLOADSUBTITLES_MENUCLASS:
DBUG("MID_MENU_UNLOADSUBTITLES_MENUCLASS\n",NULL);
			put_icommand1(MP_CMD_SUB_VISIBILITY,0);
			put_icommand1(MP_CMD_SUB_REMOVE,-1);
			break;

			case MID_MENU_SUBTITLESBIG_MENUCLASS:
//DBUG("MID_MENU_SUBTITLESBIG_MENUCLASS\n",NULL);
				put_fcommand2(MP_CMD_SUB_SCALE,0.50,0);
				break;

			case MID_MENU_SUBTITLESSMALL_MENUCLASS:
//DBUG("MID_MENU_SUBTITLESSMALL_MENUCLASS\n",NULL);
				put_fcommand2(MP_CMD_SUB_SCALE,-0.50,0);
				break;

		case MID_MENU_FULLSCREEN_MENUCLASS:
			put_command0(MP_CMD_VO_FULLSCREEN);
			break;

		case MID_MENU_SCREENSHOT_MENUCLASS:
			amigaos_screenshot();
			break;

case MID_MENU_AR_ORIGINAL_MENUCLASS:
case MID_MENU_AR_16_10_MENUCLASS:
case MID_MENU_AR_16_9_MENUCLASS:
case MID_MENU_AR_185_1_MENUCLASS:
case MID_MENU_AR_221_1_MENUCLASS:
case MID_MENU_AR_235_1_MENUCLASS:
case MID_MENU_AR_239_1_MENUCLASS:
case MID_MENU_AR_5_3_MENUCLASS:
case MID_MENU_AR_4_3_MENUCLASS:
case MID_MENU_AR_5_4_MENUCLASS:
// case MID_MENU_AR_1_1_MENUCLASS:
DBUG("MID_MENU_AR_#?_MENUCLASS = %ld\n", MenuClass_id - MID_MENU_AR_ORIGINAL_MENUCLASS);
break;

		case MID_MENU_MUTE_MENUCLASS:
			mp_input_queue_cmd(mp_input_parse_cmd("mute"));
			is_mute = !is_mute;
			break;

		case MID_MENU_VOLUMEUP_MENUCLASS:
			mplayer_put_key(KEY_VOLUME_UP);
			break;

		case MID_MENU_VOLUMEDOWN_MENUCLASS:
			mplayer_put_key(KEY_VOLUME_DOWN);
			break;

		case MID_MENU_PROPERTIES_MENUCLASS:
DBUG("MID_MENU_PROPERTIES_MENUCLASS\n",NULL);
			FileProperties();
			break;

		case MID_MENU_MPLAYERGUI_MENUCLASS:
// DBUG("MID_MENU_MPLAYERGUI_MENUCLASS\n",NULL);
			LaunchCommand(TOOL_MPGUI);
			break;
	}
}

void menu_events( struct IntuiMessage *IntuiMsg)
{
	if (!MENUCLASS_SUPPORT)
	{
		ULONG menucode = IntuiMsg->Code;

//		if (menucode != MENUNULL)
//		{
			while (menucode != MENUNULL)
			{
				struct MenuItem  *item = ItemAddress(menu,menucode);
// DBUG("MENUNUM = %ld\n",MENUNUM(menucode));
// DBUG("ITEMNUM = %ld\n",ITEMNUM(menucode));
				select_menu[ MENUNUM(menucode) ] ( menucode );

				/* Handle multiple selection */
				menucode = item->NextSelect;
			}
//		}
	}
	else
	{
		uint32 MenuClass_id = NO_MENU_ID;

		while ((MenuClass_id = IDoMethod(amiga_menuClass,MM_NEXTSELECT,0,MenuClass_id)) != NO_MENU_ID)
		{
			processMenusClass(MenuClass_id);

			/*if (loop_on()==TRUE)
				SetAttrs(menuobj_MID_MENU_LOOP_MENUCLASS,MA_Selected,TRUE,TAG_END);
			else
				SetAttrs(menuobj_MID_MENU_LOOP_MENUCLASS,MA_Selected,FALSE,TAG_END) ;

			if (is_ontop==1)
				SetAttrs(menuobj_MID_MENU_STAYONTOP_MENUCLASS,MA_Selected,TRUE,TAG_END);
			else
				SetAttrs(menuobj_MID_MENU_STAYONTOP_MENUCLASS,MA_Selected,FALSE,TAG_END);*/
		}
	}
}


void open_AboutInfowindow(struct pmpMessage *pmp)
{
	Object *win_About;
	struct Hook BackFillHook = { {NULL, NULL}, (HOOKFUNC)BackFillFunc, NULL, NULL };
	char license_path[512];
	struct Screen *scr = Menu_Window? Menu_Window->WScreen : NULL;

	// Banner loading
	struct Screen *screen;
	struct RastPort rp;
	struct BitMap *bm_About = NULL;
	Object *AboutLogo = NULL;
	APTR linfo = NULL;
	APTR dtobj = NULL;
	APTR layer = NULL;
	APTR h;
	struct TagItem tags[4];
	struct InfoWindowTab tabs[] = {
		{ CS(MSG_About_About), pmp->text, TABDATA_STRING },
		{ CS(MSG_About_Licence), license_path, TABDATA_FILE },
		{ NULL, NULL, TABDATA_END }
	};

	//choosing(TRUE);		// Not selecting file, but need to know if a window is open

	strcpy(license_path,"PROGDIR:Docs/LICENSE");

	screen = FrontMostScr();
	//screen = LockPubScreen(NULL);
	if(screen)
	{
		InitRastPort( &rp );

//		UnlockPubScreen(NULL,screen);

		// Fix AllocBitmap to work on AmigaOS 4.1.6
		tags[0].ti_Tag  = BMATags_Friend;
		tags[0].ti_Data = (uint32)screen->RastPort.BitMap;
		tags[1].ti_Tag  = BMATags_Displayable;
		tags[1].ti_Data = TRUE;
		tags[2].ti_Tag  = BMATags_PixelFormat;
		tags[2].ti_Data = PIXF_A8R8G8B8;
		tags[3].ti_Tag  = TAG_DONE;

		bm_About=AllocBitMap(BANNER_WIDTH, BANNER_HEIGHT, 32, BMF_CHECKVALUE, (struct BitMap *)tags);
		if ( bm_About == NULL )
			goto bailout;

		rp.BitMap  = bm_About;

		linfo = NewLayerInfo();
		if ( linfo == NULL )
			goto bailout;

		layer = CreateLayer(linfo,
		                    LAYA_BitMap, bm_About,
		                    LAYA_MinX,   0,
		                    LAYA_MinY,   0,
		                    LAYA_MaxX,   BANNER_WIDTH - 1,
		                    LAYA_MaxY,   BANNER_HEIGHT - 1,
		                   TAG_END);
		if ( layer == NULL )
			goto bailout;

		rp.Layer = layer;

		dtobj = NewDTObject(NULL,
		                    DTA_SourceType,     DTST_MEMORY,
		                    DTA_SourceAddress,  mplayer_banner,
		                    DTA_SourceSize,     sizeof(mplayer_banner),
		                    DTA_GroupID,        GID_PICTURE,
		                    PDTA_DestMode,      PMODE_V43,
		                   TAG_END);
		if ( dtobj == NULL )
			goto bailout;

		h = ObtainDTDrawInfo( dtobj, PDTA_Screen,screen, TAG_END);
		if ( h == NULL )
			goto bailout;

		DrawDTObject(&rp,
		             dtobj,
		             0,
		             0,
		             BANNER_WIDTH,
		             BANNER_HEIGHT,
		             0,
		             0,
		            TAG_END);

		ReleaseDTDrawInfo( dtobj, h );

		AboutLogo = NewObject(BitMapClass, NULL, // "bitmap.image",
		                      BITMAP_BitMap,  bm_About,
		                      BITMAP_Screen,  screen,
		                      BITMAP_Masking, TRUE,
		                      BITMAP_Width,   BANNER_WIDTH,
		                      BITMAP_Height,  BANNER_HEIGHT,
		                     TAG_DONE);
// DBUG("AboutLogo = 0x%08lx\n",AboutLogo);
	}

	if(!AboutLogo) BANNER_DISPLAY = FALSE;
DBUG("FrontMostScr()=0x%08lx  Menu_Window-Screen=0x%08lx\n",screen,scr);
	if ( (win_About = NewObject(InfoWindowClass, NULL,
	                   WA_Title,       CS(MSG_Requester_Title_About), // title,
	                   WA_ScreenTitle, AMIGA_VERSION,
	                   WA_PubScreen,   screen, // FrontMostScr(),
	                   WA_CloseGadget, TRUE ,
	                   WA_Height,      450,
	                   WA_Width,       375,
	                   WA_SizeGadget,  TRUE,
	                   WA_StayTop,     is_fullscreen? TRUE : FALSE,
	                   INFOWINDOW_BodyText, "\n"AMIGA_VERSION_ABOUT,
	                   INFOWINDOW_GadgetText, CS(MSG_Requester_OK),
	                   BANNER_DISPLAY? INFOWINDOW_Image         : TAG_IGNORE, AboutLogo,
	                   BANNER_DISPLAY? INFOWINDOW_ImagePlace    : TAG_IGNORE, PLACEIMAGE_ABOVE,
	                   BANNER_DISPLAY? INFOWINDOW_ImageBackFill : TAG_IGNORE, &BackFillHook,
	                   INFOWINDOW_TabPanel, tabs,
	                   INFOWINDOW_AllowSignals, TRUE, // allow sending a break signal to the infowindow process.
	                  TAG_DONE)) )
	{
		IDoMethod(win_About, IWM_OPEN, NULL);
		DisposeObject(win_About);
	}
DBUG("freeing About window resources\n");
bailout:

	if ( dtobj )
		DisposeDTObject( dtobj );

	if ( AboutLogo )
		DisposeObject( AboutLogo );

	if ( layer )
		DeleteLayer( 0, layer );

	if ( linfo )
		DisposeLayerInfo( linfo );

	if ( bm_About )
		FreeBitMap( bm_About );

	//choosing(FALSE);
}

void ShowAbout(void)
{
	struct pmpMessage *pmp;

	//if (choosing_flag == TRUE)
	if(About_Process)
	{
		blink_screen() ; // zzd10h
		return;
	}

	pmp = AllocVecTags(sizeof(struct pmpMessage), AVT_Type,MEMF_SHARED, TAG_DONE);
	if(!pmp) return;

	snprintf(pmp->text, sizeof(pmp->text), "\n\033h%s\n %s\n" \
"\n\033h%s\n %s\n" \
"\n\033h%s\n %s\n" \
"\n\033h%s\n %s\n" \
"\n\033h%s\n %ld.%ld.%ld\n" \
"\n\033h%s\n %s\n" \
"\n\033h%s\n %s\n" \
"\n\033[s:10]\n\033c%s %s\n\033chttp://www.mplayerhq.hu\n\033[s:10]" \
"\n\n\033h%s%s",
		CS(MSG_About_AmigaVersion),AMIGA_VERSION, \
		CS(MSG_About_Built),VERSION, \
		CS(MSG_About_VideoDriver),mpctx->video_out->info->short_name, \
		CS(MSG_About_FFmpegVersion),FFMPEG_VERSION, \
		CS(MSG_About_GCCVersion),GCC_VERSION,GCC_REVISION,GCC_PATCHLVL, \
		CS(MSG_About_ARexxPort),ARexxPortName, \
		CS(MSG_About_Translation),CS(MSG_About_Translator), \
		CS(MSG_About_Copyright),CS(MSG_About_MPlayer_Team), \
		CS(MSG_About_AmigaOS4Version), \
		"\n Andrea Palmatè - https://www.amigasoft.net" \
		"\n Kjetil Hvalstrand - https://github.com/khval" \
		"\n Guillaume 'zzd10h' Boesel" \
		"\n Michael Trebilcock" \
		"\n AOS4 fans from Mars");

	if(!InfoWindowBase) snprintf(pmp->text, sizeof(pmp->text), "\033c"AMIGA_VERSION_ABOUT"\n\n" \
"\n\033b%s\033n\n%s\n" \
"\n\033b%s \033n%s\n" \
"\n\033b%s \033n%s\n" \
"\033b%s \033n%ld.%ld.%ld\n" \
"\033b%s \033n%s\n" \
"\033b%s \033n%s\n" \
"\n\%s %s\nhttp://www.mplayerhq.hu\n" \
"\n\033b%s\033n%s", \
		CS(MSG_About_Built),VERSION, \
		CS(MSG_About_VideoDriver),mpctx->video_out->info->short_name, \
		CS(MSG_About_FFmpegVersion),FFMPEG_VERSION, \
		CS(MSG_About_GCCVersion),GCC_VERSION,GCC_REVISION,GCC_PATCHLVL, \
		CS(MSG_About_ARexxPort),ARexxPortName, \
		CS(MSG_About_Translation),CS(MSG_About_Translator), \
		CS(MSG_About_Copyright),CS(MSG_About_MPlayer_Team), \
		CS(MSG_About_AmigaOS4Version), \
		"\nAndrea Palmatè - https://www.amigasoft.net" \
		"\nKjetil Hvalstrand - https://github.com/khval" \
		"\nGuillaume 'zzd10h' Boesel" \
		"\nMichael Trebilcock" \
		"\nAOS4 fans from Mars");

	// PrintMsg(message_about,REQTYPE_INFO,REQIMAGE_INFO);
	pmp->type = REQTYPE_INFO;
	pmp->image = REQIMAGE_INFO;
DBUG("ShowAbout() pmp=%p\n",pmp);
	/*About_Process = (struct Process *) CreateNewProcTags (
	                                  NP_Entry,     (ULONG) PrintMsgProc,
	                                  NP_Name,      "About MPlayer",
	                                  NP_StackSize, 65535, // 262144,
	                                  NP_Child,     TRUE,
	                                  NP_Priority,  0,
	                                  NP_EntryData, pmp,
	                                  NP_ExitData,  IDOS,
	                                  NP_FinalCode, spawn_died,
	                                  NP_FinalData, TYPE_ABOUT,
	                                 TAG_DONE);
//DBUG("About process = 0x%08lx\n",About_Process);
	if (About_Process) spawn_count++;
	else*/
	{
		if(InfoWindowBase) open_AboutInfowindow(pmp) ;
		else PrintMsg(pmp->text, REQTYPE_INFO, REQIMAGE_INFO, CS(MSG_Requester_Title_About));

		FreeVec(pmp);
	}
//DBUG("  spawn_count=%ld\n",spawn_count);
}

static int32 OpenNetwork(STRPTR args UNUSED, int32 length UNUSED, struct ExecBase *execbase UNUSED)
{
	Object *netwobj;
	UBYTE buffer[513]="http://";
	ULONG result;
	struct Screen *scr = FrontMostScr(); // Menu_Window? Menu_Window->WScreen : NULL;

	if(get_icommand("pause") == 0) // 0 -> not already in pause
		put_command0(MP_CMD_PAUSE); // pause
		//put_command0(MP_CMD_FRAME_STEP); // play 1 frame and pause

	netwobj = (Object *) NewObject(RequesterClass, NULL, // "requester.class",
				REQ_Type,         REQTYPE_STRING,
				REQ_TitleText,    CS(MSG_Requester_Network_Title),
				REQ_BodyText,     CS(MSG_Requester_Network_Body),
				REQ_GadgetText,   CS(MSG_Requester_Network_Gadget),
				//REQS_Invisible,   FALSE,
				REQS_Buffer,      buffer,
				REQS_ShowDefault, TRUE,
				REQS_MaxChars,    512,
			TAG_DONE);
	if ( netwobj )
	{
//		choosing(TRUE);
//Req_Process->pr_Task.tc_UserData = (APTR)netwobj; // needed on amigaos_stuff.c::closeRemainingOpenWin()
DBUG("OpenNetwork() %p\n",netwobj);
		result = IDoMethod( netwobj, RM_OPENREQ, NULL, NULL, scr );
//		choosing(FALSE);

		/*if (result != 0 && (strcmp(buffer,"http://")!=0))
		{
			PlayFile_async(buffer);
		}*/
		if(result==1)//  &&  Req_Process->pr_Task.tc_UserData!=NULL)
		{
			if(strncmp(buffer,"http://",7) == 0)
			{
				PlayFile_async(buffer);
				printf("[%s]Buffer:%s\n",__FUNCTION__,buffer);
			}
			//else { PrintMsg(CS(MSG_Requester_DVD_Error), REQTYPE_INFO, REQIMAGE_ERROR, CS(MSG_Requester_Title_Error)); }
		}

		DisposeObject( netwobj );
	}

	put_command0(MP_CMD_PAUSE); // resume pause
DBUG("OpenNetwork() - END '%s'\n",buffer);
	return(RETURN_OK);
}


static int32 OpenDVD(STRPTR args UNUSED, int32 length UNUSED, struct ExecBase *execbase UNUSED)
{
	Object *dvdobj;
	UBYTE buffer[256]="dvd://1";
	ULONG result;
	struct Screen *scr = FrontMostScr(); // Menu_Window? Menu_Window->WScreen : NULL;

	if(get_icommand("pause") == 0) // 0 -> not already in pause
		put_command0(MP_CMD_PAUSE); // pause
		//put_command0(MP_CMD_FRAME_STEP); // play 1 frame and pause

	dvdobj = (Object *) NewObject(RequesterClass, NULL, // "requester.class",
				REQ_Type,         REQTYPE_STRING,
				REQ_TitleText,    CS(MSG_Requester_DVD_Title),
				REQ_BodyText,     CS(MSG_Requester_DVD_Body),
				REQ_GadgetText,   CS(MSG_Requester_DVD_Gadget),
				//REQS_Invisible,   FALSE,
				REQS_Buffer,      buffer,
				REQS_ShowDefault, TRUE,
				REQS_MaxChars,    255,
			TAG_DONE);
	if ( dvdobj )
	{
//		choosing(TRUE);
//Req_Process->pr_Task.tc_UserData = (APTR)dvdobj; // needed on amigaos_stuff.c::closeRemainingOpenWin()
DBUG("OpenDVD() %p\n",dvdobj);
		result = IDoMethod( dvdobj, RM_OPENREQ, NULL, NULL, scr );
//		choosing(FALSE);
//DBUG("result = %ld\n",result);
		/*if (result != 0 && (strncmp(buffer,"dvd://",6)==0))
		{
			PlayFile_async(buffer);
			printf("[%s]Buffer:%s\n",__FUNCTION__,buffer);
		}
		else
		{
			if (strncmp(buffer,"dvd://",6)!=0) PrintMsg(CS(MSG_Requester_DVD_Error), REQTYPE_INFO, REQIMAGE_ERROR, CS(MSG_Requester_Title_Error));
		}*/
		if(result==1  )//&&  Req_Process->pr_Task.tc_UserData!=NULL)
		{
			if(strncmp(buffer,"dvd://",6) == 0)
			{
				PlayFile_async(buffer);
				printf("[%s]Buffer:%s\n",__FUNCTION__,buffer);
			}
			else { PrintMsg(CS(MSG_Requester_DVD_Error), REQTYPE_INFO, REQIMAGE_ERROR, CS(MSG_Requester_Title_Error)); }
		}

		DisposeObject( dvdobj );
	}

	put_command0(MP_CMD_PAUSE); // resume pause
DBUG("OpenDVD() - END '%s'\n",buffer);
	return(RETURN_OK);
}

static int32 OpenDVDNAV(STRPTR args UNUSED, int32 length UNUSED, struct ExecBase *execbase UNUSED)
{
	Object *dvdobj;
	UBYTE buffer[256]="dvdnav://";
	ULONG result;
	struct Screen *scr = FrontMostScr(); // Menu_Window? Menu_Window->WScreen : NULL;

	if (stream_cache_size>0)
	{
		PrintMsg(CS(MSG_Requester_DVDNAV_Error), REQTYPE_INFO, REQIMAGE_ERROR, CS(MSG_Requester_Title_Error));
		return;
	}

	if(get_icommand("pause") == 0) // 0 -> not already in pause
		put_command0(MP_CMD_PAUSE); // pause
		//put_command0(MP_CMD_FRAME_STEP); // play 1 frame and pause

	dvdobj = (Object *) NewObject(RequesterClass, NULL, // "requester.class",
				REQ_Type,         REQTYPE_STRING,
				REQ_TitleText,    CS(MSG_Requester_DVDNAV_Title),
				REQ_BodyText,     CS(MSG_Requester_DVD_Body),
				REQ_GadgetText,   CS(MSG_Requester_DVD_Gadget),
				//REQS_Invisible,   FALSE,
				REQS_Buffer,      buffer,
				REQS_ShowDefault, TRUE,
				REQS_MaxChars,    255,
			TAG_DONE);
	if ( dvdobj )
	{
//		choosing(TRUE);
//Req_Process->pr_Task.tc_UserData = (APTR)dvdobj; // needed on amigaos_stuff.c::closeRemainingOpenWin()
DBUG("OpenDVDNAV() %p\n",dvdobj);
		result = IDoMethod( dvdobj, RM_OPENREQ, NULL, NULL, scr );
//		choosing(FALSE);

		/*if (result != 0 && (strncmp(buffer,"dvdnav://",9)==0))
		{
			PlayFile_async(buffer);
			printf("[%s]Buffer:%s\n",__FUNCTION__,buffer);
		}
		else
		{
			if (strncmp(buffer,"dvdnav://",9)!=0) PrintMsg(CS(MSG_Requester_DVD_Error), REQTYPE_INFO, REQIMAGE_ERROR, CS(MSG_Requester_Title_Error));
		}*/
		if(result==1)//  &&  Req_Process->pr_Task.tc_UserData!=NULL)
		{
			if(strncmp(buffer,"dvdnav://",9) == 0)
			{
				PlayFile_async(buffer);
				printf("[%s]Buffer:%s\n",__FUNCTION__,buffer);
			}
			else { PrintMsg(CS(MSG_Requester_DVD_Error), REQTYPE_INFO, REQIMAGE_ERROR, CS(MSG_Requester_Title_Error)); }
		}

		DisposeObject( dvdobj );
	}

	put_command0(MP_CMD_PAUSE); // resume pause
DBUG("OpenDVDNAV() - END '%s'\n",buffer);
	return(RETURN_OK);
}


static int32 OpenVCD(STRPTR args UNUSED, int32 length UNUSED, struct ExecBase *execbase UNUSED)
{
	Object *vcdobj;
	UBYTE buffer[256]="vcd://1";
	ULONG result;
	struct Screen *scr = FrontMostScr(); // Menu_Window? Menu_Window->WScreen : NULL;

	if(get_icommand("pause") == 0) // 0 -> not already in pause
		put_command0(MP_CMD_PAUSE); // pause
		//put_command0(MP_CMD_FRAME_STEP); // play 1 frame and pause

	vcdobj = (Object *) NewObject(RequesterClass, NULL, // "requester.class",
				REQ_Type,         REQTYPE_STRING,
				REQ_TitleText,    CS(MSG_Requester_VCD_Title),
				REQ_BodyText,     CS(MSG_Requester_VCD_Body),
				REQ_GadgetText,   CS(MSG_Requester_VCD_Gadget),
				//REQS_Invisible,   FALSE,
				REQS_Buffer,      buffer,
				REQS_ShowDefault, TRUE,
				REQS_MaxChars,    255,
			TAG_DONE);
	if ( vcdobj )
	{
//		choosing(TRUE);
//Req_Process->pr_Task.tc_UserData = (APTR)vcdobj; // needed on amigaos_stuff.c::closeRemainingOpenWin()
DBUG("OpenVCD() %p\n",vcdobj);
		result = IDoMethod( vcdobj, RM_OPENREQ, NULL, NULL, scr );
//		choosing(FALSE);

		/*if (result != 0 && (strncmp(buffer,"vcd://",6)==0))
		{
			PlayFile_async(buffer);
		}
		else
		{
			if (strncmp(buffer,"vcd://",6)!=0) PrintMsg(CS(MSG_Requester_VCD_Error), REQTYPE_INFO, REQIMAGE_ERROR, CS(MSG_Requester_Title_Error));
		}*/
		if(result==1)//  &&  Req_Process->pr_Task.tc_UserData!=NULL)
		{
			if(strncmp(buffer,"vcd://",6) == 0)
			{
				PlayFile_async(buffer);
				printf("[%s]Buffer:%s\n",__FUNCTION__,buffer);
			}
			else { PrintMsg(CS(MSG_Requester_VCD_Error), REQTYPE_INFO, REQIMAGE_ERROR, CS(MSG_Requester_Title_Error)); }
		}
		DisposeObject( vcdobj );
	}

	put_command0(MP_CMD_PAUSE); // resume pause
DBUG("OpenVCD() - END '%s'\n",buffer);
	return(RETURN_OK);
}

static UBYTE *LoadFile(const char *StartingDir, CONST_STRPTR ASL_Pattern, CONST_STRPTR ASL_WindowTitle)
{
	UBYTE *filename = NULL;

	struct FileRequester * AmigaOS_FileRequester = NULL;
	BPTR FavoritePath_File;
	char FavoritePath_Value[1024];
	BOOL FavoritePath_Ok = FALSE;
	struct Screen *scr = FrontMostScr(); // Menu_Window? Menu_Window->WScreen : NULL;

	int len = 0;

	if (StartingDir==NULL)
	{
		FavoritePath_File = Open("PROGDIR:FavoritePath", MODE_OLDFILE);
		if (FavoritePath_File)
		{
			LONG size = Read(FavoritePath_File, FavoritePath_Value, 1024);
			if (size > 0)
			{
				if ( strchr(FavoritePath_Value, '\n') ) // There is an /n -> valid file
				{
					FavoritePath_Ok = TRUE;
					*(strchr(FavoritePath_Value, '\n')) = '\0';
				}
			}
			Close(FavoritePath_File);
		}
	}
	else
	{
		FavoritePath_Ok = TRUE;
		strcpy(FavoritePath_Value, StartingDir);
	}

	AmigaOS_FileRequester = AllocAslRequest(ASL_FileRequest, NULL);
	if (!AmigaOS_FileRequester)
	{
		mp_msg(MSGT_CPLAYER, MSGL_FATAL,"Cannot open the FileRequester!\n");
DBUG("LoadFile() - ERROR\n",NULL);
		return NULL;
	}

//	choosing(TRUE);
DBUG("AOS filereq = 0x%08lx (fs=%ld)\n",AmigaOS_FileRequester, is_fullscreen);
DBUG("new process=0x%08lx (ASL)  task=0x%08lx\n",ASL_Process,ASL_Process->pr_Task);
ASL_Process->pr_Task.tc_UserData = (APTR)AmigaOS_FileRequester; // needed on amigaos_stuff.c::closeRemainingOpenWin()
	if ( ( AslRequestTags( AmigaOS_FileRequester,
				ASLFR_Screen,         scr,
				ASLFR_TitleText,      ASL_WindowTitle,
				ASLFR_DoMultiSelect,  FALSE, // Maybe in the future we can implement a playlist...
				ASLFR_RejectIcons,    TRUE,
				ASLFR_DoPatterns,     TRUE,
//				is_fullscreen? ASLFR_StayOnTop : TAG_IGNORE , TRUE,
				ASLFR_InitialPattern, ASL_Pattern, // EXTPATTERN,
				ASLFR_InitialDrawer,  (FavoritePath_Ok) ? FavoritePath_Value : "",
			TAG_DONE) ) == FALSE )
	{
		FreeAslRequest(AmigaOS_FileRequester);
		AmigaOS_FileRequester = NULL;
//		choosing(FALSE);
DBUG("LoadFile() - END\n",NULL);
		return NULL;
	}
//	choosing(FALSE);

	if (AmigaOS_FileRequester->fr_NumArgs>0);
	{
		len = strlen(AmigaOS_FileRequester->fr_Drawer) + strlen(AmigaOS_FileRequester->fr_File) + 1;
		//if ((filename = AllocMem(len,MEMF_SHARED|MEMF_CLEAR)))
		if ((filename = AllocVecTags(len, AVT_Type,MEMF_SHARED, AVT_ClearWithValue,0, TAG_DONE)))
		{
			strcpy(filename,AmigaOS_FileRequester->fr_Drawer);
			AddPart(filename,AmigaOS_FileRequester->fr_File,len + 1);
		}
	}

	FreeAslRequest(AmigaOS_FileRequester);
DBUG("LoadFile() - END '%s'\n",filename);
	return filename;
}

static int32 PrintMsgProc(STRPTR args UNUSED, int32 length UNUSED, struct ExecBase *execbase)
{
	struct ExecIFace *IExec = (struct ExecIFace *) execbase->MainInterface;
	struct Process *me = (struct Process *) FindTask(NULL);
	struct DOSIFace *IDOS = (struct DOSIFace *) me->pr_ExitData;
	struct pmpMessage *pmp = (struct pmpMessage *) GetEntryData();
DBUG("PrintMsgProc() pmp=%p\n",pmp);
	if(pmp)
	{
		if(InfoWindowBase) open_AboutInfowindow(pmp);
		else PrintMsg(pmp->text, pmp->type, pmp->image, CS(MSG_Requester_Title_About));

		FreeVec(pmp);
		return RETURN_OK;
	}

	return RETURN_FAIL;
}

void PrintMsg(CONST_STRPTR text, int REQ_TYPE, int REQ_IMAGE, CONST_STRPTR title)
{
	Object *reqobj;
	struct Screen *reqscr = FrontMostScr();//Menu_Window? Menu_Window -> WScreen : NULL;

	if (REQ_TYPE == 0) REQ_TYPE = REQTYPE_INFO;
	if (REQ_IMAGE == 0) REQ_IMAGE = REQIMAGE_DEFAULT;

	reqobj = (Object *) NewObject(RequesterClass, NULL, // "requester.class",
	                     REQ_Type,        REQ_TYPE,
	                     REQ_TitleText,   title,
	                     REQ_BodyText,    text,
	                     REQ_Image,       REQ_IMAGE,
	                     REQ_TimeOutSecs, 15,
	                     REQ_GadgetText,  CS(MSG_Requester_OK),
	                    TAG_END);
	if (reqobj)
	{
		choosing(TRUE);	// Not selecting file, but need to know if a window is open

		IDoMethod( reqobj, RM_OPENREQ, NULL, NULL, reqscr );

		choosing(FALSE);

		DisposeObject( reqobj );
	}
}

void add_file_to_que(char *FileName)
{
	mp_cmd_t MPCmd;
	mp_cmd_t *c;

// DBUG("MP_CMD_LOADFILE = %d\n",MP_CMD_LOADFILE);

	bzero(&MPCmd,sizeof(MPCmd));

	MPCmd.id			= MP_CMD_LOADFILE;
	strcpy(MPCmd.name, "");
	MPCmd.nargs			= 2;
	MPCmd.pausing		= FALSE;
	MPCmd.args[0].type	= MP_CMD_ARG_STRING;
	MPCmd.args[0].v.s		= (char*)FileName;
	MPCmd.args[1].type	= MP_CMD_ARG_INT;
	MPCmd.args[1].v.i		= TRUE;
	MPCmd.args[2]		= mp_cmds[MP_CMD_LOADFILE].args[2];

	if (( c = mp_cmd_clone(&MPCmd)))
	{
		if ((mp_input_queue_cmd(c)==0))
		{
			mp_cmd_free(c);
		}
	}
	else
	{
		printf("[%s]Failed to execute\n",__FUNCTION__);
	}
}

void lastsong(void)
{
	mp_cmd_t MPCmd;
	mp_cmd_t *c;

// DBUG("MP_CMD_PLAY_TREE_UP_STEP = %d\n",MP_CMD_PLAY_TREE_UP_STEP);

	bzero(&MPCmd,sizeof(MPCmd));

	MPCmd.id			= MP_CMD_PLAY_TREE_UP_STEP;
	strcpy(MPCmd.name, "");
	MPCmd.nargs			= 2;
	MPCmd.pausing		= FALSE;
	MPCmd.args[0].type	= MP_CMD_ARG_INT;
	MPCmd.args[0].v.i		= 1;
	MPCmd.args[1].type	= MP_CMD_ARG_INT;
	MPCmd.args[1].v.i		= TRUE;
	MPCmd.args[2]		= mp_cmds[MP_CMD_PLAY_TREE_STEP].args[2];

	if ((c = mp_cmd_clone(&MPCmd)))
	{
		if ((mp_input_queue_cmd(c)==0))
		{
			mp_cmd_free(c);
		}
		else
		{
			printf("[%s]Failed to queue\n",__FUNCTION__);
		}
	}
	else
	{
		printf("[%s]Failed to execute\n",__FUNCTION__);
	}
}

void prevfile(void)
{
	mp_cmd_t MPCmd;
	mp_cmd_t *c;

// DBUG("MP_CMD_PLAY_TREE_STEP = %d\n",MP_CMD_PLAY_TREE_STEP);

	bzero(&MPCmd,sizeof(MPCmd));

	MPCmd.id			= MP_CMD_PLAY_TREE_STEP;
	strcpy(MPCmd.name, "");
	MPCmd.nargs			= 2;
	MPCmd.pausing		= FALSE;
	MPCmd.args[0].type	= MP_CMD_ARG_INT;
	MPCmd.args[0].v.i		= -1;
	MPCmd.args[1].type	= MP_CMD_ARG_INT;
	MPCmd.args[1].v.i		= TRUE;
	MPCmd.args[2]		= mp_cmds[MP_CMD_PLAY_TREE_STEP].args[2];

	if ((c = mp_cmd_clone(&MPCmd)))
	{
		if ((mp_input_queue_cmd(c)==0))
		{
			mp_cmd_free(c);
		}
	}
	else
	{
		printf("[%s]Failed to execute\n",__FUNCTION__);
	}
}

void nextfile(void)
{
	mp_cmd_t MPCmd;
	mp_cmd_t *c;

// DBUG("MP_CMD_PLAY_TREE_STEP = %d\n",MP_CMD_PLAY_TREE_STEP);

	bzero(&MPCmd,sizeof(MPCmd));

	MPCmd.id			= MP_CMD_PLAY_TREE_STEP;
	strcpy(MPCmd.name, "");
	MPCmd.nargs			= 2;
	MPCmd.pausing		= FALSE;
	MPCmd.args[0].type	= MP_CMD_ARG_INT;
	MPCmd.args[0].v.i		= 1;
	MPCmd.args[1].type	= MP_CMD_ARG_INT;
	MPCmd.args[1].v.i		= TRUE;
	MPCmd.args[2]		= mp_cmds[MP_CMD_PLAY_TREE_STEP].args[2];

	if ((c = mp_cmd_clone(&MPCmd)))
	{
		if ((mp_input_queue_cmd(c)==0))
		{
			mp_cmd_free(c);
		}
	}
	else
	{
		printf("[%s]Failed to execute\n",__FUNCTION__);
	}
}


/****************************************/
/* Backfill function for banner borders */
static void BackFillFunc(struct Hook *hook UNUSED, struct RastPort *rp, struct BackFillMessage *msg)
{
	struct RastPort newrp;

	// Remove any clipping
	newrp = *rp;
	newrp.Layer = NULL;

	RectFillColor(&newrp, msg->Bounds.MinX, msg->Bounds.MinY,
	              msg->Bounds.MaxX, msg->Bounds.MaxY, 0xff506EAA); // #506EAA color
}
/* Backfill function for banner borders */
/****************************************/

struct Screen *FrontMostScr(void)
{
    struct Screen *front_screen_address, *public_screen_address = NULL;
    ULONG intuition_lock;
    struct List *public_screen_list;
    struct PubScreenNode *public_screen_node;

    intuition_lock = LockIBase(0L);
    front_screen_address = ((struct IntuitionBase *)IntuitionBase)->FirstScreen;
    if( (front_screen_address->Flags & PUBLICSCREEN) || (front_screen_address->Flags & WBENCHSCREEN) )
    {
        UnlockIBase(intuition_lock);
        public_screen_list = LockPubScreenList();
        public_screen_node = (struct PubScreenNode*)public_screen_list->lh_Head;
        while(public_screen_node)
        {
            if(public_screen_node->psn_Screen == front_screen_address)
            {
                public_screen_address = public_screen_node->psn_Screen;
                break;
            }

            public_screen_node = (struct PubScreenNode*)public_screen_node->psn_Node.ln_Succ;
        }

        UnlockPubScreenList();
    }
    else
    {
        UnlockIBase(intuition_lock);
    }

    if(!public_screen_address)
    {
DBUG("No public screen, using WB screen\n",NULL);
        public_screen_address = LockPubScreen(NULL);
        UnlockPubScreen(NULL, public_screen_address);
    }
// DBUG("0x%08lx\n", (int)public_screen_address);
    return(public_screen_address);
}


BOOL LaunchCommand(char *command)
{
DBUG("LaunchCommand() '%s'\n",command);
	return OpenWorkbenchObject(command, TAG_DONE);
}

/* Parts from mmplayer sources mplayer.c */
void open_InfoWindow_Properties(STRPTR vid_str, STRPTR aud_str, STRPTR clip_str)
{
	Object *win_Prop;
	struct Screen *scr = Menu_Window? Menu_Window->WScreen : NULL;
	struct InfoWindowTab tabs[] = {
		{ CS(MSG_Properties_IW_Video), vid_str, TABDATA_STRING },
		{ CS(MSG_Properties_IW_Audio), aud_str, TABDATA_STRING },
		{ CS(MSG_Properties_IW_Clip), clip_str, TABDATA_STRING },
		{ NULL, NULL, TABDATA_END }
	};
// DBUG("\n[V] %s\n\n[A] %s\n\n%s\n",vid_str,aud_str,clip_str);
DBUG("FrontMostScr()=0x%08lx   Menu_Window->WScreen=0x%08lx\n",FrontMostScr(),scr);

	win_Prop = NewObject(InfoWindowClass, NULL,
	                    WA_Title,       CS(MSG_Properties_Title),
	                    WA_ScreenTitle, AMIGA_VERSION,
	                    WA_PubScreen,   scr, //FrontMostScr(),
	                    WA_CloseGadget, TRUE,
	                    WA_Height,      250,
	                    WA_Width,       250,
	                    //WA_MinWidth,    250,
	                    WA_SizeGadget,  TRUE,
	                    WA_StayTop,     is_fullscreen? TRUE : FALSE,
	                    // INFOWINDOW_BodyText,   " \n", // AMIGA_VERSION_ABOUT,
	                    INFOWINDOW_GadgetText, CS(MSG_Requester_OK),
	                    INFOWINDOW_TabPanel,   tabs,
	                    // INFOWINDOW_TimeOut,    15,
	                    // INFOWINDOW_AllowSignals, TRUE, // allow sending a break signal to the infowindow process
	                   TAG_DONE);

	IDoMethod(win_Prop, IWM_OPEN, NULL);
	DisposeObject(win_Prop);
}

void FileProperties(void)
{
	char vid_str[128]="-", aud_str[128]="-", clip_str[256]="-", tmp[640], buf[5];
	sh_video_t * const sh_video = mpctx->sh_video;
	sh_audio_t * const sh_audio = mpctx->sh_audio;

	// DBUG("ID_DEMUXER = %s\n", mpctx->demuxer->desc->name);

// VIDEO
	if(sh_video) {
		// Assume FOURCC if all bytes >= 0x20 (' ')
		if(sh_video->format >= 0x20202020) {
			SNPrintf(buf, sizeof(buf), "%.4s",(char *)&sh_video->format);
			tmp[0] = buf[3]; tmp[1] = buf[2]; tmp[2] = buf[1]; tmp[3] = buf[0]; tmp[4]='\0';
		}
		else {
			SNPrintf(tmp, sizeof(tmp), "0x%08X",sh_video->format);
		}

		snprintf(vid_str, sizeof(vid_str), CS(MSG_Properties_Video), \
tmp, \
(int)(sh_video->i_bps * 0.008), \
sh_video->disp_w, sh_video->disp_h, \
sh_video->fps);
	}

// AUDIO
	if(sh_audio) {
		// Assume FOURCC if all bytes >= 0x20 (' ')
		if (sh_audio->format >= 0x20202020) {
			SNPrintf(buf, sizeof(buf), "%.4s",(char *)&sh_audio->format);
			tmp[0] = buf[3]; tmp[1] = buf[2]; tmp[2] = buf[1]; tmp[3] = buf[0]; tmp[4]='\0';
		}
		else {
			SNPrintf(tmp, sizeof(tmp), "0x%08lX",sh_audio->format);
		}

		snprintf(aud_str, sizeof(aud_str), CS(MSG_Properties_Audio), \
tmp, \
(int)(sh_audio->i_bps * 0.008), \
sh_audio->samplerate, \
sh_audio->channels);
	}

// CLIP
	SNPrintf(clip_str, sizeof(clip_str), CS(MSG_Properties_Clip), \
demux_info_get(mpctx->demuxer, "Title"), \
demux_info_get(mpctx->demuxer, "Artist"), \
demux_info_get(mpctx->demuxer, "Album"), \
demux_info_get(mpctx->demuxer, "Year"), \
demux_info_get(mpctx->demuxer, "Comment"), \
demux_info_get(mpctx->demuxer, "Genre") ); // from demuxer.c

	SNPrintf(tmp, sizeof(tmp), CS(MSG_Properties_Prop),vid_str,aud_str,clip_str);
	if(InfoWindowBase) open_InfoWindow_Properties(vid_str,aud_str,clip_str);
	else PrintMsg(tmp, REQTYPE_INFO, REQIMAGE_INFO, CS(MSG_Properties_Title));
}
/* Parts from mmplayer sources mplayer.c */
