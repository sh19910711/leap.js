/* 
 * Name: tuples.h
 * Description: Tuple function headers.
 * Version: tuples.h,v 1.204.2.1 2004/02/09 20:05:21 rleyton Exp
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

#ifndef _LEAP_TUPLES_
#define _LEAP_TUPLES_


#include "dtypes.h"

extern relation get_relation(tuple t); 
extern void tuple_dispose(tuple *ntuple) ;
extern tuple old_readfirst_tuple(relation rel,
			FILE **tfile,
			word *noattributes,
			int reuse,
			tuple old_tuple) ;
void close_tuple(tuple *ntuple,
            int reuse) ;

extern int old_readnext_tuple(tuple *ntuple,
			FILE **tfile,
			word *noattributes,
			int reuse) ;
extern tuple old_tuple_prepare_attributes(relation rel) ;
extern void tuple_to_string(tuple t,
    	 		     char *tuple_string) ;
extern int old_write_tuple(tuple wtuple) ;
extern void tuple_print(tuple ntuple) ;
extern void tuple_def(tuple ntuple) ;
extern void tuple_revprint(relation rel,
							tuple ntuple) ;

extern attribute tuple_find_attribute( tuple ct,
				char *name) ;
extern char *tuple_find_attribute_val( tuple ct,
				char *name ) ;

/******************
 * New tuple routines
 ******************/
extern tuple tuple_readfirst( relation rel,
                                 boolean reuse,
                                 tuple old_tuple ) ;
extern tuple tuple_readnext( tuple *ctuple,
							 boolean reuse ) ;

extern tuple tuple_prepare(relation rel);

extern int tuple_write( tuple ctuple);
extern int tuple_append( tuple ctuple);
extern int tuple_appendandreturn( tuple ctuple );

extern word getendposition( tuple ctuple );
extern boolean atend( tuple ctuple, word epos );


extern int tuple_delete( word offset,
                    tuple ctuple);

extern relation buildnewrel( relation rel ) ;
extern relation build_new_relation( relation rel,
									char *path ) ;

extern FILE *relation_open( relation rel, const char *mode);
extern int relation_create_write_attribute( attribute catt,
   											FILE **fptr ) ;

extern attribute relation_attribute_read( FILE **fptr ) ;
extern attribute relation_attribute_readfirst(relation rel,
                                        tuple *ctuple,
                                        word *anum);
extern attribute relation_attribute_readnext(relation rel,
                                        tuple *ctuple,
                                        attribute cattr,
                                        word *anum);
extern attribute relation_attribute_getfirst(tuple ctuple, word *anum) ;
extern attribute relation_attribute_getnext( tuple ctuple, 
                                        attribute cattr,
                                        word *anum); 

extern int relation_create_write_attribute( attribute catt,
                                        FILE **fptr ) ;

extern int relation_create_write_eoh_marker( NOATTRIBUTES_TYPE noattributes,
                                      FILE **fptr ) ;

 
extern int relation_create_write_header( char *name,
                                  NOATTRIBUTES_TYPE noattribs,
                                  RELATION_TEMP_TYPE temporary,
				  				  RELATION_SYSTEM_TYPE system,
                                  FILE **fptr ) ;
extern int relation_update_header( database db, char *name,
                                  NOATTRIBUTES_TYPE noattribs,
                                  RELATION_TEMP_TYPE temporary,
				  				  RELATION_SYSTEM_TYPE system,
								  FILE **fptr ) ;
extern int printrelinfo( char *path );

extern int tuple_readheader(FILE **fptr,
				    NOATTRIBUTES_TYPE *noattributes,
					RELATION_TEMP_TYPE *temp,
					RELATION_SYSTEM_TYPE *system,
				    char *rname);

extern char *get_attribute_info( char *outdtype, ATTRIBUTE_TYPE_TYPE dtype) ;
extern FILE *generate_fileh( relation rel ) ;

#endif
