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

// newlib bug workaround for libSDL IDOS == NULL crash
unsigned const int __newlib_minversion = (52 << 16) | 20;

/*
 *  vo_amiga.c
 *  VO module for MPlayer AmigaOS
 *  Using p96 PIP
 *  Written by Jörg Strohmayer
 */

#ifdef __USE_INLINE__
	#undef __USE_INLINE__
//#error AmigaOS 4.x code, fix your compiler options
#endif

#define NO_DRAW_FRAME
// #define NO_DRAW_SLICE

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/unistd.h>
#include <sys/param.h>

#include "config.h"
#include "aspect.h"
#include "input/input.h"
#include "libmpcodecs/vf_scale.h"
#include "mplayer.h"
#include "mp_fifo.h"
#include "mp_msg.h"
#include "osdep/keycodes.h"
#include "sub/sub.h"
#include "sub/eosd.h"
#include "subopt-helper.h"
#include "video_out.h"
#include "video_out_internal.h"

#include "version.h"

// OS specific
// #include "MPlayer-arexx.h"

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/intuition.h>
#include <proto/graphics.h>
#include <proto/gadtools.h>
#include <proto/Picasso96API.h>
#include <dos/dos.h>
#include <dos/dostags.h>
#include <graphics/blitattr.h>
#include <graphics/composite.h>
#include <graphics/gfxbase.h>
#include <graphics/layers.h>
#include <graphics/rpattr.h>
#include <graphics/text.h>
#include <libraries/keymap.h>


extern struct Catalog *catalog;
#define MPlayer_NUMBERS
#define MPlayer_STRINGS
#include "../amigaos/MPlayer_catalog.h"
extern STRPTR myGetCatalogStr (struct Catalog *catalog, LONG num, STRPTR def);
#define CS(id) myGetCatalogStr(catalog,id,id##_STR)


static vo_info_t info =
{
    "AmigaOS P96 video output",
    "amiga",
    "Jörg Strohmayer",
    "AmigaOS4 rules da world !"
};

LIBVO_EXTERN(amiga)

/* ARexx
extern void put_command0(int);
extern struct ArexxHandle *rxHandler;
*/


static struct Window *My_Window;
// #define SOURCE_BITMAP
#ifdef SOURCE_BITMAP
static struct BitMap *SourceBitMap;
#endif
#include <graphics/gfx.h>
// #define WRITEPIXEL
#ifdef WRITEPIXEL
static struct BitMap *tmpBitMap;
#endif
struct BitMap *R5G6B5_BitMap;

//#include <classes/window.h>
#include <intuition/imageclass.h>
#include <../amigaos/window_icons.h>
#include <../amigaos/debug.h>

extern struct kIcon iconifyIcon;
//extern struct kIcon padlockicon;
extern struct kIcon fullscreenicon;

static struct Screen *My_Screen = NULL;
extern uint32_t is_fullscreen;

//static struct Window *bf_Window = NULL;
static struct Hook *backfillhook;

//#define USERTG

enum
{
   NOBORDER,
   TINYBORDER,
   ALWAYSBORDER
};
static uint32 BorderMode = ALWAYSBORDER;

static uint32 image_width;
static uint32 image_height;
static uint32 image_format;
static uint32 out_format;
static uint32 window_left;
static uint32 window_top;
static uint32 window_width;
static uint32 window_height;
static char * window_title;
static uint32 pip_format;
static uint32 old_d_width;
static uint32 old_d_height;
static uint32 is_ontop;
static uint32 FirstTime = TRUE;
static int16  yuv2rgb[5][255];


/************************* MENU DEFINITION **************************/
/*static struct Menu *menu;
static struct NewMenu nm[] =
{
   // Type, Label, CommKey, Flags, MutualExclude, UserData

   { NM_TITLE,   "Project",         NULL, 0,                  0, NULL },
//  { NM_ITEM,    "Open File..",    "L",  0,                  0, NULL },
      { NM_ITEM,    "Quit",         "Q",  0,                  0, NULL },
   { NM_TITLE,   "Play",            NULL, 0,                  0, NULL },
      { NM_ITEM,    "Play/Pause",   "P",  0,                  0, NULL },
      { NM_ITEM,    "Stop",         "S",  0,                  0, NULL },
   { NM_TITLE,   "Video Options",   NULL, 0,                  0, NULL },
#if 0
      { NM_ITEM,    "Stay on Top",  NULL, MENUTOGGLE|CHECKIT, 0, NULL },
#endif
      { NM_ITEM,    "Cycle OSD",    "O",  0,                  0, NULL },
#if 0
   { NM_TITLE,   "Audio Options",   NULL, 0,                  0, NULL },
      { NM_ITEM,    "Mute",         NULL, MENUTOGGLE|CHECKIT, 0, NULL },
#endif
   { NM_END,     NULL,              NULL, 0,                  0, NULL }
};*/

/************************ END MENU DEFINITION ***********************/

/*static struct Gadget MyDragGadget =
{
   NULL,
   0,0,                                             // Pos
   -16,-16,                                         // Size
   GFLG_RELWIDTH | GFLG_RELHEIGHT | GFLG_GADGHNONE, // Flags
   0,                                               // Activation
   GTYP_WDRAGGING,                                  // Type
   NULL,                                            // GadgetRender
   NULL,                                            // SelectRender
   NULL,                                            // GadgetText
   0L,                                              // Obsolete
   NULL,                                            // SpecialInfo
   0,                                               // Gadget ID
   NULL                                             // UserData
};*/

/***************************************************/
static APTR oldProcWindow;

static void procWindowInit(void)
{
   oldProcWindow = IDOS->SetProcWindow((APTR)-1);
}
//__attribute__((section(".ctors"))) static void (*procWindowInitPtr)(void) USED = procWindowInit;

static void procWindowExit(void)
{
   IDOS->SetProcWindow(oldProcWindow);
}
//__attribute__((section(".dtors"))) static void (*procWindowExitPtr)(void) USED = procWindowExit;

/***************************************************/
extern BOOL gfx_CheckEvents(struct Screen * My_Screen, struct Window *My_Window, uint32_t *window_height, uint32_t *window_width,
                            uint32_t *window_left, uint32_t *window_top); // cgx_common.c
/*static BOOL CheckEvents(void)
{
    uint32 retval = FALSE;
    // static BOOL mouseonborder;
    uint32 info_sig;
//    uint32 menucode;
    // char temp_filename[MAXPATHLEN];
    // struct FileRequester * AmigaOS_FileRequester = NULL;

    info_sig = 1L << (My_Window->UserPort)->mp_SigBit;

    if (IExec->SetSignal(0L, info_sig ) & info_sig)
    { // If an event -> Go Go GO !
        struct IntuiMessage * IntuiMsg;
        uint32 msgClass;
        uint16 Code;

        while ((IntuiMsg = (struct IntuiMessage *) IExec->GetMsg(My_Window->UserPort)))
        {
            msgClass = IntuiMsg->Class;
            Code = IntuiMsg->Code;
            IExec->ReplyMsg( (struct Message *) IntuiMsg);

            switch (msgClass)
            {
                case IDCMP_MENUPICK:
                    menucode = Code;
                    if (menucode != MENUNULL)
                    {
                        while (menucode != MENUNULL)
                        {
                            struct MenuItem  *item = IIntuition->ItemAddress(menu,menucode);

                            // Printf("Menu %ld, item %ld, sub-item %2ld (item address %08lX)\n",
                            // MENUNUM(menucode),
                            // ITEMNUM(menucode),
                            // SUBNUM(menucode),
                            // item);

                            switch (MENUNUM(menucode))
                            {
                                case 0:
                                   switch (ITEMNUM(menucode))
                                   {
                                      case 0:  // Quit
                                          mplayer_put_key(KEY_ESC);
                                      break;
                                   }
                                break;

                                case 1:
                                   switch (ITEMNUM(menucode))
                                   {
                                      case 0:  // pause/play
                                         mplayer_put_key('p');
                                      break;

                                      case 1:  // Quit
                                         mplayer_put_key(KEY_ESC);
                                      break;
                                   }
                                break;

                                case 2:
                                   switch (ITEMNUM(menucode))
                                   {
#if 0
                                      case 0:
//                                         put_command0(MP_CMD_VO_ONTOP);
                                      break;

                                      case 1:
                                         mplayer_put_key('o');
                                      break;
#else
                                      case 0:
                                         mplayer_put_key('o');
                                      break;
#endif
                                   }
                                break;

#if 0
                                case 4:
//                                 put_command0(MP_CMD_MUTE);
                                break;
#endif
                            }

                            // Handle multiple selection
                            menucode = item->NextSelect;
                        }
                    }
                break;

                case IDCMP_CLOSEWINDOW:
                    mplayer_put_key(KEY_ESC);
                break;

                case IDCMP_VANILLAKEY:
                    switch ( Code )
                    {
                        case  '\e':
                            mplayer_put_key(KEY_ESC);
                        break;

                        default:
                            if (Code!='\0') mplayer_put_key(Code); break;
                    }
                break;

                case IDCMP_RAWKEY:
                    switch ( Code )
                    {
                        case  RAWKEY_PAGEDOWN:
                            mplayer_put_key(KEY_PGDWN);
                        break;
                        case  RAWKEY_PAGEUP:
                            mplayer_put_key(KEY_PGUP);
                        break;
                        case  RAWKEY_CRSRRIGHT:
                            mplayer_put_key(KEY_RIGHT);
                        break;
                        case  RAWKEY_CRSRLEFT:
                            mplayer_put_key(KEY_LEFT);
                        break;
                        case  RAWKEY_CRSRUP:
                            mplayer_put_key(KEY_UP);
                        break;
                        case  RAWKEY_CRSRDOWN:
                            mplayer_put_key(KEY_DOWN);
                        break;
                    }
                    break;

#if 0
               case IDCMP_SIZEVERIFY:
                         IP96->p96PIP_SetTags(My_Window,
                                              FA_Active, FALSE,
                                              TAG_DONE);
                    break;
#endif

               case IDCMP_CHANGEWINDOW:
                    {
                       if (0 == pip_format)
                       {
//                          uint32 w, h;
//                          IIntuition->GetWindowAttrs(My_Window, WA_InnerWidth, &w, WA_InnerHeight, &h, TAG_DONE);
//                          vo_dwidth = w;
//                          vo_dheight = h;
                          flip_page();
                       }
#if 0
                         uint32 x_offset, y_offset, inner_width, inner_height;

                         IIntuition->GetWindowAttrs(My_Window, WA_InnerWidth, &inner_width, WA_InnerHeight, &inner_height, TAG_DONE);

                         // Change the window size to fill the whole screen with good aspect
                         if ( ( (float)old_d_width / (float)old_d_height) < ( (float)inner_width / (float) inner_height) )
                         {
 #if 0
                                 // Width (Longeur) is too big
                                 y_offset = 0;
                                 x_offset = inner_width - inner_height * ( (float) old_d_width / old_d_height);
                                 x_offset /= 2;
 #else
                                 x_offset = inner_width;
                                 y_offset = inner_height + inner_width * ( (float) old_d_height / old_d_width);
                                 y_offset /= 2;
 #endif
                         } else {
 #if 0
                                 // Height (largeur) too big
                                 y_offset = inner_height - inner_width * ( (float) old_d_height / old_d_width);
                                 y_offset /= 2;
                                 x_offset = 0;
 #else
                                 y_offset = inner_height;
                                 x_offset = inner_width + inner_height * ( (float) old_d_width / old_d_height);
                                 x_offset /= 2;
 #endif
                         }

// printf("new: %ld, %ld\n\n", x_offset, y_offset);
 #if 0
                         IP96->p96PIP_SetTags(My_Window,
                                              FA_Left,   My_Window->LeftEdge + My_Window->BorderLeft + x_offset,
                                              FA_Width,  inner_width - x_offset*2,
                                              FA_Top,    My_Window->TopEdge + My_Window->BorderTop + y_offset,
                                              FA_Height, inner_height - y_offset*2,
//                                              FA_Active, TRUE,
                                              TAG_DONE);
 #else
                         {
                             static uint32 old_x_offset = 0;
                             static uint32 old_y_offset = 0;

                             if (x_offset != old_x_offset || y_offset != old_y_offset)
                             {
                                 IIntuition->SetWindowAttrs(My_Window,
                                                            WA_InnerWidth,  x_offset,
                                                            WA_InnerHeight, y_offset,
                                                            TAG_DONE);
                             }

                             old_x_offset = x_offset;
                             old_y_offset = y_offset;
                         }
 #endif
#endif // IDCMP_CHANGEWINDOW
                    }
                    break;
            }
        }
    }

    return retval;
}*/


static char *GetWindowTitle(void)
{
   if (window_title) free(window_title);

   if (filename)
   {
      window_title = (char *)malloc(strlen("MPlayer - ") + strlen(filename) + 1);
      strcpy(window_title, "MPlayer - ");
      strcat(window_title, filename);
   } else {
      window_title = strdup("MPlayer " AMIGA_VERSION " (amiga)");
   }

   return window_title;
}

/**************************** DRAW ALPHA ****************************/
// Draw_alpha series !
static void draw_alpha(int x, int y, int w, int h, unsigned char* src, unsigned char *srca, int stride)
{
#ifndef SOURCE_BITMAP
   struct BitMap *bm;
#endif
   struct YUVRenderInfo ri;
   int lock_h;
   vo_draw_alpha_func draw;

   switch (pip_format)
   {
      case 0:
         draw = vo_get_draw_alpha(IMGFMT_RGB16);
         break;
      case RGBFB_YUV422CGX:
         draw = vo_get_draw_alpha(IMGFMT_YVYU);
         break;
      default:
         draw = vo_get_draw_alpha(image_format);
   }

   if (!draw) return;

   if (!w || !h)
      return;

#ifndef SOURCE_BITMAP
   if (0 == pip_format)
   {
      if (R5G6B5_BitMap)
      {
         bm = R5G6B5_BitMap;
      } else {
         bm = My_Window->RPort->BitMap;
         x += My_Window->BorderLeft;
         y += My_Window->BorderTop;
      }
   } else {
      IP96->p96PIP_GetTags(My_Window, P96PIP_SourceBitMap, (uint32)&bm, TAG_DONE);
   }

   if (! (lock_h = IP96->p96LockBitMap(bm, (uint8 *)&ri, sizeof(ri))))
#else
   if (! (lock_h = IP96->p96LockBitMap(SourceBitMap, (uint8 *)&ri, sizeof(ri))))
#endif
   {
      // Unable to lock the BitMap -> do nothing
      return;
   }

   if (0 == pip_format || RGBFB_YUV422CGX == pip_format)
   {
      draw(w, h, src, srca, stride, (uint8 *)ri.RI.Memory + ri.RI.BytesPerRow * y + x * 2, ri.RI.BytesPerRow);
   } else {
      draw(w, h, src, srca, stride, (uint8 *)ri.Planes[0] + ri.BytesPerRow[0] * y + x, ri.BytesPerRow[0]);
   }

#ifndef SOURCE_BITMAP
   IP96->p96UnlockBitMap(bm, lock_h);
#else
   IP96->p96UnlockBitMap(SourceBitMap, lock_h);
#endif
}

/***************************** PREINIT ******************************/
static int opt_border, opt_mode, opt_help, opt_question;
static const opt_t subopts[] =
{
//   { "noborder",   OPT_ARG_BOOL,  &opt_noborder,   NULL },
//   { "tinyborder", OPT_ARG_BOOL,  &opt_tinyborder, NULL },
//   { "driver",     OPT_ARG_MSTRZ, &sdl_driver,     NULL },
   { "border",   OPT_ARG_INT,  &opt_border,   NULL },
   { "mode",     OPT_ARG_INT,  &opt_mode,     NULL },
   { "help",     OPT_ARG_BOOL, &opt_help,     NULL },
   { "?",        OPT_ARG_BOOL, &opt_question, NULL },
   { NULL, 0, NULL, NULL }
};

#ifdef USERTG
#include "/V/include/internal/picasso96/settings.h"
#include "/V/include/internal/picasso96/boardinfo.h"
#include "/V/include/internal/picasso96/rtgbase.h"
static struct RTGBase *RTGBase;
#endif

/**********************/
/*static void BackFillfunc(UNUSED struct Hook *hook, UNUSED struct RastPort *rp, UNUSED struct BackFillMessage *bm)
{
   IIntuition->GetWindowAttrs(My_Window, WA_InnerWidth,&window_width, WA_InnerHeight,&window_height, TAG_DONE);
   if (R5G6B5_BitMap && My_Window->RPort->Layer->BitMap)
   {
      uint32 scaleX = window_width  * COMP_FIX_ONE / image_width;
      uint32 scaleY = window_height * COMP_FIX_ONE / image_height;

      IGraphics->CompositeTags(COMPOSITE_Src_Over_Dest, R5G6B5_BitMap, My_Window->RPort->Layer->BitMap,
                               COMPTAG_DestX,      My_Window->BorderLeft,
                               COMPTAG_DestY,      My_Window->BorderTop,
                               COMPTAG_OffsetX,    My_Window->BorderLeft,
                               COMPTAG_OffsetY,    My_Window->BorderTop,
                               COMPTAG_DestWidth,  My_Window->Width - My_Window->BorderLeft - My_Window->BorderRight,
                               COMPTAG_DestHeight, My_Window->Height - My_Window->BorderTop - My_Window->BorderBottom,
                               COMPTAG_ScaleX,     scaleX,
                               COMPTAG_ScaleY,     scaleY,
                               TAG_DONE);
   } else {
#if 0 // hangs, can't use RastPort functions
      IGraphics->SetAPen(My_Window->RPort, 0);
      IGraphics->RectFill(My_Window->RPort,
                          My_Window->BorderLeft,
                          My_Window->BorderTop,
                          My_Window->Width - My_Window->BorderRight - 1,
                          My_Window->Height - My_Window->BorderBottom - 1);
#endif
   }
}*/

/*****************************/
static void Close_Window(void)
{
IDBUG("Close_Window()\n",NULL);
   if (My_Window)
   {
//      IIntuition->ClearMenuStrip(My_Window);
      detach_menu(My_Window);

      // markus
      dispose_icon(My_Window, &iconifyIcon );
      dispose_icon(My_Window, &fullscreenicon );
      //

      if (0 == pip_format)
      {
         IIntuition->CloseWindow(My_Window);
      } else {
         IP96->p96PIP_Close(My_Window);
      }
      My_Window = NULL;
   }
}

static uint32 Open_Window(uint32 hidden)
{
   // Window
   uint32 ModeID = INVALID_ID;
   //struct Screen *My_Screen;
IDBUG("Open_Window() %s\n",hidden? "[hidden]":"");
//   SourceBitMap = IP96->p96AllocBitMap(image_width, image_height, 8, BMF_USERPRIVATE, NULL, RGBFB_YUV420P);
//   SourceBitMap = IP96->p96AllocBitMap(image_width, image_height * 2, 8, BMF_USERPRIVATE, NULL, RGBFB_YUV420P);
#ifdef SOURCE_BITMAP
   SourceBitMap = IP96->p96AllocBitMap(image_width, image_height + (image_height + 2) / 3, 8, BMF_USERPRIVATE, NULL, RGBFB_YUV420P); // rtg_misc/BitMapSupport.c BitMapSize() bug!!!
   if (!SourceBitMap)
   {
      return INVALID_ID;
   }
#endif

#ifdef WRITEPIXEL
#define BMATags_PixelFormat                (TAG_USER+2)
#define PIXF_CLUT 1
#define PIXF_ALPHA8 21
   tmpBitMap = IGraphics->AllocBitMapTags(image_width, image_height, 8, BMATags_PixelFormat, PIXF_CLUT, BMATags_NoMemory, TRUE, BMATags_UserPrivate, TRUE, TAG_DONE);
   if (!tmpBitMap)
   {
printf("[amiga] allocating bitmap failed\n");
      return INVALID_ID;
   }
printf("tmpBitmap Planes[0] = %p\n", tmpBitMap->Planes[0]);
printf("tmpBitmap Planes[1] = %p\n", tmpBitMap->Planes[1]);
printf("tmpBitmap Planes[2] = %p\n", tmpBitMap->Planes[2]);
printf("tmpBitmap Planes[3] = %p\n", tmpBitMap->Planes[3]);
printf("tmpBitmap BytesPerRow = %d\n", tmpBitMap->BytesPerRow);
#endif

IDBUG("  My_Screen=0x%08lx\n",My_Screen);
if(!is_fullscreen) My_Screen = IIntuition->LockPubScreen(NULL);
IDBUG("  My_Screen=0x%08lx\n",My_Screen);
   if(My_Screen)
   //if ((My_Screen = IIntuition->LockPubScreen(NULL)))
   {
      APTR vi;
      ModeID = IGraphics->GetVPModeID(&My_Screen->ViewPort);

//      vi = IGadTools->GetVisualInfoA(My_Screen, NULL);
//      if (vi)
//      {
#if 0
         if (is_ontop==1)
            nm[6].nm_Flags = MENUTOGGLE|CHECKIT|CHECKED;
         else
            nm[6].nm_Flags = MENUTOGGLE|CHECKIT;
#endif

//         menu = IGadTools->CreateMenusA(nm,NULL);
//         if (menu)
//         {
//            if (IGadTools->LayoutMenus(menu, vi, GTMN_NewLookMenus, TRUE, TAG_DONE))
//            {
//               struct DrawInfo *dri;

//               if ((dri = IIntuition->GetScreenDrawInfo(My_Screen)))
//               {
                  static struct IBox zoombox = {0, 0, 9999, 9999};

                  zoombox.Width  = My_Screen->Width;
                  zoombox.Height = My_Screen->Height;

                  if (FirstTime)
                  {
                     uint32 bw, bh;

                     switch (BorderMode)
                     {
                        case NOBORDER:
                           bw = 0;
                           bh = 0;
                        break;

                        default:
                           bw = My_Screen->WBorLeft + My_Screen->WBorRight;
                           bh = My_Screen->WBorTop + My_Screen->Font->ta_YSize + 1 + My_Screen->WBorBottom;
                     }

                     window_left = (My_Screen->Width  - (window_width  + bw)) / 2;
                     window_top  = (My_Screen->Height - (window_height + bh)) / 2;
                     FirstTime = FALSE;
                  }

                  if (0 == pip_format)
                  {
IDBUG("  using OpenWindowTags()\n",NULL);
                     My_Window = IIntuition->OpenWindowTags(NULL,
#ifdef PUBLIC_SCREEN
 WA_PubScreen,       (ULONG) My_Screen,
#else
                        WA_CustomScreen,    (uint32) My_Screen,
#endif
                        //ALWAYSBORDER == BorderMode ? WA_Title : WA_WindowName, GetWindowTitle(),
                        ALWAYSBORDER == BorderMode ? WA_Title : TAG_IGNORE, GetWindowTitle(),
                        WA_ScreenTitle,     AMIGA_VERSION " (amiga)",//(uint32) "MPlayer",
                        WA_Left,            window_left,
                        WA_Top,             window_top,
                        WA_NewLookMenus,    TRUE,
//                        WA_SmartRefresh,    TRUE,
                        WA_CloseGadget,     ALWAYSBORDER == BorderMode ? TRUE : FALSE,
                        WA_DepthGadget,     ALWAYSBORDER == BorderMode ? TRUE : FALSE,
                        WA_DragBar,         ALWAYSBORDER == BorderMode ? TRUE : FALSE,
                        WA_Borderless,      NOBORDER     == BorderMode ? TRUE : FALSE,
is_fullscreen? WA_Backdrop : TAG_IGNORE, TRUE,
                        WA_SizeGadget,      ALWAYSBORDER == BorderMode ? TRUE : FALSE,
                        ALWAYSBORDER == BorderMode ? WA_SizeBBottom : TAG_IGNORE, TRUE,
                        WA_Activate,        TRUE,
                        WA_StayTop,         (is_ontop==1) ? TRUE : FALSE,
                        WA_IDCMP,           ALWAYSBORDER == BorderMode ? /*IDCMP_SIZEVERIFY | */IDCMP_CHANGEWINDOW | IDCMP_RAWKEY | IDCMP_VANILLAKEY | IDCMP_CLOSEWINDOW | IDCMP_MENUPICK | IDCMP_GADGETUP | IDCMP_MOUSEBUTTONS | IDCMP_EXTENDEDMOUSE
                                                                       : IDCMP_RAWKEY | IDCMP_VANILLAKEY | IDCMP_MENUPICK | IDCMP_GADGETUP | IDCMP_MOUSEBUTTONS | IDCMP_EXTENDEDMOUSE ,
//: IDCMP_MOUSEMOVE | IDCMP_MENUPICK | IDCMP_MENUVERIFY | IDCMP_MOUSEBUTTONS | IDCMP_RAWKEY | IDCMP_VANILLAKEY | IDCMP_EXTENDEDMOUSE | IDCMP_REFRESHWINDOW ,
WA_Flags,           WFLG_REPORTMOUSE,
                        //ALWAYSBORDER == BorderMode ? TAG_IGNORE : WA_Gadgets, &MyDragGadget,
                        WA_InnerWidth,      old_d_width,
                        WA_InnerHeight,     old_d_height,
                        WA_MinWidth,        160,
                        WA_MinHeight,       100,
                        WA_MaxWidth,        ~0,
                        WA_MaxHeight,       ~0,
                        //WA_BackFill,        backfillhook,
                        ALWAYSBORDER == BorderMode ? WA_Zoom : TAG_IGNORE, &zoombox,
//                        WA_DropShadows,     FALSE,
                        WA_Hidden,          hidden,
                     TAG_DONE);

                     /*if (My_Window && !hidden)
                     {
#if 0
                        IP96->p96RectFill(My_Window->RPort,
                                          My_Window->BorderLeft,
                                          My_Window->BorderTop,
                                          My_Window->Width - My_Window->BorderRight - 1,
                                          My_Window->Height - My_Window->BorderBottom - 1,
                                          0x00000000);
#endif
                     }*/
                  } else {
IDBUG("  using p96PIP_OpenTags()\n",NULL);
                     My_Window = IP96->p96PIP_OpenTags(
                        P96PIP_SourceFormat, pip_format,
                        P96PIP_SourceWidth,  image_width,
                        P96PIP_SourceHeight, image_height,
#ifndef SOURCE_BITMAP
                        P96PIP_NumberOfBuffers, 3,
#endif

#ifdef PUBLIC_SCREEN
 WA_PubScreen,       (ULONG) My_Screen,
#else
                        WA_CustomScreen,    (uint32) My_Screen,
#endif
                        //ALWAYSBORDER == BorderMode ? WA_Title : WA_WindowName, GetWindowTitle(),
                        ALWAYSBORDER == BorderMode ? WA_Title : TAG_IGNORE, GetWindowTitle(),
                        WA_ScreenTitle,     AMIGA_VERSION " (amiga)",//(uint32) "MPlayer",
                        WA_Left,            window_left,
                        WA_Top,             window_top,
                        WA_NewLookMenus,    TRUE,
//                        WA_SmartRefresh,    TRUE,
                        WA_CloseGadget,     ALWAYSBORDER == BorderMode ? TRUE : FALSE,
                        WA_DepthGadget,     ALWAYSBORDER == BorderMode ? TRUE : FALSE,
                        WA_DragBar,         ALWAYSBORDER == BorderMode ? TRUE : FALSE,
                        WA_Borderless,      NOBORDER     == BorderMode ? TRUE : FALSE,
is_fullscreen? WA_Backdrop : TAG_IGNORE , TRUE ,
                        WA_SizeGadget,      ALWAYSBORDER == BorderMode ? TRUE : FALSE,
                        ALWAYSBORDER == BorderMode ? WA_SizeBBottom : TAG_IGNORE , TRUE,
                        WA_Activate,        TRUE,
                        WA_StayTop,         (is_ontop==1) ? TRUE : FALSE,
                        WA_IDCMP,           ALWAYSBORDER == BorderMode ? /*IDCMP_SIZEVERIFY | */IDCMP_CHANGEWINDOW | IDCMP_RAWKEY | IDCMP_VANILLAKEY | IDCMP_CLOSEWINDOW | IDCMP_MENUPICK | IDCMP_GADGETUP | IDCMP_MOUSEBUTTONS | IDCMP_EXTENDEDMOUSE
                                                                       : IDCMP_RAWKEY | IDCMP_VANILLAKEY | IDCMP_MENUPICK | IDCMP_MOUSEBUTTONS | IDCMP_EXTENDEDMOUSE ,
WA_Flags,           WFLG_REPORTMOUSE,
                        //ALWAYSBORDER == BorderMode ? TAG_IGNORE : WA_Gadgets, &MyDragGadget,
                        WA_InnerWidth,      old_d_width,
                        WA_InnerHeight,     old_d_height,
                        WA_MinWidth,        160,
                        WA_MinHeight,       100,
                        ALWAYSBORDER == BorderMode ? WA_Zoom : TAG_IGNORE, &zoombox,
                        WA_DropShadows,     FALSE,
                        WA_Hidden,          hidden,
                     TAG_DONE);

//                     IIntuition->FreeScreenDrawInfo(My_Screen, dri);
//                  }
               }

               if (My_Window)
               {
if(BorderMode == ALWAYSBORDER)
{
open_icon( My_Window, ICONIFYIMAGE, GID_ICONIFY, &iconifyIcon );
open_icon( My_Window, SETTINGSIMAGE, GID_FULLSCREEN, &fullscreenicon );
//open_icon( My_Window, PADLOCKIMAGE, GID_PADLOCK, &padlockicon );
IIntuition->RefreshWindowFrame(My_Window); // or it won't show/render added gadgets
}

/*#if 0 // does not change anything
                  struct BitMap *bm;
                  uint32 numbuffers = 0, WorkBuf, DispBuf;
                  int32 lock;
                  struct YUVRenderInfo ri;

                  IP96->p96PIP_GetTags(My_Window, P96PIP_SourceBitMap, (uint32)&bm, P96PIP_NumberOfBuffers, &numbuffers,  P96PIP_WorkBuffer, &WorkBuf, P96PIP_DisplayedBuffer, &DispBuf, TAG_DONE);
                  lock = IP96->p96LockBitMap(bm, (uint8 *)&ri, sizeof(ri));
                  if (lock)
                  {
                     IGraphics->BltClear(ri.Planes[1], bm->Rows * ri.BytesPerRow[1] / 2, 0);

                     IP96->p96UnlockBitMap(bm, lock);

                     if (numbuffers > 1)
                     {
                        uint32 NextWorkBuf = (WorkBuf+1) % numbuffers;
                        IP96->p96PIP_SetTags(My_Window, P96PIP_VisibleBuffer, WorkBuf, P96PIP_WorkBuffer, NextWorkBuf, TAG_DONE);
                     }
                  }

#endif*/
                  //IIntuition->SetMenuStrip(My_Window, menu);
attach_menu(My_Window);
               }

//            }
//         }
//         IGadTools->FreeVisualInfo(vi);
//      }
      if(!is_fullscreen) IIntuition->UnlockPubScreen(NULL, My_Screen);
   }

   if (!My_Window)
   {
      mp_msg(MSGT_VO, MSGL_FATAL, "[amiga] Unable to open a window\n");
//      Close_Window();
      return INVALID_ID;
   }

   if (ModeID == INVALID_ID)
   {
//      Close_Window();
      return INVALID_ID;
   }

   IIntuition->ScreenToFront(My_Screen);

   return ModeID;
}


static ULONG Open_FullScreen(void)
{
//	ULONG left = 0, top = 0;
//	ULONG out_width = 0;
//	ULONG out_height = 0;
IDBUG("Open_FullScreen()\n",NULL);
	if(My_Window)
	{
		Close_Window();
	}
IDBUG("  vo_screenwidth=%ld  vo_screenheight=%ld\n",vo_screenwidth,vo_screenheight);
	My_Screen = IIntuition->OpenScreenTags(NULL,
	                         SA_LikeWorkbench, TRUE,
	                         SA_Type,          PUBLICSCREEN,
	                         SA_PubName,       CS(MSG_MPlayer_ScreenTitle),
	                         SA_Title,         CS(MSG_MPlayer_ScreenTitle),
	                         SA_ShowTitle,     FALSE,
	                         SA_Compositing,   FALSE, // No white stripes anymore, yeah! (fix from kas1e/Jörg Strohmayer)
	                         SA_Quiet,         TRUE,
is_fullscreen? SA_BackFill : TAG_IGNORE , backfillhook,
	                        TAG_END);
IDBUG("  My_Screen=0x%08lx\n",My_Screen);
	if(!My_Screen) { return INVALID_ID; }

if(is_fullscreen)
{
	// dirty way of "refreshing" screen so we get black bars
	IGraphics->RectFillColor(&(My_Screen->RastPort), 0, 0, My_Screen->Width, My_Screen->Height, 0xff000000);

	/*bf_Window = IIntuition->OpenWindowTags( NULL,
	                         WA_PubScreen,     (ULONG)My_Screen,
	                         WA_Top,0, WA_Left,0,
	                         WA_Height,        vo_screenheight,
	                         WA_Width,         vo_screenwidth,
	                         WA_SimpleRefresh, TRUE,
	                         WA_CloseGadget,   FALSE,
	                         WA_DragBar,       FALSE,
	                         WA_Borderless,    TRUE,
	                         WA_Backdrop,      TRUE,
	                         WA_Activate,      FALSE,
	                         //WA_BackFill,      backfillhook,
//	                         is_fullscreen? WA_BackFill : TAG_IGNORE , backfillhook,
	                        TAG_DONE);
//	if(!bf_Window) { return INVALID_ID; }
	IIntuition->CloseWindow(bf_Window);
	bf_Window=NULL;*/
}

	return IGraphics->GetVPModeID(&My_Screen->ViewPort);
}


extern uint32 BackFillfunc(struct Hook *hook UNUSED, struct RastPort *rp, struct BackFillMessage *msg); // cgx_common.c
// same function as in cgx_common.c ('cos we don't include cgx_common.h [yet|WIP])
/*static uint32 BackFillfunc(struct Hook *hook UNUSED, struct RastPort *rp, struct BackFillMessage *msg)
{
	struct RastPort newrp;

	// Remove any clipping
	newrp = *rp;
	newrp.Layer = NULL;
IDBUG("BackFillfunc()\n",NULL);
	if(!msg) return(0L);

	IGraphics->RectFillColor(&newrp, msg->Bounds.MinX, msg->Bounds.MinY,
	                         msg->Bounds.MaxX, msg->Bounds.MaxY, 0xff000000); // black color
	return(~0L);
}*/


/****************************/
#define NUM_MODES 4
static int preinit(const char *arg)
{
   struct Screen *screen;
   uint32 ModeID;

   mp_msg(MSGT_VO, MSGL_INFO, "VO: [amiga]\n");

   opt_border   = 2;
   opt_mode     = 0;
// #warning FixMe
//   opt_mode     = 1;

   if (subopt_parse(arg, subopts) != 0 || opt_help || opt_question)
   {
      mp_msg(MSGT_VO, MSGL_FATAL,
             "-vo amiga command line help:\n"
             "Example: mplayer -vo amiga:border=0:mode=3\n"
             "\nOptions:\n"
             "  border\n"
             "    0: Borderless window\n"
             "    1: Window with small borders\n"
             "    2: Normal window (default)\n"
             "  mode\n"
             "    0: YUV410 (default)\n"
             "    1: YUV420\n"
             "    2: YUV422\n"
             "    3: No overlay\n"
             "\n");
      return VO_ERROR;
   }

   switch (opt_border)
   {
      case 0: BorderMode = NOBORDER;     break;
      case 1: BorderMode = TINYBORDER;   break;
      case 2: BorderMode = ALWAYSBORDER; break;
      default: mp_msg(MSGT_VO, MSGL_FATAL, "[amiga] Unsupported 'border=%d' option\n", opt_border);
      return VO_ERROR;
   }

   switch (opt_mode)
   {
      case 0: pip_format = RGBFB_YUV410P;   break;
      case 1: pip_format = RGBFB_YUV420P;   break;
      case 2: pip_format = RGBFB_YUV422CGX; break;
      case 3: pip_format = 0; break;
      default: mp_msg(MSGT_VO, MSGL_FATAL, "[amiga] Unsupported 'mode=%d' option\n", opt_mode);
      return VO_ERROR;
   }

#ifdef USERTG
   RTGBase = (struct RTGBase *)IExec->OpenLibrary("rtg.library",  53);
   if (!RTGBase)
   {
      mp_msg(MSGT_VO, MSGL_FATAL, "[amiga] Couldn't open required libraries\n");
   }
#endif

   if ((screen = IIntuition->LockPubScreen(NULL)))
   {
      vo_screenwidth  = screen->Width;
      vo_screenheight = screen->Height;
//      vo_depthonscreen = 12;
//      IIntuition->GetScreenAttr(screen, SA_Depth,&vo_depthonscreen, sizeof(vo_depthonscreen));
      IIntuition->UnlockPubScreen(NULL, screen);
   }


	//backfillhook = { {NULL, NULL}, (HOOKFUNC)BackFillfunc, NULL, NULL };
	backfillhook = (struct Hook *)IExec->AllocSysObjectTags(ASOT_HOOK, ASOHOOK_Entry,BackFillfunc, TAG_END);
IDBUG("backfillhook = %p (alloc)\n",backfillhook);


   old_d_width = 640;
   old_d_height = 320;
   image_width = 640;
   image_height = 320;

   ModeID = Open_Window(TRUE);
   if (INVALID_ID == ModeID)
   {
      BOOL tested[NUM_MODES] = {FALSE};
      int i;

      tested[opt_mode] = TRUE;

      for (i = 0 ; i < NUM_MODES ; i++)
      {
         if (tested[i]) continue;
         tested[i] = TRUE;

         switch (i)
         {
            case 0: pip_format = RGBFB_YUV410P;   break;
            case 1: pip_format = RGBFB_YUV420P;   break;
            case 2: pip_format = RGBFB_YUV422CGX; break;
            case 3: pip_format = 0; break;
         }

         FirstTime = TRUE;
         ModeID = Open_Window(TRUE);
         if (INVALID_ID != ModeID)
         {
            break;
         }
      }
   }

   Close_Window();
   old_d_width = old_d_height = image_width = image_height = 0;
   FirstTime = TRUE;


   if (0 == pip_format)
   {
      int i;
      for (i = 0 ; i < 256 ; i++)
      {
         yuv2rgb[0][i] = floor(round((i -  16) * 1.164));
         yuv2rgb[1][i] = floor(round((i - 128) * 1.596));
         yuv2rgb[2][i] = floor(round((i - 128) * 0.391));
         yuv2rgb[3][i] = floor(round((i - 128) * 0.821));
         yuv2rgb[4][i] = floor(round((i - 128) * 2.017));
      }
/*#if 0
      vo_depthonscreen = 16;
   } else if (RGBFB_YUV410P == pip_format)
   {
      vo_depthonscreen =  9;
   } else if (RGBFB_YUV420P == pip_format)
   {
      vo_depthonscreen = 12;
   } else if (RGBFB_YUV422CGX == pip_format)
   {
      vo_depthonscreen = 16;
#endif*/
   }

   return 0;
}

/***************************** CONFIG *******************************/
static int config(uint32_t width, uint32_t height, uint32_t d_width,
             uint32_t d_height, uint32_t flags, UNUSED char *title,
             uint32_t format)
{
   uint32 ModeID;
//    printf("config: width: %d, height: %d, d_width: %d, d_height:%d\n",width, height, d_width, d_height);

   if (My_Window) Close_Window();
/*
   if (old_d_width==0 || old_d_height==0)
   {
       old_d_width     = d_width;
       old_d_height    = d_height;
   }
*/

   image_format = format;

   vo_fs = flags & VOFLAG_FULLSCREEN;

   image_width = width;
   image_height = height;
/*#if 0
   if (RGBFB_YUV410P == pip_format)
   {
      image_height &= ~3;
      d_height     &= ~3;
   } else (RGBFB_YUV420P == pip_format)
   {
      image_height &= ~1;
      d_height     &= ~1;
   }
#endif*/

   window_width = d_width;
   window_height = d_height;
//   vo_dwidth  = d_width;
//   vo_dheight = d_height;
//   vo_dbpp = 12;
//   aspect_save_orig(width,height);
//   aspect_save_prescale(d_width,d_height);
   aspect_save_screenres(vo_screenwidth, vo_screenheight);

//#if 0
IDBUG("vo_fs=is_fullscreen 0x%08lx\n",flags);
   if(flags&VOFLAG_FULLSCREEN) { /* -fs */
//   if(vo_fs) {
is_fullscreen = vo_fs;

      aspect(&d_width, &d_height, A_ZOOM);
      vo_fs = VO_TRUE;
   }
   else
//#endif
   {
      vo_fs = VO_FALSE;
      aspect(&d_width, &d_height, /*A_ZOOM*/ A_WINZOOM /*A_NOZOOM*/);
printf("%lu*%lu -> %d*%d\n", window_width, window_height, d_width, d_height);
      panscan_init();
      panscan_calc_windowed();
      d_height += vo_panscan_x;
      d_width  += vo_panscan_y;
printf("%lu*%lu -> %d*%d\n", window_width, window_height, d_width, d_height);
BorderMode = ALWAYSBORDER;
   }

   if (old_d_width==0 || old_d_height==0)
   {
       old_d_width     = d_width;
       old_d_height    = d_height;
   }
   window_width = d_width;
   window_height = d_height;


if(is_fullscreen  &&  (ModeID=Open_FullScreen())!=INVALID_ID)
{// window without borders in fullscreen mode
IDBUG("ModeID=0x%08lx  Open_FullScreen()\n",ModeID);
	BorderMode = NOBORDER;
}


   ModeID = Open_Window(FALSE);
IDBUG("ModeID=0x%08lx  Open_Window()\n",ModeID);
   if (INVALID_ID == ModeID)
   {
      return VO_ERROR;
   }

   R5G6B5_BitMap = IP96->p96AllocBitMap(image_width, image_height, 16, 0, NULL, RGBFB_R5G6B5);
   if (!R5G6B5_BitMap) return VO_ERROR;

   /*if (R5G6B5_BitMap)
   {
      uint32 lock;
      struct RenderInfo ri;
//      ri.Planes[2] = (void *)0xDEAD0BAD;
      if ((lock = IP96->p96LockBitMap(R5G6B5_BitMap, (uint8 *)&ri, sizeof(ri))))
      {
         uint32 board, locked;
         IP96->p96UnlockBitMap(R5G6B5_BitMap, lock);
#if 0
         printf("R5G6B5_BitMap\n");
         printf(" Memory %p, BytesPerRow %d\n", ri.Memory, ri.BytesPerRow);
         printf(" RGBFormat 0x%08x\n", ri.RGBFormat);
//         printf(" Planes[] %p %p %p\n", ri.Planes[0], ri.Planes[1], ri.Planes[2]);
//         printf(" BytesPerRow[] %d %d %d\n", ri.BytesPerRow[0], ri.BytesPerRow[1], ri.BytesPerRow[2]);
#endif

         board = IP96->p96GetModeIDAttr(ModeID, P96IDA_BOARDNUMBER);
         locked = IP96->p96LockBitMapToBoard(R5G6B5_BitMap, board, (uint8 *)&ri, sizeof(ri));
//         locked = 0;
#if 0
         printf(" Boardnumber %lu, locked = %lu\n", board, locked);
         if (locked)
         {
            printf(" Memory %p, BytesPerRow %d\n", ri.Memory, ri.BytesPerRow);
            printf(" RGBFormat 0x%08x\n", ri.RGBFormat);
//            printf(" Planes[] %p %p %p\n", ri.Planes[0], ri.Planes[1], ri.Planes[2]);
//            printf(" BytesPerRow[] %d %d %d\n", ri.BytesPerRow[0], ri.BytesPerRow[1], ri.BytesPerRow[2]);
         }
#endif
      }

//      IP96->p96FreeBitMap(R5G6B5_BitMap);
//      R5G6B5_BitMap = NULL;
   }*/

//    window_width = d_width;
//    window_height = d_height;
//    vo_dwidth  = d_width;
//    vo_dheight = d_height;
//    aspect(image_width, image_height, A_ZOOM);
#if 0
   vo_screenwidth  = d_width;
   vo_screenheight = d_height;
#endif

//   vo_draw_alpha_func = draw_alpha_yv12;
//   vo_draw_alpha_func = vo_get_draw_alpha(format);

   out_format = 0 == pip_format ? IMGFMT_RGB16 : IMGFMT_YV12;

   return 0;
}

/****************************/
static void gfxcopy(uint8 *s, uint8 *d, uint32 l)
{
   if (l >= 4)
   {
      uint32 i;
      if (l >= 8)
      {
#ifdef USERTG
         if (RTGBase && l >= 128)
         {
            RTGBase->CopyToVRAM(s, d, l);
            return;
         }
#endif
         if (((uint32)s & 3) || ((uint32)d & 7))
         {
            uint32 *sL = (uint32 *)s, *dL = (uint32 *)d, *asL = (uint32 *)(((uint32)s + 31) & ~31);
            asm volatile ("dcbt 0,%0" : : "r" (&asL[0]));
            for (i=l/(8*sizeof(uint32));i;i--)
            {
               uint32 t1, t2;
               if (i > 1) asm volatile ("dcbt 0,%0" : : "r" (&asL[8]));
               t1=sL[0]; t2=sL[1]; dL[0]=t1; dL[1]=t2; t1=sL[2]; t2=sL[3]; dL[2]=t1; dL[3]=t2;
               t1=sL[4]; t2=sL[5]; dL[4]=t1; dL[5]=t2; t1=sL[6]; t2=sL[7]; dL[6]=t1; dL[7]=t2;
               asL += 8; sL += 8; dL += 8;
            }
            l &= 8*sizeof(uint32)-1; s = (uint8 *)sL; d = (uint8 *)dL;
         } else {
            double *s64 = (double *)s, *d64 = (double *)d, *as64 = (double *)(((uint32)s + 31) & ~31);
            asm volatile ("dcbt 0,%0" : : "r" (&as64[0]));
            for (i=l/(4*sizeof(double));i;i--)
            {
               double t1, t2, t3, t4;
               if (i > 1) asm volatile ("dcbt 0,%0" : : "r" (&as64[4]));
               t1 = s64[0]; t2 = s64[1]; t3 = s64[2]; t4 = s64[3];
               d64[0] = t1; d64[1] = t2; d64[2] = t3; d64[3] = t4;
               as64 += 4; s64 += 4; d64 += 4;
            }
            l &= 4*sizeof(double)-1;
            for (i=l/sizeof(double);i;i--) *d64++ = *s64++;
            l &= sizeof(double)-1; s = (uint8 *)s64; d = (uint8 *)d64;
         }
      }
      if (l >= 4)
      {
         uint32 *sL = (uint32 *)s, *dL = (uint32 *)d;
         for (i=l/sizeof(uint32);i;i--) *dL++ = *sL++;
         l &= sizeof(uint32)-1; s = (uint8 *)sL; d = (uint8 *)dL;
      }
   }
   while (l--) *d++ = *s++;
}


static void gfxcopy_420P_410P(uint8 *s, uint8 *d, uint32 l)
{
   #define pL(a,b) (a&0xFF000000)|(a<<8&0xFF0000)|(b>>16&0xFF00)|(b>>8&0xFF)
   #define pQ(a,b) (((uint64)pL(a>>32,a))<<32)|pL(b>>32,b)
   if (l >= 4)
   {
      uint32 i;
      if (l >= 8)
      {
         if (((uint32)s & 3) || ((uint32)d & 7))
         {
            uint32 *sL = (uint32 *)s, *dL = (uint32 *)d, *asL = (uint32 *)(((uint32)s + 31) & ~31);
            asm volatile ("dcbt 0,%0" : : "r" (&asL[0]));
            for (i=l/(4*sizeof(uint32));i;i--)
            {
               uint32 t1, t2;
               if (i > 1) asm volatile ("dcbt 0,%0" : : "r" (&asL[8]));
               t1=sL[0]; t2=sL[1]; dL[0]=pL(t1, t2);
               t1=sL[2]; t2=sL[3]; dL[1]=pL(t1, t2);
               t1=sL[4]; t2=sL[5]; dL[2]=pL(t1, t2);
               t1=sL[6]; t2=sL[7]; dL[3]=pL(t1, t2);
               asL += 8; sL += 8; dL += 4;
            }
            l &= 4*sizeof(uint32)-1; s = (uint8 *)sL; d = (uint8 *)dL;
         } else {
            uint64 *s64 = (uint64 *)s;
            volatile double *d64 = (volatile double *)d, *as64 = (double *)(((uint32)s + 31) & ~31);
            asm volatile ("dcbt 0,%0" : : "r" (&as64[0]));
            for (i=l/(4*sizeof(double));i;i--)
            {
               uint64 t1, t2, t3, t4, d0, d1, d2, d3;
               if (i > 1) asm volatile ("dcbt 0,%0" : : "r" (&as64[4]));
               t1 = s64[0]; t2 = s64[1]; d0 = pQ(t1, t2);
               t3 = s64[2]; t4 = s64[3]; d1 = pQ(t3, t4);
               if (i > 2) asm volatile ("dcbt 0,%0" : : "r" (&as64[8]));
               t1 = s64[4]; t2 = s64[5]; d2 = pQ(t1, t2);
               t3 = s64[6]; t4 = s64[7]; d3 = pQ(t3, t4);
               d64[0] = *(volatile double *)&d0; d64[1] = *(volatile double *)&d1;
               d64[2] = *(volatile double *)&d2; d64[3] = *(volatile double *)&d3;
               as64 += 8; s64 += 8; d64 += 4;
            }
            l &= 4*sizeof(double)-1;
            for (i=l/sizeof(double);i;i--)
            {
               uint64 t1, t2, t3;
               t1 = s64[0]; t2 = s64[1]; t3 = pQ(t1, t2);
               d64[0] = *(volatile double *)&t3;
               s64 += 2; d64 += 1;
            }
            l &= sizeof(double)-1; s = (uint8 *)s64; d = (uint8 *)d64;
         }
      }
      if (l >= 4)
      {
         uint32 *sL = (uint32 *)s, *dL = (uint32 *)d;
         for (i=l/sizeof(uint32);i;i--)
         {
            uint32 t1 = sL[0]; uint32 t2 = sL[1];
            dL[0] = pL(t1, t2); sL += 2; dL += 1;
         }
         l &= sizeof(uint32)-1; s = (uint8 *)sL; d = (uint8 *)dL;
      }
   }
   while (l--)
   {
      *d++ = *s;
      s += 2;
   }
}


static void gfxcopy_420P_422CGX(uint8 *sY0, uint8 *sY1, uint8 *sU, uint8 *sV, uint8 *d1, uint8 *d2, uint32 l)
{
   #define pL0(Y,U,V) (Y    &0xFF000000)|(U>> 8&0xFF0000)|(Y>>8&0xFF00)|(V>>24&0xFF)
   #define pL1(Y,U,V) (Y<<16&0xFF000000)|(U    &0xFF0000)|(Y<<8&0xFF00)|(V>>16&0xFF)
   #define pL2(Y,U,V) (Y    &0xFF000000)|(U<< 8&0xFF0000)|(Y>>8&0xFF00)|(V>> 8&0xFF)
   #define pL3(Y,U,V) (Y<<16&0xFF000000)|(U<<16&0xFF0000)|(Y<<8&0xFF00)|(V    &0xFF)
   #define pQ0(Y,U,V) ((uint64)(pL0(Y>>32,U,V))<<32)|pL1(Y>>32,U,V)
   #define pQ1(Y,U,V) ((uint64)(pL2(Y    ,U,V))<<32)|pL3(Y    ,U,V)
   if (l >= 4)
   {
      uint32 i;
      if (((uint32)sY0 & 7) || ((uint32)sY1 & 7) || ((uint32)d1 & 7) || ((uint32)d2 & 7))
      {
         uint32 *d1L  = (uint32 *)d1,  *d2L = (uint32 *)d2;
         uint32 *Y0L  = (uint32 *)sY0, *Y1L = (uint32 *)sY1;
         uint32 *UL   = (uint32 *)sU,  *VL  = (uint32 *)sV;
         uint32 *aY0L = (uint32 *)(((uint32)sY0 + 31) & ~31);
         uint32 *aY1L = (uint32 *)(((uint32)sY1 + 31) & ~31);
         uint32 *aUL  = (uint32 *)(((uint32)sU  + 31) & ~31);
         uint32 *aVL  = (uint32 *)(((uint32)sV  + 31) & ~31);
         asm volatile ("dcbt 0,%0" : : "r" (&aY0L[0]));
         asm volatile ("dcbt 0,%0" : : "r" (&aY1L[0]));
         asm volatile ("dcbt 0,%0" : : "r" (&aUL[0]));
         asm volatile ("dcbt 0,%0" : : "r" (&aVL[0]));
         for (i=l/4;i;i--)
         {
            uint32 Y0, U, Y1, V;
            if (i > 1)
            {
               asm volatile ("dcbt 0,%0" : : "r" (&aY0L[8]));
               asm volatile ("dcbt 0,%0" : : "r" (&aY1L[8]));
               asm volatile ("dcbt 0,%0" : : "r" (&aUL[8]));
               asm volatile ("dcbt 0,%0" : : "r" (&aVL[8]));
            }
            Y0 = Y0L[0]; Y1 = Y1L[0]; U = UL[0]; V = VL[0];
            d1L[ 0]=pL0(Y0,U,V); d1L[ 1]=pL1(Y0,U,V); d2L[ 0]=pL0(Y1,U,V); d2L[ 1]=pL1(Y1,U,V);
            Y0 = Y0L[1]; Y1 = Y1L[1];
            d1L[ 2]=pL2(Y0,U,V); d1L[ 3]=pL3(Y0,U,V); d2L[ 2]=pL2(Y1,U,V); d2L[ 3]=pL3(Y1,U,V);
            Y0 = Y0L[2]; Y1 = Y1L[2]; U = UL[1]; V = VL[1];
            d1L[ 4]=pL0(Y0,U,V); d1L[ 5]=pL1(Y0,U,V); d2L[ 4]=pL0(Y1,U,V); d2L[ 5]=pL1(Y1,U,V);
            Y0 = Y0L[3]; Y1 = Y1L[3];
            d1L[ 6]=pL2(Y0,U,V); d1L[ 7]=pL3(Y0,U,V); d2L[ 6]=pL2(Y1,U,V); d2L[ 7]=pL3(Y1,U,V);
            Y0 = Y0L[4]; Y1 = Y1L[4]; U = UL[2]; V = VL[2];
            d1L[ 8]=pL0(Y0,U,V); d1L[ 9]=pL1(Y0,U,V); d2L[ 8]=pL0(Y1,U,V); d2L[ 9]=pL1(Y1,U,V);
            Y0 = Y0L[5]; Y1 = Y1L[5];
            d1L[10]=pL2(Y0,U,V); d1L[11]=pL3(Y0,U,V); d2L[10]=pL2(Y1,U,V); d2L[11]=pL3(Y1,U,V);
            Y0 = Y0L[6]; Y1 = Y1L[6]; U = UL[3]; V = VL[3];
            d1L[12]=pL0(Y0,U,V); d1L[13]=pL1(Y0,U,V); d2L[12]=pL0(Y1,U,V); d2L[13]=pL1(Y1,U,V);
            Y0 = Y0L[7]; Y1 = Y1L[7];
            d1L[14]=pL2(Y0,U,V); d1L[15]=pL3(Y0,U,V); d2L[14]=pL2(Y1,U,V); d2L[15]=pL3(Y1,U,V);
            d1L += 16; d2L  += 16;
            Y0L += 8;  aY0L += 8; Y1L += 8; aY1L += 8;
            UL  += 4;  aUL  += 4; VL  += 4; aVL  += 4;
         }
         l &= 4-1;
         d1  = (uint8 *)d1L; d2  = (uint8 *)d2L;
         sY0 = (uint8 *)Y0L; sY1 = (uint8 *)Y1L;
         sU  = (uint8 *)UL;  sV  = (uint8 *)VL;
      } else {
         volatile double *d1Q  = (volatile double *)d1,  *d2Q = (volatile double *)d2;
         uint64 *Y0Q  = (uint64 *)sY0, *Y1Q = (uint64 *)sY1;
         uint32 *UL   = (uint32 *)sU,   *VL = (uint32 *)sV;
         uint64 *aY0Q = (uint64 *)(((uint32)sY0 + 31) & ~31);
         uint64 *aY1Q = (uint64 *)(((uint32)sY1 + 31) & ~31);
         uint32 *aUL  = (uint32 *)(((uint32)sU  + 31) & ~31);
         uint32 *aVL  = (uint32 *)(((uint32)sV  + 31) & ~31);
         asm volatile ("dcbt 0,%0" : : "r" (&aY0Q[0]));
         asm volatile ("dcbt 0,%0" : : "r" (&aY1Q[0]));
         asm volatile ("dcbt 0,%0" : : "r" (&aUL[0]));
         asm volatile ("dcbt 0,%0" : : "r" (&aVL[0]));
         for (i=l/4;i;i--)
         {
            uint64 Y0, U, Y1, V, d10, d11, d20, d21;
            if (i > 1)
            {
               asm volatile ("dcbt 0,%0" : : "r" (&aY0Q[4]));
               asm volatile ("dcbt 0,%0" : : "r" (&aY1Q[4]));
               asm volatile ("dcbt 0,%0" : : "r" (&aUL[8]));
               asm volatile ("dcbt 0,%0" : : "r" (&aVL[8]));
            }
            Y0  = Y0Q[0]; Y1 = Y1Q[0]; U = UL[0]; V = VL[0];
            d10 = pQ0(Y0,U,V); d11 = pQ1(Y0,U,V); d20 = pQ0(Y1,U,V); d21 = pQ1(Y1,U,V);
            d1Q[0] = *(volatile double *)&d10; d1Q[1] = *(volatile double *)&d11;
            d2Q[0] = *(volatile double *)&d20; d2Q[1] = *(volatile double *)&d21;
            Y0  = Y0Q[1]; Y1 = Y1Q[1]; U = UL[1]; V = VL[1];
            d10 = pQ0(Y0,U,V); d11 = pQ1(Y0,U,V); d20 = pQ0(Y1,U,V); d21 = pQ1(Y1,U,V);
            d1Q[2] = *(volatile double *)&d10; d1Q[3] = *(volatile double *)&d11;
            d2Q[2] = *(volatile double *)&d20; d2Q[3] = *(volatile double *)&d21;
            Y0  = Y0Q[2]; Y1 = Y1Q[2]; U = UL[2]; V = VL[2];
            d10 = pQ0(Y0,U,V); d11 = pQ1(Y0,U,V); d20 = pQ0(Y1,U,V); d21 = pQ1(Y1,U,V);
            d1Q[4] = *(volatile double *)&d10; d1Q[5] = *(volatile double *)&d11;
            d2Q[4] = *(volatile double *)&d20; d2Q[5] = *(volatile double *)&d21;
            Y0  = Y0Q[3]; Y1 = Y1Q[3]; U = UL[3]; V = VL[3];
            d10 = pQ0(Y0,U,V); d11 = pQ1(Y0,U,V); d20 = pQ0(Y1,U,V); d21 = pQ1(Y1,U,V);
            d1Q[6] = *(volatile double *)&d10; d1Q[7] = *(volatile double *)&d11;
            d2Q[6] = *(volatile double *)&d20; d2Q[7] = *(volatile double *)&d21;
            d1Q += 8; d2Q  += 8;
            Y0Q += 4; aY0Q += 4; Y1Q += 4; aY1Q += 4; UL += 4; aUL += 4; VL += 4; aVL += 4;
         }
         l &= 4-1;
         d1  = (uint8 *)d1Q; d2  = (uint8 *)d2Q;
         sY0 = (uint8 *)Y0Q; sY1 = (uint8 *)Y1Q;
         sU  = (uint8 *)UL;  sV  = (uint8 *)VL;
      }
   }
   if (l)
   {
      uint32 *d1L = (uint32 *)d1,  *d2L = (uint32 *)d2;
      uint32 *Y0L = (uint32 *)sY0, *Y1L = (uint32 *)sY1;
      uint32 *UL  = (uint32 *)sU,  *VL  = (uint32 *)sV;
      while (l--)
      {
         uint32 Y0, U, Y1, V;
         Y0 = *Y0L++; Y1 = *Y1L++; U = *UL++; V = *VL++;
         *d1L++ = pL0(Y0,U,V); *d1L++ = pL1(Y0,U,V);
         *d2L++ = pL0(Y1,U,V); *d2L++ = pL1(Y1,U,V);
         Y0 = *Y0L++; Y1 = *Y1L++;
         *d1L++ = pL2(Y0,U,V); *d1L++ = pL3(Y0,U,V);
         *d2L++ = pL2(Y1,U,V); *d2L++ = pL3(Y1,U,V);
      }
   }
}


static void gfxcopy_YUV420P_RGB565(uint8 *sY0, uint8 *sY1, uint8 *sU, uint8 *sV, uint8 *d1, uint8 *d2, uint32 l)
{
   #define YUV2RGB2 R = (Y2+Cr)>>3;         G = (Y2-Cg1-Cg2)>>2;    B = (Y2+Cb)>>3; \
                    R = MAX(MIN(R, 31), 0); G = MAX(MIN(G, 63), 0); B = MAX(MIN(B, 31), 0)
   #define Y2(Y) Y2=yuv2rgb[0][Y]
   #define YUV2RGB(Y, U, V) Y2(Y); Cr=yuv2rgb[1][V]; Cg1=yuv2rgb[2][U]; Cg2=yuv2rgb[3][V]; Cb=yuv2rgb[4][U]; YUV2RGB2
   uint32 *d1L = (uint32 *)d1, *d2L = (uint32 *)d2, *Y0L = (uint32 *)sY0,
          *Y1L = (uint32 *)sY1, *UL = (uint32 *)sU, *VL = (uint32 *)sV, i, t;
   int32 Y2, Cr, Cg1, Cg2, Cb, R, G, B;
   uint16 *UW, *VW;
   for (i=l/4;i;i--)
   {
      uint32 Y0 = Y0L[0], Y1 = Y1L[0], U = UL[0], V = VL[0];
      YUV2RGB(Y0>>24    , U>>24     , V>>24     ); Y2(Y0>>16&0xFF); t  = (R<<11|G<<5|B)<<16;
      YUV2RGB2;                                                     t |=  R<<11|G<<5|B;
                                                   Y2(Y1>>24     ); d1L[0] = t;
      YUV2RGB2;                                    Y2(Y1>>16&0xFF); t  = (R<<11|G<<5|B)<<16;
      YUV2RGB2;                                                     t |=  R<<11|G<<5|B;
      YUV2RGB(Y0>>8&0xFF, U>>16&0xFF, V>>16&0xFF); Y2(Y0    &0xFF); d2L[0] = t;
                                                                    t  = (R<<11|G<<5|B)<<16;
      YUV2RGB2;                                                     t |=  R<<11|G<<5|B;
                                                   Y2(Y1>> 8&0xFF); d1L[1] = t;
      YUV2RGB2;                                    Y2(Y1    &0xFF); t  = (R<<11|G<<5|B)<<16;
      YUV2RGB2;                                                     t |=  R<<11|G<<5|B;
      Y0 = Y0L[1]; Y1 = Y1L[1];                                     d2L[1] = t;
      YUV2RGB(Y0>>24    , U>> 8&0xFF, V>> 8&0xFF); Y2(Y0>>16&0xFF); t  = (R<<11|G<<5|B)<<16;
      YUV2RGB2;                                                     t |=  R<<11|G<<5|B;
                                                   Y2(Y1>>24     ); d1L[2] = t;
                                                   Y2(Y1>>16&0xFF); t  = (R<<11|G<<5|B)<<16;
      YUV2RGB2;                                                     t |=  R<<11|G<<5|B;
      YUV2RGB(Y0>>8&0xFF, U    &0xFF, V    &0xFF); Y2(Y0    &0xFF); d2L[2] = t;
                                                                    t  = (R<<11|G<<5|B)<<16;
      YUV2RGB2;                                                     t |=  R<<11|G<<5|B;
                                                   Y2(Y1>> 8&0xFF); d1L[3] = t;
      YUV2RGB2;                                    Y2(Y1    &0xFF); t  = (R<<11|G<<5|B)<<16;
      YUV2RGB2;                                                     t |=  R<<11|G<<5|B;
      Y0L += 2; Y1L += 2; UL++; VL++;                               d2L[3] = t;
      d1L += 4; d2L += 4;
   }
   l &= 4-1; UW = (uint16 *)UL; VW = (uint16 *)VL;
   for (i=l/2;i;i--)
   {
      uint16 U = UW[0], V = VW[0]; uint32 Y0 = Y0L[0], Y1 = Y1L[0];
      YUV2RGB(Y0>>24    , U>>8  , V>>8  ); Y2(Y0>>16&0xFF); t  = (R<<11|G<<5|B)<<16;
      YUV2RGB2;                                             t |=  R<<11|G<<5|B;
                                           Y2(Y1>>24     ); d1L[0] = t;
      YUV2RGB2;                            Y2(Y1>>16&0xFF); t  = (R<<11|G<<5|B)<<16;
      YUV2RGB2;                                             t |=  R<<11|G<<5|B;
      YUV2RGB(Y0>>8&0xFF, U&0xFF, V&0xFF); Y2(Y0    &0xFF); d2L[0] = t;
                                                            t  = (R<<11|G<<5|B)<<16;
      YUV2RGB2;                                             t |=  R<<11|G<<5|B;
             ;                             Y2(Y1>> 8&0xFF); d1L[1] = t;
      YUV2RGB2;                            Y2(Y1    &0xFF); t  = (R<<11|G<<5|B)<<16;
      YUV2RGB2;                                             t |=  R<<11|G<<5|B;
      Y0L++; Y1L++; UW++; VW++;                             d2L[1] = t;
      d1L += 2; d2L += 2;
   }
   l &= 2-1;
   if (l)
   {
      uint8 U, V; uint16 Y0, Y1, *Y0W = (uint16 *)Y0L, *Y1W = (uint16 *)Y1L;
      sU = (uint8 *)UW; sV = (uint8 *)VW; Y0 = Y0W[0]; Y1 = Y1W[0]; U = sU[0]; V = sV[0];
      YUV2RGB(Y0>>8, U, V); Y2(Y0&0xFF); t  = (R<<11|G<<5|B)<<16;
      YUV2RGB2;                          t |=  R<<11|G<<5|B;
                            Y2(Y1>>8  ); d1L[0] = t;
      YUV2RGB2;             Y2(Y1&0xFF); t  = (R<<11|G<<5|B)<<16;
      YUV2RGB2;                          t |=  R<<11|G<<5|B;
                                         d2L[0] = t;
   }
}

/***************************** DRAW_OSD *****************************/

static void draw_osd(void)
{
   vo_draw_text(image_width, image_height, draw_alpha);
#warning fixme
#if 0
   if (0 == pip_format)
   {
      vo_draw_text_ext(window_width, window_height, 0, 0, 0, 0, image_width, image_height, draw_alpha);
// Only works if text is drawn into the window and not in the image RGB bitmap
// funktioniert nur wenn text ins fenster, und nicht die image RGB bitmap, gezeichnet wird
   } else {
      vo_draw_text_ext(image_width, image_height, 0, 0, 0, 0, image_width, image_height, draw_alpha);
   }
#endif
}

/***************************** FLIP_PAGE ****************************/
#if 0
static int32 currentBuffer;
#endif
static void flip_page(void)
{
// return;
#if 1
   uint32 numbuffers = 0, WorkBuf, DispBuf, NextWorkBuf;
   struct BitMap *bm;
   struct RastPort *rp;
#if 0
   struct YUVRenderInfo ri, sri;
   int32 lock1, lock2;
#endif
#ifdef SOURCE_BITMAP
   uint16 oldRowsSrc, oldRowsDst;
#else
#if 0
   struct YUVRenderInfo ri;
   int32 lock2;
#endif
#endif

#if 0
   currentBuffer--;
printf("[amiga] flip_page %d\n", currentBuffer);
#endif
   if (!My_Window)
   {
      printf("[amiga] no window\n");
      return;
   }

#ifdef SOURCE_BITMAP
   if (!SourceBitMap)
   {
      printf("[amiga] no bitmap\n");
      return;
   }
#endif

   if (0 == pip_format)
   {
//       IGraphics->BltBitMapRastPort(R5G6B5_BitMap, 0, 0, My_Window->RPort, My_Window->BorderLeft, My_Window->BorderTop, image_width, image_height, 0x0C0);
// mplayer -quiet -benchmark -vo pip:mode=3 AmigaOS.mpeg1video, int:
// BENCHMARKs: VC:   7.885s VO:  63.599s A:   0.000s Sys:   0.309s =   71.792s
// BENCHMARK%: VC: 10.9824% VO: 88.5875% A:  0.0000% Sys:  0.4301% = 100.0000%

//       IGraphics->BltBitMap(R5G6B5_BitMap, 0, 0, My_Window->RPort->BitMap,
//                            My_Window->LeftEdge + My_Window->BorderLeft, My_Window->TopEdge + My_Window->BorderTop,
//                            image_width, image_height, 0x0C0, 0xff, NULL);
// BENCHMARKs: VC:   8.311s VO:  62.456s A:   0.000s Sys:   0.348s =   71.114s
// BENCHMARK%: VC: 11.6865% VO: 87.8241% A:  0.0000% Sys:  0.4893% = 100.0000%

// *nothing*
// BENCHMARKs: VC:   7.853s VO:  20.756s A:   0.000s Sys:   0.308s =   28.918s
// BENCHMARK%: VC: 27.1578% VO: 71.7769% A:  0.0000% Sys:  1.0653% = 100.0000%

      uint32 left = My_Window->BorderLeft, top = My_Window->BorderTop, scaleX, scaleY;

      IIntuition->GetWindowAttrs(My_Window, WA_InnerWidth, &window_width, WA_InnerHeight, &window_height, TAG_DONE);

      scaleX = window_width  * COMP_FIX_ONE / image_width;
      scaleY = window_height * COMP_FIX_ONE / image_height;

      if (My_Window->RPort->Layer->BitMap)
      {
         IGraphics->CompositeTags(COMPOSITE_Src_Over_Dest, R5G6B5_BitMap, My_Window->RPort->Layer->BitMap,
                                  COMPTAG_DestX,      left,
                                  COMPTAG_DestY,      top,
                                  COMPTAG_OffsetX,    left,
                                  COMPTAG_OffsetY,    top,
                                  COMPTAG_DestWidth,  window_width,
                                  COMPTAG_DestHeight, window_height,
                                  COMPTAG_ScaleX,     scaleX,
                                  COMPTAG_ScaleY,     scaleY,
                                  TAG_DONE);

         IGraphics->BltBitMapRastPort(My_Window->RPort->Layer->BitMap, left, top, My_Window->RPort, left, top, window_width, window_height, 0x0C0);
// yuv2rgb[128*64*64]
// BENCHMARKs: VC:   8.197s VO:  16.774s A:   0.000s Sys:   0.322s =   25.293s
// BENCHMARK%: VC: 32.4075% VO: 66.3209% A:  0.0000% Sys:  1.2715% = 100.0000%
// yuv2rgb[64*64*64], too ugly
// BENCHMARKs: VC:   8.276s VO:  15.228s A:   0.000s Sys:   0.304s =   23.808s
// BENCHMARK%: VC: 34.7598% VO: 63.9616% A:  0.0000% Sys:  1.2786% = 100.0000%
// yuv2rgb[128*128*128]
// BENCHMARKs: VC:   8.019s VO:  17.287s A:   0.000s Sys:   0.304s =   25.610s
// BENCHMARK%: VC: 31.3130% VO: 67.4997% A:  0.0000% Sys:  1.1873% = 100.0000%
// yuv2rgb[5][256]
// BENCHMARKs: VC:   8.128s VO:  17.861s A:   0.000s Sys:   0.309s =   26.298s
// BENCHMARK%: VC: 30.9091% VO: 67.9173% A:  0.0000% Sys:  1.1736% = 100.0000%
// yuv2rgb[256*256*256]
// BENCHMARKs: VC:   8.304s VO:  20.583s A:   0.000s Sys:   0.312s =   29.200s
// BENCHMARK%: VC: 28.4398% VO: 70.4906% A:  0.0000% Sys:  1.0696% = 100.0000%
      } else {
         left += My_Window->LeftEdge; top += My_Window->TopEdge;
         IGraphics->CompositeTags(COMPOSITE_Src_Over_Dest, R5G6B5_BitMap, My_Window->RPort->BitMap,
                                  COMPTAG_DestX,      left,
                                  COMPTAG_DestY,      top,
                                  COMPTAG_OffsetX,    left,
                                  COMPTAG_OffsetY,    top,
                                  COMPTAG_DestWidth,  window_width,
                                  COMPTAG_DestHeight, window_height,
                                  COMPTAG_ScaleX,     scaleX,
                                  COMPTAG_ScaleY,     scaleY,
                                  TAG_DONE);
      }

      return;
   }

   IP96->p96PIP_GetTags(My_Window, P96PIP_NumberOfBuffers, &numbuffers,  P96PIP_WorkBuffer, &WorkBuf, P96PIP_DisplayedBuffer, &DispBuf, P96PIP_SourceBitMap, (uint32)&bm, P96PIP_SourceRPort, (uint32)&rp, TAG_DONE);

#if 0
   IGraphics->BltBitMap(SourceBitMap, 0, 0, bm, 0, 0, image_width, image_height, 0x0C0, 0xff, NULL);

   if (! (lock1 = IP96->p96LockBitMap(SourceBitMap, (uint8 *)&sri.RI, sizeof(struct YUVRenderInfo))))
   {
      // Unable to lock the BitMap -> do nothing
      printf("[amiga] locking failed\n");
      return;
   }
#endif

#ifndef SOURCE_BITMAP
#if 0
   if (! (lock2 = IP96->p96LockBitMap(bm, (uint8 *)&ri.RI, sizeof(struct YUVRenderInfo))))
   {
      // Unable to lock the BitMap -> do nothing
#ifdef SOURCE_BITMAP
      IP96->p96UnlockBitMap(SourceBitMap, lock1);
#endif
      printf("[amiga] locking failed\n");
      return;
   }
#endif
#endif

#ifdef SOURCE_BITMAP
#if 1
#if 1
   oldRowsSrc = SourceBitMap->Rows;
   oldRowsDst = bm->Rows;
   bm->Rows *= 2;
   SourceBitMap->Rows = bm->Rows;
   IGraphics->BltBitMap(SourceBitMap, 0, 0, bm, 0, 0, image_width, image_height * 2, 0x0C0, 0xff, NULL);
   IGraphics->WaitBlit();
   SourceBitMap->Rows = oldRowsSrc;
   bm->Rows = oldRowsDst;
#else
//    IGraphics->BltBitMap(SourceBitMap, 0, 0, bm, 0, 0, image_width, image_height, 0x0C0, 0xff, NULL);
//    gfxcopy(SourceBitMap->Planes[0] + SourceBitMap->BytesPerRow * image_height, bm->Planes[0] + bm->BytesPerRow * image_height, SourceBitMap->BytesPerRow * image_height / 2);
   gfxcopy(sri.Planes[1], ri.Planes[1], sri.BytesPerRow[1] / 2 * image_height);
#endif
#else
//    gfxcopy(sri.Planes[0], ri.Planes[0], sri.BytesPerRow[0] * SourceBitMap->Rows);
//    gfxcopy(sri.Planes[0], ri.Planes[0], ri.BytesPerRow[0] * bm->Rows);
   gfxcopy(sri.Planes[0], ri.Planes[0], sri.BytesPerRow[0] * image_height);
#if 0
   gfxcopy(sri.Planes[1], ri.Planes[1], sri.BytesPerRow[1] / 2 * bm->Rows);
   gfxcopy(sri.Planes[2], ri.Planes[2], sri.BytesPerRow[2] / 2 * bm->Rows);
#else
//    gfxcopy(sri.Planes[1], ri.Planes[1], sri.BytesPerRow[1] / 2 * SourceBitMap->Rows);
//    gfxcopy(sri.Planes[1], ri.Planes[1], ri.BytesPerRow[1] / 2 * bm->Rows);
   gfxcopy(sri.Planes[1], ri.Planes[1], sri.BytesPerRow[1] * image_height / 2);
#endif
#endif
#endif

   if (numbuffers > 1)
   {
      NextWorkBuf = (WorkBuf+1) % numbuffers;

//      if (NextWorkBuf==DispBuf) IGraphics->WaitTOF();

      IP96->p96PIP_SetTags(My_Window, P96PIP_VisibleBuffer, WorkBuf, P96PIP_WorkBuffer, NextWorkBuf, TAG_DONE);
   }

#ifdef SOURCE_BITMAP
#if 0
   IP96->p96UnlockBitMap(bm, lock2);
#endif
#endif
#if 0
   IP96->p96UnlockBitMap(SourceBitMap, lock1);
#endif

#if 0
static int printed = 0;
if (!printed)
{
   printed = 1;
   printf("s: %p %d*%d, %p %d*%d, %p %d, %p %d, %p %d\n", SourceBitMap->Planes[0], SourceBitMap->BytesPerRow, SourceBitMap->Rows, sri.RI.Memory, sri.RI.BytesPerRow, SourceBitMap->Rows, sri.Planes[0], sri.BytesPerRow[0], sri.Planes[1], sri.BytesPerRow[1], sri.Planes[2], sri.BytesPerRow[2]);
   printf("d: %p %d*%d, %p %d*%d, %p %d, %p %d, %p %d\n", bm->Planes[0], bm->BytesPerRow, bm->Rows, ri.RI.Memory, ri.RI.BytesPerRow, bm->Rows, ri.Planes[0], ri.BytesPerRow[0], ri.Planes[1], ri.BytesPerRow[1], ri.Planes[2], ri.BytesPerRow[2]);
/*
s: 0x57a75000 640*266, 0x57a75000 640, 0x57a9e900 640, 0x57a9ea40 640
d: 0xa932c2c0 640*266, 0xa932c2c0 640, 0xa9355bc0 640, 0xa9355d00 640
s: 0x580e5000 640*532, 0x580e5000 640*532, 0x580e5000 640, 0x58138200 640, 0x58138340 640
d: 0xa932c2c0 640*266, 0xa932c2c0 640*266, 0xa932c2c0 640, 0xa9355bc0 640, 0xa9355d00 640
*/
}
#endif
#endif
}

/**************************** GET_IMAGE *****************************/
static uint32_t get_image(UNUSED mp_image_t *mpi)
{
return VO_NOTIMPL;
#if 0
   currentBuffer++;
   printf("get_image: currentBuffer %d\n", currentBuffer);
   printf("flags 0x%x, type 0x%x, number %d\n", mpi->flags, mpi->type, mpi->number);
   printf("bpp %d, imgfmt 0x%x '%c%c%c%c'\n", mpi->bpp, mpi->imgfmt, mpi->imgfmt >> 24, (mpi->imgfmt >> 16) & 0xFF, (mpi->imgfmt >> 8) & 0xFF, mpi->imgfmt & 0xFF);
   printf("%d*%d, %d,%d %d*%d\n", mpi->width, mpi->height, mpi->x, mpi->y, mpi->w, mpi->h);
   printf("planes %p %p %p %p\n", mpi->planes[0], mpi->planes[1], mpi->planes[2], mpi->planes[3]);
   printf("stride %d %d %d %d\n", mpi->stride[0], mpi->stride[1], mpi->stride[2], mpi->stride[3]);
   printf("qscale %p, qstride %d\n", mpi->qscale, mpi->qstride);
   printf("pict_type %d, fields %d, qscale_type %d, num_planes %d\n", mpi->pict_type, mpi->fields, mpi->qscale_type, mpi->num_planes);
   printf("chroma_width %d, chroma_height %d, chroma_x_shift %d, chroma_y_shift %d\n", mpi->chroma_width, mpi->chroma_height, mpi->chroma_x_shift, mpi->chroma_y_shift);
   printf("usage_count %d, priv %p\n", mpi->usage_count, mpi->priv);

   if (currentBuffer > 1)
   {
      currentBuffer++;
      flip_page();
   }

#if 0
   if ((mpi->flags & MP_IMGFLAG_READABLE)
    || ((mpi->type != MP_IMGTYPE_STATIC && mpi->type != MP_IMGTYPE_TEMP &&
         mpi->type != MP_IMGTYPE_IPB &&
         mpi->type != MP_IMGTYPE_NUMBERED)))
#else
   if (mpi->type != MP_IMGTYPE_STATIC &&
       mpi->type != MP_IMGTYPE_TEMP &&
       mpi->type != MP_IMGTYPE_IPB &&
       mpi->type != MP_IMGTYPE_NUMBERED)
#endif
   {
printf("false\n");
      return VO_FALSE;
   }

   if (mpi->flags & MP_IMGFLAG_DRAW_CALLBACK)
   {
printf("callback\n");
      return VO_TRUE;
   }

   struct YUVRenderInfo ri;
   struct BitMap *bm = NULL;
   int32 lock_h;

#warning FixMe for 0 == pip_format
   IP96->p96PIP_GetTags(My_Window, P96PIP_SourceBitMap, (uint32)&bm, TAG_DONE);
   if (! (lock_h = IP96->p96LockBitMap(bm, (uint8 *)&ri.RI, sizeof(struct YUVRenderInfo))))
   {
      printf("[amiga] locking failed\n");
      return VO_ERROR;
   }

//   mpi->width  = image_width;
//   mpi->height = image_height;
   mpi->flags |= MP_IMGFLAG_DIRECT;
   mpi->flags |= MP_IMGFLAG_COMMON_STRIDE | MP_IMGFLAG_COMMON_PLANE;
   mpi->planes[0] = ri.Planes[0];
   mpi->stride[0] = ri.BytesPerRow[0];
   mpi->planes[1] = ri.Planes[1];
   mpi->stride[1] = ri.BytesPerRow[1];
   mpi->planes[2] = ri.Planes[2];
   mpi->stride[2] = ri.BytesPerRow[2];

   IP96->p96UnlockBitMap(bm, lock_h);

//   mpi->flags |= MP_IMGFLAG_DIRECT;
//   mpi->stride[0] = mode_stride;
//   mpi->planes[0] = PageStore[page].vbase + y_pos*mode_stride + (x_pos*mpi->bpp)/8;
////   mpi->priv=(void *)(uintptr_t)page;
////   mp_msg(MSGT_VO,MSGL_DBG3, "vo_svga: direct render allocated! page=%d\n",page);

printf("planes %p %p %p %p\n", mpi->planes[0], mpi->planes[1], mpi->planes[2], mpi->planes[3]);
printf("stride %d %d %d %d\n", mpi->stride[0], mpi->stride[1], mpi->stride[2], mpi->stride[3]);
   return VO_TRUE;
//   return VO_FALSE;
//   return VO_NOTIMPL;
#endif
}

/**************************** DRAW_IMAGE ****************************/
static int32 draw_image(mp_image_t *mpi)
{
//   printf("draw_image:\n");
//   printf("flags 0x%x, type 0x%x, number %d\n", mpi->flags, mpi->type, mpi->number);
//   printf("bpp %d, imgfmt 0x%x '%c%c%c%c'\n", mpi->bpp, mpi->imgfmt, mpi->imgfmt >> 24, (mpi->imgfmt >> 16) & 0xFF, (mpi->imgfmt >> 8) & 0xFF, mpi->imgfmt & 0xFF);
//   printf("%d*%d, %d,%d %d*%d\n", mpi->width, mpi->height, mpi->x, mpi->y, mpi->w, mpi->h);
//   printf("planes %p %p %p %p\n", mpi->planes[0], mpi->planes[1], mpi->planes[2], mpi->planes[3]);
//   printf("stride %d %d %d %d\n", mpi->stride[0], mpi->stride[1], mpi->stride[2], mpi->stride[3]);
//   printf("qscale %p, qstride %d\n", mpi->qscale, mpi->qstride);
//   printf("pict_type %d, fields %d, qscale_type %d, num_planes %d\n", mpi->pict_type, mpi->fields, mpi->qscale_type, mpi->num_planes);
//   printf("chroma_width %d, chroma_height %d, chroma_x_shift %d, chroma_y_shift %d\n", mpi->chroma_width, mpi->chroma_height, mpi->chroma_x_shift, mpi->chroma_y_shift);
//   printf("usage_count %d, priv %p\n", mpi->usage_count, mpi->priv);

   struct YUVRenderInfo ri;
   struct BitMap *bm = NULL;
#ifdef WRITEPIXEL
   struct RastPort *rp = NULL;
#endif
   int32 i, lock_h;
   uint8 *srcY, *srcU, *srcV, *dst, *dstU, *dstV;

   uint8_t **image = mpi->planes;
   int *stride = mpi->stride;
   int w = mpi->width;
   int h = mpi->height;
   int x = mpi->x;
   int y = mpi->y;

   if (!My_Window)
   {
      printf("[amiga] no window\n");
      return VO_ERROR;
   }

   if (mpi->flags & MP_IMGFLAG_DIRECT)
   {
printf("[amiga] direct\n");
      return VO_TRUE;
   }


#ifdef WRITEPIXEL
   IP96->p96PIP_GetTags(My_Window, P96PIP_SourceBitMap, (uint32)&bm, P96PIP_SourceRPort, (uint32)&rp, TAG_DONE);
#else
   if (0 != pip_format)
   {
      IP96->p96PIP_GetTags(My_Window, P96PIP_SourceBitMap, (uint32)&bm, TAG_DONE);
   }
#endif

   if (x > 0 || w < (int)image_width)
   {
      printf("[amiga] x=%d, w=%d\n", x, w);
      return VO_ERROR;
   }

#if 1
   if ((y + h) > (int)image_height)
   {
      int newh = image_height - y;
      printf("[amiga] y=%d h=%d -> h=%d\n", y, h, newh);
      h = newh;
   }

   if ((x + w) > (int)image_width)
   {
      int neww = image_width - x;
      printf("[amiga] x=%d w=%d -> w=%d\n", x, w, neww);
      w = neww;
   }
#endif

   if (x < 0 || y < 0 || (x + w) > (int)image_width || (y + h) > (int)image_height)
   {
      printf("[amiga] x=%d, y=%d %dx%d\n", x, y, w, h);
      return VO_ERROR;
   }

   if (stride[0] < 0 || stride[1] < 0 || stride[2] < 0)
   {
      printf("[amiga] %d %d %d\n", stride[0], stride[1], stride[2]);
      return VO_ERROR;
   }

#ifdef SOURCE_BITMAP
   if (!SourceBitMap)
   {
      printf("[amiga] no bitmap\n");
      return VO_ERROR;
   }
#endif

#if 1
   if (0 == pip_format)
   {
      struct RenderInfo RGBri;
      if (!(lock_h = IP96->p96LockBitMap(R5G6B5_BitMap, (uint8 *)&RGBri, sizeof(RGBri))))
      {
         printf("[amiga] locking failed\n");
         return VO_ERROR;
      }

      srcY = image[0];
      srcU = image[1];
      srcV = image[2];
      dst  = (uint8 *)RGBri.Memory + y * RGBri.BytesPerRow * 2;

      for (i = 0 ; i < (h + 1) / 2 ; i++)
      {
         gfxcopy_YUV420P_RGB565(srcY, srcY + stride[0], srcU, srcV, dst, dst + RGBri.BytesPerRow, (w + 1) / 2);

         srcY += stride[0] * 2;
         srcU += stride[1];
         srcV += stride[2];
         dst  += RGBri.BytesPerRow * 2;
      }

      IP96->p96UnlockBitMap(R5G6B5_BitMap, lock_h);

      return VO_TRUE;
   }
   /*if (0) //(0 == pip_format)
   {
#if 0
      struct YUVRenderInfo sri = { {image[0], stride[0], 0, RGBFB_CLUT/ * RGBFB_YUV420P * /}, {image[0], image[1], image[2]}, {stride[0], stride[1], stride[2]}, 0};
      IP96->p96WritePixelArray((struct RenderInfo *)&sri, x, y, My_Window->RPort, My_Window->BorderLeft, My_Window->BorderTop, w, h);
#endif
#if 0
      struct BitMap fakebm;
      fakebm.BytesPerRow = stride[0];
      fakebm.Rows = h;
      fakebm.Flags = 0;
      fakebm.Depth = 12;
      fakebm.Planes[0] = image[0];
      fakebm.Planes[1] = image[1];
      fakebm.Planes[2] = image[2];
      fakebm.Planes[3] = NULL;
      fakebm.Planes[4] = NULL;
      fakebm.Planes[5] = NULL;
      fakebm.Planes[6] = NULL;
      fakebm.Planes[7] = NULL;

      IGraphics->CompositeTags(COMPOSITE_Src_Over_Dest, &fakebm, My_Window->RPort->BitMap,
                               COMPTAG_SrcWidth, w,
                               COMPTAG_SrcHeight, h,
                               COMPTAG_DestX, My_Window->BorderLeft + x,
                               COMPTAG_DestY, My_Window->BorderTop + y,
                               COMPTAG_DestWidth, My_Window->Width - My_Window->BorderRight,
                               COMPTAG_DestHeight, My_Window->Height - My_Window->BorderBottom,
                               TAG_DONE);
#endif
#if 0
     IGraphics->BltBitMapTags(BLITA_Source, image[0],
                              BLITA_SrcBytesPerRow, stride[0],
                              BLITA_SrcType, BLITT_ALPHATEMPLATE, // BLITT_CHUNKY,
                              BLITA_Dest, My_Window->RPort,
                              BLITA_DestType, BLITT_RASTPORT,
                              BLITA_DestX, My_Window->BorderLeft + x,
                              BLITA_DestY, My_Window->BorderTop + y,
                              BLITA_Width, w,
                              BLITA_Height, h,
                              TAG_DONE);
#endif
//      ri = { {image[0], stride[0], 0, RGBFB_CLUT/ * RGBFB_YUV420P * /}, {image[0], image[1], image[2]}, {stride[0], stride[1], stride[2]}, 0};
//      if (!(lock_h = IP96->p96LockBitMap(SourceBitMap, (uint8 *)&ri, sizeof(ri))))
      if (!(lock_h = IP96->p96LockBitMap(R5G6B5_BitMap, (uint8 *)&ri, sizeof(ri))))
      {
         printf("[amiga] locking failed\n");
         return VO_ERROR;
      }

      srcY = image[0];
      srcU = image[1];
      srcV = image[2];
      dst  = (uint8 *)ri.RI.Memory + y * ri.RI.BytesPerRow;
//      dst  = (uint8 *)ri.Planes[0] + y * ri.BytesPerRow[0];
//      dstU = (uint8 *)ri.Planes[1] + y * ri.BytesPerRow[1] / 2;
//      dstV = (uint8 *)ri.Planes[2] + y * ri.BytesPerRow[2] / 2;

      for (i = 0 ; i < (h + 1) / 2 ; i++)
//      for (i = 0 ; i < h ; i++)
      {
//         memcpy(dst, srcY, w);
         gfxcopy_420P_422CGX(srcY, srcY + stride[0], srcU, srcV, dst, dst + ri.RI.BytesPerRow, (w + 7) / 8);
//         gfxcopy_420P_422CGX(srcY, srcU, srcV, dst, (w + 3) / 4);
         if (i < (h + 1) / 2)
         {
//            memcpy(dstU, srcU, (w + 1) / 2);
//            memcpy(dstV, srcV, (w + 1) / 2);
         }

         srcY += stride[0] * 2;
//         srcY += stride[0];
         srcU += stride[1];
         srcV += stride[2];
//         dst  += ri.RI.BytesPerRow;
         dst  += ri.RI.BytesPerRow * 2;
//         dst  += ri.BytesPerRow[1];
//         dstU += ri.BytesPerRow[1];
//         dstV += ri.BytesPerRow[2];
      }

#if 0
#if 0
      IP96->p96UnlockBitMap(SourceBitMap, lock_h);

      IGraphics->CompositeTags(COMPOSITE_Src_Over_Dest, SourceBitMap, My_Window->RPort->BitMap,
                               COMPTAG_DestX, My_Window->LeftEdge + My_Window->BorderLeft + x,
                               COMPTAG_DestY, My_Window->TopEdge + My_Window->BorderTop + y,
                               COMPTAG_DestWidth, My_Window->Width - My_Window->BorderRight,
                               COMPTAG_DestHeight, My_Window->Height - My_Window->BorderBottom,
                               COMPTAG_FriendBitMap, My_Window->RPort->BitMap,
                               TAG_DONE);
#else
      IP96->p96WritePixelArray((struct RenderInfo *)&ri, x, y, My_Window->RPort, My_Window->BorderLeft, My_Window->BorderTop, w, h);

      IP96->p96UnlockBitMap(SourceBitMap, lock_h);
#endif
#endif
      IP96->p96UnlockBitMap(R5G6B5_BitMap, lock_h);

#if 1 // Just black with YUV420P, OK with R5G6B5_BitMap!
      IGraphics->BltBitMapRastPort(R5G6B5_BitMap, 0, 0, My_Window->RPort, My_Window->BorderLeft, My_Window->BorderTop, w, h, 0x0C0);
#else
      IGraphics->CompositeTags(COMPOSITE_Src_Over_Dest, R5G6B5_BitMap, My_Window->RPort->BitMap,
                               COMPTAG_DestX, My_Window->LeftEdge + My_Window->BorderLeft + x,
                               COMPTAG_DestY, My_Window->TopEdge + My_Window->BorderTop + y,
                               COMPTAG_DestWidth, My_Window->Width - My_Window->BorderLeft - My_Window->BorderRight,
                               COMPTAG_DestHeight, My_Window->Height - My_Window->BorderTop - My_Window->BorderBottom,
                               COMPTAG_FriendBitMap, My_Window->RPort->BitMap,
                               TAG_DONE);
#endif

      return VO_TRUE;
   }*/
#endif


#ifndef SOURCE_BITMAP
//   IGraphics->WaitBlit();
   if (! (lock_h = IP96->p96LockBitMap(bm, (uint8 *)&ri.RI, sizeof(struct YUVRenderInfo))))
#else
   if (! (lock_h = IP96->p96LockBitMap(SourceBitMap, (uint8 *)&ri.RI, sizeof(struct YUVRenderInfo))))
#endif
   {
      // Unable to lock the BitMap -> do nothing
      printf("[amiga] locking failed\n");
      return VO_ERROR;
   }

   srcY = image[0];
   srcU = image[1];
   srcV = image[2];
   if (RGBFB_YUV422CGX == pip_format)
   {
      dst = (uint8 *)ri.RI.Memory + y * ri.RI.BytesPerRow * 2;
   } else {
      dst = (uint8 *)ri.Planes[0] + y * ri.BytesPerRow[0];
   }
#ifndef SOURCE_BITMAP
   // YUV420P and YUV410P only
   dstU = (uint8 *)ri.Planes[1] + y * ri.BytesPerRow[1] / 2;
   dstV = (uint8 *)ri.Planes[2] + y * ri.BytesPerRow[2] / 2;
#else
   if (RGBFB_YUV410P == pip_format)
   {
      dstU = (uint8 *)ri.Planes[0] + image_height * ri.BytesPerRow[0] + y * ri.BytesPerRow[0] / 4;
      dstV = (uint8 *)ri.Planes[1] + image_height * ri.BytesPerRow[1] + y * ri.BytesPerRow[1] / 4;
   } else if (RGBFB_YUV420P == pip_format)
   {
      dstU = (uint8 *)ri.Planes[0] + image_height * ri.BytesPerRow[0] + y * ri.BytesPerRow[0] / 2;
      dstV = (uint8 *)ri.Planes[1] + image_height * ri.BytesPerRow[1] + y * ri.BytesPerRow[1] / 2;
   } else if (RGBFB_YUV422CGX == pip_format)
      /* not used */
   } else {
      printf("[amiga] Unsupported pip_format 0x%x\n", pip_format);
      IP96->p96UnlockBitMap(bm, lock_h);
      return VO_ERROR;
   }
#endif

   if ((RGBFB_YUV410P == pip_format || RGBFB_YUV420P == pip_format) && stride[0] == ri.BytesPerRow[0] && stride[0] == w)
   {
#ifndef SOURCE_BITMAP
      if (RGBFB_YUV410P == pip_format)
      {
         int32 h2 = (h + 3) / 4;
         int32 w2 = (w + 3) / 4;

#ifdef WRITEPIXEL
         PLANEPTR oldPlane1= tmpBitMap->Planes[1];
         uint16 oldPad = rp->BitMap->pad;
         tmpBitMap->Planes[0] = image[0];
         tmpBitMap->Planes[1] = image[0];
         rp->BitMap->pad &= 0xfcc0;
         rp->BitMap->pad |= tmpBitMap->pad & 0x001f;
//         struct YUVRenderInfo sri = { {image[0], stride[0], 0, RGBFB_YUV420P}, {image[0], image[1], image[2]}, {stride[0], stride[1], stride[2]}, 0};
//         IP96->p96WritePixelArray(&sri.RI, 0, 0, rp, x, y, w, h);
         IGraphics->BltBitMap(tmpBitMap, 0, 0, bm, 0, 0, w, h, 0x0C0, 0xff, NULL);
         tmpBitMap->Planes[1] = oldPlane1;
         rp->BitMap->pad = oldPad;
#if 0
//         struct YUVRenderInfo sri = { {image[0], stride[0], 0, RGBFB_YUV410P}, {image[0], image[1], image[2]}, {stride[0], stride[1], stride[2]}, 0};
         struct RenderInfo sri = {image[0], stride[0], 0, RGBFB_CLUT};
         IP96->p96WritePixelArray(&sri, 0, 0, rp, x, y, w, h);
#endif
#else
         gfxcopy(srcY, dst, w * h);
#endif

         for (i = 0 ; i < h2 ; i++)
         {
            gfxcopy_420P_410P(srcU, dstU, w2);
            gfxcopy_420P_410P(srcV, dstV, w2);

            srcU += stride[1] * 2;
            srcV += stride[2] * 2;
            dstU += ri.BytesPerRow[1];
            dstV += ri.BytesPerRow[2];
         }
      } else if (RGBFB_YUV420P == pip_format)
      {
#ifdef WRITEPIXEL
         PLANEPTR oldPlane1= tmpBitMap->Planes[1];
         uint16 oldPad = rp->BitMap->pad;
         tmpBitMap->Planes[1] = image[0];
         rp->BitMap->pad &= 0xfcc0;
         rp->BitMap->pad |= tmpBitMap->pad & 0x001f;
//         struct YUVRenderInfo sri = { {image[0], stride[0], 0, RGBFB_YUV420P}, {image[0], image[1], image[2]}, {stride[0], stride[1], stride[2]}, 0};
//         IP96->p96WritePixelArray(&sri.RI, 0, 0, rp, x, y, w, h);
         IGraphics->BltBitMap(tmpBitMap, 0, 0, bm, 0, 0, w, h, 0x0C0, 0xff, NULL);
         tmpBitMap->Planes[1] = oldPlane1;
         rp->BitMap->pad = oldPad;
#else
         int32 h2 = (h + 1) / 2;
         int32 w2 = (w + 1) / 2;

         gfxcopy(srcY, dst, w * h);

         for (i = 0 ; i < h2 ; i++)
         {
            gfxcopy(srcU, dstU, w2);
            gfxcopy(srcV, dstV, w2);

            srcU += stride[1];
            srcV += stride[2];
            dstU += ri.BytesPerRow[1];
            dstV += ri.BytesPerRow[2];
         }
#endif
      } else
      {
         printf("[amiga] pip_format 0x%lx unsupported\n", pip_format);
         IP96->p96UnlockBitMap(bm, lock_h);
         return VO_ERROR;
      }
#else
      memcpy(dst, srcY, stride[0] * h);

      for (i = 0 ; i < h ; i++)
      {
         memcpy(dstU, srcU, (w + 1) / 2);
         memcpy(dstV, srcV, (w + 1) / 2);

         srcU += stride[1];
         srcV += stride[2];
         dstU += ri.BytesPerRow[1];
         dstV += ri.BytesPerRow[2];
      }
#endif
   } else
   {
#ifndef SOURCE_BITMAP
      if (RGBFB_YUV422CGX == pip_format)
      {
         for (i = 0 ; i < (h + 1) / 2 ; i++)
         {
            gfxcopy_420P_422CGX(srcY, srcY + stride[0], srcU, srcV, dst, dst + ri.RI.BytesPerRow, (w + 7) / 8);

            srcY += stride[0] * 2;
            srcU += stride[1];
            srcV += stride[2];
            dst  += ri.RI.BytesPerRow * 2;
         }
      } else {
         for (i = 0 ; i < h ; i++)
         {
            gfxcopy(srcY, dst, w);
            srcY += stride[0];
            dst  += ri.BytesPerRow[0];
         }
      }

      if (RGBFB_YUV410P == pip_format)
      {
         int32 h2 = (h + 3) / 4;
         int32 w2 = (w + 3) / 4;

         for (i = 0 ; i < h2 ; i++)
         {
            gfxcopy_420P_410P(srcU, dstU, w2);
            gfxcopy_420P_410P(srcV, dstV, w2);

            srcU += stride[1] * 2;
            srcV += stride[2] * 2;
            dstU += ri.BytesPerRow[1];
            dstV += ri.BytesPerRow[2];
         }
      } else if (RGBFB_YUV420P == pip_format)
      {
         int32 h2 = (h + 1) / 2;
         int32 w2 = (w + 1) / 2;

         for (i = 0 ; i < h2 ; i++)
         {
            gfxcopy(srcU, dstU, w2);
            gfxcopy(srcV, dstV, w2);

            srcU += stride[1];
            srcV += stride[2];
            dstU += ri.BytesPerRow[1];
            dstV += ri.BytesPerRow[2];
         }
      }
#else
      for (i = 0 ; i < h ; i++)
      {
         memcpy(dst,  srcY, w);
         memcpy(dstU, srcU, (w + 1) / 2);
         memcpy(dstV, srcV, (w + 1) / 2);

         srcY += stride[0];
         srcU += stride[1];
         srcV += stride[2];
         dst  += ri.BytesPerRow[1];
         dstU += ri.BytesPerRow[1];
         dstV += ri.BytesPerRow[2];
      }
#endif
   }

#ifndef SOURCE_BITMAP
   IP96->p96UnlockBitMap(bm, lock_h);
// IGraphics->WaitBlit();
#else
   IP96->p96UnlockBitMap(SourceBitMap, lock_h);
#endif

#if 0
{
static uint32 printed = 0;
if (printed < 1)
{
printed ++;
// printf("%d->%d %d->%d %d->%d\n", stride[0], ri.BytesPerRow[0], stride[1], ri.BytesPerRow[1], stride[2], ri.BytesPerRow[2]);
printf("%d->%d (%d*%d*%d) %d %d  %p %d\n", stride[0], ri.RI.BytesPerRow, bm->BytesPerRow, bm->Rows, bm->Depth, stride[1], stride[2], ri.RI.Memory, ri.RI.RGBFormat);
}
}
#endif

if (x) printf("[amiga] x=%d\n", x);
#if 0
static uint32 printed = 0;
if (printed < 2)
{
printed ++;
printf("%p %d\n", ri.Planes[0], ri.BytesPerRow[0]);
printf(" %p %d\n", ri.Planes[1], ri.BytesPerRow[1]);
printf(" %p %d\n", ri.Planes[2], ri.BytesPerRow[2]);
}
#endif

// printf("0x%x %d\n", ri.Planes[0], ri.BytesPerRow[0]);
// printf(" 0x%x %d\n", ri.Planes[1], ri.BytesPerRow[1]);
// printf(" 0x%x %d\n", ri.Planes[2], ri.BytesPerRow[2]);

   return VO_TRUE;
}

/***************************** DRAW_SLICE ***************************/
static int draw_slice(UNUSED uint8_t *image[], UNUSED int stride[], UNUSED int w, UNUSED int h, UNUSED int x, UNUSED int y)
{
//   printf("[amiga] draw_slice %d,%d %d*%d\n", x, y, w, h);
   return VO_NOTIMPL;
}

#if 0
/***************************** DRAW_FRAME ***************************/
static int draw_frame(uint8_t *src[])
{
   printf("[amiga] draw_frame %p %p %p %p\n", src[0], src[1], src[2], src[3]);
   return VO_NOTIMPL;
}
#endif

/******************************************************************************/
/* Just a little function to help uninit and control.                         */
/* It close the screen (if fullscreen), the windows, and free all gfx related */
/* stuff, but do not close any libs                                           */
/******************************************************************************/
static void FreeGfx(void)
{
	/*if(R5G6B5_BitMap)
	{
		IP96->p96UnlockBitMapFromBoard(R5G6B5_BitMap, FALSE);
		IP96->p96FreeBitMap(R5G6B5_BitMap);
		R5G6B5_BitMap = NULL;
	}

#ifdef SOURCE_BITMAP
	if(SourceBitMap)
	{
		IP96->p96FreeBitMap(SourceBitMap);
		SourceBitMap = NULL;
	}
#endif*/

	Close_Window();

	closeRemainingOpenWin();

IDBUG("is_fullscreen=%ld (0x%08lx)\n",is_fullscreen,My_Screen);
	if(My_Screen)//is_fullscreen)
	{
		IIntuition->CloseScreen(My_Screen);
		My_Screen = NULL;
	}

/*#ifdef WRITEPIXEL
	if(tmpBitMap)
	{
		IGraphics->FreeBitMap(tmpBitMap);
		tmpBitMap = NULL;
	}
#endif*/
}

/****************************** UNINIT ******************************/
static void uninit(void)
{
   if (R5G6B5_BitMap)
   {
#if 0
      uint32 lock;
      struct RenderInfo ri;
      if ((lock = IP96->p96LockBitMap(R5G6B5_BitMap, (uint8 *)&ri, sizeof(ri))))
      {
         IP96->p96UnlockBitMap(R5G6B5_BitMap, lock);
         printf("R5G6B5_BitMap\n");
         printf(" Memory %p, BytesPerRow %d\n", ri.Memory, ri.BytesPerRow);
         printf(" RGBFormat 0x%08x\n", ri.RGBFormat);
      }
// ohne LockBitMapToBoard(), CompositeTags():
// R5G6B5_BitMap
// Memory 0x56b9b000, BytesPerRow 1280
// RGBFormat 0x0000000a
// Boardnumber 0, locked = 0
// BENCHMARKs: VC:   8.004s VO:  22.287s A:   0.000s Sys:   0.321s =   30.612s
// BENCHMARK%: VC: 26.1459% VO: 72.8064% A:  0.0000% Sys:  1.0477% = 100.0000%
// R5G6B5_BitMap
//  Memory 0xaab10fa0, BytesPerRow 1280
//  RGBFormat 0x0000000a
#endif
      IP96->p96UnlockBitMapFromBoard(R5G6B5_BitMap, FALSE);
      IP96->p96FreeBitMap(R5G6B5_BitMap);
      R5G6B5_BitMap = NULL;
   }

#ifdef SOURCE_BITMAP
   if (SourceBitMap)
   {
      IP96->p96FreeBitMap(SourceBitMap);
      SourceBitMap = NULL;
   }
#endif


FreeGfx();


/*if(bf_Window)
{
	IIntuition->CloseWindow(bf_Window);
	bf_Window=NULL;
}*/

   /*Close_Window();

closeRemainingOpenWin();

IDBUG("vo_fs=is_fullscreen 0x%08lx\n",is_fullscreen);
if(is_fullscreen)
{
IDBUG("fullscreen=0x%08lx\n",My_Screen);
   IIntuition->CloseScreen(My_Screen);
   My_Screen = NULL;
}*/


IDBUG("backfillhook = %p (free)\n",backfillhook);
IExec->FreeSysObject(ASOT_HOOK, backfillhook);
backfillhook = NULL;


#ifdef WRITEPIXEL
   if (tmpBitMap)
   {
      IGraphics->FreeBitMap(tmpBitMap);
      tmpBitMap = NULL;
   }
#endif

#ifdef USERTG
   if (RTGBase)
   {
      IExec->CloseLibrary((struct Library *)RTGBase);
      RTGBase = NULL;
   }
#endif
}

/****************************** CONTROL *****************************/
static int control(uint32_t request, void *data)
{
   switch (request)
   {
      case VOCTRL_GUISUPPORT:
          return VO_FALSE;

      case VOCTRL_FULLSCREEN:
is_fullscreen ^= VOFLAG_FULLSCREEN;
IDBUG("VOCTRL_FULLSCREEN (%ld)\n",is_fullscreen);
FreeGfx(); // Free/Close all gfx stuff (screen windows, buffer...);
if( config(image_width, image_height, window_width, window_height, is_fullscreen, NULL, image_format) < 0) return VO_FALSE;
return VO_TRUE;
//          return VO_FALSE;

      case VOCTRL_ONTOP:
         {
            uint32 ModeID;

            if(vo_ontop)
            {
               vo_ontop = 0;
               is_ontop = 0;
            } else {
               vo_ontop = 1;
               is_ontop = 1;
            }

            Close_Window();
//            if ( config(image_width, image_height, window_width, window_height, FALSE /*is_fullscreen*/, NULL, image_format) < 0) return VO_FALSE;
            ModeID = Open_Window(FALSE);
            if (INVALID_ID == ModeID)
            {
               return VO_FALSE;
            }

            return VO_TRUE;
         }

      case VOCTRL_PAUSE:
         return VO_TRUE;

      case VOCTRL_RESUME:
         return VO_TRUE;

      case VOCTRL_QUERY_FORMAT:
         return query_format(*(uint32 *)data);

      case VOCTRL_UPDATE_SCREENINFO:
         if (!vo_screenwidth || !vo_screenheight)
         {
            vo_screenwidth  = 1024;
            vo_screenheight = 768;
         }
         aspect_save_screenres(vo_screenwidth, vo_screenheight);
printf("VOCTRL_UPDATE_SCREENINFO: %dx%d\n", vo_screenwidth , vo_screenheight);
         return VO_TRUE;

#if 0
      case VOCTRL_GET_EOSD_RES:
         {
            struct mp_eosd_settings *r = data;
            r->w = old_d_width; r->h = old_d_width;
            if (My_Window)
            {
               IIntuition->GetWindowAttrs(My_Window, WA_InnerWidth, &r->w, WA_InnerHeight, &r->h, TAG_DONE);
            }
            r->srcw = image_width; r->srch = image_height;
            r->mt = r->mb = r->ml = r->mr = 0;
            r->unscaled = 1;
#if 0
            if (scaled_osd) {r->w = image_width; r->h = image_height;}
            else if (aspect_scaling()) {
               r->ml = r->mr = ass_border_x;
               r->mt = r->mb = ass_border_y;
            }
#endif
         }
         return VO_TRUE;
#endif

      case VOCTRL_GET_IMAGE:
         return get_image(data);

      case VOCTRL_DRAW_IMAGE:
         return draw_image(data);

      case VOCTRL_START_SLICE:
      case VOCTRL_DRAW_EOSD:
      case VOCTRL_GET_EOSD_RES:
         return VO_NOTIMPL;
   }
printf("control %d\n", request);

   return VO_NOTIMPL;
}

/**************************** CHECK_EVENTS **************************/
static void check_events(void)
{
    //CheckEvents();
    gfx_CheckEvents(My_Screen, My_Window, &window_height, &window_width, &window_left, &window_top);
#if 0
/* ARexx */
    if(IExec->SetSignal(0L,rxHandler->sigmask) & rxHandler->sigmask)
    {
        IIntuition->IDoMethod(rxHandler->rxObject,AM_HANDLEEVENT);
    }
/* ARexx */
#endif
}

static int query_format(uint32_t format)
{
//   printf("format = 0x%x '%c%c%c%c'\n", format, format >> 24, (format >> 16) & 0xFF, (format >> 8) & 0xFF, format & 0xFF);

   switch( format)
   {
      case IMGFMT_YV12:
//      case IMGFMT_I420:
//         return VFCAP_CSP_SUPPORTED | VFCAP_CSP_SUPPORTED_BY_HW | VFCAP_OSD | VFCAP_ACCEPT_STRIDE | VFCAP_HWSCALE_UP;
//         return VFCAP_CSP_SUPPORTED | VFCAP_CSP_SUPPORTED_BY_HW | VFCAP_OSD | VFCAP_HWSCALE_UP | VFCAP_HWSCALE_DOWN /*| VFCAP_POSTPROC*/;
//         return VFCAP_CSP_SUPPORTED | VFCAP_CSP_SUPPORTED_BY_HW | VFCAP_OSD | VFCAP_HWSCALE_UP | VFCAP_HWSCALE_DOWN | VFCAP_ACCEPT_STRIDE;
         return VFCAP_CSP_SUPPORTED | VFCAP_CSP_SUPPORTED_BY_HW | VFCAP_OSD | VFCAP_HWSCALE_UP | VFCAP_HWSCALE_DOWN | /*VFCAP_ACCEPT_STRIDE |*/ /*VFCAP_POSTPROC |*/ /*VFCAP_CONSTANT |*/ VOCAP_NOSLICES;

      default:
printf("[amiga] config(0x%x) '%c%c%c%c'\n", format, format >> 24, (format >> 16) & 0xFF, (format >> 8) & 0xFF, format & 0xFF);
         return VO_FALSE;
   }
}
