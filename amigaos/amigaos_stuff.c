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

#define HAVE_AREXX 1

#ifdef __AMIGAOS4__
// #include <proto/dos.h>

/*#define debug_level 0

#if debug_level > 0
#define dprintf( ... ) DebugPrintF( __VA_ARGS__ )
#else
#define dprintf(...)
#endif

#else
#define dprintf(...)*/
#include "debug.h"
#endif

#include <exec/types.h>
#include <proto/timer.h>
#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/asl.h>
#include <devices/ahi.h>

// #include <proto/requester.h>
// #include <classes/requester.h>

/* ARexx */
#include "arexx.h"
/* ARexx */

APTR pr_WindowPtr;

static struct Task *current_task;


BOOL open_lib( const char *name, int ver , const char *iname, int iver, struct Library **base, struct Interface **interface);


// extern void ShowAbout(void);
extern void PrintMsg(CONST_STRPTR text, int REQ_TYPE, int REQ_IMAGE);
extern struct Process *About_Process, *ASL_Process, *Req_Process;
#include <classes/requester.h>


#include <stdbool.h>
#include "../osdep/keycodes.h" // KEY_ESC
#include "../mp_fifo.h"        // mplayer_put_key()
extern int spawn_count;        // Iconify()
extern BOOL Iconify(struct Window *w);             // cgx_common.c
extern BOOL UnIconify(struct Window *w);           // cgx_common.c
extern bool empty_msg_queue(struct MsgPort *port); // cgx_common.c

#ifndef __amigaos4__
#include <clib/alib_protos.h>
#endif

#include <proto/icon.h>
#include <proto/dos.h>
#include <proto/wb.h>
#include <proto/application.h>
#include <proto/intuition.h>
#include <proto/graphics.h>
#include <proto/utility.h>
#include <proto/icon.h>
#include <proto/datatypes.h>

#include <libraries/application.h>

#include <workbench/workbench.h>
#include <workbench/icon.h>
#include <workbench/startup.h>

// zzd10h
#include <proto/locale.h>
// new AEon InfoWindow Class
#include <classes/infowindow.h>


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// #include <stdarg.h>


#include "version.h"
#include "mp_msg.h"
#include "amigaos_stuff.h"

const char version[] = "$VER: " AMIGA_VERSION " " VERSION ;
// const char version[] = "$VER: AMIGA_VERSION 1.1 (10.01.2014) VERSION ";
const char STACK[] __attribute((used)) = "$STACK: 500000";

char *SCREENSHOTDIR = NULL;
char *EXTPATTERN = NULL;
char *SUBEXTPAT = NULL;
char *TOOL_MPGUI = NULL;

char fullpath_mplayer[1024] = "";

// static unsigned char ApplicationName[16];

BOOL AISS_MENU          = TRUE ;	// zzd10h
BOOL AISS_MENU_TO_SCALE = FALSE ;	// zzd10h
int AISS_MENU_SIZE      = 24 ;	// zzd10h Setted by MenuImageSize env variable
BOOL MENUCLASS_SUPPORT  = FALSE ;	// TRUE if MenuClass menu is supported (OS4 FE with intuition.library >= 54.6)
// BOOL INFOWINDOWCLASS_SUPPORT	= FALSE ;	// Check if AEon Infowindow class is available
BOOL BANNER_DISPLAY     = TRUE ;	// Display banner ? SETENV SAVE Disable_Banner_Images 1 to disable them

/*int SWAPLONG( int i )
{
	int o;
	asm
	(
		"lwarx %0,0,%1 \n"
		:  "=r" (o) : "r" (&i), "r" (&i)  : "r0"
	);
	return o;
}

short SWAPWORD( short hi )
{
	short ho;
	asm
	(
		"lhbrx %0,0,%1 \n"
		:  "=r" (ho) : "r" (&hi), "r" (&hi)  : "r0"
	);
	return ho;
}*/


struct Process *p = NULL; 	// This to remove
APTR OldPtr = NULL;		// The requesters

#ifdef USE_ASYNCIO
struct Library			*AsyncIOBase  = NULL;
struct AsyncIOIFace		*IAsyncIO = NULL;
#endif

APTR window_mx;

// Macros needs this global
// struct ARexxIFace		*IARexx = NULL;

struct Device		*TimerBase = NULL;
extern struct TimerIFace *ITimer; // NOTE: SDL 1.2.6 already 'GetInterface()'

struct Library		*IconBase = NULL;
struct IconIFace		*IIcon  = NULL;

struct Library *DataTypesBase     = NULL;
struct DataTypesIFace *IDataTypes = NULL;

struct Library		*AslBase = NULL;
struct AslIFace		*IAsl   = NULL;

struct Library		*GadToolsBase = NULL;
struct AslIFace		*IGadTools   = NULL;

struct Library		*ApplicationBase      = NULL;
struct ApplicationIFace	*IApplication = NULL;

struct Library		*GraphicsBase   = NULL;
struct GraphicsIFace	*IGraphics = NULL;

struct Library		*WorkbenchBase    = NULL;
struct WorkbenchIFace	*IWorkbench = NULL;

struct Library		*KeymapBase = NULL;
struct KeymapIFace	*IKeymap = NULL;

struct Library		*LayersBase = NULL;
struct LayersIFace	*ILayers = NULL;

struct Library		*Picasso96Base = NULL;
struct IP96IFace		*IP96        = NULL;

struct DebugIFace		*IDebug = NULL; // needed by libdvdcss ¿:-/

struct Library *UtilityBase   = NULL;
struct UtilityIFace *IUtility = NULL;

struct ClassLibrary *RequesterBase = NULL, *InfoWindowBase = NULL, *BitMapBase = NULL;
Class *RequesterClass = NULL, *InfoWindowClass = NULL, *BitMapClass = NULL;

#ifdef HAVE_CGX
struct Library *CyberGfxBase    = NULL;
struct CyberGfxIFace *ICyberGfx = NULL;
#endif

struct Library * IntuitionBase = NULL;
struct IntuitionIFace *IIntuition_SDL_workaround = NULL;
struct IntuitionIFace *IIntuition = NULL;

// UBYTE  TimerDevice = -1; // -1 -> not opened
struct TimeRequest *TimerRequest = NULL;
struct MsgPort     *TimerMsgPort = NULL;

static char **AmigaOS_argv = NULL;
static int AmigaOS_argc = 0;
uint32 AppID = 0;
struct MsgPort *applibPort = NULL;


extern STRPTR ARexxPortName;

static void Free_Arg(void);
// extern struct DiskObject *DIcon;
// extern struct AppIcon *MPLAppIcon;

// Catalog zzd10h
struct Library *LocaleBase  = NULL;
struct LocaleIFace *ILocale = NULL;
struct Catalog *catalog = NULL;
#define MPlayer_NUMBERS
#define MPlayer_STRINGS
#include "MPlayer_catalog.h"
CONST_STRPTR myGetCatalogStr (struct Catalog *catalog, LONG num, STRPTR def);
#define CS(id) myGetCatalogStr(catalog,id,id##_STR)

// Thomas Rapp function for catalog
CONST_STRPTR myGetCatalogStr (struct Catalog *catalog, LONG num, STRPTR def)
{
	if (catalog)
		return ((CONST_STRPTR)GetCatalogStr (catalog,num,def));

	return ((CONST_STRPTR)def);
}

#define GET_PATH(drawer,file,dest)                                                      \
	dest = (char *) malloc( ( strlen(drawer) + strlen(file) + 2 ) * sizeof(char) );        \
	if (dest)                                                                              \
	{                                                                                      \
		if ( strlen(drawer) == 0) strcpy(dest, file);                                         \
		else                                                                                  \
		{                                                                                     \
			if ( (drawer[ strlen(drawer) - 1  ] == ':')  ) sprintf(dest, "%s%s", drawer, file);  \
			else sprintf(dest, "%s/%s", drawer, file);                                           \
		}                                                                                     \
	}

/****************************/

static void Free_Arg(void)
{
	if (AmigaOS_argv)
	{
		ULONG i;
		for (i=0; i < AmigaOS_argc; i++)
			if(AmigaOS_argv[i]) free(AmigaOS_argv[i]);

		free(AmigaOS_argv);
	}
}


/******************************************************************************/
// Markus AltiVec info
extern struct Screen *FrontMostScr(void); // amigaos/menu.c

VOID VARARGS68K EasyRequester(CONST_STRPTR text, CONST_STRPTR button, ...)
{
  // struct TagItem tags[] = { ESA_Position, REQPOS_CENTERSCREEN, TAG_END };
  struct EasyStruct easyreq = { 0 };
  // static TEXT textbuffer[512];
  // STRPTR textbuffer = NULL;
  va_list parameters;
  STRPTR vargs = NULL;

  va_startlinear(parameters, button);
  // vsprintf(textbuffer, text, parameter);
  // va_end(parameter);
  vargs = va_getlinearva(parameters, STRPTR);

  easyreq.es_StructSize   = sizeof(struct EasyStruct);
  easyreq.es_Flags        = ESF_SCREEN | ESF_EVENSIZE; // | ESF_TAGGED;
  easyreq.es_Title        = CS(MSG_Requester_Title_Warning);
  easyreq.es_TextFormat   = text;
  easyreq.es_GadgetFormat = button;
  easyreq.es_Screen       = FrontMostScr();
  // easyreq.es_TagList      = tags;

  EasyRequestArgs(NULL, &easyreq, NULL, vargs);

  va_end(parameters);
}

/******************************/

void AmigaOS_ParseArg(int argc, char *argv[], int *new_argc, char ***new_argv)
{
	struct WBStartup *WBStartup = NULL;
	struct DiskObject *icon = NULL;
// DBUG("AmigaOS_ParseArg()\n",NULL);
/* Summer Edition Reloaded */
	/* Check for AltiVec... in case an AltiVec version is loaded on SAM or on a AmigaOne G3... */
	#if HAVE_ALTIVEC_H == 1
	ULONG result = 0;

	GetCPUInfoTags(GCIT_VectorUnit, &result, TAG_DONE);
	if (result != VECTORTYPE_ALTIVEC)
	{
		mp_msg(MSGT_VO, MSGL_ERR, "Sorry, this version is only for AltiVec capable machine!\n");
		EasyRequester( CS(MSG_Warning_AltiVec), CS(MSG_Warning_Button_OK) );

		// markus put MPlayer to exit mode
		exit_player_with_rc(0, 0);
		// end markus
	}
	#endif
	/* End Check */
/* Summer Edition Reloaded */

	*new_argc = argc;
	*new_argv = argv;

	// Some default values
	if (!SCREENSHOTDIR)
	{
		SCREENSHOTDIR = strdup("RAM:");
	}

	if (!EXTPATTERN)
	{
		EXTPATTERN = strdup("#?.(3g2|3gp|3ivx|asf|avi|cdxl|cin|dat|divx|flc|fli|flv|mpg|mpeg|mkv|mov|m2v|m4a|m4v|mjpg|mp3|mp4|m2ts|mts|nsv|nut|nuv|ogg|ogm|ogv|pva|qt|rm|rmvb|roq|smk|ts|vivo|vob|wav|webm|wma|wmv|wtv|xvid)");
	}

	if (!SUBEXTPAT)
	{
		SUBEXTPAT = strdup("#?.(aqt|ass|js|jss|pjs|rt|smi|srt|ssa|stl|sub|ttml|txt|utf|utf8|utf-8|vob|vtt)");
	}

	if(!TOOL_MPGUI)
	{
		TOOL_MPGUI = strdup("APPDIR:MPlayer-GUI");
	}

	// Ok is launched from cmd line, just do nothing
	if (argc)
	{
		return;
	}
	// Ok, ran from WB, we have to handle some funny thing now
	// 1. if there is some WBStartup->Arg then play that file and go
	// 2. else open an ASL requester and add the selected file

	// 1. WBStartup
	WBStartup = (struct WBStartup *) argv;
	if (!WBStartup)
	{
		return ; // We never know !
	}

/*
	if (Cli())
	{
		char progname[512];

		if (GetCliProgramName(progname, sizeof(progname)))
		{
			strcpy(progname, "PROGDIR:");
			strlcat(fullpath_mplayer, FilePart(progname), sizeof(fullpath_mplayer));
//			gotprogname = TRUE;
//		}
//	}
//	else
//	{
//		strlcpy(fullpath_mplayer, FindTask(NULL)->tc_Node.ln_Name, sizeof(fullpath_mplayer));
//		gotprogname = TRUE;
//	}
DBUG("2:fullpath_mplayer: '%s'\n",fullpath_mplayer);
*/

	// Check tooltypes
	icon = GetDiskObject(fullpath_mplayer);
	if (icon)
	{
		STRPTR found;

		found = FindToolType(icon->do_ToolTypes, "SCREENSHOTDIR");
		if (found)
		{
			free(SCREENSHOTDIR);
			SCREENSHOTDIR = strdup(found);
		}

		found = FindToolType(icon->do_ToolTypes, "EXTPATTERN");
		if (found)
		{
			free(EXTPATTERN);
			EXTPATTERN = strdup(found);
		}

		found = FindToolType(icon->do_ToolTypes, "SUBEXTPATTERN");
		if (found)
		{
			free(SUBEXTPAT);
			SUBEXTPAT = strdup(found);
		}

		found = FindToolType(icon->do_ToolTypes, "MPLAYERGUI");
		if (found)
		{
			free(TOOL_MPGUI);
			TOOL_MPGUI = strdup(found);
		}

		FreeDiskObject(icon);
	}

	if (WBStartup->sm_NumArgs > 1)
	{
		// The first arg is always the tool name (aka us)
		// Then if more than one arg, with have some file name
		ULONG i;

		// We will replace the original argc/argv by us
		AmigaOS_argc = WBStartup->sm_NumArgs;
		AmigaOS_argv = calloc(1, AmigaOS_argc * sizeof(char *) );
		if (!AmigaOS_argv) goto fail;

		// memset(AmigaOS_argv, 0x00, AmigaOS_argc * sizeof(char *) );

		for(i=0; i<AmigaOS_argc; i++)
		{
			AmigaOS_argv[i] = malloc(MAX_DOS_NAME + MAX_DOS_PATH + 1);
			if (!AmigaOS_argv[i])
			{
				goto fail;
			}
			NameFromLock(WBStartup->sm_ArgList[i].wa_Lock, AmigaOS_argv[i], MAX_DOS_PATH);
			AddPart(AmigaOS_argv[i], WBStartup->sm_ArgList[i].wa_Name, MAX_DOS_PATH+MAX_DOS_NAME);
// DBUG("'%s'\n",AmigaOS_argv[i]);
		}

		*new_argc = AmigaOS_argc;
		*new_argv = AmigaOS_argv;
	}
	else
	{
		ULONG i;
		struct FileRequester * AmigaOS_FileRequester = NULL;
		BPTR FavoritePath_File;
		char FavoritePath_Value[1024];
		BOOL FavoritePath_Ok = FALSE;

		FavoritePath_File = Open("PROGDIR:FavoritePath", MODE_OLDFILE);
		if (FavoritePath_File)
		{
			LONG size = Read(FavoritePath_File, FavoritePath_Value, sizeof(FavoritePath_Value) );
			if (size > 0)
			{
				if ( strchr(FavoritePath_Value, '\n') ) // There is an \n -> valid file
				{
					FavoritePath_Ok = TRUE;
					*(strchr(FavoritePath_Value, '\n')) = '\0';
				}
			}
			Close(FavoritePath_File);
		}

		AmigaOS_FileRequester = AllocAslRequest(ASL_FileRequest, NULL);
		if (!AmigaOS_FileRequester)
		{
			mp_msg(MSGT_CPLAYER, MSGL_FATAL, "Cannot open FileRequester!\n");
			goto fail;
		}

		if ( ( AslRequestTags( AmigaOS_FileRequester,
					ASLFR_TitleText,      CS(MSG_Requester_OpenFile_Video),
					ASLFR_DoMultiSelect,  TRUE,
					ASLFR_RejectIcons,    TRUE,
					ASLFR_DoPatterns,     TRUE,
					ASLFR_InitialPattern, (ULONG)EXTPATTERN,
					ASLFR_InitialDrawer,  (FavoritePath_Ok) ? FavoritePath_Value : "",
				TAG_DONE) ) == FALSE )
		{
			FreeAslRequest(AmigaOS_FileRequester);
			AmigaOS_FileRequester = NULL;
			mp_msg(MSGT_CPLAYER, MSGL_FATAL,"Fail AslRequestTags!\n");
			goto fail;
		}

		AmigaOS_argc = AmigaOS_FileRequester->fr_NumArgs + 1;
		AmigaOS_argv = calloc(1, AmigaOS_argc * sizeof(char *) );
		if (!AmigaOS_argv) goto fail;

		// memset(AmigaOS_argv, 0x00, AmigaOS_argc * sizeof(char *) );

		AmigaOS_argv[0] = strdup(FilePart(fullpath_mplayer));//"MPlayer");
// DBUG("'%s' (%s)\n",AmigaOS_argv[0],argv[0]);
		if (!AmigaOS_argv[0]) goto fail;

		for(i=1; i<AmigaOS_argc; i++)
		{
			GET_PATH(AmigaOS_FileRequester->fr_Drawer,
						AmigaOS_FileRequester->fr_ArgList[i-1].wa_Name,
						AmigaOS_argv[i]);
			if (!AmigaOS_argv[i])
			{
				FreeAslRequest(AmigaOS_FileRequester);
				AmigaOS_FileRequester = NULL;
				goto fail;
			}
		}

		*new_argc = AmigaOS_argc;
		*new_argv = AmigaOS_argv;

		FreeAslRequest(AmigaOS_FileRequester);
	}

	return;

fail:
	Free_Arg();

	*new_argc = argc;
	*new_argv = argv;
}


/****************************/
/****************************/
void AmigaOS_Close(void)
{
	// Delay(10);
DBUG("AmigaOS_Close()***\n",NULL);
	IIntuition = IIntuition_SDL_workaround;

	if (!SCREENSHOTDIR)
	{
		free(SCREENSHOTDIR);
		SCREENSHOTDIR = NULL;
	}

	if (!EXTPATTERN)
	{
		free(EXTPATTERN);
		EXTPATTERN = NULL;
	}

	if (!SUBEXTPAT)
	{
		free(SUBEXTPAT);
		SUBEXTPAT = NULL;
	}

	if (!TOOL_MPGUI)
	{
		free(TOOL_MPGUI);
		TOOL_MPGUI = NULL;
	}

	Forbid();
	current_task = FindTask(NULL);
	Permit();
	((struct Process *) current_task) -> pr_WindowPtr = pr_WindowPtr;	// enable insert disk.

	if (OldPtr)
	{
		// SetTaskPri(p,0);
		SetProcWindow((APTR) OldPtr);
	}

/*#if HAVE_APP_ICON
	if (MPLAppIcon) RemoveAppIcon(MPLAppIcon);
	if (DIcon) FreeDiskObject(DIcon);
#endif*/

	if (AppID > 0)
	{
//		if (IApplication)
//		{
			SetApplicationAttrs(AppID, APPATTR_AllowsBlanker,TRUE, TAG_DONE);
			SetApplicationAttrs(AppID, APPATTR_NeedsGameMode,FALSE, TAG_DONE);
			// SendApplicationMsg(AppID, 0, NULL, APPLIBMT_BlankerAllow);
			UnregisterApplication(AppID, NULL);
//		}
		AppID = 0;
	}

	Free_Arg();

	if (catalog) CloseCatalog(catalog);

	if (InfoWindowBase) CloseClass(InfoWindowBase);
	if (BitMapBase    ) CloseClass(BitMapBase);
	if (RequesterBase ) CloseClass(RequesterBase);

	if (IDebug) DropInterface((struct Interface*)IDebug); IDebug = 0;

#ifdef USE_ASYNCIO
	if (AsyncIOBase) CloseLibrary( AsyncIOBase ); AsyncIOBase = 0;
	if (IAsyncIO) DropInterface((struct Interface*)IAsyncIO); IAsyncIO = 0;
#endif

#if HAVE_AREXX
	StopArexx();
#endif

	if (IntuitionBase) CloseLibrary(IntuitionBase); IntuitionBase = 0;
	if (IIntuition) DropInterface((struct Interface*) IIntuition); IIntuition = 0;
	IIntuition_SDL_workaround = 0;

	if (UtilityBase) CloseLibrary(AslBase); UtilityBase = 0;
	if (IUtility) DropInterface((struct Interface*)IUtility); IUtility = 0;

	if (GraphicsBase) CloseLibrary(GraphicsBase); GraphicsBase = 0;
	if (IGraphics) DropInterface((struct Interface*) IGraphics); IGraphics = 0;

	if (ApplicationBase) CloseLibrary(ApplicationBase); ApplicationBase = 0;
	if (IApplication) DropInterface((struct Interface*)IApplication); IApplication = 0;

	if (WorkbenchBase) CloseLibrary(WorkbenchBase); WorkbenchBase = 0;
	if (IWorkbench) DropInterface((struct Interface*)IWorkbench); IWorkbench = 0;

	if (KeymapBase) CloseLibrary(KeymapBase); KeymapBase = 0;
	if (IKeymap) DropInterface((struct Interface*) IKeymap); IKeymap = 0;

	if (LayersBase) CloseLibrary(LayersBase); LayersBase = 0;
	if (ILayers) DropInterface((struct Interface*) ILayers); ILayers = 0;

	if (IconBase) CloseLibrary(IconBase); IconBase = 0;
	if (IIcon) DropInterface((struct Interface*)IIcon); IIcon = 0;

	if (DataTypesBase) CloseLibrary(DataTypesBase); DataTypesBase = 0;
	if (IDataTypes) DropInterface((struct Interface*)IDataTypes); IDataTypes = 0;

	if (AslBase) CloseLibrary(AslBase); AslBase = 0;
	if (IAsl) DropInterface((struct Interface*)IAsl); IAsl = 0;

	if (GadToolsBase) CloseLibrary(GadToolsBase); GadToolsBase = 0;
	if (IGadTools) DropInterface((struct Interface*) IGadTools); IGadTools = 0;

	if (LocaleBase) CloseLibrary(LocaleBase); LocaleBase = 0;
	if (ILocale) DropInterface((struct Interface*) ILocale); ILocale = 0;

	if (Picasso96Base) CloseLibrary(Picasso96Base); Picasso96Base = 0;
	if (IP96) DropInterface((struct Interface*)IP96); IP96 = 0;

	// if (!TimerDevice) CloseDevice( (struct IORequest *) TimerDevice); TimerDevice = 0;
	if (TimerRequest)
	{
		CloseDevice( (struct IORequest *) TimerRequest );
		FreeSysObject ( ASOT_IOREQUEST, TimerRequest ); TimerRequest = 0;
	}
	if (TimerMsgPort) FreeSysObject ( ASOT_PORT, TimerMsgPort ); TimerMsgPort = 0;
	// if (ITimer) DropInterface((struct Interface*) ITimer); ITimer = 0; // NOTE: SDL 1.2.6 already 'GetInterface()'

	if (window_mx) FreeSysObject( ASOT_MUTEX , window_mx ); window_mx = 0;

#ifdef CONFIG_AHI
//	close_ahi();
#endif

	if (DOSBase) CloseLibrary(DOSBase); DOSBase = 0;
	if (IDOS) DropInterface((struct Interface*)IDOS); IDOS = 0;
}

/****************************/
/****************************/


BOOL open_lib( const char *name, int ver , const char *iname, int iver, struct Library **base, struct Interface **interface)
{
	*interface = NULL;

	*base = OpenLibrary( name , ver);
	if (*base)
	{
		*interface = GetInterface( *base, iname, iver, TAG_END );
		if (!*interface) DebugPrintF("Unable to get %s IFace for '%s' %ld!\n",iname,name,ver);
	}
	else
	{
		DebugPrintF("Unable to open the '%s' %ld!\n",name,ver);
	}

	return (*interface) ? TRUE : FALSE;
}

void read_ENVARC(void)	// zzd10h
{
	char var_buffer[16] ;

	if (GetVar("MenuImageSize",var_buffer,sizeof(var_buffer),LV_VAR) > 0)
	{
		AISS_MENU_SIZE = atoi(var_buffer) ;
		if (AISS_MENU_SIZE > 0) AISS_MENU_TO_SCALE = TRUE ;
		else AISS_MENU = FALSE ;
	}

	// SETENV SAVE Disable_Banner_Images 1
	if (GetVar("Disable_Banner_Images",var_buffer,sizeof(var_buffer),LV_VAR) > 0)
	{
// DBUG("Disable_Banner_Images (%s)\n",var_buffer);
		if (atoi(var_buffer) == 1)
		BANNER_DISPLAY = FALSE ;
	}
}

int AmigaOS_Open(int argc, char *argv[])
{
	// struct DiskObject *icon = NULL;
	TEXT progname[512];
DBUG("***AmigaOS_Open()\n",NULL);
	AmigaOS_argv = NULL;

	// Remove requesters
	// OldPtr = SetProcWindow((APTR) -1L);

	window_mx = AllocSysObject( ASOT_MUTEX, TAG_DONE );
	if (!window_mx) return -1;

	if ( ! open_lib( "dos.library",        0L, "main", 1, &DOSBase, (struct Interface **) &IDOS ) ) return -1;
	if ( ! open_lib( "utility.library",   51L, "main", 1, &UtilityBase, (struct Interface **) &IUtility ) ) return -1;
	if ( ! open_lib( "intuition.library", 51L, "main", 1, &IntuitionBase, (struct Interface **) &IIntuition ) ) return -1;

	Forbid();
	current_task = FindTask(NULL);
	Permit();
	pr_WindowPtr = ((struct Process *) current_task) -> pr_WindowPtr;
	((struct Process *) current_task) -> pr_WindowPtr = -1L;	// Disable insert disk.

#if HAVE_AREXX
	StartArexx(); // start it ASAP, 'cos we need ARexxPortName
#endif

	TimerMsgPort = AllocSysObject(ASOT_PORT,NULL);
	if ( !TimerMsgPort )
	{
		mp_msg(MSGT_CPLAYER, MSGL_FATAL, "Failed to create timer message port\n");
		return -1;
	}

	TimerRequest = AllocSysObjectTags(ASOT_IOREQUEST,
	                ASOIOR_Size, sizeof(struct TimeRequest),
	                ASOIOR_ReplyPort, TimerMsgPort,
	               TAG_DONE);
	if ( !TimerRequest )
	{
		mp_msg(MSGT_CPLAYER, MSGL_FATAL, "Failed to create timer I/O request\n");
		return -1;
	}

	if ( OpenDevice((unsigned char*)TIMERNAME, UNIT_MICROHZ, (struct IORequest *) TimerRequest, 0) )
	{
		mp_msg(MSGT_CPLAYER, MSGL_FATAL, "Failed to open" TIMERNAME "\n");
		return -1;
	}

	TimerBase = (struct Device *) TimerRequest->Request.io_Device;
	// ITimer = (struct TimerIFace*) GetInterface( (struct Library*)TimerBase, (unsigned char*)"main", 1, NULL ); // NOTE: SDL 1.2.6 already 'GetInterface()'


#if HAVE_ALTIVEC
	SetAmiUpdateENVVariable("MPlayer-AltiVec");
#else
	SetAmiUpdateENVVariable("MPlayer");
#endif

	// IDebug = GetInterface( SysBase, "debug", 1, TAG_END );
	IDebug = (struct DebugIFace *)GetInterface((struct Library *)SysBase, "debug", 1, TAG_END);

#ifdef USE_ASYNCIO
	if ( ! ( AsyncIOBase = OpenLibrary( (unsigned char*)"asyncio.library", 0L) ) )
	{
		mp_msg(MSGT_CPLAYER, MSGL_FATAL, "Unable to open the asyncio.library\n");
		return -1;
	}

	if (!(IAsyncIO = (struct AsyncIOIFace *)GetInterface((struct Library *)AsyncIOBase,(unsigned char*)"main",1,NULL)))
	{
		mp_msg(MSGT_CPLAYER, MSGL_FATAL, "Failed to get AsyncIO interface\n");
		return -1;
	}
#endif

#ifdef HAVE_CGX
	if ( ! open_lib( "cybergraphics.library", 43L, "main", 1, &CyberGfxBase, (struct Interface **) &ICyberGfx  ) ) return -1;
#endif

	if ( ! open_lib( "asl.library",           0L, "main", 1, &AslBase, (struct Interface **) &IAsl ) ) return -1;
	if ( ! open_lib( "graphics.library",     51L, "main", 1, &GraphicsBase, (struct Interface **) &IGraphics ) ) return -1;
	if ( ! open_lib( "application.library",  51L, "application", 2, &ApplicationBase, (struct Interface **) &IApplication ) ) return -1;
	if ( ! open_lib( "workbench.library",    51L, "main", 1, &WorkbenchBase, (struct Interface **) &IWorkbench ) ) return -1;
	if ( ! open_lib( "keymap.library",       51L, "main", 1, &KeymapBase, (struct Interface **) &IKeymap ) ) return -1;
	if ( ! open_lib( "layers.library",       51L, "main", 1, &LayersBase, (struct Interface **) &ILayers ) ) return -1;
	if ( ! open_lib( "Picasso96API.library", 51L, "main", 1, &Picasso96Base, (struct Interface **) &IP96 ) ) return -1;
	if ( ! open_lib( "gadtools.library",     51L, "main", 1, &GadToolsBase, (struct Interface **) &IGadTools ) ) return -1;
	if ( ! open_lib( "icon.library",         51L, "main", 1, &IconBase, (struct Interface **) &IIcon ) ) return -1;
	if ( ! open_lib( "datatypes.library",    51L, "main", 1, &DataTypesBase, (struct Interface **) &IDataTypes ) ) return -1;
	if ( ! open_lib( "locale.library",        0L, "main", 1, &LocaleBase, (struct Interface **) &ILocale ) ) return -1;

// #if HAVE_AREXX
//	StartArexx(current_task);
// #endif

	// Classes
	RequesterBase  = OpenClass("requester.class",     53, &RequesterClass);
	BitMapBase     = OpenClass("images/bitmap.image", 52, &BitMapClass);
	InfoWindowBase = OpenClass("infowindow.class",    53, &InfoWindowClass);

	IIntuition_SDL_workaround = IIntuition;	// save IIntuition as SDL sets this to NULL :-(

	// MenuClass supported if intuition.library >= 54.6) [zzd10h]
	if ( LIB_IS_AT_LEAST(IntuitionBase, 54,6) )
	{
		MENUCLASS_SUPPORT = TRUE;
	}

	// Open catalog zzd10h
	if(LocaleBase)
		catalog=OpenCatalogA(NULL,"MPlayer.catalog",NULL);

	if (argc)
	{ // CLI/Shell
		NameFromLock( GetProgramDir(), fullpath_mplayer, sizeof(fullpath_mplayer) );
		GetCliProgramName( progname, sizeof(progname) );
	}
	else
	{ // Workbench
		struct WBStartup *WBStartup = (struct WBStartup *)argv;
		NameFromLock( WBStartup->sm_ArgList->wa_Lock, fullpath_mplayer, sizeof(fullpath_mplayer) );
		Strlcpy( progname, WBStartup->sm_ArgList->wa_Name, sizeof(progname) );
		// If argc == 0 -> Execute from the WB, then no need to display anything
		freopen("NIL:", "r", stdin);
		freopen("NIL:", "w", stderr);
		freopen("NIL:", "w", stdout);
	}
	AddPart( fullpath_mplayer, FilePart(progname), sizeof(fullpath_mplayer) );
DBUG("progname: '%s'\n",FilePart(progname));
DBUG("fullpath: '%s'\n",fullpath_mplayer);

DBUG("ARexx: '%s'\n",ARexxPortName);
	// Register the application so the ScreenBlanker don't bother us...
	AppID = RegisterApplication( ARexxPortName, // "MPlayer",
			REGAPP_Description,   CS(MSG_RegisterAppID_Description),
			REGAPP_URLIdentifier, "mplayerhq.hu",
			REGAPP_ENVDir,        "mplayer",
			REGAPP_FileName,      fullpath_mplayer,
			REGAPP_AppNotifications,  TRUE,
			REGAPP_HasIconifyFeature, TRUE,
			REGAPP_UniqueApplication, TRUE, // FALSE,
			REGAPP_HasPrefsWindow,    FALSE,
			REGAPP_LoadPrefs,         FALSE,
			REGAPP_SavePrefs,         FALSE,
			REGAPP_CanCreateNewDocs,  FALSE,
			REGAPP_CanPrintDocs,      FALSE,
			REGAPP_NoIcon,            FALSE,
			REGAPP_AllowsBlanker,     FALSE,
		TAG_DONE);
	if (AppID == 0)
	{
		mp_msg(MSGT_CPLAYER, MSGL_FATAL, "Failed to Register Application.\n");
		return -1;
	}
	else
	{
		SetApplicationAttrs(AppID, APPATTR_AllowsBlanker,FALSE, TAG_DONE);
		GetApplicationAttrs(AppID, APPATTR_Port,(uint32)&applibPort, TAG_DONE);
	}
DBUG("AppID=%ld   applibPort=0x%08lx \n",AppID,applibPort);

	// Check if MenuImageSize is stored in ENV to determine menu picture size [zzd10h]
	read_ENVARC() ;

	return 0;
}


/*
char *mp_asprintf( char *fmt, ... )
{
	char dest[1024*16];
	va_list ap;
	va_start(ap, fmt);
	vsprintf(dest, fmt, ap);
	va_end(ap);

	return strdup(dest);
}

*/

/**********************************************************
**
** The following function saves the variable name passed in
** 'varname' to the ENV(ARC) system so that the application
** can become AmiUpdate aware.
**
**********************************************************/
// markus & samo79
// From http://codebench.co.uk/amiupdate_website/index.php?page=developers
// to	https://raw.githubusercontent.com/adtools/abcsh/master/amigaos.c
void SetAmiUpdateENVVariable(const char *varname)
{
	/* AmiUpdate support code */
	BPTR lock;
	APTR oldwin;

	/* Obtain the lock to the home directory */
	if((lock = GetProgramDir()) != 0)
	{
		TEXT progpath[2048];
		TEXT varpath[1024] = "AppPaths";

		/*
		 * Get a unique name for the lock,
		 * this call uses device names,
		 * as there can be multiple volumes
		 * with the same name on the system
		 */
		if(DevNameFromLock(lock, progpath, sizeof(progpath), DN_FULLPATH))
		{
			/* stop any "Insert volume..." type requesters */
			oldwin = SetProcWindow((APTR)-1);

			/*
			 * Finally set the variable to the
			 * path the executable was run from
			 * don't forget to supply the variable
			 * name to suit your application
			 */
			AddPart(varpath, varname, 1024);
			SetVar(varpath, progpath, -1,
			       GVF_GLOBAL_ONLY|GVF_SAVE_VAR);

			/* turn requesters back on */
			SetProcWindow(oldwin);
		}
	}
}


#include "SDL/SDL_syswm.h"
struct Window *AmigaOS_GetSDLWindowPtr(void)
{
  SDL_SysWMinfo wmInfo;

  if(SDL_GetWMInfo(&wmInfo) == 1) { return(wmInfo.window); }

  return(NULL);
}

void AmigaOS_ScreenTitle(const char *vo_str)
{
  struct Window *win;
  static char scrtitle[128];

  if( (win=AmigaOS_GetSDLWindowPtr()) == NULL ) return;

  sprintf(scrtitle, AMIGA_VERSION" (%s)\0",vo_str);
  SetWindowTitles(win, (CONST_STRPTR)~0, scrtitle);
}

/*#include <intuition/imageclass.h>
#include <../amigaos/window_icons.h>
extern struct kIcon fullscreenicon;
void AmigaOS_AddSDLWinFSGadget(void)
{
	struct Window *win;

	if( (win=AmigaOS_GetSDLWindowPtr()) == NULL ) return;

	dispose_icon(win, &fullscreenicon );
	open_icon(win, SETTINGSIMAGE, GID_FULLSCREEN, &fullscreenicon);
	RefreshWindowFrame(win); // or it won't show/render added gadgets
}*/

void AmigaOS_do_applib(struct Window *w)
{
	struct ApplicationMsg *applibmsg;
// DBUG("AmigaOS_do_applib()\n",NULL);
	while( (applibmsg=(struct ApplicationMsg *)GetMsg(applibPort)) )
	{
		switch(applibmsg->type)
		{
			case APPLIBMT_Quit:
			case APPLIBMT_ForceQuit:
DBUG("APPLIB_QUIT\n",NULL);
				if(w  &&  spawn_count==0) mplayer_put_key(KEY_ESC); // if no window opened don't quit
			break;
			case APPLIBMT_Unhide:
DBUG("APPLIB_UNHIDE\n",NULL);
				if( UnIconify(w) ) SetApplicationAttrs(AppID, APPATTR_Hidden,FALSE, TAG_DONE);
				else
				{
					ActivateWindow(w);
					WindowToFront(w);
				}
			break;
			case APPLIBMT_Hide:
DBUG("APPLIB_HIDE\n",NULL);
				if( Iconify(w) ) SetApplicationAttrs(AppID, APPATTR_Hidden,TRUE, TAG_DONE);
			break;
			// default: break;
		}
		ReplyMsg( (struct Message *)applibmsg );
	}
}

VOID VARARGS68K AmigaOS_applib_Notify(STRPTR not_msg, ...)
{
	va_list parameters;
	STRPTR vargs = NULL;
	STRPTR buf = NULL;

	va_startlinear(parameters, not_msg);
	vargs = va_getlinearva(parameters, STRPTR);

	buf = VASPrintf(not_msg, vargs);

	Notify(AppID,
	       APPNOTIFY_Title,         "MPlayer",
	       // APPNOTIFY_Update,        TRUE,
	       // APPNOTIFY_Pri,           0,
	       APPNOTIFY_PubScreenName, "FRONT",
	       APPNOTIFY_ImageFile,     "TBImages:video",
	       // APPNOTIFY_CloseOnDC,     TRUE,
	       APPNOTIFY_Text,          buf,
	      TAG_DONE);

	FreeVec(buf);

	va_end(parameters);
}

void closeRemainingOpenWin(void)
{
DBUG("closeRemainingOpenWin()\n",NULL);
	if(About_Process) {
		uint32 pid = GetPID(About_Process, GPID_PROCESS);
DBUG("  [%ld]About_Process (0%08lx)\n",pid,About_Process);
		Signal( (struct Task*)About_Process, SIGBREAKF_CTRL_C );
		WaitForChildExit(pid);
	}

	if(ASL_Process) {
		uint32 pid = GetPID(ASL_Process, GPID_PROCESS);
DBUG("  [%ld]ASL_Process (0%08lx)\n",pid,ASL_Process->pr_Task.tc_UserData);
		AbortAslRequest(ASL_Process->pr_Task.tc_UserData);
		WaitForChildExit(pid);
	}

	if(Req_Process) {
		uint32 pid = GetPID(Req_Process, GPID_PROCESS);
// DBUG("  [%ld]Req_Process (0%08lx)\n",pid,Req_Process->pr_Task.tc_UserData);
DBUG("  [%ld]Req_Process\n",pid);
// ToDo: howto "quit" requester.class window ¿:-/
// SetAttrs( (Object*)Req_Process->pr_Task.tc_UserData, REQ_TimeOutSecs,1, TAG_END );
// Req_Process->pr_Task.tc_UserData = NULL;
mp_msg(MSGT_VO, MSGL_INFO, "WARNING: still one requester opened! Close it to exit properly.\n");
		WaitForChildExit(pid);
	}
}
