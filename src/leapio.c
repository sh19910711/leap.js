/*
 * Name: leapio.c
 * Description: LEAP IO operations.
 * Version: leapio.c,v 1.208.2.1 2004/02/09 20:05:20 rleyton Exp
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
#include "util.h"
#include "vars.h"
#include "globals.h"

/* By default the file is null, so we don't start
 * printing to it.
 */
FILE *ACTIVITY_FILE=NULL;
char socketBuffer[MAXIMUM_INPUT_STRING+1];

boolean level_indicator=TRUE;

#ifdef DEBUG_CODE
void levindicator( boolean levflag ) {
/* toggle the debug level indicator
 * used in high level debugging
 */

	level_indicator=levflag;
	
}

void do_debug( int debuglevel, char *fmt, ...) {
/* leap_debug
 * debug message - printed only if debug level is at or above parameter
 */
	va_list ap;
	

	if ((status_debuglevel>=debuglevel)&&(status_mindebuglevel<=debuglevel)) {
		va_start(ap,fmt);
		if (level_indicator) {
			if (status_merge_stderr==TRUE) {
				fprintf(stdout,"   D%d: ",debuglevel);
			} else {
				fprintf(stderr,"   D%d: ",debuglevel);
			}
			if (ACTIVITY_FILE!=NULL) {
				fprintf(ACTIVITY_FILE,"   D%d: ",debuglevel);
			}
		}

		if (status_merge_stderr==TRUE) {
			vfprintf(stdout,fmt,ap);
		} else {
			vfprintf(stderr,fmt,ap);
		}
		if (ACTIVITY_FILE!=NULL) {
			vfprintf(ACTIVITY_FILE,fmt,ap);
		}
	}
}
#else
void levindicator( boolean levflag ) {}
void do_debug( int debuglevel, char *fmt, ...) {}
#endif

void leap_fprintf( FILE *stream, char *fmt, ...) {
/* leap_fprintf
 * Print a message to the specified stream 
 */
	va_list ap;

	/* Startup the argument wossname */
	va_start(ap, fmt);

	if (status_daemon==TRUE) {
		vsprintf(socketBuffer,fmt,ap);
		write(slaveSocket,socketBuffer,strlen(socketBuffer));

		/* Write to server log */
		do_debug(DEBUG_ACK,"CLIENT_WRITE: %s",socketBuffer);

	} else {
		/* Merge stdout/stderr streams if required */
		if ( (status_merge_stderr==TRUE) && (stream==stderr)) {
			vfprintf(stdout,fmt,ap);
		} else {
			/* Print the message to the stream */
			vfprintf(stream,fmt,ap);
		}
	}
	
	/* log it to output file */
	if (ACTIVITY_FILE!=NULL) {
		vfprintf(ACTIVITY_FILE,fmt,ap);
	}
}


void leap_printf( char *fmt, ...) {
/* leap_printf
 * Print a message to stdout */
	va_list ap;
	
	va_start(ap, fmt);
	
	if (status_daemon==TRUE) {
		vsprintf(socketBuffer,fmt,ap);
		write(slaveSocket,socketBuffer,strlen(socketBuffer));
		
		/* Write output to server log */
		do_debug(2,"CLIENT_WRITE: %s",socketBuffer);
	} else {
		vfprintf(stdout,fmt,ap);
	}
	if (ACTIVITY_FILE!=NULL) {
		vfprintf(ACTIVITY_FILE,fmt,ap);
	}
}
