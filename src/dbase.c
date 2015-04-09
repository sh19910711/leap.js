/* 
 * Name: dbase.c
 * Description: Main database structure access/modification functions.
 * Version: dbase.c,v 1.205.2.1 2004/02/09 20:05:20 rleyton Exp
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
 
#if defined(HAVE_STDARG_H)
# include <stdarg.h>
#else
# include <varargs.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "consts.h"
#include "globals.h"
#include "dtypes.h"
#include "dbase.h"
#include "util.h"
#include "errors.h"
#include "relation.h"
#include "rtional.h"
#include "parser.h"
#include "leapio.h"

database LEAPAPI_db_create(const char *path,
						const char *name) {
/* LEAPAPI_db_create
 * API routine to create a database structure on a specific database
 */
 
	database db;

	db=(database_struct *) malloc(sizeof(database_struct));
	check_assign(db,"dbase.database_create");

	/* Make sure the database name does not exceed the
	 * size of a directory name - This may be removed when
	 * the Datadictionary xrefs names with directories.
	 */
	strncpy(db->name,name,FILE_NAME_SIZE);

	/* Place a terminating NULL at the end of the string
	 * incase the name is at the maximum, and is missing
	 * the all important last char!
	 */
	(db->name)[FILE_NAME_SIZE]='\0';


	db->first_relation=NULL;
	db->last_relation=NULL;
	
	db->datadictionary=0;

	if (path==NULL) {
		/* Populate the base directory - Not the right way
 		 * here - as the dd will be used to populate it... 
		 */
		db->API=FALSE;
		db->subdirs=TRUE;
		sprintf(db->basedir,"%s%s%s%c",LEAP_BASE_DIR,LEAP_DATABASE_DIR,name,DIR_SEPERATOR);
	} else {
		db->API=TRUE;
		db->subdirs=FALSE;
		sprintf(db->basedir,"%s",path);
	}
	
	return(db);
}

void LEAPAPI_db_destroy(database *db) {
/* This releases the memory of the database, and deallocates
 * the pointer - Should only be called at the end of the
 * database 'life' !
 */
	/* Dispose of the memory */
	free(*db);

	/* Deallocate ptr */
	(*db)=NULL;
}


char *database_name(database db) {
/* database_name
 * Returns ptr to the database name string
 */
	return(db->name);
}

FILE *ddfile=NULL;

int ddredirect_start() {
/*
 * START the redirection for data dictionary maintenance
 */
	ddfile=fopen(scratchfile,"w");

	if (ddfile==NULL) {
		raise_error(ERROR_FILE_OPENING,NONFATAL,scratchfile);
		return(RETURN_ERROR);
	}

	noddupdate=TRUE;

	return(RETURN_SUCCESS);
}


int ddredirect_stop() {
/*
 * Stop the redirection for data dictionary maintenance
 */
	if (fclose(ddfile)!=EOF) {
		noddupdate=FALSE;
		ddfile=NULL;
		return(RETURN_SUCCESS);
	} else {
		raise_error(ERROR_UNKNOWN,NONFATAL,"Trying to close DD redirection file");
		return(RETURN_ERROR);
	}

}

void ddredirect_execute( database db ) {
/*
 * Execute the data dictionary maintenance redirected
 */
	char *lptr,line[MAXIMUM_EXPRESSION+1];
	
	ddfile=fopen(scratchfile,"r");

	if (ddfile==NULL) {
		raise_error(ERROR_FILE_OPENING,NONFATAL,scratchfile);
	} else {

		lptr=fgets( line, MAXIMUM_EXPRESSION, ddfile );
		while (lptr!=NULL) {

			do_debug(DEBUG_INFO,"Executing command from ddredirection: [%s]\n",lptr);

			vprocess_query(db, lptr);
			lptr=fgets( line, MAXIMUM_EXPRESSION, ddfile );

			
		}
		fclose(ddfile);
		ddfile=NULL;

		/* Remove the file */
		if (remove(scratchfile)!=0) {
			raise_error(ERROR_DELFILE,NONFATAL,scratchfile);
		}
	}
}

void ddmaintenance(database db, char *fmt, ... ) {
/*
 * Depending on whether data dictionary maintenance is occuring,
 * execute a query, or store commands in a file
 */
	char expression[MAXIMUM_EXPRESSION+2];
	va_list ap;

	va_start(ap, fmt);

	if (noddupdate==FALSE) {
		vsprintf(expression,fmt,ap);
		process_query(db,expression);
		do_debug(DEBUG_INFO,"DDMaintenance command: [%s]\n",expression);
	} else {
		if (ddfile!=NULL) {
			vsprintf(expression,fmt,ap);
			do_debug(DEBUG_INFO,"DDMaintenance command: [%s]\n",expression);
			fprintf(ddfile, "%s\n", expression);
		}
	}
}


char *database_dir(database db) {
/* database_dir
 * Returns ptr to string containing the database directory
 */
	
	return(db->basedir);
}

database LEAPAPI_db_init(const char *path,
						 const char *name,
						 boolean sub) {
/* LEAPAPI_db_init
 * CREATE an external and internal database structure.
 * path = path to DIRECTORY to create
 * sub = create source and relation directories
 */

	char p[FILENAME_MAX+1];
	char dname[FILENAME_MAX+1];
	char expr[MAXIMUM_EXPRESSION+1];
	char dirpath[FILE_PATH_SIZE+1];
	char tpath[FILE_PATH_SIZE+1];
	database db;
	relation rel;
	boolean ismaster=FALSE;
	FILE *tf;
	
		do_debug(DEBUG_ENTER,"Entering LEAPAPI_db_init\n");
		do_debug(DEBUG_INFO,"Path: [%s]\nName: [%s]\n",path,name);
	/* Is this the master database? */
	if (strcmp(name,MASTER_DB_NAME)==0) ismaster=TRUE;

	/* Firstly create the directory */
	sprintf(tpath,"%s/%s",path,name);
	if ((mkdir( tpath,  0777)!=0) && (errno!=EEXIST) ){
		raise_error( ERROR_CREATE_DIR,NONFATAL,tpath);
		return(NULL);
	}


	strcpy(p,tpath);

	if (sub==TRUE) {

		sprintf(dname,"%s/",tpath);
		strcat(dname,LEAP_RELATION_DIR);
		/* Remove the trailing / - no good when creating directories. */
		dname[strlen(dname)-1]='\0';
		if ( (mkdir(dname,0777)!=0) && (errno!=EEXIST) ){
			raise_error( ERROR_CREATE_DIR,NONFATAL,dname);
			return(NULL);
		} else {
			sprintf(dname,"%s/",tpath);
			strcat(dname,LEAP_SOURCE_DIR);
			/* Remove the trailing / - no good when creating directories. */
			dname[strlen(dname)-1]='\0';
			if ( (mkdir(dname,0777)!=0) && (errno!=EEXIST) ) {
				raise_error( ERROR_CREATE_DIR,NONFATAL,dname);
				return(NULL);
			} else {
				/* Create the 'open.src' file */
				sprintf(dname,"%s/%s%s%s",tpath,LEAP_SOURCE_DIR,LEAP_OPEN,LEAP_SOURCE_EXT);
				tf=fopen(dname,"w+");
				fprintf(tf,"# Database: %s\n",name);
				if (tf!=NULL){
					fclose(tf);
				}
				if (ismaster==TRUE) {
						/* Create the 'startup.src' file */
						sprintf(dname,"%s/%s%s%s",tpath,LEAP_SOURCE_DIR,LEAP_STARTUP,LEAP_SOURCE_EXT);
						tf=fopen(dname,"w+");
						if (tf!=NULL){
							fclose(tf);
						}
				}

			}
		}
		db=LEAPAPI_db_create(NULL,name);
	} else {
		db=LEAPAPI_db_create(path,p);
	}
	
	/* Disable data dictionary updates */
	if (ddredirect_start()!=RETURN_SUCCESS){
		leap_fprintf(stderr,"No data dictionary updates for system tables can be made.\n");
		noddupdate=TRUE;
	}
	

	if (db!=NULL) {
		/* Is this the master database? */
		if (ismaster==TRUE) {
			sprintf(expr,"(NAME,STRING,25),(DIR,STRING,127)");
			create_user_relation(db,expr,LEAP_DD_DATABASE,FALSE,TRUE);
		}

		/* Build the leaprel relation */
		sprintf(expr,"(NAME,STRING,25),(FNAME,STRING,25),(TEMP,BOOLEAN,1),(NOATTRIBS,INTEGER,4),(UPDATED,BOOLEAN,1),(SYSTEM,BOOLEAN,1)");
		relation_insert(db,create_user_relation(db,expr,LEAP_DD_RELATIONS,FALSE,TRUE));

		/* Build the attributes relation */
		sprintf(expr,"(RELATION,STRING,25),(ATTRIBUTE,STRING,25),(TYPE,STRING,25),(SIZE,INTEGER,5)");
		relation_insert(db,create_user_relation(db,expr,LEAP_DD_ATTRIBUTES,FALSE,TRUE));
		
		/* Build the types relation */
		sprintf(expr,"(NAME,STRING,25),(SIZE,INTEGER,5)");
		relation_insert(db,create_user_relation(db,expr,LEAP_DD_TYPES,FALSE,TRUE));

		/* Build the relship relation */
		sprintf(expr,"(frelation,string,25),(prelation,string,25),(fkey1,string,25),(fkey2,string,25),(fkey3,string,25),(pkey1,string,25),(pkey2,string,25),(pkey3,string,25)");
		relation_insert(db,create_user_relation(db,expr,LEAP_DD_RSHIP,FALSE,TRUE));

		/* Build the scripts relation */
		sprintf(expr,"(NAME,string,25),(FILE,string,127)");
		relation_insert(db,create_user_relation(db,expr,LEAP_DD_SOURCES,FALSE,TRUE));

	}

	if (ddredirect_stop()!=RETURN_SUCCESS){
		leap_fprintf(stderr,"Data dictionary updates for system tables are now being made.\n");
		noddupdate=FALSE;
	}

	if (ismaster==TRUE) {
			/* This is the master database... */
			master_db=db;

			/* Read the relation details and plop it into the database */
			sprintf(dirpath,"%s%s",database_dir(db),LEAP_RELATION_DIR);
			rel=relation_new_read(dirpath,LEAP_DD_DATABASE);
			if (rel!=NULL) {
				relation_insert(db,rel);
			} 
	}

	if (sub==TRUE) {
		/* Read the relation details and plop it into the database */
		sprintf(dirpath,"%s%s",database_dir(db),LEAP_RELATION_DIR);
	} else {
		sprintf(dirpath,"%s",p);
	}

		
	/* No sub-directories == No master db, so skip the population step */
	if (sub==TRUE) {
		/* Populate the master database structures to point to the new db */
		vprocess_query(master_db,"add (%s) (%s,%s)",LEAP_DD_DATABASE,name,name);
	}

	ddredirect_execute( db );

	/*
	 * Now maintain static relations
	 */

	/* Update the leaptypes relation */
	vprocess_query(db,"add (%s) (%s,%d)",LEAP_DD_TYPES,DTS_STRING,DT_SIZE_MAXIMUM_STRING);
	vprocess_query(db,"add (%s) (%s,%d)",LEAP_DD_TYPES,DTS_INTEGER,DT_SIZE_MAXIMUM_NUMBER);
	vprocess_query(db,"add (%s) (%s,1)",LEAP_DD_TYPES,DTS_BOOLEAN);

	vprocess_query(db,"add (%s) (%s,%s,%s,-,-,%s)",LEAP_DD_RSHIP,LEAP_DD_ATTRIBUTES,LEAP_DD_RELATIONS,LEAP_DDA_ATTRIBUTES_RELATION,LEAP_DDA_RELATIONS_NAME);


	/* Clear the structures */
	relations_dispose_all(db);
	LEAPAPI_db_destroy(&db);
	
	if (sub==TRUE) {
			/* Reload the entire database to check it's ok */
			db=LEAPAPI_db_create(NULL,name);
	} else {
			/* Reload the entire database to check it's ok */
			db=LEAPAPI_db_create(p,name);
	}
	relations_ddopen(db);
	
	return(db);
}

void database_reverse( database db) {
/* database_reverse
 * Reverse engineer a database
 */

	relation crel;

	crel=relation_findfirst(db);

	leap_printf("create database (%s)\n",database_name(db));
	leap_printf("use %s\n",database_name(db));
	leap_printf("set temporary off\n");

	while (crel!=NULL) {
	
		if ( ((relation_system(crel)!=TRUE) || (strcmp(relation_name(crel),"relship")==0)) && (relation_temporary(crel)!=TRUE) ) {
			relation_reverse(crel);
		} else {
			do_debug(DEBUG_INFO,"[%s] is a temporary/system relation.\n",relation_name(crel));
		}

		crel=relation_findnext(crel);
	}

	reverse_source_code();
}

database whichdb( database db, char *rel ) {
	database dbtouse=db;

	if ( (status_tempdb==TRUE) && (tempdb!=NULL) && (strcmp(database_name(db),TEMPDB_NAME)!=0) && (rel[0]=='z') && (rel[1]=='z')) {
		do_debug(DEBUG_INFO,"Relation [%s] is to be in tempdb.\n",rel);
		dbtouse=tempdb;
	} else {
		if ((status_debug)&&(status_debuglevel>=DEBUG_INFO)) {
			leap_fprintf(stderr,"Relation [%s] is normal, db is tempdb, or config in process.\n",rel);
			dbtouse=db;
		}
	}
	return(dbtouse);
}
database whichdb_struct( database db, relation rel ) {
	database dbtouse;

	if ( (status_tempdb==TRUE) && (tempdb!=NULL) && (strcmp(database_name(db),TEMPDB_NAME)!=0) && (relation_name(rel)[0]=='z') && (relation_name(rel)[1]=='z')) {
		do_debug(DEBUG_INFO,"Relation [%s] is to be in tempdb.\n",relation_name(rel));
		dbtouse=tempdb;
	} else {
		if ((status_debug)&&(status_debuglevel>=DEBUG_INFO)) {
			leap_fprintf(stderr,"Relation [%s] is normal, db is tempdb, or config in process.\n",relation_name(rel));
		}
	}

	return(dbtouse);

}
