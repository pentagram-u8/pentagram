/*
 *  macosx_include.h
 *
 *  Copyright (C) 2006-2007  The Pentagram Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

//#ifndef MACOSX_INCLUDE_H
//#define MACOSX_INCLUDE_H

#include <sys/types.h>

#define BASIC_TYPES_DEFINED

#ifndef VERSION
#define VERSION "0.1cvs"
#endif

#define MACOSX

#define HAVE_SYS_STAT_H     1
#define HAVE_SYS_TYPES_H	1

#undef DATA_PATH

typedef	uint8_t     uint8;
typedef uint16_t    uint16;
typedef	uint32_t    uint32;

typedef	int8_t      sint8;
typedef	int16_t     sint16;
typedef	int32_t     sint32;

typedef uintptr_t   uintptr;
typedef intptr_t    sintptr;

#ifdef __cplusplus
extern "C" {
#endif
    
const char * macosxResourcePath();

#ifdef __cplusplus
}
#endif

//#endif