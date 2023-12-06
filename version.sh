#!/bin/sh

#################################################################################
#  This file is part of MPlayer.
#
#  MPlayer is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.
#
#  MPlayer is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License along
#  with MPlayer; if not, write to the Free Software Foundation, Inc.,
#  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
#
#################################################################################


test "$1" && extra="-$1"

# releases extract the version number from the VERSION file
version=$(cat VERSION 2> /dev/null)

if test -z $version ; then
# Extract revision number from file used by daily tarball snapshots
# or from the places different Subversion versions have it.
svn_revision=$(cat snapshot_version 2> /dev/null)
test $svn_revision || svn_revision=$(LC_ALL=C svn info 2> /dev/null | grep Revision | cut -d' ' -f2)
test $svn_revision || svn_revision=$(grep revision .svn/entries 2>/dev/null | cut -d '"' -f2)
test $svn_revision || svn_revision=$(sed -n -e '/^dir$/{n;p;q;}' .svn/entries 2>/dev/null)
test $svn_revision && svn_revision=SVN-r$svn_revision
test $svn_revision || svn_revision=UNKNOWN
version=$svn_revision
fi

AMIGA_VERSION=1.5-rc1
NEW_REVISION="#define VERSION \"${version}${extra}\""
OLD_REVISION=$(head -n 1 version.h 2> /dev/null)
TITLE='#define MP_TITLE "%s '$AMIGA_VERSION' "VERSION" (C) 2000-2023 MPlayer Team\n"'


AMIGA_VERSION_ABOUT='#if HAVE_ALTIVEC
#define AMIGA_VERSION_ABOUT "\033bMPlayer AltiVec Edition \033n'$AMIGA_VERSION' ('`date +"%d"`.`date +"%m"`.`date +"%Y"`')"
#else
#define AMIGA_VERSION_ABOUT "\033bMPlayer Generic Edition \033n'$AMIGA_VERSION' ('`date +"%d"`.`date +"%m"`.`date +"%Y"`')"
#endif'

AMIGA_VERSION='#define AMIGA_VERSION "MPlayer '$AMIGA_VERSION' ('`date +"%d"`.`date +"%m"`.`date +"%Y"`')"'
AMIGA_GCC_VERSION='#define GCC_VERSION __GNUC__'
AMIGA_GCC_REVISION='#define GCC_REVISION __GNUC_MINOR__'
AMIGA_GCC_GCC_PATCHLVL='#define GCC_PATCHLVL __GNUC_PATCHLEVEL__'

# Update version.h only on revision changes to avoid spurious rebuilds
if test "$NEW_REVISION" != "$OLD_REVISION"; then
    cat <<EOF > version.h
$NEW_REVISION
$AMIGA_VERSION
$AMIGA_VERSION_ABOUT
$AMIGA_GCC_VERSION
$AMIGA_GCC_REVISION
$AMIGA_GCC_GCC_PATCHLVL
$TITLE
EOF
fi
