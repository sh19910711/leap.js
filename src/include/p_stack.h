/* 
 * Name: p_stack.h
 * Description: Parser Stack Header File
 * Version: p_stack.h,v 1.204.2.1 2004/02/09 20:05:21 rleyton Exp
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

#ifndef _LEAP_PTSTACK_
#define _LEAP_PTSTACK_

#include "dtypes.h"

extern pt_stack pt_create_stack(void) ;
extern void pt_push_stack( pt_stack st,
		parse_tree data) ;
extern parse_tree pt_pop_stack( pt_stack st ) ;
extern void pt_stack_dispose(pt_stack *st) ;
extern void pt_flush_stack(pt_stack *st) ;
extern boolean pt_stack_empty( pt_stack stack) ;
#endif
