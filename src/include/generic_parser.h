/* 
 * Name: generic_parser.h
 * Description: Generic Parser routine(s) header file.
 * Version: generic_parser.h,v 1.5.2.1 2004/02/09 20:05:20 rleyton Exp
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

#ifndef _LEAP_GENERIC_PARSE_
#define _LEAP_GENERIC_PARSE_

#ifdef USE_ORIGINAL_PARSER
#include "orig_parser.h"

#define process_query orig_process_query
#define vprocess_query orig_vprocess_query
#define vprocess_expression orig_vprocess_expression

#elif USE_SQL_PARSER
/* Not used (for now) */
#else

#include <stdarg.h>
#include "dtypes.h"
#include "algebra_parser.h"

#define process_query algebra_process_query
#define vprocess_query algebra_vprocess_query
#define vprocess_expression algebra_vprocess_expression

/*
extern relation process_query( database db,
    				char *query);
extern relation process_expression( database db,
    				char *query);
extern relation vprocess_query( database db,
						char *fmt, ... ) ;
*/

#endif /* USE_ORIGINAL_PARSER */

#endif /* ifdef _LEAP_GENERIC_PARSER_ */
