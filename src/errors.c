/*
 * Name: error.c
 * Description: Handle errors.
 * Version: errors.c,v 1.208.2.1 2004/02/09 20:05:20 rleyton Exp
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

#if defined(HAVE_STDARG_H)
# include <stdarg.h>
#else
# include <varargs.h>
#endif
#include <stdio.h>
#include "errors.h"
#include "consts.h"
#include "globals.h"
#include "leapio.h"
#include "util.h"

/* Define the error structure */
struct errors_struct errors[] = {
	{ 0, "Command completed successfully.",0 },
	{ ERROR_FILE_NOT_FOUND, "File [%s] not found.\n",UNKNOWN_FATALITY},
	{ ERROR_FILE_OPENING, "Error opening file [%s].\n",UNKNOWN_FATALITY},
	{ ERROR_UNKNOWN, "Unknown error occured.\n%s\n",UNKNOWN_FATALITY},
	{ ERROR_INSUFFICENT_MEMORY, "Insufficent memory for operations.\n", UNKNOWN_FATALITY},
	{ ERROR_CANTFIND_ATTR, "Specified Attribute [%s] not found.\n",UNKNOWN_FATALITY},
	{ ERROR_DUPLICATE_ITEM, "Duplicate item [%s] exists.\n",UNKNOWN_FATALITY},
	{ ERROR_UNION_COMPATIBILITY, "Union compatibility does not hold.\n",UNKNOWN_FATALITY},
	{ ERROR_ERASE_FILE, "Unable to erase file [%s].\n",UNKNOWN_FATALITY},
	{ ERROR_CANNOT_FIND_REL, "Cannot find relation [%s].\n",UNKNOWN_FATALITY},
	{ ERROR_DELETE_NONEX_REL, "Attempted to delete a non-existant relation [%s].\n",UNKNOWN_FATALITY},
	{ ERROR_EXCEEDED_ATTRIBUTE_LIMIT, "Exceeded the maximum number of attributes in a relation (%d).\n",UNKNOWN_FATALITY},
	{ ERROR_MISMATCHING_BRACKETS, "Brackets in the expression do not match.\n",UNKNOWN_FATALITY},
	{ ERROR_NO_RELATION_CREATED, "No relation created where required (Check brackets!).\n",UNKNOWN_FATALITY},
	{ ERROR_INTERNAL_STACK, "Internal stack processing error.\n",UNKNOWN_FATALITY},
	{ ERROR_EVALUATING_EXPR, "Error evaluating expression.\n",UNKNOWN_FATALITY},
	{ ERROR_INVALID_RELNAME, "Invalid character in relation name.\n",UNKNOWN_FATALITY},
	{ ERROR_UNKNOWN_DATABASE, "Unknown database specified [%s].\n",UNKNOWN_FATALITY},
	{ ERROR_ALREADY_OPEN, "Attempt to open the currently open database [%s].\n",UNKNOWN_FATALITY},
	{ ERROR_TYPE_INCOMPATIBLE, "Data types are incompatible.\n",UNKNOWN_FATALITY},
	{ ERROR_TYPE_CONVERSION, "Type conversion error.\n",UNKNOWN_FATALITY},
	{ ERROR_UNIMPLEMENTED, "Operator (%s) not implemented.\n",UNKNOWN_FATALITY},
	{ ERROR_COMMAND_LINE, "Invalid command line parameter [%s].\n",UNKNOWN_FATALITY},
	{ ERROR_CLOSING_FILE, "Error closing file.\n",UNKNOWN_FATALITY},
	{ ERROR_UNSUPPORTED_DTYPE, "Unsupported data type encountered.\n",UNKNOWN_FATALITY},
	{ ERROR_TIME, "Error determining the time.\n",UNKNOWN_FATALITY},
	{ ERROR_OBSOLETE, "Obsoleted operation (%s).\n",UNKNOWN_FATALITY},
	{ ERROR_HASH_FILE_CORRUPT, "Hash file appears corrupt.\n",UNKNOWN_FATALITY},
	{ ERROR_PARSE_UNRECOGNISED_TOKEN, "Unrecognised token(s) in expression.\n:%s\n",UNKNOWN_FATALITY},
	{ ERROR_PARSE_EXECUTION, "Error occured during expression execution.\n:%s\n",UNKNOWN_FATALITY},
	{ ERROR_UNEXPECTED_CONDITION_CHAR, "Condition contains unexpected character.\n",UNKNOWN_FATALITY},
	{ ERROR_UNSUPPORTED_BOOLEAN_OPERATOR, "Condition contains unsupported boolean operator.\n",UNKNOWN_FATALITY},
	{ ERROR_NO_CLOSING_QUOTE, "Condition contains unterminated quote.\n",UNKNOWN_FATALITY},
	{ ERROR_NO_CONDITION, "Condition not specified.\n",UNKNOWN_FATALITY},
	{ ERROR_DATA_DICTIONARY, "Error (%s) occured during data dictionary lookup.\n",UNKNOWN_FATALITY},
	{ ERROR_CONFIGURATION, "Invalid configuration.\n",UNKNOWN_FATALITY},
	{ ERROR_SETTING, "Invalid setting.\n",UNKNOWN_FATALITY},
	{ ERROR_DATATYPE_SIZE, "Length of data type invalid.\n",UNKNOWN_FATALITY},
	{ ERROR_CONFIG_FILE, "Error in configuration file.\n",UNKNOWN_FATALITY},
	{ ERROR_VARIABLES, "Error with variables.\n",UNKNOWN_FATALITY},
	{ ERROR_UNDEFINED_VARIABLE, "Undefined variable name (%s).\n",UNKNOWN_FATALITY},
	{ ERROR_DATABASE_FORMAT, "Database is in the incorrect format. Has convert been run?\n",UNKNOWN_FATALITY},
	{ ERROR_OPEN_DATABASE,"Error opening database [%s]\n",FATAL},
	{ ERROR_OBSOLETED_CODE,"Obsoleted code called!\n",NONFATAL},
	{ ERROR_CREATE_DIR,"Error creating directory [%s]!\n",NONFATAL},
	{ ERROR_NODDUPDATES,"No updates to the data dictionary are being made!\n",NONFATAL},
	{ ERROR_INSTALL,"Configuration/Setup error.\n",FATAL},
	{ ERROR_DISABLED,"Disabled operator.\n",NONFATAL},
	{ ERROR_DELREL,"Error deleting relation [%s]\n",NONFATAL},
	{ ERROR_DELFILE,"Error deleting file [%s]\n",NONFATAL},
	{ ERROR_TAMPERSYSTEM,"Cannot tamper with system relations [%s].\n",NONFATAL},
	{ ERROR_EVENT,"Event: %s\n",NONFATAL},
	{ ERROR_MESSAGE,"Message: %s\n",NONFATAL},
	{ ERROR_OSOPERATION,"Error performing an OS operation [%s].\n",NONFATAL},
	{ ERROR_SOCKETINIT,"Error initialising socket [%s].\n",NONFATAL},
	{ ERROR_ATTRIBUTE_SIZE_LARGE,"Attribute size is larger than maximum permitted.\n:%s",NONFATAL},
	{ ERROR_REGEXP,"Regular Expression error: %s\n",NONFATAL},
	{ ERROR_DISABLED_CODE,"Disabled code called! [%s]\n",NONFATAL},
	{ ERROR_ATTRIBUTE,"Error with attribute(s) [%s]\n",NONFATAL},
	{ ERROR_TUPLE_ERASE,"Error erasing tuple. [%s]\n",NONFATAL},
	{ ERROR_RELATION_COMPACT,"Error compacting relation. [%s]\n",NONFATAL},
	/**********************************************************
	 * ERROR_UNDEFINED should be last... It catches incorrect
	 * or unknown errors
	 **********************************************************/
	{ ERROR_UNDEFINED,"Undefined error: %s\n",UNKNOWN_FATALITY}
};

/* Definition of variables */
Func *errorHandler=NULL;
Func *eventHandler=NULL;
Func *messageHandler=NULL;

/* Definition of error handler */
int define_handle(Func *ehandle,Func **handle) {

	if (ehandle!=NULL) {
		*handle=ehandle;
		return(RETURN_SUCCESS);
	} else {
		*handle=NULL;
		return(RETURN_ERROR);
	}
		
}

void default_quiethandler(int errornumber, int errorfatality, char *errorstring) {
		char temp[10];

		/* Switch on error # severity */
		switch (errorfatality) {

			/* Non-fatal errors */
			case NONFATAL: break;
							
			/* Old errors all depend on the parameter... switch on that */
			case UNKNOWN_FATALITY:
						if (errorfatality==NONFATAL) {
							strcpy(temp,"NON-FATAL");
						} else {
							strcpy(temp,"FATAL");
						}
						break;
				
			/* This is always a fatal error. Abort. */
			default: strcpy(temp,"FATAL");
					 break;
		}
		

		if (errorfatality==NONFATAL) {
			/* Print the error message */
			leap_fprintf(stderr,"%s",errorstring);
		} else {
			/* Print the error message */
			leap_fprintf(stderr,"[%s] #%d - %s",temp,errornumber,errorstring);
		}
		
		/* Abort! */
		if ( (errors[errornumber].severity==FATAL) || (errornumber==FATAL) ) {
			leap_fprintf(stderr,"LEAP terminated abnormally.\n");
			util_close();
			exit(errornumber);
		}
}

void default_handler(int errornumber, int errorfatality, char *errorstring) {
		char temp[10];

		/* Switch on error # severity */
		switch (errorfatality) {

			/* Non-fatal errors */
			case NONFATAL: strcpy(temp,"NON-FATAL");
						   break;
							
			/* Old errors all depend on the parameter... switch on that */
			case UNKNOWN_FATALITY:
						if (errorfatality==NONFATAL) {
							strcpy(temp,"NON-FATAL");
						} else {
							strcpy(temp,"FATAL");
						}
						break;
				
			/* This is always a fatal error. Abort. */
			default: strcpy(temp,"FATAL");
					 break;
		}
		
		/* Print the error message */
		leap_fprintf(stderr,"[%s] #%d - %s",temp,errornumber,errorstring);
		
		/* Abort! */
		if ( ((errors[errornumber].severity==FATAL) || (errornumber==FATAL))
			&&(errorfatality!=NONFATAL) ) {
			leap_fprintf(stderr,"LEAP terminated abnormally.\n");
			util_close();
			exit(errornumber);
		}
}

void raise_error(short int error_num, int fatality, char *fmt,...) {
/* raise_error
 * Raise an error message 
 */
	va_list ap;
	char expression[MAXIMUM_EXPRESSION+1],preexpression[MAXIMUM_EXPRESSION+1];
	
	va_start(ap, fmt);

	/* Check the error is sane! */
	if (error_num<NERRORS) {

		vsprintf(preexpression, fmt, ap);

		sprintf(expression, errors[error_num].errorfmt,preexpression); 
		
		if (*errorHandler!=NULL) {
			errorHandler(error_num,fatality,expression);
		} else {
			define_handle(&default_handler,&errorHandler);
			errorHandler(error_num,fatality,expression);
		}
	}
}

void raise_message(short int type,char *fmt,...) {
/* raise_error
 * Raise an informational message 
 */
	va_list ap;
	char expression[MAXIMUM_EXPRESSION+1],preexpression[MAXIMUM_EXPRESSION+1];
	
	va_start(ap, fmt);

	vsprintf(preexpression, fmt, ap);
	if (type==EVENT) {
		sprintf(expression, errors[ERROR_EVENT].errorfmt,preexpression);
		
		if (*eventHandler!=NULL) {
			eventHandler(ERROR_EVENT,NONFATAL,expression);
		} 
	} else {
		sprintf(expression, errors[ERROR_MESSAGE].errorfmt,preexpression);
		
		if (*messageHandler!=NULL) {
			messageHandler(ERROR_MESSAGE,NONFATAL,expression);
		} 
	}
}

