/* 
 * Name: convert.c
 * Description: Convert (pre)1.0 database to 1.1 format.
 * Version: convert.c,v 1.204.2.1 2004/02/09 20:05:20 rleyton Exp
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
#include <stdlib.h>
#include <stdio.h>

/* These are needed for directory access */
#ifndef __MSDOS__
#include <sys/types.h>
#else
#include <sys\types.h>
#include <dirent.h>
#endif

/* This is taken from autoconf.info, and defines how
 * the directory structure will be accessed. If
 * none is defined, something really strange
 * exists...
 */
#if HAVE_DIRENT_H
#include <dirent.h>
#define NAMLEN(dirent) strlen((dirent)->d_name)
#else
#define dirent direct
#define NAMLEN(dirent) (dirent)->d_namlen
#if HAVE_SYS_NDIR_H
#include <sys/ndir.h>
#endif
#if HAVE_SYS_DIR_H
#include <sys/dir.h>
#endif
#if HAVE_NDIR_H
#include <ndir.h>
#endif
#endif

#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "consts.h"
#include "globals.h"
#include "dtypes.h"
#include "relation.h"
#include "util.h"
#include "dbase.h"
#include "attribs.h"
#include "hashing.h"
#include "relation.h"
#include "tuples.h"
#include "parser.h"
#include "leapio.h"



relation relation_read( char *path, char *name) {
/* relation_read
 * Read a specified relation into memory, and return
 * a ptr to it
 */
	relation rel;
	char relname[RELATION_NAME_SIZE+FILENAME_EXT_SIZE+1];
	FILE *tmp; /* Used to test if a file exists */
	char tstring[FILE_PATH_SIZE]; /* Used for test */
	char newrelname[FILE_PATH_SIZE];
	char oldrelname[FILE_PATH_SIZE];
	FILE *nrf;

	/* Create the relation and check its ok */	
	rel=(relation_struct *) malloc(sizeof(relation_struct));
	check_assign(rel,"relation.relation_read");

	/* Copy the relation filename into the "name" */
	strcpy(relname,name);		

	if (strtok(relname,".")==NULL) {
		leap_fprintf(stderr,"Not a relation? Something strange has happened");

		/* Return NULL. This is never reached (unless things really are going weird) */
		return(NULL);
	} else {
		strcpy(rel->name,relname);

		if (  (strcmp(rel->name,LEAP_DD_RSHIP)==0) 
	  		||(strcmp(rel->name,LEAP_DD_DATABASE)==0)
            ||(strcmp(rel->name,LEAP_DD_RELATIONS)==0)
           )
			rel->system=TRUE;
		else 
			rel->system=FALSE;

		/* Populate with some information */	
		strcpy(rel->filepath,path);
		strcpy(rel->filename,name);

		/* Populate the field information */
		strcpy(rel->fieldname,relname);
		/* And add the field extension */
		strcat(rel->fieldname,LEAP_FIELD_EXT);

		/* Reset the structural information */	
		rel->next=NULL;
		rel->previous=NULL;

		rel->rcache=NULL;

		/* Build the temporary file name */	
		strcpy(tstring,path);
		strcat(tstring,relname);
		strcat(tstring,LEAP_TEMPORARY_EXT);
		
		/* Try to open the temporary file */
		tmp=fopen(tstring,"r");

		/* If tmp is NULL, no file. (False)
		 * If tmp is NOT null, file (True)
 		 */
		if (tmp==NULL) {
			rel->temporary=FALSE;
		} else {
			rel->temporary=TRUE;
			/* Close the file */
			fclose(tmp);
		}

		rel->noattributes=0;
		/* TODO: This is stored in the new structure. When the
		 * new structure is enabled, read from this 
		 */

		strcpy(newrelname,path);
		strcat(newrelname,relname);
		strcat(newrelname,LEAP_NEW_RELATION_EXT);

		nrf=fopen(newrelname,"r");

		if (nrf==NULL) {
			if (status_trace) {
				leap_fprintf(stderr,"No new relation format file for relation %s exists, creating...",name);
				fflush(stdout);
			}


			build_new_relation( rel, newrelname );

			sprintf(oldrelname,"%s%s%s",path,relname,LEAP_FIELD_EXT);	
			sprintf(newrelname,"%soldfmt/%s%s",path,relname,LEAP_FIELD_EXT);	
			if (rename(oldrelname,newrelname)!=0) {
				leap_fprintf(stderr,"(Unable to relocate old format field def. file.)\n");
			}

			sprintf(oldrelname,"%s%s%s",path,relname,LEAP_RELATION_EXT);	
			sprintf(newrelname,"%soldfmt/%s%s",path,relname,LEAP_RELATION_EXT);	
			if (rename(oldrelname,newrelname)!=0) {
				leap_fprintf(stderr,"(Unable to relocate old format relation)\n");
			}


			if (status_trace) {
				leap_fprintf(stdout,"Done!\n");
			}
		} else {
			fclose(nrf);
		}
	
		/* We've loaded the relation, so return the result */
		return(rel);
	}
}

void relations_convert_open(database db) {
/* relations_open
 * Load all of the relations in the specified database
 */
	boolean masterdb;
	DIR *directory;
	char dirpath[FILE_PATH_SIZE+1];
	char newdir[FILE_PATH_SIZE+1],ddname[FILE_PATH_SIZE+1];
	mode_t mode;
	tuple nt;
	relation ddrel;
	struct dirent *d;
	relation rel;

	masterdb=(strcmp(database_name(db),MASTER_DB_NAME)==0);

	leap_fprintf(stdout,"Converting database [%s] ",database_name(db));
	if (masterdb==TRUE) {
		leap_printf("(Master Database)\n");
	} else {
		leap_printf("(User Database)\n");
	}

	sprintf(dirpath,"%s%s",database_dir(db),LEAP_RELATION_DIR);
	directory=opendir(dirpath);
		
		if (directory!=NULL) {

			/* Create a backup directory for the old format
			 * relations.
			 */

			mode=sprintf(newdir,"%soldfmt",dirpath);
			mkdir(newdir,0777);
		
			d=readdir(directory);

			while (d!=NULL) {

#ifdef __MSDOS__
				/* See comment in util.c for list_source
				 * DOS filenames are always in capitals.
				 */
				downcase(d->d_name);
#endif

					if (strstr(d->d_name,LEAP_RELATION_EXT)!=NULL) {
						/* Read the relation from the disk */
						rel=relation_read(dirpath,d->d_name);
						/* Insert the relation into the database */
						relation_insert(db,rel);
					}

					d=readdir(directory);
				}		

			/* Close the directory */
			closedir(directory);
	}

	leap_fprintf(stdout,"Completed conversion of [%s]\n",database_name(db));
	
	leap_fprintf(stdout,"Updating [%s]...",LEAP_DD_RELATIONS);
	fflush(stdout);

	/* Open the leaprel database */
	sprintf(dirpath,"%s%s",database_dir(db),LEAP_RELATION_DIR);
	sprintf(ddname,"%s%s",LEAP_DD_RELATIONS,LEAP_NEW_RELATION_EXT);
	ddrel=relation_new_read(dirpath,ddname);

	/* Did the relation get returned? */
	if (ddrel!=NULL) {
	
		/* Prepare a tuple */
		nt=tuple_prepare(ddrel);
		
		/* Find the first relation in the database */
		rel=relation_findfirst(db);
		
		/* Whilst a relation is being returned */
		while( (rel!=NULL) && (strlen(relation_name(rel))>=0) ){

			/* TODO: Why does the relation name sometimes have nothing in it? */
		
			/* Populate the output tuple */
			
			/* Relation name */
			strcpy( tuple_d(nt,LEAP_DDA_RELATIONS_NAME_NO),relation_name(rel));
			
			/* Relation filename */
			sprintf(tuple_d(nt,LEAP_DDA_RELATIONS_FNAME_NO), "%s%s", relation_name(rel), LEAP_NEW_RELATION_EXT);
			
			/* Temporary status */
			if ( relation_temporary(rel) ) {
				strcpy( tuple_d(nt,LEAP_DDA_RELATIONS_TEMP_NO),TRUE_STRING);
			} else {
				strcpy( tuple_d(nt,LEAP_DDA_RELATIONS_TEMP_NO),FALSE_STRING);
			}

			/* Noattributes status */
			strcpy( tuple_d(nt,LEAP_DDA_RELATIONS_NOATTRIBS_NO),"1");
			
			/* Updated status */
			strcpy( tuple_d(nt,LEAP_DDA_RELATIONS_UPDATED_NO),FALSE_STRING);
			
			/* System relation status */			
			if ( relation_system(rel) ) {
				strcpy( tuple_d(nt,LEAP_DDA_RELATIONS_SYSTEM_NO),TRUE_STRING);
			} else {
				strcpy( tuple_d(nt,LEAP_DDA_RELATIONS_SYSTEM_NO),FALSE_STRING);
			}
		
			/* Write the tuple */
			(void)tuple_append(nt);
	
			/* Read the next relation */
			rel=relation_findnext(rel);
		}
		
		/* Dispose of the output tuple */
		tuple_dispose(&nt);
	} else {
		leap_fprintf(stderr,"\nERROR: System relation [%s] does not exist. See release NOTES\nin base directory!\n",
			LEAP_DD_RELATIONS);
		exit(1);
	}
	
	leap_fprintf(stdout,"Completed conversion.\n");
}


int main() {
	database db;

	/* Set the flag that this is an upgrade, so don't flag obsoleted errors */
	leapver=LEAP_UPGRADE;

	/* Create the database structure... */
	db=LEAPAPI_db_create(NULL,"convert");
	
	/* Overwrite the base directory. */
	sprintf(db->basedir,".%c",DIR_SEPERATOR);
	
	/* Open the relations and convert them to the new format */
	relations_convert_open(db);

	return(0);
}
