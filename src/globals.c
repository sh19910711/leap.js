/*
 * Name: globals.c,v
 * Description: Global variables.
 * Version: globals.c,v 1.204.2.1 2004/02/09 20:05:20 rleyton Exp
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
#include "dtypes.h"
#include "globals.h"
#include "consts.h"

/* Actual database's (defined as extern in dtypes.h) */
database master_db, current_db, tempdb=NULL;

/* TRUE When LEAP is on the way down... */
boolean terminate, terminatenow;

/* This is normally stdin, but the user might want to source a file */
FILE *input_stream;

char BEEP=7;

char current_prompt[MAXIMUM_PROMPT+1];

/* LEAP internal version number */
int leapver=(LEAP_MAJOR_VERSION*100)+(LEAP_MINOR_VERSION*10)+(LEAP_REVISION_VERSION);

/* No updates to data dictionary */
boolean noddupdate=FALSE;

/* Configuration mode */
boolean configuration=FALSE;
boolean opening=TRUE;

/* Cleanup on startup */
boolean cleanup=FALSE;

/* Recording mode */
boolean recording=FALSE;

/* Recording fptr */
FILE *recordingfptr;

/* Client socket */
int slaveSocket;

