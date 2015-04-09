/*
 * Name: leapd.c
 * Description: Main LEAP loop and functions.
 * Version: leapd.c,v 1.6.2.1 2004/02/09 20:05:20 rleyton Exp
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

/* Daemon specific includes */
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/utsname.h>

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
#include "tuples.h"
#include "rtional.h"

/* Name of db to open if specified */
char dbtoopen[DATABASE_NAME_SIZE+1];

/* File to log output to */
char activityfile[FILE_PATH_SIZE+1];

/* File to run after base configuration completed */
char configurationfile[FILE_PATH_SIZE+1];

void do_daemon() {
/* do_leap
 * Main LEAP daemon routine - Contains socket handler
 */
	char buffer[MAXIMUM_INPUT_STRING+1];
	char loginname[MAXIMUM_INPUT_STRING+1];
	char password[MAXIMUM_INPUT_STRING+1];
	char maincommand[MAXIMUM_INPUT_STRING+1];
	char tprompt[MAXIMUM_INPUT_STRING+1];
	char *result,*tresult,*plogin,*ppass;
	relation result_relation;
	int res,startscript=0;
	int serverSocket=0, on=0, port=0, status=0, childPid=0;
	struct hostent *hostPtr=NULL;
	char hostname[80]="";
	struct sockaddr_in serverName={0},clientName={0};
	struct linger linger={0};
	struct utsname sysinfo;
	int clientLength;
	tuple ctuple;

	clientLength=sizeof(clientName);

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

	/* Check to see if the logins relation exists */
	result_relation=relation_find(master_db,LEAP_DD_LOGINS);	

	if (result_relation==NULL) {
		raise_error(ERROR_CANNOT_FIND_REL,NONFATAL,LEAP_DD_LOGINS);
		raise_message(MESSAGE,"Building %s",LEAP_DD_LOGINS);

        /* Build the leaplogins relation */
        sprintf(buffer,"(SUID,INTEGER,3),(NAME,string,25),(PASSWORD,string,25),(DEFAULTDB,string,25)");
        relation_insert(master_db,create_user_relation(master_db,buffer,LEAP_DD_LOGINS,FALSE,TRUE));

        vprocess_query(master_db,"add (%s) (%d,%s,%s,%s)",LEAP_DD_LOGINS,LEAP_SUID_DBA,LEAP_LOGIN_DBA,LEAP_PASS_DBA,MASTER_DB_NAME);
	} else {
		raise_message(MESSAGE,"Found %s!",LEAP_DD_LOGINS);
	}


	if (status_quiet!=TRUE) {
		raise_message(MESSAGE,"%s","Startup sequence initiated.");
	}

	terminate=FALSE;
	terminatenow=FALSE;

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


	serverSocket=socket(PF_INET,SOCK_STREAM,0);

	if (serverSocket==-1) {
		raise_error(ERROR_SOCKETINIT,FATAL,"socket()");
	}

	on=1;

	status=setsockopt(serverSocket,SOL_SOCKET,SO_REUSEADDR,(const char *) &on,sizeof(on));

	if (status==-1) {
		raise_error(ERROR_SOCKETINIT,FATAL,"setsockopt(...,SO_REUSEADDR,...)");
	}
	
	linger.l_onoff=1;
	linger.l_linger=30;
	status=setsockopt(serverSocket,SOL_SOCKET,SO_LINGER,(const char *) &linger,sizeof(linger));
	if (status==-1) {
		raise_error(ERROR_SOCKETINIT,FATAL,"setsockopt(...,SO_LINGER,...)");
	}
		
	status=uname(&sysinfo);
	if (status==-1) {
		raise_error(ERROR_SOCKETINIT,FATAL,"uname");
	} else {
		strncpy(hostname,sysinfo.nodename,sizeof(hostname));
	}
		
	status=gethostname(hostname,sizeof(hostname));
	hostPtr=gethostbyname(hostname);

	if (hostPtr==NULL) {
		raise_error(ERROR_SOCKETINIT,FATAL,"gethostbyname");
	} 
	
	(void) memset(&serverName,0,sizeof(serverName));
	(void) memcpy(&serverName.sin_addr,hostPtr->h_addr,hostPtr->h_length);
	
	serverName.sin_family=AF_INET;
	serverName.sin_port=htons(LEAPD_PORT);
	status=bind(serverSocket,(struct sockaddr *) &serverName,sizeof(serverName));

	if (status<0) {
		raise_error(ERROR_SOCKETINIT,FATAL,"bind() - port %u - errno %u",LEAPD_PORT,errno);
	} else {
		raise_message(MESSAGE,"Daemon starting on machine %s, port %u",hostname,LEAPD_PORT);
	}

	status=listen(serverSocket,LEAPD_BACK_LOG);
	if (status==-1) {
		raise_error(ERROR_SOCKETINIT,FATAL,"listen()");
	}


	while (!terminatenow) {
			slaveSocket=accept(serverSocket,(struct sockaddr *) &clientName,&clientLength);

			if (slaveSocket==-1) {
				raise_error(ERROR_SOCKETINIT,FATAL,"accept()");
			}
				
			raise_message(MESSAGE,"Connection received from [%s]",inet_ntoa(clientName.sin_addr));

			raise_message(MESSAGE,"Authenication expected");

			strcpy(buffer,"Please authenticate yourself: ");
			write(slaveSocket,buffer,strlen(buffer));

			read(slaveSocket,loginname,MAXIMUM_INPUT_STRING);
			plogin=strtok(loginname,"\r\n");
			raise_message(MESSAGE,"Login [%s] connecting...",plogin);

			if (strcmp(plogin,"guest")==0) {
				strcpy(buffer,"Enter your e-mail address: ");
				write(slaveSocket,buffer,strlen(buffer));
			} else {
				strcpy(buffer,"Password: ");
				write(slaveSocket,buffer,strlen(buffer));
			}

			/* Get the login id from the db */
			sprintf(buffer,"project (select (%s) (%s='%s')) (%s)",LEAP_DD_LOGINS,LEAP_DDA_LOGINS_NAME,plogin,LEAP_DDA_LOGINS_PASSWORD);
			result_relation=process_query(master_db,buffer);

			status=read(slaveSocket,password,MAXIMUM_INPUT_STRING);
			ppass=strtok(password,"\r\n");

			if ((ppass!=NULL) && (status==2)) {
				*ppass='\0';
			}

			ctuple=tuple_readfirst(result_relation,TUPLE_BUILD,NULL);
			if (ctuple!=NULL) {
				tuple_to_string(ctuple,buffer);
				raise_message(MESSAGE,"Password expected [%s]",buffer);
				raise_message(MESSAGE,"Password received [%s]",ppass);
			} else {
				if (strcmp(plogin,"guest")==0) {
					raise_message(MESSAGE,"Guest ID [%s]",ppass);
				} else {
					raise_message(MESSAGE,"No login [%s] exists!",plogin);
				}
			}
	
			if (((strcmp(plogin,"guest")==0))||(strcmp(buffer,ppass)==0)) {
					raise_message(MESSAGE,"Login [%s] validated!",plogin); 
					/* Enable daemon on - this will send io to client */
					status_daemon=TRUE;
				
					strcpy(buffer,"version");
					result_relation=process_query(current_db,buffer);

					/* Ok, socket is initialised! */
					while (!terminate) {

						write(slaveSocket,DEFAULT_PROMPT,sizeof(DEFAULT_PROMPT));
						status=read(slaveSocket,buffer,MAXIMUM_INPUT_STRING);
						/* Null terminate reqd....*/
						
						result=strtok(buffer,"\r\n");
						
						status_daemon=FALSE;
						raise_message(MESSAGE,"received: %s",result);
						status_daemon=TRUE;

						result_relation=process_query(current_db,result);

					}
					/* Disable daemon */
					status_daemon=FALSE;
			} else {
				raise_message(MESSAGE,"Invalid password for login [%s]",plogin);
			}

			/* Reset terminate - one client has disconnected */
			terminate=FALSE;

			raise_message(MESSAGE,"Connection closed");
			close(slaveSocket);
	}

	if (!status_quiet) { raise_message(MESSAGE,"Closing [%s] database.",database_name(current_db)); }
	
	relations_dispose_all(current_db);

	LEAPAPI_db_destroy(&current_db);

	raise_event(EVENT,"[current_db] closed.");

	if (!status_quiet) { raise_message(MESSAGE,"Closing [%s] database.",database_name(master_db)); }

	relations_dispose_all(master_db);
	
	LEAPAPI_db_destroy(&master_db);

	raise_event(EVENT,"[master_db] closed.");

	if (!status_quiet) { raise_message(MESSAGE,"Closing [%s] database.",database_name(tempdb)); }

	relations_dispose_all(tempdb);
	
	LEAPAPI_db_destroy(&tempdb);

	raise_event(EVENT,"[tempdb] closed.");
}


void signal_handler(int signum) {
/* signal_handler
 * Signal handler for signals requiring shutdown - Hang-up, termination.
 */

	switch (signum) {
		case SIGINT:raise_message(MESSAGE,"Interrupt (CTRL-C) (SIGHUP) ");
				    break;
		case SIGHUP:raise_message(MESSAGE,"Hangup (SIGHUP) ");
					break;
		case SIGTERM:raise_message(MESSAGE,"Terminate (SIGTERM) ");
					 break;
		default:raise_message(MESSAGE,"Signal (%d) ",signum);
				break;
	}

	raise_message(MESSAGE,"Signal handler - closing databases... ",signum);

	relations_dispose_all(current_db);

	LEAPAPI_db_destroy(&current_db);

	relations_dispose_all(master_db);
	
	LEAPAPI_db_destroy(&master_db);

	relations_dispose_all(tempdb);

	LEAPAPI_db_destroy(&tempdb);

	/* Close the various files opened earlier */
	util_close();

	print_shutdown();

	leap_fprintf(stdout,"\nLEAP terminated successfully following termination signal.\n");

	exit(0);
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
	time_t tp;
	char *s;
	char *argptr;

	/********************************
	 * Startup - Firstly set up the
         * signal handlers
	 ********************************/

	/* Signal Interrupt from the keyboard - the daemon should handle it */
	signal(SIGINT,&signal_handler); 

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

	while ((argc > 1) && (argv[1][0] == '-')) {
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
				/* The user wants debug information */
				leap_printf("Debug messages enabled\n");
				tdebug=TRUE;
			}
			break;

		case 'e':
		case 'E':
			if (strlen(argptr)==1) {
				/* The user wants debug information */
				leap_printf("Debug messages enabled\n");
				tdebug=TRUE;
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
		

	if (tdebug==TRUE) set_variable(STATUS_DEBUG,STATUS_SETTING_ON);
	if (ttiming==TRUE) set_variable(STATUS_TIMING,STATUS_SETTING_ON);
	if (ttimelog==TRUE) set_variable(STATUS_TIMELOG,STATUS_SETTING_ON);
	if (tlong==FALSE) set_variable(STATUS_LONGLINE,STATUS_SETTING_OFF);
	if (tquiet==TRUE) set_variable(STATUS_QUIET,STATUS_SETTING_ON);
	if (ttrace==TRUE) set_variable(STATUS_TRACE,STATUS_SETTING_ON);
	if (tpad==TRUE) set_variable(STATUS_PADDING,STATUS_SETTING_ON);
	if (tpjoin==TRUE) set_variable(STATUS_PRODUCTJOIN,STATUS_SETTING_ON);

	/* Display some information */
	if (status_quiet!=TRUE) {
		raise_message(MESSAGE,"LEAP is starting...");
	}
	
#ifdef DEBUG
	status_debug=TRUE;
	leap_fprintf(stderr,"DEBUG: LEAP debug mode forced on\n");
#endif

	/* Do the main leap operation */
	(void) do_daemon();
	
	print_shutdown();

	/* Close the various files opened earlier */
	util_close();

	if (status_quiet!=TRUE) {
			/* Inform the user of a clean termination */
			raise_message(MESSAGE,"LEAP Terminated successfully!");
	}

	/* Return success. Elsewhere, non-zero should be returned */
	return(0);
}
