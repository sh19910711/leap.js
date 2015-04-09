/* 
 * Name: cond.c
 * Description: Routines for evaluating conditions against tuples.
 * Version: cond.c,v 1.9.2.1 2004/02/09 20:05:20 rleyton Exp
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
#include <string.h>
#include "consts.h"
#include "globals.h"
#include "dtypes.h"
#include "errors.h"
#include "cond.h"
#include "tuples.h"
#include "util.h"
#include "relation.h"
#include "leapio.h"
#include "regexp.h"

boolean do_int_eval(int boolval, 
				   attribute left_attr, char *lptr, 
				   attribute right_attr, char *rptr,
				   boolean *errorflag) {
/* do_int_eval
 * Performs an integer based evaluation of the data
 */
    int lint=0,rint=0,errs=-1, err= 0;
	boolean cresult=FALSE;
    regexp *regresult;
                

 		do_debug(DEBUG_ENTER,"Start: do_int_eval\n");

		/* Mark no error. */
		*errorflag=FALSE;

		/* Convert the left attribute to a number */
		errs=sscanf(lptr,"%d",&lint);

		/* Check for conversion errors */
		if (errs==EOF) {
			/* An error occured */
			*errorflag=TRUE;
			raise_error(ERROR_TYPE_CONVERSION,NONFATAL,lptr);
 			do_debug(DEBUG_ENTER,"End (Error): do_int_eval\n");
		} else {
			/* No error occured */

			/* Convert the right attribute to a number */
			errs=sscanf(rptr,"%d",&rint);

			/* If an error occured during conversion */
			if (errs==EOF) {
	
				/* Mark the error, and abort */
				*errorflag=TRUE;
				raise_error(ERROR_TYPE_CONVERSION,NONFATAL,rptr);
 				do_debug(DEBUG_ENTER,"End (Error): do_int_eval\n");
			} else {

				/* Depending on the type of evaluation... */
				switch (boolval) {

				/* Calculate the result */
					case BOOLEAN_EQUAL : cresult=(lint==rint);
									     break;
		
					case BOOLEAN_NOT_EQUAL : cresult=(lint!=rint);
											 break;
		
					case BOOLEAN_GREATER_THAN : cresult=(lint>rint);
												break;
		
					case BOOLEAN_GREATER_EQUAL_THAN : cresult=(lint>=rint);
												      break;
				
					case BOOLEAN_LESS_THAN : cresult=(lint<rint);
												break;
		
					case BOOLEAN_LESS_EQUAL_THAN : cresult=(lint<=rint);
													break;
		
	                case BOOLEAN_LIKE:
	   			        regresult=regcomp(rptr);
						if (regresult==NULL) {
							raise_error( ERROR_REGEXP,NONFATAL,rptr);
						}
					do_debug(DEBUG_ENTER,"Start (LEAP): regexec\n");
   	        			err=regexec(regresult, lptr);
					do_debug(DEBUG_ENTER,"End (LEAP): regexec\n");
					do_debug(DEBUG_INFO,"err: [%d]\n",err);
					if (err==0) {
					    cresult=FALSE;
					} else {
					    cresult=TRUE;
					}
					do_debug(DEBUG_INFO,"cresult: [%d]\n",cresult);
					free(regresult); 
                   		break;

					default : raise_error(ERROR_UNDEFINED,NONFATAL,"Something Strange has happenened (Query evaluation)");	
							  *errorflag=TRUE;
 							  do_debug(DEBUG_ENTER,"End (Error): do_int_eval\n");
							  break;
				}
			}
		}

 	do_debug(DEBUG_ENTER,"End (Ok): do_int_eval\n");
	/* Return the result */
	return(cresult);
}

boolean do_string_eval(int boolval,
					   attribute left_attr, char *lptr,
					   attribute right_attr, char *rptr,
					   boolean *errorflag) {

		boolean cresult=FALSE;
     	regexp *regresult;
    	int err;

 		do_debug(DEBUG_ENTER,"Start: do_string_eval\n");
		*errorflag=FALSE;

		if ( (status_debug) && (lptr) && (rptr) ){
			do_debug(DEBUG_MODERATE,"STR COMP: Left==[%s] Right==[%s]\n",lptr,rptr);
		}
		/* Convert to upper case if case sensitivity is on */
		if (status_case==TRUE) {
			upcase(lptr);
			upcase(rptr);
		}
		switch (boolval) {

			case BOOLEAN_EQUAL : cresult=(strcmp(lptr,rptr)==0);
							     break;

			case BOOLEAN_NOT_EQUAL : cresult=(strcmp(lptr,rptr)!=0);
									 break;

			case BOOLEAN_GREATER_THAN : cresult=(strcmp(lptr,rptr)>0);
										break;

			case BOOLEAN_GREATER_EQUAL_THAN : cresult=(strcmp(lptr,rptr)>=0);
										      break;
		
			case BOOLEAN_LESS_THAN : cresult=(strcmp(lptr,rptr)<0);
										break;

			case BOOLEAN_LESS_EQUAL_THAN : cresult=(strcmp(lptr,rptr)<=0);
											break;

        	case BOOLEAN_LIKE:

					do_debug(DEBUG_INFO,"Regular expression string comparison\n");
 					do_debug(DEBUG_ENTER,"Start (LEAP): regcomp\n");
   			        regresult=regcomp(rptr);
 					do_debug(DEBUG_ENTER,"End (LEAP): regcomp\n");
					if (regresult==NULL) {
						raise_error( ERROR_REGEXP,NONFATAL,rptr);
 						do_debug(DEBUG_ENTER,"End (LEAP-Err): regcomp\n");
					}
 					do_debug(DEBUG_ENTER,"Start (LEAP): regexec\n");
           			err=regexec(regresult, lptr);
 					do_debug(DEBUG_ENTER,"End (LEAP): regexec\n");
					do_debug(DEBUG_INFO,"err: [%d]\n",err);
					if (err==0) {
					    cresult=FALSE;
					} else {
					    cresult=TRUE;
					}
					do_debug(DEBUG_INFO,"cresult: [%d]\n",cresult);
				free(regresult); 
                	break;
                    
			default : raise_error(ERROR_UNDEFINED,NONFATAL,"Something Strange has happenened (Query evaluation)");	
 					  do_debug(DEBUG_ENTER,"End (Error): do_string_eval\n");
					  *errorflag=TRUE;
					  break;
		}

	 	do_debug(DEBUG_ENTER,"End (Ok): do_string_eval\n");
		return(cresult);
}

boolean evaluate( condp condition,
				  tuple ltuple, tuple rtuple ) {
/* evaluate
 * Evaluates a condition based on condition structure, and 
 * specified tuple, and returns TRUE or FALSE 
 */
	condp ccond;
	boolean errorflag=FALSE,cresult=FALSE,final_result=FALSE;
	int booleanops[MAXIMUM_ATTRIBUTES]; /* For boolean operators */
	boolean allresults[MAXIMUM_ATTRIBUTES]; /* For results */
	int counter,noresults;
	char *lptr,*rptr;
	int ldtype=DT_UNDEFINED, rdtype=DT_UNDEFINED;


	do_debug(DEBUG_ENTER,"Start: evaluate\n");
	ccond=condition;

	counter=0;
	while ( (errorflag==FALSE) && (ccond!=NULL) ) {

		if (ccond->left_field==TRUE) {
			lptr=ccond->left_attribute_val;
		} else {
			lptr=ccond->left;
		}
		
		if (ccond->right_field==TRUE) {
			rptr=ccond->right_attribute_val;
		} else {
			rptr=ccond->right;
		}

		if (ccond->left_field == TRUE) {
			ldtype=ccond->left_attribute->data_type;
		} else {
			ldtype=DT_UNDEFINED;
		}

		if (ccond->right_field == TRUE) {
			rdtype=ccond->right_attribute->data_type;
		} else {
			rdtype=DT_UNDEFINED;
		}

		if ( (ldtype!=DT_UNDEFINED) && (rdtype!=DT_UNDEFINED) ) {
			if ( (ldtype!=rdtype) ) {
				raise_error(ERROR_TYPE_INCOMPATIBLE,NONFATAL,"");
				return(FALSE);
			}
		}

		if ( (ldtype==DT_NUMBER) || (rdtype==DT_NUMBER) ) {
			cresult=do_int_eval(ccond->boolval, ccond->left_attribute, lptr, ccond->right_attribute, rptr, &errorflag); 
		} else 
		if ( (ldtype==DT_STRING) || (rdtype==DT_STRING) ) {
			cresult=do_string_eval(ccond->boolval, ccond->left_attribute, lptr, ccond->right_attribute, rptr, &errorflag); 
		} else
		if ( (ldtype==DT_BOOLEAN) || (rdtype==DT_BOOLEAN) ) {
			cresult=do_string_eval(ccond->boolval, ccond->left_attribute, lptr, ccond->right_attribute, rptr, &errorflag); 
		} else
			cresult=do_string_eval(ccond->boolval, ccond->left_attribute, lptr, ccond->right_attribute, rptr, &errorflag); 

		/* Capture information to save repassing Condition structure */
		allresults[counter]=cresult;
		booleanops[counter]=ccond->boolean_condition;

		/* Move onto the next item */
		counter++;	
		ccond=ccond->next_condition;
	}

	noresults=counter;

	final_result=allresults[0];

	for (counter=1; counter<noresults; counter++) {

		switch (booleanops[counter-1]) {

			case BOOLEAN_AND : final_result=(final_result && allresults[counter]);
								break;

			case BOOLEAN_OR  : final_result=(final_result || allresults[counter]);
								break;

			case BOOLEAN_XOR : raise_error(ERROR_UNIMPLEMENTED, NONFATAL, "XOR - Sorry to have lead you on...");
							   do_debug(DEBUG_ENTER,"End (Unimplemented=Err): evaluate\n");
								break;

			case BOOLEAN_END_MARKER : break; /* Do nothing. The end is nigh */
			default : raise_error(ERROR_UNIMPLEMENTED,NONFATAL,"Unsupported boolean operator.");
							    do_debug(DEBUG_ENTER,"End (Unsupported=Err): evaluate\n");
								break;
		}
	}
	
	do_debug(DEBUG_ENTER,"End (Ok): evaluate\n");
	return(final_result);
}

boolean string_position_ok(int cpos, int string_length, char *text) {
/* check_string_position
 * Check the current position (cpos) in the string (string_length),
 * whilst evaluating text
 */
	/* If cposition = string_length at this stage, we're missing
	 * some important features! 
	 */
	if (cpos>=string_length) {
		raise_error(ERROR_EVALUATING_EXPR,NONFATAL,text);
		return(FALSE);
	} else {
		return(TRUE);
	}
}

int get_first_expression(char *text,
			 condp cond) {
/* get_first_expression
 * Takes an expression from the string specified (text), where
 * a string is seperated by boolean expressions in the bool array
 * REWRITE: This needs to be tidied up somewhat.
 */

	int start=0,cposition=0,boolean_start=0,boolean_end=0,endpos=0,string_length=0;
	int start_end=0, end_start=0;
	boolean errorflag=FALSE,sposok;
	char temp[MAXIMUM_EXPRESSION],*lptr,*rptr;

	string_length=strlen(text);

	cond->left_always=ALWAYS_UNKNOWN;
	cond->right_always=ALWAYS_UNKNOWN;

#ifdef FULL_DEBUG
	fprintf(stderr,"About to process (%d): %s\n",string_length,text);
#endif

/* This section here locates the start and end of the appropriate
 * sections of the expression: left bool right
 */

	/* Locate the first text */
	while ( (cposition<string_length) && ( ( (text[cposition]==' ') || (text[cposition]=='(') )
		&& ( (text[cposition]<'A') || (text[cposition]>'Z') )
		&& ( (text[cposition]<'a') || (text[cposition]>'z') ) ) ) {
		cposition++;
	}

	/* Store the start of the text */
	start=cposition;

	/* Check the string position. Return early if things don't work */
	sposok=string_position_ok(cposition,string_length,text);
	if ( sposok != TRUE ) {
		return(RETURN_ERROR);
	}	

	/* Scan through the string to locate the first non
	 * character, ie. condition - <, <=, =, >=, >, <>
	 * OR SPACE! or TILDE
	 */

	/* Locate the start of the condition */
	while ( (cposition<string_length) &&  (text[cposition]!='=') && (text[cposition]!='<')
            && (text[cposition]!='>') && (text[cposition]!=' ') && (text[cposition]!='~')) {
		cposition++;
	}

	/* Check the string position. Return early if things don't work */
	sposok=string_position_ok(cposition,string_length,text);

	/* Record END of STARTing section */
	start_end=cposition;
	
	if (text[cposition]==' ') {
		/* Skip over any spaces between left val and condition */
		while ( (cposition<string_length) && (text[cposition]==' ')) {
			cposition++;
		}
	}

	/* Store the start of the condition */
	boolean_start=cposition;

	/* Locate the end of the condition */	
	while ( (cposition<string_length) && ( (text[cposition]=='=') || (text[cposition]=='<')
                                           || (text[cposition]=='>') || (text[cposition]=='~') ) ){
		cposition++;
	}


	/* Check the string position. Return early if things don't work */
	sposok=string_position_ok(cposition,string_length,text);
	if ( sposok != TRUE ) {
		return(RETURN_ERROR);
	}	

	/* boolean_end is also the start of the next field */
	boolean_end=cposition;

	if (text[cposition]==' ') {
		/* Skip over any spaces between left val and condition */
		while ( (cposition<string_length) && (text[cposition]==' ')) {
			cposition++;
		}
	}

	end_start=cposition;

	while ( (cposition<string_length) && (text[cposition]!=')')) {
		cposition++;
	}	

	endpos=cposition;

	/* Populate the string fields with all the details */

	/* Second bit is placing a NULL char at the end, to make
	 * sure it is properly terminated.
	 */

	strncpy(cond->left,&text[start],start_end-start);
	cond->left[start_end-start]='\0';
	strncpy(cond->bool,&text[boolean_start],boolean_end-boolean_start);
	cond->bool[boolean_end-boolean_start]='\0';
	strncpy(cond->right,&text[end_start],endpos-end_start);
	cond->right[endpos-end_start]='\0';

	do_debug( DEBUG_MODERATE,"Left: >%s<  Bool: >%s<  Right: >%s<\n",cond->left, cond->bool, cond->right);

	/* Check the first character of the left item, if it is
	 * a quote, then it is a value, otherwise it is a field
	 * that will need to be resolved each time
	 */
	if ( (lptr=strpbrk(cond->left,VALUES))!=NULL) {
		cond->left_field=FALSE;
		lptr++;
		strcpy(temp,lptr);
		strcpy(cond->left,temp);
		rptr=strpbrk(cond->left,VALUES);
		if (rptr!=NULL) {
			/* Make the quote the end of the string */
			*rptr='\0';
		} else {
			raise_error(ERROR_NO_CLOSING_QUOTE,NONFATAL,cond->left);
			errorflag=TRUE;
		}
	} else {
		/* Check that the left string is not TRUE or FALSE */
		if (strcmp(cond->left,TRUE_STRING)==0) {
			cond->left_always=ALWAYS_TRUE;
		} else if (strcmp(cond->left,FALSE_STRING)==0) {
			cond->left_always=ALWAYS_FALSE;
		} else {
			cond->left_field=TRUE;
		}
	}

	/* Check the first character of the right item, if it is
	 * a quote, then it is a value, otherwise it is a field
	 * that will need to be resolved each time
	 */
	if ( (lptr=strpbrk(cond->right,VALUES))!=NULL) {
		cond->right_field=FALSE;
		lptr++;
		strcpy(temp,lptr);
		strcpy(cond->right,temp);
		rptr=strpbrk(cond->right,VALUES);
		if (rptr!=NULL) {
			/* Make the quote the end of the string */
			*rptr='\0';
		} else {
			raise_error(ERROR_NO_CLOSING_QUOTE,NONFATAL,cond->right);
			errorflag=TRUE;
		}
	} else {
		/* Check that the right string is not TRUE or FALSE */
		if (strcmp(cond->right,TRUE_STRING)==0) {
			cond->right_always=ALWAYS_TRUE;
		} else if (strcmp(cond->right,FALSE_STRING)==0) {
			cond->right_always=ALWAYS_FALSE;
		} else {
			cond->right_field=TRUE;
		}
	}

	switch (cond->bool[0]) {

		case L_EQUAL : cond->boolval=BOOLEAN_EQUAL;
			  break;

        case L_LIKE : cond->boolval=BOOLEAN_LIKE;
            break;
            
		case L_LESSTHAN : switch (cond->bool[1]) {
				case L_END :  cond->boolval=BOOLEAN_LESS_THAN;
					break;

				case L_EQUAL: cond->boolval=BOOLEAN_LESS_EQUAL_THAN;
					 break;

				case L_GREATERTHAN: cond->boolval=BOOLEAN_NOT_EQUAL;
						break;

				default: raise_error(ERROR_UNEXPECTED_CONDITION_CHAR,NONFATAL,cond->bool);
					 errorflag=TRUE;
					 break;
			      }
			      break;

		case L_GREATERTHAN : switch (cond->bool[1]) {
					case L_END : cond->boolval=BOOLEAN_GREATER_THAN;
					       break;

					case L_EQUAL : cond->boolval=BOOLEAN_GREATER_EQUAL_THAN;
						  break;

					default: raise_error(ERROR_UNEXPECTED_CONDITION_CHAR,NONFATAL,cond->bool);
						 errorflag=TRUE;
						 break;
				}
			        break;


		default:
			raise_error(ERROR_UNEXPECTED_CONDITION_CHAR,NONFATAL,cond->bool);
			errorflag=TRUE;
			break;

	}

	if (errorflag==FALSE) {
		return(RETURN_SUCCESS);
	} else {
		return(RETURN_ERROR);
	}
}

void print_condition( condp condition ) {
/* Displays the condition structure 
 */
		condp currentc;

		currentc=condition;

		while (currentc!=NULL) {
			printf("( %s %s %s )\n",
				currentc->left, currentc->bool, currentc->right);
			switch (currentc->boolean_condition) {
				case BOOLEAN_END_MARKER : printf("<END>\n");
											break;
				case BOOLEAN_AND : printf("AND\n");
									break;
				case BOOLEAN_OR : printf("OR\n");
									break;
				case BOOLEAN_XOR : printf("XOR\n");
									break;
				default: printf(">UNKNOWN<");
						break;
			}

			currentc=currentc->next_condition;				
		}
}

int matchup( condp ccondition, int leftorright, tuple ltuple, tuple rtuple ) {
/* matchup
 * Performs the work of matching the condition to the appropriate
 * tuple, fetching ptrs to data, and so on.
 */
	boolean *lr_field;
	char *datumptr,*lr_condition,**lr_attribute_val;
	char object_name[ATTRIBUTE_NAME_SIZE+1],*sptr;
	attribute lr_attribute;
	int source;

	switch (leftorright) {
		case DIR_LEFT: lr_field=&ccondition->left_field;
				lr_condition=ccondition->left;
				lr_attribute_val=&ccondition->left_attribute_val;
				break;
		case DIR_RIGHT: lr_field=&ccondition->right_field;
				lr_condition=ccondition->right;
				lr_attribute_val=&ccondition->right_attribute_val;
				break;
		default:raise_error(ERROR_UNKNOWN,NONFATAL,"Something strange has happened (Tuple matchup, can't determine left/right)");
				return(RETURN_ERROR);
				/* Lint-ified */
				/* break; */
	}

	/* If the item is a field, then process it */
	if (*lr_field==TRUE) {
		/* Get the pointer to the data string */
		source=DIR_LEFT;	

		/* Check that the attribute specified does not contain a 
		 * relation name.
	     */
		sptr=strchr(lr_condition,'.');
		if (sptr!=NULL) {
			/* Get the relation and attribute seperate */
			strcpy(object_name,lr_condition);

			sptr=strchr(object_name,'.');
			*sptr='\0';	
			sptr++;
			/* Now object_name contains the relation name, and
		     * sptr point to the attribute name 
			 */

			/* Check that the object is actually a relation */
			if (strcmp(object_name,relation_name(get_relation(ltuple)))==0) {
				datumptr=tuple_find_attribute_val(ltuple,sptr);
				if (datumptr==NULL) {
					raise_error(ERROR_CANTFIND_ATTR,NONFATAL,sptr);
					return(RETURN_ERROR);
				}
				*lr_attribute_val=datumptr;
				lr_attribute=tuple_find_attribute(ltuple,sptr);
			} else
			if (strcmp(object_name,relation_name(get_relation(rtuple)))==0) {
				datumptr=tuple_find_attribute_val(rtuple,sptr);
				if (datumptr==NULL) {
					raise_error(ERROR_CANTFIND_ATTR,NONFATAL,sptr);
					return(RETURN_ERROR);
				}
				*lr_attribute_val=datumptr;
				lr_attribute=tuple_find_attribute(rtuple,sptr);
			} else {
				/* Not a valid object! Abort with an error */
				raise_error(ERROR_CANNOT_FIND_REL,NONFATAL,object_name);
				return(RETURN_ERROR);
			}	

		} else {
			datumptr=tuple_find_attribute_val(ltuple,lr_condition);

			/* If it was not in the "left" source tuple, and there is a 
			 * "right" source tuple, then search that one.
			 */
			if ( (rtuple!=NULL) && (datumptr==NULL) ) {
				source=DIR_RIGHT;
				datumptr=tuple_find_attribute_val(rtuple,lr_condition);
			}
	
			/* If there is no such thing, then raise an error of some sort */ 
			if (datumptr==NULL) {
				/* Report an error */
				raise_error(ERROR_CANTFIND_ATTR,NONFATAL,lr_condition);
				return(RETURN_ERROR);
			} else {
	
				/* Set values to point to the located data */
				*lr_attribute_val=datumptr;
				switch (source) {
					case DIR_LEFT:*lr_attribute_val=datumptr;
						      lr_attribute=tuple_find_attribute(ltuple,lr_condition);
							break;
					case DIR_RIGHT:*lr_attribute_val=datumptr;
						      lr_attribute=tuple_find_attribute(rtuple,lr_condition);
							break;
					default:raise_error(ERROR_CANTFIND_ATTR,NONFATAL,lr_condition);
						    return(RETURN_ERROR);
							/* Lintified */
							/* break; */
				}
			}
		}

		/* Do an assignment that is easier this way */
		switch (leftorright) {
			case DIR_LEFT: ccondition->left_attribute=lr_attribute;
					break;
			case DIR_RIGHT: ccondition->right_attribute=lr_attribute;
					break;
			default:raise_error(ERROR_UNKNOWN,NONFATAL,"Something strange has happened (Tuple matchup, can't determine 2nd 2nd left/right)");
					return(RETURN_ERROR);
					/* Lintified */
					/* break; */
		}
	}
	return(RETURN_SUCCESS);
}

int match_tuples( condp condition, tuple left_tuple, tuple right_tuple) {
/* Match up condition fields to tuples, so references can quickly
 * be made when evaluating if a condition is TRUE or FALSE
 */
	condp ccondition;
	boolean errorflag=FALSE;

	/* Start at the top of the condition list */
	ccondition=condition;

	/* While the condition has nodes to be processed */
	while ( (errorflag==FALSE) && (ccondition!=NULL) ) {

		errorflag=(matchup(ccondition, DIR_LEFT , left_tuple, right_tuple)!=RETURN_SUCCESS); 
		if (errorflag!=TRUE) {
			errorflag=(matchup(ccondition, DIR_RIGHT, left_tuple, right_tuple)!=RETURN_SUCCESS);
		}

		/* While the condition list has nodes to be processed! */
		ccondition=ccondition->next_condition;
	}

	if (errorflag==FALSE) {
		return(RETURN_SUCCESS);
	} else {
		return(RETURN_ERROR);
	}
}

int destroy_condition( condp *condition ) {
/* Dispose of a condition structure. Returns SUCCESS. Could this possibly
 * fail? 
 */
	condp previousc,currentc;

	/* Get the condition into a local ptr */
	currentc=*condition;

	/* Whilst it is valid */
	while (currentc!=NULL) {
	
		/* Put it into a backup */
		previousc=currentc;

		/* Move to the next condition */
		currentc=currentc->next_condition;

		/* Dispose of the old condition */
		free(previousc);
	}

	/* Dispose of the holder memory 
	if ((condition!=NULL) && (*condition!=NULL)) {
		free(*condition);
	}
 	*/
	*condition=NULL;

	return(RETURN_SUCCESS);
}

condp create_condition() {
	condp ccondition;
	ccondition=(ncond_struct *) malloc (sizeof(ncond_struct));

	strcpy(ccondition->left,"");
	ccondition->left_field=FALSE;
	ccondition->left_attribute_val=NULL;
	ccondition->left_attribute=NULL;
	ccondition->left_always=0;
	strcpy(ccondition->bool,"");
	ccondition->boolval=0;
	strcpy(ccondition->right,"");	
	ccondition->right_field=FALSE;
	ccondition->right_attribute_val=NULL;
	ccondition->right_attribute=NULL;
	ccondition->right_always=0;
	ccondition->boolean_condition=0;
	ccondition->next_condition=NULL;
	ccondition->eval_result=FALSE;

	return(ccondition);	
}

condp build_condition( char *qualification, tuple left_tuple, tuple right_tuple) {
/* Takes a qualification, and builds a linked list structure with
 * the qualification built.
 * Return NULL If fails, ptr to first condition if success
 */

	int counter,result,matchupres=0;
	char booleanexp[MAXIMUM_EXPRESSION+1],currentexp[MAXIMUM_EXPRESSION+1],*stringptr;
	condp pcondition,ccondition,rcondition;
	boolean errorflag=FALSE;

	/* Initialise ptrs to conditions */
	pcondition=NULL;
	ccondition=create_condition();

	/* Set return condition, ie. first item on list */
	rcondition=ccondition;

	/* Cut out the first set of brackets */
	(void) cut_to_right_bracket(qualification,1,TRUE,currentexp);

	/* Reset the counter */
	counter=0;

	/* Process each of the conditions */
	while ( (errorflag!=TRUE) && (strlen(currentexp)!=0) ) {

		/* Load the expressions */
		result=get_first_expression(currentexp, ccondition);

		/* If the function was NOT successful... */
		if (result!=RETURN_SUCCESS) {

			/* Release memory, and set an error flag */
			if (ccondition!=NULL) free(ccondition);
			ccondition=NULL;

			errorflag=TRUE;

		} else {
			/* If an expression was returned */

			/* Is there a previous condition? */
			if (pcondition!=NULL) {
				/* Link the previous condition to the new condition */
				pcondition->next_condition=ccondition;

			}

			/* Move the previous condition to the current condition */
			pcondition=ccondition;

			/* Create a new condition node */
			ccondition=create_condition();

			/* Mark it as if it were the last. */
			ccondition->boolean_condition=BOOLEAN_END_MARKER;
		}

		/* Increment the counter */
		counter++;

		/* Locate the first non-space */
		stringptr=find_start_of_data(qualification);

		/* If something was found */
		if (stringptr!=NULL) {
			qualification=stringptr;

			/* Get the token, ie. the linking "and, or, xor"... */
			cut_token(qualification,'\0',booleanexp);

#ifdef DEBUG
	fprintf(stderr,"Boolean seperator is: >%s<\n",booleanexp);
#endif

			/* Determine what sort of condition it is... */

			/* TODO: Is there maybe a better way of doing this? */
			/* Worst case it involves searching the string 3 times */
			if (strcmp(booleanexp,BOOLEAN_AND_STRING)==0) {
				pcondition->boolean_condition=BOOLEAN_AND;
			} else if (strcmp(booleanexp,BOOLEAN_OR_STRING)==0) {
				pcondition->boolean_condition=BOOLEAN_OR;
			} else if (strcmp(booleanexp,BOOLEAN_XOR_STRING)==0) {
				pcondition->boolean_condition=BOOLEAN_XOR;
			} else {
				pcondition->boolean_condition=BOOLEAN_UNDETERMINED;
				raise_error(ERROR_UNSUPPORTED_BOOLEAN_OPERATOR,NONFATAL,booleanexp);
				errorflag=TRUE;
			}
			
			/* Cut out the next set of brackets */
			(void) cut_to_right_bracket(qualification,1,TRUE,currentexp);

		} else {
			/* Nothing was found in the string. So force an end. */

			/* Make the currentexp zero length, to end the loop */
			strcpy(currentexp,"");
		}

	}
	/* At this point, the conditions should be loaded,
	 * or an error occured. */

	if (pcondition!=NULL) {
		/* Make sure the previous condition is tidied up */
		pcondition->next_condition=NULL;
		pcondition->boolean_condition=BOOLEAN_END_MARKER;
	}

	/* Free up the tailing condition */
	if (ccondition!=NULL) free(ccondition);
	ccondition=NULL;

	if (errorflag==FALSE) {

		/* Match the condition list up against the source
		 * tuple, so attribute names are paired with
		 * ptrs to the attribute in the tuple, to speed
		 * up evaluation 
	     */
		matchupres=match_tuples(rcondition,left_tuple,right_tuple);

		/* If matching was successful... */
		if (matchupres==RETURN_SUCCESS) {
			/* Return the complete condition */
			return(rcondition);

		} else {
			/* If it wasn't - destory the condition structure, 
			 * and return NULL
			 */
			(void) destroy_condition(&rcondition);
			return(NULL);
		}			
	} else {
		/* An error occured... */
		/* TODO - Free up condition list... */
		return(NULL);
	}
}

