/* 
 * Name: pointer_stack.c
 * Description: Generic stack functions
 * Version: pointer_stack.c,v 1.4.2.1 2004/02/09 20:05:20 rleyton Exp
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
# include "defines.h"
#endif
#include <stdio.h>
#include <stdlib.h>
#include "consts.h"
#include "global_vars.h"
#include "dtypes.h"
#include "errors.h"
#include "pointer_stack.h"
#include "util.h"
#include "leapio.h"

pt_stack pt_create_stack(void) {
/* ps_create_stack 
 * Creates a stack structure
 */
	pt_stack pts;

	pts=(pt_stack_struct *) malloc (sizeof(pt_stack_struct));
	check_assign(pts,"p_stack.pt_create_stack");

	pts->head_node=NULL;
	pts->no_items=0;
	
	return(pts);
}

void pt_push_stack( pt_stack st,
		void *data) {
/* pt_push_stack
 * Push an item onto the stack
 */
	pt_stack_node st_node;

	st_node=(pt_stack_node_struct *) malloc (sizeof(pt_stack_node_struct));
	check_assign(st_node,"p_stack.pt_push_stack");

	do_trace("Populating stack...");
	st_node->data=data;
	
	st_node->next=st->head_node;
	st->head_node=st_node;
	(st->no_items)++;
}

void *pt_pop_stack( pt_stack st ) {
/* pt_pop_stack
 * Pop an item off of the stac
 */
	pt_stack_node node;
	void *rdata;

#ifdef FULL_DEBUG
	leap_fprintf(stdout,"no_items==%d\n",st->no_items);
#endif

	/* Get the data from the head node */
	if (st->no_items!=0) {
		rdata=st->head_node->data;

		/* Get the head node */	
		node=st->head_node;

		/* Change the head node, to the next node */
		st->head_node=node->next;

		/* If its an ok node */
		if (node!=NULL) {
			/* Dispose of the memory */
			free(node);
			node=NULL;
		} else {
			/* Its a NULL node... */
			raise_error(ERROR_INTERNAL_STACK,NONFATAL,"(pt_pop_stack)");
		}

		(st->no_items)--;

		/* Return the data of node we took */
		return(rdata);
	} else {
		raise_error(ERROR_INTERNAL_STACK,NONFATAL,"(pt_pop_stack) - No more nodes!");
		return(NULL);
	}
}



unsigned int pt_stack_size( pt_stack stack) {
	return(stack->no_items);
}

boolean pt_stack_empty( pt_stack stack) {
	if ( stack->head_node == NULL ) return(TRUE);
	else {
#ifdef FULL_DEBUG
		leap_fprintf(stdout,"Stack contains %d items\n",stack->no_items);
#endif 
		return(FALSE);
	}
}

void pt_stack_dispose(pt_stack *st) {
/* pt_stack_dispose
 * Dispose of a stack structure 
 */

	if (*st!=NULL) {
		free(*st);
	} else {
		raise_error(ERROR_INTERNAL_STACK,NONFATAL,"(pt_stack_dispose)");
	}
	(*st)=NULL;
}

void pt_flush_stack(pt_stack *st) {
/* pt_flush_stack
 * Dispose of ALL Items in a stack structure
 */
#ifdef FULL_DEBUG
	leap_fprintf(stdout,"Stack contains %d items\n",(*st)->no_items);
#endif
	while (!(pt_stack_empty(*st))) 
		(void) pt_pop_stack(*st);	

	pt_stack_dispose(st);
}
	
