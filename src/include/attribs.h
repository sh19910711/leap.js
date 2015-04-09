/* 
 * Name: attribs.h
 * Description: Attribute function header file.
 * Version: attribs.h,v 1.204.2.1 2004/02/09 20:05:20 rleyton Exp
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


#ifdef HAVE_CONFIG_H
#include "defines.h"
#endif

#ifndef _LEAP_ATTRIBUTE_
#define _LEAP_ATTRIBUTE_



#include "dtypes.h"
#include "stdio.h"

extern int attribute_create(	relation rel, 
			char *attribute_name, 
			char data_type,
			int  attrib_size ) ;
extern attribute attribute_build(	relation rel, 
			char *attribute_name, 
			char data_type,
			int  attrib_size ) ;
extern attribute attribute_findfirst(relation rel,FILE **attribute_file) ;
extern attribute attribute_findnext( attribute att,
				FILE **attribute_file,
				boolean newnode,
				boolean node_dispose) ;
extern attribute attribute_find ( relation rel, char *name) ;
extern void attribute_display(relation rel) ;
extern void attribute_print(attribute att) ;
extern void attributes_printtuple(tuple ctuple) ;
extern void attributes_printfromrel(relation rel) ;
extern void attribute_dispose(attribute *att);
extern char *fmt_build(char *fmt,attribute att,int *attrsize) ;

/* Some rather useful macros - better than functions */
#define attribute_name(attribute) attribute->name
#define attribute_type(attribute) attribute->data_type
#define attribute_size(attribute) attribute->attrib_size
#define attribute_no(attribute) attribute->no
#define attribute_fmt(attribute) attribute->fmt

#endif
