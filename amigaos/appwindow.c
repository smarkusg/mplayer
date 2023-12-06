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

/* This code comes from Excalibur */
/* Written by: Kjetil Hvalstrand */
/* Licence: MIT  (take use, modify, give credits) */


#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/intuition.h>
#include <proto/wb.h>
#include <workbench/startup.h>

#include "debug.h"
#include "appwindow.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern struct Catalog *catalog ;
#define MPlayer_NUMBERS
#define MPlayer_STRINGS
#include "../amigaos/MPlayer_catalog.h"
extern STRPTR myGetCatalogStr (struct Catalog *catalog, LONG num, STRPTR def);
#define CS(id) myGetCatalogStr(catalog,id,id##_STR)

#include "arexx.h"
#include "amigaos_stuff.h"

#include "input/input.h" // MP_CMD_LOADFILE
#include "mp_osd.h" // set_osd_msg() & osd_duration

extern int32 PlayFile_async(const char *FileName);


struct MsgPort          *appwin_port = NULL;
static struct AppWindow *app_window = NULL;
struct AppMessage       *appmsg = NULL;

ULONG appwindow_sig = 0;


void make_appwindow(struct Window *win)
{
	if (!appwin_port) appwin_port = AllocSysObject( ASOT_PORT, TAG_END);

	if (appwin_port)
	{
		appwindow_sig = 1L<< appwin_port -> mp_SigBit;
		app_window = AddAppWindowA(0, 0, win, appwin_port, NULL);
DBUG("make_appwindow()  0x%08lx  0x%08lx\n",app_window,appwin_port);
	}
}

void delete_appwindow(void)
{
DBUG("delete_appwindow() 0x%08lx  0x%08lx\n",app_window,appwin_port);
	if (app_window)
	{
		RemoveAppWindow(app_window);
		app_window = NULL;
	}

	if (appwin_port)
	{
		FreeSysObject(ASOT_PORT, (APTR) appwin_port);
		appwin_port = NULL;
		appwindow_sig = 0;
	}
}

char *to_name_and_path(char *path, char *name)
{
	char *path_whit_name = NULL;
//DBUG("to_name_and_path() : '%s'(%ld) '%s'(%ld)\n",path,strlen(path),name,strlen(name));
	if (path_whit_name = (void *) malloc ( strlen(path) + strlen(name) + 2) )
	{
		path_whit_name[0] = 0;

		if (strlen(path)>0)
		{
			if (path[strlen(path)-1]==':')
			{
				sprintf(path_whit_name, "%s%s",path,name);
			}
			else
			{
				sprintf(path_whit_name, "%s/%s",path,name);
			}
		}
	}
//DBUG("\t'%s'(%ld)\n",path_whit_name,strlen(path_whit_name));
	return path_whit_name;
}


void appwindow_LoopDnD(struct AppMessage *am)
{
	char temp[1000];
	BPTR temp_lock;
	int n, r;
//DBUG("am->am_NumArgs = %ld\n",am->am_NumArgs);
	for(n=0,r=0; n<am->am_NumArgs; n++)
	{
		if( (temp_lock = DupLock(am->am_ArgList[n].wa_Lock)) )
		{
			if( NameFromLock(temp_lock,temp,sizeof(temp)) )
			{
				if(am->am_ArgList[n].wa_Name[0] != 0) // exclude drawers
				{
					AddPart( temp, am->am_ArgList[n].wa_Name, sizeof(temp) );
					if(am->am_NumArgs == 1) // only 1 item DnD
					{
						put_scommand2(MP_CMD_LOADFILE,temp,0); // 0:play_right_away
					}
					else
					{
//DBUG("D&D[%ld]: '%s' (%s)\n",n,temp,am->am_ArgList[n].wa_Name);
						++r;
						put_scommand2(MP_CMD_LOADFILE,temp,1); // 1:append
						AmigaOS_applib_Notify(CS(MSG_Notify_Added_File),am->am_ArgList[n].wa_Name);
					}
				}
			}
			UnLock(temp_lock);
		}
	}
	if(r > 1) { set_osd_msg(OSD_MSG_TEXT, 1, osd_duration, CS(MSG_OSD_Added_Files),r); }
}

void AmigaOS_do_appwindow(void)
{
	struct AppMessage *am;
//DBUG("AmigaOS_do_appwindow(0x%08lx)\n",appwin_port);
	while( (am = (struct AppMessage *)GetMsg(appwin_port)) )
	//if( (am = (struct AppMessage *)GetMsg(appwin_port)) )
	{
		if (am->am_Type == AMTYPE_APPWINDOW)
		{
			appwindow_LoopDnD(am);
		}
		else
		{
			DBUG("Unknown AppMessage %d %p\n", am->am_Type, (APTR)am->am_UserData);
		}

		ReplyMsg((struct Message *) am);
	}
}
