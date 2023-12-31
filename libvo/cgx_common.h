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

#ifndef __gfx_COMMON__
#define __gfx_COMMON__


#include <proto/exec.h>
#include <proto/graphics.h>


#ifndef __amigaos4__
#include <proto/cybergraphics.h>
#endif

#include <proto/layers.h>

#include <intuition/intuition.h>

#include "video_out.h"

#define __OLDMOS__

#ifndef __amigaos4__
// Some Libs
extern struct GfxBase *		GfxBase;
extern struct Library *		CyberGfxBase;
extern struct IntuitionBase *	IntuitionBase;
extern struct Library *		LayersBase;
#endif

extern ULONG gfx_common_rgb_format;
extern ULONG gfx_novsync;
extern ULONG gfx_nodri;
extern ULONG gfx_nodma;	// No buffer, only vram bitmap.
extern char *gfx_screenname;

#define IDCMP_COMMON IDCMP_MOUSEBUTTONS | IDCMP_INACTIVEWINDOW | IDCMP_ACTIVEWINDOW  | \
	IDCMP_CHANGEWINDOW | /*IDCMP_MOUSEMOVE |*/ IDCMP_REFRESHWINDOW | IDCMP_RAWKEY | IDCMP_VANILLAKEY | \
	IDCMP_EXTENDEDMOUSE | IDCMP_CLOSEWINDOW | IDCMP_NEWSIZE | IDCMP_INTUITICKS | IDCMP_MENUPICK | IDCMP_MENUVERIFY
// | IDCMP_SIZEVERIFY

/****/
#define NOBORDER			0
#define TINYBORDER		1
#define TRANSPBORDER		2
#define ALWAYSBORDER		3
extern ULONG gfx_BorderMode;

/* Some flags */
#define gfx_FULLSCREEN	( 1L << 0)	// Fullscreen flags has you guessed
#define gfx_FASTMODE	( 1L << 1) 	// Tell the driver to privilege the speed instead of the quality if possible
#define gfx_NOSCALE	( 1L << 2)	// Do not scale (this way it's faster but can be very ugly)
#define gfx_BUFFERING	( 1L << 3)	// Ask to use Multiple buffering if available
#define gfx_NOPLANAR	( 1L << 4)	// Don't use planar overlay

#define gfx_common_screenname "MPlayer for AmigaOS4"

#ifndef SA_StopBlanker
#define SA_StopBlanker (SA_Dummy + 121)
#endif

#ifndef SA_ShowPointer
#define SA_ShowPointer (SA_Dummy + 122)
#endif

/***/
extern ULONG gfx_monitor;


/***************************/
BOOL gfx_GiveArg(const char *arg);
VOID gfx_ReleaseArg(VOID);

void gfx_StartWindow(struct Window *My_Window);
void gfx_StopWindow(struct Window *My_Window);
void gfx_HandleBorder(struct Window *My_Window, ULONG handle_mouse);

BOOL gfx_CheckEvents(struct Screen * My_Screen, struct Window *My_Window, uint32_t *window_height, uint32_t *window_width,
                     uint32_t *window_left, uint32_t *window_top);
void gfx_Start(struct Window *My_Window);
void gfx_Stop(struct Window *My_Window);

void gfx_Message(void);

void gfx_ControlBlanker(struct Screen * screen, ULONG enable);
void gfx_BlankerState(void);
void gfx_ShowMouse(struct Screen * screen, struct Window * window, ULONG enable);

void gfx_get_max_mode(int RTGBoardNum, int depth_bits, ULONG *mw,ULONG *mh);
void gfx_center_window(struct Screen *My_Screen, uint32_t window_width, uint32_t window_height, uint32_t *win_left, uint32_t *win_top);


struct gfx_command
{
	char *name;
	char type;
	ULONG *target;
	char *help;
	BOOL (*hook_fn) (struct gfx_command *commands, char *cmd_name, ULONG *value );
};


// std commands
extern struct gfx_command gfx_commands[];

extern BOOL gfx_help(struct gfx_command *commands, char *cmd_name, ULONG *value);
extern BOOL fix_rgb_modes(struct gfx_command *commands, char *cmd_name, ULONG *value);
extern BOOL fix_border(struct gfx_command *commands, char *cmd_name, ULONG *value);
extern BOOL gfx_process_args(char *arg, struct gfx_command *commands);
extern uint32 find_best_screenmode(int RTGBoardNum, uint32 depth_bits, float aspect, int get_w, int get_h);
extern void print_screen_info( ULONG monitor, ULONG ModeID );

extern void set_gfx_rendering_option(void);


/******************************/
#ifdef __amigaos4__
// Note: No REG_crap on OS4.
#define PREPARE_BACKFILL(BufferWidth, BufferHeight) \
	struct Layer *MyLayer = ArgRP->Layer; \
	\
	struct RastPort	MyRP;	\
	ULONG SizeX;		\
	ULONG SizeY;		\
	ULONG StartX;		\
	ULONG StartY;		\
	ULONG BufferStartX;	\
	ULONG BufferStartY;	\
	\
	if (!MyLayer) return;	\
	\
	SizeX			= MyArgs->bounds.MaxX - MyArgs->bounds.MinX + 1; \
	SizeY			= MyArgs->bounds.MaxY - MyArgs->bounds.MinY + 1; \
	StartX		= MyLayer->bounds.MinX + MyArgs->offsetx;	\
	StartY		= MyLayer->bounds.MinY + MyArgs->offsety;	\
	BufferStartX	= MyArgs->offsetx - My_Window->BorderLeft; \
	BufferStartY	= MyArgs->offsety - My_Window->BorderTop;	\
	\
	if (SizeX <= 0 	\
			|| SizeY <= 0	\
			|| MyArgs->offsetx < 0	\
			|| MyArgs->offsety < 0	\
			|| BufferStartX < 0	\
			|| BufferStartY < 0	\
			) return;	\
	memcpy(&MyRP, ArgRP, sizeof(struct RastPort) );	\
	MyRP.Layer = NULL;	\
	\
	if ( (SizeX + StartX) > MyLayer->bounds.MaxX) SizeX = MyLayer->bounds.MaxX - StartX;	\
	if ( (SizeY + StartY) > MyLayer->bounds.MaxY) SizeY = MyLayer->bounds.MaxY - StartY;
#else
#define PREPARE_BACKFILL(BufferWidth, BufferHeight) \
	struct BackFillArgs *MyArgs 	= (struct BackFillArgs *) REG_A1; \
	struct RastPort *ArgRP     		= (struct RastPort *) REG_A2; \
	struct Layer *MyLayer = ArgRP->Layer; \
	\
	struct RastPort	MyRP;	\
	ULONG SizeX;		\
	ULONG SizeY;		\
	ULONG StartX;		\
	ULONG StartY;		\
	ULONG BufferStartX;	\
	ULONG BufferStartY;	\
	\
	if (!MyLayer) return;	\
	\
	SizeX			= MyArgs->bounds.MaxX - MyArgs->bounds.MinX + 1; \
	SizeY			= MyArgs->bounds.MaxY - MyArgs->bounds.MinY + 1; \
	StartX		= MyLayer->bounds.MinX + MyArgs->offsetx; \
	StartY		= MyLayer->bounds.MinY + MyArgs->offsety; \
	BufferStartX 	= MyArgs->offsetx - My_Window->BorderLeft; \
	BufferStartY 	= MyArgs->offsety - My_Window->BorderTop; \
	\
	if (SizeX <= 0 	\
			|| SizeY <= 0		\
			|| MyArgs->offsetx < 0	\
			|| MyArgs->offsety < 0	\
			|| BufferStartX < 0	\
			|| BufferStartY < 0	\
			) return;	\
	memcpy(&MyRP, ArgRP, sizeof(struct RastPort) );	\
	MyRP.Layer = NULL;	\
	\
	if ( (SizeX + StartX) > MyLayer->bounds.MaxX) SizeX = MyLayer->bounds.MaxX - StartX;	\
	if ( (SizeY + StartY) > MyLayer->bounds.MaxY) SizeY = MyLayer->bounds.MaxY - StartY;
#endif
/*
	if ( (SizeX + BufferStartX) > BufferWidth) SizeX = BufferWidth - StartX;	\
	if ( (SizeY + BufferStartY) > BufferHeight) SizeY = BufferHeight - StartY;
*/


BOOL Iconify(struct Window *w);
BOOL UnIconify(struct Window *w);

BOOL isRadeonHDorSM502_chip(struct Screen *scr);
void BackFillfunc(struct Hook *hook UNUSED, struct RastPort *rp, struct BackFillMessage *msg);


#endif
