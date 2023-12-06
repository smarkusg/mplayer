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
 *  Written by Jörg Strohmayer and Andrea Palmatè
*/

void ChangePauseButton(void)
{
	Object *temp = NULL;
	struct Screen *tempScreen = LockPubScreen("Workbench");

	SetGadgetAttrs((struct Gadget *)OBJ[GID_MAIN], My_Window, NULL,
		LAYOUT_ModifyChild, OBJ[GID_Pause],
					CHILD_ReplaceObject, 				temp = ButtonObject,
					GA_ID, 						GID_Pause,
					GA_ReadOnly, 					FALSE,
					GA_RelVerify, 					TRUE,
					BUTTON_Transparent,				TRUE,
					BUTTON_BevelStyle, 				BVS_NONE,
					BUTTON_RenderImage, 				image[1] =  BitMapObject,	// Pause
						BITMAP_Screen, 				tempScreen,
						BITMAP_Masking, 				TRUE,
						BITMAP_Transparent, 			TRUE,
						BITMAP_SourceFile, 			isPaused == TRUE ? "PROGDIR:Gui/td_play.png"	: "PROGDIR:Gui/td_pause.png",
						BITMAP_SelectSourceFile, 		isPaused == TRUE ? "PROGDIR:Gui/td_play_s.png"	: "PROGDIR:Gui/td_pause_s.png",
						BITMAP_DisabledSourceFile, 		isPaused == TRUE ? "PROGDIR:Gui/td_play_g.png"	: "PROGDIR:Gui/td_pause_g.png",
						BitMapEnd,
					ButtonEnd,
		TAG_DONE);

	OBJ[GID_Pause] = temp;
	UnlockPubScreen(NULL, tempScreen);
	RethinkLayout((struct Gadget *)OBJ[GID_MAIN], My_Window, NULL, TRUE);
}


#if 0
struct Gadget *CreateWinLayout(struct Screen *screen, struct Window *window)
{
	Object *TempLayout;
	TempLayout = VLayoutObject,
					GA_Left, window->BorderLeft,
					GA_Top, window->BorderTop,
					GA_RelWidth, -(window->BorderLeft + window->BorderRight),
					GA_RelHeight, -(window->BorderTop + window->BorderBottom),
					ICA_TARGET, 		ICTARGET_IDCMP,
					LAYOUT_DeferLayout, FALSE,
					LAYOUT_RelVerify, 	TRUE,
                    LAYOUT_AddChild, VLayoutObject,
						LAYOUT_AddChild, Space_Object = SpaceObject,
							GA_ReadOnly, TRUE,
							//SPACE_MinWidth, image_width,
							//SPACE_MinHeight, image_height,
							SPACE_Transparent, TRUE,
						SpaceEnd,
					LayoutEnd,
                    CHILD_WeightedWidth, 100,
                    CHILD_WeightedHeight, 100,
					LAYOUT_AddChild, ToolBarObject = (struct Gadget*)HLayoutObject,
						LAYOUT_VertAlignment, LALIGN_CENTER,
						StartMember, OBJ[GID_Back] = ButtonObject,
							GA_ID, GID_Back,
							GA_ReadOnly, FALSE,
							GA_RelVerify, TRUE,
							BUTTON_Transparent,TRUE,
							BUTTON_BevelStyle, BVS_NONE,
							BUTTON_RenderImage,image[0] = BitMapObject,
								BITMAP_SourceFile,"PROGDIR:Gui/td_rewind.png",
								BITMAP_SelectSourceFile,"PROGDIR:Gui/td_rewind_s.png",
								BITMAP_DisabledSourceFile,"PROGDIR:Gui/td_rewind_g.png",
								BITMAP_Screen,screen,
								BITMAP_Masking,TRUE,
							BitMapEnd,
						EndMember,
						CHILD_WeightedWidth, 0,
						CHILD_WeightedHeight, 0,
						StartMember, OBJ[GID_Pause] = ButtonObject,
							GA_ID, GID_Pause,
							GA_ReadOnly, FALSE,
							GA_RelVerify, TRUE,
							BUTTON_Transparent,TRUE,
							BUTTON_BevelStyle, BVS_NONE,
							BUTTON_RenderImage,image[1] = BitMapObject,
								BITMAP_SourceFile,"PROGDIR:Gui/td_pause.png",
								BITMAP_SelectSourceFile,"PROGDIR:Gui/td_pause_s.png",
								BITMAP_DisabledSourceFile,"PROGDIR:Gui/td_pause_g.png",
								BITMAP_Screen,screen,
								BITMAP_Masking,TRUE,
							BitMapEnd,
						EndMember,
						CHILD_WeightedWidth, 0,
						CHILD_WeightedHeight, 0,
						StartMember, OBJ[GID_Forward] = ButtonObject,
							GA_ID, GID_Forward,
							GA_ReadOnly, FALSE,
							GA_RelVerify, TRUE,
							BUTTON_Transparent,TRUE,
							BUTTON_BevelStyle, BVS_NONE,
							BUTTON_RenderImage,image[2] = BitMapObject,
								BITMAP_SourceFile,"PROGDIR:Gui/td_ff.png",
								BITMAP_SelectSourceFile,"PROGDIR:Gui/td_ff_s.png",
								BITMAP_DisabledSourceFile,"PROGDIR:Gui/td_ff_g.png",
								BITMAP_Screen,screen,
								BITMAP_Masking,TRUE,
							BitMapEnd,
						EndMember,
						CHILD_WeightedWidth, 0,
						CHILD_WeightedHeight, 0,
						StartMember, OBJ[GID_Stop] = ButtonObject,
							GA_ID, GID_Stop,
							GA_ReadOnly, FALSE,
							GA_RelVerify, TRUE,
							BUTTON_Transparent,TRUE,
							BUTTON_BevelStyle, BVS_NONE,
							BUTTON_RenderImage,image[3] = BitMapObject,
								BITMAP_SourceFile,"PROGDIR:Gui/td_stop.png",
								BITMAP_SelectSourceFile,"PROGDIR:Gui/td_stop_s.png",
								BITMAP_DisabledSourceFile,"PROGDIR:Gui/td_stop_g.png",
								BITMAP_Screen,screen,
								BITMAP_Masking,TRUE,
							BitMapEnd,
						EndMember,
						CHILD_WeightedWidth, 0,
						CHILD_WeightedHeight, 0,
						StartMember, OBJ[GID_Eject] = ButtonObject,
							GA_ID, GID_Eject,
							GA_ReadOnly, FALSE,
							GA_RelVerify, TRUE,
							BUTTON_Transparent,TRUE,
							BUTTON_BevelStyle, BVS_NONE,
							BUTTON_RenderImage,image[4] = BitMapObject,
								BITMAP_SourceFile,		"PROGDIR:Gui/td_eject.png",
								BITMAP_SelectSourceFile,	"PROGDIR:Gui/td_eject_s.png",
								BITMAP_DisabledSourceFile,	"PROGDIR:Gui/td_eject_g.png",
								BITMAP_Screen,screen,
								BITMAP_Masking,TRUE,
							BitMapEnd,
						EndMember,
						CHILD_WeightedWidth, 0,
						CHILD_WeightedHeight, 0,
						StartMember, OBJ[GID_Time] = ScrollerObject,
							GA_ID, 				GID_Time,
							GA_RelVerify, 			TRUE,
							GA_FollowMouse,			TRUE,
							GA_Immediate, 			TRUE,
							ICA_TARGET, 			ICTARGET_IDCMP,
							SCROLLER_Total, 			100,
							SCROLLER_Arrows, 			FALSE,
							SCROLLER_Orientation, 		SORIENT_HORIZ,
							PGA_ArrowDelta,			10000,
							PGA_Freedom, 			FREEHORIZ,
						End,
						CHILD_MinHeight, screen->Font->ta_YSize + 6,
						StartMember, OBJ[GID_FullScreen] = ButtonObject,
							GA_ID, 				GID_FullScreen,
							GA_ReadOnly, 			FALSE,
							GA_RelVerify, 			TRUE,
							BUTTON_Transparent,		TRUE,
							BUTTON_BevelStyle, 		BVS_NONE,
							BUTTON_RenderImage,image[5] = BitMapObject,
								BITMAP_SourceFile,		"PROGDIR:Gui/td_fullscreen.png",
								BITMAP_SelectSourceFile,	"PROGDIR:Gui/td_fullscreen_s.png",
								BITMAP_DisabledSourceFile,	"PROGDIR:Gui/td_fullscreen_g.png",
								BITMAP_Screen,			screen,
								BITMAP_Masking,			TRUE,
							BitMapEnd,
						EndMember,
						CHILD_WeightedWidth, 	0,
						CHILD_WeightedHeight, 	0,
						StartMember, OBJ[GID_Volume] = ScrollerObject,
							GA_ID, 				GID_Volume,
							GA_RelVerify, 			TRUE,
							GA_FollowMouse,			TRUE,
							GA_Immediate, 			TRUE,
							ICA_TARGET, 			ICTARGET_IDCMP,
							SCROLLER_Total, 			100,
							SCROLLER_Top, 			100,
							SCROLLER_Arrows, 			FALSE,
							SCROLLER_Orientation, 		SORIENT_HORIZ,
							PGA_ArrowDelta,			50,
							PGA_Freedom, 			FREEHORIZ,
						End,
						CHILD_MinHeight, screen->Font->ta_YSize + 6,
						CHILD_MinWidth, 50,
						CHILD_WeightedWidth, 0,
					LayoutEnd,
                    CHILD_WeightedHeight, 0,
				LayoutEnd;

	return (struct Gadget *)TempLayout;
}
#endif


#if 0
static void ClearSpaceArea(struct Window *window)
{
	struct IBox* ibox;
	GetAttr(SPACE_AreaBox, Space_Object, (uint32*)(void*)&ibox);
	IGraphics->SetAPen(window->RPort,1);
	IGraphics->RectFill(window->RPort,ibox->Left,ibox->Top,ibox->Left+ibox->Width-1,ibox->Top+ibox->Height-1);
	ChangeWindowBox(window, window->LeftEdge, window->TopEdge, window->Width, window->Height);
}
#endif

#if 0

static int32 PlayFile(const char *FileName)
{
	mp_cmd_t MPCmd;
	mp_cmd_t *c;
	ULONG iValue = 0;
	int abs = 2;

	/* Reset some variables.. */
	old_d_width  = 0;
	old_d_height = 0;
   	keep_width = 0;
	keep_height = 0;
	image_width = 0;
	image_height = 0;

	FirstTime = TRUE;

	/* Stop the current file */
	GetAttr(SCROLLER_Total, OBJ[GID_Time], &iValue);
	MPCmd.pausing = FALSE;
	MPCmd.id    = MP_CMD_SEEK;
	MPCmd.name  = strdup("seek");
	MPCmd.nargs = 2;
	MPCmd.args[0].type = MP_CMD_ARG_FLOAT;
	MPCmd.args[0].v.f=iValue/1000.0f;
	MPCmd.args[1].type = MP_CMD_ARG_INT;
	MPCmd.args[1].v.i=abs;
	MPCmd.args[2].type = -1;
	MPCmd.args[2].v.i=0;
	if (( c = mp_cmd_clone(&MPCmd)))
		if ((mp_input_queue_cmd(c)==0))
		{
			mp_cmd_free(c);
		}
	ChangePauseButton();

	/* Add the file to the queue */
	MPCmd.id			= MP_CMD_LOADFILE;
	MPCmd.name			= NULL;
	MPCmd.nargs			= 2;
	MPCmd.pausing		= FALSE;
	MPCmd.args[0].type	= MP_CMD_ARG_STRING;
	MPCmd.args[0].v.s	= (char*)FileName;
	MPCmd.args[1].type	= MP_CMD_ARG_INT;
	MPCmd.args[1].v.i	= TRUE;
	MPCmd.args[2]		= mp_cmds[MP_CMD_LOADFILE].args[2];
	if (( c = mp_cmd_clone(&MPCmd)))
		if ((mp_input_queue_cmd(c)==0))
		{
			mp_cmd_free(c);
		}

	/* Play it! */
	MPCmd.id			= Stopped==FALSE ? MP_CMD_PLAY_TREE_STEP : MP_CMD_PLAY_TREE_UP_STEP;
	MPCmd.args[0].type	= MP_CMD_ARG_INT;
	MPCmd.args[0].v.i	= 1;
	MPCmd.args[1].type	= MP_CMD_ARG_INT;
	MPCmd.args[1].v.i	= TRUE;
	MPCmd.args[2]		= mp_cmds[MP_CMD_PLAY_TREE_STEP].args[2];

	if ((c = mp_cmd_clone(&MPCmd)))
		if ((mp_input_queue_cmd(c)==0))
		{
			mp_cmd_free(c);
		}

	isPaused = FALSE;
	ChangePauseButton();

	return RETURN_OK;
}

#endif

static void NotifyServer(STRPTR FileName)
{
	/* Send a message to Notification System (AKA Ringhio.. ;) */
	if (hasNotificationSystem == TRUE)
	{
		uint32 result  = 0;
		int32  success = 0;
		STRPTR iconname = NULL;
		static char text[512];

		sprintf(text,"Playing %s..",FileName);
		iconname = AllocVec(512,MEMF_SHARED | MEMF_CLEAR);
		if (iconname)
		{
			success = NameFromLock(GetCurrentDir(),iconname,512);
			if (success)
			{
				strcat(iconname,"/GUI/mplayer.png");
			}
		}
/*
		result = Notify(AppID,
						APPNOTIFY_Title,			"MPlayer",
						APPNOTIFY_Update,			TRUE,
						APPNOTIFY_PubScreenName,	"FRONT",
						APPNOTIFY_ImageFile,		(STRPTR)iconname,
						APPNOTIFY_Text,			text,
					TAG_DONE);
*/
		if (iconname) FreeVec(iconname);
	}
}

void RemoveAppPort(void)
{
/*
    if (AppPort)
    {
		void *msg;
		while ((msg = GetMsg(AppPort)))
		    ReplyMsg(msg);
		FreeSysObject(ASOT_PORT, AppPort);
		AppPort = NULL;
    }
*/
}

static void appw_events (void)
{
    if (AppWin)
    {
		struct AppMessage *msg;
		struct WBArg *arg;
	    char cmd[512] = {0};
/*
		if (AppPort)
		{
			while ((msg = (void*) GetMsg(AppPort)))
			{
			    arg = msg->am_ArgList;

		    	NameFromLock(arg[0].wa_Lock, &cmd[0], 512);

				if ((strcmp(arg[0].wa_Name,"")!=0))
				{
				    AddPart(cmd,arg[0].wa_Name,512);
					NotifyServer(cmd);
					PlayFile(cmd);
				}
				else
				{
					char *file;
					file = LoadFile(cmd);
					if (file)
					{
						NotifyServer(file);
						PlayFile(file);
					}
				}
		    	ReplyMsg ((void*) msg);
			}
		}
*/
    }
}

/* STUFF FUNCTIONS FOR MENU */
static void CalcAspectRatio(float a_ratio, int _oldwidth, int _oldheight, int *_newwidth, int *_newheight)
{
	int tempLen = 0;

	tempLen = _oldwidth / a_ratio;
	*_newheight = tempLen;
	*_newwidth = _oldwidth;
}

static void ChangeWindowSize(int mode)
{
	int new_width = 0, new_height = 0;
	int new_left  = 0, new_top    = 0;
	int new_bw    = 0, new_bh     = 0;
	struct Screen *tempScreen = NULL;

    if ( ( tempScreen = LockPubScreen ( NULL ) ) )
    {
		new_bw = tempScreen->WBorLeft + tempScreen->WBorRight;
		new_bh = tempScreen->WBorTop + tempScreen->Font->ta_YSize + 1 + tempScreen->WBorBottom;

		switch (mode)
		{
			case x1:
			case AR_ORIGINAL:
				new_width = keep_width;
				new_height = keep_height;
			break;
			case x2:
				new_width = keep_width * 2;
				new_height = keep_height * 2;
			break;
			case xF:
				new_width = My_Screen->Width - new_bw;
				new_height = My_Screen->Height - tempScreen->BarHeight - new_bh;
			break;
			case xH:
				new_width = keep_width / 2;
				new_height = keep_height / 2;
			break;
			case AR_4_3:
				CalcAspectRatio(1.333333f, keep_width, keep_height, &new_width, &new_height);
			break;
			case AR_5_3:
				CalcAspectRatio(1.666667f, keep_width, keep_height, &new_width, &new_height);
			break;
			case AR_5_4:
				CalcAspectRatio(1.25f, keep_width, keep_height, &new_width, &new_height);
			break;
			case AR_16_10:
				CalcAspectRatio(1.6f, keep_width, keep_height, &new_width, &new_height);
			break;
			case AR_16_9:
				CalcAspectRatio(1.777777f, keep_width, keep_height, &new_width, &new_height);
			break;
			case AR_235_1:
				CalcAspectRatio(2.35f, keep_width, keep_height, &new_width, &new_height);
			break;
			case AR_239_1:
				CalcAspectRatio(2.39f, keep_width, keep_height, &new_width, &new_height);
			break;
			case AR_221_1:
				CalcAspectRatio(2.21f, keep_width, keep_height, &new_width, &new_height);
			break;
			case AR_185_1:
				CalcAspectRatio(1.85f, keep_width, keep_height, &new_width, &new_height);
			break;
		}
		if (mode!=xF)
		{
			new_left = (tempScreen->Width - (new_width + new_bw)) / 2;
			new_top  = (tempScreen->Height - (new_height + new_bh)) / 2;
		}
		else
		{
			new_left = 0;
			new_top  = tempScreen->BarHeight;
		}

		if (new_top<0)
		{
			new_top 	= tempScreen->BarHeight;
			new_height 	= new_height - tempScreen->BarHeight;
		}
		if (new_left<0)
		{
			new_left 	= tempScreen->WBorRight;
			new_width 	= new_width - tempScreen->WBorRight;
		}

	    SetWindowAttrs(My_Window,
                               		WA_Left,		new_left,
                               		WA_Top,		new_top,
                               		WA_InnerWidth,	new_width,
                               		WA_InnerHeight,	new_height,
                               		TAG_DONE);
	}

}

static void SetVolume(int direction)
{
	switch (direction)
	{
		case VOL_UP:
			mplayer_put_key(KEY_VOLUME_UP);
		break;
		case VOL_DOWN:
			mplayer_put_key(KEY_VOLUME_DOWN);
		break;
	}
}

#if 0

static UBYTE *LoadFile(const char *StartingDir)
{
	struct FileRequester * AmigaOS_FileRequester = NULL;
	BPTR FavoritePath_File;
	char FavoritePath_Value[1024];
	BOOL FavoritePath_Ok = FALSE;
	UBYTE *filename = NULL;
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
		return NULL;
	}

	choosing = 1;
	if ( ( AslRequestTags( AmigaOS_FileRequester,
					ASLFR_TitleText,        "Choose a video",
					ASLFR_DoMultiSelect,    FALSE, // Maybe in a future we can implement a playlist...
					ASLFR_RejectIcons,      TRUE,
					ASLFR_DoPatterns,		TRUE,
					ASLFR_StayOnTop,		TRUE,
					ASLFR_InitialPattern,	(ULONG)EXTPATTERN,
					ASLFR_InitialDrawer,    (FavoritePath_Ok) ? FavoritePath_Value : "",
					TAG_DONE) ) == FALSE )
	{
		FreeAslRequest(AmigaOS_FileRequester);
		AmigaOS_FileRequester = NULL;
		choosing = 0;
		return NULL;
	}
	choosing = 0;

	if (AmigaOS_FileRequester->fr_NumArgs>0);
	{
		len = strlen(AmigaOS_FileRequester->fr_Drawer) + strlen(AmigaOS_FileRequester->fr_File) + 1;
		if ((filename = AllocMem(len,MEMF_SHARED|MEMF_CLEAR)))
		{
    	    strcpy(filename,AmigaOS_FileRequester->fr_Drawer);
        	AddPart(filename,AmigaOS_FileRequester->fr_File,len + 1);
		}
	}

	FreeAslRequest(AmigaOS_FileRequester);
	return filename;
}

#endif

static int32 PrintMsgProc(STRPTR args UNUSED, int32 length UNUSED, struct ExecBase *execbase)
{
        struct ExecIFace *iexec = (APTR)execbase->MainInterface;
        struct Process *me = (APTR)FindTask(NULL);
        struct DOSIFace *idos = (APTR)me->pr_ExitData;
        struct pmpMessage *pmp = (struct pmpMessage *)GetEntryData();

        if(pmp)
        {
                PrintMsg(pmp->about, pmp->type, pmp->image);
                FreeVec(pmp);
                return RETURN_OK;
        }
        else
        	return RETURN_FAIL;

}

static void PrintMsg(CONST_STRPTR text,int REQ_TYPE, int REQ_IMAGE)
{
	Object *reqobj;

	if (REQ_TYPE  == 0) REQ_TYPE	= REQTYPE_INFO;
	if (REQ_IMAGE == 0) REQ_IMAGE	= REQIMAGE_DEFAULT;

	reqobj = (Object *)NewObject( REQUESTER_GetClass(), NULL,
						REQ_Type,		REQ_TYPE,
						REQ_TitleText,	"MPlayer for AmigaOS4",
						REQ_BodyText,	text,
						REQ_Image,		REQ_IMAGE,
						REQ_TimeOutSecs,	10,
						REQ_GadgetText,	"_Ok",
						TAG_END
        );

	if ( reqobj )
	{
		IDoMethod( reqobj, RM_OPENREQ, NULL, My_Window, NULL, TAG_END );
		DisposeObject( reqobj );
	}
}

static void ShowAbout(void)
{
	struct Process *TaskMessage;
	struct pmpMessage *pmp = AllocVec(sizeof(struct pmpMessage), MEMF_SHARED);

	if(!pmp)
        return;

	pmp->about = "MPlayer - The Video Player\n"
				"\nPianeta Amiga Version"
				"\n\nBuilt against MPlayer version: "	VERSION
				"\n\nCopyright (C) MPlayer Team - http://www.mplayerhq.hu"
				"\nAmigaOS4 Version by Andrea Palmatè - http://www.amigasoft.net";

	pmp->type = REQTYPE_INFO;
	pmp->image = REQIMAGE_INFO;

	TaskMessage = (struct Process *) CreateNewProcTags (
										NP_Entry,		(ULONG) PrintMsgProc,
										NP_Name,		"About MPlayer",
										NP_StackSize,	262144,
										NP_Child,		TRUE,
										NP_Priority,	0,
										NP_EntryData,	pmp,
										NP_ExitData,	IDOS,
										TAG_DONE);
	if (!TaskMessage)
	{
		PrintMsg(pmp->about,REQTYPE_INFO,REQIMAGE_INFO);
		FreeVec(pmp);
	}
}

static void OpenNetwork(STRPTR args UNUSED, int32 length UNUSED, struct ExecBase *execbase UNUSED)
{
	Object *netwobj;
	UBYTE buffer[513]="http://";
	ULONG result;

	netwobj = (Object *) NewObject(REQUESTER_GetClass(),NULL,
										REQ_Type,		REQTYPE_STRING,
										REQ_TitleText,	"Load Network",
										REQ_BodyText,	"Enter the URL to Open",
										REQ_GadgetText,	"L_oad|_Cancel",
										REQS_Invisible,	FALSE,
										REQS_Buffer,	buffer,
										REQS_ShowDefault,	TRUE,
										REQS_MaxChars,	512,
										TAG_DONE);
	if ( netwobj )
	{
		choosing = 1;
		result = IDoMethod( netwobj, RM_OPENREQ, NULL, NULL, NULL, TAG_END );
		if (result != 0 && (strcmp(buffer,"http://")!=0))
		{
//       		#ifdef CONFIG_GUI
//			guiGetEvent(guiCEvent, (void *)guiSetPlay);
//			#endif
			//PlayFile(buffer);
		}
		DisposeObject( netwobj );
		choosing = 0;
	}
}

static void OpenDVD(STRPTR args UNUSED, int32 length UNUSED, struct ExecBase *execbase UNUSED)
{
	Object *dvdobj;
	UBYTE buffer[256]="dvd://1";
	ULONG result;

	dvdobj = (Object *) NewObject(REQUESTER_GetClass(),NULL,
										REQ_Type,		REQTYPE_STRING,
										REQ_TitleText,	"Load DVD",
										REQ_BodyText,	"Enter Chapter to Open",
										REQ_GadgetText,	"O_pen|_Cancel",
										REQS_Invisible,	FALSE,
										REQS_Buffer,	buffer,
										REQS_ShowDefault,	TRUE,
										REQS_MaxChars,	255,
										TAG_DONE);
	if ( dvdobj )
	{
		choosing = 1;
		result = IDoMethod( dvdobj, RM_OPENREQ, NULL, NULL, NULL, TAG_END );
		if (result != 0 && (strncmp(buffer,"dvd://",6)==0))
		{
//       		#ifdef CONFIG_GUI
//			guiGetEvent(guiCEvent, (void *)guiSetPlay);
//			#endif
			// PlayFile (buffer);
			printf("Buffer:%s\n",buffer);
		}
		else
			if (strncmp(buffer,"dvd://",6)!=0)
				PrintMsg("Enter a valid DVD protocol", REQTYPE_INFO, REQIMAGE_ERROR);
		DisposeObject( dvdobj );
		choosing = 0;
	}
}

static void OpenVCD(STRPTR args UNUSED, int32 length UNUSED, struct ExecBase *execbase UNUSED)
{
	Object *vcdobj;
	UBYTE buffer[256]="vcd://1";
	ULONG result;

	vcdobj = (Object *) NewObject(REQUESTER_GetClass(),NULL,
										REQ_Type,		REQTYPE_STRING,
										REQ_TitleText,	"Load VCD",
										REQ_BodyText,	"Enter Chapter to Open",
										REQ_GadgetText,	"O_pen|_Cancel",
										REQS_Invisible,	FALSE,
										REQS_Buffer,	buffer,
										REQS_ShowDefault,	TRUE,
										REQS_MaxChars,	255,
										TAG_DONE);
	if ( vcdobj )
	{
		choosing = 1;
		result = IDoMethod( vcdobj, RM_OPENREQ, NULL, NULL, NULL, TAG_END );
		if (result != 0 && (strncmp(buffer,"vcd://",6)==0))
		{
//       		#ifdef CONFIG_GUI
//			guiGetEvent(guiCEvent, (void *)guiSetPlay);
//			#endif
			// PlayFile (buffer);
		}
		else
			if (strncmp(buffer,"vcd://",6)!=0)
				PrintMsg("Enter a valid VCD protocol", REQTYPE_INFO, REQIMAGE_ERROR);
		DisposeObject( vcdobj );
		choosing = 0;
	}
}

void TimerReset(uint32 microDelay)
{
	if (microDelay>0)
	{
		_timerio->Request.io_Command = TR_ADDREQUEST;
		_timerio->Time.Seconds = 0;
		_timerio->Time.Microseconds = microDelay;
		SendIO((struct IORequest *)_timerio);
	}
}

BOOL TimerInit(void)
{
	_port = (struct MsgPort *)AllocSysObject(ASOT_PORT, NULL);
	if (!_port) return FALSE;

	_timerSig = 1L << _port->mp_SigBit;

	_timerio = (struct TimeRequest *)AllocSysObjectTags(ASOT_IOREQUEST,
		ASOIOR_Size,		sizeof(struct TimeRequest),
		ASOIOR_ReplyPort,	_port,
	TAG_DONE);

	if (!_timerio) return FALSE;

	if (!OpenDevice("timer.device", UNIT_MICROHZ, (struct IORequest *)
		_timerio, 0))
	{
		ITimer = (struct TimerIFace *)GetInterface((struct Library *)
			_timerio->Request.io_Device, "main", 1, NULL);
		if (ITimer) return TRUE;
	}

	return FALSE;
}

void TimerExit(void)
{
	if (_timerio)
	{
		if (!CheckIO((struct IORequest *)_timerio))
		{
			AbortIO((struct IORequest *)_timerio);
			WaitIO((struct IORequest *)_timerio);
		}
	}

	if (_port)
	{
		FreeSysObject(ASOT_PORT, _port);
		_port = 0;
	}

	if (_timerio && _timerio->Request.io_Device)
	{
		CloseDevice((struct IORequest *)_timerio);
		FreeSysObject(ASOT_IOREQUEST, _timerio);
		_timerio = 0;
	}

	if (ITimer)
	{
		DropInterface((struct Interface *)ITimer);
		ITimer = 0;
	}
}
