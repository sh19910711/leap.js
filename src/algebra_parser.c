/* 
 * Name: algebra_parser.c
 * Description: Algebra parser module
 * Version: algebra_parser.c,v 1.5.2.1 2004/02/09 20:05:20 rleyton Exp
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

#ifndef USE_ORIGINAL_PARSER

#include <stdio.h>
#include "consts.h"
#include "global_vars.h"
#include "errors.h"
#include "dtypes.h"
#include "algebra_parser.h"
#include "util.h"
#include "leapio.h"
#include "pointer_stack.h"

/* lex/yacc support routines */
#include "algebra.h"

relation algebra_execute( database db, parse_tree parsetree ) {

	boolean finished=FALSE,exec_error=FALSE;
	int count;
	pt_stack parser_stack;
	parse_tree temp_tree;

	do_debug(DEBUG_ENTER,"ALGEBRA Start: algebra_EXECUTE_query\n");

	/* Create a stack for iterating through the tree */
	parser_stack=pt_create_stack();

	/* Whilst finished override not set, and the stack's not empty! */
	while (( finished!=TRUE )||(pt_stack_empty(parser_stack)==FALSE)) {

		/* Locate the left most node */
		while ((parsetree!=NULL)&&(ptree_subnode(parsetree,ptree_currentpos(parsetree))!=NULL)){

			do_debug(DEBUG_MODERATE,"ALGEBRA: Pushing node [%d]\n",ptree_currentpos(parsetree));
			pt_push_stack(parser_stack,ptree_subnode(parsetree,ptree_currentpos(parsetree)));	

			parsetree=ptree_subnode(parsetree,ptree_currentpos(parsetree));
			do_debug(DEBUG_INFO,"ALGEBRA: New node has [%d] sub-nodes\n",ptree_no_subnodes(parsetree));
		}
		do_debug(DEBUG_INFO,"ALGEBRA: Reached LML-Node\n");

		do_debug(DEBUG_INFO,"ALGEBRA: Switch on parse tree type [%d]\n",ptree_type(parsetree));

		switch (ptree_type(parsetree)) {

			case NODE_TYPE_UNKNOWN:
					raise_error(ERROR_UNIMPLEMENTED,NONFATAL,"Uninitialised node!");
					break;

			case NODE_TYPE_TERMINATE:
					do_debug(DEBUG_INFO,"ALGEBRA: Termination node (end program) encountered\n");
					terminate=TRUE;
					break;

			case NODE_TYPE_BASE:
					do_debug(DEBUG_INFO,"ALGEBRA: Base node encountered\n");
					break;

			case NODE_TYPE_SELECT:
					do_debug(DEBUG_INFO,"ALGEBRA: Select operation\n");
					break;

			case NODE_TYPE_PROJECT:
					do_debug(DEBUG_INFO,"ALGEBRA: Project operation\n");
					break;

			case NODE_TYPE_NAME:
					do_debug(DEBUG_INFO,"ALGEBRA: Relation Name [%s]\n",parsetree->leaf[0]->item);
					break;
			default:
					raise_error(ERROR_UNIMPLEMENTED,NONFATAL,"Cannot determine operation");
					exec_error=TRUE;
					break;
		}


		/* Pass the result back up the tree */
		do_debug(DEBUG_INFO,"ALGEBRA: Handing back result relation [X]\n");

		if (pt_stack_empty(parser_stack)!=TRUE) {
			/* Move back up the tree */
			do_debug(DEBUG_MODERATE,"ALGEBRA: Popping node\n");

			parsetree=pt_pop_stack(parser_stack);
/*
			temp_tree=pt_pop_stack(parser_stack);
			parsetree=ptree_subnode(temp_tree,ptree_currentpos(temp_tree));
*/

			do_debug(DEBUG_MODERATE,"ALGEBRA: Popped node\n");
			/* Move to the next node (if appropriate) */
			ptree_currentpos(parsetree)++;
		} else {
			do_debug(DEBUG_MODERATE,"ALGEBRA: Finished with stack!\n");
			finished=TRUE;
		}
	}

	/* Dispose of the stack */
	pt_stack_dispose(&parser_stack);

	do_debug(DEBUG_ENTER,"ALGEBRA End (OK): algebra_EXECUTE_query\n");

}

relation algebra_process_query( database db,
                    char *query) {
	FILE *tmpfile;
	char tmpname[L_tmpnam+1];

	do_debug(DEBUG_ENTER,"ALGEBRA Start: algebra_process_query\n");

	do_debug(DEBUG_INFO,"Query is [%s]\n",query);

	tmpnam(tmpname);

	do_debug(DEBUG_INFO,"Got file [%s]\n",tmpname);

	tmpfile=fopen(tmpname,"wb+");

	if (tmpfile!=NULL) {
		do_debug(DEBUG_INFO,"Opened [%s]\n",tmpname);

		fprintf(tmpfile,"%s",query);
		fseek(tmpfile,SEEK_SET,0);
		yyin=tmpfile;

		if (init_parse()) {
			raise_message(MESSAGE,"%s","algebra_process_query: Parsed expression.");	

			algebra_execute(db,parse_base);

			printf("parse_base p: [%p]\n",parse_base);
			dump_parse_tree(parse_base);

		} else {
			raise_message(MESSAGE,"%s","algebra_process_query: Unable to parse expression.");	
		}

		/* Close & Remove the temporary file */
		fclose(tmpfile);
		remove(tmpname);

		do_debug(DEBUG_ENTER,"ALGEBRA End (OK): algebra_process_query\n");
		return(NULL);
	} else {
		do_debug(DEBUG_ENTER,"ALGEBRA End (PROB): algebra_process_query\n");
		raise_error(ERROR_FILE_OPENING,NONFATAL,"Generic temporary file");
		return(NULL);
	}
}

relation algebra_process_expression( database db,
                    char *query){
	raise_message(MESSAGE,"%s","algebra_process_expression Not implemented!");	

	return(NULL);
}

relation algebra_vprocess_query( database db,
                        char *fmt, ... ) {

	raise_message(MESSAGE,"%s","algebra_vprocess_query Not implemented!");	
	return(NULL);
}


#endif /*USE_ORIGINAL_PARSER*/
