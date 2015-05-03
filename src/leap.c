/*
 * Name: leap.c
 * Description: Main LEAP loop and functions.
 * Version: leap.c,v 1.213.2.2 2004/02/09 20:05:20 rleyton Exp
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

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
/*
 * readline libraries
 */
#ifdef HAVE_READLINE
#include <readline/readline.h>
#include <readline/history.h>
#endif
/*
 * LEAP includes
 */
#include "consts.h"
#include "globals.h"
#include "dtypes.h"
#include "errors.h"
#include "dbase.h"
#include "util.h"
#include "parser.h"
#include "info.h"
#include "relation.h"
#include "vars.h"
#include "leapio.h"


/* Name of db to open if specified */
char dbtoopen[DATABASE_NAME_SIZE+1];

/* File to log output to */
char activityfile[FILE_PATH_SIZE+1];

/* File to run after base configuration completed */
char configurationfile[FILE_PATH_SIZE+1];

void print_returned_relation(relation result_relation) {
/* 
 * Display a relation's name
 */
	char buffer[MAXIMUM_INPUT_STRING+1];

	/* If a relation has been returned */
	if (result_relation!=NULL) {
		/* We can use buffer as the string has been processed, and
		 * as a relation has been returned, we're not quitting or
		 * anything like that. 
		 */
		 
		/* Produce a report string */
		if (status_regression==TRUE) {
			sprintf(buffer,"Relation RELNAME returned.");
		} else {
			sprintf(buffer,"Relation %s returned.",relation_name(result_relation));
		}

		raise_message(MESSAGE,buffer);
	}
}

void do_leap() {
/* do_leap
 * Main LEAP routine - Contains user interface etc, and calls
 * things.
 */
	char buffer[MAXIMUM_INPUT_STRING+1];
	char maincommand[MAXIMUM_INPUT_STRING+1];
	char tprompt[MAXIMUM_INPUT_STRING+1];
	char *result,*tresult;
	relation result_relation=NULL;
	int res,startscript=0;

	do_debug(DEBUG_ENTER,"Entering do_leap()");

	tempdb=LEAPAPI_db_create(NULL,TEMPDB_NAME);
	res=relations_ddopen(tempdb);
	if (res!=RETURN_SUCCESS) {
		raise_error(ERROR_OPEN_DATABASE,FATAL,TEMPDB_NAME);		
	}

	/* Open the master database */
	master_db=LEAPAPI_db_create(NULL,MASTER_DB_NAME);
	res=relations_ddopen(master_db);
	if (res!=RETURN_SUCCESS) {
		raise_error(ERROR_OPEN_DATABASE,FATAL,MASTER_DB_NAME);		
	}


	if (strlen(dbtoopen)==0) {
		/* Open up the default user database */
		current_db=LEAPAPI_db_create(NULL,DEFAULT_DB);
	} else {
		current_db=LEAPAPI_db_create(NULL,dbtoopen);
	}

	res=relations_ddopen(current_db);
	if (res!=RETURN_SUCCESS) {
		raise_error(ERROR_OPEN_DATABASE,FATAL,database_name(current_db));		
	}
	
	set_variable(VAR_CURRENTDB,database_name(current_db));

	if (status_quiet!=TRUE) {
		raise_message(MESSAGE,"%s","Startup sequence initiated.");
	}

	terminate=FALSE;

	if (status_quiet) {
			strcpy(current_prompt,"");
			set_prompt("");
	} else {
			strcpy(current_prompt,DEFAULT_PROMPT);
			set_prompt(DEFAULT_PROMPT);
	}

	if (configuration!=TRUE) {
		raise_message(MESSAGE,"Sourcing %s%s in %s",LEAP_STARTUP,LEAP_SOURCE_EXT,database_name(master_db));
		sprintf(buffer,"%s%s%s%s",database_dir(master_db),LEAP_SOURCE_DIR,LEAP_STARTUP,LEAP_SOURCE_EXT);
		assign_input_stream(buffer);
	} else {
		sprintf(buffer,"%s",configurationfile);
		assign_input_stream(buffer);
	}


#ifdef HAVE_READLINE
		raise_message(MESSAGE,"%s","Readline library available for command history/editing");
		/* Display the prompt */
#endif
	/* The MAIN program loop */
	while (!terminate) {

#ifndef HAVE_READLINE
		if (input_stream==stdin) {
			/* Print the returned relation */
			print_returned_relation(result_relation);
		}

		if (status_quiet!=TRUE)  {
			leap_printf("[%s] %s ",database_name(current_db),current_prompt);
		}

		/* Flush the buffer (no CR printed, so nothing there,
		 * but we do want the prompt...)
		 */
		fflush(stdout);
#endif

		/* Make sure that we get something, rather than just
		 * a CR on its own */
		if (global_eof==TRUE) {
			/* Quick check of global var, incase eof comes
			 * from somewhere unexpected...
			 */
			result=strcpy(buffer,L_EXIT);
		} else {
#ifndef HAVE_READLINE
				/* This is the basic "enter text, delete text"
				 * version
				 */
				result=fgets(buffer,sizeof(buffer),input_stream); 
#else
			/* This is the all singing, all dancing
			 * command history, VI/Emacs keys, easy to
			 * user version, using readline library.
			 * Lovelly jubbly!
			 */
			if (input_stream==stdin) {
		
			 	/* Print the returned relation */
				print_returned_relation(result_relation);

				if (status_quiet==TRUE) {
					strcpy(tprompt,"");
				} else {
					sprintf(tprompt,"[%s] %s ",database_name(current_db),current_prompt);
				}
				tresult=readline(tprompt);

				if (tresult) {
					if (strlen(tresult)==0) {
						strcpy(buffer,"> \n");
					} else {
						add_history(tresult);
						strcpy(buffer,tresult);
					}
					result=buffer;
				} else {
					strcpy(buffer,"exit");
					result=buffer;
				}
					
			} else {
				result=fgets(buffer,sizeof(buffer),input_stream); 
			}
#endif
		}

		if (recording==TRUE) {
			if (result[0]==STOP_RECORDING_CHAR) {
				stop_record();
				strcpy(result,"# ");
			} else {
				fprintf(recordingfptr,"%s",result);
			}
		}



		if (result==NULL) {
			if (input_stream!=stdin) {
				/* This isn't stdin, so close the file */
				fclose(input_stream);
				input_stream=NULL;
	
			 	/* Print the returned relation */
				print_returned_relation(result_relation);

				/* Reset the result relation ptr */	
				result_relation=NULL;

				/* Pop a CR onto the screen to tidy up the output */
				leap_printf("\n");

				switch(startscript) {
					case STARTUP_SCRIPT: sprintf(buffer,"%s%s%s%s",database_dir(current_db),LEAP_SOURCE_DIR,LEAP_OPEN,LEAP_SOURCE_EXT);
										 raise_message(MESSAGE,"Sourcing %s%s in %s",LEAP_OPEN,LEAP_SOURCE_EXT,database_name(current_db));
										 assign_input_stream(buffer);
										 startscript++;
										 break;
					case OPEN_SCRIPT: 
				    case NORMAL_OPS: assign_input_stream("");
									  break;
					default: assign_input_stream("");
									  break;
				}
			} else {
				/* EOF from stdin was read, so terminate nicely */
				do_trace("EOF from stdin");	

				/* Reset the result relation ptr */	
				result_relation=NULL;
	
				/* Do it normally, no short cuts setting terminate - this 
			 	* ensures that all the normal procedures are followed 
			 	*/
				strcpy(buffer,L_EXIT);	

			  if  (recording!=TRUE) {
					result_relation=process_query(current_db,buffer);
			  }
			}
		} else if (buffer[0]!='\n') {
			if ( input_stream!=stdin ) {
				if (buffer[0]!=COMMENT_CHAR) 
					leap_printf("%s",buffer);
				else
					leap_printf("\r");
			}

			/* Replace the cr at the end with a space */
			if (buffer[strlen(buffer)-1]=='\n')
				buffer[strlen(buffer)-1]=' ';

			/* Reset the result relation ptr */	
			result_relation=NULL;

			/* Is "long line" mode on, to allow commands to 
			 * span multiple lines?
			 */
			if (status_longline==TRUE) {
				strip_leading_spaces(buffer);
	
				/* Is this the end? */
				if (strchr(buffer,';')!=NULL) {
					while (result==strchr(maincommand,'\n')) {
						/* Replace the return */
						*result=' ';
					}
					raise_message(EVENT,"\nExecuting...\n%s\n",maincommand);
				  if (recording!=TRUE) {
						result_relation=process_query(current_db,maincommand);
				}
					/* Flush out the "old" command */
					strcpy(maincommand,"");
				} else {
					if (strlen(maincommand)+strlen(buffer)<MAXIMUM_INPUT_STRING)
						strcat(maincommand,buffer);
					else
					{
						raise_error(ERROR_UNKNOWN,NONFATAL,"Command too long!");
					}
					result_relation=NULL;
				}

			} else {
				/* Make a copy of the string, untainted version is
					needed later... */

			  if (recording!=TRUE) {
					result_relation=process_query(current_db,buffer);
			  }

			}
	
		}

	}

	if (!status_quiet) { raise_message(MESSAGE,"Closing [%s] database.",database_name(current_db)); }

	if ( (strcmp(database_name(current_db),TEMPDB_NAME)==0)||(strcmp(database_name(current_db),MASTER_DB_NAME)==0) ) {
		raise_message(MESSAGE,"Active DB is a system database. Skipping shutdown to avert corruption\n");
	} else {
		relations_dispose_all(current_db);

		LEAPAPI_db_destroy(&current_db);

		raise_event(EVENT,"[current_db] closed.");
	}

	if (!status_quiet) { raise_message(MESSAGE,"Closing [%s] database.",database_name(master_db)); }

	relations_dispose_all(master_db);
	
	LEAPAPI_db_destroy(&master_db);

	raise_event(EVENT,"[master_db] closed.");

	if (!status_quiet) { raise_message(MESSAGE,"Closing [%s] database.",database_name(tempdb)); }

	relations_dispose_all(tempdb);
	
	LEAPAPI_db_destroy(&tempdb);

	raise_event(EVENT,"[tempdb] closed.");

	/* Close the input stream if this a configuration instance - it's not done otherwise */
	if (configuration==TRUE) {
		fclose(input_stream);
	}
}


void signal_handler(int signum) {
/* signal_handler
 * Signal handler for signals requiring shutdown - Hang-up, termination.
 */

	switch (signum) {
		case SIGHUP:leap_printf("Hangup (SIGHUP) ");
					break;
		case SIGTERM:leap_printf("Terminate (SIGTERM) ");
					 break;
		default:leap_printf("Signal (%d) ",signum);
				break;
	}
	leap_fprintf(stdout," detected! Closing databases...\n");

	raise_message(MESSAGE,"Closing [%s] database.",database_name(current_db));
	
	relations_dispose_all(current_db);

	LEAPAPI_db_destroy(&current_db);

	raise_message(MESSAGE,"Closing [%s] database.",database_name(master_db));
	relations_dispose_all(master_db);
	
	LEAPAPI_db_destroy(&master_db);


	raise_message(MESSAGE,"Closing [%s] database.",database_name(tempdb));
	relations_dispose_all(tempdb);

	LEAPAPI_db_destroy(&tempdb);

	/* Close the various files opened earlier */
	util_close();

	print_shutdown();

	leap_fprintf(stdout,"\nLEAP terminated successfully following termination signal.\n");

	exit(0);
}

int do_configuration() {
/* do_configuration
 * Configure the LEAP setup.
 */

	database configdb;
	char fpath[FILENAME_MAX+1];
	int pos;

	do_debug(DEBUG_ENTER,"Entering do_configuration()");

	leap_printf("Checking: [%s]\n",LEAP_BASE_DIR);

	sprintf(fpath,"%s%s",LEAP_BASE_DIR,LEAP_DATABASE_DIR);

	/* If a seperator is there, remove it, so we can create
	 * the directory in question
	 */
	pos=strlen(fpath)-1;
	if (fpath[pos]==DIR_SEPERATOR) {
		fpath[pos]='\0';
	}

	if ( (mkdir( fpath, 0777)!=0) && (errno!=EEXIST) ) {
		raise_error( ERROR_CREATE_DIR,NONFATAL,fpath);
		do_debug(DEBUG_ENTER,"Exiting (unsuccessfully) do_configuration()");
		return(RETURN_ERROR);
	} else {

		leap_printf("+------------------------------------------------------------------------------+\n");
		leap_printf("| Configuring the >MASTER< database                                            |\n");
		leap_printf("+------------------------------------------------------------------------------+\n");
		sprintf(fpath,"%s%s",LEAP_BASE_DIR,LEAP_DATABASE_DIR);
		configdb=LEAPAPI_db_init(fpath, MASTER_DB_NAME, TRUE);
		master_db=configdb;

		if (master_db!=NULL) {
				leap_printf("+------------------------------------------------------------------------------+\n");
				leap_printf("| Configuring the >default< (user) database                                    |\n");
				leap_printf("+------------------------------------------------------------------------------+\n");
				sprintf(fpath,"%s%s",LEAP_BASE_DIR,LEAP_DATABASE_DIR);
				configdb=LEAPAPI_db_init(fpath, DEFAULT_DB, TRUE);
				
				/* Dispose of the database to clear out temporary relations etc. */
				relations_dispose_all(configdb);
				LEAPAPI_db_destroy(&configdb);

				leap_printf("+------------------------------------------------------------------------------+\n");
				leap_printf("| Configuring the >temporary< (tempdb) database                                    |\n");
				leap_printf("+------------------------------------------------------------------------------+\n");
				sprintf(fpath,"%s%s",LEAP_BASE_DIR,LEAP_DATABASE_DIR);
				configdb=LEAPAPI_db_init(fpath, TEMPDB_NAME, TRUE);
		} else {
			raise_error(ERROR_INSTALL,FATAL,"");
			do_debug(DEBUG_ENTER,"Exiting (unsuccessfully) do_configuration()");
			return(RETURN_ERROR);
		}

		relations_dispose_all(configdb);
		LEAPAPI_db_destroy(&configdb);

		relations_dispose_all(master_db);
		LEAPAPI_db_destroy(&master_db);

		do_debug(DEBUG_ENTER,"Exiting (successfully) do_configuration()");
		return(RETURN_SUCCESS);
	}

}

int specify(char *var, char *value, int length) {
	if (value!=NULL) {
		strncpy(var, value, length);
		return(RETURN_SUCCESS);
	} else {
		return(RETURN_ERROR);
	}

}

int main(int argc, char *argv[]) {
  printf("sizeof(unsigned char) = %d\n", sizeof(unsigned char));
/* main
 * The main LEAP program - This performs the display of the
 * title, and processes the command line. Clean termination
 * also should occur here. Everywhere else termination is
 * "unclean", and should return an error status. Define
 * error status' in the dtypes.h file
 */
#ifdef FULL_DEBUG
	/* Test vars */
	char source[50],result[50],*sptr;
#endif
	boolean tdebug=FALSE,ttiming=FALSE,ttimelog=FALSE,tlong=FALSE;
	boolean tquiet=FALSE,ttrace=FALSE,tpad=FALSE,tpjoin=FALSE;
	boolean tmergestderr=FALSE;
	time_t tp;
	char *s,*debuglevel=NULL;
	char *argptr;

	/********************************
	 * Startup - Firstly set up the
     * signal handlers
	 ********************************/

	/* Signal Interrupt from the keyboard - Ignore it */ 
	signal(SIGINT,SIG_IGN);

	/* MSDOS/Windows does not know SIGQUIT/SIGHUP, whereas
	 * these are important in Unix
	 */
#ifndef __MSDOS__

	/* Signal Quit from the keyboard - Ignore it */
	signal(SIGQUIT,SIG_IGN);

	/* Signal Hang up - Handle it */
	signal(SIGHUP,&signal_handler);

#else

#endif

	define_handle(&default_handler,&errorHandler);
	raise_message(EVENT, "%s","Event Handler initialised.");

	define_handle(&default_quiethandler,&messageHandler);
	raise_message(EVENT,"%s","Message Handler initialised.");

	/* Signal Terminate - Handle it */
	signal(SIGTERM,&signal_handler);


	/* Perform some configuration... */
	build_base_dir(LEAP_DEFAULT_DIR);
	
	/* Set the random seed to the time in secs since 01.01.1970
	 * should be random enough!
	 */
	srand(time(NULL));

	s=getenv(LEAP_ENV_DIR);

	strcpy(dbtoopen,"");
	strcpy(activityfile,"");
	strcpy(tempdir,"");
	ACTIVITY_FILE=NULL;

	if (s!=NULL) {
		leap_fprintf(stdout,"Using environment variable %s for path (%s).\n",LEAP_ENV_DIR,s);
		build_base_dir(s);
	}

	/* Process the command line
		Stop if we run out of arguments
		or we get an argument without a dash */

	/* NB. This is based on O'Reilly & Associates "Practical
	  C Programming", by Steve Oualline, 2nd Ed. (pg 178) */

	while ((argc > 1) && (argv[1][0] == ARGUMENT_PREFIX)) {
		/* 
		 * argv[1][1] is the actual option character
		 */
		if ((argv[1][0]==ARGUMENT_PREFIX) && (argv[1][1]==ARGUMENT_PREFIX)) {
			argptr=&argv[1][2];
		} else {
			argptr=&argv[1][1];
		}
		switch (*argptr) {
		case 'a':
		case 'A':
			if ((strcmp(argptr,"activity-file")==0)||(strcmp(argptr,"activity")==0)||(strlen(argptr)==1)) {
				if (specify(activityfile,argv[2],FILE_PATH_SIZE)){
					argv++;
					argc--;
				} else {
					leap_fprintf(stderr,"No file specified. Using %s\n",LEAP_ACTIVITY_FILE);
					strncpy(activityfile,LEAP_ACTIVITY_FILE,FILE_PATH_SIZE);
				}
				ACTIVITY_FILE=fopen(activityfile,"a");
				if (ACTIVITY_FILE==NULL) {
					leap_fprintf(stderr,"Unable to open activity file for appending.\n");
				} else {
					tp=time(NULL);
					fprintf(ACTIVITY_FILE,"###\n# Activity file STARTED at: %s###\n",ctime(&tp));
					leap_printf("Activity file: %s\n",activityfile);
				}
			}
			break;
			
		case 'b':
		case 'B':
			if (specify(dbtoopen,argv[2],DATABASE_NAME_SIZE)){
				argv++;
				argc--;
			} else {
				raise_message(MESSAGE,"No database specified on command line. %s will be used.",DEFAULT_DB);
			}
			break;

		case 'c':
		case 'C':
			if ((strcmp(argptr,"configure")==0)||(strcmp(argptr,"configuration")==0)||(strlen(argptr)==1)){
				specify(configurationfile,argv[2],FILE_PATH_SIZE);	
				leap_printf("CONFIGURING...\n");
				configuration=TRUE;
				argv++;
				argc--;
			} else if (strcmp(argptr,"cleanup")==0) {
				cleanup=TRUE;
				leap_printf("Startup Cleanup enabled...\n");
				argv++;
				argc--;
			}
			break;
		case 'd':
		case 'D':
			if ((strcmp(argptr,"dir")==0)||(strcmp(argptr,"directory")==0)||(strlen(argptr)==1)) {
				if (argv[2]) {
					build_base_dir(argv[2]);
	
					/* Increment the counts... */
					argv++;
					argc--;
				} else {
					leap_fprintf(stderr,"ERROR: No directory specified after directory flag.\n");
					exit(1);
				}
			} else if (strcmp(argptr,"database")==0) {
				if (specify(dbtoopen,argv[2],DATABASE_NAME_SIZE)){
					argv++;
					argc--;
				}
			} else if (strcmp(argptr,"debug")==0) {
#ifdef DEBUG_CODE
				/* The user wants debug information */
				leap_printf("Debug messages enabled\n");
				tdebug=TRUE;

				if ((argv[2]) && (argv[2][0]!=ARGUMENT_PREFIX)) {
					/* debug level specified */
					debuglevel=argv[2];
					argv++;
					argc--;
				}
#else
				raise_error(ERROR_DISABLED_CODE,NONFATAL,"Debugging code is disabled");
#endif
			}
			break;

		case 'e':
		case 'E':
			if (strlen(argptr)==1) {
#ifdef DEBUG_CODE
				/* The user wants debug information */
				leap_printf("Debug messages enabled\n");
				tdebug=TRUE;
#else
				raise_error(ERROR_DISABLED_CODE,NONFATAL,"Debugging code is Disabled");
#endif
			} else if ((strcmp(argptr,"events")==0)||(strcmp(argptr,"event")==0)) {
				/* User wants event reports */
				define_handle(&default_quiethandler,&eventHandler);
				raise_message(EVENT,"%s","Event Handler initialised.");
			}
			break;
		case 'h':
		case '?':
		case 'H':
			if ((strcmp(argptr,"help")==0)||(strlen(argptr)==1)) {
				/* The user wants some help... */
				print_help();

				/* Exit without an error */
				exit(0);
	
				/* Lint-ified - The break is not reached */
				/* break; */
			}
		case 'i':
		case 'I':
			/* The user wants timing information */
			leap_printf("Timing information enabled\n");
			ttiming=TRUE;
			break;

		case 'l':
		case 'L':
			if ((strcmp(argptr,"time-logging")==0)||(strlen(argptr)==1)) {
				/* Do not fetch the system time in log file */
				leap_printf("Time logging disabled\n");
				ttimelog=FALSE;
			} else if ((strcmp(argptr,"long-commands")==0)||(strcmp(argptr,"long")==0)){
				leap_printf("Long commands enabled\n");
				tlong=TRUE;
			}
			break;
		case 'M':
		case 'm':
			if ((strcmp(argptr,"merge-stderr")==0)||(strlen(argptr)==1)) {
				/* Merge stderr into stdout (for WinLEAP) */
				leap_printf("stderr will be merged into stdout\n");
				tmergestderr=TRUE;
			} 
			break;

		case 'n':
		case 'N':
			/* Display warranty information */
			do_warranty();
			exit(0);
				
			/* Lint-ified - The break is not reached */
			/* break; */
		case 'o':
		case 'O':
			leap_printf("Long commands enabled\n");
			tlong=TRUE;
			break;
		case 'p':
		case 'P':
			if ((strcmp(argptr,"padding")==0)||(strlen(argptr)==1)) {
				leap_printf("Relation Name Padding enabled\n");
				tpad=TRUE;
			} else if ((strcmp(argptr,"product-join")==0)||(strcmp(argptr,"pjoin")==0)) {
				leap_printf("Product performed in no-condition join\n");
				tpjoin=TRUE;
			} 
			break;
		case 'q':
		case 'Q':
			if ((strcmp(argptr,"quiet")==0)||(strlen(argptr)==1)) {
				tquiet=TRUE;
				BEEP=' ';

				/* Shutdown the message handler */
				define_handle(NULL,&messageHandler);
			}
			break;
		case 'r':
		case 'R':
			if ((strcmp(argptr,"regression")==0)||(strlen(argptr)==1)) {
				/*  What!? This is to disable outputting
 		 		 * items that might cause regression tests
				 * to fail for no good reason, ie. 
				 * temporary relation names which are random,
				 * and will differ between runs.
			     */
				status_regression=TRUE;
				leap_printf("Regression test mode on\n");
			}
			break;
		case 's':
		case 'S':
			if ((strcmp(argptr,"status")==0)||(strlen(argptr)==1)) {
				/* The user wants status messages to be
			   	displayed */
				leap_printf("Status messages enabled\n");
				status=TRUE;
			}
			break;
		case 't':
		case 'T':
			if ((strcmp(argptr,"time")==0)||(strcmp(argptr,"timing")==0)) {
				/* The user wants timing information */
				leap_printf("Timing information enabled\n");
				ttiming=TRUE;
			} else if ((strlen(argptr)==1)||(strcmp(argptr,"trace")==0)||(strcmp(argptr,"tracing")==0)) {
				/* The user wants tracing information */
				leap_printf("Tracing information enabled\n");
				ttrace=TRUE;
			}
			break;
		case 'v':
		case 'V':
			if ((strcmp(argptr,"version")==0)||(strlen(argptr)==1))  {
				/* Print BRIEF version information */
				print_header(TRUE);
				exit(0);
			} else if (strcmp(argptr,"versiononly")==0) {
				leap_printf("%d.%d.%d\n",LEAP_MAJOR_VERSION,LEAP_MINOR_VERSION,LEAP_REVISION_VERSION);
				exit(0);
			}

			break;
		case 'w':
		case 'W':
			if ((strcmp(argptr,"warranty")==0)||(strlen(argptr)==1)) {
				do_warranty();
				exit(0);
			}
			break;
		case 'x':
		case 'X':
			if (argv[2]) {
				strncpy(tempdir,argv[2],FILE_PATH_SIZE);
				/* Increment the counts... */
				argv++;
				argc--;
			} else {
				leap_fprintf(stderr,"ERROR: No directory specified after temporary directory flag.\n");
				exit(1);
			}
			break;

			
		default:
			raise_error(ERROR_COMMAND_LINE,NONFATAL,argptr);
		}

		/* Move the argument list up one and the count down one */
		argv++;
		argc--;
	}
	raise_message(EVENT,"%s","Command line processed.");

	/* First things first, report (verbosely) what we are. */
	if ((status_regression!=TRUE) && (tquiet!=TRUE))
		print_header(FALSE);
	
	if ( (status) && (tquiet!=TRUE) ) {
		sprintf(temp_80_chars,"LEAP Base directory set to: %s",LEAP_BASE_DIR);
		leap_printf(temp_80_chars);
	}

	/* Call any initialisation routines... */
	util_init();

	/* This has to be done after the path is read, so that
     * the variable config file is located.
     */
	if (init_variables()!=RETURN_SUCCESS) {
		raise_message(MESSAGE,"Directory specified [%s] not valid. Trying [%s]",LEAP_BASE_DIR,LEAP_TRY_DIR);
		build_base_dir(LEAP_TRY_DIR);
		if (init_variables()!=RETURN_SUCCESS) {
			raise_message(MESSAGE,"[%s] is also not valid - Problems are likely",LEAP_TRY_DIR);
		} else {
			raise_message(MESSAGE,"Variables are now set.");
		}
	}

	raise_message(MESSAGE,"Applying command line options...");

	if (tdebug==TRUE) set_variable(STATUS_DEBUG,STATUS_SETTING_ON);
	if (ttiming==TRUE) set_variable(STATUS_TIMING,STATUS_SETTING_ON);
	if (ttimelog==TRUE) set_variable(STATUS_TIMELOG,STATUS_SETTING_ON);
	if (tlong==FALSE) set_variable(STATUS_LONGLINE,STATUS_SETTING_OFF);
	if (tquiet==TRUE) set_variable(STATUS_QUIET,STATUS_SETTING_ON);
	if (ttrace==TRUE) set_variable(STATUS_TRACE,STATUS_SETTING_ON);
	if (tpad==TRUE) set_variable(STATUS_PADDING,STATUS_SETTING_ON);
	if (tpjoin==TRUE) set_variable(STATUS_PRODUCTJOIN,STATUS_SETTING_ON);
	if (debuglevel!=NULL) set_variable(STATUS_DEBUG_LEVEL,debuglevel);
	if (tmergestderr==TRUE) set_variable(STATUS_MERGE_STDERR,STATUS_SETTING_ON);

#ifndef DEBUG_CODE
	raise_message(MESSAGE,"Non-debugging version.");
#endif
	raise_message(MESSAGE,"Completed application of command line options...");

	/* Display some information */
	if (status_quiet!=TRUE) {
		raise_message(MESSAGE,"LEAP is starting...");
	}
	
#ifdef DEBUG
	status_debug=TRUE;
	status_debuglevel=MAX_DEBUG_LEVEL;
	leap_fprintf(stderr,"DEBUG: LEAP debug mode forced on\n");
#endif

	if (configuration!=TRUE) {
		/* Do the main leap operation */
		(void) do_leap();
	
		print_shutdown();

		/* Close the various files opened earlier */
		util_close();

		if (status_quiet!=TRUE) {
				/* Inform the user of a clean termination */
				raise_message(MESSAGE,"LEAP Terminated successfully!");
		}
	} else {
		if (do_configuration()==RETURN_SUCCESS) {
			(void) do_leap();
			if (status_quiet!=TRUE) {
				raise_message(MESSAGE,"LEAP Configuration completed successfully!");
			}
		} else {
		}

		/* Close the various files opened earlier */
		util_close();
	}

	/* Return success. Elsewhere, non-zero should be returned */
	return(0);
}
