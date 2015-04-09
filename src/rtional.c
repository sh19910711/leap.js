/* 
 * Name: rtional.c
 * Description: Relational Operators
 * Version: rtional.c,v 1.208.2.1 2004/02/09 20:05:20 rleyton Exp
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
#include "rtional.h"
#include "relation.h"
#include "attribs.h"
#include "tuples.h"
#include "hashing.h"
#include "util.h"
#include "cond.h"
#include "database.h"
#include "dbase.h"
#include "leapio.h"


char special_condition_true[]="(TRUE=TRUE)";
char special_condition_false[]="(TRUE=FALSE)";

relation project_create_rtn_relation(database db,
				     relation rel,
				     char *attribute_list,
				     char *destname) {
/* project_create_rtn_relation
 * Creates an (empty) relation with the specified attributes only
 */
	relation return_rel;
	char *rname,rel_name[RELATION_NAME_SIZE+1]="",cur_attribute[ATTRIBUTE_NAME_SIZE+1]="",*captr;
	char attribs[MAXIMUM_MAXIMUM_ATTRIBUTE]="", attrdescr[MAX_DTS_SIZE+1];
	attribute attrib;
	FILE *nf;
	word noattribs;
	char path[FILENAME_MAX+1]="";
	int res;

	/* If the destination name is empty, a name should
	 * be generated
  	 */

	if (strlen(destname)==0) {
		rname=generate_random_string(RANDOM_NAME_SIZE,rel_name);
	} else {
		rname=destname;
	}

	/* Create the relation */
	return_rel=relation_create(db,rname,TRUE,FALSE);

	/* If something happened, and the relation is not created,
	 * then return NULL
	 */	
	if ( (return_rel==NULL) || (rel==NULL) ) {
		return NULL;
	} else {
	
		sprintf(path,"%s%s%s",return_rel->filepath,rname,LEAP_NEW_RELATION_EXT);
		do_debug(DEBUG_INFO,"Opening >%s<\n",path);
		nf=fopen(path,"a+b");

		if ((status_debug)&&(status_debuglevel>=DEBUG_INFO)) {
			if (nf!=NULL) {
				leap_fprintf(stderr,"Opened >%s<\n",path);
			} else {
				leap_fprintf(stderr,"FAILED to open >%s<\n",path);
			}
		}
		/* Reset the number of attributes */
		return_rel->noattributes=0;

		/* Copy the attribute list - we want to modify it */
		strcpy(attribs,attribute_list);

		/* Get the first field from the list */
		cut_token(attribs,'\0',cur_attribute);
		captr=skip_to_alnum(cur_attribute);

		attrib=attribute_find(rel,captr);

		noattribs=0;
		while (attrib!=NULL) {
			noattribs++;
			/* Create the attribute in the new relation */
				res=fseek(nf,0,SEEK_END);
				do_debug(DEBUG_ALL,"  fseek returned [%d]\n",res);
				relation_create_write_attribute(attrib,&nf);

			/* Check that enough data dictionary relations are open
			 * before we start updating relations.
			 */
			if (database_datadictionary(db)>=NO_DD_RELATIONS) {
				if ( (status_tempdb==TRUE) && (configuration!=TRUE) && (strcmp(database_name(db),TEMPDB_NAME)!=0) && (return_rel->name[0]=='z') && (return_rel->name[1]=='z')) {
					
					ddmaintenance(tempdb,"add (%s) (%s,%s,%s,%d)",LEAP_DD_ATTRIBUTES,relation_name(return_rel),
														   attribute_name(attrib), get_attribute_info( attrdescr, attribute_type( attrib)),
															attribute_size(attrib));
				} else {
					ddmaintenance(db,"add (%s) (%s,%s,%s,%d)",LEAP_DD_ATTRIBUTES,relation_name(return_rel),
														   attribute_name(attrib),  get_attribute_info( attrdescr, attribute_type( attrib)),
															attribute_size(attrib));
				}
			}

			/* Get the next string */
			cut_token(attribs,'\0',cur_attribute);
			captr=skip_to_alnum(cur_attribute);

			attribute_dispose(&attrib);

			/* Locate the attribute */
			attrib=attribute_find(rel,captr);
		}

		fclose(nf);
		nf=fopen(path,"r+b");
		relation_update_header(db, rname, noattribs, return_rel->temporary, return_rel->system, &nf);
		fclose(nf);
		nf=fopen(path,"r+b"); 
		res=fseek(nf,0,SEEK_END);  
			do_debug(DEBUG_ALL,"  fseek returned [%d]\n",res);
		res=fflush(nf); 
			do_debug(DEBUG_ALL,"  fflush returned [%d]\n",res);
		
		relation_create_write_eoh_marker( noattribs, &nf );  
		
		res=fclose(nf);
			do_debug(DEBUG_ALL,"  fclose returned [%d]\n",res);

#ifdef FULL_DEBUG
		if ((status_debug)&&(status_debuglevel>=DEBUG_ALL)) {
			leap_fprintf(stderr,">>>DEBUG<<<\n");
			dump_rel(rname);
			leap_fprintf(stderr,">>>END OF DEBUG<<<\n");
		}
#endif
		return(return_rel);
	}
}

relation create_user_relation(database db,
 			      char *attrib_list,
			      char *dest_name,
			      boolean istemporary,
			      boolean issystem) {
/* create_user_relation
 * Creates a relation for the user according to schema in attrib_list
 * of format ( (name1,type1,length1) ... )
 */

	relation rtrel;
	attribute wattrib;
	char attrib[MAXIMUM_EXPRESSION+1],*attribptr;
	char name[ATTRIBUTE_NAME_SIZE+1],type[ATTRIBUTE_TYPE_SIZE+1],size[10],*sptr;
	char path[FILE_PATH_SIZE+1];
	FILE *nf;
	unsigned int count;
	int intsize;

	if (strlen(attrib_list)==0) {
		raise_error(ERROR_ATTRIBUTE,NONFATAL,"None specified!");
		return(NULL);
	}

	/* Assume that destname is valid. Wouldn't be called otherwise */
	rtrel=relation_create(db,dest_name,istemporary, issystem);

	if (rtrel==NULL) {
		/* Something happened when creating the relation */
		return(NULL);
	} else {
	
		sprintf(path,"%s%s%s",rtrel->filepath,dest_name,LEAP_NEW_RELATION_EXT);
		do_debug(DEBUG_INFO,"Opening >%s<\n",path);
		nf=fopen(path,"a+b");
	
		if ((status_debug)&&(status_debuglevel>=DEBUG_INFO)) {
			if (nf!=NULL) {
				leap_fprintf(stderr,"Opened >%s<\n",path);
			} else {
				leap_fprintf(stderr,"FAILED to open >%s<\n",path);
			}
		}
		/* Reset the number of attributes */
		rtrel->noattributes=0;
		count=0;
		
		attribptr=cut_to_right_bracket(attrib_list,1,TRUE,attrib);
		while ( (attribptr!=NULL) && (strlen(attribptr)!=0) ) {

			do_debug(DEBUG_MEM,"attribptr [#%d:%s]",count,attribptr);

			if ( ( copy_to_token(attrib,", ",name)!=NULL)
  		   	&&( copy_to_token(NULL,", ",type)!=NULL) ) {
					upcase(type);
			
					sptr=copy_to_token(NULL,", ",size);
					if (sptr!=NULL) {
						intsize=atoi(size);
					} else {
						intsize=-1;
					}
					do_debug(DEBUG_MEM,"intsize==[%d]\n",intsize);
					if (strcmp(type,DTS_STRING)==0) {
						if ( (intsize==-1) ||
						 	( (intsize>=DT_SIZE_MINIMUM_STRING) &&
						 	(intsize<=DT_SIZE_MAXIMUM_STRING) ) ) {
								wattrib=attribute_build(rtrel,name,DT_STRING,intsize);
							} else {
									wattrib=attribute_build(rtrel,name,DT_STRING,DT_SIZE_MAXIMUM_STRING);
									raise_error(ERROR_ATTRIBUTE_SIZE_LARGE,NONFATAL,"Attribute [%s] Size [%d] Max [%d]",
										name,intsize,DT_SIZE_MAXIMUM_STRING);
									intsize=DT_SIZE_MAXIMUM_STRING;
							}
							if (wattrib!=NULL) {
								relation_create_write_attribute(wattrib,&nf);
							} 
					} else 
					if (strcmp(type,DTS_NUMBER)==0) {
						if ( (intsize==-1) ||
						 	( (intsize>=DT_SIZE_MINIMUM_NUMBER) &&
						 	(intsize<=DT_SIZE_MAXIMUM_NUMBER) ) ) {
								wattrib=attribute_build(rtrel,name,DT_NUMBER,intsize);
							} else {
								wattrib=attribute_build(rtrel,name,DT_NUMBER,DT_SIZE_MAXIMUM_NUMBER);
								intsize=DT_SIZE_MAXIMUM_NUMBER;
								raise_error(ERROR_ATTRIBUTE_SIZE_LARGE,NONFATAL,"Attribute [%s] Size [%d] Max [%d]",
									name,intsize,DT_SIZE_MAXIMUM_NUMBER);
							}
							if (wattrib!=NULL) {
								relation_create_write_attribute(wattrib,&nf);
							}
					} else 
					if (strcmp(type,DTS_BOOLEAN)==0) {
							wattrib=attribute_build(rtrel,name,DT_BOOLEAN,intsize);
							if (wattrib!=NULL) {
								relation_create_write_attribute(wattrib,&nf);
							}
					} else {
						raise_error(ERROR_UNSUPPORTED_DTYPE,NONFATAL,type);	
						relation_remove(db,&rtrel);
						return(NULL);
					}

				if (wattrib!=NULL) {
					count++;

					ddmaintenance(db,"add (%s) (%s,%s,%s,%d)",LEAP_DD_ATTRIBUTES,relation_name(rtrel),
					   			name, type,
								intsize);

					attribute_dispose(&wattrib);
				}
			}
			attribptr=cut_to_right_bracket(attrib_list,1,TRUE,attrib);
		}

		do_debug(DEBUG_INFO,"[%d] attributes.\n",count-1);
		fclose(nf);
		nf=fopen(path,"r+b"); 
		relation_update_header(db, dest_name, count, rtrel->temporary,rtrel->system, &nf);
		fseek(nf,0,SEEK_END);
		fflush(nf);
		relation_create_write_eoh_marker( count, &nf );

		fclose(nf);
		
	}
	if (status_temporary_relations==FALSE) {
		/* Make all user create relations permanent if
		 * option for temp. relations is not set.
         	 */
		remove_tempfile(db,relation_name(rtrel));
		relation_temporary(rtrel)=FALSE;
	}
	return(rtrel);
}

relation create_duplicate_rtn_relation( database db,
					relation rel,
					char *destname) {
/* create_duplicate_rtn_relation
 * Create a relation with the same attribute names as specified relation
 */
	relation rtrel;
	char *rname,rel_name[RELATION_NAME_SIZE+1]="",path[FILE_PATH_SIZE]="",attrdescr[MAX_DTS_SIZE+1];
	attribute attr;
	unsigned int count;
	tuple ctuple;
	word anum;
	FILE *nf;

	if (strlen(destname)==0) {
		rname=generate_random_string(RANDOM_NAME_SIZE,rel_name);
	} else {
		rname=destname;
	}
	rtrel=relation_create(db,rname,TRUE,FALSE);

	if ( (rtrel==NULL) || (rel==NULL) ) {
		return(NULL);
	} else {
		sprintf(path,"%s%s%s",rtrel->filepath,rname,LEAP_NEW_RELATION_EXT);
		do_debug(DEBUG_INFO,"Opening >%s<\n",path);
		nf=fopen(path,"a+b");

		if ((status_debug)&&(status_debuglevel>=DEBUG_ACK)) {
			if (nf!=NULL) {
				leap_fprintf(stderr,"Opened >%s<\n",path);
			} else {
				leap_fprintf(stderr,"FAILED to open >%s<\n",path);
			}
		}

		/* Populate the values */
		rtrel->noattributes=0;

		/* Get the first attribute in the source relation */
		attr=relation_attribute_readfirst(rel,&ctuple,&anum);

		count=0;

		/* While the attribute is valid */				
		while (attr!=NULL) {

			/* Create a attribute in the new relation with
			 * the correct name and types
			 */
			fseek(nf,0,SEEK_END);
			relation_create_write_attribute(attr,&nf);

			count++;

            /* Check that enough data dictionary relations are open
             * before we start updating relations.
             */
            if (database_datadictionary(db)>=NO_DD_RELATIONS) {
                if ( (status_tempdb==TRUE) && (configuration!=TRUE) && (strcmp(database_name(db),TEMPDB_NAME)!=0) && (rtrel->name[0]=='z') && (rtrel->name[1]=='z')) {
       
                    ddmaintenance(tempdb,"add (%s) (%s,%s,%s,%d)",LEAP_DD_ATTRIBUTES,relation_name(rtrel),
                                                           attribute_name(attr), get_attribute_info( attrdescr, attribute_type( attr)),
                                                           attribute_size(attr));
                } else {
                    ddmaintenance(db,"add (%s) (%s,%s,%s,%d)",LEAP_DD_ATTRIBUTES,relation_name(rtrel),
                                                           attribute_name(attr),  get_attribute_info( attrdescr, attribute_type( attr)),
                                                            attribute_size(attr));
                }
            }


			if (count>=MAXIMUM_ATTRIBUTES) {
				/* Report an error */
				raise_error(ERROR_EXCEEDED_ATTRIBUTE_LIMIT,NONFATAL,"Current limit set to %u.",MAXIMUM_ATTRIBUTES);

				attribute_dispose(&attr);
			} else {
				attr=relation_attribute_readnext(rel,&ctuple,attr,&anum);
			}
		}

		fclose(nf);
		nf=fopen(path,"r+b"); 
		relation_update_header(db, rname, count, rtrel->temporary,rtrel->system, &nf);
		fseek(nf,0,SEEK_END);
		fflush(nf);
		relation_create_write_eoh_marker( count, &nf );

		fclose(nf);

		close_tuple(&ctuple,TUPLE_DISPOSE);
	}

	/* Return the new relation */
	return(rtrel);
}

relation rl_project(database db,
		    relation rel,
		    char *attribute_list,
		    char *destname) {
/* rl_project
 * Returns the result of performing a relation project 
 * operation on the specified relation, taking the attributes
 * specified in attribs
 */

	relation rtrel;
	HashTable HT;
	char att_list[MAXIMUM_MAXIMUM_ATTRIBUTE],*attrib;
	char tuple_string[MAXIMUM_ALL_DATA],*attribmkr;
	char found_string[HASH_KEY_SIZE];
	boolean success;
	attribute att;
	tuple ta,bta,nt;
	unsigned int c,noattributes;
	unsigned int reference[MAXIMUM_ATTRIBUTES],reference_position;
	int readresult=0;

	do_debug(DEBUG_ENTER,"OPERATOR Start: rl_project\n");

	/* Create the return relation */
	rtrel=project_create_rtn_relation(db,rel,attribute_list,destname);

	/* If an error occured */
	if ( (rtrel==NULL) || (rel==NULL) ) {
		/* Return NULL */
		return(NULL);
	} else {
		/* Otherwise... */

		/* Initialise the reference array */
		for (c=0;c<MAXIMUM_ATTRIBUTES;c++) reference[c]=0;

		/* Create a hash table */
		HT=hashing_create();

		/* Add an additional seperator to check make
		 * sure the last attribute is picked up 
		 */
		sprintf(att_list,"%c%s%c",ATTRIBUTE_SEPERATOR,attribute_list,ATTRIBUTE_SEPERATOR);

		/* TODO - What if there is an empty attribute, ie. ,, 
		 * or multiple spaces in the list! 
		 */
		/* Get the first attribute in the field list */
		do_debug(DEBUG_INFO,"Attribute List: %s\n",att_list);

		attrib=strchr(att_list,ATTRIBUTE_SEPERATOR);		
		/* strchr will give us the first seperator, we want
	     * the next char 
		 */
		attrib++;
		
		/* Now strip any leading spaces... BUG FIX! 25/05/99 */
		strip_leading_spaces(attrib);

		/* Find the NEXT seperator, and put a null there */
		attribmkr=strchr(attrib,ATTRIBUTE_SEPERATOR);
		*attribmkr='\0';		
	
		att=attribute_find(rel,attrib);

		/* If we did find the attribute... */
		if (att!=NULL) {

			attribute_dispose(&att);
			/* ta=readfirst_tuple(rel,&tuple_file,&rel->noattributes,TUPLE_BUILD,NULL); */
			ta=tuple_readfirst(rel,TUPLE_BUILD,NULL);
			bta=ta;

			/* Ok, now we are going to move through the
 		 	* tuple and determine which attributes are necessary
 		 	* for the project operation. We're going to use
 	 	 	* an array of unsigned ints for this, so we can
		 	* determine the location of the attributes in the
		 	* tuple, and copy them out quickly
		 	*/

			/* Initialise the reference index */
			reference_position=0;

			/* Whilst we have a valid tuple (we might not have
		 	* anything) - TODO - What if the first tuple is empty?
		 	* move through and determine which fields are where.
		 	*/
			while ( (ta!=NULL) && (attrib!=NULL) ) {
		
				/* Start at the first attribute */	
				c=0;
	
				/* This needs some explanation! Whilst the current
			 	* attribute in the tuple is not null, AND the name of
		 	 	* the attribute in the tuple is not the one we're looking
			 	* for...
			 	*/
				do_debug(DEBUG_INFO,"rl_project: No attribs: [%d]\n",rel->noattributes);
				do_debug(DEBUG_INFO,"rl_project: Search Attrib: [%s]\n",attrib);
				while ( (c<rel->noattributes) 
 					 && (strcmp(attrib,(tuple_attribute(ta,c))->name)!=0) )  {
					/* Move to the next attribute in
				 	* the tuple
				 	*/
					c++;
				}
	
				/* On exiting the above while loop, we'll
			 	* have found the attribute, or exhausted
			 	* the possibilities
			 	*/
		
				do_debug(DEBUG_MODERATE,"At attrib: [%d]\n",c);
					
				if ( tuple_attribute(ta,c) != NULL ) {
					/* We've found something */
		
					/* Store the position of the node */
					reference[reference_position]=c;
	
					/* Increment the positioning index */
					reference_position++;
				} else {
					raise_error(ERROR_CANTFIND_ATTR,NONFATAL,"Project: >%s<",attrib);
					close_tuple(&ta,TUPLE_DISPOSE);
					return(NULL);
				}
	
				/* Get the next attribute from the list */
				attrib=attribmkr;
				attrib++;
	

				attribmkr=strchr(attrib,ATTRIBUTE_SEPERATOR);
				if (attribmkr!=NULL) { 
					*attribmkr='\0'; 

					/* Now strip any leading spaces... BUG FIX! 25/05/99 */
					strip_leading_spaces(attrib);

				} else { 
					attrib=NULL; 
				}

			} /* while */
	
			/* Create a tuple from the new return relation */
			nt=tuple_prepare(rtrel);

			if (nt!=NULL) {
				noattributes=reference_position;
		
				/* Whilst we have a valid tuple */
				while ( (ta!=NULL) && (readresult!=EOF) ){
		
					reference_position=0;
		
					/* Move through our array of attributes */
					while (reference_position<noattributes) {
						/* Copy INTO the new tuple, the data from the old tuple. Take each
				 	 	* position one at a time...
					 	*/
						strcpy( tuple_d(nt,reference_position), tuple_d(ta,reference[reference_position]));
						reference_position++;
					} /*while*/	
	
					/* Convert the data in the tuple to a string */
					tuple_to_string( nt, tuple_string );

					do_debug(DEBUG_MODERATE,"Tuple String: %s\n",tuple_string);
		
					/* Try and locate the string version of the tuple
				 	* in the hash table 
				 	*/
					hashing_retrieve(HT,tuple_string,found_string,&success);
		
					/* If the tuple was not located in the hash table */
					if (!success) {
						/* Insert the string version of the tuple into
					 	* the hash table */
	
						hashing_insert(HT,tuple_string,REQ_CALC);	
		
						/* Write the tuple out to disk */
						/* TODO - Handle return code of write_tuple */	
						tuple_append(nt);
					} else if (status_debug) {
						do_debug(DEBUG_INFO,"[project] Attempted to add duplicate tuple to hash table! (%s)\n",tuple_string);
					} /* if */
		
					ta=tuple_readnext(&ta,TUPLE_BUILD);
				} /* while */

				/* Dispose of the tuples */
				close_tuple(&nt,TUPLE_DISPOSE);
			} else {
				leap_fprintf(stderr,"No output tuple prepared...\n");
			}
			close_tuple(&ta,TUPLE_DISPOSE);
		} /*if*/ else {
			raise_error(ERROR_CANTFIND_ATTR,NONFATAL,attrib);
		}
	} /*if*/

	hashing_terminate(&HT);

	do_debug(DEBUG_ENTER,"OPERATOR End: rl_project\n");

	return(rtrel);
}

void populate( char string_array[MAXIMUM_ATTRIBUTES][ATTRIBUTE_MAXIMUM_SIZE],
	       relation rel ) {

	attribute attr;
	unsigned int count;
	tuple ctuple;
	word anum;

	count=0;

	/* Load the first attribute */	
	attr=relation_attribute_readfirst( rel, &ctuple,&anum );

	/* Whilst valid attribute */
	while (attr!=NULL) {
		strcpy(string_array[count],attribute_name(attr));

		/* Fetch the next attribute name */
		attr=relation_attribute_readnext(NULL, &ctuple, attr, &anum);

		/* Increment the counter */
		count++;
	}

	close_tuple(&ctuple,TUPLE_DISPOSE);
}
	
boolean rl_is_union_compatible( relation rel1,	
				relation rel2 ) {
/* rl_is_union_compatible
 * Checks if the specified relations are union compatible
 */
	boolean result=FALSE;
	unsigned int noattributes,idx1,idx2;
	char r1[MAXIMUM_ATTRIBUTES][ATTRIBUTE_MAXIMUM_SIZE];
	char r2[MAXIMUM_ATTRIBUTES][ATTRIBUTE_MAXIMUM_SIZE];

	/* Check the relations are valid, and that they
	 * have the same number of attributes
	 */
	if ( ( (rel1!=NULL) && (rel2!=NULL) )
		 &&  (rel1->noattributes==rel2->noattributes)
		) {
		noattributes=rel1->noattributes;

		populate(r1,rel1);
		populate(r2,rel2);

		idx1=0;
		idx2=0;

		/* Whilst nothing has happened to abort, and the index
		 * of the first relation is within bounds
		 */
		while ( (result==FALSE) && (idx1<noattributes) ) {

			while ( (strcmp(r1[idx1],r2[idx2])!=0) && (idx2<noattributes) )
				idx2++;

			/* If we went over the number of attributes, then we've
			 * found a field that doesn't exist
			 */
			if (idx2>noattributes)
				result=TRUE;
			else
				idx1++;

		}

		/* Negate the result, because for union compatibility
		 * post-requisite of loop will be result=FALSE 
		 */
		result=(!result);
	} 

	/* Return the contents of result */
	return(result);
}

relation rl_union( database db,
		   relation rel1, relation rel2,
		   char *destname) {
/* rl_union
 * Performs the algebraic operation UNION with the specified relations
 */
	relation rtrel;
	word counter;
	tuple ct,nt,oct;

	/* Check that the source relations are union compatible */
	if ( rl_is_union_compatible(rel1,rel2) ) {

		rtrel=create_duplicate_rtn_relation(db,rel2,destname);

		/* If a relation has been returned */
		if (rtrel!=NULL) {
			/* Create an empty tuple based on the new relation */
			nt=tuple_prepare(rtrel);

			/* Read the first tuple from the first relation */
			ct=tuple_readfirst(rel1,TUPLE_BUILD,NULL);
			oct=ct;

			/* Whilst the tuple is valid */
			while (ct!=NULL) {
				counter=0;
				
				/* Whilst the current attribute datum is ok */
				while (counter<ct->noattributes) {
					/* Copy the data from the source tuple into the dest. tuple */	
					strcpy( tuple_d(nt,counter),tuple_d(ct,counter));	

					/* Increment the counter */
					counter++;
				}

				/* TODO - handle return code of write_tuple */
				(void) tuple_append(nt);

				(void) tuple_readnext(&ct,TUPLE_REUSE);
			}

			close_tuple(&oct,TUPLE_DISPOSE);

			/* Move onto the second relation, and write any
			 * tuples not in the first to disk.
			 * TODO - Check that the attributes are in the same order 
		 	 * this is done in Pascal version with tuple_to_string_order_by_relation
			 * or such like, where the tuple is converted to the string in the
			 * order of a specified relation known to be union compatible
			 */
			ct=tuple_readfirst(rel2,TUPLE_BUILD,NULL);
			oct=ct;
		
			while (ct!=NULL) {
				counter=0;
				
				/* Whilst the current attribute datum is ok */
				while (counter<ct->noattributes) {
					/* Copy the data from the source tuple into the dest. tuple */	
					strcpy( tuple_d(nt,counter),tuple_d(ct,counter));	

					/* Increment the counter */
					counter++;
				}

				/* Write the tuple to disk */
				(void) tuple_append(nt);

				/* Load the next tuple from the second relation */	
				(void) tuple_readnext(&ct,TUPLE_REUSE);
			}
		} else {
			/* No relation was created */
			return(NULL);
		}

		close_tuple(&nt,TUPLE_DISPOSE);
		close_tuple(&oct,TUPLE_DISPOSE);

	} else {
		rtrel=NULL;
		raise_error(ERROR_UNION_COMPATIBILITY,NONFATAL,"Union of %s and %s.",relation_name(rel1),relation_name(rel2));
	}
	return(rtrel);
}

relation rl_intersect( database db,
		       relation rel1, 
		       relation rel2,
		       char *destname) {
/* rl_intersect
 * Intersection operator
 */
	relation rtrel;
	char s[MAXIMUM_ALL_DATA],result[MAXIMUM_ALL_DATA];
	boolean success;
	word counter;
	HashTable htable;
	tuple ct,nt;

	/* Check that the source relations are union compatible */
	if ( rl_is_union_compatible(rel1,rel2) ) {

		rtrel=create_duplicate_rtn_relation(db,rel2,destname);

		/* If a relation has been returned */
		if (rtrel!=NULL) {
			/* Create an empty tuple based on the new relation */
			nt=tuple_prepare(rtrel);

			/* Create the hash table */
			htable=hashing_create();

			/* Read the first tuple from the first relation */
			ct=tuple_readfirst(rel1,TUPLE_BUILD,NULL);

			/* Whilst the tuple is valid */
			while (ct!=NULL) {
				counter=0;
				
				/* Whilst the current attribute datum is ok */
				while (counter<ct->noattributes) {
					/* Copy the data from the source tuple into the dest. tuple */	
					strcpy( tuple_d(nt,counter),tuple_d(ct,counter));	

					/* Increment the counter */
					counter++;
				}
				
				/* Convert the tuple to a string - TODO - Order by attributes */
				tuple_to_string(nt,s);

				/* Insert the data into our hashing table */
				hashing_insert(htable,s,REQ_CALC);

				/* Read the next tuple from the disk */
				(void) tuple_readnext(&ct,TUPLE_BUILD);
			}

			/* Move onto the second relation, and write any
			 * tuples IN the first hash table (so its in both relations).
			 *
			 * TODO - Check that the attributes are in the same order 
		 	 * this is done in Pascal version with tuple_to_string_order_by_relation
			 * or such like, where the tuple is converted to the string in the
			 * order of a specified relation known to be union compatible
			 */
			ct=tuple_readfirst(rel2,TUPLE_BUILD,NULL);
		
			while (ct!=NULL) {
				counter=0;
		
				/* Convert the tuple to a string
				 * TODO - Order by relation's attributes 
				 */	
				tuple_to_string(ct,s);	

				/* Retrieve the entry in the hash table
				 * if it exists in the table, then its in
				 * both relations, so can be written to disl 
				 */	
				success=FALSE;
				hashing_retrieve(htable,s,result,&success);

				if (success) {
					/* Whilst the current attribute datum is ok */
					while (counter<ct->noattributes) {
						/* Copy the data from the source tuple into the dest. tuple */	
						strcpy( tuple_d(nt,counter),tuple_d(ct,counter));	

						/* Increment the counter */
						counter++;
					}

					/* Write the tuple to disk */
					(void) tuple_append(nt);
				}

				/* Load the next tuple from the second relation */	
				(void) tuple_readnext(&ct,TUPLE_BUILD);
			}

			/* Dispose of the output tuple */
			close_tuple(&nt,TUPLE_DISPOSE);

			/* Terminate the hashing table */
			hashing_terminate(&htable);			
		} else {
			/* No relation was created */
			return(NULL);
		}
	} else {
		rtrel=NULL;
		raise_error(ERROR_UNION_COMPATIBILITY,NONFATAL,"Union of %s and %s.",relation_name(rel1),relation_name(rel2));
	}

	return(rtrel);
}


relation rl_difference( database db,
		       relation rel1, 
		       relation rel2,
		       char *destname) {
/* rl_difference
 * Difference operator - based extensively on intersect.
 * TODO (ONE DAY) - Combine difference and intersect. They're so
 * damn similair...
 */
	relation rtrel;
	char s[MAXIMUM_ALL_DATA],result[MAXIMUM_ALL_DATA];
	boolean success;
	word counter;
	HashTable htable;
	tuple ct,nt;

	do_debug(DEBUG_ENTER,"OPERATOR Start: rl_difference\n");

	/* Check that the source relations are union compatible */
	if ( rl_is_union_compatible(rel1,rel2) ) {

		rtrel=create_duplicate_rtn_relation(db,rel2,destname);

		/* If a relation has been returned */
		if (rtrel!=NULL) {
			/* Create an empty tuple based on the new relation */
			nt=tuple_prepare(rtrel);

			/* Create the hash table */
			htable=hashing_create();

			/* Read the first tuple from the SECOND relation */
			ct=tuple_readfirst(rel2,TUPLE_BUILD,NULL);

			/* Whilst the tuple is valid */
			while (ct!=NULL) {
				counter=0;
				
				/* Whilst the current attribute datum is ok */
				while (counter<ct->noattributes) {
					/* Copy the data from the source tuple into the dest. tuple */	
					strcpy( tuple_d(nt,counter),tuple_d(ct,counter));	

					/* Increment the counter */
					counter++;
				}
				
				/* Convert the tuple to a string - TODO - Order by attributes */
				tuple_to_string(nt,s);

				/* Insert the data into our hashing table */
				hashing_insert(htable,s,REQ_CALC);

				/* Read the next tuple from the disk */
				(void) tuple_readnext(&ct,TUPLE_BUILD);
			}

			/* Move onto the FIRST relation, and write any
			 * uniques to disk.
			 *
			 * TODO - Check that the attributes are in the same order 
		 	 * this is done in Pascal version with tuple_to_string_order_by_relation
			 * or such like, where the tuple is converted to the string in the
			 * order of a specified relation known to be union compatible
			 */
			ct=tuple_readfirst(rel1,TUPLE_BUILD,NULL);
		
			while (ct!=NULL) {
				counter=0;
		
				/* Convert the tuple to a string
				 * TODO - Order by relation's attributes 
				 */	
				tuple_to_string(ct,s);	

				/* Retrieve the entry in the hash table
				 * if it DOESN'T exist in the table, then its in
				 * the second, but not the first
				 */	
				success=FALSE;
				hashing_retrieve(htable,s,result,&success);

				if ((status_debug)&&(status_debuglevel>=DEBUG_INFO)) {
					if (success) {
						leap_printf("[%s] is already present\n",s);
					} else {
						leap_printf("[%s] is NOT present\n",s);
					}
				}

				if (!success) {
					/* Whilst the current attribute datum is ok */
					while (counter<ct->noattributes) {
						/* Copy the data from the source tuple into the dest. tuple */	
						strcpy( tuple_d(nt,counter),tuple_d(ct,counter));	

						/* Increment the counter */
						counter++;
					}

					/* Write the tuple to disk */
					(void) tuple_append(nt);
				}

				/* Load the next tuple from the second relation */	
				(void) tuple_readnext(&ct,TUPLE_BUILD);
			}

			/* Dispose of the output tuple */
			close_tuple(&nt,TUPLE_DISPOSE);

			/* Terminate the hashing table */
			hashing_terminate(&htable);			
		} else {
			do_debug(DEBUG_ENTER,"OPERATOR End (ERROR): rl_difference\n");
			/* No relation was created */
			return(NULL);
		}
	} else {
		rtrel=NULL;
		raise_error(ERROR_UNION_COMPATIBILITY,NONFATAL,"Union of %s and %s.",relation_name(rel1),relation_name(rel2));
	}
	do_debug(DEBUG_ENTER,"OPERATOR End: rl_difference\n");
	return(rtrel);
}

relation rl_display(relation rel) {
/* rl_display
 * Display the tuples in a relation
 */
	tuple ntuple,bntuple;

	do_debug(DEBUG_ENTER,"OPERATOR start: rl_display\n");

	if (rel!=NULL) {
		/* Load the first tuple */
		ntuple=tuple_readfirst(rel,TUPLE_BUILD,NULL);

		if (ntuple!=NULL) {
			do_debug(DEBUG_INFO,"display: Data present - Using tuple struct for attribs.\n");
			bntuple=ntuple;

			attributes_printtuple(ntuple);

			/* Whilst the tuple is valid */
			while (ntuple!=NULL) {
		
				/* Print out the information */
				tuple_print(ntuple);
	
				/* Fetch the next tuple */
				(void) tuple_readnext(&ntuple,TUPLE_REUSE);
			}

			close_tuple(&bntuple,TUPLE_DISPOSE);
		} else {
			do_debug(DEBUG_INFO,"display: NO Data present - Building tuple from relation.\n");
			attributes_printfromrel(rel);
		}
	}	

	do_debug(DEBUG_ENTER,"OPERATOR End: rl_display\n");

	/* Return the relation, so no garbled output on "Relation Returned" */
	return(rel);
}

relation rl_revdisplay(relation rel) {
/* rl_display
 * Display the tuples in a relation for reverse engineering.
 */
    tuple ntuple,bntuple;

    if (rel!=NULL) {
        /* Load the first tuple */
        ntuple=tuple_readfirst(rel,TUPLE_BUILD,NULL);
        bntuple=ntuple;

        /* Whilst the tuple is valid */
        while (ntuple!=NULL) {
    
            /* Print out the information */
            tuple_revprint(rel,ntuple);
    
            /* Fetch the next tuple */
            (void) tuple_readnext(&ntuple,TUPLE_REUSE);
        }

        close_tuple(&bntuple,TUPLE_DISPOSE);
    }

    /* Return the relation, so no garbled output on "Relation Returned" */
    return(rel);
}


/* This is a structure I used, because I got annoyed with 
 * C arrays and couldn't work it out for some reason. I wanted
 * to have an array of strings, but things weren't working quite
 * correctly...
 * TODO - Implement array of strings.
 */
struct attribute_list {
	char attribute[ATTRIBUTE_NAME_SIZE+1];
};

boolean attribute_used(struct attribute_list attrib_list[MAXIMUM_ATTRIBUTES],
			attribute attrib) {
/* attribute_used
 * Returns TRUE If the attribute specified in attrib is located in
 * the "string" array.
 */
	word counter;

	/* Reset the counter */
	counter=0;

	/* Whilst the string is not found in the array, and we're within the array bounds */
	while ( (strcmp(attrib_list[counter].attribute,attribute_name(attrib))!=0) 
	     && (counter<MAXIMUM_ATTRIBUTES) ) {
		counter++;
	}

	/* If the string is located, ie. counter isn't outside of the array */
	if ( (strlen(attrib_list[counter].attribute)!=0) && (counter<MAXIMUM_ATTRIBUTES) ) {

		/* We've found the attribute */
		return(TRUE);
	} else {
		
		/* We haven't found the attribute */
		return(FALSE);
	}
		
}

relation relation_create_both(database db,
				relation rel1, relation rel2,
				char *name, char *skip_list) {
/* relation_create_both
 * Creates a new relation that contains the attributes from two
 * source relations - it is used by product & join 
 */
	char *rname,rel_name[RELATION_NAME_SIZE+1],path[FILE_PATH_SIZE],attrdescr[MAX_DTS_SIZE+1];
	relation rtrel;
	struct attribute_list attrib_names[MAXIMUM_ATTRIBUTES];
	char *sptr,tmp[MAXIMUM_ATTRIBUTES];
	char temp[100];
	word counter;
	attribute cattr;
	boolean exceeded_limit=FALSE;
	tuple ctuple;
	word anum;
	FILE *nf;


	/* If the caller has not provided a relation name, then
	 * generate one. TODO - Is this really necessary? Prerequisite
	 * could be that a name is provided that doesn't exist...
	 */

	do_debug(DEBUG_ENTER,"ENTER rl_relation_create_both\n");

	/* Check that the parameter relations are valid */
	if ( (rel1!=NULL) && (rel2!=NULL) ) {

		if (strlen(name)==0) {
			rname=generate_random_string(RANDOM_NAME_SIZE,rel_name);
		} else {
			rname=name;
		}	

		/* Create a temporary relation with the specified name */
		rtrel=relation_create(db,rname,TRUE,FALSE);	

		/* Check all is ok  */
		if (rtrel!=NULL) {
			/* Reset the number of attributes */
			rtrel->noattributes=0;

			sprintf(path,"%s%s%s",rtrel->filepath,rname,LEAP_NEW_RELATION_EXT);
	
			do_debug(DEBUG_INFO,"Opening %s\n",path);
			nf=fopen(path,"a+b");

			if ((status_debug)&&(status_debuglevel>=DEBUG_INFO)) {
				if (nf!=NULL) {
            				leap_fprintf(stdout,"Opened >%s<\n",path);
	        		} else {
        	    			leap_fprintf(stdout,"FAILED to open >%s<\n",path);
        			}
			}

			/* Reset the array */
			for (counter=0;counter<MAXIMUM_ATTRIBUTES;counter++) {
				strcpy(attrib_names[counter].attribute,"");
			}

			strcpy(tmp,skip_list);

			/* Reset the counter for the name array */	
			counter=0;

			/* Locate the first attribute in the relation */
			cattr=relation_attribute_readfirst(rel1, &ctuple,  &anum);

			/* Whilst we have a valid attribute */	
			while ( (cattr!=NULL) && (exceeded_limit==FALSE) ){

				/* Check that the attribute is not to be skipped. */
				sptr=strstr(tmp,attribute_name(cattr));
				if (sptr==NULL) {

					/* Put a ptr to the attribute name into the name array */
					strcpy(attrib_names[counter].attribute,attribute_name(cattr));
	
					/* Increment the counter */
					counter++;

					if (status_padding==TRUE) {
						sprintf(temp,"%s.%s",relation_name(rel1),attribute_name(cattr));
					} else {
						strcpy(temp,attribute_name(cattr));
					}
					strcpy(attribute_name(cattr),temp);

					/* Create the attribute in the destination relation */
					fseek(nf,0,SEEK_END);
					relation_create_write_attribute(cattr,&nf);

           			if (database_datadictionary(db)>=NO_DD_RELATIONS) {
                		if ( (status_tempdb==TRUE) && (configuration!=TRUE) && (strcmp(database_name(db),TEMPDB_NAME)!=0) && (rtrel->name[0]=='z') && (rtrel->name[1]=='z')) {

                    		ddmaintenance(tempdb,"add (%s) (%s,%s,%s,%d)",LEAP_DD_ATTRIBUTES,relation_name(rtrel),
                                                           attribute_name(cattr), get_attribute_info( attrdescr, attribute_type( cattr)),
                                                           attribute_size(cattr));
                		} else {
                    		ddmaintenance(db,"add (%s) (%s,%s,%s,%d)",LEAP_DD_ATTRIBUTES,relation_name(rtrel),
                                                           	attribute_name(cattr),  get_attribute_info( attrdescr, attribute_type( cattr)),
                                                            attribute_size(cattr));
                		}
            		}


					/* Check that we are not now at the maximum number of attributes
				 	* permitted in a relation 
				 	*/
					if (counter==MAXIMUM_ATTRIBUTES) {
	
						/* Report an error */
						raise_error(ERROR_EXCEEDED_ATTRIBUTE_LIMIT,NONFATAL,"Current limit set to %d attributes",MAXIMUM_ATTRIBUTES);

						/* Dispose of the attribute to abort the while loop */
						attribute_dispose(&cattr);

						/* Set a flag so that the next section isn't executed. */
						exceeded_limit=TRUE;

					}
				} else {
					/* Strike out the first char of the string to
					 * prevent further matches being made, ie. incase
					 * foreign key has the same name (quite often!)
					 */
					(*sptr)='_';
				}
				/* Locate the next attribute */
				cattr=relation_attribute_readnext(NULL,&ctuple,cattr,&anum);
			}	

			/* Aha! Gotcha! This meant that a tuple ptr wasn't entirely disposed of
			 * between relation attribute sweeps
 			 */
			close_tuple(&ctuple,TUPLE_DISPOSE);

			if (exceeded_limit==FALSE) {
			
				cattr=relation_attribute_readfirst(rel2, &ctuple, &anum);

				while (cattr!=NULL) {
					
					if ( (attribute_used(attrib_names,cattr)) || (status_padding==TRUE) ) {
						sprintf(attrib_names[counter].attribute,"%s.%s",relation_name(rel2),attribute_name(cattr));
					} else {
						/* Put a ptr to the attribute name into the name array */
						strcpy(attrib_names[counter].attribute,attribute_name(cattr));
					}
					
					strcpy(attribute_name(cattr),attrib_names[counter].attribute);

					fseek(nf,0,SEEK_END);
					relation_create_write_attribute(cattr,&nf);

					counter++;

					/* Check that we are not now at the maximum number of attributes
				 	* permitted in a relation 
				 	*/
					if (counter==MAXIMUM_ATTRIBUTES) {
	
						/* Report an error */
						raise_error(ERROR_EXCEEDED_ATTRIBUTE_LIMIT,NONFATAL,"Current limit set to %d attributes.",MAXIMUM_ATTRIBUTES);

						/* Dispose of the attribute to abort the while loop */
						attribute_dispose(&cattr);
	
						/* Set a flag so that the next section isn't executed. */
						exceeded_limit=TRUE;

					} else {

						/* Locate the next attribute */
						cattr=relation_attribute_readnext(NULL,&ctuple,cattr,&anum);
					}
				}

				/* This is all a bit crap, but it was developed when there were multiple
				 * file handles per relation file... needs an urgent rewrite when handles
				 * are cleaned up.
				 */
				fclose(nf);
				nf=fopen(path,"r+b");
				relation_update_header(db, rname,counter,rtrel->temporary,rtrel->system,&nf);
				fclose(nf);
				nf=fopen(path,"r+b");
				fseek(nf,0,SEEK_END);
				fflush(nf);
				relation_create_write_eoh_marker(counter,&nf);
				fclose(nf);
			}
			
			close_tuple(&ctuple,TUPLE_DISPOSE);

		}
		/* "else rtrel==NULL, so break out... */
	} else {
		do_debug(DEBUG_ENTER,"EXITing UNsuccessfully relation_create_both\n");
		/* Why dat waskally wabbit... One of our pawameter welations was scwewy... */
		rtrel=NULL;	
	}

	do_debug(DEBUG_ENTER,"EXITing successfully relation_create_both\n");
	return (rtrel);
}

relation rl_product(database db,
			relation rel1, relation rel2,
			char *destname) {
/* rl_product
 * Produces the cartesian product of the two specified relations
 */
	char *rname;
	relation return_rel;
	tuple ct1,ct2,nt,pct1,pct2;
	word count;
	char rel_name[RELATION_NAME_SIZE+1];

	do_debug(DEBUG_ENTER,"OPERATOR Start: rl_product\n");

	if (strlen(destname)==0) {
		rname=generate_random_string(RANDOM_NAME_SIZE,rel_name);
	} else {
		rname=destname;
	}	

	/* Create the return relation */
	return_rel=relation_create_both(db,rel1,rel2,rname,"");

	/* If we have a valid relation! */
	if (return_rel!=NULL) {
		/* Prepare the output tuple */
		nt=tuple_prepare(return_rel);

		/* Read the first tuple */
		ct1=tuple_readfirst(rel1,TUPLE_BUILD,NULL);
		pct1=ct1;
		ct2=NULL;

		/* Build a structure and store ptr that is not involved in ops */
		pct2=tuple_readfirst(rel2,TUPLE_BUILD,NULL);

		/* Whilst the tuple is valid */
		while (ct1!=NULL) {

			/* Read the first tuple of the first relation */
			ct2=tuple_readfirst(rel2,TUPLE_REUSE,pct2);

			/* Whilst the tuple is valid */
			while (ct2!=NULL) {

				count=0;
				while (count<ct1->noattributes) {
					strcpy( tuple_d(nt,count),tuple_d(ct1,count) );
					count++;
				}
				count=0;
				while (count<ct2->noattributes) {
					strcpy( tuple_d(nt,count+ct1->noattributes),tuple_d(ct2,count) );
					count++;
				}

				(void) tuple_append(nt);
		
				(void) tuple_readnext(&ct2,TUPLE_REUSE);
			}	

			close_tuple(&ct2,TUPLE_REUSE);
			
			(void) tuple_readnext(&ct1,TUPLE_REUSE);
		}
		close_tuple(&pct1,TUPLE_DISPOSE);
		close_tuple(&pct2,TUPLE_DISPOSE);
		close_tuple(&ct2,TUPLE_DISPOSE);
		close_tuple(&nt,TUPLE_DISPOSE);
	}

	do_debug(DEBUG_ENTER,"OPERATOR End: rl_product\n");
	return(return_rel);
}

relation rl_select( database db,
			relation rel,
			char *qualification,
			char *dest) {
/* rl_select
 * The relational select/restrict operator
 */
	relation rtrel;
	tuple ct,nt; /* Current tuple, and new tuple */
	condp condition;
	int counter;
	char spare_qualification[MAXIMUM_EXPRESSION]; /* Incase a qual. is not provided! */


	do_debug(DEBUG_ENTER,"OPERATOR Start: rl_select\n");

	do_debug(DEBUG_INFO,"Qualification: %s\nDestination: %s\n",qualification,dest);
	if (strlen(qualification)==0) {
		raise_error(ERROR_NO_CONDITION,NONFATAL,"No condition specified. Defaulting to an always TRUE condition.");
		strcpy(spare_qualification,special_condition_true);
		qualification=spare_qualification;
	}

	/* Create the result relation */
	rtrel=create_duplicate_rtn_relation(db,rel,dest);

	/* Check that a relation was created */
	if ( (rtrel!=NULL) && (rel!=NULL) ) {
		/* Relation is ok, and new relation is ok */

		/* Create our "input" tuple */
		ct=tuple_readfirst(rel, TUPLE_BUILD,NULL);

		if (ct!=NULL) {
				/* Create the "output" tuple */
				nt=tuple_prepare(rtrel);

				/* Build the condition structure */
				condition=build_condition(qualification,ct,NULL);

				if (condition!=NULL) {

					/* If debug output is required */
					if ((status_debug)&&(status_debuglevel>=DEBUG_INFO)) {
						/* Display the condition */
						print_condition(condition);	
					}
			
					/* Process the "input" tuples */
					while (ct!=NULL) {

						/* Evaluate the condition on the current tuple */
						if (evaluate( condition, ct, NULL )==TRUE) {
							do_debug(DEBUG_ACK,"Tuple *WILL* be written to disk\n");

							/* Reset the counter */
							counter=0;

							/* Whilst the current attribute datum is ok */
							while (counter<ct->noattributes) {
								/* Copy the data from the source tuple into the dest. tuple */	
								strcpy( tuple_d(nt,counter),tuple_d(ct,counter));	

								/* Increment the counter */
								counter++;
							}

							/* TODO - handle return code of write_tuple */
							tuple_append(nt);
						} else {
							do_debug(DEBUG_ACK,"Tuple will **NOT** be written to disk\n");
						}

						/* Read the next tuple */
						(void) tuple_readnext(&ct,TUPLE_BUILD);
					}

					destroy_condition(&condition);

					close_tuple(&nt,TUPLE_DISPOSE);
					close_tuple(&ct,TUPLE_DISPOSE);

					do_debug(DEBUG_ENTER,"OPERATOR End: rl_select\n");
					/* Return the resulting relation */
					return(rtrel);
				} else {
					close_tuple(&nt,TUPLE_DISPOSE);

					do_debug(DEBUG_ENTER,"OPERATOR End Err: rl_select\n");
					/* Condition was messed up for some reason */
					return(NULL);
				}	
		} else {
			do_debug(DEBUG_ENTER,"OPERATOR End Err: rl_select\n");
			/* Relation was empty */
			return(NULL);
		}
	} else {
		do_debug(DEBUG_ENTER,"OPERATOR End Err: rl_select\n");
		/* Relation created was null */
		/* Error is "handled" will be printed in above func. */
		return(NULL);
	}

}

relation rl_erase( database db,
			relation rel,
			char *qualification,
			char *dest) {
/* rl_erase
 * The delete tuple operator
 * (rl_delete is used in the readline library... Doh!)
 */
	tuple ct; /* Current tuple, and new tuple */
	condp condition;
	char spare_qualification[MAXIMUM_EXPRESSION]; /* Incase a qual. is not provided! */


	do_debug(DEBUG_ENTER,"OPERATOR Start: rl_erase\n");
	do_debug(DEBUG_INFO,"Qualification: %s\nDestination: %s\n",qualification,dest);

	if (strlen(qualification)==0) {
		raise_error(ERROR_NO_CONDITION,NONFATAL,"No condition specified. Defaulting to an always TRUE condition.");
		strcpy(spare_qualification,special_condition_true);
		qualification=spare_qualification;
	}

	if (rel!=NULL) {
		/* Relation is ok */

		/* Create our "input" tuple */
		ct=tuple_readfirst(rel, TUPLE_BUILD,NULL);

		/* Build the condition structure */
		condition=build_condition(qualification,ct,NULL);

		if (condition!=NULL) {

			/* If debug output is required */
			if ((status_debug)&&(status_debuglevel>=DEBUG_INFO)) {
				/* Display the condition */
				print_condition(condition);	
			}
	
			/* Process the "input" tuples */
			while (ct!=NULL) {

				/* Evaluate the condition on the current tuple */
				if (evaluate( condition, ct, NULL )==TRUE) {
				
					/* Delete the tuple */
					tuple_delete( ct->offset, ct);
					
				}

				/* Read the next tuple */
				(void) tuple_readnext(&ct,TUPLE_BUILD);
			}

			destroy_condition(&condition);
			close_tuple(&ct,TUPLE_DISPOSE);

			/* Return the resulting relation */
			do_debug(DEBUG_ENTER,"OPERATOR End : rl_erase\n");
			return(rel);
		} else {
			do_debug(DEBUG_ENTER,"OPERATOR End Err: rl_erase\n");
			/* Condition was messed up for some reason */

			destroy_condition(&condition);
			close_tuple(&ct,TUPLE_DISPOSE);

			return(NULL);
		}	
	} else {
		do_debug(DEBUG_ENTER,"OPERATOR End Err: rl_erase\n");
		/* Relation created was null */
		/* Error is "handled" will be printed in above func. */
		return(NULL);
	}

}

relation rl_update( database db,
		    relation rel,
		    char *qualification,
		    char *set,
		    char *dest) {
/* rl_update
 * essentially the select operator to identify the
 * tuple, but then some balls
 */
	relation rtrel;
	tuple ct; /* Current tuple, and new tuple */
	condp condition;
	attribute attrib;
	char spare_qualification[MAXIMUM_EXPRESSION+1]; /* Incase a qual. is not provided! */
	char *nvalue,*key,*value,*assign,string[MAXIMUM_EXPRESSION+1],setm[MAXIMUM_EXPRESSION+1];
	char expression[MAXIMUM_EXPRESSION+1];
	word epos;

	do_debug(DEBUG_ENTER,"OPERATOR Start: rl_update\n");

	do_debug(DEBUG_INFO,"Qualification: %s\nDestination:  %s\nSet string:   %s\n",qualification,dest,set);

	if (strlen(qualification)==0) {
		raise_error(ERROR_NO_CONDITION,NONFATAL,"No condition specified. Defaulting to an always TRUE condition.");
		strcpy(spare_qualification,special_condition_true);
		qualification=spare_qualification;
	}

	if (strlen(set)==0) {
		raise_error(ERROR_EVALUATING_EXPR,NONFATAL,"Invalid set operation");
		do_debug(DEBUG_ENTER,"OPERATOR End Err: rl_update\n");
		return(NULL);
	}

	/* Copy the result relation */
	rtrel=rel;

	/* Check that a relation was created */
	if ( (rtrel!=NULL) && (rel!=NULL) ) {
		/* Relation is ok, and new relation is ok */

		/* Create our "input" tuple */
		ct=tuple_readfirst(rel, TUPLE_BUILD,NULL);

		if (ct!=NULL) {
				epos=getendposition(ct);

				/* Build the condition structure */
				condition=build_condition(qualification,ct,NULL);

				if (condition!=NULL) {

					/* If debug output is required */
					if ((status_debug)&&(status_debuglevel>=DEBUG_INFO)) {
						/* Display the condition */
						print_condition(condition);	
					}
			
					/* Backup the set command */
					strcpy(setm,set);

					/* Process the "input" tuple */
					while ((ct!=NULL) && (atend(ct,epos)!=TRUE)) {

						/* Reset the set command */
						strcpy(set,setm);

						/* Evaluate the condition on the current tuple */
						if (evaluate( condition, ct, NULL )==TRUE) {
				
							if (status_debuglevel>=DEBUG_MODERATE) {
									do_debug(DEBUG_MODERATE,"PRE-Update tuple:");
									if (ct!=NULL) {
										attributes_printtuple(ct);
										tuple_print(ct);
									}
							}
							do_debug(DEBUG_INFO,"Tuple to be updated **FOUND**\n");
							tuple_delete( ct->offset, ct);

							cut_token(set,',',string);

							/* Anything found? */
							if (strlen(string)==0) {
								/* No? Then there's only one set expr. */
								strcpy(string,set);
							}

							while (strlen(string)>0) {
								/* process ct to set fields */
								key=string;
								assign=strchr(string,'=');
		
								if (assign==NULL) {
									raise_error(ERROR_EVALUATING_EXPR,NONFATAL,"Invalid set operation");
									raise_event(MESSAGE,"Update not disposing of memory");
									destroy_condition(&condition);
									close_tuple(&ct,TUPLE_DISPOSE);

									return(NULL);
								}

								*assign='\0';

								value=assign+1;

								nvalue=NULL;
								nvalue=allbut(value,"'\"");

								/* Get the old value */
								attrib=attribute_find(rel,string);

								if (attrib!=NULL) {
									do_debug(DEBUG_MODERATE,"Old val: [%s] - New val: [%s]\n",tuple_d(ct,attrib->no),nvalue);
									strcpy(tuple_d(ct,attrib->no),nvalue);
									attribute_dispose(&attrib);
								} else {
									raise_error(ERROR_CANTFIND_ATTR,NONFATAL,string);
								}

								if (nvalue!=NULL) {
									free(nvalue);
								}

								cut_token(set,',',string);
								do_debug(DEBUG_MODERATE,"Length of next update (set) statement: [%d]\n",strlen(string)); 
	
							}

							/* Append the tuple, but return the file ptr to the
 							 * original location
							 */
							tuple_appendandreturn(ct);
							do_debug(DEBUG_INFO,"Tuple ***UPDATED***\n");
							if (status_debuglevel>=DEBUG_MODERATE) {
									do_debug(DEBUG_MODERATE,"POST-Update tuple:");
									if (ct!=NULL) {
										attributes_printtuple(ct);
										tuple_print(ct);
									}
							}

						} else {
							do_debug(DEBUG_INFO,"Tuple will **NOT** be updated\n");
						}

						/* Read the next tuple */
						(void) tuple_readnext(&ct,TUPLE_BUILD);
					}

					destroy_condition(&condition);

					close_tuple(&ct,TUPLE_DISPOSE);

					do_debug(DEBUG_ENTER,"OPERATOR End : rl_update\n");

					if (strcmp(relation_name(rel),LEAP_DD_RELATIONS)!=0) {
							do_debug(DEBUG_INFO,"Updating leaprel info");
							sprintf(expression,"update (%s) (%s='%s') (%s='%s')",
								LEAP_DD_RELATIONS, LEAP_DDA_RELATIONS_NAME, relation_name(rel),
								LEAP_DDA_RELATIONS_UPDATED,TRUE_STRING);
							ddmaintenance(db,expression);
					} else {
							do_debug(DEBUG_INFO,"No updating info on leaprel - causes recursion!");
					}

					/* Return the resulting relation */
					return(rtrel);
				} else {
					do_debug(DEBUG_ENTER,"OPERATOR End Err: rl_update\n");

					/* Condition was messed up for some reason */
					return(NULL);
				}	
		} else {
			do_debug(DEBUG_ENTER,"OPERATOR End Err: rl_update\n");

			/* Relation was empty */
			return(NULL);
		}
	} else {
		do_debug(DEBUG_ENTER,"OPERATOR End Err: rl_update\n");

		/* Relation created was null */
		/* Error is "handled" will be printed in above func. */
		return(NULL);
	}

}

relation rl_naturaljoin( database db,
				relation lrel, relation rrel,
				char *qualification,
				char *destname,
				int join_type) {
/* rl_naturaljoin
 * The new join operator. 
 * Returns NULL if fails, or ptr to a new relation 
 */

	relation rtrel;
	char *rname;
	char rel_name[RELATION_NAME_SIZE+1];
	tuple ltuple, rtuple, ntuple, prtuple, pltuple;
	condp condition=NULL;
	char spare_qualification[MAXIMUM_EXPRESSION+1]; /* Incase a qual. is not provided! */
	char generated_qualification[MAXIMUM_EXPRESSION+1]="";
	int lresult,rresult,count,position,internal_jointype=UNKNOWN_JOIN;
	int qualification_length;
	char primary_keys[MAXIMUM_EXPRESSION+1],foreign_keys[MAXIMUM_EXPRESSION+1];
	char spare_foreign_keys[MAXIMUM_EXPRESSION+1];
	char spare_primary_keys[MAXIMUM_EXPRESSION+1];
	char temp[100];
	char pkey[ATTRIBUTE_NAME_SIZE+1],fkey[ATTRIBUTE_NAME_SIZE+1];
	boolean finished=FALSE;
	int skip_list[MAXIMUM_ATTRIBUTES],noskippable=0;
	attribute att;


	do_debug(DEBUG_ENTER,"OPERATOR Start: rl_naturaljoin\n");
	if ( (lrel!=NULL) && (rrel!=NULL) ) {

		if (join_type!=UNKNOWN_JOIN) 
			internal_jointype=join_type;

		qualification_length=strlen(qualification);

		if ((internal_jointype==NATURAL_JOIN) || (qualification_length==0)) {

			if (qualification_length==0) {
				(void) get_keys( db, lrel, rrel, foreign_keys, primary_keys);	
				if ( (strlen(primary_keys)==0) && (strlen(foreign_keys)==0) ) {

					if ((status_trace)&&(status_debug)) raise_message(MESSAGE,"DD Lookup: Relationship not found. Reversing");

					(void) get_keys( db, rrel, lrel, primary_keys, foreign_keys);

					/* Does this return us a relationship? */

					if ( (strlen(primary_keys)==0) && (strlen(foreign_keys)==0) ) {

						if ( (status_debug) || (status_trace) ) {
							raise_message(MESSAGE,"DD Lookup: No entry found in relship.\n");
						}	

						raise_message(MESSAGE,"No join condition can be determined from command line or %s relation.",LEAP_DD_RSHIP);

						if (status_productjoin==TRUE) {
							raise_message(MESSAGE,"Assuming always TRUE condition.");
							strcpy(qualification,"(TRUE=TRUE)");
							return(rl_naturaljoin(db,lrel,rrel,qualification,destname,UNKNOWN_JOIN));
						}
						return(NULL);
					}

				}

				strcpy(spare_foreign_keys,foreign_keys);
				strcpy(spare_primary_keys,primary_keys);

				if ((status_debug)&&(status_debuglevel>=DEBUG_INFO)) {
					do_trace("Primary/Foreign keys:");
					do_trace(primary_keys);
					do_trace(foreign_keys);
				}
	
				cut_token(primary_keys,'\0',pkey);
				cut_token(foreign_keys,'\0',fkey);
				sprintf(temp,"(%s.%s=%s.%s)",relation_name(lrel),fkey,relation_name(rrel),pkey);

				strcat(generated_qualification,temp);

				finished=FALSE;	
				do {

					cut_token(primary_keys,'\0',pkey);
					cut_token(foreign_keys,'\0',fkey);

					if ( (strlen(pkey)>0) && (strlen(fkey)>0) ) {
						sprintf(temp," and (%s.%s=%s.%s)",relation_name(lrel),fkey,relation_name(rrel),pkey);
						strcat(generated_qualification,temp);
					} else {
						finished=TRUE;
					}
					do_debug(DEBUG_INFO,"Qualification: %s\n",temp);

				} while (finished!=TRUE);

				qualification=generated_qualification;
			}


			internal_jointype=NATURAL_JOIN;

		} else {
			/* We want all keys to appear - it's an equi-join */
			strcpy(spare_foreign_keys,"");
			strcpy(spare_primary_keys,"");

			internal_jointype=THETA_JOIN;

		}

		do_debug(DEBUG_INFO,"Final Qualification: %s\n",qualification);

		/* Generate a relation name if one is not provided */
		if (strlen(destname)==0) {
			rname=generate_random_string(RANDOM_NAME_SIZE,rel_name);
		} else {
			rname=destname;
		}	

		/* Create the relation */
		rtrel=relation_create_both( db, lrel, rrel, rname, spare_foreign_keys );
		
		/* Check the returned relation */
		if (rtrel!=NULL) {

			if (internal_jointype==NATURAL_JOIN) {
  				/* Reset the array */
				for (noskippable=MAXIMUM_ATTRIBUTES;noskippable>=0;noskippable--){
					skip_list[noskippable]=-1;
				}

				/* Build the array of skippable items in first relation */
				cut_token(spare_foreign_keys,'\0',temp);
				noskippable=0;
				while (strlen(temp)>0) {
					att=attribute_find(lrel, temp);
					if (att!=NULL) {
	  					skip_list[noskippable]=att->no;
						noskippable++;
						cut_token(spare_foreign_keys,'\0',temp);
						attribute_dispose(&att);
					} else {
						raise_error(ERROR_CANTFIND_ATTR,NONFATAL,temp);
						return(NULL);
					}
				}
			}


			/* Build the default condition if appropriate */
			if (strlen(qualification)==0)  {
				raise_error(ERROR_NO_CONDITION,NONFATAL,"No condition specified. Defaulting to an always TRUE condition.");
				strcpy(spare_qualification,special_condition_true);
				qualification=spare_qualification;
			}

			/* Open the tuple in left source relation */
			ltuple=tuple_readfirst(lrel, TUPLE_BUILD,NULL);
			pltuple=ltuple;
			do_debug(DEBUG_MODERATE,"pltuple [%p]",pltuple);

			/* Open the right source relation */
			rtuple=tuple_readfirst(rrel, TUPLE_BUILD,NULL);
			prtuple=rtuple; 

			/* Create the "output" tuple */
			ntuple=tuple_prepare(rtrel);

			if ((ntuple!=NULL) && (rtuple!=NULL) && (ltuple!=NULL)) {

					/* Build the condition structure */
					condition=build_condition(qualification,ltuple,rtuple);
	
					/* Check a valid condition structure was built */
					if (condition!=NULL) {

						/* Close the file, as we use this ptr again... */
						/* TODO: Function */
						lresult=0;
			
						/* Whilst there is a valid tuple in the left */
						while ( (condition!=NULL) && (ltuple!=NULL) ) {

							/* Open the tuple in the right source relation */
							/* Note that the tuple must be REUSED, not BUILT, as this
							* Would make the ptrs in the condition structure invald 
							*/
							rtuple=tuple_readfirst(rrel, TUPLE_REUSE,prtuple);
							/*prtuple=rtuple;*/
							rresult=0;
				
							/* Whilst there is a valid tuple in the right */
							while ( (rtuple!=NULL) ) {
			
			
								if ((status_debug)&&(status_debuglevel>=DEBUG_INFO)) {
									leap_fprintf(stderr,"Left :");
									tuple_print(ltuple);
									leap_fprintf(stderr,"Right:");
									tuple_print(rtuple);
								}

							
								/* Evaluate the condition on the current tuple */
								if (evaluate( condition, ltuple, rtuple )==TRUE) {

									do_debug(DEBUG_INFO,"Joined tuple WILL be written to disk\n");
									count=0;
									if (internal_jointype==NATURAL_JOIN) {
										position=0;
										while (count<ltuple->noattributes) {
											if (skip_list[position]==count) {
												position++;
											} else {
												strcpy( tuple_d(ntuple,count-position),tuple_d(ltuple,count) );
											}
											if ((status_debug)&&(status_debuglevel>=DEBUG_INFO)) {
												tuple_def(ntuple);
											}
											count++;
										}
									} else {
										position=0;
										while (count<ltuple->noattributes) {
											strcpy( tuple_d(ntuple,count),tuple_d(ltuple,count) );
											count++;
										}
									}
									if ((status_debug)&&(status_debuglevel>=DEBUG_INFO)) {
										tuple_def(ntuple);
									}
									count=0;
									while (count<rtuple->noattributes) {
										strcpy( tuple_d(ntuple,count+(ltuple->noattributes)-position),tuple_d(rtuple,count) );
										count++;
									}
									if ((status_debug)&&(status_debuglevel>=DEBUG_INFO)) {
										tuple_def(ntuple);
									}
									tuple_append(ntuple);
								} else {

									do_debug(DEBUG_INFO,"Joined tuple will *NOT* be written to disk\n");
								}
								/* Whilst there is a valid tuple in the right */
								tuple_readnext(&rtuple,TUPLE_REUSE);
							}

							close_tuple(&rtuple,TUPLE_REUSE);

							/* Whilst there is a valid tuple in the left */
							tuple_readnext(&ltuple,TUPLE_REUSE);
						}	
						destroy_condition(&condition);
						do_debug(DEBUG_INFO,"pltuple [%p]",pltuple);
						close_tuple(&pltuple,TUPLE_DISPOSE);
						close_tuple(&ntuple,TUPLE_DISPOSE);
						close_tuple(&prtuple,TUPLE_DISPOSE);
					} else {
						do_debug(DEBUG_INFO,"Trouble with conditions\n");
						close_tuple(&ltuple,TUPLE_DISPOSE);
						close_tuple(&rtuple,TUPLE_DISPOSE);
						close_tuple(&ntuple,TUPLE_DISPOSE);
					}
				} else {
						do_debug(DEBUG_INFO,"Trouble allocating ltuple, rtuple or ntuple struct.\n");
						close_tuple(&ltuple,TUPLE_DISPOSE);
						close_tuple(&rtuple,TUPLE_DISPOSE);
						close_tuple(&ntuple,TUPLE_DISPOSE);
				}
			do_debug(DEBUG_ENTER,"OPERATOR End Ok: rl_naturaljoin\n");
			return(rtrel);
		} else {
			do_debug(DEBUG_ENTER,"OPERATOR End Err: rl_naturaljoin\n");
			return(NULL);	
		} 

	} else {
		do_debug(DEBUG_ENTER,"OPERATOR End: rl_naturaljoin\n");
		return(NULL);
	}
}

relation rl_duplicate( database db, 
			relation rel,
			char *destname) {
/* rl_duplicate
 * Creates a duplicate relation
 */
	relation rtrel;
	char *rname;
	char rel_name[RELATION_NAME_SIZE+1];
	tuple ct,nt; /* Current tuple, and new tuple */
	word counter;

	do_debug(DEBUG_ENTER,"OPERATOR Start: rl_duplicate\n");

	/* Generate a relation name if one is not provided */
	if (strlen(destname)==0) {
		rname=generate_random_string(RANDOM_NAME_SIZE,rel_name);
	} else {
		rname=destname;
	}	

	/* Create the relation */
	rtrel=create_duplicate_rtn_relation( db, rel, rname);

	/* Check that a relation was created */
	if ( (rtrel!=NULL) && (rel!=NULL) ) {
		/* Relation is ok, and new relation is ok */

		/* Create our "input" tuple */
		ct=tuple_readfirst(rel, TUPLE_BUILD,NULL);

		/* Create the "output" tuple */
		nt=tuple_prepare(rtrel);

		/* Process the "input" tuples */
		while (ct!=NULL) {

			counter=0;
			while (counter<ct->noattributes) {
				/* Copy the data from the source tuple into the dest. tuple */	
				strcpy( tuple_d(nt,counter),tuple_d(ct,counter));	

				/* Increment the counter */
				counter++;
			}

			(void) tuple_append(nt);

			/* Read the next tuple */
			(void) tuple_readnext(&ct,TUPLE_REUSE);
		}

	} else {
		do_debug(DEBUG_ENTER,"OPERATOR End Err: rl_duplicate\n");
		/* Relation created was null */
		return(NULL);
	}
	
	close_tuple(&nt,TUPLE_DISPOSE);

	do_debug(DEBUG_ENTER,"OPERATOR End : rl_duplicate\n");

	return(rtrel);
}


relation rl_compact( database db, relation *rel ) {
/* Compact a relation
 */
 
	relation nrel,rtrel;
	char targetname[RELATION_NAME_SIZE+1]="";
        NOATTRIBUTES_TYPE noattribs,nnoattribs;
	RELATION_TEMP_TYPE temp,ntemp;
	RELATION_SYSTEM_TYPE sys,nsys;
	FILE *fptr;
	char nname[RELATION_NAME_SIZE+1],rname[RELATION_NAME_SIZE+1];	
	boolean redirect;

	redirect=relation_system((*rel));
	if (redirect==TRUE) {
		ddredirect_start();
	}

	/* Create a new duplicate relation */
	nrel=rl_duplicate( db, *rel, "_temp");

	if (nrel!=NULL) {
		/* Get the original relation details */
		strcpy(targetname,relation_name((*rel)));

		fptr=generate_fileh(*rel);
		tuple_readheader(&fptr,&noattribs,&temp,&sys,rname);
		fclose(fptr);
		

		/* Delete the old structure */
		relation_remove(db,rel);

		/* Rename the new relation */
		rtrel=relation_rename( db, relation_name(nrel), targetname );
		relation_temporary(rtrel)=temp;
		relation_system(rtrel)=sys;

		fptr=generate_fileh(rtrel);
		tuple_readheader(&fptr,&nnoattribs,&ntemp,&nsys,nname);
		relation_update_header(db,targetname,noattribs,temp,sys,&fptr);
		fclose(fptr);

		if (redirect==TRUE) {
			if (ddredirect_stop()==RETURN_SUCCESS) {
		/*		ddredirect_execute( db ); */
			}
		}

		if (rtrel!=NULL) {
			*rel=rtrel;
		} else {
			raise_error(ERROR_RELATION_COMPACT,NONFATAL,targetname);
			return(NULL);
		}
	} else {
		if (ddredirect_stop()==RETURN_SUCCESS) {
			ddredirect_execute( db );
		}
		raise_error(ERROR_RELATION_COMPACT,NONFATAL,relation_name((*rel)));
		return(NULL);
	}
	
}

relation insert( database db,
		 relation rel,
 		 char *data ) {
/* insert
 * Performs an insert of data in data
 */

	tuple nt;
	char all[MAXIMUM_ALL_DATA],*citem,expression[MAXIMUM_EXPRESSION+1];
	unsigned int counter=0;

	if (rel!=NULL) {
		nt=tuple_prepare(rel);

		strcpy(all,data);

		citem=strtok(all,",");		

		while ( (counter<=(tuple_relation(nt,0)->noattributes)) && (citem!=NULL) ){

			/* Skip over the white space */
			strip_leading_spaces(citem);
			strip_trailing_spaces(citem);

			/* Copy to the tuple */
			if (strcmp(citem,"-")==0) {
				tuple_data(nt,counter,"");
			} else {
				tuple_data(nt,counter,citem);	
			}

			/* Get the next token */
			citem=strtok(NULL,",");		

			counter++;
		}

		(void) tuple_append(nt);

		/* Close the tuple */
		close_tuple(&nt,TUPLE_DISPOSE);

		if (strcmp(relation_name(rel),LEAP_DD_RELATIONS)!=0) {
				do_debug(DEBUG_INFO,"Updating leaprel info");
				sprintf(expression,"update (%s) (%s='%s') (%s='%s')",
					LEAP_DD_RELATIONS, LEAP_DDA_RELATIONS_NAME, relation_name(rel),
					LEAP_DDA_RELATIONS_UPDATED,TRUE_STRING);
				ddmaintenance(db,expression);
		} else {
				do_debug(DEBUG_INFO,"No updating info on leaprel - causes recursion!");
		}


		return(rel);
		
	} else {
		/* Invalid relation */
		return(NULL);
	}
}

