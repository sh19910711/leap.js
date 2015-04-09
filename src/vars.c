/* 
 * Name: vars.c,v
 * Description: Variable resolution/maintenance.
 * Version: vars.c,v 1.207.2.1 2004/02/09 20:05:20 rleyton Exp
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

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "vars.h"
#include "dtypes.h"
#include "consts.h"
#include "globals.h"
#include "errors.h"
#include "util.h"
#include "leapio.h"

/* The variables */
static var_struct variables[VARIABLE_NUMBER];
static word new_position;

char *resolve_variable(char *variable) {
/* resolve_variable
 * Resolves specified variable
 */
	int position=0;
	boolean found=FALSE;

	while ( (position<new_position) && (found==FALSE) ) {	
		found=(strcmp(variables[position].name,variable)==0);
		if (found!=TRUE) position++;
	}
	
	if (found==TRUE) {
		return(variables[position].value);
	} else {
		return(NULL);
	}
}

char *pull_variable(int var_number) {
/* pull_variable
 * Pulls out a specific variable number
 */
	if (var_number<new_position) {
		return(variables[var_number].value);
	} else {
		return(NULL);
	}
}

char *set_variable(char *variable, char *value) {
/* set_variable
 * Specifies the given variable
 */
	int position=0;
	boolean found=FALSE,enable,optioninteger=FALSE,*option=NULL;

	while ( (position<new_position) && (found==FALSE) ) {	
		position++;
		found=(strcmp(variables[position].name,variable)==0);
	}

	enable=(strcmp(value,STATUS_SETTING_ON)==0);
	
	if (strcmp(variable,STATUS_TRACE)==0) {
		option=&status_trace;
	} else if (strcmp(variable,STATUS_DEBUG)==0) {
		option=&status_debug;
	} else if (strcmp(variable,STATUS_TIMING)==0) {
		option=&status_timing;
	} else if (strcmp(variable,STATUS_CASE)==0) {
		option=&status_case;
	} else if (strcmp(variable,STATUS_QUIET)==0) {
		option=&status_quiet;
	} else if (strcmp(variable,STATUS_TEMPORARY_RELATIONS)==0) {
		option=&status_temporary_relations;
	} else if (strcmp(variable,STATUS_TIMELOG)==0) {
		option=&status_timelog;
	} else if (strcmp(variable,STATUS_PADDING)==0) {
		option=&status_padding;
	} else if (strcmp(variable,STATUS_LONGLINE)==0) {
		option=&status_longline;
	} else if (strcmp(variable,STATUS_TEMPDB)==0) {
		option=&status_tempdb;
	} else if (strcmp(variable,STATUS_PRODUCTJOIN)==0) {
		option=&status_productjoin;
	} else if (strcmp(variable,STATUS_DAEMON)==0) {
		option=&status_daemon;
	} else if (strcmp(variable,STATUS_DEBUG_LEVEL)==0) {
		optioninteger=TRUE;
	} else if (strcmp(variable,STATUS_MINDEBUG_LEVEL)==0) {
		optioninteger=TRUE;
	} else if (strcmp(variable,STATUS_MERGE_STDERR)==0) {
		option=&status_merge_stderr;
	} else if (strcmp(variable,STATUS_REGRESSION)==0) {
		/* This is to prevent any tampering. */
		raise_error(ERROR_CONFIGURATION,NONFATAL,"Cannot change regression status within program");
		return(RETURN_ERROR);	
	} else {
		/* This is a different variable, or user defined var */
	    option=NULL;
	}

	/* Check if this is a system variable, and if so, set it */
	if ((option!=NULL) && (optioninteger!=TRUE)) {
		*option=enable;
	}
	
	if (found==TRUE) {
		strcpy(variables[position].value,value);
		if (strcmp(variables[position].name,STATUS_DEBUG_LEVEL)==0) {
#ifdef DEBUG_CODE
			status_debuglevel=atoi(variables[position].value);

			if (status_debuglevel>MAX_DEBUG_LEVEL) {
				status_debuglevel=MAX_DEBUG_LEVEL;
				raise_error(ERROR_SETTING,NONFATAL,"Debug level exceeds maximum of %d.\n",MAX_DEBUG_LEVEL);
			}
			else
			if (status_debuglevel<MIN_DEBUG_LEVEL) {
				status_debuglevel=MIN_DEBUG_LEVEL;
				raise_error(ERROR_SETTING,NONFATAL,"Debug level falls below minimum of %d.\n",MIN_DEBUG_LEVEL);
			}

			sprintf(variables[position].value,"%d",status_debuglevel);
			raise_message(MESSAGE,"Debug level set to: %d",status_debuglevel);
#else
			raise_message(MESSAGE,"Debug level cannot be set in non-debug version of LEAP.");
#endif
		} else if (strcmp(variables[position].name,STATUS_MINDEBUG_LEVEL)==0) { 
#ifdef DEBUG_CODE
            status_mindebuglevel=atoi(variables[position].value);

            if (status_mindebuglevel>MAX_DEBUG_LEVEL) {
                status_mindebuglevel=MAX_DEBUG_LEVEL;
                raise_error(ERROR_SETTING,NONFATAL,"mindebug level exceeds maximum of %d.\n",MAX_DEBUG_LEVEL);
            }
            else
            if (status_mindebuglevel<MIN_DEBUG_LEVEL) {
                status_mindebuglevel=MIN_DEBUG_LEVEL;
                raise_error(ERROR_SETTING,NONFATAL,"mindebug level falls below minimum of %d.\n",MIN_DEBUG_LEVEL);
            }
    
            sprintf(variables[position].value,"%d",status_mindebuglevel);
            raise_message(MESSAGE,"Minimum debug level set to: %d",status_mindebuglevel);
#else
            raise_message(MESSAGE,"mindebug level cannot be set in non-debug version of LEAP.");
#endif
		} else {
			strcpy(variables[position].value,value);
		}
	
		return(variables[position].value);
	} else {
		return(NULL);
	}
}

void show_variables() {
/* show_variables
 * Shows settings of all variables
 */
	int position;

	for (position=0; position<new_position; position++) {
		leap_printf("%-25s:%s\n",variables[position].name,variables[position].value);
	}

} 

int new_variable(char *var_name, char *var_value) {
/* new_variable
 * Creates a new variable
 */
	char temp[100];

	if (new_position<VARIABLE_NUMBER) {
		sprintf(temp,"Setting: %s to %s\n",var_name,var_value);
		do_trace(temp);
		strcpy(variables[new_position].name,var_name);
		strcpy(variables[new_position].value,var_value);
		variables[new_position].set=TRUE;
		new_position++;
		return(RETURN_SUCCESS);
	} else {
		sprintf(temp,"No more space for variable \"%s\".\n",var_name);
		raise_error(ERROR_VARIABLES,NONFATAL,temp);
		return(RETURN_ERROR);
	}
}

int init_variables() {
/* init_variables
 * Initialises internal structures
 */
	int count,lineno=0;
	FILE *variable_file;
	char temp[FILENAME_MAX+1],*result,config_line[MAXIMUM_EXPRESSION+1];
					/* +2 - 1 for seperator, 1 for null */
	char var_name[VARIABLE_NAME_SIZE+1];

	for (count=0; count<VARIABLE_NUMBER; count++) {
		variables[count].set=FALSE;
		strcpy(variables[count].name,"");
		strcpy(variables[count].value,"");
	}

	new_position=0;

	sprintf(temp,"%s%s%s",LEAP_BASE_DIR,LEAP_CONFIG_DIR,LEAP_VARIABLE_FILE);
	variable_file=fopen(temp,"r");
	if (variable_file==NULL) {
		raise_error(ERROR_FILE_OPENING,NONFATAL,temp);
		raise_message(MESSAGE,"No variables set!");
		return(RETURN_ERROR);
	} else {
		result=fgets(config_line,sizeof(config_line),variable_file);
		while (result!=NULL) {
			/* Keep track of the line, for error reporting purposes */
			lineno++;

			/* Begone, evil spaces! */
			strip_leading_spaces(config_line);

			/* Check that the line isn't a comment... */
			if (config_line[0]!=COMMENT_CHAR) {

				/* By 'eck, there might be more! */
				strip_trailing_spaces(config_line);

				/* Get it into vars... */
				cut_token(config_line,'\0',var_name);
				new_variable(var_name,config_line);
			
				/* This *second* call ensures that system
				 * booleans are set...
				 */
				set_variable(var_name,config_line);
			}
			/* Get the next line */
			result=fgets(config_line,sizeof(config_line),variable_file);
		}
		fclose(variable_file);
		return(RETURN_SUCCESS);
	}
}
