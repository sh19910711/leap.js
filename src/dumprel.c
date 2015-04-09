/* 
 * Name: dumprel.c
 * Description: Utility program to dump a relation.
 * Version: dumprel.c,v 1.202.2.1 2004/02/09 20:05:20 rleyton Exp
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
#include <string.h>
#include "tuples.h"


/* Main routine */
int main(int argc, char *argv[]) {
	
	/* Check the argument count */
	if (argc<=1) {
	
		/* Error message */
		fprintf(stderr,"Usage: %s [relation...]\n",argv[0]);
	}
	
	/* For each of the arguments */
	while (argc > 1) {

		if (strcmp(argv[1],"-v")==0) {
			print_header(TRUE);
			printf("Utility: dumprel - Dumps out internal relation contents\n");
		} else {
	
			/* Dump the relation */
			printrelinfo(argv[1]);
		}
		
		/* Update the arguments */
		argc--;
		argv++;
	
	}
	return(0);
}

