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

#ifndef __AMIGAOS_STUFF__
#define __AMIGAOS_STUFF__


#include <exec/types.h>


int AmigaOS_Open(int argc, char *argv[]); // returns -1 if a problem
void AmigaOS_Close(void);
void AmigaOS_ParseArg(int argc, char *argv[], int *new_argc, char ***new_argv);

void read_ENVARC(void);
void SetAmiUpdateENVVariable(const char *varname);
struct Window *AmigaOS_GetSDLWindowPtr(void);
void AmigaOS_ScreenTitle(const char *vo_str);
VOID VARARGS68K EasyRequester(CONST_STRPTR text, CONST_STRPTR button, ...);
void AmigaOS_do_applib(struct Window *w);
VOID VARARGS68K AmigaOS_applib_Notify(STRPTR not_msg, ...);
//void finishProcess(struct Process *proc);
void closeRemainingOpenWin(void);


#endif
