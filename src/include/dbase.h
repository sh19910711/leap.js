/* 
 * Name: dbase.h
 * Description: Database function header file
 * Version: dbase.h,v 1.203.2.1 2004/02/09 20:05:20 rleyton Exp
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

#ifndef _LEAP_DBASE_
#define _LEAP_DBASE_

#include "dtypes.h"

#if defined(HAVE_STDARG_H)
# include <stdarg.h>
#else
# include <varargs.h>
#endif

extern void database_reverse(database db); 
extern void database_new(char name[DATABASE_NAME_SIZE], char info[DATABASE_NAME_SIZE]);
extern database LEAPAPI_db_create(const char *path,
								const char *name);
extern database LEAPAPI_db_init(const char *path, const char *name,
						 boolean sub) ;
extern void LEAPAPI_db_destroy(database *db);

extern char *database_name(database db); 
extern char *database_dir(database db);
extern void ddmaintenance(database db, char *fmt, ...);

extern FILE *ddfile;

#define database_datadictionary(db) db->datadictionary

extern database whichdb( database db, char *rel ) ;


#endif
