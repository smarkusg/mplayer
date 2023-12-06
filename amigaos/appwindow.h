#ifndef __AMIGAOS_SNSPSHOT_H__
#define __AMIGAOS_SNSPSHOT_H__

#include <workbench/workbench.h>


void make_appwindow(struct Window *win);
void delete_appwindow(void);
char *to_name_and_path(char *path, char *name);
void appwindow_LoopDnD(struct AppMessage *am);
void AmigaOS_do_appwindow(void);


#endif
