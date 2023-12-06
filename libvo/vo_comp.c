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

/*
 *  vo_comp.c
 *  VO module for MPlayer AmigaOS4
 *  Using Composition / direct VMEM
 *  Written by Kjetil Hvalstrand
 *  Based on work by DET Nicolas and FAB, see vo_wpa_cgx.c
 */

/*markus
  we do not currently compile the old printf debug
  future code to be removed as it will work
*/
//#define OLD_DEBUG
//end markus

/*#define debug_level 0

#if debug_level > 0
#define dprintf( ... ) Printf( __VA_ARGS__ )
#else
#define dprintf(...)
#endif*/
#include "../amigaos/debug.h"

extern struct Catalog *catalog;
#define MPlayer_NUMBERS
#define MPlayer_STRINGS
#include "../amigaos/MPlayer_catalog.h"
extern STRPTR myGetCatalogStr (struct Catalog *catalog, LONG num, STRPTR def);
#define CS(id) myGetCatalogStr(catalog,id,id##_STR)


#define SYSTEM_PRIVATE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "config.h"

#include "mp_msg.h"
#include "video_out.h"
#include "video_out_internal.h"
#include "cgx_common.h"
#include "version.h"
#include "sub/osd.h"
#include "sub/sub.h"

// markus iconify ?
//#include <classes/window.h>
#include <intuition/imageclass.h>
#include <../amigaos/window_icons.h>

extern struct kIcon iconifyIcon;
//extern struct kIcon padlockicon;
extern struct kIcon fullscreenicon;
// end markus

#include "aspect.h"

// --benchmark--
static struct timezone dontcare = { 0,0 };


static struct timeval bt_before, bt_after;

static struct timeval before, after;
ULONG benchmark_frame_cnt = 0;
// ------------------

// markus
static char * window_title;
extern char * filename;

static char *GetWindowTitle(void)
{
   if (window_title) free(window_title);

   if (filename)
   {
      window_title = (char *)malloc(strlen("MPlayer - ") + strlen(filename) + 1);
      strcpy(window_title, "MPlayer - ");
      strcat(window_title, filename);
   } else {
      window_title = strdup("MPlayer " AMIGA_VERSION " (comp)");
   }

   return window_title;
}
// end markus

/*
#include <proto/Picasso96API.h>
*/

#include "../ffmpeg/libswscale/swscale.h"
#include "../ffmpeg/libswscale/swscale_internal.h" // FIXME
#include "../ffmpeg/libswscale/rgb2rgb.h"
#include "../libmpcodecs/vf_scale.h"


#ifdef CONFIG_GUI
#include "gui/interface.h"
#include "gui/morphos/gui.h"
#include "mplayer.h"
#endif

#include <inttypes.h>		// Fix <postproc/rgb2rgb.h> bug

// Debug
//#define kk(x)
//#define dprintf(...)

// OS specific


#include <proto/intuition.h>
#include <proto/graphics.h>
#include <proto/window.h>

#include <graphics/composite.h>
#include <proto/dos.h>
#include <proto/exec.h>
#include <exec/types.h>
#include <dos/dostags.h>
#include <dos/dos.h>

struct Task *init_task = NULL;
extern APTR window_mx;

// Legacy from p96 (not p96 anymore)

struct RenderInfo
{
	char *Memory;
	ULONG BytesPerRow;
	enum enPixelFormat RGBFormat;
	ULONG pad;
};


static BOOL FirstTime = TRUE;

void draw_comp(struct BitMap *the_bitmap,struct Window * the_win, int width,int height);

static vo_info_t info =
{
	"Composition, R8G8B8",
	"comp",
	"Kjetil Hvalstrand & DET Nicolas & FAB",
	"AmigaOS4 rules da world !"
};

LIBVO_EXTERN(comp)

struct XYSTW_Vertex3D {
float x, y;
float s, t, w;
};

#ifdef OLD_DEBUG
void print_monitor_info()
{
		struct MonitorSpec *ms;
		if (ms = OpenMonitor(NULL,NULL))
	{
		Printf("Monitor ratio: %ld, %ld\n", ms -> ratioh, ms-> ratiov);

		CloseMonitor(ms);
	}
 }
#endif

struct BitMap *the_bitmap = NULL;
struct RenderInfo output_ri;

static uint32_t org_amiga_image_width;	// Well no comment
static uint32_t org_amiga_image_height;

extern uint32_t amiga_image_width;	// Well no comment
extern uint32_t amiga_image_height;
extern float amiga_aspect_ratio;
static uint32_t amiga_image_bpp;		// Image bpp

static struct Window *        My_Window = NULL;
static struct Screen *        My_Screen = NULL;

//static struct Window *        bf_Window = NULL;
static struct Hook *backfillhook;

static float best_screen_aspect_ratio;

// static uint32_t get_image(mp_image_t * mpi);

void alloc_output_buffer( ULONG width, ULONG height )
{
	amiga_image_width = width ;
	amiga_image_height = height ;
	org_amiga_image_width = width;
	org_amiga_image_height = height;

	output_ri.BytesPerRow = amiga_image_width*amiga_image_bpp;
	output_ri.RGBFormat = gfx_common_rgb_format;
	output_ri.pad = 0;
	output_ri.Memory = AllocVec( output_ri.BytesPerRow * height+2 , MEMF_CLEAR);
}


void draw_comp_bitmap(struct BitMap *the_bitmap,struct BitMap *the_bitmap_dest, int width,int height, int wx,int wy,int ww, int wh)
{
	#define STEP(a,xx,yy,ss,tt,ww)   P[a].x= xx; P[a].y= yy; P[a].s= ss; P[a].t= tt; P[a].w= ww;

	int error;
	struct XYSTW_Vertex3D P[6];

	STEP(0, wx, wy ,0 ,0 ,1);
	STEP(1, wx+ww,wy,width,0,1);
	STEP(2, wx+ww,wy+wh,width,height,1);

	STEP(3, wx,wy, 0,0,1);
	STEP(4, wx+ww,wy+wh,width,height,1);
	STEP(5, wx, wy+wh ,0 ,height ,1);

	if (the_bitmap)
	{
		LockLayer(0, My_Window->RPort->Layer);

		error = CompositeTags(COMPOSITE_Src,
			the_bitmap, the_bitmap_dest,

			COMPTAG_VertexArray, P,
			COMPTAG_VertexFormat,COMPVF_STW0_Present,
			COMPTAG_NumTriangles,2,

			COMPTAG_SrcAlpha, (uint32) (0x0010000 ),
			COMPTAG_Flags, COMPFLAG_SrcAlphaOverride | COMPFLAG_HardwareOnly | COMPFLAG_SrcFilter ,
		TAG_DONE);

		UnlockLayer(My_Window->RPort->Layer);
	}
}


void draw_comp(struct BitMap *the_bitmap,struct Window * the_win, int width,int height)
{
	draw_comp_bitmap(the_bitmap, the_win->RPort -> BitMap, width, height,
		the_win->BorderLeft + the_win -> LeftEdge,
		the_win->BorderTop + the_win -> TopEdge,
		the_win->Width - the_win->BorderLeft - the_win->BorderRight,
		the_win->Height -  the_win->BorderTop - the_win->BorderBottom);
}


static vo_draw_alpha_func draw_alpha_func;



static struct RastPort *       rp             = NULL;

static struct MsgPort *        UserMsg        = NULL;

// not OS specific

static uint32_t   window_width;           // width and height on the window
static uint32_t   window_height;          // can be different from the image
static uint32_t   screen_width;           // Indicates the size of the screen in full screen
static uint32_t   screen_height;          // Only use for triple buffering

extern uint32_t is_fullscreen;
extern uint32_t can_go_faster;


BOOL	is_paused = FALSE;
extern BOOL    is_ontop;

extern UWORD *EmptyPointer;               // Blank pointer
extern ULONG WantedModeID;

static uint32_t   offset_x;               // Offset in the rp where we have to display the image
static uint32_t   offset_y;               // ...

static uint32_t   amiga_image_format;

static uint32_t   win_left;
static uint32_t   win_top;

static SwsContext *swsContext=NULL;

/***************************/
struct BackFillArgs
{
	struct Layer     *layer;
	struct Rectangle  bounds;
	LONG              offsetx;
	LONG              offsety;
};



static void BackFill_Func(struct RastPort *ArgRP, struct BackFillArgs *MyArgs);

static struct Hook BackFill_Hook =
{
	{NULL, NULL},
	(HOOKFUNC) &BackFill_Func,
	NULL,
	NULL
};


#ifdef __GNUC__
# pragma pack(2)
#endif

#ifdef __GNUC__
# pragma pack()
#endif

struct BitMap *inner_bitmap = NULL;

static void draw_inside_window()
{
	int ww = 0;
	int wh = 0;

	if (!My_Window) return;

	MutexObtain( window_mx );

	ww = My_Window->Width - My_Window->BorderLeft - My_Window->BorderRight;
	wh = My_Window->Height - My_Window->BorderTop - My_Window->BorderBottom;

	if (inner_bitmap)
	{
		if ((GetBitMapAttr(inner_bitmap,BMA_WIDTH) != ww)||(GetBitMapAttr(inner_bitmap,BMA_HEIGHT) != wh))
		{
			FreeBitMap(inner_bitmap);
			inner_bitmap = NULL;
		}
	}

	if (!inner_bitmap)
	{
		inner_bitmap = AllocBitMap( ww , wh, 32, BMF_DISPLAYABLE, My_Window ->RPort -> BitMap);
	}

	if (inner_bitmap)
	{
		draw_comp_bitmap(the_bitmap, inner_bitmap, amiga_image_width, amiga_image_height,0,0,ww,wh);

		BltBitMapRastPort(inner_bitmap, 0, 0, My_Window -> RPort,
			My_Window -> BorderLeft,
			My_Window -> BorderTop,
			ww, wh,0xc0);
	}

	MutexRelease(window_mx);
}

static void BackFill_Func(struct RastPort *ArgRP, struct BackFillArgs *MyArgs)
{
	struct BitMap *bitmap;
	struct RastPort rp;
	int ww;
	int wh;

	ww = My_Window->Width - My_Window->BorderLeft - My_Window->BorderRight;
	wh = My_Window->Height - My_Window->BorderTop - My_Window->BorderBottom;

	bitmap = AllocBitMap( ww , wh, 32, BMF_DISPLAYABLE, My_Window ->RPort -> BitMap);
	if (bitmap)
	{
		InitRastPort(&rp);
		rp.BitMap = the_bitmap;

		WritePixelArray( output_ri.Memory, 0, 0, output_ri.BytesPerRow, PIXF_A8R8G8B8, &rp, 0,0, amiga_image_width,amiga_image_height);

		draw_comp_bitmap(the_bitmap, bitmap, amiga_image_width, amiga_image_height,	0,0,ww,wh);

		BltBitMapRastPort(bitmap, 0, 0, My_Window -> RPort,
			My_Window -> BorderLeft,
			My_Window -> BorderTop,
			ww, wh,0xc0);

		FreeBitMap(bitmap);
	}
}




/******************************** DRAW ALPHA *****************************************/
// Draw_alpha series !
static void draw_alpha_null (int x0,int y0, int w,int h, unsigned char* src, unsigned char *srca, int stride)
{
	// Nothing
}

static void draw_alpha_rgb32 (int x0,int y0, int w,int h, unsigned char* src, unsigned char * srca, int stride)
{
	vo_draw_alpha_rgb32(w,h,src,srca,
		stride,
		(UBYTE *) ( (ULONG) output_ri.Memory + (y0*output_ri.BytesPerRow)+(x0*amiga_image_bpp) ), output_ri.BytesPerRow );
}

static void draw_alpha_rgb24 (int x0,int y0, int w,int h, unsigned char* src, unsigned char * srca, int stride)
{
	vo_draw_alpha_rgb24(w,h,src,srca,
		stride,
		(UBYTE *) ( (ULONG) output_ri.Memory + (y0*output_ri.BytesPerRow)+(x0*amiga_image_bpp) ), output_ri.BytesPerRow );
}

static void draw_alpha_rgb16 (int x0,int y0, int w,int h, unsigned char* src, unsigned char * srca, int stride)
{
	vo_draw_alpha_rgb16(w,h,src,srca,
		stride,
		(UBYTE *) ( (ULONG) output_ri.Memory + (y0*output_ri.BytesPerRow)+(x0*amiga_image_bpp) ), output_ri.BytesPerRow );
}



static struct gfx_command commands[] = {
	{	"help",		's',	NULL,	"HELP/S", gfx_help},
	{	"NOBORDER",		's',	NULL,	"NOBORDER/S", fix_border },
	{	"TINYBORDER",	's',	NULL,	"TINYBORDER/S", fix_border },
	{	"ALWAYSBORDER",	's',	NULL,	"ALWAYSBORDER/S", fix_border },
	{	"DISAPBORDER",	's',	NULL,	"DISAPBORDER/S", fix_border },

	{	"16",			's',	NULL,	"16/S (only for comp)", fix_rgb_modes },
	{	"24",			's',	NULL,	"24/S (only for comp)", fix_rgb_modes },
	{	"32",			's',	NULL,	"32/S (only for comp)", fix_rgb_modes },

	{	"NOVSYNC",		's',	(ULONG *) &gfx_novsync,		"NOVSYNC/S", NULL },
	{	"MODEID=",		'h',	(ULONG *) &WantedModeID,	"MODEID=ScreenModeID/N  (some video outputs might ignore this one)", NULL },
	{	"PUBSCREEN=",	'k',	(ULONG *) &gfx_screenname,	"PUBSCREEN=ScreenName/K", NULL },
	{	"MONITOR=",		'n',	(ULONG *) &gfx_monitor,		"MONITOR=Monitor nr/N", NULL },

	{NULL,'?',NULL,NULL, NULL}
};


extern struct Library *GraphicsBase;
extern struct Library *LayersBase;

/*********************************** PREINIT *****************************************/
static int preinit(const char *arg)
{
	BOOL have_compositing = FALSE;

	if ( ! LIB_IS_AT_LEAST(GraphicsBase, 54,0) )
	{
		mp_msg(MSGT_VO, MSGL_INFO, "VO: [comp] You need graphics.library version 54.0 or newer.\n");
		return -1;
	}

//	mp_msg(MSGT_VO, MSGL_INFO, "VO: [comp] Welcome man !\n");

	if (!gfx_process_args( (char *) arg, commands))
	{
		mp_msg(MSGT_VO, MSGL_INFO, "VO: [comp] Config is wrong\n");
		return -1;
	}

	mp_msg(MSGT_VO, MSGL_INFO, "VO: [comp] Welcome man !\n");

	if (gfx_screenname)
	{
		DBUG(" *****\nOpen on screen: %s\n*******\n",gfx_screenname);
	}

	if ( ( My_Screen = LockPubScreen ( gfx_screenname ) ) )
	{
		ULONG DisplayID = 0;
		struct DisplayInfo display_info;
		char *ChipDriverName = 0;

		best_screen_aspect_ratio = (float) My_Screen -> Width / (float) My_Screen -> Height;

		GetScreenAttr( My_Screen, SA_DisplayID, &DisplayID , sizeof(DisplayID)  );
		GetDisplayInfoData( NULL, &display_info, sizeof(display_info) , DTAG_DISP, DisplayID);
		GetBoardDataTags( display_info.RTGBoardNum, GBD_ChipDriver, &ChipDriverName, TAG_END);

		mp_msg(MSGT_VO, MSGL_INFO, "VO: [comp] Screen use driver: %s\n",ChipDriverName);

		UnlockPubScreen(NULL, My_Screen);
	}


	//backfillhook = { {NULL, NULL}, (HOOKFUNC)BackFillfunc, NULL, NULL };
	backfillhook = (struct Hook *)AllocSysObjectTags(ASOT_HOOK, ASOHOOK_Entry,BackFillfunc, TAG_END);
DBUG("backfillhook = %p (alloc)\n",backfillhook);


	benchmark_frame_cnt = 0;
	gettimeofday(&before,&dontcare);	// To get time before

	return 0;
}

extern char * filename;

static ULONG Open_Window()
{
	// Window
	ULONG ModeID = INVALID_ID;
	BOOL WindowActivate = TRUE;

	My_Window = NULL;

	if (gfx_screenname)
	{
		DBUG(" *****\nOpen on screen: %s\n*******\n",gfx_screenname);
	}

	if ( ( My_Screen = LockPubScreen ( gfx_screenname ) ) )
	{
		ModeID = GetVPModeID(&My_Screen->ViewPort);

		DBUG("Screen %f\n", (float) My_Screen -> Width / (float) My_Screen -> Height);

#ifdef OLD_DEBUG
		print_monitor_info();
#endif
		if (FirstTime)
		{
			gfx_center_window(My_Screen, window_width, window_height, &win_left, &win_top);
			FirstTime = FALSE;
		}

		switch(gfx_BorderMode)
		{
			case NOBORDER:
				My_Window = OpenWindowTags( NULL,
#ifdef PUBLIC_SCREEN
					WA_PubScreen,	(ULONG) My_Screen,
#else
					WA_CustomScreen,	(ULONG) My_Screen,
#endif
					WA_Left,		win_left,
					WA_Top,		win_top,
					WA_InnerWidth,	window_width,
					WA_InnerHeight,	window_height,
					WA_SimpleRefresh,	TRUE,
					WA_CloseGadget,	FALSE,
					WA_DepthGadget,	FALSE,
					WA_DragBar,		FALSE,
					WA_Borderless,	TRUE,
is_fullscreen? WA_Backdrop : TAG_IGNORE , TRUE,
					WA_SizeGadget,	FALSE,
					WA_NewLookMenus,	TRUE,
					WA_Activate,	WindowActivate,
					WA_StayTop,		(is_ontop==1) ? TRUE : FALSE,
					WA_IDCMP,		IDCMP_COMMON,
					WA_Flags,		WFLG_REPORTMOUSE,
					// WA_SkinInfo,	NULL,
				TAG_DONE);
				break;

			case TINYBORDER:
				My_Window = OpenWindowTags( NULL,
#ifdef PUBLIC_SCREEN
					WA_PubScreen,	(ULONG) My_Screen,
#else
					WA_CustomScreen,	(ULONG) My_Screen,
#endif
					WA_Left,		win_left,
					WA_Top,		win_top,
					WA_InnerWidth,	window_width,
					WA_InnerHeight,	window_height,
					WA_SimpleRefresh,	TRUE,
					WA_CloseGadget,	FALSE,
					WA_DepthGadget,	FALSE,
					WA_DragBar,		FALSE,
					//WA_Borderless,	FALSE,
					WA_SizeGadget,	TRUE,
					WA_NewLookMenus,	TRUE,
					WA_Activate,	WindowActivate,
					WA_StayTop,		(is_ontop==1) ? TRUE : FALSE,
					WA_IDCMP,		IDCMP_COMMON,
					WA_Flags,		WFLG_REPORTMOUSE,
					// WA_SkinInfo,	NULL,
					TAG_DONE);
				break;

			default:
				My_Window = OpenWindowTags( NULL,
#ifdef PUBLIC_SCREEN
					WA_PubScreen,	(ULONG) My_Screen,
#else
					WA_CustomScreen,	(ULONG) My_Screen,
#endif
// rem markus			WA_ScreenTitle,	(ULONG) gfx_common_screenname,
//					WA_ScreenTitle,	(ULONG) GetWindowTitle(),
// #ifdef __morphos__
//					WA_Title,		(ULONG) filename ? MorphOS_GetWindowTitle() : "MPlayer for MorphOS",
// #endif
// #ifdef __amigaos4__
//					WA_Title,		"MPlayer " VERSION " (comp)",
// #endif
					WA_ScreenTitle,	AMIGA_VERSION " (comp)",
					WA_Title,		(ULONG) GetWindowTitle(),
					WA_Left,		win_left,
					WA_Top,		win_top,
					WA_InnerWidth,	window_width,
					WA_InnerHeight,	window_height,
//					WA_MinWidth,	(window_width/3) > 100 ? window_width/3 : 100,
//					WA_MinHeight,	(window_height/3) > 100 ? window_height/3 : 100,
//					WA_MaxWidth,	My_Screen -> Width,
//					WA_MaxHeight,	My_Screen -> Height,
					WA_SimpleRefresh,	TRUE,
					WA_CloseGadget,	TRUE,
					WA_DepthGadget,	TRUE,
					WA_DragBar,		TRUE,
					//WA_Borderless,	(gfx_BorderMode == NOBORDER) ? TRUE : FALSE,
					WA_SizeGadget,	TRUE,
					WA_SizeBBottom,	TRUE,
					WA_Hidden,		FALSE,
					WA_NewLookMenus,	TRUE,
					WA_Activate,	WindowActivate,
					WA_StayTop,		(is_ontop==1) ? TRUE : FALSE,
					WA_IDCMP,		IDCMP_COMMON | IDCMP_GADGETUP,
					WA_Flags,		WFLG_REPORTMOUSE,
					// WA_SkinInfo,	NULL,
				TAG_DONE);
//markus gadget
				if (My_Window)
				{
					open_icon( My_Window, ICONIFYIMAGE, GID_ICONIFY, &iconifyIcon );
					open_icon( My_Window, SETTINGSIMAGE, GID_FULLSCREEN, &fullscreenicon );
//					open_icon( My_Window, PADLOCKIMAGE, GID_PADLOCK, &padlockicon );
					RefreshWindowFrame(My_Window); // or it won't show/render added gadgets
				}
				break;
		}

		vo_screenwidth = My_Screen->Width;
		vo_screenheight = My_Screen->Height;

		vo_dwidth = amiga_image_width;
		vo_dheight = amiga_image_height;

		vo_fs = 0;

		UnlockPubScreen(NULL, My_Screen);
		My_Screen= NULL;
	}

	EmptyPointer = AllocVec(16, MEMF_PUBLIC | MEMF_CLEAR);

	if ( !My_Window || !EmptyPointer)
	{
		mp_msg(MSGT_VO, MSGL_ERR, "Unable to open a window\n");

		if ( gfx_screenname )
		{
			free( gfx_screenname);
			gfx_screenname = NULL;	// open on workbench if no pubscreen found.
		}

		uninit();
		return INVALID_ID;
	}

	offset_x = (gfx_BorderMode == NOBORDER) ? 0 : My_Window->BorderRight;
	offset_y = (gfx_BorderMode == NOBORDER) ? 0 : My_Window->BorderTop;

	if ( (ModeID = GetVPModeID(&My_Window->WScreen->ViewPort) ) == INVALID_ID)
	{
		uninit();
		return INVALID_ID;
	}

	ScreenToFront(My_Window->WScreen);

	gfx_StartWindow(My_Window);
	gfx_ControlBlanker(My_Screen, FALSE);

	return ModeID;
}

static ULONG Open_FullScreen()
{
	// if fullscreen -> let's open our own screen
	// It is not a very clean way to get a good ModeID, but at least it works
	struct Screen *Screen;
	struct DimensionInfo buffer_Dimmension;
	ULONG depth;
	ULONG ModeID;
	ULONG max_width,max_height;

	ULONG left = 0, top = 0;
	ULONG out_width = 0;
	ULONG out_height = 0;

	struct DimensionInfo dimi;

	screen_width=amiga_image_width;
	screen_height=amiga_image_height;

	// DBUG("%s:%ld\n",__FUNCTION__,__LINE__);

	if(WantedModeID)
	{
		ModeID = WantedModeID;
	}
	else
	{
		ModeID = find_best_screenmode( gfx_monitor, 32, best_screen_aspect_ratio,amiga_image_width, amiga_image_height);
	}

	mp_msg(MSGT_VO, MSGL_INFO, "VO: [comp] Full screen.\n");

	My_Screen = NULL;

	if ( ModeID != INVALID_ID )
	{
		if (GetDisplayInfoData( NULL, &dimi, sizeof(dimi) , DTAG_DIMS, ModeID))
		{
			depth = ( FALSE ) ? 16 : dimi.MaxDepth ;
		}

		My_Screen = OpenScreenTags ( NULL,
			SA_DisplayID,     ModeID,
#ifdef PUBLIC_SCREEN
			SA_Type,          PUBLICSCREEN,
			SA_PubName,       CS(MSG_MPlayer_ScreenTitle), // "MPlayer Screen",
#else
			SA_Type,          CUSTOMSCREEN,
#endif
			SA_Title,         CS(MSG_MPlayer_ScreenTitle), // "MPlayer Screen",
			SA_ShowTitle,     FALSE,
			SA_Quiet,         TRUE,
			SA_LikeWorkbench, TRUE,
is_fullscreen? SA_BackFill : TAG_IGNORE , backfillhook,
		TAG_DONE);
	}

	if ( ! My_Screen )
	{
		mp_msg(MSGT_VO, MSGL_ERR, "Unable to open the screen ID:%x\n", (int) ModeID);
		uninit();
		return INVALID_ID;
	}

#ifdef PUBLIC_SCREEN
	PubScreenStatus( My_Screen, 0 );
#endif

	vo_screenwidth = screen_width = My_Screen->Width;
	vo_screenheight = screen_height = My_Screen->Height;

	offset_x = (screen_width - window_width)/2;
	offset_y = (screen_height - window_height)/2;

	/* Calculate the new video size/aspect */
	aspect_save_screenres(My_Screen->Width,My_Screen->Height);

	out_width = amiga_image_width;
	out_height = amiga_image_height;

	aspect(&out_width,&out_height,A_ZOOM);

	// DBUG("%s:%ld OpenWindowTags()  \n",__FUNCTION__,__LINE__);

	left=(My_Screen->Width-out_width)/2;
	top=(My_Screen->Height-out_height)/2;

if(is_fullscreen)
{
	// dirty way of "refreshing" screen so we get black bars
	RectFillColor(&(My_Screen->RastPort), 0, 0, My_Screen->Width, My_Screen->Height, 0xff000000);

	/*bf_Window = OpenWindowTags( NULL,
			WA_PubScreen,	(ULONG) My_Screen,
			WA_Top,		0,
			WA_Left,		0,
			WA_Height,		My_Screen->Height,
			WA_Width,		My_Screen->Width,
			WA_SimpleRefresh,	TRUE,
			WA_CloseGadget,	FALSE,
			WA_DragBar,		FALSE,
			WA_Borderless,	TRUE,
			WA_Backdrop,	TRUE,
			WA_Activate,	FALSE,
			WINDOW_Position, WPOS_CENTERSCREEN,
//rem samir no declaration port			WINDOW_IconifyGadget, port? TRUE: FALSE,
			WINDOW_PopupGadget, TRUE,
			WINDOW_JumpScreensMenu, TRUE,
//rem samir no declaration port			WINDOW_AppPort, port,
//rem samir no declaration layoutobj		WINDOW_ParentGroup, layoutobj,
//rem samir no declaration menustripobj		WINDOW_MenuStrip, menustripobj,
			is_fullscreen? WA_BackFill : TAG_IGNORE , backfillhook,
		TAG_DONE);*/

	/*if (bf_Window)
	{
		// Fill the screen with a black color
		RectFillColor(bf_Window->RPort, 0,0, My_Screen->Width, My_Screen->Height, 0x00000000);
	}*/
}

	My_Window = OpenWindowTags( NULL,
			#ifdef PUBLIC_SCREEN
			WA_PubScreen,     (ULONG) My_Screen,
			#else
			WA_CustomScreen,  (ULONG) My_Screen,
			#endif
			WA_PubScreen,     (ULONG) My_Screen,
			WA_Top,           top,
			WA_Left,          left,
			WA_Height,        out_height,
			WA_Width,         out_width,
			WA_SimpleRefresh, TRUE,
			WA_CloseGadget,   FALSE,
			WA_DragBar,       FALSE,
			WA_Borderless,    TRUE,
			WA_Backdrop,      TRUE,
			WA_Activate,      TRUE,
			WA_NewLookMenus,  TRUE,
			WA_IDCMP, IDCMP_MOUSEMOVE | IDCMP_MENUPICK | IDCMP_MENUVERIFY | IDCMP_MOUSEBUTTONS | IDCMP_RAWKEY |
			          IDCMP_EXTENDEDMOUSE | IDCMP_REFRESHWINDOW | IDCMP_INACTIVEWINDOW | IDCMP_ACTIVEWINDOW,
			WA_Flags,         WFLG_REPORTMOUSE,
		//	WINDOW_Position,  WPOS_CENTERSCREEN,
// rem Samir no declaration port	WINDOW_IconifyGadget, port? TRUE: FALSE,
			//WINDOW_PopupGadget, TRUE,
			//WINDOW_JumpScreensMenu, TRUE,
// rem Samir no declaration port			WINDOW_AppPort, port,
// rem Samir no declaration layoutobj		WINDOW_ParentGroup, layoutobj,
// rem Samir no declaration menustripobj		WINDOW_MenuStrip, menustripobj,
		TAG_DONE);

	if ( !My_Window )
	{
		mp_msg(MSGT_VO, MSGL_ERR, "Unable to open a window\n");
		uninit();
		return INVALID_ID;
	}

	/*SetWindowAttrs(My_Window,
	               WA_Left,left,
	               WA_Top,top,
	               WA_Width,out_width,
	               WA_Height,out_height,
	              TAG_DONE);

	RectFillColor( My_Window->RPort, 0,0, screen_width, screen_height, 0x00000000);*/

#ifdef OLD_DEBUG
	Printf("Screen w %ld h %ld\n",My_Screen->Width,My_Screen->Height);
#endif
	vo_screenwidth = My_Screen->Width;
	vo_screenheight = My_Screen->Height;

	vo_dwidth = amiga_image_width;
	vo_dheight = amiga_image_height;

	vo_fs = 1;

	gfx_ControlBlanker(My_Screen, FALSE);

	return ModeID;
}

static int PrepareBuffer(uint32_t in_format, uint32_t out_format)
{
	swsContext= sws_getContextFromCmdLine(amiga_image_width, amiga_image_height, in_format, amiga_image_width, amiga_image_height, out_format );

	if (!swsContext)
	{
		uninit();
		return -1;
	}

	return 0;
}


/************************************ CONFIG *****************************************/
static int config(uint32_t width, uint32_t height, uint32_t d_width,
		     uint32_t d_height, uint32_t flags, char *title,
		     uint32_t in_format)
{
	ULONG ModeID = INVALID_ID;
	uint32 BMLock;
	uint32 fmt;

	if (My_Window) return 0;

	is_fullscreen = flags & VOFLAG_FULLSCREEN;
	set_gfx_rendering_option();

	is_paused = FALSE;

	amiga_image_format = in_format;

	DBUG("vo_directrendering %d\n",vo_directrendering);

	switch (gfx_common_rgb_format )
	{
		case PIXF_A8R8G8B8: amiga_image_bpp = 4;	break;
		case PIXF_R8G8B8: amiga_image_bpp 	= 3;	break;
		case PIXF_R5G6B5: amiga_image_bpp 	= 2; 	break;
		default:
			amiga_image_bpp = 4;
			gfx_common_rgb_format = PIXF_A8R8G8B8;
	}

	org_amiga_image_width = width;
	org_amiga_image_height =height;

	amiga_image_width = width & -16;
	amiga_image_height = height ;

	amiga_aspect_ratio = (float) d_width /  (float) d_height;

	window_width = d_width ;
	window_height = d_height;

	while ((window_width<300)||(window_height<300))
	{
		window_width *= 2;
		window_height *= 2;
	}

	ModeID = INVALID_ID;

	MutexObtain( window_mx );

	if ( is_fullscreen )
	{
		// DBUG("%s:%ld\n",__FUNCTION__,__LINE__);
		ModeID = Open_FullScreen();
	}

	if (ModeID == INVALID_ID)
	{
		is_fullscreen &= ~VOFLAG_FULLSCREEN;	// We do not have fullscreen
		set_gfx_rendering_option();

		ModeID = Open_Window();
	}

	if (My_Window)
	{
		attach_menu(My_Window);
	}

	if (!My_Window)
	{
		MutexRelease(window_mx);
		uninit();
		return -1;
	}

	if (the_bitmap)
	{
		// DBUG("%s:%ld -- Free BitMap here\n",__FUNCTION__,__LINE__);
		FreeBitMap(the_bitmap);
	}

	the_bitmap = AllocBitMap( amiga_image_width , amiga_image_height,  32 , BMF_DISPLAYABLE, My_Window ->RPort -> BitMap);

	alloc_output_buffer( org_amiga_image_width, org_amiga_image_height );

	if (!output_ri.Memory)
	{
		mp_msg(MSGT_VO, MSGL_WARN, "******* Buffer not allocated :-(, some info bpp=%ld \n ", amiga_image_bpp);
	}

	rp = My_Window->RPort;

	UserMsg = My_Window->UserPort;

	MutexRelease(window_mx);

	switch (gfx_common_rgb_format )
	{
		case PIXF_A8R8G8B8:
			draw_alpha_func = draw_alpha_rgb32;
			fmt = IMGFMT_BGR32;
			break;
		case PIXF_R8G8B8:
			draw_alpha_func = draw_alpha_rgb24;
			fmt = IMGFMT_RGB24;
			break;
		case PIXF_R5G6B5:
			draw_alpha_func = draw_alpha_rgb16;
			fmt = IMGFMT_BGR16;
			break;
		default: fmt = IMGFMT_BGR32; break;
	}

	if (PrepareBuffer(in_format, fmt ) < 0)
	{
		uninit();
		return -1;
	}

	gfx_Start(My_Window);

#ifdef CONFIG_GUI
	if (use_gui)
		guiGetEvent(guiSetWindowPtr, (char *) My_Window);
#endif

	return 0; // -> Ok
}

int pub_frame = 0;

/******************************** DRAW_SLICE *****************************************/
static int draw_slice(uint8_t *image[], int stride[], int w,int h,int x,int y)
{
	uint8_t *dst[3];
	int dstStride[3];
//DBUG("draw_slice()\n",NULL);
	MutexObtain( window_mx );

	if (( amiga_image_width != w )&&(x==0) )
	{
		DBUG("%d,%d,%d,%d,%d\n",amiga_image_width,w,h,x,y);

		if (output_ri.Memory) { FreeVec(output_ri.Memory); output_ri.Memory = NULL; }
		alloc_output_buffer( w, h );
	}

	dstStride[0] = org_amiga_image_width*amiga_image_bpp;
	dstStride[1] = 0;
	dstStride[2] = 0;

	dst[0] = (uint8_t *) ( (ULONG) output_ri.Memory +x*amiga_image_bpp) ;
	dst[1] = NULL;
	dst[2] = NULL;

	sws_scale(swsContext,
			image,
			stride,
			y,
			h,
			dst,
			dstStride);

	MutexRelease(window_mx);

	return 0;
}
/********************************* DRAW_OSD ******************************************/

static void draw_osd(void)
{
	vo_draw_text(amiga_image_width,amiga_image_height,draw_alpha_func);
}

/******************************** FLIP_PAGE ******************************************/
static void flip_page(void)
{
	struct RastPort rp;
	int ww,wh;

	benchmark_frame_cnt ++;

	if ((My_Window)&&(the_bitmap))
	{
		ww = My_Window->Width - My_Window->BorderLeft - My_Window->BorderRight;
		wh = My_Window->Height - My_Window->BorderTop - My_Window->BorderBottom;

		if ((ww==amiga_image_width)  &&  (wh==amiga_image_height))	// no scaling so we dump it into the window.
		{
			if (gfx_novsync==0) WaitTOF();
			WritePixelArray( output_ri.Memory, 0, 0, output_ri.BytesPerRow, PIXF_A8R8G8B8,
					My_Window -> RPort, My_Window->BorderLeft,My_Window->BorderTop, amiga_image_width,amiga_image_height);
		}
		else
		{
			InitRastPort(&rp);
			rp.BitMap = the_bitmap;

			WritePixelArray( output_ri.Memory, 0, 0, output_ri.BytesPerRow, PIXF_A8R8G8B8,
					&rp, 0,0, amiga_image_width,amiga_image_height);

			if (gfx_novsync==0) WaitTOF();

			// can't use this any more when we have menus!! :-(
			if (can_go_faster)	// do it fast
			{
				draw_comp( the_bitmap,My_Window, amiga_image_width,amiga_image_height);
			}
			else				// Try not to trash graphics.
			{
				draw_inside_window();
			}
		}
	}
}
/******************************** DRAW_FRAME *****************************************/
static int draw_frame(uint8_t *src[])
{
	// Nothing
	return -1;
}


/*************************/
// Just a little func to help uninit and control
// it close screen (if fullscreen), the windows, and free all gfx related stuff
// but do not close any libs

static void FreeGfx(void)
{
	gfx_ControlBlanker(My_Screen, TRUE);
	gfx_Stop(My_Window);

	MutexObtain( window_mx );

	if (My_Window)
	{
		gfx_StopWindow(My_Window);
		if (rp) InstallLayerHook(rp->Layer, NULL);
		detach_menu(My_Window);
//markus
		dispose_icon(My_Window, &iconifyIcon );
		dispose_icon(My_Window, &fullscreenicon );
//
		CloseWindow(My_Window);
		My_Window=NULL;
	}

	/*if (bf_Window)
	{
		CloseWindow(bf_Window);
		bf_Window=NULL;
	}*/

	closeRemainingOpenWin();

	if (My_Screen)
	{
		CloseScreen(My_Screen);
		My_Screen = NULL;
	}

	rp = NULL;

	if (the_bitmap) { FreeBitMap(the_bitmap); 	the_bitmap = NULL; }
	if (output_ri.Memory)
	{
		FreeVec(output_ri.Memory);	output_ri.Memory = NULL;
	}

	if (inner_bitmap) { FreeBitMap(inner_bitmap);  inner_bitmap = NULL; }

	MutexRelease(window_mx);
}

/*********************************** UNINIT ******************************************/
static void uninit(void)
{
	unsigned long long int microsec;

	if (benchmark_frame_cnt>0)
	{
		gettimeofday(&after,&dontcare);		// To get time after
		microsec = (after.tv_usec - before.tv_usec) +(1000000 * (after.tv_sec - before.tv_sec));
		printf("Internal COMP FPS %0.00f\n",(float)  benchmark_frame_cnt / (float) (microsec / 1000 / 1000) );
	}
	benchmark_frame_cnt = 0;

DBUG("%s:%ld - rp is 0x%08lx \n",__FUNCTION__,__LINE__,rp);
	FreeGfx();
DBUG("%s:%ld - rp is 0x%08lx \n",__FUNCTION__,__LINE__,rp);


DBUG("backfillhook = %p (free)\n",backfillhook);
FreeSysObject(ASOT_HOOK, backfillhook);
backfillhook = NULL;


	if (EmptyPointer)
	{
		FreeVec(EmptyPointer);
		EmptyPointer=NULL;
	}
DBUG("%s:%ld - rp is 0x%08lx \n",__FUNCTION__,__LINE__,rp);
	gfx_ReleaseArg();
DBUG("%s:%ld - rp is 0x%08lx \n",__FUNCTION__,__LINE__,rp);
}

/********************************** CONTROL ******************************************/
static int control(uint32_t request, void *data)
{
	switch (request)
	{
		case VOCTRL_GUISUPPORT:
			return VO_TRUE;

		case VOCTRL_FULLSCREEN:
			is_fullscreen ^= VOFLAG_FULLSCREEN;
			set_gfx_rendering_option();

			FreeGfx();
			if ( config(org_amiga_image_width, org_amiga_image_height, window_width, window_height, is_fullscreen, NULL, amiga_image_format) < 0) return VO_FALSE;

			return VO_TRUE;

		case VOCTRL_ONTOP:
			vo_ontop = !vo_ontop;
			is_ontop = !is_ontop;
			FreeGfx(); // Free/Close all gfx stuff (screen windows, buffer...);
			if ( config(amiga_image_width, amiga_image_height, window_width, window_height, FALSE, NULL, amiga_image_format) < 0) return VO_FALSE;

			return VO_TRUE;

		case VOCTRL_PAUSE:
			gfx_Stop(My_Window);
			if (rp) InstallLayerHook(rp->Layer, &BackFill_Hook);

			gfx_ControlBlanker(My_Screen, TRUE);

			is_paused = is_paused ? FALSE : TRUE;

			BackFill_Func( My_Window -> RPort, NULL );
			return VO_TRUE;

		case VOCTRL_RESUME:
			gfx_Start(My_Window);

			if (rp) InstallLayerHook(rp->Layer, NULL );

			gfx_ControlBlanker(My_Screen, FALSE);

			is_paused = is_paused ? FALSE : TRUE;

			BackFill_Func( My_Window -> RPort, NULL );

			return VO_TRUE;

		case VOCTRL_QUERY_FORMAT:
			return query_format(*(ULONG *)data);

//		case VOCTRL_GET_IMAGE:
//			return get_image(data);
	}

	return VO_NOTIMPL;
}

/********************************** CHECK_EVENTS *************************************/
static void check_events(void)
{
	gfx_CheckEvents(My_Screen, My_Window, &window_height, &window_width, &win_left, &win_top);
}

static int query_format(uint32_t format)
{
	switch( format)
	{
		case IMGFMT_YV12:
		case IMGFMT_I420:
		case IMGFMT_IYUV:
			return VO_TRUE;
		default:
			return VO_FALSE;
	}
}

