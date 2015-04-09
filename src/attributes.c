/* 
 * Name: attributes.c
 * Description: Attribute Functions - Access/Modification of
 *		attritbutes in a relation.
 * Version: attributes.c,v 1.3.2.1 2004/02/09 20:05:20 rleyton Exp
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

/* NOTE
 * Much of the functionality in this module is no longer necessary.
 * It will be stripped out in due course.
 */
#ifdef HAVE_CONFIG_H
# include "defines.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "consts.h"
#include "global_vars.h"
#include "dtypes.h"
#include "errors.h"
#include "util.h"
#include "relation.h"
#include "attributes.h"
#include "vars.h"
#include "tuples.h"
#include "leapio.h"

FILE *open_attribute_file( FILE **attribute_file,
			   relation rel,
			   char *mode) {
/* open_attribute_file
 * Opens the attribute file, and returns a FILE ptr
 */
	char filename[FILE_PATH_SIZE+1];

	if (leapver!=LEAP_UPGRADE) {
		raise_error( ERROR_OBSOLETED_CODE, NONFATAL,"");
	}

	sprintf(filename,"%s%s",rel->filepath,rel->fieldname);
#ifdef FULL_DEBUG
	leap_fprintf(stdout,"Opening for %s file %s\n",filename,mode);
#endif
	*attribute_file=fopen(filename,mode);

	if (attribute_file==NULL) {
		raise_error(ERROR_FILE_OPENING,NONFATAL,filename);
		return(NULL);
	}

	return *attribute_file;
}

void close_attribute_file( FILE **attribute_file ) {
/* close_attribute_file
 * Close the specified file
 */
	if (leapver!=LEAP_UPGRADE) {
		raise_error( ERROR_OBSOLETED_CODE, NONFATAL,"");
	}
	/* Close the file */
	(void) fclose(*attribute_file);
}

int attribute_create(	relation rel,
			char *attribute_name,
			char data_type,
			int  attrib_size ) {
/* attribute_create
 * (Same as field_create)
 * Create an attribute for a relation 
 */
	FILE *attribute_file;
	int nowritten;

	if (leapver!=LEAP_UPGRADE) {
		raise_error( ERROR_OBSOLETED_CODE, NONFATAL,"");
	}
	/* Open the file - This function will ensure that the
	 * file is opened. If not, then something strange is afoot
  	 * and it will abort LEAP. Its opened for appending.
	 */
	attribute_file=open_attribute_file(&attribute_file,rel,"a");

	if (attribute_file!=NULL) {

		/* Write the relation name to the file */
		nowritten=fprintf(attribute_file, "%s\n", attribute_name);

#ifdef FULL_DEBUG
	leap_fprintf(stdout,"nowritten==%i\n",nowritten);
#endif
	
		/* Depending on the data type */
		switch (data_type) {
			case DT_STRING : 
					/* It's a string */
					fprintf(attribute_file,"%s",DTS_STRING);
					break;
			case DT_NUMBER :
					/* It's a number/integer */
					fprintf(attribute_file,"%s",DTS_NUMBER);
					break;
			case DT_BOOLEAN :
					/* It's a Boolean */
					fprintf(attribute_file,"%s",DTS_BOOLEAN);
					break;
			default :
				/* Something strange has certainly happened
			 	* if this point is reached. Nonetheless, just
			 	* incase... 
			 	*/
				fprintf(attribute_file,"%s",DTS_UNSUPPORTED);
				raise_error(ERROR_UNSUPPORTED_DTYPE,NONFATAL,"");
		}	

		/* Save the size */	
		if (attrib_size>0) {
			fprintf(attribute_file," %d\n",attrib_size);
		} else {
			fprintf(attribute_file,"\n");
		}

		/* Increment the number of fields in our relation
  	 	*/
		rel->noattributes++;
	
		/* Close the attribute file */
		close_attribute_file(&attribute_file);

		return(RETURN_SUCCESS);
	} else {
		return(RETURN_ERROR);
	}
}

int attribute_populate( attribute att,
			FILE **attribute_file) {
/* attribute_populate
 * Reads from the specified file an attribute, and populates
 * the specified attribute with the data read. Returns the
 * result of the last read.
 */
	int readresult;
	char type[ATTRIBUTE_TYPE_SIZE];

	if (leapver!=LEAP_UPGRADE) {
		raise_error( ERROR_OBSOLETED_CODE, NONFATAL,"");
	}
	/* Read the entry (Attribute name, type) */
#ifdef FULL_DEBUG
	printf("feof returns: %i\n",feof(*attribute_file));
#endif
	/* Set the default size (not defined) */
	att->attrib_size=0;
	readresult=fscanf(*attribute_file,"%s\n%s %d\n",att->name,type,&(att->attrib_size));

#ifdef FULL_DEBUG
	printf("Name read: >%s<\n",att->name);
	printf("Type: >%s<\nSize: >%d< ",type,att->attrib_size);
#endif

	/* Process the type, and populate the structure
	   appropriately */
	if (readresult>=0) {

		if (strcmp(type,DTS_STRING)==0) {
			/* Type is string */
			att->data_type=DT_STRING;
			
			if ((att->attrib_size)==0) {
				att->attrib_size=DEFAULT_ATTRIBUTE_SIZE;
			}

		} else if (strcmp(type,DTS_NUMBER)==0) {
			/* Type is a number */
			att->data_type=DT_NUMBER;
			
		} else if (strcmp(type,DTS_BOOLEAN)==0) {
			/* Type is a number */
			att->data_type=DT_BOOLEAN;

		} else {
			/* Type is not known, raise a nonfatal error */
			raise_error(ERROR_UNSUPPORTED_DTYPE,NONFATAL,type);
			att->data_type=DT_UNDEFINED;
		}

		att->no=0;
	}

	/* Return the result of the last read */
	return readresult;
}

attribute attribute_findfirst(relation rel,
			      FILE **attribute_file) {
/* attribute_findfirst
 * Return the first attribute in a relation
 */
	attribute att;

	if (leapver!=LEAP_UPGRADE) {
		raise_error( ERROR_OBSOLETED_CODE, NONFATAL,"");
	}
	/* Create a ptr to an attribute structure */
	att=(attribute_struct *) malloc(sizeof(attribute_struct));
	
	/* Check that the ptr was allocated. */
	check_assign(att,"attributes.attribute_findfirst");

	(void) open_attribute_file(attribute_file,rel,"r");

	if (attribute_file==NULL) {
		return(NULL);
	} else {
		(void) attribute_populate(att,attribute_file);

		return(att);			
	}
}	


attribute attribute_build(relation rel,
			char *attribute_name,
			char data_type,
			int  attrib_size ) {
/* attribute_build
 * Builds an attribute from parameters 
 */
	attribute att;
 
	/* Create a ptr to an attribute structure */
	att=(attribute_struct *) malloc(sizeof(attribute_struct));
	
	/* Check that the ptr was allocated. */
	check_assign(att,"attributes.attribute_findfirst");

	strcpy(att->name,attribute_name);
	att->attrib_size=attrib_size;
			
	if (data_type==DT_STRING) {
		/* Type is string */
		att->data_type=DT_STRING;
		
		if ((att->attrib_size)==0) {
			att->attrib_size=DEFAULT_ATTRIBUTE_SIZE;
		}

	} else if (data_type==DT_NUMBER) {
		/* Type is a number */
		att->data_type=DT_NUMBER;
		
		if ((att->attrib_size)==0) {
			att->attrib_size=DEFAULT_NUMERIC_ATTRIBUTE_SIZE;
		}
	
	} else if (data_type==DT_BOOLEAN) {
		/* Type is a number */
		att->data_type=DT_BOOLEAN;

		if ((att->attrib_size)==0) {
			att->attrib_size=DEFAULT_BOOLEAN_ATTRIBUTE_SIZE;
		}
	} else {
		att->data_type=DT_UNDEFINED;
	}

	return(att);	
			
}

attribute attribute_findnext( attribute att,
				FILE **attribute_file,
				boolean newnode,
				boolean node_dispose) {
/* attribute_findnext
 * Locates the next attribute, and optionally creates
 * a new node for this. In addition, if the last node is
 * located, it is optional whether the node is disposed.
 * (This is all in order to populate a tuple structure
 */
	attribute natt;
	int oldno;

	if (leapver!=LEAP_UPGRADE) {
		raise_error( ERROR_OBSOLETED_CODE, NONFATAL,"");
	}
	/* Check that the end of file hasn't been reached */
	if (feof(*attribute_file)==0) {

		/* A new node is required */
		if (newnode==TRUE) {
			/* Create a ptr to an attribute structure */
			natt=(attribute_struct *) malloc(sizeof(attribute_struct));
			check_assign(natt,"attribute.findnext");	
		
			/* Populate the attribute from the file */	
			attribute_populate(natt,attribute_file);

			natt->no=att->no+1;
			/* Return our new node */	
			return(natt);
		} else {
			oldno=att->no;
			/* Populate the attribute */
			attribute_populate(att,attribute_file);
			/* Again, the likelyhood of an error should be handled */

			att->no=oldno+1;
			return(att);
		}
	} else {
		/* End of file encountered */

#ifdef FULL_DEBUG
	printf("Closing Attribute file");
#endif
		/* Close the attribute file */
		close_attribute_file(attribute_file); 

		if (node_dispose) {
			/* We want to deallocate the memory */
			attribute_dispose(&att);
		}
		/* TODO - Investigate this - This means that the
		 * attributes can become dereferenced if we don't
		 * want to dispose of the node...
		 * Maybe we want to RETURN NULL, but not dispose
		 * of the option...., but this is a parameter...
		 */
		att=NULL; 
		return(NULL);
	}	

}

void attribute_dispose(attribute *att) {
/* attribute_dispose 
 * Dispose of the memory used by the specified attribute,
 * and close files accordingly 
 */

	if ((att!=NULL) && (*att!=NULL)) {
		free(*att);
		att=NULL;
	}
}

char *fmt_build(char *fmt,attribute att) {
/* fmt_build
 * Builds a format string for printing out an attribute or value of
 * specified size
 */
	int tabs=0;
	char *spacing_var;

    spacing_var=resolve_variable("width");
    if (spacing_var!=NULL) {
        if (strcmp(spacing_var,"auto")==0) {
            tabs=0;
        } else if (strcmp(spacing_var,"tab")==0) {
            tabs=-1;
        } else {
            tabs=atoi(spacing_var);
        }
    }

	if ( (att->attrib_size>0) && (tabs==0) )
		if ( (attribute_type(att)==DT_BOOLEAN) ) {
			sprintf(fmt,"%%-5.5s ");
		} else {
			sprintf(fmt,"%%-%d.%ds ",att->attrib_size,att->attrib_size);
		}
	else if  ( (att->attrib_size<=0)
			 || (tabs==-1) ) {
		sprintf(fmt,"%%s \t");
	} else {
		sprintf(fmt,"%%-%d.%ds ",tabs,tabs);
	}

	return(fmt);

}

void attribute_print(attribute att) {
/* attribute_print
 * Print out the information contained in the specified
 * attribute
 */
	char fmt[FORMAT_SIZE];

	fmt_build(fmt,att);

	/* Check that the ptr is assigned! */
	if (att!=NULL) {
		switch (attribute_type(att)) {
			case DT_STRING:
				leap_printf(DTS_STRING);
				leap_printf("      ");
				break;
			case DT_NUMBER:
				leap_printf(DTS_NUMBER);
				leap_printf("     ");
				break;
			case DT_BOOLEAN:
				leap_printf(DTS_BOOLEAN);
				leap_printf("     ");
				break;
			default:
				leap_printf(DTS_UNSUPPORTED);
				leap_printf(" ");
		}
		if (attribute_size(att)==0) leap_printf("-");
		else leap_printf("%d",attribute_size(att));

		if (attribute_size(att)<9) leap_printf("   ");
		else if (attribute_size(att)<99) leap_printf("  ");
		else leap_printf(" ");

		leap_printf("%-25.25s \n",attribute_name(att));
	}
}

void attribute_display(relation rel) {
/* attribute_display
 * Print out all of the attributes associated with
 * a relation
 */
	attribute att;
	tuple ctuple;
	word count;

	printf("%-12.12s%-4.4s%s\n","Type","Sz.","Name");
	printf("----------- --- ----------\n");
	/* Locate the first attribute */
	att=relation_attribute_readfirst(rel,&ctuple,&count);

	/* Move through the attributes */
	while (att!=NULL) {
		/* Display the first attribute */
		attribute_print(att);
		
		/* Locate the next attribute */
		/*att=attribute_findnext(att,&attribute_file,FALSE,TRUE);	*/
		att=relation_attribute_readnext(rel,&ctuple,att,&count);
	}
}
	
void attributes_print(relation rel) {
/* attribute_print
 * Print out all of the attributes associated with
 * a relation - For rl_display mainly.
 */
	attribute att;
	char fmt[FORMAT_SIZE];
	int counter;
	tuple ctuple;
	word count;

	att=relation_attribute_readfirst(rel,&ctuple,&count);
	/* Locate the first attribute */
	/* att=attribute_findfirst(rel,&attribute_file); */

	/* Move through the attributes */
	while (att!=NULL) {
		fmt_build(fmt,att);
		/* Display the first attribute */
		leap_printf(fmt,attribute_name(att));
		
		/* Locate the next attribute */
	/*	att=attribute_findnext(att,&attribute_file,FALSE,TRUE);	 */
		att=relation_attribute_readnext(rel,&ctuple,att,&count);
	}
	leap_printf("\n");
   /* Set to 79 for MS-DOS display wrapping... */
	for (counter=0;counter<79;counter++) leap_printf("-");
	leap_printf("\n");
}

attribute attribute_find ( relation rel, char *name) {
/* attribute_find
 * Locates the given attribute, and returns an attribute structure
 */
	attribute catt,ratt;
	tuple ctuple;
	word anum;

	/* Get the first attribute */
	/* catt=attribute_findfirst(rel,&attribute_file); */
	catt=relation_attribute_readfirst(rel,&ctuple,&anum);

	/* Whilst the attribute is valid, and the attribute does
	 * not match the search criteria 
	 */
	while ( (catt!=NULL) && (name!=NULL) && (strcmp(attribute_name(catt),name)!=0) ) {

		/* Locate the next field */
		catt=relation_attribute_readnext(rel,&ctuple,catt,&anum);
	}

	/* Check to see if we actually found anything */
	if ( (catt!=NULL) && (name!=NULL) && (strcmp(attribute_name(catt),name)==0) ) {
	
		/* We're about to destroy the tuple, which will destroy the attribute
		 * we just found. Make a copy quick!
		 */
		/* Create a ptr to an attribute structure */
		ratt=(attribute_struct *) malloc(sizeof(attribute_struct));
	
		/* Check that the ptr was allocated. */
		check_assign(ratt,"attributes.attribute_find(cpy)");

		strcpy(attribute_name(ratt),attribute_name(catt));
		attribute_type(ratt)=attribute_type(catt);
		attribute_size(ratt)=attribute_size(catt);
		attribute_no(ratt)=attribute_no(catt);
		
		close_tuple(&ctuple,TUPLE_DISPOSE);

		/* Return the ptr */
		return(ratt);
	} else {
	
		/* Close the tuple */
		close_tuple(&ctuple,TUPLE_DISPOSE);

		/* Return nothing */	
		return(NULL);
	}
}

