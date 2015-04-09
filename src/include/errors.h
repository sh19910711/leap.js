/* 
 * Name: errors.h
 * Description: Errors header file
 * Version: errors.h,v 1.207.2.1 2004/02/09 20:05:20 rleyton Exp
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

#ifndef _LEAP_ERRORS_
#define _LEAP_ERRORS_

#include <stdarg.h>

extern struct errors_struct {
	int errnumber;
	char *errorfmt;
	int severity;
} errors[];

#define NERRORS (sizeof errors/sizeof errors[0])

/* Adapted from 'Advanced Programming in the Unix Environment',
 * by W.Richard Stephens.
 * Define a type which 
 */
typedef void Func(int, int, char *);

extern void do_error(short int error_num, char *string, int fatality);
extern void raise_error(short int error_num, int fatality, char *fmt, ...);
extern void raise_message(short int type, char *fmt,...);
extern void default_handler(int errornumber, int errorfatality, char *errorstring);
extern void default_quiethandler(int errornumber, int errorfatality, char *errorstring);
extern int define_handle(Func *ehandle,Func **handle);

extern Func *errorHandler;
extern Func *eventHandler;
extern Func *messageHandler;


/* Define the severity of the error raised */
#define NONFATAL 0
#define UNKNOWN_FATALITY 1
#define FATAL 2

/* Define the type of message raised */
#define EVENT 0
#define MESSAGE 1

#define raise_event raise_message

/* 
 * Internal Error numbers. 
 */
#define ERROR_OK 0
#define ERROR_FILE_NOT_FOUND 1
#define ERROR_FILE_OPENING 2
#define ERROR_UNKNOWN 3
#define ERROR_INSUFFICENT_MEMORY 4
#define ERROR_CANTFIND_ATTR 5
#define ERROR_DUPLICATE_ITEM 6
#define ERROR_UNION_COMPATIBILITY 7
#define ERROR_ERASE_FILE 8
#define ERROR_CANNOT_FIND_REL 9
#define ERROR_DELETE_NONEX_REL 10
#define ERROR_EXCEEDED_ATTRIBUTE_LIMIT 11
#define ERROR_MISMATCHING_BRACKETS 12
#define ERROR_NO_RELATION_CREATED 13
#define ERROR_INTERNAL_STACK 14
#define ERROR_EVALUATING_EXPR 15
#define ERROR_INVALID_RELNAME 16 
#define ERROR_UNKNOWN_DATABASE 17
#define ERROR_ALREADY_OPEN 18
#define ERROR_TYPE_INCOMPATIBLE 19
#define ERROR_TYPE_CONVERSION 20
#define ERROR_UNIMPLEMENTED 21
#define ERROR_COMMAND_LINE 22
#define ERROR_CLOSING_FILE 23
#define ERROR_UNSUPPORTED_DTYPE 24
#define ERROR_TIME 25
#define ERROR_OBSOLETE 26
#define ERROR_HASH_FILE_CORRUPT 27
#define ERROR_PARSE_UNRECOGNISED_TOKEN 28
#define ERROR_PARSE_EXECUTION 29
#define ERROR_UNEXPECTED_CONDITION_CHAR 30
#define ERROR_UNSUPPORTED_BOOLEAN_OPERATOR 31
#define ERROR_NO_CLOSING_QUOTE 32
#define ERROR_NO_CONDITION 33
#define ERROR_DATA_DICTIONARY 34
#define ERROR_CONFIGURATION 35
#define ERROR_SETTING 36
#define ERROR_DATATYPE_SIZE 37
#define ERROR_CONFIG_FILE 38
#define ERROR_VARIABLES 39
#define ERROR_UNDEFINED_VARIABLE 40
#define ERROR_DATABASE_FORMAT 41
#define ERROR_OPEN_DATABASE 42
#define ERROR_OBSOLETED_CODE 43
#define ERROR_CREATE_DIR 44
#define ERROR_NODDUPDATES 45
#define ERROR_INSTALL 46
#define ERROR_DISABLED 47
#define ERROR_DELREL 48
#define ERROR_DELFILE 49
#define ERROR_TAMPERSYSTEM 50
#define ERROR_EVENT 51
#define ERROR_MESSAGE 52
#define ERROR_ATTRIBUTE_SIZE_LARGE 53
#define ERROR_OSOPERATION 54
#define ERROR_SOCKETINIT 55
#define ERROR_REGEXP 56
#define ERROR_DISABLED_CODE 57
#define ERROR_ATTRIBUTE 58
#define ERROR_TUPLE_ERASE 59
#define ERROR_RELATION_COMPACT 60
#define ERROR_UNDEFINED 999


#endif
