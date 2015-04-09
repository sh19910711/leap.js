/*
 * Name: util.c
 * Description: Utility Module - Contains all of the useful
 *		routines, and OS dependent stuff (where possible)
 * Version: util.c,v 1.208.2.1 2004/02/09 20:05:20 rleyton Exp
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
#include <time.h>
#include <signal.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>

/* Taken from autoconf.info */
#ifndef __MSDOS__
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
#else
#include <dirent.h>
#endif



#include "consts.h"
#include "globals.h"
#include "dtypes.h"
#include "util.h"
#include "dbase.h"
#include "vars.h"
#include "leapio.h"
#include "errors.h"
#include "parser.h"
#include "relation.h"

/* The definitions that were declared in the header file */
char tempdir[FILE_PATH_SIZE+1];
char scratchfile[FILE_PATH_SIZE+1];

/* Status flag */
boolean status=FALSE;

/* The error file */
char ERROR_FILE[FILE_PATH_SIZE+1];

/* The report file - for tracing */
FILE *REPORT_FILE=NULL;

/* Global var. for EOF at strange places... */
/* Could occur in a "press any key" type prompt...*/
boolean global_eof=FALSE;

char LEAP_BASE_DIR[FILE_PATH_SIZE+1];

/* This is just a holdall for impromptu sprintf calls. */
char temp_80_chars[80];

/*
 * Commands available - THEY MUST BE IN ALPHABETICAL ORDER
 * IN ORDER TO MAKE SEARCHING MORE EFFICENT (Binary chop
 * is used) (K&R again!)
 */
struct commands_struct commands[] = {
	{"!", C_DOS},
	{"#", C_COMMENT},
	{"##", C_COMMENT},
	{"###", C_COMMENT},
	{";", C_COMMENT}, /* Not a comment, but treat it as such if the parser 
					   * should get hold of it somehow. */
	{">", C_FILE_COMMENT},
	{"?", C_HELP},
	{"@", C_SRCFILE},
	{"add", C_ADD},
	{"addresses", C_ADDRESSES},
	{"average", C_SUMMARISE_AVG},
	{"avg", C_SUMMARISE_AVG},
	{"break", C_BREAK},
	{"cache", C_CACHE},
	{"case", C_CASE},
	{"change", C_CHANGE},
	{"clear", C_CLEAR},
	{"compact", C_COMPACT},
	{"copying", C_INFO},
	{"create", C_CREATE},
	{"debug", C_DEBUG},	
	{"delete", C_DELETE},
	{"delrel", C_DELREL},
	{"describe", C_DESCRIBE},
	{"difference", C_DIFFERENCE},
	{"dir", C_DIR},
	{"display", C_PRINT},
	{"dispose", C_DISPOSE},
	{"dump", C_DUMP},
	{"duplicate", C_DUPLICATE},
	{"erase", C_DELETE},
	{"exec", C_SRCFILE},
	{"exit", C_EXIT},
	{"flush", C_FLUSH},
	{"help", C_HELP},
	{"high", C_HIGH},
	{"idxprint", C_PRINT_IDX},
	{"idxstore", C_IDX_STORE},
	{"indexes", C_DISPLAY_INDEX},
	{"indices", C_DISPLAY_INDEX},
	{"infix", C_INFIX},
	{"internal", C_INTERNAL},
	{"intersect", C_INTERSECT},
	{"ioon", C_IOON},
	{"iterative", C_ITERATIVE},
	{"join", C_JOIN},
	{"l", C_LISTSRC},
	{"list", C_DISPLAY_REL},
	{"load", C_LOAD},
	{"mem", C_MEM},
	{"minus", C_DIFFERENCE},
	{"natjoin", C_NATURAL_JOIN},
	{"normal", C_NORMAL},
	{"panic", C_PANIC},
	{"parse", C_PARSE},
	{"print", C_PRINT},
	{"product", C_PRODUCT},
	{"project", C_PROJECT},
	{"prompt", C_PROMPT},
	{"quit", C_EXIT},
	{"record", C_RECORD},
	{"relation", C_CREATE_RELATION},
	{"relations", C_DISPLAY_REL},
	{"rename", C_RENAME},
	{"report", C_REPORT},
	{"restrict", C_SELECT},
	{"reverse", C_REVERSE_ENG},
	{"rmvtmp", C_RMVTMP},
	{"select", C_SELECT},
	{"set", C_SET},
	{"smjoin", C_SMJOIN},
	{"source", C_SRCFILE},
	{"sources", C_LIST},
	{"sp_help", C_DISPLAY_REL},
	{"specidx", C_IDX},
	{"status", C_STATUS},
	{"stop", C_EXIT},
	{"sum", C_SUMMARISE_SUM},
	{"summarise", C_SUMMARISE},
	{"summarize", C_SUMMARISE},
	{"terminatenow", C_TERMINATENOW},
	{"timing", C_TIMING},
	{"union", C_UNION},
	{"update", C_UPDATE},
	{"use", C_USE},
	{"ustime", C_US},
	{"variables", C_SHOWVARS},
	{"vars", C_SHOWVARS},
	{"ver", C_VERSION},
	{"version", C_VERSION},
	{"warranty", C_WARRANTY},
	{"what", C_WHAT}
};



/* Define the size of the above structure 
 * (See K&R 2nd Ed. pg 135 
 */
#define NKEYS (sizeof commands / sizeof commands[0])


/* Define various status flags */
boolean status_trace=DEFAULT_STATUS_TRACE;
boolean status_debug=DEFAULT_STATUS_DEBUG;
int status_debuglevel=DEFAULT_STATUS_DEBUG_LEVEL;
int status_mindebuglevel=DEFAULT_STATUS_MINDEBUG_LEVEL;
boolean status_timing=DEFAULT_STATUS_TIMING;
boolean status_case=DEFAULT_STATUS_CASE;
boolean status_quiet=DEFAULT_STATUS_QUIET;
boolean status_regression=DEFAULT_STATUS_REGRESSION;
boolean status_temporary_relations=DEFAULT_STATUS_TRELS;
boolean status_timelog=DEFAULT_STATUS_TIMELOG;
boolean status_longline=DEFAULT_STATUS_LONGLINE;
boolean status_padding=DEFAULT_STATUS_PADDING;
boolean status_tempdb=DEFAULT_STATUS_TEMPDB;
boolean status_productjoin=DEFAULT_STATUS_PRODUCTJOIN;
boolean status_daemon=DEFAULT_STATUS_DAEMON;
boolean status_merge_stderr=DEFAULT_STATUS_DAEMON;

char *generate_random_string(word size,
                 char *pstring) {
/* generate_random_string
 * Generates a string of 'size' random(ish) characters
 */
    char *string;
    int counter;

    /* Allocate the memory - +1 for null. */
#ifdef FULL_DEBUG
    leap_fprintf(stderr,"size: %d\n",size);
#endif
    if (pstring==NULL) {
        /* If the string is not allocated, allocate some
         * memory! */
        string=malloc(size+1);
    } else {
        string=pstring;
    }


    for(counter=0;( (unsigned int) counter)<size;counter++) {
        string[counter]=( (rand() % 26)+97);
#ifdef FULL_DEBUG
    leap_fprintf(stderr,"counter: %d\nstring: %s\n",counter,string);
#endif
    }

    string[size]='\0';

    /* Add an 'zz' at the beginning to make it a little
     * easier to find the LEAP temporary files...
     */
    string[0]='z';
    string[1]='z';

    return(string);
}

void util_close(void) {
/* util_close
 * Close files and stuff
 */
	int error;
	time_t tp;

	if (ACTIVITY_FILE!=NULL) {
		tp=time(NULL);
		fprintf(ACTIVITY_FILE,"###\n# Activity file STOPPED at: %s###\n",ctime(&tp));

		error=fclose(ACTIVITY_FILE);
		if (error==EOF) {
			raise_error(ERROR_CLOSING_FILE,NONFATAL,"Activity File");
		}
	}
}


void build_base_dir( char *directory ) {
/* build_base_dir
 * Build the global variables for the base directory. Putting this
 * in a routine makes sense, so we can try various options if a 
 * file is not found.
 */

	/* Do the initial configuration that is necessary */
	strncpy(LEAP_BASE_DIR,directory,FILE_PATH_SIZE);

	/* Check that the directory specified contains a seperator
	 * at the end, because we mindlessly append directories to
	 * this, and that causes icky problems...
	 */
	if (LEAP_BASE_DIR[strlen(LEAP_BASE_DIR)-1]!=DIR_SEPERATOR) {
		strcat(LEAP_BASE_DIR,DIR_SEPERATOR_STRING);
	}

	/* Build the error file file name */
	sprintf(ERROR_FILE,"%s%s%s",LEAP_BASE_DIR,LEAP_ERROR_DIR,LEAP_ERROR_FILE);

}

void util_init() {
/* util_init
 * This routine sets up any global variables for the utilities
 * unit, for example, the full path to the errors file, to
 * save work later.
 */
	char fname[FILE_PATH_SIZE+1];
	char randomstr[7];

	/* Setup temporary directory - it may be specified on the command line (-x) */
	if (strlen(tempdir)==0) {
		strcpy(tempdir,LEAP_TEMP_DIR);
	}

	if (tempdir[strlen(tempdir)-1]!=DIR_SEPERATOR) {
		strcat(tempdir,DIR_SEPERATOR_STRING);
	}

	sprintf(scratchfile,"%s%s",tempdir,generate_random_string(6,randomstr));

	do_debug(DEBUG_INFO,"Temporary directory: [%s]\nScratch file: [%s]\n",tempdir,scratchfile);

	do_debug(DEBUG_INFO,"Error file: %s\t Report file: %s\n",ERROR_FILE,fname);
}

void check_assign(void *ptr, char *string) {
/* check_assign
 * Checks whether a pointer is assigned or not. If not, then
 * memory has probably run out.
 */
	if (ptr==NULL) {
		raise_error(ERROR_INSUFFICENT_MEMORY,FATAL,string);
	} 
}


void do_trace(char *trace_string) {
/* do_trace
 * Reports something to the screen, and if set, to the report
 * file as well 
 */
	if ((status_trace) || (status_debuglevel>=DEBUG_ENTER)) raise_event(MESSAGE,trace_string);
}


char *cut_to_right_bracket( char *s,
			   int bdepth, int force,
			   char *result) {
/* cut_to_right_bracket
 * This should return a ptr to an expression contained
 * within the brackets of depth bdepth in *result and as result
 * new for new parser.
 * if force==TRUE then force brackets onto the string, else
 * don't - and return nothing if no brackets exist.
 */

	struct bracket {
		int pos,depth;
	} bracket[MAXIMUM_BRACKETS];

	boolean bexist=FALSE;
	int start,finish,counter,c,no,depth;
	char rstr[MAXIMUM_EXPRESSION+1],tstr[MAXIMUM_EXPRESSION+1];
	char *tstrp,*endstart,*beginning,*ostart;
	char removed[MAXIMUM_EXPRESSION+1];
	int slen=0;

	/* Check that the parameters are ok */
	if ( (s==NULL) || (strlen(s)==0) ) {
		strcpy(result,"");
		return(result);
	}

	slen=strlen(s);

	/* Reset the data */		
	for (counter=0;counter<MAXIMUM_BRACKETS;counter++) bracket[counter].pos=bracket[counter].depth=0;
	
	bracket[0].pos=0;
	bracket[0].depth=1;
	bracket[1].pos=slen+1;
	bracket[1].depth=1;
	c=0;
	depth=1;
	no=0;

	while ( ((unsigned int) c) <=slen) {
		if (s[c]=='(') {
			bracket[no].pos=c;
			bracket[no].depth=depth;

			/* Increment the bracket 'depth' counter */
			depth++;

			/* Set the brackets-exist flag */
			bexist=TRUE;
			no++;
		}

		if (s[c]==')') {

			depth--;

			bracket[no].pos=c;
			bracket[no].depth=depth;
			no++;
		}

		c++;
	}

	/* If the specified depth is greater than the maximum depth
	 * of the brackets in the given string, then set the parameter
	 * to the maximum depth possible 
	 */
	if (bdepth>depth) {
		bdepth=depth;
	}

	/* If no brackets occur in the string */
	/* ...and they can be forced on */
	if ( (!bexist) && (force==TRUE) ) {
		/* Add 'em */
		/* sprintf(tstr,"(%s)",s); */

		/* Test... */
		strcpy(result,s);
		strcpy(s,"");

		/* And copy it back */
		/* strcpy(s,tstr); */

		return(result);

	/* If no brackets occur in the string */
	/* ...but they cannot be forced on */
	} else if ( (!bexist) && (force==FALSE) ) {

		/* Return an empty string */
		strcpy(result,"\0");
		return(result);

	/* ELSE  brackets occur in the string, so
	 * carry on - brackets are there 
   	 */
	} else {
		strncpy(tstr,s,MAXIMUM_EXPRESSION);
	}

	ostart=tstr;

	strncpy(rstr,"",MAXIMUM_EXPRESSION);

	c=0;
	while (bracket[c].depth<bdepth) c++;
	
	start=c;

	c++;

	while (bracket[c].depth>bdepth) c++;
	finish=c;


	/* tstr still contains s - so it can be played with */	
	tstr[(bracket[finish].pos)]='\0';
	tstrp=&(tstr[(bracket[start].pos)+1]);
	strcpy(result,tstrp);

	/* This puts a null at bracket point */
	tstrp=&tstr[(bracket[start].pos)];
	*tstrp='\0';

	/* This gets the start of the string after the bracket */
	endstart=&tstr[(bracket[finish].pos)+1];
	beginning=ostart;

	/* And this puts the beginning and end bits together, to
	 * give a string without the bracketed bit
	 */
	sprintf(removed,"%s%s",beginning,endstart);

	/* Bugger... */
	if (strlen(removed)==0) {
		*s=(int) NULL;
	} else {
		strncpy(s,removed,MAXIMUM_EXPRESSION);
	}
#ifdef DEBUG
	leap_fprintf(stderr,"Retrieved: >%s<\n",result);
#endif	
	return(result);	
}

char *get_token(char *string,
	        char seperator,
		char *result) {
/* get_token
 * Gets the first token from the string, and returns
 * it (in result, and as a result)
 */
	char seperator_list[MAX_SEPERATORS];
	char *rptr;

	/* If sepereator is NULL, then use the normal seperators */
	if (seperator=='\0') {
		strcpy(seperator_list,TOKEN_SEPERATORS);
	} else {
		/* Otherwise, use the character specified */
		sprintf(seperator_list,"%c",seperator);
	}
	/* Copy the original string into "result" */
	strcpy(result,string);

	/* Locate the first symbol - strtok puts a null at
 	 * the location of the seperator, so we can then return
	 * the string, with the first item located. strtok returns
	 * NULL if no seperator is found. 
	 */
	rptr=strtok(result,seperator_list);

	/* If there is no token, then make the string empty */
	/* (Can't return NULL, incase used in strcpy) */
	if (rptr==NULL) result[0]='\0';

	/* Return the result */
	return(result);

}	

char *allbut(char *string, char *chars) {
/* allbut
 * return string with contents of *string without
 * any chars specified in *chars
 */
	char *newstring;
	int ilength,length,count,icount,ocount;
	boolean found=FALSE;


	length=strlen(string);
	ilength=strlen(chars);

	newstring=malloc(length+1);

	ocount=0;

	/* Loop through the original string */
	for (count=0; count<length; count++) {
		
		/* Nothing found yet */
		found=FALSE;

		/* Loop throught the tokens, whilst nothing found... */
		for (icount=0; ((found==FALSE)&&(icount<ilength)); icount++) {

			/* If something is found */
			if (string[count]==chars[icount]) {
				found=TRUE;	
			}

		}

		/* Nothing was found... */
		if (found==FALSE) {
			newstring[ocount]=string[count];
			ocount++;
		}
	}	

	newstring[ocount]='\0';
	return(newstring);
}

char *cut_token(char *string,
	        char seperator,
   		    char *result) {
/* cut_token
 * Cuts the first token from the string, and returns
 * it (in result, and as a result)
 */
	char seperator_list[MAX_SEPERATORS];
	char *rptr;
	size_t length;

	/* If seperator is NULL, then use the normal seperators */
	if (seperator=='\0') {
		strcpy(seperator_list,TOKEN_SEPERATORS);
	} else {
		/* Otherwise, use the character specified */
		sprintf(seperator_list,"%c",seperator);
	}

	/* Copy the original string into "result" */
	strcpy(result,string);
	
	/* Locate the first symbol - strtok puts a null at
 	 * the location of the seperator, so we can then return
	 * the string, with the first item located. strtok returns
	 * NULL if no seperator is found. 
	 */
	rptr=strtok(result,seperator_list);

	/* If there is no token, then make the string empty */
	/* (Can't return NULL, incase used in strcpy) */
	if (rptr==NULL) {
		result[0]='\0';
	} else {
		/* Get the length of the first token, to 
		 * indicate where the seperator may be found 
		 */
		length=strlen(rptr);

		/* Copy into the source string the remainder,
		 * so that the string is "cut" out
		 */

		/* BUT first, skip over any spaces... They're never
		 * going to be any use */
		while ((string[length]==' ') || (string[length]==',')){
			length++;
		}

		strcpy(string,&string[length]);
	}

	/* Return the result */
	return(result);

}	

char *skip_to_alnum(char *string ) {

	int pos=0;

	while ((string[pos]!='\0')&&(!isalnum(string[pos]))) pos++;

	return(&string[pos]);

	
}

int binsearch(char *word, struct commands_struct commands[], int n) {
	int cond;
	int low, high, mid;

	low=0;
	high=n-1;

	while (low <= high) {
		mid=(low+high)/2;
		if ( (cond = strcmp(word, commands[mid].text)) < 0)
			high=mid-1;
		else if (cond>0)
			low=mid+1;
		else
			return mid;
	}
	return -1;
}

int get_command(char *word) {
	int result;
	int counter=0;

	/* skip over spaces */
	while (word[counter]==' ') counter++;
	word=&word[counter];

	result=binsearch(word,commands,NKEYS);

	if (result!=C_UNKNOWN) {
		return( commands[result].command );
	} else {
		return( C_UNKNOWN );
	}
}

void assign_input_stream( char *filen ) {
/* assign_input_stream
 * This is used to assign the input stream (input_stream)
 * declared as extern in dtypes, and in the main module, to
 * the specified file. If empty, then the file is reset to stdin.
 */
	char temp[100];
	FILE *ninputstream;
	
	do_debug(DEBUG_ENTER,"ENTERing assign_input_stream [%s].\n",filen);

	if (strlen(filen)==0) {
		do_trace("Setting source file to stdin.");
		if (input_stream!=NULL) fclose(input_stream);
		input_stream=stdin;
	} else {
		sprintf(temp,"Setting source file to %s.",filen);
		do_trace(temp);
		ninputstream=fopen(filen,"r");
		if (ninputstream==NULL) {
			input_stream=stdin;
			raise_error(ERROR_FILE_OPENING,NONFATAL,filen);
			leap_printf("Resetting input file\n");
		} else {
			input_stream=ninputstream;
		}
	}

	do_debug(DEBUG_ENTER,"EXITing assign_input_stream.\n");
}

char *find_start_of_data( char *string) {
/* Scans over a string, and locates the first text. */
	char *sptr;

	/* Start at the start! */
	sptr=string;

	/* Whilst the string is not null, and
	 * not equal to a character 
	 */
	while ( (*sptr!='\0') &&
		(    ( (*sptr<'A') || (*sptr>'Z') )
                  && ( (*sptr<'a') || (*sptr>'z') )
		) ) {

		/* Skip over to the next pointer */
		sptr++;

	}

	/* If we have found the end of the string... */
	if (*sptr=='\0') {
		
		/* Return a NULL string */
		return(NULL);
	} else {

		/* Otherwise, return the start of the data */
		return(sptr);
	}

}

void list_source_code( ) {
/* list_source_code
 * Prints out the source files in a database.
 */

	struct dirent *d;
	DIR *directory;
	char dirpath[FILE_PATH_SIZE+1],filename[FILE_PATH_SIZE+1],*cptr;

	/* Build the directory path */
	sprintf(dirpath,"%s%s",database_dir(current_db),LEAP_SOURCE_DIR);

	/* Open the directory */
	directory=opendir(dirpath);

	/* Read the first entry */
	d=readdir(directory);

	leap_printf("Source files:\n");
	leap_printf("-------------\n");

	/* Whilst there are directory entries to process */	
	while (d!=NULL) {

		/* Check if the entry contains the extension signifying
		 * it's a source file 
		 */

#ifdef __MSDOS__
		/* MSDOS returns the filename in uppercase, so
		 * we should convert it to lower case. The alternative
		 * is to do the comparison in a compiler directive, to
		 * save pre-converting items that don't contain the
		 * string.
		 */
	downcase(d->d_name);
#endif
		if (strstr(d->d_name,LEAP_SOURCE_EXT)!=NULL) {
			/* Make a copy of the filename, so we don't
			 * risk messing up things out of our control
			 */
			strcpy(filename,d->d_name);

			/* Terminate where the extension starts */
			cptr=strstr(filename,LEAP_SOURCE_EXT);
			*cptr='\0';

			leap_printf("%s\n",filename);
		}

		/* Read the next directory entry */
		d=readdir(directory);
	}

	closedir(directory);

}

void print_source_code( char *file_name ) {
/* print_source_code
 * Prints the specified source file name. Assumes the .src extension
 * is not specified.
 */
	char dirpath[FILE_PATH_SIZE+1],source_line[MAXIMUM_EXPRESSION];
	char *read_status;
	FILE *fptr;
	int x;

	/* Build the directory path */
	sprintf(dirpath,"%s%s%s%s",database_dir(current_db),LEAP_SOURCE_DIR,file_name,LEAP_SOURCE_EXT);

	/* Open the file */
	fptr=fopen(dirpath,"r");

	/* If the file could not be found */
	if (fptr==NULL) {
			/* Report an error */
			raise_error(ERROR_FILE_OPENING,NONFATAL,file_name);
	} else {

		/* No error occured - so read the first line */
		read_status=fgets(source_line,MAXIMUM_EXPRESSION,fptr);

		if (read_status!=NULL) {
			leap_fprintf(stdout,"Source File: %s\n",file_name);
			leap_fprintf(stdout,"-------------");
			for (x=0; x<strlen(file_name); x++) leap_fprintf(stdout,"-"); 		
			leap_fprintf(stdout,"\n");
		}

		/* Whilst no error occurs reading from the file */
		while( read_status!=NULL ) {

			/* Display the current line */
			/* Includes cr, so skip that and use write */
			leap_fprintf(stdout,source_line);

			/* Get the next line */
			read_status=fgets(source_line,MAXIMUM_EXPRESSION,fptr);
		}
		leap_printf("<EOF>\n");

		fclose(fptr);
	}
}


void reverse_source_code() {
/* reverse_source_code
 * Reverse out the source code in a directory.
 */
    char dirpath[FILE_PATH_SIZE+1],source_line[MAXIMUM_EXPRESSION];
    char *read_status, ofname[FILE_PATH_SIZE+1], filename[FILE_PATH_SIZE+1],*cptr;
    FILE *fptr;
	DIR *directory;
	struct dirent *d;

    /* Build the directory path */
    sprintf(dirpath,"%s%s",database_dir(current_db),LEAP_SOURCE_DIR);
	
    /* Open the directory */
    directory=opendir(dirpath);

    /* Read the first entry */
    d=readdir(directory);

    /* Whilst there are directory entries to process */
    while (d!=NULL) {

        /* Check if the entry contains the extension signifying
         * it's a source file 
         */

#ifdef __MSDOS__
        /* MSDOS returns the filename in uppercase, so
         * we should convert it to lower case. The alternative
         * is to do the comparison in a compiler directive, to
         * save pre-converting items that don't contain the
         * string.
         */
    downcase(d->d_name);
#endif
        if (strstr(d->d_name,LEAP_SOURCE_EXT)!=NULL) {
            /* Make a copy of the filename, so we don't
             * risk messing up things out of our control
             */
            strcpy(filename,d->d_name);

			sprintf(ofname,"%s%s",dirpath,filename);

            /* Terminate where the extension starts */
            cptr=strstr(filename,LEAP_SOURCE_EXT);
            *cptr='\0';

            leap_printf("record %s\n",filename);

    		/* Open the file */
   			fptr=fopen(ofname,"r");
            /* If the file could not be found */
            if (fptr==NULL) {
                /* Report an error */
                raise_error(ERROR_FILE_OPENING,NONFATAL,filename);
            } else {

                /* No error occured - so read the first line */
                read_status=fgets(source_line,MAXIMUM_EXPRESSION,fptr);

                /* Whilst no error occurs reading from the file */
                while( read_status!=NULL ) {

                    /* Display the current line */
                    /* Includes cr, so skip that and use write */
                    leap_printf(source_line);

                    /* Get the next line */
                    read_status=fgets(source_line,MAXIMUM_EXPRESSION,fptr);
                }

                leap_printf(".\n");

                fclose(fptr);

            }

		}

        /* Read the next directory entry */
        d=readdir(directory);
    }

    closedir(directory);

}
			
void upcase(char *string) {
/* upcase
 * Converts the specified string to upper case
 */

	int counter;
	char *s;

	for (counter=0; counter<strlen(string); counter++) {

		s=&string[counter];

		if ( (*s>='a') && (*s<='z') ) {
			*s-=32;
		}
	}
}

void downcase(char *string) {
/* down
 * Converts the specified string to lower case
 */

	int counter;
	char *s;

	for (counter=0; counter<strlen(string); counter++) {

		s=&string[counter];

		if ( (*s>='A') && (*s<='Z') ) {
			*s+=32;
		}
	}
}

void print_helppage( char *page ) {
/* print_helppage
 * Prints the help page specified, or the introduction/summary
 * page in the absence of a value
 */
	FILE *helpfile;
	char helpfile_name[FILE_PATH_SIZE+1],line[HELP_PAGE_LINE_SIZE+1],*lptr,*sptr;
	int counter,tabcounter=0,introcount=0;
	boolean finished=FALSE,printing=FALSE,summary=FALSE,intro=FALSE;

	leap_fprintf(stdout,"Help page: %s\n",page);

	/* Build the help file name */
	sprintf(helpfile_name,"%s%s%s",LEAP_BASE_DIR,LEAP_HELP_DIR,LEAP_HELP_FILE);

	/* Get a file ptr */
	helpfile=fopen(helpfile_name,"r");

	if (helpfile!=NULL) {
		/* File is ok */

		upcase(page);

		/* Is the user after a summary? */
		summary=(strcmp(page,HELP_SUMMARY)==0);

		/* If no help segment was specified */
		if (strlen(page)==0) {
			printing=TRUE;
			intro=TRUE;
		}

		do {
			/* Read the line in */
			lptr=fgets( line, HELP_PAGE_LINE_SIZE, helpfile );

			/* Whilst the page has not been located, and a line has been read */

			/* Change the newline to a null */
			line[strlen(line)-1]='\0';

			if (line[0]==HELP_PAGE_COMMENT) {

			} else if (line[0]==HELP_PAGE_START) {

				/* Get the length of the line */
				counter=strlen(line)-1;

				/* Scan back to the first non-space */
				while (line[counter]==' ') counter--;

				/* Mark the end of the DATA */
				line[counter+1]='\0';

				/* Get the start of the actual header... */
				sptr=&line[2];

				/* If this is the help page we are after */
				if (strcmp(sptr,page)==0) {
					/* Enable printing */

					/* because of nested if, this will
					 * take effect next iteration 
					 */

					printing=TRUE;
				} else if (summary==TRUE) {
					leap_fprintf(stdout,"%s",sptr);
					for (counter=strlen(sptr);counter<20;counter++) leap_fprintf(stdout," ");
					tabcounter++;
					if (tabcounter>=HELP_ITEMS) {
						tabcounter=0;
						leap_fprintf(stdout,"\n");
					}

					fflush(stdout);
				}
			} else if ( (printing==TRUE) && (line[0]==HELP_PAGE_END) ) { 

				/* If printing true to prevent reading of
				 * page seperators in first block 
				 */
				/* If this is the end of the help page
				 * Disable printing, and ensure the
				 * loop terminates neatly 
				 */

				if ( (intro==TRUE) && (introcount<3) ) {
					introcount++;
				} else {
					printing=FALSE;
					finished=TRUE;
				}

			} else if (printing==TRUE) {

				/* If printing is on, then print the current
				 * line
				 */
				leap_printf("%s\n",line);
			} 


		} while ( (finished!=TRUE) && (lptr!=NULL) );

		leap_printf("\n");

		/* Close the file */	
		fclose(helpfile);	

	} else {
		/* No file ptr */

		raise_error(ERROR_FILE_OPENING,NONFATAL,helpfile_name);

	}
}

int set_status( char *status_option, char *value ) {
/* set_status
 * Sets status flags on or off
 */
	char *rval;

		rval=set_variable(status_option,value);

		/* Success? */
		if (rval==NULL) {
			/* Create a new variable - error code/message
			 * if cannot be done 
			 */
			return(new_variable(status_option,value));
		} else {
			/* Success! */
			return(RETURN_SUCCESS);
		}
}

/* Macro to do all the hard work.
 * All this does is evaluate value. If true, then prints "on",
 * otherwise, prints off. Take a look at show_status calls
 * to the macro, and all should be clear...
 */

/* For util.c use only... */
static char backup_prompt[MAXIMUM_PROMPT+1];
static boolean prompt_change;

void set_prompt(char *prompt) {
/* set_prompt
 * Sets the prompt to that specified
 */
	strcpy(backup_prompt,current_prompt);

	strcpy(current_prompt,prompt);

	prompt_change=TRUE;
}

void unset_prompt() {
/* unset_prompt
 * Sets the prompt back to the default
 */
	if (prompt_change==TRUE) {
		strcpy(current_prompt,backup_prompt);
		prompt_change=FALSE;
	} 
}

char *copy_to_token( char *source,
					 char *delimiter,
					 char *destination ) {
/* copy_to_token
 * Tokenises source using strtok, and copies to destination
 */
	char *sptr;

	sptr=strtok(source,delimiter);
	if (sptr!=NULL) {
		strcpy(destination,sptr);
		return(destination);
	} else {
		return(NULL);
	}
}

void strip_leading_spaces(char *source) {
/* strip_leading_spaces
 * Strips the leading spaces from source
 */

	char *sptr;
	char temp[MAXIMUM_EXPRESSION+1];
	int counter=0;

	sptr=&source[counter];
	while (*sptr==' ') {
		counter++;
		sptr=&source[counter];
	}

	/* TODO - There is no doubt a better, more efficent way 
     * of doing this.
	 */
	strcpy(temp,sptr);
	strcpy(source,temp);

}

void strip_trailing_spaces(char *string) {
/* strip_trailing_spaces
 * strips out trailing spaces in string
 */
	char *sptr;

	sptr=&string[strlen(string)-1];

	/* Identify any white space at the end of the string.
	 * This has caused problems with Unix scripts not understanding
	 * non-parameterised operators (eg. "change <rel>")	
	 */
	while ( (*sptr==' ') || (*sptr=='\n') || (*sptr=='\r') ){ 
		sptr--;
	}
	*(sptr+1)='\0';
}

void start_record(char *fname) {
	char dirpath[FILE_PATH_SIZE+1];

    /* Build the directory path */
    sprintf(dirpath,"%s%s%s%s",database_dir(current_db),LEAP_SOURCE_DIR,fname,LEAP_SOURCE_EXT);

	vprocess_query(current_db,"add (leapscripts) (%s,%s)",fname,dirpath);

    /* Open the file */
    recordingfptr=fopen(dirpath,"w");

	recording=TRUE;

}

void stop_record() {

	fclose(recordingfptr);

	recording=FALSE;
}

void dumprelstruct(relation rel) {

	if (rel!=NULL) {
		leap_printf("Name:\t[%s]\n",rel->name);
		leap_printf("noattr:\t[%d]\n",rel->noattributes);
		leap_printf("curpos:\t[%d]\n",rel->current_pos);
		leap_printf("temp:\t[%d]\n",rel->temporary);
		leap_printf("syst:\t[%d]\n",rel->system);
		leap_printf("update:\t[%d]\n",rel->updated);
		leap_printf("hash:\t[%p]\n",rel->hash_table);
		leap_printf("fpath:\t[%s]\n",rel->filepath);
		leap_printf("fname:\t[%s]\n",rel->filename);
		leap_printf("next:\t[%p]\n",rel->next);
		leap_printf("prev:\t[%p]\n",rel->previous);
	} else {
		leap_printf("NULL ptr passed to dumprelstruct\n");
	}
	
}

void util_internal( char *desc ) {
/* util_internal
 * Display internal LEAP structures - this will be expanded to provide a 
 * mechanism to probe internal structures in due course. 
 *
 */

	relation rel;

	if (desc!=NULL) {
		if (strcmp(desc,"listrel")==0) {
			leap_printf("Internal db rel. structure\n");
			relation_display(current_db);
		} else  {
			rel=relation_find(current_db,desc);

			dumprelstruct(rel);
		}
	}

}
