/*
 * Name: relation.c
 * Description: Controls the basic relation structure.
 * Version: relation.c,v 1.206.2.1 2004/02/09 20:05:20 rleyton Exp
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
#include <sys/stat.h>

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
#include "errors.h"
#include "relation.h"
#include "util.h"
#include "dbase.h"
#include "attribs.h"
#include "hashing.h"
#include "relation.h"
#include "tuples.h"
#include "parser.h"
#include "leapio.h"
#include "rtional.h"

void delete_relation( relation rel) {
/* delete_relation
 * Delete the files associated with a given relation 
 */

	char bfilename[FILE_PATH_SIZE+1],filename[FILE_PATH_SIZE+1];
	int ret,count;

	/* Debug */
	do_debug(DEBUG_ENTER,"ENTERing delete_relation [%s]\n",relation_name(rel));

	/* Get the base directory to a relation */	
	relation_full_path(rel,bfilename);

	for (count=0;count<4;count++) {
		strcpy(filename,bfilename);

		/* Determine which file to delete */

		switch (count) {
			case 0:
				strcat(filename,LEAP_NEW_RELATION_EXT);
				break;
			case 1:
				strcat(filename,LEAP_FIELD_EXT);
				break;
			case 2:
				strcat(filename,LEAP_TEMPORARY_EXT);
				break;
			case 3:
				strcat(filename,LEAP_HASH_EXT);
				break;
			default:
				raise_error(ERROR_UNKNOWN,FATAL,"delete_relation: Something strange has happened");
		}

		ret=remove(filename);	

		if (ret!=0) {
			if ((errno)!=ENOENT)
				raise_error(ERROR_ERASE_FILE,NONFATAL,filename);
		}

	}


	/* Debug */
	do_debug(DEBUG_ENTER,"EXITing delete_relation\n");
}

int relation_insert( database db,
		      relation newrel) {
/* relation_insert
 * Insert the relation into the database relation structure 
 */
	relation currentRel,previousRel;
	database dbtouse;

	/* Debug */
	do_debug(DEBUG_ENTER,"ENTERing relation_insert [%s]\n",newrel->name);

	/* Check the insert is going into the right db */
	if ( (status_tempdb==TRUE) && (configuration!=TRUE) && (strcmp(database_name(db),TEMPDB_NAME)!=0) && (newrel->name[0]=='z') && (newrel->name[1]=='z')) {
		do_debug(DEBUG_INFO,"Inserting temporary relation [%s] in tempdb.\n",newrel->name);
		dbtouse=tempdb;
	} else {
		if ((status_debug)&&(status_debuglevel>=DEBUG_INFO)) {
			leap_fprintf(stderr,"Inserting normal relation [%s] into [%s].\n",newrel->name,database_name(db));
			leap_fprintf(stderr,"Relation [%s] is normal, db is tempdb, or config in process.\n",newrel->name);
		}
		dbtouse=db;
	}
	
	/* Check if this is the first relation to be inserted. */
	if (dbtouse->first_relation==NULL) {

		do_debug(DEBUG_INFO,"This is the first relation in the database\n");

		/* It is, so we can put it at the head of the structure */
		dbtouse->first_relation=newrel;
	} else {
		/* This isn't the FIRST relation, but it may yet
	      	 *	go at the head of the structure 
		 */
		currentRel=dbtouse->first_relation;
		previousRel=NULL;

		/* Search the structure for the position at which
		 * to insert the relation
		 */			
		while ( (currentRel!=NULL) && (strcmp(currentRel->name,newrel->name)<0)) {
				previousRel=currentRel;
				currentRel=currentRel->next;			
		}

		if ( (currentRel!=NULL) && (strcmp(currentRel->name,newrel->name)==0) ) {
			if (status_debug) {
				do_debug(DEBUG_INFO,"This is a duplicate.\n");
				raise_error(ERROR_DUPLICATE_ITEM,NONFATAL,currentRel->name);
			}
			return(RETURN_ERROR);
		} else if (previousRel==NULL) {
		/* We've found the right place in the structure */

			/* Insert at the start of the structure */
			do_debug(DEBUG_INFO,"This is to become the first relation in the database\n");

			dbtouse->first_relation->previous=newrel;
			newrel->next=dbtouse->first_relation;
			dbtouse->first_relation=newrel;

		} else if (currentRel==NULL) {
			/* Insert at the end of the structure */
			
			do_debug(DEBUG_INFO,"This is the last relation in the database \n");

			previousRel->next=newrel;
			newrel->previous=previousRel;
			dbtouse->last_relation=newrel;	
		} else {
			/* Insert it "normally" */

			do_debug(DEBUG_INFO,"This is a normal insert into the structure\n");

			previousRel->next=newrel;
			currentRel->previous=newrel;
			newrel->previous=previousRel;
			newrel->next=currentRel;
		}
	}


	/* Debug */
	do_debug(DEBUG_ENTER,"EXITing relation_insert\n");

	return(RETURN_SUCCESS);	
}

void create_tempfile( database db,
			char *relation_name) {
/* create_tempfile
 * Create a temporary "tag" file
 */
	FILE *tempfile;
	char fname[FILE_PATH_SIZE+1];

	/* Create the filename */
	sprintf(fname,"%s%s%s%s",database_dir(db),LEAP_RELATION_DIR,relation_name,LEAP_TEMPORARY_EXT);

	tempfile=fopen(fname,"w");

	if (tempfile==NULL) {
		raise_error(ERROR_FILE_OPENING,FATAL,fname);
	} else {

		fprintf(tempfile,"%s\nRelation: %s\n",LEAP_TEMPORARY_TXT,relation_name);
		fclose(tempfile);
	}
}
 
void remove_tempfile( database db, 
					char *relation_name) {
/* remove_tempfile
 * remove temporary file
 */
	char fname[FILE_PATH_SIZE+1];
	int ret;

	/* Create the filename */
	sprintf(fname,"%s%s%s%s",database_dir(db),LEAP_RELATION_DIR,relation_name,LEAP_TEMPORARY_EXT);

	ret=remove(fname);
	if (ret!=0) {
		if ((errno)!=ENOENT) 
			/* TODO - Does this have to be fatal? - DONE. No. 12.05.1995*/
			raise_error(ERROR_ERASE_FILE,NONFATAL,fname);
	}
}

void relation_change( database db,
  		  relation rel) {
/* relation_change
 * Change status of a relation from temp, to temp... 
 */

  printf("relation_change(): entered");
  exit(0);
	char expression[MAXIMUM_EXPRESSION+1];
	NOATTRIBUTES_TYPE noattribs;
	RELATION_TEMP_TYPE temp;
	RELATION_SYSTEM_TYPE system;
	FILE *fptr;
	char rname[RELATION_NAME_SIZE+1],*truth;

	fptr=generate_fileh(rel);
	if (fptr!=NULL) {
		/* Read the header information */
		if (tuple_readheader(&fptr,&noattribs,&temp,&system,rname)==RETURN_SUCCESS){
			/* Check the relation isn't a system relation! */
			if (system!=TRUE) {
				if (relation_update_header( db, rname, noattribs, !temp, system, &fptr)!=RETURN_SUCCESS){
					raise_error(ERROR_UNKNOWN,NONFATAL,"Updating temporary status in [%s]",relation_name(rel));
				} else {
					relation_temporary(rel)=(!relation_temporary(rel));
					/* We're inverting it in the update_header command */
					if (temp==FALSE) {
						truth=TRUE_STRING;
					}
					else {
						truth=FALSE_STRING;
					}
					sprintf(expression,"update (%s) (%s='%s') (%s='%s')",
						LEAP_DD_RELATIONS,
						LEAP_DDA_RELATIONS_NAME,rname,
						LEAP_DDA_RELATIONS_TEMP,truth);
					ddmaintenance(db,expression);
				}
			} else {
				/* Don't tamper with system relations! */
				raise_error(ERROR_TAMPERSYSTEM,NONFATAL,rname);
			}
			fclose(fptr);
		}
	}
}

relation relation_create( database db,
			  char *relation_name,
			  boolean temporary,
			  boolean system)
/* relation_create
 * Create the specified relation, and insert it into the
 * database structure
 */
{
  printf("relation_create(): entered\n");
  printf("relation_create(): relation_name = %s\n", relation_name);

	relation_struct *rel;
	FILE *relationfile;
	char fullname[FILE_PATH_SIZE+1]="",expression[MAXIMUM_EXPRESSION+1]="",systemstring[MAXIMUM_EXPRESSION+1]="";
	database dbtouse;

	/* Debug */
	do_debug(DEBUG_ENTER,"ENTERing relation_create\n");

	if (strlen(relation_name)>0) {
			if ( (status_tempdb==TRUE) && (tempdb!=NULL) && (strcmp(database_name(db),TEMPDB_NAME)!=0) && (relation_name[0]=='z') && (relation_name[1]=='z')) {
				do_debug(DEBUG_INFO,"Creating temporary relation [%s] in tempdb.\n",relation_name);
				dbtouse=tempdb;
			} else {
				if ((status_debug)&&(status_debuglevel>=DEBUG_INFO)) {
					leap_fprintf(stderr,"Creating normal relation [%s].\n",relation_name);
					leap_fprintf(stderr,"Relation [%s] is normal, db is tempdb, or config in process.\n",relation_name);
				}
				dbtouse=db;
			}

			if ( !(strstr(relation_name,"*")==NULL) || !(strstr(relation_name,"?")==NULL) 
				 || (strpbrk(relation_name,TOKEN_SEPERATORS))) {
				raise_error(ERROR_INVALID_RELNAME,NONFATAL,relation_name);

				return NULL;
			} else if (relation_find(dbtouse,relation_name)==NULL) {
				/* Create the relation structure */	
				rel=(relation_struct *) malloc(sizeof(relation_struct));
				check_assign(rel,"relation.relation_create");

				/* Populate the data */
			
				/* Copy the relation name */
				memset(rel->name,'\0',RELATION_NAME_SIZE+1);
				strcpy(rel->name,relation_name);

				/* Setup the paths */
				/* Using the API, we don't want to bother with directories... */
				if (dbtouse->API==TRUE) {
					sprintf(rel->filepath,"%s/",database_dir(dbtouse));
				} else {
					sprintf(rel->filepath,"%s%s",database_dir(dbtouse),LEAP_RELATION_DIR);
				}

#				ifdef FULL_DEBUG
					leap_fprintf(stderr,"Filepath (%i): %s\n",strlen(rel->filepath),rel->filepath);
#				endif

				/* Setup the name */
				sprintf(rel->filename,"%s%s",relation_name,LEAP_NEW_RELATION_EXT);

				/* Reset the position counter */
				rel->current_pos=0;	

				/* Reset the number of attributes in the relation */
				rel->noattributes=0;

				/* Mark the last deleted offset as unset */
				rel->last_deleted=0;

				/* Setup the cache */
				rel->rcache=NULL; /* Call cache_create TODO */

				/* Create the relation file */
				sprintf(fullname,"%s%s",rel->filepath,rel->filename);
				do_debug(DEBUG_INFO,"Creating file: [%s]\n",fullname);
#				ifdef FULL_DEBUG
					leap_printf("path:%s\nname:%s\n",rel->filepath,rel->filename);
#				endif
				relationfile=fopen(fullname,"wb");

        // debug
        printf("relation_create(): file fullname = %s\n", fullname);
        {
          struct stat buf;
          int fd = fileno(relationfile);
          fstat(fd, &buf);
          printf("relation_create(): filesize = %d\n", buf.st_size);
          printf("relation_create(): mode = %o\n", buf.st_mode);
          printf("relation_create(): uid = %d\n", buf.st_uid);
          printf("relation_create(): gid = %d\n", buf.st_gid);
        }
        // ok

				/* Set system relation to false */
				rel->system=system;

				rel->temporary=temporary;
				
				/* If the relations are to be permanent
				 * AND we're NOT in the master database
				 * override the temporary flag so the relation is permanent
				 */
				if ( (status_temporary_relations==FALSE) 
				  && (strcmp(database_name(dbtouse),MASTER_DB_NAME)!=0) ){
					rel->temporary=FALSE;
					temporary=FALSE;
				}

				/* If there was an error creating the file */
				if (relationfile==NULL) {
					raise_error(ERROR_FILE_OPENING,FATAL,fullname);
				} else {

					if ((strcmp(database_name(dbtouse),MASTER_DB_NAME)==0) && (relation_system(rel)!=TRUE)) {
						rel->temporary=TRUE;
						temporary=TRUE;
					}

					if (rel->system) {
						strcpy(systemstring,"TRUE");
					} else {
						strcpy(systemstring,"FALSE");
					}
          {
            struct stat buf;
            int fd = fileno(relationfile);
            fstat(fd, &buf);
            printf("relation_create()-2: filesize = %d\n", buf.st_size);
            printf("relation_create()-2: mode = %o\n", buf.st_mode);
            printf("relation_create()-2: uid = %d\n", buf.st_uid);
            printf("relation_create()-2: gid = %d\n", buf.st_gid);
          }
          // ok
					/* Write the new header. No attributes... */
					relation_create_write_header(relation_name,1,rel->temporary,system,&relationfile);
					if (rel->temporary) {
						sprintf(expression,"add (%s) (%s,%s,TRUE,0,FALSE,%s)",
							LEAP_DD_RELATIONS,relation_name(rel),
							rel->filename,systemstring);
					} else {
						sprintf(expression,"add (%s) (%s,%s,FALSE,0,FALSE,%s)",
							LEAP_DD_RELATIONS,relation_name(rel),
							rel->filename,systemstring);
					}
					/* Check that data dictionary updates are permitted */
					ddmaintenance(dbtouse,expression);
					/* Nope. */
					fclose(relationfile);
          {
            relationfile=fopen(fullname,"rb");
            printf("relation_create()-3: after fclose()\n");
            struct stat buf;
            int fd = fileno(relationfile);
            fstat(fd, &buf);
            printf("relation_create()-3: filesize = %d\n", buf.st_size);
            printf("relation_create()-3: mode = %o\n", buf.st_mode);
            printf("relation_create()-3: uid = %d\n", buf.st_uid);
            printf("relation_create()-3: gid = %d\n", buf.st_gid);
            unsigned int leapver;
            fread( &leapver, sizeof(leapver),1, relationfile);
            printf("relation-create()-3: leapver = %d\n", leapver);
            fclose(relationfile);
          }
				}

				/* Reset the structural information */
				rel->next=NULL;
				rel->previous=NULL;

				/* Set the updated flag. If a relation is not
				 * updated, then no need to write it out, or its
				 * hash table, which can be expensive and slow 
				 */		
				rel->updated=FALSE;
		

				/* Insert the relation into the db structure */
				if (relation_insert(dbtouse,rel)!=RETURN_SUCCESS) {
					delete_relation(rel);
				
					/* Debug */
					do_debug(DEBUG_ENTER,"EXITing (UNsuccessfully) relation_create\n");
					return(NULL);
				}

				/* Build the hash structure */
				rel->hash_table=hashing_create();


				/* Debug */
				do_debug(DEBUG_ENTER,"EXITing (successfully) relation_create\n");

				/* Return the relation ptr */
				return rel;
			} else {	
				/* Debug */
				do_debug(DEBUG_ENTER,"EXITing (UNsuccessfully) relation_create\n");

				raise_error(ERROR_DUPLICATE_ITEM,NONFATAL,relation_name);
				return(NULL);
			}
		} else {
				/* Debug */
				do_debug(DEBUG_ENTER,"EXITing (UNsuccessfully) relation_create\n");

				raise_error(ERROR_INVALID_RELNAME,NONFATAL,relation_name);
				return(NULL);
		} 
}

void relation_print( relation rel)
/* relation_print
 * Print out basic information about the relation
 */
{
	/* Debug */
	do_debug(DEBUG_ENTER,"ENTERing relation_print\n");

	if (relation_system(rel))
		leap_fprintf(stdout,"(");

	/* Print the relations name, and a tab */
	leap_fprintf(stdout,"%s",rel->name);

	/* Indicate if the relation is temporary or not */
	if (relation_temporary(rel)) {
		leap_fprintf(stdout,"*\n");
	} else if (relation_system(rel)) {
		leap_fprintf(stdout,")\n");
	} else {
		leap_fprintf(stdout,"\n");
	}

	/* Debug */
	do_debug(DEBUG_ENTER,"EXITing relation_print\n");
}

relation relation_find( database db,
			char *relname) {
/* relation_find
 * Locates the specified relation in the relation structure
 */
	relation rel;
	database dbtosearch;

	/* Debug */
	do_debug(DEBUG_ENTER,"ENTERing relation_find (%s)\n",relname);

	/* Temporary relation? */
	if ( (status_tempdb==TRUE) && (configuration!=TRUE) && (tempdb!=NULL) && (db!=tempdb) && (relname[0]=='z') && (relname[1]=='z')) {
		do_debug(DEBUG_INFO,"Relation [%s] is temporary.\n",relname);
		dbtosearch=tempdb;
	} else {
		if ((status_debug) && (status_debuglevel>=DEBUG_INFO)){
			if ((tempdb==NULL) && (relname[0]=='z') && (relname[1]=='z')) {
				leap_fprintf(stderr,"Tempdb not initialised and relation is temporary...\n");
			}
			leap_fprintf(stderr,"Relation [%s] is normal.\n",relname);
		}
		dbtosearch=db;
	}
	

	if (dbtosearch!=NULL) {
		/* Locate the first relation in the structure */
		rel=relation_findfirst(dbtosearch);

		/* Whilst the relation is not null (assigned), and the name
 	 	* doesn't match, move through the structure 
	 	*/	
		while ( (rel!=NULL) && (strcmp(relname,relation_name(rel))!=0) ) {
			rel=relation_findnext(rel);
		}
	} else {
		do_debug(DEBUG_ENTER,"EXITing UNsuccessfully (NULL db) relation_find\n");
		return(NULL);
	}

	/* Debug */
	do_debug(DEBUG_ENTER,"EXITing relation_find\n");

	/* Return the contents of rel - Which will be NULL if a 
	 * relation was not found, or contain the relation we want
	 */	
	return(rel);
}

void relation_display( database db ) {
/* relation_display
 * Displays all of the relations in the database structure
 */
	relation currentRel;

	/* Get the first relation */
	currentRel=relation_findfirst(db);

	leap_printf("Relation Name\n-------------\n");
	/* Whilst we have a valid relation */
	while (currentRel!=NULL) {
		/* Print the current relation */
		if ((status_regression==FALSE) || (relation_temporary(currentRel)!=TRUE))
			relation_print(currentRel);

		/* Fetch the next relation from the structure */
		currentRel=relation_findnext(currentRel);
	}
}

void relation_full_path(relation rel,
 			 char *string) {
/* relation_full_path
 * Gets the relations full path (including the name,
 * BUT NOT THE EXTENSION
 */

	sprintf(string,"%s%s",rel->filepath,rel->name);
}

relation relation_new_read( const char *path, char *name) {
/* relation_new_read
 * Read a specified relation into memory, and return
 * a ptr to it. Only works with new format relations.
 */
  printf("relation_new_read(): entered\n");
  printf("relation_new_read(): path = %s\n", path);
  printf("relation_new_read(): name = %s\n", name);

	relation rel;
	char relname[RELATION_NAME_SIZE+FILENAME_EXT_SIZE+1];
	FILE *tmp; /* Used to test if a file exists */
	char temp[FILE_PATH_SIZE+1];	
	char newrelname[FILE_PATH_SIZE+1];
	FILE *nrf;

	/* Debug */
	do_debug(DEBUG_ENTER,"ENTERing relation_new_read\n");

	/* Create the relation and check its ok */	
	rel=(relation_struct *) malloc(sizeof(relation_struct));
	check_assign(rel,"relation.relation_read");

	/* Copy the relation filename into the "name" */
	strcpy(relname,name);		
  printf("relation_new_read(): path = %s\n", path);
  printf("relation_new_read(): name = %s\n", name);

	if (strtok(relname,".")==NULL) {
		/* This shouldn't ever happen, because its only
		 * through relations_open that we're called, and
		 * it checks for LEAP_RELATION_EXT in the name...
		 */
		raise_error(ERROR_UNKNOWN,FATAL,"Not a relation? Something strange has happened");

		/* Dispose of the relations memory */
		free(rel);

		/* Debug */
		do_debug(DEBUG_ENTER,"EXITing (UNsuccessfully) relation_new_read\n");

		/* Return NULL. This is never reached (unless things really are going weird) */
		return(NULL);
	} else {
#ifdef FULL_DEBUG
	leap_fprintf(stderr,"relname==%s\n",relname);
#endif
		strcpy(rel->name,relname);

		if (  (strcmp(rel->name,LEAP_DD_RSHIP)==0) 
            ||(strcmp(rel->name,LEAP_DD_DATABASE)==0)
            ||(strcmp(rel->name,LEAP_DD_RELATIONS)==0)
            ||(strcmp(rel->name,LEAP_DD_ATTRIBUTES)==0)
            ||(strcmp(rel->name,LEAP_DD_TYPES)==0)
            ||(strcmp(rel->name,LEAP_DD_SOURCES)==0)
           ) {
				rel->system=TRUE;
			}
		else 
			rel->system=FALSE;
			
		strcpy(newrelname,path);
		strcat(newrelname,relname);
		strcat(newrelname,LEAP_NEW_RELATION_EXT);

		nrf=fopen(newrelname,"r");

		if (nrf!=NULL) {
			if (tuple_readheader( &nrf,&rel->noattributes, &rel->temporary, &rel->system, relname)!=RETURN_SUCCESS) {
				raise_error(ERROR_UNKNOWN,NONFATAL,"Error reading from relation file");
			}
			fclose(nrf);
		} else {
			free(rel);

			/* Debug */
			do_debug(DEBUG_ENTER,"EXITing (UNsuccesfully - No fhandle!) relation_new_read - File [%s]\n",newrelname);
			return(NULL);
		}

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

		/* rel->temporary=FALSE; */

		relation_full_path(rel,temp);
		strcat(temp,LEAP_HASH_EXT);
		tmp=fopen(temp,"r");

		/* If the hash file exists... */
		if (tmp!=NULL) {
			/* Try to load it */

			if (status_trace) {
				raise_message(MESSAGE,"Loading hash table for relation [%s]",name);
			}

			/* hashing_load(&rel->hash_table,temp);  */
			rel->hash_table=NULL; 
			rel->updated=FALSE;
			/* TODO - Implement hash load when relation is updated/inserted etc. *
			 * There is no need for it right now */

			/* Close the temporary file */
			fclose(tmp);
		} else {
			/* No table exists, so create it... */

			if ( (status_regression!=TRUE) && (status_quiet!=TRUE) ) {
				raise_message(MESSAGE,"Creating hash table for [%s].",rel->name);
			}

			/* Generate the hash table */
			rel->hash_table=build_hash_table(rel);
			rel->updated=FALSE;

			/* If this is a system table, save the newly generated hash table */
			if (relation_system(rel)) {
				hashing_save(rel->hash_table,temp); 
			}
		}

		/* TODO: This is stored in the new structure. When the
		 * new structure is enabled, read from this 
		 */

		/* Debug */
		do_debug(DEBUG_ENTER,"EXITing (successfully) relation_new_read\n");
	
		/* We've loaded the relation, so return the result */
		return(rel);
	}
}


void relations_open(database db,
		    const char *dirpath,
		    relation ddrel) {
	char stuple[ATTRIBUTE_MAXIMUM_SIZE+1];
	relation rtrel,rel;
	tuple ctuple;

	/* Debug */
	do_debug(DEBUG_ENTER,"ENTERing relations_open\n");

	if (ddrel!=NULL) {

		/* Insert the relation into the database */
		relation_insert(db,ddrel);
		database_datadictionary(db)++;

		rtrel=vprocess_query(db,"project (%s) (%s)",LEAP_DD_RELATIONS,LEAP_DDA_RELATIONS_NAME);

		if (rtrel!=NULL) {
			ctuple=tuple_readfirst(rtrel, TUPLE_BUILD, NULL);
			while (ctuple!=NULL) {
				tuple_to_string(ctuple,stuple);

				rel=relation_new_read(dirpath,stuple);

				/* Insert the relation into the database */
				if (rel!=NULL) {

					/* If the relation is a system relation, then increment the
				 	* counter of system relations - used to determine whether 
				 	* database is to be maintained in certain circumstances
				 	*/
					if (relation_system(rel)) {
						database_datadictionary(db)++;
					}

					if (relation_insert(db,rel)!=RETURN_SUCCESS) {
						/* If the relation already exists, get rid of
						 * this entry - it's already here!
						 */

						/* Doh! I was searching for this memory leak
						 * for a *long* time (it delayed 1.1.7) - simple
						 * when you think about it!!!! 
						 */

						/* Dispose of the relation's memory */
						relation_dispose_mem(db,&rel);

					} else {

#ifdef NEVER_SET
				/* This isn't finished yet... */
					/* Why do cleanup AFTER inserting the tuple? Because
					 * it makes deleting all the details much easier.
					 * A minor rewrite of relation_remove should enable
					 * a decent speedup
					 */
					if ((cleanup==TRUE) && (relation_temporary(rel)==TRUE)) {
						do_debug(DEBUG_INFO,"relations_open: [%s] is a temporary relation... Cleaning!\n",relation_name(rel));
						leap_printf("relations_open: [%s] is a temporary relation... Cleaning!\n",relation_name(rel));
						relation_remove(db,&rel);
					} else {
						do_debug(DEBUG_INFO,"relations_open: [%s] is a NORMAL relation...\n",relation_name(rel));
						leap_printf("relations_open: [%s] is a NORMAL relation...\n",relation_name(rel));
					}
#endif
					}
				} else {
					/* The relation does not exist on disk any more - delete the
					 * entries in the appropriate system relations
					 */
					if (status_regression!=TRUE) {
						leap_fprintf(stderr,"ERROR: Relation [%s] no longer exists. Updating (%s).\n",
							stuple,LEAP_DD_RELATIONS);
					} else {
						leap_fprintf(stderr,"ERROR: Relation [%s] no longer exists. Updating: %s, %s.\n",
							"TEMPNAME",LEAP_DD_RELATIONS,LEAP_DD_ATTRIBUTES);
					}
					(void)vprocess_query(db,"delete (%s) (%s='%s')",
							LEAP_DD_RELATIONS,LEAP_DDA_RELATIONS_NAME,stuple);
					(void)vprocess_query(db,"delete (%s) (%s='%s')",
							LEAP_DD_ATTRIBUTES,LEAP_DDA_ATTRIBUTES_RELATION,stuple);

				}

				(void) tuple_readnext( &ctuple, TUPLE_BUILD);
			}
			close_tuple(&ctuple,TUPLE_DISPOSE);

		} else {
			/* No relation */
			raise_error(ERROR_DATABASE_FORMAT,FATAL,"");
		}
	} else {
			/* No relation */
			raise_error(ERROR_DATABASE_FORMAT,FATAL,"No %s relation",LEAP_DD_RELATIONS);
	}

	/* Debug */
	do_debug(DEBUG_ENTER,"EXITing relations_open\n");

}

int relations_ddopen(database db) {
/* relations_ddopen
 * Load all of the relations in the specified database according
 * to data dictionary definitions
 */
	char dirpath[FILE_PATH_SIZE+1],ddname[FILE_PATH_SIZE+1];
	relation ddrel;

	/* Debug */
	do_debug(1,"ENTERing relations_ddopen\n");

	if (!status_quiet) raise_message(MESSAGE,"Opening the [%s] database...",database_name(db));

	if (db->subdirs) {
		sprintf(dirpath,"%s%s",database_dir(db),LEAP_RELATION_DIR);
	} else {
		sprintf(dirpath,"%s",database_dir(db));
	}
	sprintf(ddname,"%s%s",LEAP_DD_RELATIONS,LEAP_NEW_RELATION_EXT);
	ddrel=relation_new_read(dirpath,ddname);
	
	if (ddrel!=NULL) {
		relations_open(db, dirpath, ddrel);

		/* Debug */
		do_debug(DEBUG_ENTER,"EXITing (successfully) relations_ddopen\n");
		
		return(RETURN_SUCCESS);
	} else {
		/* Debug */
		do_debug(DEBUG_ENTER,"EXITing (UNsuccessfully) relations_ddopen\n");
		
		leap_printf("Database [%s] could not be opened (via [%s]).\n",database_name(db),LEAP_DD_RELATIONS);
		return(RETURN_ERROR);
	}
}

int LEAPAPI_relations_open(database db) {
/* LEAPAPI_relations_open
 * 
 */
	char dirpath[FILE_PATH_SIZE+1],ddname[FILE_PATH_SIZE+1];
	relation ddrel;

	sprintf(dirpath,"%s/",database_dir(db));
	sprintf(ddname,"%s%s",LEAP_DD_RELATIONS,LEAP_NEW_RELATION_EXT);
	ddrel=relation_new_read(dirpath,ddname);
	
	if (ddrel!=NULL) {
		relations_open(db, dirpath, ddrel);
		return(RETURN_SUCCESS);
	} else {
		return(RETURN_ERROR);
	}
 
}

/* relation_dispose...
 *
 * Occasionally we need to dispose of a relation, but not the file, especially
 * when opening the database  - ie. the project operation on leaprel creates
 * a temporary relation that is inserted into structures. If the temporary relation
 * file is then deleted because it already exists, it causes the odd situation
 * when a relation exists in the structures, but not on disk...
 */

void relation_dispose_mem( database db,
			relation *rel ) {
/* relation_dispose
 * Dispose of a relation's memory (and nothing else)
 */
 
 	do_debug(DEBUG_MEM,"***\nrel [%s] {%p::*%p}\n",relation_name((*rel)),rel,*rel);
	/* If the relation exists */
	if ((rel!=NULL) && (*rel!=NULL))  {

		if ((status_debug)&&(status_debuglevel>=DEBUG_MEM)) {
			leap_fprintf(stderr,"rel [%s] {%p}\n",relation_name((*rel)),*rel);
			leap_fprintf(stderr,"hsh      {%p}\n",(*rel)->hash_table);
		}
		/* Dispose of the relation's hash table */
		if ((*rel)->hash_table!=NULL) {
			do_debug(DEBUG_MEM,"*** disposing of hashtable *%p [%s]\n",&((*rel)->hash_table),relation_name((*rel)));
			hashing_terminate(&((*rel)->hash_table));
		}
	
		/* Dispose of the memory associated with the relation */
		do_debug(DEBUG_MEM,"disposing relation *%p [%s]\n",*rel,relation_name((*rel)));
		free(*rel);
		rel=NULL;
	} else {
		/* The relation doesn't exist, so complain... Can't say
		 * what, as the relation doesn't exist! 
		 */
		raise_error(ERROR_DELETE_NONEX_REL,NONFATAL,"(relation_dispose)");
	}
}

void relation_dispose( database db,
			relation *rel ) {
/* relation_dispose
 * Dispose of a relation
 */
 
	do_debug(DEBUG_ENTER,"ENTERing relation_dispose\n");
 	do_debug(DEBUG_MEM,"***\nrel [%s] {%p::*%p}\n",relation_name((*rel)),rel,*rel);

	/* If the relation exists */
	if ((rel!=NULL) && (*rel!=NULL))  {

		/* If the relation is temporary */
		if (relation_temporary((*rel))==TRUE) {

			/* If we're terminating, and this is the master database,
			 * or if its a user database... Delete the relation completely
			 * as its never needed - Temporary! 
			 */
			if ( ( (strcmp(database_name(db),MASTER_DB_NAME)==0) && (terminate==TRUE) )
			     || (strcmp(database_name(db),MASTER_DB_NAME)!=0) ) {

				do_debug(DEBUG_INFO,"deleting temporary *%p [%s]\n",*rel,relation_name((*rel)));

				delete_relation(*rel);
			}
		}	

		if ((status_debug)&&(status_debuglevel>=DEBUG_MEM)) {
			leap_fprintf(stderr,"rel [%s] {%p}\n",relation_name((*rel)),*rel);
			leap_fprintf(stderr,"hsh      {%p}\n",(*rel)->hash_table);
		}
		/* Dispose of the relation's hash table */
		if ((*rel)->hash_table!=NULL) {
			
			do_debug(DEBUG_MEM,"*** disposing of hashtable *%p [%s]\n",&((*rel)->hash_table),relation_name((*rel)));
			hashing_terminate(&((*rel)->hash_table));
		}
	
		/* Dispose of the memory associated with the relation */
		do_debug(DEBUG_MEM,"disposing relation *%p [%s]\n",*rel,relation_name((*rel)));

		free(*rel);
		rel=NULL;
	} else {
		/* The relation doesn't exist, so complain... Can't say
		 * what, as the relation doesn't exist! 
		 */
		raise_error(ERROR_DELETE_NONEX_REL,NONFATAL,"(relation_dispose)");
	}
}
	
void relations_dispose_all( database db ) {
/* relations_dispose_all
 * Dispose of all relations in a relation structure 
 */

	relation crel,prel,trel;
	char hashfile[FILE_PATH_SIZE+1];

	if (db!=NULL) {
	
			/* Updated flag in leaprel gets set to true every time there is an insert 
			 * - This section resets it to false, ready for the next run
		     */
			if (status_quiet!=TRUE) leap_printf("Resetting all updated flags: ");
			(void)vprocess_query(db,"update (%s) (%s='%s') (%s='%s')",LEAP_DD_RELATIONS,LEAP_DDA_RELATIONS_UPDATED,TRUE_STRING,LEAP_DDA_RELATIONS_UPDATED,FALSE_STRING);
			leap_printf("Done.\n");

			if (status_quiet!=TRUE) leap_printf("Updating hash tables: ");

			/* Locate the first relation */
			crel=relation_findfirst(db);

			/* Whilst the relation is valid */
			while (crel!=NULL) {
			
				if (status_quiet!=TRUE) {
						if ((status_regression==TRUE) && (relation_temporary(crel))) {
							leap_printf("TEMPNAME");
						} else {
							leap_printf(relation_name(crel));
						}
				}

				if (!relation_temporary(crel)) {
					relation_full_path(crel,hashfile);
					strcat(hashfile,LEAP_HASH_EXT);
					if (crel->updated==TRUE) {
						hashing_save(crel->hash_table,hashfile);
					}
					if (status_quiet!=TRUE) leap_printf(" ");
				} else {
					do_debug(DEBUG_INFO,"Data Dictionary Maintenance - leaprel\n");
					(void)vprocess_query(db,"delete (%s) (%s='%s')",
						LEAP_DD_RELATIONS,LEAP_DDA_RELATIONS_NAME,relation_name(crel));
					do_debug(DEBUG_INFO,"Data Dictionary Maintenance - leapattributes\n");
					(void)vprocess_query(db,"delete (%s) (%s='%s')",
						LEAP_DD_ATTRIBUTES, LEAP_DDA_ATTRIBUTES_RELATION,relation_name(crel));
					if (status_quiet!=TRUE) leap_fprintf(stdout,"* ");
				}
				fflush(stdout);

				prel=crel;

				/* Get the next relation */
				crel=relation_findnext(crel);

			}

			if (status_quiet!=TRUE) leap_printf("\n");

			/* Locate the first relation */
			crel=relation_findfirst(db);

			/* Whilst the relation is assigned */
			while (crel!=NULL) {

				/* Make a copy */
				prel=crel;

				/* Get the next relation */
				crel=relation_findnext(crel);

				/* Dispose of the relation's memory */
				relation_dispose(db,&prel);
			}
			/* Set the header to NULL */
			db->first_relation=NULL;
	} else {
		raise_message(MESSAGE,"Database not initialised...");
	}
}

void relation_remove( database db,
		      relation *rel ) {
/* relation_remove
 * Remove a relation from the relation structure 
 */

	if (*rel==NULL) {
		raise_error(ERROR_DELETE_NONEX_REL,NONFATAL,"(relation_remove)");
	} else {

		/* If the relation is the first relation */
		if (*rel==db->first_relation) {
			db->first_relation=(*rel)->next;
		} else {

			/* The relation is in the body somewhere */

			/* Point the relation before and after to each other
			 * (After checking that we're not going to hit NULL's
			 */
			if ( (*rel)->previous!=NULL )
				(*rel)->previous->next=(*rel)->next;

			if ( (*rel)->next!=NULL )
				(*rel)->next->previous=(*rel)->previous;
		}

		/* Remove the hashing table from memory */
		if ( &(*rel)->hash_table != NULL )
			hashing_terminate(&(*rel)->hash_table);

		/* Now that the relation has been removed from referring structures
		 * we should remove any trace of it's existance
		 */


		if (noddupdate==FALSE) {

		if (vprocess_query(db,"delete (%s) (%s='%s')",
			LEAP_DD_RELATIONS,LEAP_DDA_RELATIONS_NAME,relation_name((*rel)))==NULL) {
				raise_error(ERROR_DELREL,NONFATAL,relation_name((*rel)),NONFATAL);	
		} 

		if (vprocess_query(db,"delete (%s) (%s='%s')",
			LEAP_DD_ATTRIBUTES,LEAP_DDA_ATTRIBUTES_RELATION,relation_name((*rel)))==NULL) {
				raise_error(ERROR_DELREL,NONFATAL,relation_name((*rel)),NONFATAL);	
		} 

		}
		
		delete_relation(*rel); 
	
				
		/* Dispose of the memory used */
		free(*rel);
		*rel=NULL;
	}
}

relation relation_rename(    database db,
				char *first,
				char *second) {
/* relation_rename
 * Rename specified relation, or
 * specified attribute
 */
  printf("renamed?");
  exit(1);
	relation rtrel=NULL;
	int returncode;
	char relpath[FILENAME_MAX+1],nrelpath[FILENAME_MAX+1];
	char source[FILENAME_MAX+1],dest[FILENAME_MAX+1],*sptr;
	char rname[RELATION_NAME_SIZE+1],aname[ATTRIBUTE_NAME_SIZE+1];
	FILE *fptr;
	NOATTRIBUTES_TYPE noattributes,count;
	RELATION_TEMP_TYPE temp;
	RELATION_SYSTEM_TYPE system;
	boolean renamed=FALSE;
	long pos;
	attribute att;
	ATTRIBUTE_TYPE_TYPE atype=0;
	ATTRIBUTE_SIZE_TYPE asize=0;
	char ctype[ATTRIBUTE_TYPE_SIZE+1];

	/* See if the first string contains a ., ie. referencing an
	 * attribute. 
	 */

	if ( (strlen(first)>0) && (strlen(second)>0) ) {
		if (strchr(first,'.')==NULL) {
			/* No . - Therefore, we are renaming a relation... */
	
			rtrel=relation_find(db,first);
	
			if (rtrel!=NULL) {
				/* Do some renaming... */
				sprintf(rtrel->filename,"%s%s",second,LEAP_NEW_RELATION_EXT);
				sprintf(relpath,"%s%s%s",database_dir(db),LEAP_RELATION_DIR,first);
				sprintf(nrelpath,"%s%s%s",database_dir(db),LEAP_RELATION_DIR,second);
				sprintf(source,"%s%s",relpath,LEAP_NEW_RELATION_EXT);
				sprintf(dest,"%s%s",nrelpath,LEAP_NEW_RELATION_EXT);
				do_debug(DEBUG_INFO,"Renaming: [%s] to [%s].\n",source,dest);
				returncode=rename(source,dest);
				if (returncode==0) {
					sprintf(source,"%s%s",relpath,LEAP_HASH_EXT);
					sprintf(dest,"%s%s",nrelpath,LEAP_HASH_EXT);
					/* Check if the file exists! */
					fptr=fopen(source,"r");
					if (fptr!=NULL) {
						fclose(fptr);
						do_debug(DEBUG_INFO,"Renaming: [%s] to [%s].\n",source,dest);
						returncode=rename(source,dest);
					}
				} 
	
				if (returncode!=0) {
						raise_error(ERROR_UNKNOWN,NONFATAL,"Renaming relation...");
				} else {
					strcpy(relation_name(rtrel),second);
				}
			} else {
				raise_error(ERROR_CANNOT_FIND_REL,NONFATAL,first);
			}
		} else {

			cut_token(first,'.',rname);
			sptr=first; sptr++;
			cut_token(sptr,'\0',aname);

			rtrel=relation_find(db,rname);
			if (rtrel!=NULL) {

				if ( (strlen(rname)>0) && (strlen(aname)>0)) {
					sprintf(source,"%s%s%s%s",database_dir(db),LEAP_RELATION_DIR,rname,LEAP_NEW_RELATION_EXT);
					fptr=fopen(source,"r+b");
					if (fptr!=NULL) {
							/* Read the header information */
							if (tuple_readheader(&fptr,&noattributes,&temp,&system,rname)!=RETURN_SUCCESS){
								return(NULL);
							}
	
							/* Iterate through the attributes */
							for (count=0; ((renamed==FALSE)&&(count<noattributes)); count++) {
								/* Where are we in the file? */
								pos=ftell(fptr);
                printf("relation_rename(): pos = %d\n", pos);
                exit(1);

								/* Read the attribute */
								att=relation_attribute_read(&fptr);
	
								/* Is this the attribute we're after? */
								if (strcmp(attribute_name(att),aname)==0) {
									/* Rename the attribute name */
									strcpy(attribute_name(att),second);
	
									/* Find the original position */
									fseek(fptr,pos,SEEK_SET);

									/* Write the record */
									if (relation_create_write_attribute(att,&fptr)==RETURN_SUCCESS) {
										asize=attribute_size(att);
										atype=attribute_type(att);
										renamed=TRUE;
									} else {
										renamed=FALSE;
									}
								}

								/* Dispose of the attribute memory */
								attribute_dispose(&att);
							}						

							/* Close the file handle */
							fclose(fptr);

							if (renamed==FALSE) {
								raise_message(MESSAGE,"Could not find attribute [%s]",aname);
							} else {
								/* Update the leapattributes relation */
								ddmaintenance(db,"delete (%s) ((%s='%s') and (%s='%s'))",
									LEAP_DD_ATTRIBUTES,LEAP_DDA_ATTRIBUTES_RELATION,rname,
									LEAP_DDA_ATTRIBUTES_ATTRIBUTE,aname);
								ddmaintenance(db,"add (%s) (%s,%s,%s,%d)",LEAP_DD_ATTRIBUTES,
									rname,second,get_attribute_info(ctype,atype),asize);
							}
							return(rtrel);
					} else {
						raise_error(ERROR_FILE_OPENING,NONFATAL,source);
						return(NULL);
					}
				} else {
					raise_error(ERROR_UNKNOWN,NONFATAL,"Format of source attribute incorrect.");
					return(NULL);
				}
			} else {
				raise_error(ERROR_CANNOT_FIND_REL,NONFATAL,first);
			}
		}

		return(rtrel);
	} else {
		return(NULL);
	}
}

void relation_revattrib(attribute att) {
/* relation_revattrib
 * Print out the information contained in the specified
 * attribute in a reverse form that can be embedded in
 * a 'create database' command.
 */

    if (att!=NULL) {
        switch (attribute_type(att)) {
            case DT_STRING:
                leap_printf("(%s,%s,%d)",attribute_name(att),DTS_STRING,attribute_size(att));
                break;
            case DT_NUMBER:
                leap_printf("(%s,%s,%d)",attribute_name(att),DTS_NUMBER,attribute_size(att));
                break;
            case DT_BOOLEAN:
                leap_printf("(%s,%s,%d)",attribute_name(att),DTS_BOOLEAN,attribute_size(att));
                break;
            default:
                leap_printf(DTS_UNSUPPORTED);
        }
    }
}

void relation_reverse(relation rel) {
/* relation_reverse
 * Print out all of the attributes associated with
 * a relation in a reversable format.
 */
    attribute att;
    tuple ctuple;
    word count;

    /* Locate the first attribute */
    att=relation_attribute_readfirst(rel,&ctuple,&count);

	leap_printf("relation (%s) (",relation_name(rel));
    /* Move through the attributes */
    while (att!=NULL) {
        /* Display the first attribute */
        relation_revattrib(att);

        /* Locate the next attribute */
        att=relation_attribute_readnext(rel,&ctuple,att,&count);

		if (att!=NULL) leap_printf(",");
    }
	leap_printf(")\n");

    /* Display the relation database */
	rl_revdisplay( rel );
}
