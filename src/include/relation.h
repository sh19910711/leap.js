/* 
 * Name: relation.h
 * Description: Relation Structure function header file
 * Version: relation.h,v 1.203.2.1 2004/02/09 20:05:21 rleyton Exp
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


#include "defines.h"

#ifndef _LEAP_RELATION_
#define _LEAP_RELATION_

#include "dtypes.h"

extern relation relation_create(database db,
				 char *relation_name,
				 boolean temporary,
				 boolean system);
extern void relation_print( relation rel);
#define relation_findfirst(db) db->first_relation
#define relation_findnext(rel) rel->next
extern relation relation_find( database db,
			char *relname) ;
extern void relation_display( database db ) ;

extern void remove_tempfile( database db, 
					char *relation_name) ;
extern void relation_change( database db,
					  relation rel) ;

/* Some useful defines, better than function calls */
#define relation_temporary(rel) rel->temporary
#define relation_name(rel) rel->name
#define relation_noattributes(rel) rel->noattributes
#define relation_system(rel) rel->system

extern void relation_full_path(relation rel,
			 char *string) ;
extern void relations_dispose_all( database db ) ;
extern void relation_dispose( database db, relation *rel ) ;
extern void relation_dispose_mem( database db, relation *rel ) ;
extern int relations_ddopen(database db) ;
extern int LEAPAPI_relations_open(database db);
extern void relations_dispose_all( database db );
extern relation relation_rename(    database db,
						 			char *first,
									char *second) ;
extern void relation_remove( database db,
		      relation *rel ) ;
extern relation relation_new_read( const char *path, char *name);
extern int relation_insert( database db,
		      relation newrel) ;
		    

extern void dump_rel( relation rel ) ;

extern void relation_reverse(relation rel) ;

#endif
