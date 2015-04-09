/* 
 * Name: hashing.h
 * Description: Hashing functions header file.
 * Version: hashing.h,v 1.203.2.1 2004/02/09 20:05:21 rleyton Exp
 *
 *   LEAP RDBMS - An extensible and free RDBMS
 *   Copyright (C) 1995-2004 Richard Leyton
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *   See the file COPYING for more information.
 *
 *   Richard Leyton
 *   leap@leyton.org
 *   http://leap.sourceforge.net
 *   http://www.leyton.org
 *
 */



#ifndef _LEAP_HASHING_
#define _LEAP_HASHING_

#include "defines.h"
#include "dtypes.h"

/* This is the possible position of elements in the
 * hashing table
 */
typedef unsigned short int position;

extern HashTable hashing_create(void);

extern void hashing_insert( HashTable HT,
			char *e,
			position hk) ;
extern void hashing_delete ( HashTable HT,
		char *e,
		boolean *updated ) ;
extern void hashing_retrieve(   HashTable HT,
			char *tkey,
			char *e,
			boolean *retrieved) ;
extern void hashing_retrieve_extra(   HashTable HT,
            char *tkey,
            char *e,
            int *count, 
            float *fval,
            char *sval,
            int *ival,
            boolean *retrieved) ;
extern void hashing_terminate( HashTable *HT) ;
extern void hashing_load( HashTable *HT,
			char *filen) ;
extern void hashing_save( HashTable HT,
			char *filen) ;
extern HashTable build_hash_table( relation rel ) ;

#endif

