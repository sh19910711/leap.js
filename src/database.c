/* 
 * Name: database.c
 * Description: Database management functions
 * Version: database.c,v 1.205.2.1 2004/02/09 20:05:20 rleyton Exp
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
#include <stdlib.h>
#include <string.h>
#include "consts.h"
#include "globals.h"
#include "dtypes.h"
#include "errors.h"
#include "database.h"
#include "rtional.h"
#include "relation.h"
#include "tuples.h"
#include "util.h"
#include "dbase.h"
#include "leapio.h"
#include "vars.h"
#include "parser.h"

int change_db( char *new_db ) {
/* change_db
 * Changes the current database to that specified in the parameter
 */
	char temp[MAXIMUM_EXPRESSION+1];
	char filen[FILE_PATH_SIZE+1];
	relation selectr,projectr;
	tuple ctuple;

	if (strlen(new_db)==0) {
		projectr=rl_project( master_db, relation_find(master_db,LEAP_DD_DATABASE), LEAP_DDA_DATABASE_NAME, "");	

		if (projectr!=NULL) {
				printf("Valid databases are:\n--------------------\n");
				/* Read the first tuple */
				ctuple=tuple_readfirst(projectr, TUPLE_BUILD, NULL);

				while (ctuple!=NULL) {
					tuple_to_string(ctuple,temp);
					if (strlen(temp) <= FILE_NAME_SIZE)
						leap_printf("%s\n",temp);
					else {
						leap_printf("%s - WARNING: Name exceeds OS limit of %d characters.\n",temp,FILE_NAME_SIZE);
					}
					(void) tuple_readnext( &ctuple, TUPLE_BUILD);
					/* (void) readnext_tuple( &ctuple, &tfile, &tnoattributes, TUPLE_BUILD); */
				}

				close_tuple(&ctuple,TUPLE_DISPOSE);

		} else {
			/* Can't locate the data dictionary relation */
			sprintf(temp,"Location of %s",LEAP_DD_DATABASE);
			raise_error(ERROR_DATA_DICTIONARY,NONFATAL,temp);
			return(RETURN_ERROR);
		}
	} else if (strcmp(database_name(current_db),new_db)==0) {
		raise_error(ERROR_ALREADY_OPEN,NONFATAL,new_db);
	} else {

		if (strlen(new_db)<=FILE_NAME_SIZE) {

			sprintf(temp,"(%s=\"%s\")",LEAP_DDA_DATABASE_NAME,new_db);

			/* Select the table from the DD_DATABASE (leapdata) table */
			selectr=rl_select( master_db, relation_find(master_db,LEAP_DD_DATABASE), temp, "");

#ifdef FULL_DEBUG
			rl_display(selectr);
#endif

			/* If that worked... */
			if (selectr!=NULL) {

				/* Project out the database */
				projectr=rl_project( master_db, selectr, LEAP_DDA_DATABASE_NAME, "");

				/* If no error occured  */
				if (projectr!=NULL) {

					/* Read the first tuple */
					ctuple=tuple_readfirst(projectr, TUPLE_BUILD, NULL);

					if ( (ctuple==NULL) ) {
						/* Read returned nothing */
						raise_error(ERROR_UNKNOWN_DATABASE,NONFATAL,new_db);
						return(RETURN_ERROR);

					} else {
						/* Convert it to a string */
						tuple_to_string(ctuple,temp); 
						

						/* Check the data returned. */
						/* is it empty, or "NULL" */
 						if ( (strcmp(temp,"")==0) || (strcmp(temp,"-")==0) ) {
							/* Make it so */
							close_tuple(&ctuple,TUPLE_DISPOSE);

							sprintf(temp,"Access of %s",LEAP_DD_DATABASE);
							raise_error(ERROR_DATA_DICTIONARY,NONFATAL,temp);

							return(RETURN_ERROR);
						} else {

							/* Read the next tuple */
							(void) tuple_readnext( &ctuple, TUPLE_BUILD);

						}

						/* If the read did not get to the end of the relation */
						if (ctuple!=NULL) {
							/* Close the tuple */
							close_tuple(&ctuple,TUPLE_DISPOSE);

							/* Report the error */
							raise_error(ERROR_UNDEFINED,NONFATAL,"Multiple instances in a dd table!");

							/* Return */
							return(RETURN_ERROR);
						} else {
							/* The read took us to the end of the tuple (and disposed
							 * of the tuple
							 */
							if (strlen(temp)>0) {
								/* Dispose of old db */
								relations_dispose_all(current_db);
								LEAPAPI_db_destroy(&current_db);

								/* Change the database */
								current_db=LEAPAPI_db_create(NULL,temp);
								relations_ddopen(current_db);
	
								raise_message(EVENT,"Database changed to: %s",database_name(current_db));

								/* Set the variable appropriately */
								set_variable(VAR_CURRENTDB,database_name(current_db));

								if (configuration!=TRUE) {
									sprintf(filen,"%s%s%s%s",database_dir(current_db),LEAP_SOURCE_DIR,LEAP_OPEN,LEAP_SOURCE_EXT);
									do_debug(DEBUG_INFO,"Assigning input to [%s]\n",filen);
									assign_input_stream(filen);
								}

							} else {
								raise_error(ERROR_UNKNOWN_DATABASE,NONFATAL,new_db);
								return(RETURN_ERROR);
							}
						}
					}
				} else {
					/* Can't access the Data dictionary relation in master database */
					sprintf(temp,"Access of %s",LEAP_DD_DATABASE);
					raise_error(ERROR_DATA_DICTIONARY,NONFATAL,temp);
					return(RETURN_ERROR);
				}
			} else {
				/* Can't locate the data dictionary relation */
				sprintf(temp,"Location of %s",LEAP_DD_DATABASE);
				raise_error(ERROR_DATA_DICTIONARY,NONFATAL,temp);
				return(RETURN_ERROR);
			}
		}
		else {
			fprintf(stderr,"Error with database name - exceeding OS limit (%d chars)!\n", FILE_NAME_SIZE);
			return(RETURN_ERROR);
		}
	}

	return(RETURN_SUCCESS);
}

int get_keys( database db,
				relation foreign,
			    relation primary,
			    char *foreign_names,
			    char *primary_names) {
/* get_keys
 * Fetches list of keys into strings pointed to by 
 * foreign/primary names, for relationship between relations primary and
 * foreign.
 */

	char *pname, *fname, temp[160], temp2[80], *current_keyset;
	relation select_result;
	tuple ctuple;
	unsigned int sposition,cposition;
	

	/* Reset the string */
	strcpy(primary_names,"");
	strcpy(foreign_names,"");
	strcpy(temp2,"");

	/* Check that the relations are valid */
	if ( (primary!=NULL) && (foreign!=NULL) ) {
		/* Get the relation names */
		pname=relation_name(primary);
		fname=relation_name(foreign);

		/* Build the select condition */
		sprintf(temp,"select (%s) ((%s=\"%s\")and(%s=\"%s\"))",LEAP_DD_RSHIP,
				LEAP_DDA_RSHIP_PREL, pname,
				LEAP_DDA_RSHIP_FREL, fname);
				
		do_debug(DEBUG_INFO,"Qual: %s",temp);

		select_result=vprocess_query(db,temp);
			/* Check that that worked */
			if (select_result!=NULL) {
	
					/* Read the first (and only, hopefully!) tuple */
					ctuple=tuple_readfirst(select_result, TUPLE_BUILD, NULL);
	
					if (ctuple!=NULL) {
							/* We will always start with the primary key */
							current_keyset=foreign_names;
			
							/* Start of data */
							sposition=LEAP_DDA_RSHIP_BDATA;
			
							cposition=sposition;
							/* Read the data */
							while ( (tuple_attribute(ctuple,cposition)!=NULL)
								&& (cposition<sposition+TOTAL_KEY_ATTRIBUTES) ) {
			
								if (cposition>=sposition+MAXIMUM_KEYS) 
									current_keyset=primary_names;
								
								/* Build the item, for later strtok'ing */
								if (strlen(tuple_d(ctuple,cposition))>0) {
									/* Include the item only if there are values there */
									sprintf(temp,"%s ",tuple_d(ctuple,cposition));
									strcat(current_keyset,temp);			
								} 
			
								cposition++;
							}
			
							/* Read the next tuple */
							(void) tuple_readnext( &ctuple, TUPLE_BUILD);
			
							/* If the read did not get to the end of the relation */
							if (ctuple!=NULL) {
								/* Close the tuple */
								close_tuple(&ctuple, TUPLE_DISPOSE);
								/* Report the error */
								raise_error(ERROR_UNDEFINED,NONFATAL,"Multiple instances in a dd table!");
								return(RETURN_ERROR);
							} 

							close_tuple(&ctuple, TUPLE_DISPOSE);
					} else {
						return(RETURN_ERROR);
					}
			} else {
				raise_error(ERROR_DATA_DICTIONARY,NONFATAL,"Cannot process [%s]",LEAP_DD_RSHIP);
				return(RETURN_ERROR);
			}
		
	}

	return(RETURN_SUCCESS);
}
