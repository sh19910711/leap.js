/* 
 * Name: globals.h
 * Description: LEAP Global variables
 * Version: global_vars.h,v 1.4.2.1 2004/02/09 20:05:21 rleyton Exp
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


/* Prevent multiple inclusion */
#ifndef _LEAP_GLOBALS_
#define _LEAP_GLOBALS_

#include "consts.h"
#include "dtypes.h"

/* The base directory for LEAP - Set as a parameter on
   calling the main program. */
extern char LEAP_BASE_DIR[FILE_PATH_SIZE+1];

/* This is just a holdall for impromptu sprintf calls. */
extern char temp_80_chars[80];

extern struct commands_struct {
	char *text;
	int command;
} commands[] ;


/* Define the current and master database 
 * These will be defined in the main program 
 */
extern database current_db,master_db,tempdb;

/* Define the termination flag - When this is
 * TRUE, termination has been requested.
 */
extern boolean terminate;
extern boolean terminatenow;

/* The input stream. This is normally assigned to
 * standard input, but then the user might want to 
 * source a file 
 */
extern FILE *input_stream;

/* Character for beeps - Either BELL or SPACE */
extern char BEEP;

/* Define special conditions */
extern char special_condition_true[];
extern char special_condition_false[];

/* Define status flags */
extern boolean status_trace;
extern boolean status_debug;
extern boolean status_timing;
extern boolean status_case;
extern boolean status_quiet;
extern boolean status_regression;
extern boolean status_temporary_relations;
extern boolean status_timelog;
extern boolean status_longline;
extern boolean status_padding;
extern boolean status_tempdb;
extern boolean status_productjoin;
extern boolean status_daemon;

extern int status_debuglevel;
extern int status_mindebuglevel;

/* Defined in util.c */
extern char tempdir[FILE_PATH_SIZE+1];
extern char scratchfile[FILE_PATH_SIZE+1];

/* Defined in leap.c */
extern char current_prompt[MAXIMUM_PROMPT+1];

/* Count info. */
/* Defined in tuples.c */
extern unsigned int no_written;
extern unsigned int no_read_physical;
extern unsigned int no_read_logical;

/* Version */
extern int leapver;

/* No data dictionary update */
extern boolean noddupdate;

/* Configuration mode */
extern boolean configuration;
extern boolean opening;

/* Recording mode */
extern boolean recording;

extern FILE *recordingfptr;

extern int slaveSocket;

#endif
/* End of ifndef */
