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


/****************************************************************

   This file was created automatically by `FlexCat 2.18'
   from "locale/MPlayer.cd"

   using, adapted to OS4, CatComp.sd 1.2 (24.09.1999)

   Do NOT edit by hand!

****************************************************************/

#ifndef MPlayer_STRINGS_H
#define MPlayer_STRINGS_H

#ifndef EXEC_TYPES_H
#include <exec/types.h>
#endif

#ifdef  MPlayer_BASIC_CODE
#undef  MPlayer_BASIC
#undef  MPlayer_CODE
#define MPlayer_BASIC
#define MPlayer_CODE
#endif

#ifdef  MPlayer_BASIC
#undef  MPlayer_ARRAY
#undef  MPlayer_BLOCK
#define MPlayer_ARRAY
#define MPlayer_BLOCK
#endif

#ifdef  MPlayer_ARRAY
#undef  MPlayer_NUMBERS
#undef  MPlayer_STRINGS
#define MPlayer_NUMBERS
#define MPlayer_STRINGS
#endif

#ifdef  MPlayer_BLOCK
#undef  MPlayer_STRINGS
#define MPlayer_STRINGS
#endif


#ifdef MPlayer_CODE
//#include <proto/locale.h>
//extern struct Library *LocaleBase;
#ifndef PROTO_LOCALE_H
 #ifndef __NOLIBBASE__
  #define _NLB_DEFINED_
  #define __NOLIBBASE__
 #endif
 #ifndef __NOGLOBALIFACE__
  #define _NGI_DEFINED_
  #define __NOGLOBALIFACE__
 #endif
 #include <proto/locale.h>
 #ifdef _NLB_DEFINED_
  #undef __NOLIBBASE__
  #undef _NLB_DEFINED_
 #endif
 #ifdef _NGI_DEFINED_
  #undef __NOGLOBALIFACE__
  #undef _NGI_DEFINED_
 #endif
#endif

struct LocaleInfo
{
#ifndef __amigaos4__
    struct Library     *li_LocaleBase;
#else
    struct LocaleIFace *li_ILocale;
#endif
    struct Catalog     *li_Catalog;
};
#endif

#ifdef MPlayer_NUMBERS

#define MSG_Menu_Project 0
#define MSG_Menu_Project_OpenFile 1
#define MSG_Menu_Project_OpenDVD 2
#define MSG_Menu_Project_OpenDVDNAV 3
#define MSG_Menu_Project_OpenVCD 4
#define MSG_Menu_Project_OpenNetwork 5
#define MSG_Menu_Project_Iconify 6
#define MSG_Menu_Project_About 7
#define MSG_Menu_Project_Quit 8
#define MSG_Menu_Play 9
#define MSG_Menu_Play_PlayPause 10
#define MSG_Menu_Play_Stop 11
#define MSG_Menu_Play_Record 12
#define MSG_Menu_Play_Loop 13
#define MSG_Menu_Play_PrevFile 14
#define MSG_Menu_Play_NextFile 15
#define MSG_Menu_Play_Properties 16
#define MSG_Menu_OptionsVideo 17
#define MSG_Menu_OptionsVideo_StayOnTop 18
#define MSG_Menu_OptionsVideo_Fullscreen 19
#define MSG_Menu_OptionsVideo_Screenshot 20
#define MSG_Menu_OptionsVideo_OpenSubtitles 21
#define MSG_Menu_OptionsVideo_CycleSubtitles 22
#define MSG_Menu_OptionsVideo_UnloadSubtitles 23
#define MSG_Menu_OptionsVideo_SubtitlesSize 24
#define MSG_Menu_OptionsVideo_SubtitlesBig 25
#define MSG_Menu_OptionsVideo_SubtitlesSmall 26
#define MSG_Menu_OptionsVideo_AspectRatio 27
#define MSG_Menu_OptionsVideo_AR_Original 28
#define MSG_Menu_OptionsAudio 29
#define MSG_Menu_OptionsAudio_Mute 30
#define MSG_Menu_OptionsAudio_VolumeUp 31
#define MSG_Menu_OptionsAudio_VolumeDown 32
#define MSG_Menu_Settings 33
#define MSG_Menu_Settings_MPlayerGUI 34
#define MSG_About_About 35
#define MSG_About_Licence 36
#define MSG_About_Built 37
#define MSG_About_AmigaVersion 38
#define MSG_About_FFmpegVersion 39
#define MSG_About_Copyright 40
#define MSG_About_MPlayer_Team 41
#define MSG_About_AmigaOS4Version 42
#define MSG_About_GCCVersion 43
#define MSG_About_ARexxPort 44
#define MSG_About_Translation 45
#define MSG_About_Translator 46
#define MSG_About_VideoDriver 47
#define MSG_Requester_OK 48
#define MSG_Requester_Title 49
#define MSG_Requester_Network_Title 50
#define MSG_Requester_Network_Body 51
#define MSG_Requester_Network_Gadget 52
#define MSG_Requester_DVD_Title 53
#define MSG_Requester_DVD_Body 54
#define MSG_Requester_DVD_Gadget 55
#define MSG_Requester_DVD_Error 56
#define MSG_Requester_DVDNAV_Title 57
#define MSG_Requester_DVDNAV_Error 58
#define MSG_Requester_VCD_Title 59
#define MSG_Requester_VCD_Body 60
#define MSG_Requester_VCD_Gadget 61
#define MSG_Requester_VCD_Error 62
#define MSG_Requester_OpenFile_Video 63
#define MSG_Requester_OpenFile_SubTitles 64
#define MSG_Requester_Title_About 65
#define MSG_Requester_Title_Warning 66
#define MSG_Requester_Title_Error 67
#define MSG_Warning_AltiVec 68
#define MSG_Warning_Button_OK 69
#define MSG_Warning_Button_Cancel 70
#define MSG_Properties_Video 71
#define MSG_Properties_Audio 72
#define MSG_Properties_Clip 73
#define MSG_Properties_Title 74
#define MSG_Properties_Prop 75
#define MSG_Properties_IW_Video 76
#define MSG_Properties_IW_Audio 77
#define MSG_Properties_IW_Clip 78
#define MSG_Warning_Cant_Iconify 79
#define MSG_RegisterAppID_Description 80
#define MSG_OSD_Added_Files 81
#define MSG_Notify_Added_File 82
#define MSG_MPlayer_ScreenTitle 83

#endif /* MPlayer_NUMBERS */


/****************************************************************************/


#ifdef MPlayer_STRINGS

#define MSG_Menu_Project_STR "Project"
#define MSG_Menu_Project_OpenFile_STR "Open file..."
#define MSG_Menu_Project_OpenDVD_STR "Open DVD..."
#define MSG_Menu_Project_OpenDVDNAV_STR "Open DVDNAV..."
#define MSG_Menu_Project_OpenVCD_STR "Open VCD..."
#define MSG_Menu_Project_OpenNetwork_STR "Open stream..."
#define MSG_Menu_Project_Iconify_STR "Iconify"
#define MSG_Menu_Project_About_STR "About..."
#define MSG_Menu_Project_Quit_STR "Quit"
#define MSG_Menu_Play_STR "Play"
#define MSG_Menu_Play_PlayPause_STR "Play/Pause"
#define MSG_Menu_Play_Stop_STR "Stop"
#define MSG_Menu_Play_Record_STR "Record stream"
#define MSG_Menu_Play_Loop_STR "Loop"
#define MSG_Menu_Play_PrevFile_STR "Prev file"
#define MSG_Menu_Play_NextFile_STR "Next file"
#define MSG_Menu_Play_Properties_STR "Properties..."
#define MSG_Menu_OptionsVideo_STR "Video options"
#define MSG_Menu_OptionsVideo_StayOnTop_STR "Stay on top"
#define MSG_Menu_OptionsVideo_Fullscreen_STR "Toggle fullscreen"
#define MSG_Menu_OptionsVideo_Screenshot_STR "Screenshot"
#define MSG_Menu_OptionsVideo_OpenSubtitles_STR "Open subtitles..."
#define MSG_Menu_OptionsVideo_CycleSubtitles_STR "Cycle subtitles"
#define MSG_Menu_OptionsVideo_UnloadSubtitles_STR "Unload subtitles"
#define MSG_Menu_OptionsVideo_SubtitlesSize_STR "Subtitles size"
#define MSG_Menu_OptionsVideo_SubtitlesBig_STR "Bigger"
#define MSG_Menu_OptionsVideo_SubtitlesSmall_STR "Smaller"
#define MSG_Menu_OptionsVideo_AspectRatio_STR "Aspect Ratio"
#define MSG_Menu_OptionsVideo_AR_Original_STR "Original"
#define MSG_Menu_OptionsAudio_STR "Audio options"
#define MSG_Menu_OptionsAudio_Mute_STR "Mute"
#define MSG_Menu_OptionsAudio_VolumeUp_STR "Volume Up"
#define MSG_Menu_OptionsAudio_VolumeDown_STR "Volume Down"
#define MSG_Menu_Settings_STR "Settings"
#define MSG_Menu_Settings_MPlayerGUI_STR "MPlayer-GUI..."
#define MSG_About_About_STR "About"
#define MSG_About_Licence_STR "Licence"
#define MSG_About_Built_STR "Built against MPlayer version:"
#define MSG_About_AmigaVersion_STR "Amiga version:"
#define MSG_About_FFmpegVersion_STR "FFmpeg version:"
#define MSG_About_Copyright_STR "Copyright (C)"
#define MSG_About_MPlayer_Team_STR "MPlayer Team"
#define MSG_About_AmigaOS4Version_STR "AmigaOS4 release:"
#define MSG_About_GCCVersion_STR "GCC version:"
#define MSG_About_ARexxPort_STR "ARexx Port:"
#define MSG_About_Translation_STR "Translation:"
#define MSG_About_Translator_STR "(using built-in strings)"
#define MSG_About_VideoDriver_STR "Video driver in use:"
#define MSG_Requester_OK_STR "_Ok"
#define MSG_Requester_Title_STR "MPlayer for AmigaOS4"
#define MSG_Requester_Network_Title_STR "Open stream"
#define MSG_Requester_Network_Body_STR "Enter URL to open:"
#define MSG_Requester_Network_Gadget_STR "_Load|_Cancel"
#define MSG_Requester_DVD_Title_STR "Load DVD"
#define MSG_Requester_DVD_Body_STR "Enter Chapter to open:"
#define MSG_Requester_DVD_Gadget_STR "_Open|_Cancel"
#define MSG_Requester_DVD_Error_STR "Enter valid DVD protocol:"
#define MSG_Requester_DVDNAV_Title_STR "Load DVDNAV"
#define MSG_Requester_DVDNAV_Error_STR "To use DVDNAV, you must comment out cache in the config file"
#define MSG_Requester_VCD_Title_STR "Load VCD"
#define MSG_Requester_VCD_Body_STR "Enter Chapter to open:"
#define MSG_Requester_VCD_Gadget_STR "_Open|_Cancel"
#define MSG_Requester_VCD_Error_STR "Enter valid VCD protocol:"
#define MSG_Requester_OpenFile_Video_STR "Choose video"
#define MSG_Requester_OpenFile_SubTitles_STR "Choose subtitles"
#define MSG_Requester_Title_About_STR "About MPlayer"
#define MSG_Requester_Title_Warning_STR "MPlayer: WARNING!"
#define MSG_Requester_Title_Error_STR "MPlayer: ERROR!"
#define MSG_Warning_AltiVec_STR "Sorry, this version is only for\nAltiVec capable machine!"
#define MSG_Warning_Button_OK_STR "Ok"
#define MSG_Warning_Button_Cancel_STR "Cancel"
#define MSG_Properties_Video_STR "\033bCodec: \033n%s \n\033bBitrate: \033n%d kbps \n\033bResolution: \033n%d x %d \n\033bFrames/sec.: \033n%5.2f \000"
#define MSG_Properties_Audio_STR "\033bCodec: \033n%s \n\033bBitrate: \033n%d kbps \n\033bRate: \033n%d Hz \n\033bChannels: \033n%d \000"
#define MSG_Properties_Clip_STR "\033bTitle: \033n%s \n\033bArtist: \033n%s \n\033bAlbum: \033n%s \n\033bYear: \033n%s \n\033bComment: \033n%s \n\033bGenre: \033n%s \000"
#define MSG_Properties_Title_STR "MPlayer: Properties"
#define MSG_Properties_Prop_STR "\033b VIDEO \033n\n%s\n\n\033b AUDIO \033n\n%s\n\n\033b CLIP \033n\n%s\n"
#define MSG_Properties_IW_Video_STR "Video"
#define MSG_Properties_IW_Audio_STR "Audio"
#define MSG_Properties_IW_Clip_STR "Clip"
#define MSG_Warning_Cant_Iconify_STR "Can't iconify!\nTrying to get icon from wrong path."
#define MSG_RegisterAppID_Description_STR "The Movie Player"
#define MSG_OSD_Added_Files_STR "Added %d file(s)"
#define MSG_Notify_Added_File_STR "Added: '%s'"
#define MSG_MPlayer_ScreenTitle_STR "MPlayer Screen"

#endif /* MPlayer_STRINGS */


/****************************************************************************/


#ifdef MPlayer_ARRAY

struct MPlayer_ArrayType
{
    LONG   cca_ID;
    STRPTR cca_Str;
};

static const struct MPlayer_ArrayType MPlayer_Array[] =
{
    { MSG_Menu_Project, (STRPTR)MSG_Menu_Project_STR },
    { MSG_Menu_Project_OpenFile, (STRPTR)MSG_Menu_Project_OpenFile_STR },
    { MSG_Menu_Project_OpenDVD, (STRPTR)MSG_Menu_Project_OpenDVD_STR },
    { MSG_Menu_Project_OpenDVDNAV, (STRPTR)MSG_Menu_Project_OpenDVDNAV_STR },
    { MSG_Menu_Project_OpenVCD, (STRPTR)MSG_Menu_Project_OpenVCD_STR },
    { MSG_Menu_Project_OpenNetwork, (STRPTR)MSG_Menu_Project_OpenNetwork_STR },
    { MSG_Menu_Project_Iconify, (STRPTR)MSG_Menu_Project_Iconify_STR },
    { MSG_Menu_Project_About, (STRPTR)MSG_Menu_Project_About_STR },
    { MSG_Menu_Project_Quit, (STRPTR)MSG_Menu_Project_Quit_STR },
    { MSG_Menu_Play, (STRPTR)MSG_Menu_Play_STR },
    { MSG_Menu_Play_PlayPause, (STRPTR)MSG_Menu_Play_PlayPause_STR },
    { MSG_Menu_Play_Stop, (STRPTR)MSG_Menu_Play_Stop_STR },
    { MSG_Menu_Play_Record, (STRPTR)MSG_Menu_Play_Record_STR },
    { MSG_Menu_Play_Loop, (STRPTR)MSG_Menu_Play_Loop_STR },
    { MSG_Menu_Play_PrevFile, (STRPTR)MSG_Menu_Play_PrevFile_STR },
    { MSG_Menu_Play_NextFile, (STRPTR)MSG_Menu_Play_NextFile_STR },
    { MSG_Menu_Play_Properties, (STRPTR)MSG_Menu_Play_Properties_STR },
    { MSG_Menu_OptionsVideo, (STRPTR)MSG_Menu_OptionsVideo_STR },
    { MSG_Menu_OptionsVideo_StayOnTop, (STRPTR)MSG_Menu_OptionsVideo_StayOnTop_STR },
    { MSG_Menu_OptionsVideo_Fullscreen, (STRPTR)MSG_Menu_OptionsVideo_Fullscreen_STR },
    { MSG_Menu_OptionsVideo_Screenshot, (STRPTR)MSG_Menu_OptionsVideo_Screenshot_STR },
    { MSG_Menu_OptionsVideo_OpenSubtitles, (STRPTR)MSG_Menu_OptionsVideo_OpenSubtitles_STR },
    { MSG_Menu_OptionsVideo_CycleSubtitles, (STRPTR)MSG_Menu_OptionsVideo_CycleSubtitles_STR },
    { MSG_Menu_OptionsVideo_UnloadSubtitles, (STRPTR)MSG_Menu_OptionsVideo_UnloadSubtitles_STR },
    { MSG_Menu_OptionsVideo_SubtitlesSize, (STRPTR)MSG_Menu_OptionsVideo_SubtitlesSize_STR },
    { MSG_Menu_OptionsVideo_SubtitlesBig, (STRPTR)MSG_Menu_OptionsVideo_SubtitlesBig_STR },
    { MSG_Menu_OptionsVideo_SubtitlesSmall, (STRPTR)MSG_Menu_OptionsVideo_SubtitlesSmall_STR },
    { MSG_Menu_OptionsVideo_AspectRatio, (STRPTR)MSG_Menu_OptionsVideo_AspectRatio_STR },
    { MSG_Menu_OptionsVideo_AR_Original, (STRPTR)MSG_Menu_OptionsVideo_AR_Original_STR },
    { MSG_Menu_OptionsAudio, (STRPTR)MSG_Menu_OptionsAudio_STR },
    { MSG_Menu_OptionsAudio_Mute, (STRPTR)MSG_Menu_OptionsAudio_Mute_STR },
    { MSG_Menu_OptionsAudio_VolumeUp, (STRPTR)MSG_Menu_OptionsAudio_VolumeUp_STR },
    { MSG_Menu_OptionsAudio_VolumeDown, (STRPTR)MSG_Menu_OptionsAudio_VolumeDown_STR },
    { MSG_Menu_Settings, (STRPTR)MSG_Menu_Settings_STR },
    { MSG_Menu_Settings_MPlayerGUI, (STRPTR)MSG_Menu_Settings_MPlayerGUI_STR },
    { MSG_About_About, (STRPTR)MSG_About_About_STR },
    { MSG_About_Licence, (STRPTR)MSG_About_Licence_STR },
    { MSG_About_Built, (STRPTR)MSG_About_Built_STR },
    { MSG_About_AmigaVersion, (STRPTR)MSG_About_AmigaVersion_STR },
    { MSG_About_FFmpegVersion, (STRPTR)MSG_About_FFmpegVersion_STR },
    { MSG_About_Copyright, (STRPTR)MSG_About_Copyright_STR },
    { MSG_About_MPlayer_Team, (STRPTR)MSG_About_MPlayer_Team_STR },
    { MSG_About_AmigaOS4Version, (STRPTR)MSG_About_AmigaOS4Version_STR },
    { MSG_About_GCCVersion, (STRPTR)MSG_About_GCCVersion_STR },
    { MSG_About_ARexxPort, (STRPTR)MSG_About_ARexxPort_STR },
    { MSG_About_Translation, (STRPTR)MSG_About_Translation_STR },
    { MSG_About_Translator, (STRPTR)MSG_About_Translator_STR },
    { MSG_About_VideoDriver, (STRPTR)MSG_About_VideoDriver_STR },
    { MSG_Requester_OK, (STRPTR)MSG_Requester_OK_STR },
    { MSG_Requester_Title, (STRPTR)MSG_Requester_Title_STR },
    { MSG_Requester_Network_Title, (STRPTR)MSG_Requester_Network_Title_STR },
    { MSG_Requester_Network_Body, (STRPTR)MSG_Requester_Network_Body_STR },
    { MSG_Requester_Network_Gadget, (STRPTR)MSG_Requester_Network_Gadget_STR },
    { MSG_Requester_DVD_Title, (STRPTR)MSG_Requester_DVD_Title_STR },
    { MSG_Requester_DVD_Body, (STRPTR)MSG_Requester_DVD_Body_STR },
    { MSG_Requester_DVD_Gadget, (STRPTR)MSG_Requester_DVD_Gadget_STR },
    { MSG_Requester_DVD_Error, (STRPTR)MSG_Requester_DVD_Error_STR },
    { MSG_Requester_DVDNAV_Title, (STRPTR)MSG_Requester_DVDNAV_Title_STR },
    { MSG_Requester_DVDNAV_Error, (STRPTR)MSG_Requester_DVDNAV_Error_STR },
    { MSG_Requester_VCD_Title, (STRPTR)MSG_Requester_VCD_Title_STR },
    { MSG_Requester_VCD_Body, (STRPTR)MSG_Requester_VCD_Body_STR },
    { MSG_Requester_VCD_Gadget, (STRPTR)MSG_Requester_VCD_Gadget_STR },
    { MSG_Requester_VCD_Error, (STRPTR)MSG_Requester_VCD_Error_STR },
    { MSG_Requester_OpenFile_Video, (STRPTR)MSG_Requester_OpenFile_Video_STR },
    { MSG_Requester_OpenFile_SubTitles, (STRPTR)MSG_Requester_OpenFile_SubTitles_STR },
    { MSG_Requester_Title_About, (STRPTR)MSG_Requester_Title_About_STR },
    { MSG_Requester_Title_Warning, (STRPTR)MSG_Requester_Title_Warning_STR },
    { MSG_Requester_Title_Error, (STRPTR)MSG_Requester_Title_Error_STR },
    { MSG_Warning_AltiVec, (STRPTR)MSG_Warning_AltiVec_STR },
    { MSG_Warning_Button_OK, (STRPTR)MSG_Warning_Button_OK_STR },
    { MSG_Warning_Button_Cancel, (STRPTR)MSG_Warning_Button_Cancel_STR },
    { MSG_Properties_Video, (STRPTR)MSG_Properties_Video_STR },
    { MSG_Properties_Audio, (STRPTR)MSG_Properties_Audio_STR },
    { MSG_Properties_Clip, (STRPTR)MSG_Properties_Clip_STR },
    { MSG_Properties_Title, (STRPTR)MSG_Properties_Title_STR },
    { MSG_Properties_Prop, (STRPTR)MSG_Properties_Prop_STR },
    { MSG_Properties_IW_Video, (STRPTR)MSG_Properties_IW_Video_STR },
    { MSG_Properties_IW_Audio, (STRPTR)MSG_Properties_IW_Audio_STR },
    { MSG_Properties_IW_Clip, (STRPTR)MSG_Properties_IW_Clip_STR },
    { MSG_Warning_Cant_Iconify, (STRPTR)MSG_Warning_Cant_Iconify_STR },
    { MSG_RegisterAppID_Description, (STRPTR)MSG_RegisterAppID_Description_STR },
    { MSG_OSD_Added_Files, (STRPTR)MSG_OSD_Added_Files_STR },
    { MSG_Notify_Added_File, (STRPTR)MSG_Notify_Added_File_STR },
    { MSG_MPlayer_ScreenTitle, (STRPTR)MSG_MPlayer_ScreenTitle_STR },
};


#endif /* MPlayer_ARRAY */


/****************************************************************************/


#ifdef MPlayer_BLOCK

static const char MPlayer_Block[] =
{

     "\x00\x00\x00\x00" "\x00\x08"
    MSG_Menu_Project_STR "\x00"
     "\x00\x00\x00\x01" "\x00\x0c"
    MSG_Menu_Project_OpenFile_STR ""
     "\x00\x00\x00\x02" "\x00\x0c"
    MSG_Menu_Project_OpenDVD_STR "\x00"
     "\x00\x00\x00\x03" "\x00\x0e"
    MSG_Menu_Project_OpenDVDNAV_STR ""
     "\x00\x00\x00\x04" "\x00\x0c"
    MSG_Menu_Project_OpenVCD_STR "\x00"
     "\x00\x00\x00\x05" "\x00\x0e"
    MSG_Menu_Project_OpenNetwork_STR ""
     "\x00\x00\x00\x06" "\x00\x08"
    MSG_Menu_Project_Iconify_STR "\x00"
     "\x00\x00\x00\x07" "\x00\x08"
    MSG_Menu_Project_About_STR ""
     "\x00\x00\x00\x08" "\x00\x04"
    MSG_Menu_Project_Quit_STR ""
     "\x00\x00\x00\x09" "\x00\x04"
    MSG_Menu_Play_STR ""
     "\x00\x00\x00\x0a" "\x00\x0a"
    MSG_Menu_Play_PlayPause_STR ""
     "\x00\x00\x00\x0b" "\x00\x04"
    MSG_Menu_Play_Stop_STR ""
     "\x00\x00\x00\x0c" "\x00\x0e"
    MSG_Menu_Play_Record_STR "\x00"
     "\x00\x00\x00\x0d" "\x00\x04"
    MSG_Menu_Play_Loop_STR ""
     "\x00\x00\x00\x0e" "\x00\x0a"
    MSG_Menu_Play_PrevFile_STR "\x00"
     "\x00\x00\x00\x0f" "\x00\x0a"
    MSG_Menu_Play_NextFile_STR "\x00"
     "\x00\x00\x00\x10" "\x00\x0e"
    MSG_Menu_Play_Properties_STR "\x00"
     "\x00\x00\x00\x11" "\x00\x0e"
    MSG_Menu_OptionsVideo_STR "\x00"
     "\x00\x00\x00\x12" "\x00\x0c"
    MSG_Menu_OptionsVideo_StayOnTop_STR "\x00"
     "\x00\x00\x00\x13" "\x00\x12"
    MSG_Menu_OptionsVideo_Fullscreen_STR "\x00"
     "\x00\x00\x00\x14" "\x00\x0a"
    MSG_Menu_OptionsVideo_Screenshot_STR ""
     "\x00\x00\x00\x15" "\x00\x12"
    MSG_Menu_OptionsVideo_OpenSubtitles_STR "\x00"
     "\x00\x00\x00\x16" "\x00\x10"
    MSG_Menu_OptionsVideo_CycleSubtitles_STR "\x00"
     "\x00\x00\x00\x17" "\x00\x10"
    MSG_Menu_OptionsVideo_UnloadSubtitles_STR ""
     "\x00\x00\x00\x18" "\x00\x0e"
    MSG_Menu_OptionsVideo_SubtitlesSize_STR ""
     "\x00\x00\x00\x19" "\x00\x06"
    MSG_Menu_OptionsVideo_SubtitlesBig_STR ""
     "\x00\x00\x00\x1a" "\x00\x08"
    MSG_Menu_OptionsVideo_SubtitlesSmall_STR "\x00"
     "\x00\x00\x00\x1b" "\x00\x0c"
    MSG_Menu_OptionsVideo_AspectRatio_STR ""
     "\x00\x00\x00\x1c" "\x00\x08"
    MSG_Menu_OptionsVideo_AR_Original_STR ""
     "\x00\x00\x00\x1d" "\x00\x0e"
    MSG_Menu_OptionsAudio_STR "\x00"
     "\x00\x00\x00\x1e" "\x00\x04"
    MSG_Menu_OptionsAudio_Mute_STR ""
     "\x00\x00\x00\x1f" "\x00\x0a"
    MSG_Menu_OptionsAudio_VolumeUp_STR "\x00"
     "\x00\x00\x00\x20" "\x00\x0c"
    MSG_Menu_OptionsAudio_VolumeDown_STR "\x00"
     "\x00\x00\x00\x21" "\x00\x08"
    MSG_Menu_Settings_STR ""
     "\x00\x00\x00\x22" "\x00\x0e"
    MSG_Menu_Settings_MPlayerGUI_STR ""
     "\x00\x00\x00\x23" "\x00\x06"
    MSG_About_About_STR "\x00"
     "\x00\x00\x00\x24" "\x00\x08"
    MSG_About_Licence_STR "\x00"
     "\x00\x00\x00\x25" "\x00\x1e"
    MSG_About_Built_STR ""
     "\x00\x00\x00\x26" "\x00\x0e"
    MSG_About_AmigaVersion_STR ""
     "\x00\x00\x00\x27" "\x00\x10"
    MSG_About_FFmpegVersion_STR "\x00"
     "\x00\x00\x00\x28" "\x00\x0e"
    MSG_About_Copyright_STR "\x00"
     "\x00\x00\x00\x29" "\x00\x0c"
    MSG_About_MPlayer_Team_STR ""
     "\x00\x00\x00\x2a" "\x00\x12"
    MSG_About_AmigaOS4Version_STR "\x00"
     "\x00\x00\x00\x2b" "\x00\x0c"
    MSG_About_GCCVersion_STR ""
     "\x00\x00\x00\x2c" "\x00\x0c"
    MSG_About_ARexxPort_STR "\x00"
     "\x00\x00\x00\x2d" "\x00\x0c"
    MSG_About_Translation_STR ""
     "\x00\x00\x00\x2e" "\x00\x18"
    MSG_About_Translator_STR ""
     "\x00\x00\x00\x2f" "\x00\x14"
    MSG_About_VideoDriver_STR ""
     "\x00\x00\x00\x30" "\x00\x04"
    MSG_Requester_OK_STR "\x00"
     "\x00\x00\x00\x31" "\x00\x14"
    MSG_Requester_Title_STR ""
     "\x00\x00\x00\x32" "\x00\x0c"
    MSG_Requester_Network_Title_STR "\x00"
     "\x00\x00\x00\x33" "\x00\x12"
    MSG_Requester_Network_Body_STR ""
     "\x00\x00\x00\x34" "\x00\x0e"
    MSG_Requester_Network_Gadget_STR "\x00"
     "\x00\x00\x00\x35" "\x00\x08"
    MSG_Requester_DVD_Title_STR ""
     "\x00\x00\x00\x36" "\x00\x16"
    MSG_Requester_DVD_Body_STR ""
     "\x00\x00\x00\x37" "\x00\x0e"
    MSG_Requester_DVD_Gadget_STR "\x00"
     "\x00\x00\x00\x38" "\x00\x1a"
    MSG_Requester_DVD_Error_STR "\x00"
     "\x00\x00\x00\x39" "\x00\x0c"
    MSG_Requester_DVDNAV_Title_STR "\x00"
     "\x00\x00\x00\x3a" "\x00\x3c"
    MSG_Requester_DVDNAV_Error_STR ""
     "\x00\x00\x00\x3b" "\x00\x08"
    MSG_Requester_VCD_Title_STR ""
     "\x00\x00\x00\x3c" "\x00\x16"
    MSG_Requester_VCD_Body_STR ""
     "\x00\x00\x00\x3d" "\x00\x0e"
    MSG_Requester_VCD_Gadget_STR "\x00"
     "\x00\x00\x00\x3e" "\x00\x1a"
    MSG_Requester_VCD_Error_STR "\x00"
     "\x00\x00\x00\x3f" "\x00\x0c"
    MSG_Requester_OpenFile_Video_STR ""
     "\x00\x00\x00\x40" "\x00\x10"
    MSG_Requester_OpenFile_SubTitles_STR ""
     "\x00\x00\x00\x41" "\x00\x0e"
    MSG_Requester_Title_About_STR "\x00"
     "\x00\x00\x00\x42" "\x00\x12"
    MSG_Requester_Title_Warning_STR "\x00"
     "\x00\x00\x00\x43" "\x00\x10"
    MSG_Requester_Title_Error_STR "\x00"
     "\x00\x00\x00\x44" "\x00\x38"
    MSG_Warning_AltiVec_STR ""
     "\x00\x00\x00\x45" "\x00\x02"
    MSG_Warning_Button_OK_STR ""
     "\x00\x00\x00\x46" "\x00\x06"
    MSG_Warning_Button_Cancel_STR ""
     "\x00\x00\x00\x47" "\x00\x56"
    MSG_Properties_Video_STR ""
     "\x00\x00\x00\x48" "\x00\x48"
    MSG_Properties_Audio_STR ""
     "\x00\x00\x00\x49" "\x00\x5c"
    MSG_Properties_Clip_STR ""
     "\x00\x00\x00\x4a" "\x00\x14"
    MSG_Properties_Title_STR "\x00"
     "\x00\x00\x00\x4b" "\x00\x2e"
    MSG_Properties_Prop_STR ""
     "\x00\x00\x00\x4c" "\x00\x06"
    MSG_Properties_IW_Video_STR "\x00"
     "\x00\x00\x00\x4d" "\x00\x06"
    MSG_Properties_IW_Audio_STR "\x00"
     "\x00\x00\x00\x4e" "\x00\x04"
    MSG_Properties_IW_Clip_STR ""
     "\x00\x00\x00\x4f" "\x00\x32"
    MSG_Warning_Cant_Iconify_STR ""
     "\x00\x00\x00\x50" "\x00\x10"
    MSG_RegisterAppID_Description_STR ""
     "\x00\x00\x00\x51" "\x00\x10"
    MSG_OSD_Added_Files_STR ""
     "\x00\x00\x00\x52" "\x00\x0c"
    MSG_Notify_Added_File_STR "\x00"
     "\x00\x00\x00\x53" "\x00\x0e"
    MSG_MPlayer_ScreenTitle_STR ""

};

#endif /* MPlayer_BLOCK */


/****************************************************************************/


#ifdef MPlayer_CODE

#ifndef MPlayer_CODE_EXISTS
 #define MPlayer_CODE_EXISTS

 STRPTR GetMPlayerString(struct LocaleInfo *li, LONG stringNum)
 {
#ifndef __amigaos4__
    struct Library     *LocaleBase = li->li_LocaleBase;
#else
    struct LocaleIFace *ILocale    = li->li_ILocale;
#endif
 LONG   *l;
 UWORD  *w;
 STRPTR  builtIn;

     l = (LONG *)MPlayer_Block;

     while (*l != stringNum)
       {
       w = (UWORD *)((ULONG)l + 4);
       l = (LONG *)((ULONG)l + (ULONG)*w + 6);
       }
     builtIn = (STRPTR)((ULONG)l + 6);

// #define MPlayer_XLocaleBase LocaleBase
// #define LocaleBase li->li_LocaleBase

#ifndef __amigaos4__
     if(LocaleBase && li)
        return(GetCatalogStr(li->li_Catalog, stringNum, builtIn));
#else
    if (ILocale)
    {
#ifdef __USE_INLINE__
        return GetCatalogStr(li->li_Catalog, stringNum, builtIn);
#else
        return ILocale->GetCatalogStr(li->li_Catalog, stringNum, builtIn);
#endif
    }
#endif
// #undef  LocaleBase
// #define LocaleBase XLocaleBase
// #undef  MPlayer_XLocaleBase

     return(builtIn);
 }

#else

 STRPTR GetMPlayerString(struct LocaleInfo *li, LONG stringNum);

#endif /* MPlayer_CODE_EXISTS */

#endif /* MPlayer_CODE */


/****************************************************************************/


#endif /* MPlayer_STRINGS_H */
