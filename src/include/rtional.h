/* 
 * Name: rtional.h
 * Description: Relational Operators Header file.
 * Version: rtional.h,v 1.204.2.1 2004/02/09 20:05:21 rleyton Exp
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

#ifndef _LEAP_RTIONAL_
#define _LEAP_RTIONAL_

#include "dtypes.h"

extern relation project_create_rtn_relation(database db,
				     relation rel,
				     char *attribute_list,
				     char *destname) ;
extern relation rl_project(database db,
		    relation rel,
		    char *attribute_list,
		    char *destname) ;
extern relation rl_union( database db,
		   relation rel1, relation rel2,
		   char *destname) ;
extern relation rl_intersect( database db,
		       relation rel1, 
		       relation rel2,
		       char *destname) ;
extern relation rl_difference( database db,
		       relation rel1, 
		       relation rel2,
		       char *destname) ;
extern relation rl_display( relation rl);
extern relation rl_revdisplay(relation rel) ;
extern relation rl_product(database db,
			relation rel1, relation rel2,
			char *destname) ;
extern relation rl_update( database db,
			relation rel,
			char *qualification,
			char *set,
			char *dest) ;
extern relation rl_erase( database db,
			relation rel,
			char *qualification,
			char *dest) ;
extern relation rl_select( database db,
			relation rel,
			char *qualification,
			char *dest) ;
extern relation rl_naturaljoin( database db,
				relation lrel, relation rrel,
				char *qualification,
				char *destname,
				int join_type) ;
extern relation rl_duplicate(  database db, 
                        relation rel,
                        char *destname) ;
extern relation insert( database db,
			relation rel,
			char *data ) ;
extern relation create_user_relation( database db,
									char *attrib_list,
									char *dest_name,
									boolean istemporary,
									boolean issystem) ;
extern relation rl_compact( database db, relation *rel ) ;

#endif
