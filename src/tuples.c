/* 
 * Name: tuples.c
 * Description: Tuple function for control of tuples.
 * Version: tuples.c,v 1.210.2.1 2004/02/09 20:05:20 rleyton Exp
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
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include "consts.h"
#include "globals.h"
#include "dtypes.h"
#include "errors.h"
#include "tuples.h"
#include "dbase.h"
#include "relation.h"
#include "attribs.h"
#include "util.h"
#include "hashing.h"
#include "vars.h"
#include "leapio.h"

#ifdef MEMORY_DEBUG
#include "dmalloc.h"
#endif

/* Count info. */
/* Declared in globals.h */
unsigned int no_written;
unsigned int no_read_physical;
unsigned int no_read_logical;

void tuple_dispose(tuple *ntuple) {
  word count;

  do_debug(DEBUG_ENTER,"ENTERing tuple_dispose\n");

  if ( (ntuple!=NULL) && (*ntuple!=NULL) ) {

    levindicator(FALSE);
    for (count=0;count<MAXIMUM_ATTRIBUTES;count++) {

      do_debug(DEBUG_MEM,"[%d]",count);

      attribute_dispose(&tuple_item(*ntuple,count)->att);
      free ( tuple_item(*ntuple,count) );
      tuple_item(*ntuple,count)=NULL;
    }
    levindicator(TRUE);

    do_debug(DEBUG_MEM,"Done.\n",count);

    /* Check the file isn't still open.
     * - If so, close it.
     */	
    if (tuple_f(*ntuple)!=NULL) {
      do_debug(DEBUG_MEM,"Closing.\n",count);
      fclose(tuple_f(*ntuple));
      do_debug(DEBUG_MEM,"Closed.\n",count);
    }

    free(tuple_dat(*ntuple));

    do_debug(DEBUG_MEM,"Disposed.\n",count);

    free(*ntuple);
    *ntuple=NULL; 
  }

  do_debug(DEBUG_ENTER,"EXITing tuple_dispose\n");
}

tuple raw_tuple_prepare(void) {
  /* tuple_prepare
   * This function prepares a tuple for our use, by allocating
   * memory where necessary. The tuple is useless until populated.
   */
  word count;
  tuple newtuple;

  /* Create the main tuple structure */
  newtuple=(tuple_master_struct *) malloc(sizeof(tuple_master_struct));
  check_assign(newtuple,"tuples.tuple_prepare");  

  /* Create the array */
  (*newtuple).tuple_data=(tuple_array *) malloc(sizeof(tuple_array));
  check_assign((*newtuple).tuple_data,"tuples.tuple_prepare(array)");     

  /* Move through the array of tuple node ptrs */
  for (count=0;count<MAXIMUM_ATTRIBUTES;count++) {
    /* Create an item */
    tuple_item(newtuple,count)=(tuple_item_structure *) malloc(sizeof(tuple_item_structure));
    check_assign(tuple_item(newtuple,count),"tuples.tuple_prepare.item");  

    /* Reset the entries */
    tuple_relation(newtuple,count)=NULL;
    tuple_attribute(newtuple,count)=NULL;
    strcpy(tuple_d(newtuple,count),"");
  }

  tuple_f(newtuple)=NULL;
  tuple_no(newtuple)=-1;
  tuple_offset(newtuple)=0;
  newtuple->startofdata=0;

  /* Return the new tuple */
  return(newtuple);
}


tuple tuple_prepare_attributes(relation rel) {
  /* tuple_prepare_attributes
   * Prepare a tuple by taking all of the attributes from
   * the specified relation, and populating it accordingly
   */
  attribute att;
  tuple ntuple;
  word count;
  FILE *attribute_file;

  /* Create the new tuple */
  ntuple=raw_tuple_prepare();

  /* Locate the first field in our relation */
  att=attribute_findfirst(rel,&attribute_file);

  ntuple->offset=0;

  count=0;
  /* Whilst the field is assigned */
  while (att!=NULL) {
    /* Set the relation */
    tuple_relation(ntuple,count)=rel;

    /* Set the attribute */
    tuple_attribute(ntuple,count)=att;

    /* Fetch the next attribute */
    att=attribute_findnext(att,&attribute_file,TRUE,FALSE);

    /* Increment the counter */
    count++;
  }

  /* We've not necessarily populated the entire structure,
   * but one look at tuple_prepare should show us that the
   * structure is reset there, so no need to repeat the process
   * (as in the Pascal version)
   */

  /* Return the new tuple! */
  return(ntuple);
}	

relation get_relation(tuple t) {
  /* get_relation
   * This routine fetches the relation from the tuple structure
   */
  return( tuple_relation(t,0) );
}


void populate_tuple(tuple ntuple,
    char buffer[ATTRIBUTE_MAXIMUM_SIZE*MAXIMUM_ATTRIBUTES],
    word *noattributes) {
  /* populate_tuple
   * Takes the data in buffer, and populates the tuple ntuple 
   */

  char *str,*sptr;
  word counter,position;

  for (counter=0; counter<MAXIMUM_ATTRIBUTES; counter++) {
    tuple_data( ntuple, counter, "");		
  }

  if (status_case==TRUE) {
    upcase(buffer);
  }

  sptr=strchr(buffer,DELIMETER_CHAR);
  while (sptr!=NULL) {
    if (*(sptr+1)==DELIMETER_CHAR) {
      *(sptr)=1;
      *(sptr+1)=1;
      sptr+=1;
    } else if (*(sptr+1)==NULL_INDICATOR) {
      *(sptr+1)=2;
      sptr+=1;
    }
    sptr=strchr(sptr+1,DELIMETER_CHAR); 
  }


  /* Locate the first delimiter in the string */
  str=strtok(buffer,DATA_DELIMITER);

  /* Reset the counter */	
  counter=0;

  /* Whilst we have a valid string */
  while (str!=NULL) {

    sptr=strchr(str,1);
    if (sptr!=NULL) {
      for (position=0;position<strlen(str);position++) {
        if ( (*(sptr+position)==1) &&
            (*(sptr+position+1)==1) ) {
          *(sptr+position+1)=DELIMETER_CHAR;
        }
        *(sptr+position)=*(sptr+position+1);
      }
    }
    sptr=strchr(str,2);
    if (sptr!=NULL) {
      *sptr='\0';
    }

    /* Copy the data from str into the appropriate
     * position (tuple_data is a macro)
     */
    tuple_ndata(ntuple,counter,str);

    /* Increment our counter */
    counter++;

    /* Locate the next delimiter in the string */
    str=strtok(NULL,DATA_DELIMITER);
  }

  *noattributes=counter;

}


tuple old_readfirst_tuple(relation rel,
    FILE **tfile,
    word *noattributes,
    int reuse,
    tuple old_tuple) {
  /* old_readfirst_tuple
   * Read the first tuple from the relation
   */

  char fname[FILE_PATH_SIZE+1];
  char buffer[ATTRIBUTE_MAXIMUM_SIZE*MAXIMUM_ATTRIBUTES];
  tuple ntuple=NULL;

  relation_full_path(rel,fname);

  /* Tac the .REL Extension - This could be more efficent, if
   * relation_full_path perhaps had an optional third parameter
   */
  strcat(fname,LEAP_RELATION_EXT);

  *tfile=fopen(fname,"r");

  if (*tfile==NULL) {
    raise_error(ERROR_FILE_OPENING,NONFATAL,fname);
    return(NULL);
  }

  rel->current_pos=1;

  /* TODO Cache read? */

  /* Increase the number of reads made */
  no_read_physical++;

  strcpy(buffer,"");

  if (fgets(buffer,sizeof(buffer),*tfile)==NULL) {
#ifdef FULL_DEBUG
    leap_fprintf(stderr,"Error reading file or empty string?\n");
#endif
    if (reuse!=TUPLE_REUSE) {
      ntuple=tuple_prepare_attributes(rel);
    } else {
      ntuple=old_tuple;
    }

    populate_tuple(ntuple,"",noattributes);
  } else {
    if (strcmp(buffer,"")!=0) {
      /* Create the raw tuple */
      if (reuse!=TUPLE_REUSE) {
        ntuple=tuple_prepare_attributes(rel);
      } else {
        ntuple=old_tuple;
      }

      populate_tuple(ntuple,buffer,noattributes);
    } else {
#ifdef FULL_DEBUG
      leap_fprintf(stderr,"Empty string read\n");
#endif
    }
  }

  /* Return our (possibly) spanking new tuple all fresh and clean and
   * pure and innocent.
   */
  return(ntuple);	
}

void close_tuple(tuple *ntuple,
    int reuse) {
  /* close_tuple
   * Closes the tuple and tidies up. Useful if processing is
   * aborted halfway through, or since first open 
   */

  if (ntuple!=NULL) {
    if (*ntuple!=NULL) {

      /* Is the tuple to be reused... */
      if (reuse!=TUPLE_REUSE) {
        tuple_dispose(ntuple); 
        *ntuple=NULL;
      }
    }
  }
}

int old_readnext_tuple(tuple *ntuple,
    FILE **tfile,
    word *noattributes,
    int reuse) {
  /* old_readnext_tuple
   * Reads the next tuple from the file opened by readfirst_tuple.
   */

  char buffer[ATTRIBUTE_MAXIMUM_SIZE*MAXIMUM_ATTRIBUTES];

  /* Increase read count */
  no_read_physical++;

  strcpy(buffer,"");

  if (fgets(buffer,sizeof(buffer),*tfile)==NULL) {
    /* End of file has been reached */

    close_tuple(ntuple, reuse);

    return(EOF);
  } else {
    /* Process the next tuple */
    populate_tuple(*ntuple,buffer,noattributes);

    return(RETURN_SUCCESS);
  }

}

void spaces(done, need) {
  int counter;

  for (counter=done; counter<need; counter++) leap_printf(" ");
}

void tuple_revprint(relation rel,
    tuple ntuple) {
  /* tuple_revprint
   * Prints the tuple in reverse engineering format. 
   */
  word counter=0;

  leap_printf("add (%s) (",relation_name(rel));

  while ( tuple_relation(ntuple,counter)!=NULL ) {
#ifdef FULL_DEBUG
    leap_fprintf(stderr,"Pos[%d]: %s\n",counter,tuple_d(ntuple,counter));
#endif
    if (strlen(tuple_d(ntuple,counter))==0) {
      leap_printf("-");
    } else {
      leap_printf("%s",tuple_d(ntuple,counter));   
    }

    /* Increment the counter! */
    counter++;

    if (tuple_relation(ntuple,counter)!=NULL ) leap_printf(",");
  }
  leap_printf(")\n");
}

void tuple_def(tuple ntuple) {
  /* tuple_def
   * Prints the tuple definition. 
   */
  word counter;
  char fmt[20];
  char *spacing_var;
  int  tabs=0;

  counter=0;

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

  while ( tuple_relation(ntuple,counter)!=NULL ) {
#ifdef FULL_DEBUG
    leap_fprintf(stderr,"Pos[%d]: %s\n",counter,tuple_d(ntuple,counter));	
#endif
    if ( (tuple_attribute(ntuple,counter)->attrib_size>0) && (tabs==0) )
      if ( attribute_type(tuple_attribute(ntuple,counter))==DT_BOOLEAN) {
        sprintf(fmt,"%%-5.5s ");
      } else {
        sprintf(fmt,"%%-%d.%ds ",tuple_attribute(ntuple,counter)->attrib_size,tuple_attribute(ntuple,counter)->attrib_size);
      }
      else if ( (tuple_attribute(ntuple,counter)->attrib_size<=0) 
          ||(tabs==-1) ) {
        sprintf(fmt,"%%s \t");
      } else {
        if ( attribute_type(tuple_attribute(ntuple,counter))==DT_BOOLEAN) {
          sprintf(fmt,"%%-5.5s ");
        } else {
          sprintf(fmt,"%%-%d.%ds ",tabs,tabs);
        }
      }

    leap_fprintf(stdout,"[[%d]%s]",tuple_attribute(ntuple,counter)->no,tuple_attribute(ntuple,counter)->name);	

    /* Increment the counter! */
    counter++;
  }

  leap_fprintf(stdout,"\n");
  counter=0;

  while ( tuple_relation(ntuple,counter)!=NULL ) {
#ifdef FULL_DEBUG
    leap_fprintf(stderr,"Pos[%d]: %s\n",counter,tuple_d(ntuple,counter));	
#endif
    if ( (tuple_attribute(ntuple,counter)->attrib_size>0) && (tabs==0) )
      if ( attribute_type(tuple_attribute(ntuple,counter))==DT_BOOLEAN) {
        sprintf(fmt,"%%-5.5s ");
      } else {
        sprintf(fmt,"%%-%d.%ds ",tuple_attribute(ntuple,counter)->attrib_size,tuple_attribute(ntuple,counter)->attrib_size);
      }
      else if ( (tuple_attribute(ntuple,counter)->attrib_size<=0) 
          ||(tabs==-1) ) {
        sprintf(fmt,"%%s \t");
      } else {
        if ( attribute_type(tuple_attribute(ntuple,counter))==DT_BOOLEAN) {
          sprintf(fmt,"%%-5.5s ");
        } else {
          sprintf(fmt,"%%-%d.%ds ",tabs,tabs);
        }
      }

    leap_fprintf(stdout,fmt,tuple_d(ntuple,counter));	

    /* Increment the counter! */
    counter++;
  }
  leap_fprintf(stdout,"\n");
}

void tuple_print(tuple ntuple) {
  /* tuple_print
   * Prints the tuple. 
   */
  word counter;
  char *spacing_var;
  int  tabs=0;

  counter=0;

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

  while ( tuple_relation(ntuple,counter)!=NULL ) {
#ifdef FULL_DEBUG
    leap_fprintf(stderr,"Pos[%d]: %s\n",counter,tuple_d(ntuple,counter));	
#endif
#ifdef NEVER_SET
    if ( (tuple_attribute(ntuple,counter)->attrib_size>0) && (tabs==0) )
      if ( attribute_type(tuple_attribute(ntuple,counter))==DT_BOOLEAN) {
        sprintf(fmt,"%%-5.5s ");
      } else {
        sprintf(fmt,"%%-%d.%ds ",tuple_attribute(ntuple,counter)->attrib_size,tuple_attribute(ntuple,counter)->attrib_size);
      }
      else if ( (tuple_attribute(ntuple,counter)->attrib_size<=0) 
          ||(tabs==-1) ) {
        sprintf(fmt,"%%s \t");
      } else {
        if ( attribute_type(tuple_attribute(ntuple,counter))==DT_BOOLEAN) {
          sprintf(fmt,"%%-5.5s ");
        } else {
          sprintf(fmt,"%%-%d.%ds ",tabs,tabs);
        }
      }
#endif

    leap_printf(attribute_fmt(tuple_attribute(ntuple,counter)),tuple_d(ntuple,counter));	

    /* Increment the counter! */
    counter++;
  }
  leap_printf("\n");
}

int write_tuple(tuple wtuple) {
  printf("write_tuple(): entered\n");
  /* write_tuple
   * This routine writes the specified tuple to the file, and
   * returns the position to which it was stored (negative if
   * an error occured)
   */

  relation rel;
  FILE *fl;
  char fname[FILE_PATH_SIZE+1],hashfile[FILE_PATH_SIZE+1];
  unsigned int index;
  char tuple_string[MAXIMUM_ALL_DATA],found_string[HASH_KEY_SIZE];
  boolean success;

  /* Get the relation to which this tuple relates */
  rel=get_relation(wtuple);

  /* Mark it updated. It'd be expensive to check as well (kind of) */
  rel->updated=TRUE;

  /* Load the hash table if it has not been loaded */
  if (tuple_relation(wtuple,0)->hash_table==NULL) {
    relation_full_path(tuple_relation(wtuple,0),hashfile);
    strcat(hashfile,LEAP_HASH_EXT);
    hashing_load(&rel->hash_table,hashfile); 
  }

  tuple_to_string(wtuple,tuple_string);
  printf("write_tuple(): tuple_string = %s\n", tuple_string);
  hashing_retrieve( tuple_relation(wtuple,0)->hash_table, tuple_string, found_string, &success);

  if (!success) {	
    /* Determine the position at which the insert
     * will take place - This is for the index
     * structures - TODO 
     */	

    /* Determine the filename */
    sprintf(fname,"%s%s",rel->filepath,rel->filename);

    /* Open the file for appending */
    if (tuple_f(wtuple)==NULL) {
      tuple_f(wtuple)=fopen(fname,"ab");	
    }
    fl=tuple_f(wtuple);

    /* Handle (well, die!) errors with the file open... */
    if (fl==NULL) {
      raise_error(ERROR_FILE_OPENING,FATAL,fname);
    }

    index=0;

    while( index<rel->noattributes ) {
#ifdef FULL_DEBUG
      leap_fprintf(stderr,"Write: >%s\\<",tuple_d(wtuple,index));
#endif
      if (tuple_d(wtuple,index)[0]=='\0')
        tuple_data(wtuple,index,"-");
      fprintf(fl,"%s\\",tuple_d(wtuple,index));
      index++;
    }
    fprintf(fl,"\n");

    /* Insert the tuple into the hash table */
    hashing_insert((tuple_relation(wtuple,0))->hash_table,tuple_string,REQ_CALC);

    /* Increase the number of tuples written */
    no_written++;

    /* Set the updated flag. */
    (tuple_relation(wtuple,0))->updated=TRUE;
  } else {
    do_trace("Trace> Attempt to write duplicate tuple");
  }

  return(0);
}

void tuple_to_string(tuple t,
    char *tuple_string) {
  /* tuple_to_string
   * Convert a tuple's data to a single string (for hashing
   * purposes)
   */
  unsigned int position;

  /* Initialise our position counter */
  position=0;

  /* Whilst the attribute ptr at this tuple node is not null */
  while (   tuple_attribute(t,position) != NULL  ) {

    /* If this is the first tuple node */
    if (position==0) {
      /* Copy the data into the string */
      strcpy( tuple_string, tuple_d(t,position) );
    } else {
      /* Otherwise, concatenate the data with the string */
      strcat( tuple_string, tuple_d(t,position) );
    }

    /* Increment the position counter */
    position++;
  }
}

attribute tuple_find_attribute( tuple ct,
    char *name) {
  /* tuple_find_attribute
   * Locates an attribute within a tuple, and returns it.
   * Primarily of use for the condition evaluation, when building
   */
  word counter=0;
  boolean found=FALSE;

  /* Whilst the attribute has not been found, and the current item in the
   * tuple is not NULL
   */
  while ( (found==FALSE) && ( tuple_relation(ct, counter ) != NULL ) ) {

    if ( strcmp(name, attribute_name( tuple_attribute(ct, counter) ) ) == 0 ) { 
      /* It matches! */
      found=TRUE;	
    } else {
      /* It's not the one we are after */
      counter++;
    }
  }

  if ( found == TRUE ) {
    return( tuple_attribute(ct,counter) );
  } else {
    return( NULL );
  }
}

char *tuple_find_attribute_val( tuple ct,
    char *name ) {

  word counter=0;
  boolean found=FALSE;

  /* Whilst the attribute has not been found, and the current item in the
   * tuple is not NULL
   */
  while ( (found==FALSE) && (ct!=NULL) && ( tuple_relation(ct, counter ) != NULL ) ) {

    /* Compare the attribute name to the string we are after... */
    if ( strcmp(name, attribute_name( tuple_attribute(ct, counter) ) ) == 0 ) {
      /* It matches! */
      found=TRUE;	
    } else {
      /* It's not the one we are after */
      counter++;
    }
  }

  if ( found == TRUE ) {
    return( tuple_d( ct, counter ) );
  } else {
    return( NULL );
  }
}

char *get_attribute_info( char *outdtype, ATTRIBUTE_TYPE_TYPE dtype) {
  /* Produce a string version of the string type
  */

  switch ( dtype ) {
    case DT_STRING: strcpy(outdtype,DTS_STRING);
                    break;
    case DT_NUMBER: strcpy(outdtype,DTS_NUMBER);
                    break;
    case DT_BOOLEAN:strcpy(outdtype,DTS_BOOLEAN);
                    break;
    default: strcpy(outdtype,DTS_UNDEFINED);
             break;
  }
  return (outdtype);

}

ATTRIBUTE_SIZE_TYPE attribsize(ATTRIBUTE_TYPE_TYPE attrtype) {
  ATTRIBUTE_SIZE_TYPE size;

  switch ( attrtype ) {
    case DT_STRING: size=0;	
                    if (size==0) size=DEFAULT_ATTRIBUTE_SIZE;
                    break;
    case DT_NUMBER: size=sizeof(long);
                    break;
    case DT_BOOLEAN: size=1;
                     break;
    default: size=0;
             break;
  }
  return(size);
}

int relation_create_write_attribute( attribute catt,
    FILE **fptr ) {
  printf("relation_create_write_attribute(): entered\n");
  printf("relation_create_write_attribute(): catt->name = %s\n", catt->name);
  /* relation_create_write_attribute
   * Writes an attribute record to the relation file
   */

  int res;
  ATTRIBUTE_TYPE_TYPE dtype;
  ATTRIBUTE_SIZE_TYPE size;
  ATTRIBUTE_KEYCOMP_TYPE keycomponent=0;

  do_debug(DEBUG_ENTER,"ENTERing relation_create_write_attribute\n");

  res=fwrite( attribute_name( catt ),ATTRIBUTE_NAME_SIZE,1, *fptr );

  if (res!=1) {
    leap_fprintf(stdout,"Error writing attribute name\n");
    do_debug(DEBUG_ENTER,"EXITing (unsuccessfully) relation_create_write_attribute\n");
    return(RETURN_ERROR);
  } 

  switch ( attribute_type( catt ) ) {
    case DT_STRING: dtype=DT_STRING;
                    size=attribute_size( catt );	
                    if (size==0) size=DEFAULT_ATTRIBUTE_SIZE;
                    break;
    case DT_NUMBER: dtype=DT_NUMBER;
                    size=sizeof(long);
                    break;
    case DT_BOOLEAN:dtype=DT_BOOLEAN;
                    size=1;
                    break;
    default: dtype=DT_UNDEFINED;
             size=DEFAULT_ATTRIBUTE_SIZE;
             break;
  }

  res=fwrite( &dtype, sizeof(dtype),1, *fptr );

  if (res!=1) {
    printf("Error writing dtype\n");
    do_debug(DEBUG_ENTER,"EXITing (unsuccessfully) relation_create_write_attribute\n");
    return(RETURN_ERROR);
  }

  res=fwrite( &size, sizeof(size),1, *fptr );

  if (res!=1) {
    leap_fprintf(stdout,"Error writing size\n");
    do_debug(DEBUG_ENTER,"EXITing (unsuccessfully) relation_create_write_attribute\n");
    return(RETURN_ERROR);
  }

  res=fwrite( &keycomponent, sizeof(keycomponent),1, *fptr );

  if (res!=1) {
    leap_fprintf(stdout,"Error writing keycomponent\n");
    do_debug(DEBUG_ENTER,"EXITing (unsuccessfully) relation_create_write_attribute\n");
    return(RETURN_ERROR);
  }

  (void) fflush(*fptr);

  {
    printf("relation_create_write_attribute(): after fflush()\n");
    struct stat buf;
    int fd = fileno(*fptr);
    fstat(fd, &buf);
    printf("relation_create_write_attribute(): filesize = %d\n", buf.st_size);
    printf("relation_create_write_attribute(): mode = %o\n", buf.st_mode);
    printf("relation_create_write_attribute(): uid = %d\n", buf.st_uid);
    printf("relation_create_write_attribute(): gid = %d\n", buf.st_gid);
  }

  do_debug(DEBUG_ENTER,"EXITing (successfully) relation_create_write_attribute\n");
  return(RETURN_SUCCESS);
}


int relation_create_write_header( char *name,
    NOATTRIBUTES_TYPE noattribs,
    RELATION_TEMP_TYPE temporary,
    RELATION_SYSTEM_TYPE system,
    FILE **fptr ) {
  printf("relation_create_write_header(): name = %s\n", name);
  printf("relation_create_write_header(): entered\n");
  /* relation_create_write_header
   * Writes the relation header to the specified file
   */
  int retname,retatt,rettmp,retver;
  unsigned int ver=LEAP_VERSION_IDENT;

  do_debug(DEBUG_ENTER,"ENTERing relation_create_write_header\n");

  {
    struct stat buf;
    int fd = fileno(*fptr);
    fstat(fd, &buf);
    printf("relation_create_write_header()-2: filesize = %d\n", buf.st_size);
    printf("relation_create_write_header()-2: mode = %o\n", buf.st_mode);
    printf("relation_create_write_header()-2: uid = %d\n", buf.st_uid);
    printf("relation_create_write_header()-2: gid = %d\n", buf.st_gid);
  }

  /* Write the LEAP version of attributes */
  retver=fwrite( &ver, sizeof(ver),1, *fptr);
  printf("relation_create_write_header(): ver = %d\n", ver);

  /* Write the relation name */
  retname=fwrite( name, RELATION_NAME_SIZE, 1, *fptr );

  /* Write the temporary flag */
  rettmp=fwrite( &temporary, sizeof(temporary), 1, *fptr );

  /* Write the temporary flag */
  rettmp=fwrite( &system, sizeof(system),1, *fptr );

  /* Write the number of attributes */
  retatt=fwrite( &noattribs, sizeof(noattribs),1, *fptr);

  (void) fflush(*fptr);

  {
    struct stat buf;
    int fd = fileno(*fptr);
    fstat(fd, &buf);
    printf("relation_create_write_header()-2: filesize = %d\n", buf.st_size);
    printf("relation_create_write_header()-2: mode = %o\n", buf.st_mode);
    printf("relation_create_write_header()-2: uid = %d\n", buf.st_uid);
    printf("relation_create_write_header()-2: gid = %d\n", buf.st_gid);
  }

  if ( (retver==0) || (retname==0) || (retatt==0) || (rettmp==0) ) {
    do_debug(DEBUG_ENTER,"EXITing (unsuccessfully) relation_create_write_header\n");
    return(RETURN_ERROR);
  } else {
    do_debug(DEBUG_ENTER,"EXITing (successfully) relation_create_write_header\n");
    return(RETURN_SUCCESS);
  }

}

int relation_update_header( database db, char *name,
    NOATTRIBUTES_TYPE noattribs,
    RELATION_TEMP_TYPE temporary,
    RELATION_SYSTEM_TYPE system,
    FILE **fptr ) {
  /* relation_update_header
   * updates the header with specific info
   */

  long curpos,tpos;
  int retval;
  char expression[MAXIMUM_EXPRESSION+1];

  do_debug(DEBUG_ENTER,"ENTERing relation_update_header\n");

  /* Determine the current position in the file */
  curpos=ftell(*fptr);
  do_debug(DEBUG_INFO,"Start pos: %ld\n",curpos);

  /* Jump to the beginning of the file */
  fseek(*fptr,0,SEEK_SET); 
  tpos=ftell(*fptr);
  do_debug(DEBUG_INFO,"Current pos: %ld\n",tpos);

  /* Update the current header with the new information */
  retval=relation_create_write_header( name, noattribs, temporary, system, fptr);

  /* Reposition the pointer */
  fseek(*fptr,curpos,SEEK_SET);
  tpos=ftell(*fptr);
  do_debug(DEBUG_INFO,"Return pos: %ld\n",tpos);

  sprintf(expression, "update (%s) (%s='%s') (%s='%d')",
      LEAP_DD_RELATIONS,LEAP_DDA_RELATIONS_NAME,name,LEAP_DDA_RELATIONS_NOATTRIBS,noattribs);
  ddmaintenance( whichdb(db,name), expression );
  do_debug(DEBUG_ENTER,"EXITing relation_update_header\n");
  return(retval);	
}



int relation_create_write_eoh_marker( NOATTRIBUTES_TYPE noattributes,
    FILE **fptr ) {
  printf("relation_create_write_eoh_marker(): entered");
  /* relation_create_write_eoh_marker
   * Creates a special marker at the end of the attribute definition section
   * which can be checked to ensure reads occured correctly. A kind of 
   * checksum if you like 
   */
  char marker[4],*mptr;
  int res;

  do_debug(DEBUG_ENTER,"ENTERing relation_create_write_eoh_marker\n");
  sprintf(marker,"%c%c%c",'\0','\0','\0');
  mptr=&marker[0];

  sprintf(mptr,"%c",noattributes);

  res=fwrite( marker, sizeof(marker),1, *fptr );

  if ( res!=1 )	{
    leap_fprintf(stdout,"Error writing marker\n");
    do_debug(DEBUG_ENTER,"EXITing (unsuccessfully) relation_create_write_eoh_marker\n");
    return(RETURN_ERROR);
  } else {
    do_debug(DEBUG_ENTER,"EXITing (successfully) relation_create_write_eoh_marker\n");
    return(RETURN_SUCCESS);
  }	
}


int relation_read_eoh_marker( NOATTRIBUTES_TYPE noattributes,
    FILE **fptr ) {

  /* relation_read_eoh_marker
   * Reads the special marker at the end of the attribute definition section,
   * compares the value with the expected value, and returns success/failure
   * appropriately
   */

  char marker[4],*mptr;
  NOATTRIBUTES_TYPE readnoattribs;
  int res;
  long t;


  do_debug(DEBUG_ENTER,"ENTERing relation_read_eoh_marker\n");
  t=ftell(*fptr);
  do_debug(DEBUG_INFO,"At: %ld\n",t);

  res=fread( marker, sizeof(marker),1, *fptr );

  if ( res!=1 ) {
    leap_fprintf(stdout,"Error reading marker\n");
    do_debug(DEBUG_ENTER,"EXITing (unsuccessfully) relation_read_eoh_marker\n");
    return(RETURN_ERROR);
  } else {
    mptr=&marker[0];
    sscanf(mptr,"%c",&readnoattribs);
    do_debug(DEBUG_INFO,"Read marker val: %d. Expected val: %d\n",readnoattribs,noattributes);

    if (readnoattribs==noattributes) {
      do_debug(DEBUG_ENTER,"EXITing (successfully) relation_read_eoh_marker\n");
      return(RETURN_SUCCESS);
    } else {
      do_debug(DEBUG_ENTER,"EXITing (unsuccessfully) relation_read_eoh_marker\n");
      return(RETURN_ERROR);
    }
  }
}


int printrelinfo( char *path ) {

  FILE *fptr;
  char name[RELATION_NAME_SIZE+1];
  char aname[ATTRIBUTE_NAME_SIZE+1];
  NOATTRIBUTES_TYPE noattribs, count, c;
  int res;
  boolean finish;
  ATTRIBUTE_TYPE_TYPE dtype;
  ATTRIBUTE_TYPE_TYPE types[MAXIMUM_ATTRIBUTES];
  int rsize;
  unsigned int leapver;
  ATTRIBUTE_SIZE_TYPE size;
  int totalsize;
  ATTRIBUTE_SIZE_TYPE sizes[MAXIMUM_ATTRIBUTES];
  ATTRIBUTE_KEYCOMP_TYPE keycomponent;
  RELATION_TEMP_TYPE temporary;
  RELATION_SYSTEM_TYPE system;
  TUPLE_STATUS_TYPE status;
  DT_NUMBER_TYPE number;
  DT_BOOLEAN_TYPE bool;
  char string[ATTRIBUTE_MAXIMUM_SIZE+1];

  do_debug(DEBUG_ENTER,"ENTERing printrelinfo\n");
  leap_fprintf(stdout,"Disk relation data...\n");
  fptr=fopen(path,"rb");

  if (fptr!=NULL) {

    res=fread( &leapver, sizeof(leapver),1, fptr );
    if (res!=1) {
      leap_fprintf(stdout,"Error reading version identifier\n");
      do_debug(DEBUG_ENTER,"EXITing (unsuccessfully) printrelinfo\n");
      return(RETURN_ERROR);
    }

    res=fread( name, RELATION_NAME_SIZE,1, fptr );

    if (res!=1) {
      leap_fprintf(stdout,"Error reading relation name\n");
      do_debug(DEBUG_ENTER,"EXITing (unsuccessfully) printrelinfo\n");
      return(RETURN_ERROR);
    }

    res=fread( &temporary, sizeof(temporary), 1,fptr );

    if (res!=1) {
      leap_fprintf(stdout,"Error reading relation temporary status\n");
      do_debug(DEBUG_ENTER,"EXITing (unsuccessfully) printrelinfo\n");
      return(RETURN_ERROR);
    }

    res=fread( &system, sizeof(system),1, fptr );

    if (res!=1) {
      leap_fprintf(stdout,"Error reading relation system status\n");
      do_debug(DEBUG_ENTER,"EXITing (unsuccessfully) printrelinfo\n");
      return(RETURN_ERROR);
    }

    res=fread( &noattribs, sizeof(noattribs), 1, fptr );

    if (res!=1) {
      leap_fprintf(stdout,"Error reading no attributes\n");
      do_debug(DEBUG_ENTER,"EXITing (unsuccessfully) printrelinfo\n");
      return(RETURN_ERROR);
    }

    leap_fprintf(stdout,"Relation Name: %s\n",name);

    leap_printf("Relation is ");
    if (temporary==TRUE) {
      leap_fprintf(stdout,"temporary\n");
    } else {
      leap_fprintf(stdout,"permanent\n");
    }
    leap_printf("Relation is ");
    if (system==TRUE) {
      leap_printf("a SYSTEM relation.\n");
    } else {
      leap_printf("a USER relation.\n");
    }

    do_debug(DEBUG_INFO,"No Attributes: %d\n",noattribs);

    leap_printf("Version tag: %d\n",leapver);

    totalsize=0;

    for (count=0; count<noattribs; count++) {

      res=fread(aname, ATTRIBUTE_NAME_SIZE, 1,fptr );

      if (res!=1) {
        leap_fprintf(stdout,"Error reading attribute name (# %d)\n",count);
        do_debug(DEBUG_ENTER,"EXITing (unsuccessfully) printrelinfo\n");
        return(RETURN_ERROR);
      }

      res=fread( &dtype,sizeof(dtype),1, fptr );

      if (res!=1) {
        leap_fprintf(stdout,"Error reading attribute type (# %d)\n",count);
        do_debug(DEBUG_ENTER,"EXITing (unsuccessfully) printrelinfo\n");
        return(RETURN_ERROR);
      }

      res=fread( &size, sizeof(size), 1,fptr );

      if (res!=1) {
        leap_fprintf(stdout,"Error reading attribute size (# %d)\n",count);
        do_debug(DEBUG_ENTER,"EXITing (unsuccessfully) printrelinfo\n");
        return(RETURN_ERROR);
      }

      res=fread( &keycomponent, sizeof(keycomponent), 1,fptr );

      if (res!=1) {
        leap_fprintf(stdout,"Error reading attribute key component (# %d)\n",count);
        do_debug(DEBUG_ENTER,"EXITing (unsuccessfully) printrelinfo\n");
        return(RETURN_ERROR);
      }

      leap_fprintf(stdout,"Attribute #%d - Name: <%s>\n",count,aname);
      leap_fprintf(stdout,"Attribute #%d - Type: <%d>\n",count,dtype);	
      types[count]=dtype;
      sizes[count]=size;
      totalsize+=size;
      leap_fprintf(stdout,"Attribute #%d - Size: <%d>\n",count,size);	
      leap_fprintf(stdout,"Attribute #%d - KeyC: <%d>\n\n",count,keycomponent);	
    }

    leap_fprintf(stdout,"Status marker:    %d bytes\n",sizeof(status));
    leap_fprintf(stdout,"Data size/tuple: %d bytes\n\n",totalsize);

    if (relation_read_eoh_marker( noattribs, &fptr )==RETURN_SUCCESS) {
      leap_fprintf(stdout,"Relation Structure read ok!\n");

      finish=FALSE;
      while (finish!=TRUE) {
        res=fread( &status, sizeof(status),1,fptr);
        if (res==0) {
          if (status_trace) leap_fprintf(stdout,"End of relation reached\n");
          finish=TRUE;
        } else {
          switch (status) {
            case TUPLE_OK:
              leap_fprintf(stdout,"[OK ] ");
              break;
            case TUPLE_DELETED:
              leap_fprintf(stdout,"[DEL] ");
              break;
            case TUPLE_SUSPECT:
              leap_fprintf(stdout,"[SUS] ");
              break;
            default:
              leap_fprintf(stdout,"[DEF] ");
              break;
          }
        }

        if (finish!=TRUE) {
          for (c=0; c<count; c++) {

            if (c>0) {
              leap_fprintf(stdout,"      ");
            }
            leap_printf("[%-4.4d] - ",c);
            leap_printf("[%-7.7d] - ",ftell(fptr));
            switch(types[c]) {
              case DT_STRING:
                rsize=sizes[c];
                leap_printf("[STR] [%-4.4d]",rsize);
                res=fread(string,rsize,1,fptr);
                string[rsize]='\0'; 
                break;
              case DT_NUMBER:
                rsize=sizeof(number);	
                leap_printf("[NUM] [%-4.4d]",rsize);
                res=fread(&number,rsize,1,fptr);
                break;
              case DT_BOOLEAN: 
                rsize=sizeof(bool);
                leap_printf("[BOO] [%-4.4d]",rsize);
                res=fread(&bool,rsize,1,fptr);
                break;
              default: 
                rsize=sizes[c];
                leap_printf("[DEF] [%-4.4d]",rsize);
                res=fread(string,rsize,1,fptr);
                string[rsize]='\0'; 
                break;
            }

            if (res!=1) {
              leap_printf("[Error!]");
            }

            switch ( types[c] ) {
              case DT_STRING:
                leap_printf("[%s]\n",string);
                break;
              case DT_NUMBER:
                leap_printf("[%d]\n",number);
                break;
              case DT_BOOLEAN:
                switch ( bool ) {
                  case TRUE: 
                    leap_printf("[TRUE]\n");
                    break;
                  case FALSE:
                    leap_printf("[FALSE]\n");
                    break;
                  default:    
                    leap_printf("[FALSE]\n");
                    break;
                }
                break;
              default:
                leap_printf(">>>%s<<<\n",string);
                break;
            }

          }
        }

        no_read_physical++;
      }
    } else {
      leap_printf("Relation Structure NOT read correctly!\n");
      do_debug(DEBUG_ENTER,"EXITing (unsuccessfully) printrelinfo\n");
      return(RETURN_ERROR);
    }

    fclose(fptr);
    do_debug(DEBUG_ENTER,"EXITing (successfully) printrelinfo\n");
    return(RETURN_SUCCESS);
  } else {
    leap_printf("Error opening file: %s\n",path);
    do_debug(DEBUG_ENTER,"EXITing (unsuccessfully) printrelinfo\n");
    return(RETURN_ERROR);
  }

}

attribute relation_attribute_read( FILE **fptr ) {
  /* relation_attribute_read
   * Read the next attribute in the record.
   * No pre-emptive error handling - Assumed controlling function 
   * knows how many attributes to expect!
   */

  attribute att;
  char aname[ATTRIBUTE_NAME_SIZE+1]; 
  ATTRIBUTE_TYPE_TYPE dtype;
  ATTRIBUTE_SIZE_TYPE size;
  ATTRIBUTE_KEYCOMP_TYPE keycomp;
  int res;

  do_debug(DEBUG_ENTER,"ENTERing relation_attribute_read\n");

  /* Create a ptr to an attribute structure */
  printf("relation_attribute_read(): sizeof(attribute_struct) = %d\n", sizeof(attribute_struct));
  att=(attribute_struct *) malloc(sizeof(attribute_struct));
  strncpy(attribute_name(att),"",ATTRIBUTE_NAME_SIZE);
  attribute_type(att)=0;
  attribute_size(att)=0;

  printf("relation_attribute_read(): entered\n");
  printf("relation_attribute_read(): ATTRIBUTE_NAME_SIZE = %d\n", ATTRIBUTE_NAME_SIZE);

  /* Check that the ptr was allocated. */
  check_assign(att,"tuples.relation_attribute_read");

  printf("relation_attribute_read(): fptr = %p\n", fptr);
  res=fread(aname, ATTRIBUTE_NAME_SIZE, 1, *fptr );
  printf("relation_attribute_read(): aname = %s\n", aname);
  printf("relation_attribute_read(): fread's res = %d\n", res);
  if (res!=1) {
    leap_printf("Error reading attribute name\n");
    free(att);
    do_debug(DEBUG_ENTER,"EXITing (unsuccessfully) relation_attribute_read\n");
    return(NULL);
  }

  res=fread( &dtype, sizeof(dtype),1, *fptr );
  if (res!=1) {
    leap_printf("Error reading attribute type)\n");
    free(att);
    do_debug(DEBUG_ENTER,"EXITing (unsuccessfully) relation_attribute_read\n");
    return(NULL);
  }

  res=fread( &size, sizeof(size), 1,*fptr );
  if (res!=1) {
    leap_printf("Error reading attribute size\n");
    free(att);
    do_debug(DEBUG_ENTER,"EXITing (unsuccessfully) relation_attribute_read\n");
    return(NULL);
  }

  res=fread( &keycomp,sizeof(keycomp),1, *fptr );
  if (res!=1) {
    leap_printf("Error reading attribute key component");
    free(att);
    do_debug(DEBUG_ENTER,"EXITing (unsuccessfully) relation_attribute_read\n");
    return(NULL);
  }

  strncpy(attribute_name(att),aname,ATTRIBUTE_NAME_SIZE);
  attribute_type(att)=dtype;
  attribute_size(att)=size;

  do_debug(DEBUG_ENTER,"EXITing (successfully) relation_attribute_read\n");
  return(att);

}

int tuple_readheader(FILE **fptr, unsigned char *noattributes, boolean *temp,
    boolean *system, char *rname) {

  char t[RELATION_NAME_SIZE+1];
  int res=0;
  unsigned int leapver;

  struct stat buf;
  int fd = fileno(*fptr);
  fstat(fd, &buf);
  printf("tuple_readheader(): filesize = %d\n", buf.st_size);
  printf("tuple_readheader(): mode = %o\n", buf.st_mode);
  printf("tuple_readheader(): uid = %d\n", buf.st_uid);
  printf("tuple_readheader(): gid = %d\n", buf.st_gid);
  printf("tuple_readheader(): seek test: %ld\n", ftell(*fptr));

  res=fread( &leapver,sizeof(leapver),1,*fptr);
  if (res!=1) {
    leap_printf("Error reading version number\n");
    /* Dispose of tuple structure - TODO!!! */
    do_debug(DEBUG_ENTER,"EXITing (unsuccessfully) tuple_readheader\n");
    return(RETURN_ERROR);
  }
  printf("tuple_readheader(): leapver = %d\n", (int)leapver);

  /* Read the relation name. Not needed, but to move the pointer... */
  /* TODO: fseek */
  res=fread( t,RELATION_NAME_SIZE,1, *fptr ); 
  strncpy(rname,t,RELATION_NAME_SIZE);
  printf("tuple_readheader(): rname = %s\n", rname);

  if (res!=1) {
    leap_printf("Error reading relation name\n");
    /* Dispose of tuple structure - TODO!!! */
    do_debug(DEBUG_ENTER,"EXITing (unsuccessfully) tuple_readheader\n");
    return(RETURN_ERROR);
  }

  /* Read the relation temporary stat. Not needed, but to move the pointer... */
  /* TODO: fseek */
  res=fread( temp, sizeof(*temp),1, *fptr ); 
  if (res!=1) {
    leap_printf("Error reading relation temporary status\n");
    /* Dispose of tuple structure - TODO!!! */
    do_debug(DEBUG_ENTER,"EXITing (unsuccessfully) tuple_readheader\n");
    return(RETURN_ERROR);
  }

  /* Read the relation system stat. Not needed, but to move the pointer... */
  /* TODO: fseek */
  res=fread( system,sizeof(*system),1, *fptr ); 
  if (res!=1) {
    leap_printf("Error reading relation system status\n");
    /* Dispose of tuple structure - TODO!!! */
    do_debug(DEBUG_ENTER,"EXITing (unsuccessfully) tuple_readheader\n");
    return(RETURN_ERROR);
  }

  res=fread( noattributes, sizeof(*noattributes),1, *fptr );
  if (res!=1) {
    leap_printf("Error reading number of attributes\n");
    /* Dispose of tuple structure - TODO!!! */
    do_debug(DEBUG_ENTER,"EXITing (unsuccessfully) tuple_readheader\n");
    return(RETURN_ERROR);
  }

  return(RETURN_SUCCESS);
}


FILE *generate_fileh( relation rel ) {

  FILE *fileh;
  char fname[FILE_PATH_SIZE+1];


  relation_full_path(rel, fname);
  strcat(fname,LEAP_NEW_RELATION_EXT);
  fileh=fopen(fname,"r+b");

  return(fileh);

}

tuple tuple_prepare(relation rel) {

  printf("tuple_prepare(): entered\n");
  printf("tuple_prepare(): rel->name = %s\n", rel->name);
  printf("tuple_prepare(): rel->filepath = %s\n", rel->filepath);
  printf("tuple_prepare(): rel->filename = %s\n", rel->filename);
  printf("tuple_prepare(): rel->fieldname = %s\n", rel->fieldname);
  /* tuple_prepare
   * Prepare a tuple by taking all of the attributes from
   * the specified relation, and populating it accordingly
   */
  NOATTRIBUTES_TYPE noattributes;
  RELATION_TEMP_TYPE temp;
  RELATION_SYSTEM_TYPE system;
  attribute att;
  word count;
  tuple ntuple;
  char fname[FILE_PATH_SIZE+1]="";
  char rname[RELATION_NAME_SIZE+1]="";

  strcpy(rname,"");
  strcpy(fname,"");

  do_debug(DEBUG_ENTER,"ENTERing tuple_prepare\n");

  /* Create the new tuple */
  ntuple=raw_tuple_prepare();

  tuple_f(ntuple)=generate_fileh(rel);

  if (tuple_f(ntuple)==NULL) {
    relation_full_path(rel,fname);
    strcat(fname,LEAP_NEW_RELATION_EXT);
    raise_error(ERROR_FILE_OPENING,NONFATAL,fname);
    do_debug(DEBUG_ENTER,"EXITing (unsuccessfully) tuple_prepare\n");
    return(NULL);
  }

  rel->current_pos=1;

  temp=1;
  system=1;
  noattributes=2;
  if (tuple_readheader(&tuple_f(ntuple), &noattributes, &temp, &system,rname)!=RETURN_SUCCESS) {
    return(NULL);
  }

  /* Populate relation structure to make things simpler now */
  rel->noattributes=noattributes;

  do_debug(DEBUG_INFO,"No Attributes: %d\n",noattributes);

  ntuple->noattributes=noattributes;

  printf("tuple_prepare(): noattributes = %d\n", noattributes);
  for (count=0;count<noattributes;count++) {

    att=relation_attribute_read( &tuple_f(ntuple) );
    printf("tuple_prepare() [%d]: att->name = %s\n", count, att->name);
    attribute_no(att)=count;

    tuple_attribute(ntuple,count)=att;
    tuple_relation(ntuple,count)=rel;
  }

  if (relation_read_eoh_marker( noattributes, &tuple_f(ntuple) ) != RETURN_SUCCESS ) {
    leap_printf("Error reading end of header marker\n");
    /* Dispose of tuple structure - TODO!!! */
    do_debug(DEBUG_ENTER,"EXITing (unsuccessfully) tuple_prepare\n");
    return(NULL);
  } else {
    do_debug(DEBUG_ACK,"Read relation definitions ok!\n");

    /* We've not necessarily populated the entire structure,
     * but one look at tuple_prepare should show us that the
     * structure is reset there, so no need to repeat the process
     * (as in the Pascal version)
     */

    /* Return the new tuple! */
    do_debug(DEBUG_ENTER,"EXITing (successfully) tuple_prepare\n");
    return(ntuple);
  }
}

attribute relation_attribute_getfirst(tuple ctuple, word *anum) {


  do_debug(DEBUG_ENTER,"ENTERing relation_attribute_GETfirst\n");

  /* Open an empty tuple for faster reading */
  if ((status_debug)&&(status_debuglevel>=DEBUG_INFO)) {
    tuple_def(ctuple);
  }

  if (ctuple!=NULL) {
    *anum=0;
    do_debug(DEBUG_ENTER,"EXITing (successfully) relation_attribute_GETfirst\n");
    return(tuple_attribute(ctuple,0));
  } else {
    do_debug(DEBUG_ENTER,"EXITing (unsuccessfully) relation_attribute_GETfirst\n");
    return(NULL);
  }
}

attribute relation_attribute_readfirst(relation rel,
    tuple *ctuple,
    word *anum) {


  do_debug(DEBUG_ENTER,"ENTERing relation_attribute_readfirst\n");

  /* Open an empty tuple for faster reading */
  *ctuple=tuple_prepare(rel);
  if ((status_debug)&&(status_debuglevel>=DEBUG_INFO)) {
    tuple_def(*ctuple);
  }

  if ((ctuple!=NULL) && (*ctuple!=NULL)) {

    *anum=0;
    do_debug(DEBUG_ENTER,"EXITing (successfully) relation_attribute_readfirst\n");
    return(tuple_attribute(*ctuple,0));

  } else {

    do_debug(DEBUG_ENTER,"EXITing (unsuccessfully) relation_attribute_readfirst\n");
    return(NULL);

  }
}

attribute relation_attribute_getnext( tuple ctuple,
    attribute cattr,
    word *anum){
  /* relation_attribute_GETnext 
   * Read the next attribute from the relation structure 
   */
  attribute nattr;

  /* Debug */
  do_debug(DEBUG_ENTER,"ENTERing relation_attribute_GETnext\n");

  /* Get the next attribute */
  nattr=tuple_attribute(ctuple,(*anum+1));

  /* Check it */
  if (nattr!=NULL) {

    /* Increment the attribute number */
    (*anum)++;

    /* Debug */
    do_debug(DEBUG_ENTER,"EXITing (successfully) relation_attribute_GETnext\n");

    /* Return the attribute */
    return(nattr);
  } else {
    /* End of attributes. Return NULL */
    *anum=0;
    do_debug(DEBUG_ENTER,"EXITing (successfully) relation_attribute_GETnext\n");
    return(NULL);
  }
}

attribute relation_attribute_readnext(relation rel,
    tuple *ctuple,
    attribute cattr,
    word *anum){
  /* relation_attribute_readnext 
   * Read the next attribute from the relation structure 
   */
  attribute nattr;

  /* Debug */
  do_debug(DEBUG_ENTER,"ENTERing relation_attribute_readnext\n");
#ifdef FULL_DEBUG
  leap_fprintf(stderr,"Have attribute %d\n",*anum);
#endif

  /* Get the next attribute */
  nattr=tuple_attribute(*ctuple,(*anum+1));

  /* Check it */
  if (nattr!=NULL) {

    /* Increment the attribute number */
    (*anum)++;

#ifdef FULL_DEBUG
    leap_fprintf(stderr,"Returning attribute %d\n",*anum);
#endif

    /* Debug */
    do_debug(DEBUG_ENTER,"EXITing (successfully) relation_attribute_readnext\n");

    /* Return the attribute */
    return(nattr);
  } else {
    /* Dispose of the tuple */
    /* NB. This is because a tuple is invariably needed to read all of the tuples */
    close_tuple(ctuple,TUPLE_DISPOSE);		
    /* tuple_dispose(ctuple);
     *ctuple=NULL;
     *anum=0;*/
    do_debug(DEBUG_ENTER,"EXITing (successfully) relation_attribute_readnext\n");
    return(NULL);
  }
}



int tuplesize( tuple *ctuple ) {
  /* tuplesize
   * Get the Size of the tuple */

  int count,noattribs,tsize=0;
  DT_BOOLEAN_TYPE bool;

  /* Home many attributes are in the tuple? */
  noattribs=(*ctuple)->noattributes;

  /* For each of them... */
  for (count=0; count<noattribs;count++) {

    /* Switch on the tuple */
    switch( tuple_attribute(*ctuple,count)->data_type ) {

      /* For each type, increment the tuple size counter */
      case DT_STRING:
        tsize+=tuple_attribute(*ctuple,count)->attrib_size;	
        break;

      case DT_NUMBER:
        tsize+=sizeof(DT_NUMBER_TYPE);
        break;

      case DT_BOOLEAN:
        tsize+=sizeof(bool);
        break;
      default: 
        tsize+=tuple_attribute(*ctuple,count)->attrib_size ;	
        break;
    }
  }

  /* Return the tuple size */
  return(tsize);
}

tuple tuple_readnext( tuple *ctuple,
    boolean reuse ) {
  /* tuple_readnext
   * Reads the next tuple record from the new format relation structure
   */
  NOATTRIBUTES_TYPE current;
  ATTRIBUTE_TYPE_TYPE type;
  char string[ATTRIBUTE_MAXIMUM_SIZE+1];
  char t2[ATTRIBUTE_MAXIMUM_SIZE+1];
  DT_NUMBER_TYPE number=0;
  DT_BOOLEAN_TYPE bool=FALSE,canreturn;
  TUPLE_STATUS_TYPE status=TUPLE_SUSPECT;
  int result=0;
  int size=0;

  /* Debug */
  do_debug(DEBUG_ENTER,"ENTERing tuple_readnext\n");

  /* Set the break out flag. This saves on ugly breaks which are not
   * particularly structured. This is (marginally) better
   */
  canreturn=FALSE;

  /* Whilst there is nothing to return... */
  while (canreturn==FALSE) {

    /* Get the current offset */
    tuple_offset(*ctuple)=ftell(tuple_f(*ctuple));

    /* Read the status value of the tuple */
    result=fread(&status,sizeof(status),1,tuple_f(*ctuple));

    /* If no block is returned, we are at the end of the relation */
    if (result==0) {
      /* Debug */
      do_debug(DEBUG_ACK,"End of relation reached!\n");

      /* Close the relation file */
      fclose(tuple_f(*ctuple));
      tuple_f(*ctuple)=NULL;

      /* Check that the tuple is not for reuse */
      if (reuse!=TUPLE_REUSE) {
        do_debug(DEBUG_MODERATE,"BEGIN Tuple dispose\n");
        /* It's not, so begone foul tuple! */
        close_tuple(ctuple,TUPLE_DISPOSE); 
        do_debug(DEBUG_MODERATE,"END Tuple dispose\n");
      } else {
        do_debug(DEBUG_ACK,"Tuple NOT disposed\n");
        /* Set the pointer to NULL to break out loops. If you're
         * planning on reusing the tuple, you better have another
         * pointer! 
         */
        *ctuple=NULL;
      }

      do_debug(DEBUG_ENTER,"EXITing (End of relation) tuple_readnext\n");

      /* Return NULL */
      return(NULL);

      /* NB - At some point, the return value should be NULL to indicate
       * end of a relation, so that only one ptr to a structure is
       * necessary. For now though, it's both. 
       */

    } else if (result!=1) {
      /* If something different was returned... report an error. */
      leap_printf("Error reading status of tuple\n");

      /* NB - This should probably be an abort. TBD. */
    }

    /* Was the tuple ok? */
    if (status==TUPLE_OK) {

      /* For all of the attributes in a tuple */
      for (current=0; ((*ctuple)&&(current<(*ctuple)->noattributes)); current++) {

        /* Get the attribute type for the current attribute */
        type=attribute_type( tuple_attribute( *ctuple, current ) );

        /* Switch on the type */
        switch ( type ) {
          case DT_STRING: 
            size=tuple_attribute( *ctuple, current )->attrib_size;
            result=fread(string,size,1,tuple_f(*ctuple));
            string[size]='\0';
            do_debug(DEBUG_MODERATE,"STR[%d:%d:%d][%s]\n",type,size,result,string);
            break;
          case DT_NUMBER: 
            size=sizeof(number);
            result=fread(&number,size,1,tuple_f(*ctuple));
            do_debug(DEBUG_MODERATE,"NUM[%d:%d:%d][%d]\n",type,size,result,number);
            break;
          case DT_BOOLEAN: 
            size=sizeof(bool);
            result=fread(&bool,size,1,tuple_f(*ctuple));
            do_debug(DEBUG_MODERATE,"BOO[%d:%d:%d][%d]\n",type,size,result,bool);
            break;
          default: 
            size=tuple_attribute( *ctuple, current )->attrib_size;
            result=fread(string,size,1,tuple_f(*ctuple));
            string[size]='\0';
            do_debug(DEBUG_MODERATE,"DEF[%d:%d:%d][%s]\n",type,size,result,string);
            break;
        }

        /* Check the result returned */
        if (result!=1) {
          leap_printf("Error reading tuple data from file!\n");
        } else {
          /* Something was returned */
          canreturn=TRUE;

          /* TODO: Return an error tuple */
        }

        /* Switch on the tuple for populating */
        switch ( type ) {
          case DT_STRING:
            tuple_data(*ctuple,current,string);
            break;
          case DT_NUMBER:
            sprintf(t2,"%d",number);
            tuple_data(*ctuple,current,t2);
            break;
          case DT_BOOLEAN:
            switch ( bool ) {
              case TRUE: 
                tuple_data(*ctuple,current,"TRUE");
                break;
              case FALSE:
                tuple_data(*ctuple,current,"FALSE");
                break;
              default:	
                tuple_data(*ctuple,current,"FALSE");
                break;
            }
            break;
          default:
            sprintf(t2,"%d",number);
            tuple_data(*ctuple,current,t2);
            /*tuple_data(*ctuple,current,string);*/
            break;
        }	
      }
      /* Increment the tuple no. */
      /* Only counts VALID tuples */
      tuple_no(*ctuple)++;
    } else {
      /* The tuple was deleted... Get the size of the tuple */
      size=tuplesize(ctuple);

      /* Debug */			
      do_debug(DEBUG_INFO,"Deleted Tuple Encountered - Skipping [%d] bytes (to offset [%d])\n",size,tuple_offset(*ctuple)+size+sizeof(status)+1);

      /* Skip the tuple */
      fseek(tuple_f(*ctuple),size,SEEK_CUR);
    }

    /* Increment the trace values */
    no_read_physical++;

  }

  do_debug(DEBUG_ENTER,"EXITing (with tuple) tuple_readnext\n");
  return(*ctuple);

}

tuple tuple_readfirst( relation rel,
    boolean reuse,
    tuple old_tuple ) {
  /* tuple_readfirst
   * Reads the first tuple from a new format relation structure
   */

  tuple ntuple=NULL;

  /* Debug */
  do_debug(DEBUG_ENTER,"ENTERing tuple_readfirst\n");

  rel->current_pos=1;

  /* Is the tuple to be reused? */
  if (reuse!=TUPLE_REUSE) {
    /* Nope. Create a new tuple */
    ntuple=tuple_prepare( rel );
  } else {
    /* Check the old tuple actually exists... */
    if (old_tuple==NULL) {
      /* Create a new tuple */
      ntuple=tuple_prepare( rel);
    } else {
      /* Use the old tuple */
      ntuple=old_tuple;
    }
  }

  if (ntuple!=NULL) {

    if (reuse==TUPLE_REUSE) {
      if (tuple_f(ntuple)==NULL) {
        /* Reopen the relation file */ 
        tuple_f(ntuple)=generate_fileh(rel);
      }

      /* Find the start position of the data */
      fseek(tuple_f(ntuple),ntuple->startofdata,SEEK_SET);
    } else {
      /* Get the start position of the data */
      ntuple->startofdata=ftell(tuple_f(ntuple));
    }

    /* Read the next (well, first) tuple... */
    tuple_readnext( &ntuple, reuse );
  } else {
    /* Debug */
    do_debug(DEBUG_ENTER,"EXITing (unsuccessfully) tuple_readfirst\n");
    return(NULL);
  }

  /* Debug */
  do_debug(DEBUG_ENTER,"EXITing tuple_readfirst\n");

  /* Return our (possibly) spanking new tuple all fresh and clean and
   * pure and innocent.
   */
  return(ntuple);	
}

int tuple_delete( word offset,
    tuple ctuple) {
  /* tuple_delete
   * Delete the current tuple
   */
  TUPLE_STATUS_TYPE status;
  word coffset;
  relation rel;
  char hashfile[FILE_PATH_SIZE+1],tuple_string[MAXIMUM_EXPRESSION+1];
  boolean success;

  /* Debug */
  do_debug(DEBUG_ENTER,"ENTERing tuple_delete\n");

  /* Get the relation to which this tuple relates */
  rel=get_relation(ctuple);

  /* Mark it updated. It'd be expensive to check as well (kind of) */
  rel->updated=TRUE;

  /* Load the hash table if it's not already present */
  if (tuple_relation(ctuple,0)->hash_table==NULL) {
    relation_full_path(tuple_relation(ctuple,0),hashfile);
    strcat(hashfile,LEAP_HASH_EXT);
    hashing_load(&rel->hash_table,hashfile); 
  }

  /* Build the hash key */
  tuple_to_string(ctuple,tuple_string);

#ifdef PEDANTIC_DELETE	
  /* Check the value isn't in the hash table */
  hashing_retrieve( tuple_relation(ctuple,0)->hash_table, tuple_string, found_string, &success);

  /* The tuple wasn't found in the relation!!! */
  if (success==FALSE) {
    if (status_debuglevel>=1) {
      raise_error(ERROR_TUPLE_ERASE,NONFATAL,tuple_string);
    }
    return(RETURN_ERROR);
  } else {

#endif		
    hashing_delete(rel->hash_table, tuple_string, &success);

#ifdef PEDANTIC_DELETE
    if (success==TRUE) {
#endif
      /* Find the current position */
      coffset=ftell(tuple_f(ctuple));

      /* Seek back the size passed in the offset variable */
      if (offset!=0) {
        fseek(tuple_f(ctuple),offset,SEEK_SET);
      }

      /* Set the status value */
      status=TUPLE_DELETED;
      do_debug(DEBUG_INFO,"Deleting tuple at offset %d\n",offset);

      /* Write the status value */
      fwrite(&status, sizeof(status),1, tuple_f(ctuple));
      fflush(tuple_f(ctuple));
      /* Update the number of tuples written */
      /* NB. This is not really a write, but what else is it? It's
       * updating data, and seeking about a tuple in length, so counts
       * as such really, until a nodeletes is available...
       */
      no_written++;

      do_debug(DEBUG_INFO,"Setting last_deleted to %d\n",offset);
      tuple_relation(ctuple,0)->last_deleted=offset;	
      do_debug(DEBUG_INFO,"Done\n",offset);

      /* Seek back to the original place */
      fseek(tuple_f(ctuple),coffset,SEEK_SET);

      /* Debug */
      do_debug(DEBUG_ENTER,"EXITing tuple_delete\n");

      return(RETURN_SUCCESS);
#ifdef PEDANTIC_DELETE

    } else {
      raise_error(ERROR_TUPLE_ERASE,NONFATAL,tuple_string);
      do_debug(DEBUG_INFO,"Weirdness: Found tuple, but couldn't delete it.");
    }
  }
#endif
}

int tuple_write( tuple ctuple ) {
  printf("tuple_write(): entered\n");
  /* tuple_write
   * Write a new tuple to the relation
   */
  TUPLE_STATUS_TYPE status;
  int counter;
  DT_BOOLEAN_TYPE bool;
  DT_NUMBER_TYPE numb;
  char string[ATTRIBUTE_MAXIMUM_SIZE+1],hashfile[FILE_PATH_SIZE+1],tuple_string[MAXIMUM_EXPRESSION+1];
  char found_string[MAXIMUM_EXPRESSION+1];
  int size,dtype,res;
  relation rel;
  boolean success;


  /* Debug */
  do_debug(DEBUG_ENTER,"ENTERing tuple_write\n");

  /* Get the relation to which this tuple relates */
  rel=get_relation(ctuple);

  /* Mark it updated. It'd be expensive to check as well (kind of) */
  rel->updated=TRUE;

  /* Load the hash table if it has not been loaded */
  if (tuple_relation(ctuple,0)->hash_table==NULL) {
    relation_full_path(tuple_relation(ctuple,0),hashfile);
    strcat(hashfile,LEAP_HASH_EXT);
    hashing_load(&rel->hash_table,hashfile); 
  }

  /* Build the hash key */
  tuple_to_string(ctuple,tuple_string);
  printf("tuple_write(): tuple_string = %s\n", tuple_string);

  /* Check the value isn't in the hash table */
  hashing_retrieve( tuple_relation(ctuple,0)->hash_table, tuple_string, found_string, &success);

  if (!success) {	
    /* Write an 'ok' marker */
    status=TUPLE_OK;
    fwrite(&status, sizeof(status),1, tuple_f(ctuple));

    /* For each of the attributes in the tuple */
    for (counter=0;((ctuple)&&(counter<ctuple->noattributes));counter++) {

      /* Get the data type */
      dtype= attribute_type(tuple_attribute( ctuple, counter )) ;	

      /* Switch on it */
      switch ( dtype ) {

        /* For the type, get the value from the tuple and write it */
        case DT_STRING: size=tuple_attribute(ctuple,counter)->attrib_size;
                        strncpy(string,tuple_d(ctuple,counter),size);
                        res=fwrite(string, size,1, tuple_f(ctuple));
                        break;
        case DT_NUMBER: numb=atoi(tuple_d(ctuple,counter));
                        size=sizeof(numb);
                        res=fwrite(&numb, size,1, tuple_f(ctuple));
                        break;
        case DT_BOOLEAN: if (strcmp(tuple_d(ctuple,counter),"TRUE")==0) {
                           bool=TRUE;
                         } else {
                           bool=FALSE;
                         }
                         size=sizeof(bool);
                         res=fwrite(&bool,size,1,tuple_f(ctuple));	
                         break;
        default: size=tuple_attribute(ctuple,counter)->attrib_size;
                 strncpy(string,tuple_d(ctuple,counter),size);
                 res=fwrite(string, size, 1, tuple_f(ctuple));
                 break;
      }

      /* Check that the value returned from fwrite matches the value
       * that *should* be returned.
       */
      if (res!=1) {
        leap_printf("Error writing tuple data!\n");
        return(RETURN_ERROR);
      }

    }

    /* Increment the trace values */
    no_written++;

    /* Insert the tuple into the hash table */
    hashing_insert((tuple_relation(ctuple,0))->hash_table,tuple_string,REQ_CALC);
  } else {
    if (status_debuglevel>=1) {
      raise_error(ERROR_DUPLICATE_ITEM,NONFATAL,tuple_string);
    }
  }	
  /* Debug */
  do_debug(DEBUG_ENTER,"EXITing tuple_write\n");

  return(RETURN_SUCCESS);
}

int tuple_appendandreturn( tuple ctuple ) {
  /* tuple_append
   * Write tuple specifically at the _end_ of a relation and return to original loc.
   */
  word	coffset;

  /* Find the current possition */
  coffset=ftell(tuple_f(ctuple));

  /* Seek the end of the file */
  fseek(tuple_f(ctuple),0, SEEK_END);

  /* Write the tuple */
  tuple_write( ctuple );

  fflush( tuple_f(ctuple) );

  /* Seek the original loc. */
  fseek(tuple_f(ctuple),coffset,SEEK_SET);

  return(RETURN_SUCCESS);
}

word getendposition( tuple ctuple ) {
  /* getendposition
   * get end record for updating...
   */
  word	coffset,epos;

  /* Find the current possition */
  coffset=ftell(tuple_f(ctuple));

  /* Seek the end of the file */
  fseek(tuple_f(ctuple),0, SEEK_END);

  epos=ftell(tuple_f(ctuple));

  /* Seek the original loc. */
  fseek(tuple_f(ctuple),coffset,SEEK_SET);

  return(epos);
}

boolean atend( tuple ctuple, word epos ) {
  /* atend
   * check if at the end of the relation?
   */

  word coffset;

  /* Find the current possition */
  coffset=ftell(tuple_f(ctuple));

  if (coffset>epos) return(TRUE);
  else return(FALSE);

}

int tuple_append( tuple ctuple ) {
  /* tuple_append
   * Write tuple specifically at the _end_ of a relation
   */

  /* Seek the last deleted position, or the end of the file */

  if ((tuple_relation(ctuple,0)->last_deleted)!=0)  {
    do_debug(DEBUG_INFO,"Seeking to last_deleted (%d)\n",tuple_relation(ctuple,0)->last_deleted);
    fseek(tuple_f(ctuple),0, tuple_relation(ctuple,0)->last_deleted);
    /* We reset last_deleted so we don't overwrite what may be
     * valid data. Could put a check to locate next DELETED status tuple, but
     * that would be expensive. better still would be a stack of deleted
     * tuples...
     */
    do_debug(DEBUG_INFO,"Resetting last_deleted to 0\n",tuple_relation(ctuple,0)->last_deleted);
    tuple_relation(ctuple,0)->last_deleted=0;
  } else {
    fseek(tuple_f(ctuple),0, SEEK_END); 
  }

  /* Write the tuple */
  return(tuple_write( ctuple ));
}				 


FILE *relation_open( relation rel, const char *mode) {

  FILE *fptr;
  char name[FILE_PATH_SIZE+1];

  sprintf(name,"%s%s",rel->filepath,rel->filename);

  printf("relation_open(): fopen %s, %s\n", name, mode);

  fptr=fopen(name,mode);

  return(fptr);
}

void dump_rel( relation rel) {
  /* dump_rel
   * dump relation details... for debugging.
   */

  char fname[FILE_PATH_SIZE+1];

  relation_full_path(rel,fname);

  /* Tac the .relation Extension 
  */
  strcat(fname,LEAP_NEW_RELATION_EXT);

  printrelinfo(fname);

}

relation_struct *build_new_relation( relation rel,	
    char *path ) {
  /* build_new_relation
   * Build a new relation structure from the specified tuple
   */

  attribute catt;
  FILE *fptr, *ofptr, *ctfile;
  short int noattributes=0;
  word ctnoattribs;
  tuple ct;
  int result;

  leap_fprintf(stderr,"Converting relation [%s] to new format... ",relation_name(rel));
  fflush(stderr);

  /* Open the attribute structure */
  catt=attribute_findfirst(rel, &fptr);	

  if ( catt!=NULL ) {

    /* Open output file */
    ofptr=fopen(path,"w+b");

    if (ofptr!=NULL) {

      noattributes=0;
      while ( (catt!=NULL) ) {
        noattributes++;

        /* Locate the next attribute */
        catt=attribute_findnext(catt,&fptr, FALSE, TRUE);
      }
      attribute_dispose(&catt); 

      result=relation_create_write_header(relation_name(rel),noattributes,FALSE,relation_system(rel),&ofptr);
      do_debug(DEBUG_MODERATE,"Writeheader: <%d>\n",result);

      /* Open the attribute structure */
      catt=attribute_findfirst(rel, &fptr);	

      while ( catt!=NULL ) {

        result=relation_create_write_attribute( catt, &ofptr );

        catt=attribute_findnext(catt,&fptr, FALSE, TRUE);
      }

      relation_create_write_eoh_marker( noattributes, &ofptr ); 

    }

    if (status_trace) {
      fclose(ofptr);

      printrelinfo(path);	 

      ofptr=fopen(path,"a+b");
    }

    /* There's no point converting the relation if it's going to be
     * rebuilt!
     */
    if (strcmp(relation_name(rel),LEAP_DD_RELATIONS)!=0) {
      ct=old_readfirst_tuple(rel, &ctfile, &ctnoattribs, TUPLE_BUILD, NULL);
      tuple_f(ct)=ofptr;
      while (ct!=NULL) {

        /* Cludge for new data item */
        ct->noattributes=noattributes;

        /* TODO: Check that there is data to write! */
        tuple_write( ct );	

        (void) old_readnext_tuple(&ct,&ctfile,&ctnoattribs,TUPLE_BUILD);
      }

      close_tuple(&ct,TUPLE_DISPOSE);
    } else {

      /* Close the output file - it's closed by the close tuple/reassignment */
      fclose(ofptr);

    }

    leap_fprintf(stderr,"Done.\n");

    return(rel);

  } else {
    return( NULL );
  }
}

relation buildnewrel( relation rel ) {

  return( build_new_relation( rel, "/tmp/test.dat" ));
}
