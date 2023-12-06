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
 *  vo_p96_pip.c
 *  VO module for MPlayer AmigaOS4
 *  Using p96 PIP
 *  Written by Jörg Strohmayer
 *  Modified by Kjetil Hvalstrand
*/

#define USE_VMEM64	1


//#include <reaction/reaction.h>
//#include <reaction/reaction_macros.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "config.h"
#include "version.h"

#include "../sub/osd.h"
#include "../sub/sub.h"

#include "mp_msg.h"
#include "video_out.h"
#include "video_out_internal.h"

#include "../mplayer.h"
#include "../libmpcodecs/vf_scale.h"
#include "../input/input.h"

// OS specific
#include <libraries/keymap.h>
#include <osdep/keycodes.h>

#include <proto/graphics.h>
#include <proto/intuition.h>
#include <proto/exec.h>
#include <proto/dos.h>

#include <proto/icon.h>
#include <proto/requester.h>
#include <proto/layout.h>
#include <proto/utility.h>

#include <proto/Picasso96API.h>
#include <exec/types.h>

#include <graphics/rpattr.h>
#include <dos/dostags.h>
#include <dos/dos.h>

extern struct Catalog *catalog;
#define MPlayer_NUMBERS
#define MPlayer_STRINGS
#include "../amigaos/MPlayer_catalog.h"
extern STRPTR myGetCatalogStr (struct Catalog *catalog, LONG num, STRPTR def);
#define CS(id) myGetCatalogStr(catalog,id,id##_STR)


#include "aspect.h"
#include "input/input.h"
#include "../libmpdemux/demuxer.h"

#include <assert.h>
#undef CONFIG_GUI

#include "cgx_common.h"

// #define PUBLIC_SCREEN 0

static vo_info_t info =
{
	"Picasso96 overlay",
	"p96_pip",
	"Jörg strohmayer",
	"AmigaOS4 rules da world !"
};


LIBVO_EXTERN(p96_pip)

#define MIN_WIDTH 290
#define MIN_HEIGHT 217

// markus iconify ?
#include <classes/window.h>
#include <intuition/imageclass.h>
#include <../amigaos/window_icons.h>
#include <../amigaos/debug.h>

extern struct kIcon iconifyIcon;
// extern struct kIcon padlockicon;
extern struct kIcon fullscreenicon;
// end markus

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
      window_title = strdup("MPlayer " AMIGA_VERSION " (p96pip)");
   }

   return window_title;
}
// end markus

// Some proto for our private func
static void (*draw_alpha_fnc)(int x0,int y0, int w,int h, unsigned char* src, unsigned char *srca, int stride);

#if 0 /* MickJT - I prefer other function */
static vo_draw_alpha_func draw_alpha_func;
#endif

extern BOOL gfx_CheckEvents(struct Screen *the_screen, struct Window *My_Window, uint32_t *window_height, uint32_t *window_width,uint32_t *window_left, uint32_t *window_top );

static void FreeGfx(void);

static struct Screen *the_screen	= NULL;
static struct Window *My_Window	= NULL;
//static struct Window *bf_Window	= NULL;
static struct Hook *backfillhook;

extern uint32	amiga_image_width;
extern uint32	amiga_image_height;
extern float	amiga_aspect_ratio;

static uint32_t	window_width	= 0;
static uint32_t	window_height	= 0;
static uint32_t	old_d_width		= 0;
static uint32_t	old_d_height	= 0;

static uint32_t	win_top		= 0;
static uint32_t	win_left		= 0;
static uint32_t	old_win_top		= 0;
static uint32_t	old_win_left	= 0;

static uint32_t	screen_width;	// Indicates the size of the screen in full screen
static uint32_t	screen_height;	// Only use for triple buffering

static BOOL FirstTime = TRUE;

extern uint32_t is_fullscreen;

// Found in cgx_common
extern UWORD *EmptyPointer;
extern BOOL mouse_hidden;


ULONG g_in_format;
ULONG g_out_format;
BOOL Stopped = FALSE;
BOOL keep_width = FALSE;
BOOL keep_height = FALSE;
extern BOOL is_ontop;
static BOOL is_my_screen;

uint32_t vsync_is_enabled;
extern int benchmark;

static float best_screen_aspect_ratio;

ULONG amiga_image_format;
// Float ratio;

struct RastPort *rp;


extern APTR window_mx;


static int numbuffers=3;

/**************************** DRAW ALPHA ****************************/

/**
 *
 * Draw_alpha is used for OSD and subtitle display
 *
 **/

#if 0 /* MickJT - I prefer other function */
static void draw_alpha_yv12(int x0, int y0, int w, int h, unsigned char* src, unsigned char *srca, int stride)
{
	register int x;
	register UWORD *dstbase;
	register UWORD *dst;
	struct BitMap *bm;
	struct RenderInfo yuvInfo;
	int lock;
	//uint32 bpr;
	//APTR *ba;

	if (!w || !h) return;

	p96PIP_GetTags(My_Window, P96PIP_SourceBitMap,(ULONG)&bm, TAG_DONE );

//	lock = LockBitMapTags(ram_bitmap[n], LBM_PlanarYUVInfo,&yuvInfo, TAG_END);		// Like it is comp_yuv2

	lock = LockBitMapTags( bm,
	                      LBM_BaseAddress, &yuvInfo.Memory,
	                      LBM_BytesPerRow, &yuvInfo.BytesPerRow,
	                     TAG_END);
	if (lock == NULL)
	{
		// Unable to lock the BitMap -> do nothing
		return;
	}

	yuvInfo.BytesPerRow = (w + 7) & -8 ;	// LockBitMapTags() returns broken bpr.

	dstbase = (UWORD *)(yuvInfo.Memory + ( ( (y0 * yuvInfo.BytesPerRow) + x0) * 2));

	do
	{
		x = 0;
		do
		{
			dst = dstbase;
			if (srca[x])
			{
				__asm__ __volatile__(
					"li %%r4,0x0080;"
					"rlwimi %%r4,%1,8,16,23;"
					"sth %%r4,0(%0);"
					:
					: "b" ( dst + x), "r" (src[x])
					: "r4"
				);
			}
		} while (++x < w);

		src     += stride;
		srca    += stride;
		dstbase += amiga_image_width;
	} while (--h);

	UnlockBitMap(lock);
}
#else
static void draw_alpha_yv12(int x0, int y0, int w, int h, unsigned char* src, unsigned char *srca, int stride)
{
        register int x;
        register UWORD *dstbase;
        register UWORD *dst;
        struct BitMap *bm;
    struct RenderInfo ri;
    int lock_h;

        if (!w || !h)
                return;

        p96PIP_GetTags(My_Window, P96PIP_SourceBitMap,(ULONG)&bm, TAG_DONE );

    if (! (lock_h = p96LockBitMap(bm, (uint8 *)&ri, sizeof(struct RenderInfo))))
    {
        // Unable to lock the BitMap -> do nothing
        return;
    }

    dstbase = (UWORD *)(ri.Memory + ( ( (y0 * amiga_image_width) + x0) * 2));

    do
    {
        x = 0;
        do
        {
            dst = dstbase;
            if (srca[x])
            {
                __asm__ __volatile__(
                    "li %%r4,0x0080;"
                    "rlwimi %%r4,%1,8,16,23;"
                    "sth %%r4,0(%0);"
                    :
                    : "b" ( dst + x), "r" (src[x])
                    : "r4"
                );
            }
        } while (++x < w);

        src     += stride;
        srca    += stride;
        dstbase += amiga_image_width;

    } while (--h);

        p96UnlockBitMap(bm, lock_h);
}
#endif


int pip_in_use()
{
	char ret[100];
	sprintf(ret,"0");

	GetVar("p96pip_in_use", ret , 100, GVF_GLOBAL_ONLY );
	if (ret[0] == '1')
	{
		return 1;
	}

	return 0;
}


/***************************** PREINIT ******************************/
static int preinit(const char *arg)
{
	BOOL have_radeonhd = FALSE;

//	mp_msg(MSGT_VO, MSGL_INFO, "VO: [p96_pip] Welcome man !\n");

	if (!gfx_GiveArg(arg))
	{
		mp_msg(MSGT_VO, MSGL_INFO, "VO: [p96_pip] Config is wrong\n");
		return -1;
	}

	if ( ( the_screen = LockPubScreen(gfx_screenname) ) )
	{
		/*ULONG DisplayID = 0;
		struct DisplayInfo display_info;
		char *ChipDriverName = 0;

		is_my_screen = FALSE;

		GetScreenAttr( the_screen, SA_DisplayID, &DisplayID , sizeof(DisplayID)  );
		GetDisplayInfoData( NULL, &display_info, sizeof(display_info), DTAG_DISP, DisplayID);
		GetBoardDataTags( display_info.RTGBoardNum, GBD_ChipDriver, &ChipDriverName, TAG_END);

                if (strcasecmp(ChipDriverName,"RadeonHD.chip") == 0 || strcasecmp(ChipDriverName,"RadeonRX.chip") == 0 || strcasecmp(ChipDriverName,"SiliconMotion502.chip") == 0)
		{
			have_radeonhd = TRUE;
		}*/
		have_radeonhd = isRadeonHDorSM502_chip(the_screen);

		best_screen_aspect_ratio = (float) the_screen -> Width / (float) the_screen -> Height;

		UnlockPubScreen(NULL, the_screen);
	}

	if (have_radeonhd == TRUE)
	{
		mp_msg(MSGT_VO, MSGL_INFO, "VO: [p96_pip] Is not supported by Radeon HD/RX and SM502 gfx cards\n");
		return -1;
	}

	// Only one MPlayer window can be opened with p96

	if ( pip_in_use() )
	{
		mp_msg(MSGT_VO, MSGL_INFO, "VO: [p96_pip] Only one PIP window can be opened\n");
		return -1;
	}

	mp_msg(MSGT_VO, MSGL_INFO, "VO: [p96_pip] Welcome man !\n");


	//backfillhook = { {NULL, NULL}, (HOOKFUNC)BackFillfunc, NULL, NULL };
	backfillhook = (struct Hook *)AllocSysObjectTags(ASOT_HOOK, ASOHOOK_Entry,BackFillfunc, TAG_END);
DBUG("backfillhook = %p (alloc)\n",backfillhook);


	// Fix vsync

	vsync_is_enabled = ((benchmark == 0) && (gfx_novsync == 0)) ? 1 : 0;
	vo_vsync = vsync_is_enabled;

	printf("Vsync is enabled = %d\n", (int) vsync_is_enabled);

	return 0;
}


static ULONG Open_PIPWindow(void)
{
    // Window
    ULONG ModeID = INVALID_ID;

    My_Window = NULL;

    if ( ( the_screen = LockPubScreen(gfx_screenname) ) )
    {
		is_my_screen = FALSE;

		if (FirstTime)
		{
			gfx_center_window(the_screen, window_width, window_height, &win_left, &win_top);
			FirstTime = FALSE;
		}

		My_Window = p96PIP_OpenTags(
			P96PIP_SourceFormat,    RGBFB_YUV422CGX,
			P96PIP_SourceWidth,     amiga_image_width,//  - (amiga_image_width%4),  //jabierdlr
			P96PIP_SourceHeight,    amiga_image_height,// - (amiga_image_height%4), //jabierdlr
			P96PIP_AllowCropping,   TRUE,
			P96PIP_NumberOfBuffers, numbuffers,
			P96PIP_Relativity,      PIPRel_Width | PIPRel_Height,

#ifdef CONFIG_GUI
			P96PIP_Width,  (ULONG) -1,
			P96PIP_Height, (ULONG)-34,
#endif
			P96PIP_ColorKeyPen, 0,
//P96PIP_ColorKeyARGB, 0,

#ifdef PUBLIC_SCREEN
			WA_PubScreen, (ULONG) the_screen,
#else
			WA_CustomScreen, (ULONG) the_screen,
#endif
			// rem markus WA_ScreenTitle,	(ULONG) gfx_common_screenname,
			// old
			// WA_ScreenTitle,	(ULONG) GetWindowTitle(),
			// WA_Title,		"MPlayer " VERSION " (p96_pip)",
			// end old
			WA_ScreenTitle,   AMIGA_VERSION " (p96_pip)",
			WA_Title,         (ULONG)GetWindowTitle(),
			WA_Left,          win_left,
			WA_Top,           win_top,
			WA_InnerWidth,    window_width,
			WA_InnerHeight,   window_height,
			WA_MaxWidth,      ~0,
			WA_MaxHeight,     ~0,
			WA_MinWidth,      MIN_WIDTH,
			WA_MinHeight,     MIN_HEIGHT,
			WA_NewLookMenus,  TRUE,
			WA_SimpleRefresh, TRUE,
			WA_NoCareRefresh, TRUE,
			WA_CloseGadget,   TRUE,
			WA_DepthGadget,   TRUE,
			WA_DragBar,       TRUE,
			WA_Borderless,    FALSE,
			WA_SizeGadget,    TRUE,
			WA_SizeBBottom,   TRUE,
			WA_Activate,      TRUE,
			WA_Hidden,        FALSE, // This avoid a bug into WA_StayTop when used with WA_Hidden
			//WA_PubScreen,		the_screen,
			WA_StayTop,       (is_ontop==1) ? TRUE : FALSE,
			WA_IDCMP,         IDCMP_COMMON | IDCMP_GADGETUP,
			TAG_DONE);

// markus gadget
	if (My_Window)
	{
	 	open_icon( My_Window, ICONIFYIMAGE, GID_ICONIFY, &iconifyIcon );
	 	open_icon( My_Window, SETTINGSIMAGE, GID_FULLSCREEN, &fullscreenicon );
//	 	open_icon( My_Window, PADLOCKIMAGE, GID_PADLOCK, &padlockicon );
	 	RefreshWindowFrame(My_Window); // or it won't show/render added gadgets
/*	}

		if (My_Window)
		{*/
			/* Clearing Pointer */
			ClearPointer(My_Window);

		/* Fill the window with a black color */
		//RectFillColor(My_Window->RPort, win_left,win_top, window_width, window_height, 0xFF000000);
		}

		vo_screenwidth  = the_screen->Width;
		vo_screenheight = the_screen->Height;

		UnlockPubScreen(NULL, the_screen);
    }

    if ( !My_Window )
    {
        mp_msg(MSGT_VO, MSGL_ERR, "[p96_pip] Unable to open a window\n");
        return INVALID_ID;
    }

    if ( (ModeID = GetVPModeID(&My_Window->WScreen->ViewPort) ) == INVALID_ID)
    {
        return INVALID_ID;
    }

	mouse_hidden = FALSE;
	ClearPointer(My_Window);

	vo_dwidth  = amiga_image_width;
	vo_dheight = amiga_image_height;

	is_fullscreen = 0;

	ScreenToFront(My_Window->WScreen);

	return ModeID;
}

static ULONG GoFullScreen(void)
{
	ULONG left = 0, top = 0;
	ULONG ModeID = INVALID_ID;
	ULONG out_width = 0;
	ULONG out_height = 0;

    if (My_Window)
    {
		p96PIP_Close(My_Window);
		My_Window = NULL;
	}

	the_screen = OpenScreenTags ( NULL,
						SA_LikeWorkbench, TRUE,
						SA_Type,          PUBLICSCREEN,
						SA_PubName,       CS(MSG_MPlayer_ScreenTitle), // "MPlayer Screen",
						SA_Title,         CS(MSG_MPlayer_ScreenTitle), // "MPlayer Screen",
						SA_ShowTitle,     FALSE,
						SA_Compositing,   FALSE,	// No white stripes anymore, yeah! (fix from kas1e/Jörg Strohmayer)
						SA_Quiet,         TRUE,
is_fullscreen? SA_BackFill : TAG_IGNORE , backfillhook,
						TAG_DONE);

	if ( ! the_screen )
	{
		return INVALID_ID;
	}

	is_my_screen = TRUE;

if(is_fullscreen)
{
	// dirty way of "refreshing" screen so we get black bars
	RectFillColor(&(the_screen->RastPort), 0, 0, the_screen->Width, the_screen->Height, 0xff000000);

	/*bf_Window = OpenWindowTags( NULL,
			WA_PubScreen,    (ULONG)the_screen,
			WA_Top,          0,
			WA_Left,         0,
			WA_Height,       vo_screenheight,
			WA_Width,        vo_screenwidth,
			WA_CloseGadget,  FALSE,
			WA_SmartRefresh, TRUE,
			WA_DragBar,      TRUE,
			WA_Borderless,   TRUE,
			WA_Backdrop,     TRUE,
			WA_Activate,     FALSE,
		TAG_DONE);

	if (bf_Window)
	{
		// Fill the screen with a black color
		RectFillColor(bf_Window->RPort, 0,0, the_screen->Width, the_screen->Height, 0xFF000000);
	}*/
}

	/* Calculate the new video size/aspect */
	aspect_save_screenres(the_screen->Width,the_screen->Height);

	out_width  = amiga_image_width;
	out_height = amiga_image_height;

	aspect(&out_width,&out_height,A_ZOOM);

	left=(the_screen->Width-out_width)/2;
	top=(the_screen->Height-out_height)/2;

	My_Window = p96PIP_OpenTags(
				P96PIP_SourceFormat,    RGBFB_YUV422CGX,
				P96PIP_SourceWidth,     amiga_image_width,
				P96PIP_SourceHeight,    amiga_image_height,
				P96PIP_NumberOfBuffers, numbuffers,

#ifdef PUBLIC_SCREEN
                      	WA_PubScreen, (ULONG) the_screen,
#else
                      	WA_CustomScreen, (ULONG) the_screen,
#endif
                      	WA_ScreenTitle,  AMIGA_VERSION " (p96_pip)",
                      	WA_SmartRefresh, TRUE,
                      	WA_CloseGadget,  FALSE,
                      	WA_DepthGadget,  FALSE,
                      	WA_DragBar,      FALSE,
                      	WA_Borderless,   TRUE,
WA_Backdrop, TRUE,
                      	WA_NewLookMenus, TRUE,
                      	WA_SizeGadget,   FALSE,
                      	WA_Activate,     TRUE,
//                      	WA_StayTop,      TRUE,
                      	WA_DropShadows,  FALSE,
                      	WA_IDCMP,        IDCMP_COMMON,
                      TAG_DONE);

	if ( !My_Window )
	{
		return INVALID_ID;
	}

	if ( (ModeID = GetVPModeID(&My_Window->WScreen->ViewPort) ) == INVALID_ID)
   	{
		return INVALID_ID;
   	}

	SetWindowAttrs(My_Window,
	               WA_Left,   left,
	               WA_Top,    top,
	               WA_Width,  out_width,
	               WA_Height, out_height,
	              TAG_DONE);

//	RectFillColor(My_Window->RPort, 0,0, the_screen->Width, the_screen->Height, 0xFF000000);
//	RectFillColor(bf_Window->RPort, 0,0, the_screen->Width, the_screen->Height, 0xFF000000);

	vo_screenwidth  = the_screen->Width;
	vo_screenheight = the_screen->Height;
	vo_dwidth  = window_width;
	vo_dheight = window_height;
	vo_fs = 1;

	gfx_ControlBlanker(the_screen, FALSE);

	ScreenToFront(the_screen);

	return ModeID;
}

static ULONG Open_FullScreen(void)
{

    ULONG ModeID = INVALID_ID;

    ModeID = GoFullScreen();
    return ModeID;
}

/****************************/
static int PrepareVideo(uint32_t in_format, uint32_t out_format)
{
    g_in_format  = in_format;
    g_out_format = out_format;

    return 0;
}


/***************************** CONFIG *******************************/
static int config(uint32_t width, uint32_t height, uint32_t d_width,
             uint32_t d_height, uint32_t flags, char *title,
             uint32_t format)
{
    uint32_t out_format;
    ULONG ModeID = INVALID_ID;

	if (Stopped==TRUE || My_Window)
		FreeGfx();

	amiga_aspect_ratio = (float) d_width /  (float) d_height;

	if (d_width<MIN_WIDTH || d_height<MIN_HEIGHT)
	{
		d_width  = MIN_WIDTH * amiga_aspect_ratio;
		d_height = MIN_HEIGHT * amiga_aspect_ratio;
	}
	// else
	// d_height = d_width / amiga_aspect_ratio;

	if (old_d_width==0 || old_d_height==0)
	{
		old_d_width  = d_width;
		old_d_height = d_height;
	}

	if (keep_width==0 || keep_height==0)
	{
		keep_width  = d_width;
		keep_height = d_height;
	}

#if 0 // MickJT
	amiga_image_width  = width;
	amiga_image_height = height;
	amiga_image_format = format;
#endif

    is_fullscreen = flags & VOFLAG_FULLSCREEN;

#if 0 // MickJT
	window_width  = d_width;
	window_height = d_height;
#endif

#if 1 // MickJT
amiga_image_width  = (width - (width % 16));
amiga_image_height = (height - (height % 16));
amiga_image_format = format;
window_width  = (d_width - (d_width % 16));
window_height = (d_height - (d_height % 16));
#endif

	EmptyPointer = AllocVec(12, MEMF_PUBLIC | MEMF_CLEAR);

	MutexObtain( window_mx );

	if ( is_fullscreen )
	{
		ModeID = Open_FullScreen();
		//if (INVALID_ID == ModeID) ModeID = Open_PIPWindow();
	}
	/*else
	{
		ModeID = Open_PIPWindow();
	}*/
	if (INVALID_ID == ModeID) ModeID = Open_PIPWindow();

	MutexRelease(window_mx);

	draw_alpha_fnc = draw_alpha_yv12;
#if 0 /* MickJT - I prefer other function */
	draw_alpha_func = draw_alpha_yv12;
#endif

	out_format = IMGFMT_YV12;

	if (My_Window)
	{
		SetVar("p96pip_in_use","1",1,GVF_GLOBAL_ONLY);
		attach_menu(My_Window);
		rp = My_Window->RPort;
	}
	else
	{
		SetVar("p96pip_in_use","0",1,GVF_GLOBAL_ONLY);
		return 1;
	}

	if (INVALID_ID == ModeID)
	{
		Printf("INVALID_ID == ModeID");
		return 1;
   	}

	if (PrepareVideo(format, out_format) < 0)
	{
		return 1;
	}

	Stopped = FALSE;
    return 0; // -> Ok
}

#define PLANAR_2_CHUNKY(Py, Py2, Pu, Pv, dst, dst2)	\
	{								\
	register ULONG Y = *++Py;				\
	register ULONG Y2= *++Py;				\
	register ULONG U = *++Pu;				\
	register ULONG V = *++Pv;				\
									\
	__asm__ __volatile__ (					\
		"rlwinm %%r3,%1,0,0,7;"				\
		"rlwinm %%r4,%1,16,0,7;"			\
		"rlwimi %%r3,%2,24,8,15;"			\
		"rlwimi %%r4,%2,0,8,15;"			\
		"rlwimi %%r3,%1,24,16,23;"			\
		"rlwimi %%r4,%1,8,16,23;"			\
		"rlwimi %%r3,%3,8,24,31;"			\
		"rlwimi %%r4,%3,16,24,31;"			\
									\
		"stw %%r3,4(%0);"					\
		"stwu %%r4,8(%0);"				\
									\
	    : "+b" (dst)						\
	    : "r" (Y), "r" (U), "r" (V)			\
	    : "r3", "r4");					\
									\
	Y = *++Py2;							\
	__asm__ __volatile__ (					\
		"rlwinm %%r3,%1,0,0,7;"				\
		"rlwinm %%r4,%1,16,0,7;"			\
		"rlwimi %%r3,%2,8,8,15;"			\
		"rlwimi %%r4,%2,16,8,15;"			\
		"rlwimi %%r3,%1,24,16,23;"			\
		"rlwimi %%r4,%1,8,16,23;"			\
		"rlwimi %%r3,%3,24,24,31;"			\
		"rlwimi %%r4,%3,0,24,31;"			\
									\
		"stw %%r3,4(%0);"					\
		"stwu %%r4,8(%0);"				\
									\
	    : "+b" (dst)						\
	    : "r" (Y2), "r" (U), "r" (V)			\
	    : "r3", "r4");					\
									\
	Y2= *++Py2;							\
	__asm__ __volatile__ (					\
		"rlwinm %%r3,%1,0,0,7;"				\
		"rlwinm %%r4,%1,16,0,7;"			\
		"rlwimi %%r3,%2,24,8,15;"			\
		"rlwimi %%r4,%2,0,8,15;"			\
		"rlwimi %%r3,%1,24,16,23;"			\
		"rlwimi %%r4,%1,8,16,23;"			\
		"rlwimi %%r3,%3,8,24,31;"			\
		"rlwimi %%r4,%3,16,24,31;"			\
									\
		"stw %%r3,4(%0);"					\
		"stwu %%r4,8(%0);"				\
									\
	    : "+b" (dst2)						\
	    : "r" (Y), "r" (U), "r" (V)			\
	    : "r3", "r4");					\
									\
	__asm__ __volatile__ (					\
		"rlwinm %%r3,%1,0,0,7;"				\
		"rlwinm %%r4,%1,16,0,7;"			\
		"rlwimi %%r3,%2,8,8,15;"			\
		"rlwimi %%r4,%2,16,8,15;"			\
		"rlwimi %%r3,%1,24,16,23;"			\
		"rlwimi %%r4,%1,8,16,23;"			\
		"rlwimi %%r3,%3,24,24,31;"			\
		"rlwimi %%r4,%3,0,24,31;"			\
									\
		"stw %%r3,4(%0);"					\
		"stwu %%r4,8(%0);"				\
									\
	    : "+b" (dst2)						\
	    : "r" (Y2), "r" (U), "r" (V)			\
	    : "r3", "r4");					\
	}

#if USE_VMEM64
#define PLANAR_2_CHUNKY_64(Py, Py2, Pu, Pv, dst, dst2, tmp)	\
	{									\
	register ULONG Y = *++Py;					\
	register ULONG Y2= *++Py;					\
	register ULONG U = *++Pu;					\
	register ULONG V = *++Pv;					\
										\
	__asm__ __volatile__ (						\
		"rlwinm %%r3,%1,0,0,7;"					\
		"rlwinm %%r4,%1,16,0,7;"   				\
		"rlwimi %%r3,%2,24,8,15;"				\
		"rlwimi %%r4,%2,0,8,15;"				\
		"rlwimi %%r3,%1,24,16,23;"				\
		"rlwimi %%r4,%1,8,16,23;"				\
		"rlwimi %%r3,%3,8,24,31;"				\
		"rlwimi %%r4,%3,16,24,31;"				\
										\
		"stw %%r3,0(%4);"						\
		"stw %%r4,4(%4);"						\
		"lfd %%f1,0(%4);"						\
		"stfdu %%f1,8(%0);"					\
										\
	    : "+b" (dst)							\
	    : "r" (Y), "r" (U), "r" (V), "r" (tmp) 		\
	    : "r3", "r4", "fr1");					\
										\
	Y = *++Py2;								\
	__asm__ __volatile__ (						\
		"rlwinm %%r3,%1,0,0,7;"					\
		"rlwinm %%r4,%1,16,0,7;"				\
		"rlwimi %%r3,%2,8,8,15;"				\
		"rlwimi %%r4,%2,16,8,15;"				\
		"rlwimi %%r3,%1,24,16,23;"				\
		"rlwimi %%r4,%1,8,16,23;"				\
		"rlwimi %%r3,%3,24,24,31;"				\
		"rlwimi %%r4,%3,0,24,31;"				\
										\
		"stw %%r3,0(%4);"						\
		"stw %%r4,4(%4);"						\
		"lfd %%f1,0(%4);"						\
		"stfdu %%f1,8(%0);"					\
										\
	    : "+b" (dst)							\
	    : "r" (Y2), "r" (U), "r" (V), "r" (tmp)		\
	    : "r3", "r4", "fr1");					\
										\
	Y2= *++Py2;								\
	__asm__ __volatile__ (						\
		"rlwinm %%r3,%1,0,0,7;" 				\
		"rlwinm %%r4,%1,16,0,7;"   				\
		"rlwimi %%r3,%2,24,8,15;"				\
		"rlwimi %%r4,%2,0,8,15;"				\
		"rlwimi %%r3,%1,24,16,23;"				\
		"rlwimi %%r4,%1,8,16,23;"				\
		"rlwimi %%r3,%3,8,24,31;"				\
		"rlwimi %%r4,%3,16,24,31;"				\
										\
		"stw %%r3,0(%4);"						\
		"stw %%r4,4(%4);"						\
		"lfd %%f1,0(%4);"						\
		"stfdu %%f1,8(%0);"					\
										\
	    : "+b" (dst2)							\
	    : "r" (Y), "r" (U), "r" (V), "r" (tmp) 		\
	    : "r3", "r4", "fr1");					\
										\
	__asm__ __volatile__ (						\
		"rlwinm %%r3,%1,0,0,7;"					\
		"rlwinm %%r4,%1,16,0,7;"				\
		"rlwimi %%r3,%2,8,8,15;"				\
		"rlwimi %%r4,%2,16,8,15;"				\
		"rlwimi %%r3,%1,24,16,23;"				\
		"rlwimi %%r4,%1,8,16,23;"				\
		"rlwimi %%r3,%3,24,24,31;"				\
		"rlwimi %%r4,%3,0,24,31;"				\
										\
		"stw %%r3,0(%4);"						\
		"stw %%r4,4(%4);"						\
		"lfd %%f1,0(%4);"						\
		"stfdu %%f1,8(%0);"					\
										\
	    : "+b" (dst2)							\
	    : "r" (Y2), "r" (U), "r" (V), "r" (tmp)		\
	    : "r3", "r4", "fr1");					\
	}
#endif

/***************************** DRAW_SLICE ***************************/

static int draw_slice(uint8_t *image[], int stride[], int w,int h,int x,int y)
{
	//struct RenderInfo yuvInfo;
	struct BitMap *bm = NULL;
	int lock;
	unsigned int w3, _w2;
	uint32 bpr;
	APTR ba;

	// By set w -= (w%16) and h -= (h%16) we have a background color problem when starting videos
	// due to a bug in ATIRadeon.chip that it always opens a YUV pixel format instead of R5G6B5)
	// By set w -= (w%8) and h -= (h%8) instead we doesn't have the background color issue but
	// video with unusual resolution may be corrupted
	w -= (w%16);
	h -= (h%16);

	MutexObtain( window_mx );
	if (!My_Window)
	{
		MutexRelease(window_mx);
		return 0;
	}

	p96PIP_GetTags(My_Window, P96PIP_SourceBitMap,(ULONG)&bm, TAG_DONE);

	lock = LockBitMapTags( bm,
		LBM_BaseAddress, &ba,//&yuvInfo.Memory,
		LBM_BytesPerRow, &bpr,//&yuvInfo.BytesPerRow,
	TAG_END);
DBUG("BytesPerRow = %ld ('LockBitMapTags()')\n",bpr);
//	yuvInfo.BytesPerRow = (w + 7) & -8;	// LockBitMapTags() returns broken bpr ( = 0). (UWORD BytesPerRow; but LBM_BytesPerRow tag is uint32)
DBUG("BytesPerRow = %ld (hand calculated)\n",(w + 7) & -8);

	if (lock == NULL)
	{
		// Unable to lock the BitMap -> do nothing
		return -1;
	}

	w3 = bpr>>1;//yuvInfo.BytesPerRow;
//	w3 = w & -8;
	_w2 = w3 / 8; // Because we write 8 pixels (aka 16 bit word) at each loop

    h &= -2;

    if (h > 0 && _w2)
    {
        ULONG *srcdata, *srcdata2;

        ULONG *py, *py2, *pu, *pv;
        ULONG *_py, *_py2, *_pu, *_pv;

        _py = py = (ULONG *) image[0];
        _pu = pu = (ULONG *) image[1];
        _pv = pv = (ULONG *) image[2];

        //srcdata = (ULONG *) ((uint32)yuvInfo.Memory + ( ( (y * yuvInfo.BytesPerRow) + x) * 2));
        srcdata = (ULONG *) ((uint32)ba + ( ( (y * bpr) + x) * 2));

        // The data must be stored like this:
        // YYYYYYYY UUUUUUUU : pixel 0
        // YYYYYYYY VVVVVVVV : pixel 1

        // We will read one word of Y (4 byte)
        // then read one word of U and V
        // After another Y word because we need 2 Y for 1 (U & V)

        _py--;
        _pu--;
        _pv--;

        stride[0] &= -8;
        stride[1] &= -8;
        stride[2] &= -8;

#if USE_VMEM64
        /* If aligned buffer, use 64bit writes. It might be worth
           the effort to manually align in other cases, but that'd
           need to handle all conditions such like:
           a) UWORD, UQUAD ... [end aligment]
           b) ULONG, UQUAD ... [end aligment]
           c) ULONG, UWORD, UQUAD ... [end aligment]
           - Piru
        */
        if (!(((uint32) srcdata) & 7))
        {
            ULONG _tmp[3];
            // This alignment crap is needed in case we don't have an aligned stack
            register ULONG *tmp = (APTR)((((ULONG) _tmp) + 4) & -8);

            srcdata -= 2;

            do
            {
                int w2 = _w2;
                h -= 2;

                _py2 = (ULONG *)(((ULONG)_py) + (stride[0] ) );

                py = _py;
                py2 = _py2;
                pu = _pu;
                pv = _pv;

                srcdata2 = (ULONG *) ( (ULONG) srcdata + w3 * 2 );

                do
                {
                    // Y is like that : Y1 Y2 Y3 Y4
                    // U is like that : U1 U2 U3 U4
                    // V is like that : V1 V2 V3 V4

                    PLANAR_2_CHUNKY_64(py, py2, pu, pv, srcdata, srcdata2, tmp);

                } while (--w2);

                srcdata = (ULONG *) ( (ULONG) srcdata + (w3 << 1) );
                _py = (ULONG *)(((ULONG)_py) + (stride[0] << 1) );
                _pu = (ULONG *)(((ULONG)_pu) + stride[1]);
                _pv = (ULONG *)(((ULONG)_pv) + stride[2]);

            } while (h);
        }
        else
#endif
        {
//            srcdata--;

            do
            {
                int w2 = _w2;

                h -= 2;

                _py2 = (ULONG *)(((ULONG)_py) + (stride[0] ) );

                py = _py;
                py2 = _py2;
                pu = _pu;
                pv = _pv;

                srcdata2 = (ULONG *) ( (ULONG) srcdata + w3 * 2 );

                do
                {
                    // Y is like that : Y1 Y2 Y3 Y4
                    // U is like that : U1 U2 U3 U4
                    // V is like that : V1 V2 V3 V4

                    PLANAR_2_CHUNKY(py, py2, pu, pv, srcdata, srcdata2);

                } while (--w2);

                srcdata = (ULONG *) ( (ULONG) srcdata + (w3 << 1) );
                _py = (ULONG *)(((ULONG)_py) + (stride[0] << 1) );
                _pu = (ULONG *)(((ULONG)_pu) + stride[1]);
                _pv = (ULONG *)(((ULONG)_pv) + stride[2]);

            } while (h);
        }
    }

	UnlockBitMap(lock);

	MutexRelease(window_mx);

	return 0;
}

/***************************** DRAW_OSD *****************************/

#if 0 /* MickJT -- I prefer other function */
static void draw_osd(void)
{
	if (draw_alpha_func) vo_draw_text(amiga_image_width, amiga_image_height, draw_alpha_func);
}
#endif

static void draw_osd(void)
{
    vo_draw_text(amiga_image_width, amiga_image_height, draw_alpha_fnc);
}

/***************************** FLIP_PAGE ****************************/

static void flip_page(void)
{
	uint32 WorkBuf, DispBuf, NextWorkBuf;

	if (numbuffers>1)
	{
		p96PIP_GetTags(My_Window,
		               P96PIP_WorkBuffer,      &WorkBuf,
		               P96PIP_DisplayedBuffer, &DispBuf,
		              TAG_END/*, TAG_DONE*/ );

		NextWorkBuf = (WorkBuf+1) % numbuffers;

		if  ((NextWorkBuf == DispBuf) && (vsync_is_enabled == 1))
		{
			WaitBOVP(&(My_Window->WScreen->ViewPort));
		}

//			if ((NextWorkBuf == DispBuf)&&(vsync_is_enabled == 1)) WaitTOF();

		p96PIP_SetTags(My_Window,
		               P96PIP_VisibleBuffer, WorkBuf,
		               P96PIP_WorkBuffer,    NextWorkBuf,
		              TAG_END/*, TAG_DONE*/ );
	}

   /* Nothing */
}

/***************************** DRAW_FRAME ***************************/
static int draw_frame(uint8_t *src[])
{
	// Nothing
	return -1;
}

/******************************************************************************/
/* Just a little function to help uninit and control.					*/
/* It close the screen (if fullscreen), the windows, and free all gfx related	*/
/* stuff, but do not close any libs								*/
/******************************************************************************/

static void FreeGfx(void)
{
	MutexObtain( window_mx );

	if (My_Window)
	{
		gfx_Stop(My_Window);

		detach_menu(My_Window);
// markus
		dispose_icon(My_Window, &iconifyIcon );
		dispose_icon(My_Window, &fullscreenicon );
//
		p96PIP_Close(My_Window);
		My_Window = NULL;

		SetVar("p96pip_in_use","0",1,GVF_GLOBAL_ONLY);
	}

	closeRemainingOpenWin();

	/*if (bf_Window)
	{
		CloseWindow(bf_Window);
		bf_Window=NULL;
	}*/

	if (is_my_screen)
	{
		if (the_screen)
		{
			gfx_ControlBlanker(the_screen, TRUE);
			CloseScreen(the_screen);
			the_screen = NULL;
		}
	}

	MutexRelease(window_mx);
}

/****************************** UNINIT ******************************/
static void uninit(void)
{
	    FreeGfx();

DBUG("backfillhook = %p (free)\n",backfillhook);
FreeSysObject(ASOT_HOOK, backfillhook);
backfillhook = NULL;
}

/****************************** CONTROL *****************************/
static int control(uint32_t request, void *data)
{
    switch (request)
    {
        case VOCTRL_GUISUPPORT:
            return VO_FALSE;

        case VOCTRL_FULLSCREEN:
		if (!is_fullscreen) {
			old_d_width  = window_width;
			old_d_height = window_height;
			old_win_top  = win_top;
			old_win_left = win_left;
		}
		else {
			win_top  = old_win_top;
			win_left = old_win_left;
		}
		is_fullscreen ^= VOFLAG_FULLSCREEN;
		FreeGfx(); // Free/Close all gfx stuff (screen windows, buffer...);
		if ( config(amiga_image_width, amiga_image_height, old_d_width, old_d_height, is_fullscreen, NULL, amiga_image_format) < 0) return VO_FALSE;
            return VO_TRUE;

        case VOCTRL_ONTOP:
        	vo_ontop = !vo_ontop;
        	is_ontop = !is_ontop;
		FreeGfx(); // Free/Close all gfx stuff (screen windows, buffer...);
		if ( config(amiga_image_width, amiga_image_height, window_width, window_height, FALSE, NULL, amiga_image_format) < 0) return VO_FALSE;
            return VO_TRUE;

		case VOCTRL_UPDATE_SCREENINFO:
		if (is_fullscreen)
		{
			vo_screenwidth  = the_screen->Width;
			vo_screenheight = the_screen->Height;
		}
		else
		{
			struct Screen *scr = LockPubScreen(NULL);
			if(scr)
			{
				vo_screenwidth  = scr->Width;
				vo_screenheight = scr->Height;
				UnlockPubScreen(NULL, scr);
			}
	        }
		aspect_save_screenres(vo_screenwidth, vo_screenheight);
            return VO_TRUE;

        case VOCTRL_PAUSE:
            return VO_TRUE;

        case VOCTRL_RESUME:
            return VO_TRUE;

        case VOCTRL_QUERY_FORMAT:
            return query_format(*(ULONG *)data);
    }

    return VO_NOTIMPL;
}

/**************************** CHECK_EVENTS **************************/

static void check_events(void)
{
	gfx_CheckEvents(the_screen, My_Window, &window_height, &window_width, &win_left, &win_top);
}



static int query_format(uint32_t format)
{
    switch( format)
    {
	    case IMGFMT_YV12:
	    case IMGFMT_YUY2:
	    case IMGFMT_I420:
	    case IMGFMT_UYVY:
	    case IMGFMT_YVYU:
    	    return  VFCAP_CSP_SUPPORTED | VFCAP_CSP_SUPPORTED_BY_HW | VFCAP_OSD |
        	    	VFCAP_HWSCALE_UP 	| VFCAP_HWSCALE_DOWN 		| VFCAP_ACCEPT_STRIDE;

	    case IMGFMT_RGB15:
	    case IMGFMT_BGR15:
	    case IMGFMT_RGB16:
	    case IMGFMT_BGR16:
	    case IMGFMT_RGB24:
	    case IMGFMT_BGR24:
	    case IMGFMT_RGB32:
	    case IMGFMT_BGR32:
    	    return VFCAP_CSP_SUPPORTED | VFCAP_OSD | VFCAP_FLIP;

        default:
            return VO_FALSE;
    }
}


#if 0
static void ClearSpaceArea(struct Window *window)
{
	struct IBox* ibox;
	IIntuition->GetAttr(SPACE_AreaBox, Space_Object, (uint32*)(void*)&ibox);
	IGraphics->SetAPen(window->RPort,1);
	IGraphics->RectFill(window->RPort,ibox->Left,ibox->Top,ibox->Left+ibox->Width-1,ibox->Top+ibox->Height-1);
	IIntuition->ChangeWindowBox(window, window->LeftEdge, window->TopEdge, window->Width, window->Height);
}
#endif

