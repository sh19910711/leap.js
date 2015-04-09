/* 
 * Name: info.h
 * Description: Information function headers.
 * Version: info.h,v 1.203.2.1 2004/02/09 20:05:21 rleyton Exp
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
#include "dtypes.h"

#ifndef _LEAP_INFO_
#define _LEAP_INFO_
extern void print_info(void) ;
extern void print_header(boolean brief) ;
extern void print_help(void) ;
extern void print_shutdown(void) ;
extern void do_warranty(void) ;
extern void do_addresses(void) ;
#endif
