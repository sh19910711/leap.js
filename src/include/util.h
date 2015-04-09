/* 
 * Name: util.h
 * Description: Utility operations header file.
 * Version: util.h,v 1.204.2.1 2004/02/09 20:05:21 rleyton Exp
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


#include "defines.h"

#ifndef _LEAP_UTIL_
#define _LEAP_UTIL_

#include "dtypes.h"

extern char *generate_random_string(word size,
			     char *pstring) ;
extern void do_error(short int error_num, char *string, int fatality) ;
extern void writeln(char *string); 
extern void util_init(void);
extern void upstring(char *string);
extern void do_trace(char *trace_string) ;
extern void util_close(void) ;
extern char *cut_to_right_bracket( char *s,
				int bdepth, int force,
				char *result) ;
extern char *get_token(char *string,
					char seperator,
			char *result) ;
extern char *allbut(char *string, char *chars) ;
extern char *cut_token(char *string,
					char seperator,
			char *result) ;
extern int get_command(char *word) ;
extern void check_assign(void *ptr, char *string) ;
extern void assign_input_stream( char *filen ) ;
extern char *find_start_of_data( char *string) ;
extern void list_source_code( void ) ;
extern void print_source_code( char *file_name ) ;
extern void reverse_source_code() ;
extern void build_base_dir( char *directory ) ;
extern void print_helppage( char *page ) ;
extern int set_status( char *status_option, char *value ) ;
extern void show_status( void ) ;
extern void downcase(char *string) ;
extern void upcase(char *string) ;
extern void set_prompt(char *newprompt) ;
extern void unset_prompt(void);
extern char *copy_to_token( char *source,
					 char *delimiter,
					 char *destination ) ;
extern void strip_leading_spaces(char *source);
extern void strip_trailing_spaces(char *string);
extern void start_record(char *fname) ;
extern void stop_record() ;
extern char *concat (char *s1, char *s2) ;
extern char *skip_to_alnum(char *string);
extern void util_internal( char *desc ) ;


extern boolean status;
extern char ERROR_FILE[FILE_PATH_SIZE+1];
extern FILE *REPORT_FILE;
extern FILE *ACTIVITY_FILE;
extern boolean global_eof;

#endif
