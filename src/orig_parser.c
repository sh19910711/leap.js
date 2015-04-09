/* 
 * Name: parser.c
 * Description: Original LEAP Parser module functions - Takes a string, and
 *		builds a parse tree, then executes it...
 * Version: orig_parser.c,v 1.6.2.1 2004/02/09 20:05:20 rleyton Exp
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

#ifdef USE_ORIGINAL_PARSER

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>
#if defined(HAVE_STDARG_H)
# include <stdarg.h>
#else
# include <varargs.h>
#endif

#include "consts.h"
#include "global_vars.h"
#include "dtypes.h"
#include "errors.h"
#include "orig_parser.h"
#include "util.h"
#include "pointer_stack.h"
#include "relation.h"
#include "relational_ops.h"
#include "database.h"
#include "info.h"
#include "dbase.h"
#include "tuples.h"
#include "attributes.h"
#include "vars.h"
#include "leapio.h"

#ifdef MEMORY_DEBUG
#include "dmalloc.h"
#endif

boolean parse_error;	/* TRUE if an error occurs during ptable build */
boolean exec_error;	/* TRUE if an error occurs during ptable exec */

parse_tree new_parse_node( parse_tree parent,
			   char handed) {
/* new_parse_node
 * Create a new parse node
 */
	parse_tree pt;

	pt=(parse_tree_def *) malloc(sizeof(parse_tree_def));
	check_assign(pt,"parser.new_parse_node");
	
	pt->left=NULL;
	pt->ldone=FALSE;

	pt->right=NULL;
	pt->rdone=FALSE;

	strcpy(pt->expression,"");
	pt->result=NULL;
	pt->lresult=NULL;
	pt->rresult=NULL;
	pt->hand=handed;
	strcpy(pt->target,"");

	pt->parent=parent;

	return(pt);
}

void display_ptree(parse_tree ptree,
		   int depth) {
	char temp[MAXIMUM_EXPRESSION];

	switch (ptree->hand) {
		case NODE_LEFT:
			leap_printf("LEFT NODE\n");
			break;
		case NODE_RIGHT:
			leap_printf("RIGHT NODE\n");	
			break;
		case NODE_ROOT:
			leap_printf("ROOT NODE\n");
			break;
	}
	sprintf(temp,"Depth: %i",depth);
	do_trace(temp);
	sprintf(temp,"Expression: %s",ptree->expression);
	do_trace(temp);
	sprintf(temp,"Result: <NOT KNOWN>");
	do_trace(temp);
	
	if (ptree->left!=NULL) {
		display_ptree(ptree->left,depth+1);
		sprintf(temp,"Returned to depth: %i Expression: %s",depth,ptree->expression);
		do_trace(temp);
	} 
	if (ptree->right!=NULL) {
		display_ptree(ptree->right,depth+1);
		sprintf(temp,"Returned to depth: %i Expression: %s",depth,ptree->expression);
		do_trace(temp);
	}
}

boolean checkcomment(char *string){
    int c=0;
    boolean stringfound=FALSE,commentfound=FALSE;

    
    while ((string)&&(c<strlen(string))&&((stringfound==FALSE)&&(commentfound==FALSE))) {
        if (isalpha(string[c])) {
            stringfound=TRUE;
        }
    
        if ((string[c]==COMMENT_CHAR)){
            commentfound=TRUE;
        }
        c++;
    }

    return(commentfound);
}

relation execute( database db,
			     parse_tree parsetree) {
/* process_parse_tree
 * This function takes a parse tree, and applies the operations
 * as appropriate, in the correct order, ITERATIVELY
 */
	boolean abort=FALSE;
	parse_tree ptree=parsetree,result_tree=NULL;
	pt_stack ptstack;
	char temp[MAXIMUM_INPUT_STRING];
	char tdir[FILENAME_MAX+1];
	relation rtrel;
	word count;
	unsigned int total_read_physical,total_read_logical,total_written;
	clock_t start_time=0, end_time=0;

	/* Set the execution error flag false */
	exec_error=FALSE;
	
	do_trace("Processing parse tree...");

	ptstack=pt_create_stack();

	count=0;
	total_read_physical=0;
	total_read_logical=0;
	total_written=0;

	while ( (result_tree==NULL) && (abort==FALSE) )	 {
	
		/* Reset the global counters before an operation */
		no_written=0;
		no_read_logical=0;
		no_read_physical=0;

		/* Locate the left most node */
		while (ptree->left!=NULL) {
			pt_push_stack(ptstack,ptree);
			ptree=ptree->left;
			do_trace("Done assignment");
		}

		if (result_tree==NULL) {
	
			/* Reset the parse tree node result, to save
			 * doing this in each operation
			 * TODO - Check its not already NULL..
			 */
			sprintf(temp,"Operation (%u): %s",count++,ptree->expression);
			do_trace(temp);

			ptree->result=NULL;

			if (status_timing) {
				start_time=clock();
			}

			switch (ptree->operation) {

				/* No action - just tidy the screen */
				case C_FILE_COMMENT:
				case C_COMMENT:
					leap_fprintf(stdout,"\r");
					fflush(stdout);
					break;

				/* Obsolete operations */
				case C_INFIX:
				case C_FLUSH:
				case C_HIGH:
				case C_PRINT_IDX:
				case C_IDX_STORE:
				case C_DISPLAY_INDEX:
				case C_PANIC:
					raise_error(ERROR_OBSOLETE,NONFATAL,ptree->command);
					break;

				case C_DISPLAY_REL:
					ptree->result=orig_vprocess_query(db,"display(project(%s)(%s))",LEAP_DD_RELATIONS,LEAP_DDA_RELATIONS_NAME);
					break;

				case C_LIST:
					list_source_code();
					break;

				case C_STATUS:
				case C_SHOWVARS:
					show_variables();
					break;

				/* One (non-relation) parameter */
				case C_USE:
					unset_prompt();
					change_db(ptree->first);
					break;
			  case C_LISTSRC:	
					print_source_code(ptree->first);
					break;	

			  case C_INTERNAL:
					util_internal(ptree->first);
					break;


			  case C_RECORD:
				  if (recording==FALSE) {
				  	start_record(ptree->first);
				  } else {
					stop_record();
				  }
					break;
				case C_HELP:
					print_helppage(ptree->first);
					break;
				case C_PROMPT:
					if (strlen(ptree->first)==0) {
						set_prompt(DEFAULT_PROMPT);
					} else {
						set_prompt(ptree->first);
					}
					break;

				/* Two (non-relation) parameters */
				case C_SET:
					exec_error=(set_variable(ptree->first,ptree->second)==RETURN_ERROR);
					break;

				case C_RELATION:
					ptree->result=relation_find(db,ptree->command);
					if (ptree->result==NULL) {
						abort=TRUE;
						raise_error(ERROR_CANNOT_FIND_REL,NONFATAL,ptree->command);
						exec_error=TRUE;
					}
					break;

				case C_DUPLICATE:
					ptree->result=rl_duplicate(db,ptree->lresult, ptree->target);
					break;

				case C_SELECT:
					ptree->result=rl_select(db,ptree->lresult,ptree->second,ptree->target);
					break;

				case C_DELETE:
					ptree->result=rl_erase(db,ptree->lresult,ptree->second,ptree->target);
					break;

				case C_UPDATE:
					ptree->result=rl_update(db,ptree->lresult,ptree->second,ptree->third,ptree->target);
					break;

				case C_PRODUCT:
					ptree->result=rl_product(db,ptree->lresult,ptree->rresult,ptree->target);
					break;

				case C_JOIN:
					ptree->result=rl_naturaljoin(db,ptree->lresult,ptree->rresult,ptree->third,ptree->target,UNKNOWN_JOIN);
					break;

				case C_NATURAL_JOIN:
					ptree->result=rl_naturaljoin(db,ptree->lresult,ptree->rresult,ptree->third,ptree->target,NATURAL_JOIN);
					break;
	
				case C_INTERSECT:
					ptree->result=rl_intersect(db,ptree->lresult,ptree->rresult,ptree->target);
					break;

				case C_DIFFERENCE:
					ptree->result=rl_difference(db,ptree->lresult,ptree->rresult,ptree->target);
					break;
	
				case C_UNION:
					ptree->result=rl_union(db,ptree->lresult,ptree->rresult,ptree->target);
					break;

				case C_PROJECT:
					ptree->result=rl_project(db,ptree->lresult,ptree->second,ptree->target);
					break;

				case C_ADD:
					ptree->result=insert(ptree->lresult,ptree->second);
					break;
	
				case C_DISPLAY:
					break;

				case C_RENAME:
					ptree->result=relation_rename(db,ptree->first,ptree->second); 
					break;

				case C_SRCFILE:
					if (strlen(ptree->first)>0) {
						sprintf(temp,"%s%s%s%s",database_dir(current_db),LEAP_SOURCE_DIR,ptree->first,LEAP_SOURCE_EXT);
						assign_input_stream(temp);
					} 
					break;	

				case C_PRINT:
					ptree->result=rl_display(ptree->lresult);
					break;
				case C_DESCRIBE:

					if ((status_tempdb==TRUE) && (configuration!=TRUE) && (strcmp(database_name(db),TEMPDB_NAME)!=0) && (ptree->lresult->name[0]=='z') && (ptree->lresult->name[1]=='z')) {
						ptree->result=orig_vprocess_query(tempdb,"display(project(select (%s) (%s=\"%s\")) (%s,%s,%s))",
						LEAP_DD_ATTRIBUTES,LEAP_DDA_ATTRIBUTES_RELATION,relation_name(ptree->lresult),
					    LEAP_DDA_ATTRIBUTES_ATTRIBUTE,LEAP_DDA_ATTRIBUTES_TYPE,LEAP_DDA_ATTRIBUTES_SIZE);
					} else {
						ptree->result=orig_vprocess_query(db,"display(project(select (%s) (%s=\"%s\")) (%s,%s,%s))",
						LEAP_DD_ATTRIBUTES,LEAP_DDA_ATTRIBUTES_RELATION,relation_name(ptree->lresult),
					    LEAP_DDA_ATTRIBUTES_ATTRIBUTE,LEAP_DDA_ATTRIBUTES_TYPE,LEAP_DDA_ATTRIBUTES_SIZE);
					}
					break;
				case C_CHANGE:
					relation_change(db,ptree->lresult);
					ptree->result=ptree->lresult;
					break;
				case C_DUMP:
					dump_rel(ptree->lresult);
					ptree->result=ptree->lresult;
					break;
				case C_DELREL:
					if (relation_system(ptree->lresult)!=TRUE) {
						relation_remove(db,&ptree->lresult);
						ptree->result=ptree->lresult;
					} else {
						raise_error(ERROR_DELSYSTEM,NONFATAL,relation_name(ptree->lresult));	
					}
					break;
				case C_VERSION:
					print_header(FALSE);
					break;
				case C_INFO:
					print_header(FALSE);
					print_info();
					break;
				case C_WARRANTY:
					do_warranty();
					break;

				case C_ADDRESSES:
					do_addresses();
					break;

				case C_TERMINATENOW:
					/* Terminate and shutdown */
					terminate=TRUE;
					terminatenow=TRUE;
					break;
					
				case C_EXIT:
					/* Terminate! */
					terminate=TRUE;
					break;

				case C_CREATE_RELATION:
					/* Create relation */
					do_trace("Creating a relation...");
					if (configuration==FALSE) {
						ptree->result=create_user_relation(db,ptree->second,ptree->first,TRUE,FALSE);
					} else {
						ptree->result=create_user_relation(db,ptree->second,ptree->first,FALSE,FALSE);
					}
					break;
					
				case C_CREATE:
					do_trace("Creating a database...");
					sprintf(tdir,"%s%s",LEAP_BASE_DIR,LEAP_DATABASE_DIR);
					(void) LEAPAPI_db_init( tdir, ptree->first, TRUE );
					break;

				case C_REVERSE_ENG:
					/* Reverse engineer a database..."); */
					do_trace("Reverse engineering database...");
					database_reverse( db );
					break;

				case C_DOS:
					/* Execute an Operating system call. 
				     * TODO: Handle non-execution etc. This could be
					 * very environment specific...
					 */
#ifndef __MSDOS__
					system(ptree->first);
#endif
					break;

				default:
					raise_error(ERROR_UNIMPLEMENTED,NONFATAL,"Cannot determine operation");
					exec_error=TRUE;
					break;
			}

			if (status_timing) {		
				end_time=clock();
			}
	
			if (exec_error!=FALSE) {
				raise_error(ERROR_PARSE_EXECUTION,NONFATAL,ptree->expression);
			}
			
			/* If the parent node is assigned, ie. this is not the
			 * Root node of the parse tree 
			 */
			if (ptree->parent!=NULL) {
				switch (ptree->hand) {
					case NODE_LEFT:
						ptree->parent->lresult=ptree->result;
						ptree->parent->left=NULL;
						break;
					case NODE_RIGHT:
						ptree->parent->rresult=ptree->result;
						ptree->parent->right=NULL;
						break;
					case NODE_ROOT:
					default:
						parse_error=TRUE;
						raise_error(ERROR_UNDEFINED,NONFATAL,"");
				}
	
				/* Free the memory used - TODO - Keep this for inspection
				 * of parse results after execution 
				 */
				free(ptree);
				ptree=NULL;
			
				if (pt_stack_empty(ptstack)==FALSE) {
					ptree=pt_pop_stack(ptstack);
				} else {
					exec_error=TRUE;
					raise_error(ERROR_PARSE_EXECUTION,NONFATAL,"Cannot determine next token.");
					/* TODO - Sort out error messages - DONE 11.05.1996 */	
	
					/* MEMORY-LEAK: What about unexecuted operations on the stack? */
				}
			} else 
				result_tree=ptree;
		}
		/* Get the parent node off of the stack */

		if (ptree->right!=NULL) {
			/* Push the node back onto the stack (again) */
			pt_push_stack(ptstack,ptree);

			/* Goto the right hand node */
			ptree=ptree->right;
		}

		if (status_timing==TRUE) {
			leap_fprintf(stdout,"Logical Reads:  %d\nPhysical Reads: %d\nWrites:         %d\n",
				no_read_logical, no_read_physical, no_written);

			leap_fprintf(stdout,"Time: %-.11f\n",(((float)end_time)-((float)start_time))/CLOCKS_PER_SEC);

			total_read_logical+=no_read_logical;
			total_read_physical+=no_read_physical;
			total_written+=no_written;
		}	
	}

	do_trace("Finished processing.");

	if (exec_error==FALSE) {
		rtrel=result_tree->result;
	} else {
		rtrel=NULL;
		do_trace("Errors occured.");
	}

	do_trace("Completed parse tree execution.");

	if (result_tree!=NULL) {
		free(result_tree);
	}

	pt_flush_stack(&ptstack);

	if (status_timing==TRUE) {
		leap_fprintf(stdout,"Total Logical Reads:  %d\nTotal Physical Reads: %d\nTotal Writes:         %d\n",
			total_read_logical, total_read_physical, total_written);
	}

	return(rtrel);
}

parse_tree parse( database db,
			char *query) {
/* process_query
 * Takes a string, and processes it as an expression, returning
 * a relation, or NULL if nothing results, or an error occurs
 * check error codes in global vars for error.
 */
	parse_tree ptree,reftree;
	char result[MAXIMUM_EXPRESSION],*assignment,destination[RELATION_NAME_SIZE+1];
	char temp[255],*sptr;
	int operation;

#ifdef DEBUG
	report("Using Iterative parser");
	report("Building Parse Tree");
#endif	

	/* Create a new node */
	ptree=new_parse_node(NULL,NODE_ROOT);
	reftree=ptree;

	/* Put the main query in it */
	strcpy(ptree->expression,query);

	/* Reset the parse_error inicator */
	parse_error=FALSE;
	exec_error=FALSE;

	/* Whilst the ptree is assigned */
	while (ptree!=NULL) {

		/* Preserve the full expression (just in case) */
		/* TODO - Do we need to preserve the whole expression? DONE: Yes */
 		/*     because of create [relation|database] which takes lots of parameters */
		/*     that this routine cannot deal with properly (just yet) */

		strip_leading_spaces(ptree->expression);

		strcpy(ptree->work,ptree->expression);

		/* Cut out all of the bracketed items */
		/* Don't force brackets on a string */
		if (ptree->expression[0]!='!') {

			if ( (ptree->expression[0]=='@') && (ptree->expression[1]!=' ') ) {
				sprintf(temp,"Parser: Processing variable %s\n",ptree->command);
				do_trace(temp);
				sptr=resolve_variable(&(ptree->expression[1]));
				if (sptr!=NULL) {
					strcpy(ptree->work,sptr);
				} else {
					raise_error(ERROR_UNDEFINED_VARIABLE,NONFATAL,ptree->work);
				}
			}

			strcpy(ptree->first,cut_to_right_bracket(ptree->work,1,FALSE,result));
			ptree->expressions[1]=ptree->first;

			strcpy(ptree->second,cut_to_right_bracket(ptree->work,1,FALSE,result));
			ptree->expressions[2]=ptree->second;

			strcpy(ptree->third,cut_to_right_bracket(ptree->work,1,FALSE,result));
			ptree->expressions[3]=ptree->third;

			/* Check to see if there is an assignment operation in effect */
			assignment=strstr(ptree->work,ASSIGNMENT);
	
			if (assignment!=NULL) {
				strncpy(ptree->target,ptree->work,RELATION_NAME_SIZE);
				strtok(ptree->target,ASSIGNMENT);
				strip_trailing_spaces(ptree->target);
				strcpy(ptree->work,assignment+1);
			} else { 
				strcpy(ptree->target,generate_random_string(RANDOM_NAME_SIZE,destination));
			}
		
			/* Get the command in operation - doesn't matter where its located */
			strcpy(ptree->command,cut_token(ptree->work,'\0',result));

			if (strlen(ptree->first)==0) {
				strcpy(ptree->first,cut_token(ptree->work,'\0',result));
				if (strlen(ptree->second)==0) {
					strcpy(ptree->second,cut_token(ptree->work,'\0',result));
					if (strlen(ptree->third)==0) {
						strcpy(ptree->third,cut_token(ptree->work,'\0',result));
					}
				}
			}

#ifdef FULL_DEBUG
 	leap_fprintf(stdout,"NEW: command==%s\n1st==%s\n2nd==%s\n3rd==%s\n",
		ptree->command,ptree->first,ptree->second,ptree->third);
#endif	

			/* Try to locate the command */
			operation=get_command(ptree->command);
		} else {
			strcpy(ptree->first,&ptree->work[1]);
			operation=get_command("!");
		}

		/* If it's NOT found, then it's a relation, a variable, or 
		 * invalid command */	
		if (operation==-1) {
			/* Print what we think... */
			sprintf(temp,"Parser: (Leaf node) Relation: %s",ptree->command);
			do_trace(temp);

			/* Store our thoughts... */
			ptree->operation=C_RELATION;
		/* Ok, so its an operation of some sort... */
		} else {
			ptree->operation=operation;
			sprintf(temp,"Parser: (Branch node) Operation: %s",ptree->command);
			do_trace(temp);
	
			switch (ptree->operation) {
				/* What sort of operation is it... */

				/* Obsoleted (Treat the same as no args) */
				case C_INFIX:
				case C_FLUSH:
				case C_HIGH:
				case C_PRINT_IDX:
				case C_IDX_STORE:
				case C_DISPLAY_INDEX:
				case C_PANIC:

				/* No arguments */
				case C_COMMENT:
				case C_FILE_COMMENT:
				case C_VERSION:
				case C_INFO:
				case C_EXIT:
				case C_TERMINATENOW:
				case C_DISPLAY_REL:
				case C_WARRANTY:
				case C_ADDRESSES:
				case C_LIST:
				case C_STATUS:
				case C_SHOWVARS:
				case C_REVERSE_ENG:
				/* Treated the same as one argument  */

				/* Any number of non-relation arguments */
				case C_USE:
				case C_SRCFILE:
				case C_HELP:
				case C_SET:
				case C_LISTSRC:
				case C_PROMPT:
				case C_RENAME:
				case C_CREATE_RELATION:
			    case C_RECORD:
				case C_INTERNAL:
					ptree->ldone=TRUE;
					ptree->rdone=TRUE;
					break;

				/* Lots of relations and arguments... */
				case C_CREATE:
					/* Sneaky way of getting the info we need in the
					 * place we want it. Irregular use of brakets in
					 * create operation necessitates this (for now!).
					 */
					cut_token(ptree->work,'\0',ptree->second);
					cut_token(ptree->work,'\0',ptree->third);
					ptree->ldone=TRUE;
					ptree->rdone=TRUE;
					break;

				/* All remaining items are arguments... */
				case C_DOS:
					ptree->ldone=TRUE;
					ptree->rdone=TRUE;
					break;

				/* One relation */
				case C_PRINT:
				case C_DESCRIBE:
				case C_CHANGE:
				case C_DUMP:
				case C_DELREL:
					ptree->left=new_parse_node(ptree,NODE_LEFT);
					ptree->rdone=TRUE;
					strcpy(ptree->left->expression,ptree->first);
					break;

				/* Two relations */
				case C_PRODUCT:
				case C_UNION:
				case C_INTERSECT:
				case C_DIFFERENCE:
				case C_NATURAL_JOIN:
					ptree->left=new_parse_node(ptree,NODE_LEFT);
					ptree->right=new_parse_node(ptree,NODE_RIGHT);
					strcpy(ptree->left->expression,ptree->first);
					strcpy(ptree->right->expression,ptree->second);
					break;

				/* One relation + One set of arguments */
				case C_DUPLICATE:
				case C_SELECT:
				case C_PROJECT:
				case C_ADD:
				case C_DELETE:
					ptree->left=new_parse_node(ptree,NODE_LEFT);
					/* Mark the right (empty) node as "done" */
					ptree->rdone=TRUE;

					strcpy(ptree->left->expression,ptree->first);

					break;

				/* One relation + Two sets of arguments */
				case C_UPDATE:
					ptree->left=new_parse_node(ptree,NODE_LEFT);
					/* Mark the right (empty) node as "done" */
					ptree->rdone=TRUE;

					strcpy(ptree->left->expression,ptree->first);
					break;

				/* Two relations + One set of arguments */
				case C_JOIN:
					ptree->left=new_parse_node(ptree,NODE_LEFT);
					ptree->right=new_parse_node(ptree,NODE_RIGHT);

					strcpy(ptree->left->expression,ptree->first);
					strcpy(ptree->right->expression,ptree->second);
					break;
			
				/* Anything else is probably a mess up. */
				default:
					ptree->ldone=TRUE;
					ptree->rdone=TRUE;
					parse_error=TRUE;
					raise_error(ERROR_UNIMPLEMENTED,NONFATAL,ptree->command);

			}
		}

		/* OLD parse_tree build section in old_code */

		/* Determine where to go next */

		if ((ptree->left!=NULL) && (!ptree->ldone)) {
			ptree->ldone=TRUE;
			ptree=ptree->left;
		} else if ((ptree->right!=NULL) && (!ptree->rdone)) {
			ptree->rdone=TRUE;
			ptree=ptree->right;
		} else if (ptree->parent!=NULL) {
			ptree=ptree->parent;
			while ( (ptree->parent!=NULL) && (ptree->ldone) && (ptree->rdone) ) 
				ptree=ptree->parent;

			if (ptree->parent!=NULL) {
				if ( (ptree->right!=NULL) && (!ptree->rdone) ) {
					ptree->rdone=TRUE;
					ptree=ptree->right;
				}
			}
		}
		/* This is the root node */
		if (ptree->parent==NULL) {
			if ( (ptree->ldone) && (ptree->rdone) ) {
				ptree=NULL;
			} else {
				if ( (ptree->left!=NULL) && (!ptree->ldone) ){
					/* This shouldn't ever need to be executed */
					ptree->rdone=TRUE;
					ptree=ptree->left;
				} else if ( (ptree->right!=NULL) && (!ptree->rdone) ) {
					ptree->rdone=TRUE;
					ptree=ptree->right;
				} else {
					parse_error=TRUE;
					raise_error(ERROR_PARSE_UNRECOGNISED_TOKEN,NONFATAL,ptree->expression);
					ptree=NULL;
				}
			}
		}
	}
	do_trace("Parse Tree build completed.");

	return(reftree);
}

int optimise( parse_tree ptree ) {
/*
 * optimise
 * Performs an optimisation sweep of the parse tree
 */

	do_trace("Optimisation Stage: Not yet implemented");

	return(RETURN_SUCCESS);

}

relation orig_process_query( database db,
						char *query ) {
/* 
 * process_query
 * High level control routine for processing a query 
 */ 
	parse_tree ptree;
	relation rtrel;

	/* Absolutely first... record the command for the activity log... */
	if (ACTIVITY_FILE!=NULL) {
		fprintf(ACTIVITY_FILE,query);
		fprintf(ACTIVITY_FILE,"\n");
	}

	/* Is this a comment? */
	if (!checkcomment(query))  {

		/* Now, build a parse tree */
		ptree=parse(db,query);

		/* Does the user want to display the tree? */
		if (status_trace) {
			/* display parse tree */
			do_trace("Parse Tree");
			do_trace("==========");
			display_ptree(ptree,0);
			do_trace("End Parse Tree");
			do_trace("==============");
		}
	
		/* If a parse error occured, then report it. 
	 	*/
		if (parse_error==TRUE) {
			do_trace("Error(s) occured during parse tree build.");
			rtrel=NULL;
		} else {
			do_trace("Parse tree built successfully.");
	
			/* Optimise the parse tree */
			if (optimise(ptree)!=RETURN_SUCCESS) {
				do_trace("Error(s) occured during optimisation stage.");
			}
	
			/* Do the actual work of executing the parse tree */
			rtrel=execute(db,ptree);
		}
	
		if ( (exec_error==FALSE) && (parse_error==FALSE) ) {
			unset_prompt();
		} else {
			set_prompt(DEFAULT_ERROR_PROMPT);
		}
	
	
		if (rtrel!=NULL) {
			set_variable(VAR_LAST,relation_name(rtrel));
		}	
		/* return relation */
		return(rtrel);
	} else {
		return(NULL);
	}
}
	
relation orig_vprocess_query( database db,
						char *fmt, ... ) {
/* vprocess_query
 * Allows variable length query processing
 */
						
	va_list ap;
	char expression[MAXIMUM_EXPRESSION+1];
	
	va_start(ap, fmt);
	
	vsprintf(expression, fmt, ap);
	
	do_debug(DEBUG_INFO,"Query generated by vprocess_query: %s\n",expression);
	
	return( orig_process_query(db,expression) );
		
}


#endif /* USE_ORIGINAL_PARSER */
