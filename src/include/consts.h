/* 
 * Name: consts.h
 * Description: Main LEAP constants.
 * Version: consts.h,v 1.213.2.1 2004/02/09 20:05:20 rleyton Exp
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

/* To prevent multiple definitions */
#ifndef _LEAP_CONSTANTS_
#define _LEAP_CONSTANTS_

/* Pull in system defined limits */
#include <limits.h>

/* Some important strings... */
#define LEAP_CORE_VERSION "1.2.6.1"
#define LEAP_MAJOR_VERSION 1
#define LEAP_MINOR_VERSION 2
#define LEAP_REVISION_VERSION 6
#define LEAP_PATCH 1
#define LEAP_VERSION_IDENT (LEAP_MAJOR_VERSION * 100)+(LEAP_MINOR_VERSION*10)+LEAP_REVISION_VERSION
#define LEAP_VERSION ""
#define LEAP_VERSION_TEXT "An extensible and free RDBMS"
#define LEAP_COPYRIGHT "Copyright (C) 1995-2004 Richard Leyton."
#define LEAP_DISTRIBUTION_1 "LEAP comes with ABSOLUTELY NO WARRANTY; for details type \"warranty\"."
#define LEAP_DISTRIBUTION_2 "This is free software, and you are welcome to redistribute it"
#define LEAP_DISTRIBUTION_3 "under certain conditions; type \"copying\" for details."
#define LEAP_KEY_PROMPT "Press enter/return to continue..."


/* Blame Bill... */

#ifndef __MSDOS__
#define LEAP_DEFAULT_DIR "./"   /* Default directory for LEAP */
#define LEAP_TRY_DIR "../"      /* Directory to try before you die */
#define LEAP_ERROR_DIR "errors/"
#define LEAP_RELATION_DIR "relation/"
#define LEAP_DATABASE_DIR "database/"
#define LEAP_SOURCE_DIR "source/"
#define LEAP_REPORT_DIR "report/"
#define LEAP_HELP_DIR "help/"
#define LEAP_CONFIG_DIR "configs/"
#define LEAP_TEMP_DIR "/tmp/"
#else
#define LEAP_DEFAULT_DIR ".\\"   /* Default directory for LEAP */
#define LEAP_TRY_DIR "..\\"      /* Directory to try before you die */
#define LEAP_ERROR_DIR "errors\\"
#define LEAP_RELATION_DIR "relation\\"
#define LEAP_DATABASE_DIR "database\\"
#define LEAP_SOURCE_DIR "source\\"
#define LEAP_REPORT_DIR "report\\"
#define LEAP_HELP_DIR "help\\"
#define LEAP_CONFIG_DIR "configs\\"
#define LEAP_TEMP_DIR ".\\"
#endif

#define LEAP_RELATION_EXT ".rel"
#define LEAP_NEW_RELATION_EXT ".relation"
#define LEAP_FIELD_EXT ".fld"
#define LEAP_ATTRIBUTE_EXT ".fld"
#define LEAP_TEMPORARY_EXT ".tmp"
#define LEAP_HASH_EXT ".hsh"
#define LEAP_SOURCE_EXT ".src"
#define LEAP_TEMPORARY_TXT "LEAP Temporary indicator file - Do not delete!"
#define LEAP_REPORT_FILE "report.txt"
#define LEAP_HELP_FILE "helppgs"
#define LEAP_DB_INFO "info.msg"  /* Database information file */
#define LEAP_ERROR_FILE "errors"
#define LEAP_VARIABLE_FILE "vars"
#define LEAP_ACTIVITY_FILE "leap.out"

#define MAXIMUM_PROMPT 255		/* Maximum Prompt size */
#define DEFAULT_PROMPT ":-)"		/* Default LEAP Prompt */
#define DEFAULT_ERROR_PROMPT ":-o"	/* Default LEAP errorPrompt */

#define LEAP_STARTUP "startup"  /* Startup of LEAP  */
#define LEAP_SHUTDOWN "shutdown"/* Shutdown of LEAP */
#define LEAP_OPEN "open"	    /* Opening of a db  */

/*******************************
 * Data dictionary definitions *
 *******************************/

/* Database names */
#define MASTER_DB_NAME "master"       /* The master database */
#define DEFAULT_DB "user"             /* The default database */
#define TEMPDB_NAME "tempdb"             /* The default database */

/* Daemon login names/suid */
#define LEAP_LOGIN_DBA "sa"
#define LEAP_SUID_DBA 1
#define LEAP_PASS_DBA "-"

/* Database relation */
#define LEAP_DD_DATABASE "leapdata"   /* Relation with databases defined */
#define LEAP_DDA_DATABASE_NAME "NAME" /* Attribute with database names */
#define LEAP_DDA_DATABASE_DIR  "DIR"  /* Attribute with database directories */

#define LEAP_DD_RELATIONS "leaprel"   /* Relation with relations */
#define LEAP_DDA_RELATIONS_NAME "NAME"/* Attribute with relation name */
#define LEAP_DDA_RELATIONS_NAME_NO 0
#define LEAP_DDA_RELATIONS_FNAME "FNAME" /* Attribute with file name */
#define LEAP_DDA_RELATIONS_FNAME_NO 1 /* Attribute with file name */
#define LEAP_DDA_RELATIONS_TEMP "TEMP"/* Attribute with temporary status */
#define LEAP_DDA_RELATIONS_TEMP_NO 2 /* Attribute with file name */
#define LEAP_DDA_RELATIONS_NOATTRIBS "NOATTRIBS"/* Attribute with no attribs */
#define LEAP_DDA_RELATIONS_NOATTRIBS_NO 3 /* Attribute with file name */
#define LEAP_DDA_RELATIONS_UPDATED "UPDATED"/* Attribute with updated flag*/
#define LEAP_DDA_RELATIONS_UPDATED_NO 4 /* Attribute with file name */
#define LEAP_DDA_RELATIONS_SYSTEM "SYSTEM"/* Attribute with system flag */
#define LEAP_DDA_RELATIONS_SYSTEM_NO 5 /* Attribute with file name */

#define LEAP_DD_ATTRIBUTES "leapattributes" /* Relation with attributes */
#define LEAP_DDA_ATTRIBUTES_RELATION "RELATION" /* Relation attribute belongs to */
#define LEAP_DDA_ATTRIBUTES_ATTRIBUTE "ATTRIBUTE" /* attribute name */
#define LEAP_DDA_ATTRIBUTES_TYPE "TYPE" /* attribute type */
#define LEAP_DDA_ATTRIBUTES_SIZE "SIZE" /* attribute size */

#define LEAP_DD_TYPES "leaptypes" /* Relation with leap types */
#define LEAP_DDA_TYPES_NAME "NAME" /* Type name */
#define LEAP_DDA_TYPES_SIZE "SIZE" /* Default size */

#define LEAP_DD_SOURCES "leapscripts" /* Relation with script names */
#define LEAP_DDA_SOURCES_NAME "NAME" /* Script name */
#define LEAP_DDA_SOURCES_FILE "FILE" /* File pathname */

#define LEAP_DD_LOGINS "leaplogins" /* Relation with server logins */
#define LEAP_DDA_LOGINS_SUID "SUID" 
#define LEAP_DDA_LOGINS_NAME "NAME"
#define LEAP_DDA_LOGINS_PASSWORD "PASSWORD"
#define LEAP_DDA_LOGINS_DEFAULTDB "DEFAULTDB"

#define NO_DD_RELATIONS 5 /* Temporary constant */

/* Default values */
#define DEFAULT_ATTRIBUTE_SIZE 255 /* Attribute size if none specified */
#define DEFAULT_NUMERIC_ATTRIBUTE_SIZE 5 /* Attribute size if none specified */
#define DEFAULT_BOOLEAN_ATTRIBUTE_SIZE 5 /* Attribute size if none specified */


/* Each database will have relation defined in LEAP_DD_RSHIP if there
 * are to be relationships between relations. Primary key can hold
 * more than one attribute - each attribute is held in seperate
 * column, eg. pkey1, pkey2, ..., pkeyn - 1-1 relationship
 * between pkey1 and fkey1. 
 */
#define LEAP_DD_RSHIP "relship" /* Relationship relation */
#define LEAP_DDA_RSHIP_PREL "prelation"   /* Primary relation */
#define LEAP_DDA_RSHIP_FREL "frelation"   /* Foreign relation */
#define LEAP_DDA_RSHIP_FKEY "fkey"  /* Foreign key */
#define LEAP_DDA_RSHIP_PKEY "pkey"	/* Primary key */

/* Following two defines are used when extracting information out
 * of a tuple from this relation, ie. addressing tuple array 
 */

/* Number of NON-key attributes */
#define LEAP_DDA_RSHIP_NUM_NON 2

/* Position at which attribute key data starts */
#define LEAP_DDA_RSHIP_BDATA LEAP_DDA_RSHIP_NUM_NON

/* Maximum number of keys */
#define MAXIMUM_KEYS 3

/* Total number of key sets */
/* 2 for p/f keys, more for sets of candidate keys perhaps? */
#define TOTAL_KEYS 2

/* Total number of attributes in a complete key set */
#define TOTAL_KEY_ATTRIBUTES (MAXIMUM_KEYS * TOTAL_KEYS)

/* Some machine/OS dependant definitions */
/* ie. Bills fault again... */
#ifndef __MSDOS__
#define DIR_SEPERATOR '/'
#define DIR_SEPERATOR_STRING "/"  /* MUST be the same as DIR_SEPERATOR */
				   /* But as a string, not a char       */
#else
#define DIR_SEPERATOR '\\'
#define DIR_SEPERATOR_STRING "\\"  /* MUST be the same as DIR_SEPERATOR */
				   /* But as a string, not a char       */
#endif

/* Define some symbols */
#define ASSIGNMENT "="

#define COMMENT_CHAR '#'
#define FCOMMENT_CHAR '>'
#define STOP_RECORDING_CHAR '.'
#define ARGUMENT_PREFIX '-'

/* Various maximums and sizes. 
 */

#define RELATION_NAME_SIZE 25
#define RANDOM_NAME_SIZE 6

#define FILENAME_EXT_SIZE 4  /* Includes dot */
/* 
 * POSIX.1 defines PATH_MAX for maximum path size.
 * Use this if available (limits.h), otherwise, take a guess
 */
#ifdef PATH_MAX
# define FILE_PATH_SIZE PATH_MAX
# ifdef NAME_MAX
#  define FILE_NAME_SIZE NAME_MAX
# else
#  define FILE_NAME_SIZE 25
# endif
#else
# ifdef __MSDOS__
#  define FILE_PATH_SIZE 127
#  define FILE_NAME_SIZE 8+FILENAME_EXT_SIZE
# else
#  define FILE_PATH_SIZE 255
#  define FILE_NAME_SIZE 25
# endif
#endif

#ifndef __MSDOS__
#else
#endif

/* Maximum size of an attribute name */
#define ATTRIBUTE_NAME_SIZE 25

/* Maximum size of a string describing the TYPE of relation
 * (String, number, etc) 
 */
#define ATTRIBUTE_TYPE_SIZE 15

/* Buffer size for reading the attributes from disk 
 * Should be no smaller than ATTRIBUTE_NAME_SIZE or TYPE_SIZE
 * (Whichever is bigger)
 */
#define ATTRIBUTE_BUFFER_SIZE 25

/* Maximum size for the attribute data - Beware
 * that this can lead to high memory usage if set very high!
 */
#define ATTRIBUTE_MAXIMUM_SIZE 255

/* Maximum number of attributes allowed. Watch out if this
 * is set too high (>64)
 */
#define MAXIMUM_ATTRIBUTES 64

/* Maximum size needed to store the complete list of attributes
 * in a string, with a space seperating each attribute
 */
#define MAXIMUM_MAXIMUM_ATTRIBUTE (ATTRIBUTE_NAME_SIZE+1)*MAXIMUM_ATTRIBUTES

/* For hashing - The maximum size of all the data in a tuple */
#define MAXIMUM_ALL_DATA (ATTRIBUTE_MAXIMUM_SIZE * MAXIMUM_ATTRIBUTES)

#define DATABASE_NAME_SIZE 25

/* The maximum size allowable for an input string. 
 */
#define MAXIMUM_INPUT_STRING 1024

/* A better name for this is an "expression". But this implies
 * nesting... :)
 */
#define MAXIMUM_EXPRESSION MAXIMUM_INPUT_STRING

/* Define the maximum number of brackets allowed... */
#define MAXIMUM_BRACKETS 255

/* Define the maximum number of conditions in a condition */
#define MAXIMUM_CONDITIONS 20

/* Define the size of a condition */
/* This will never be higher than MAXIMUM_EXPRESSION */
#define MAXIMUM_CONDITION_SIZE MAXIMUM_EXPRESSION 

/* Define the size of a format string for attribute/data outputs */
#define FORMAT_SIZE 20


/*
 * Define some help file processing options
 *
 */

/* Define maximum size for help file lines */
#define HELP_PAGE_LINE_SIZE 80

/* Character for head of the page */
#define HELP_PAGE_START '#'

/* Character for end of the page */
#define HELP_PAGE_END '+'

/* Character for comments (RCS History) */
#define HELP_PAGE_COMMENT '['

/* Number of items per summary line */
#define HELP_ITEMS 3

/* Number of intro pages */
#define HELP_INTRO_PAGES 3

/* Various strings */
#define HELP_SUMMARY "INDEX"

/* Define boolean flags */
#define BOOLEAN_UNDETERMINED 0
#define BOOLEAN_END_MARKER 99
#define BOOLEAN_AND 1
#define BOOLEAN_OR 2
#define BOOLEAN_XOR 3 /* TODO */
#define BOOLEAN_LESS_THAN 4
#define BOOLEAN_GREATER_THAN 5
#define BOOLEAN_EQUAL 6
#define BOOLEAN_LESS_EQUAL_THAN 7
#define BOOLEAN_GREATER_EQUAL_THAN 8
#define BOOLEAN_NOT_EQUAL 9
#define BOOLEAN_LIKE 10

#define BOOLEAN_AND_STRING "and"
#define BOOLEAN_OR_STRING "or"
#define BOOLEAN_XOR_STRING "xor"

/* For comparison if specified */
#define TRUE_STRING "TRUE"
#define FALSE_STRING "FALSE"

/* Define some useful items for boolean type */
#define TRUE 1
#define FALSE 0

/* Couple of occasionally useful defs 
 * for match_tuples etc.
 */
#define DIR_UNKNOWN 0
#define DIR_LEFT 1
#define DIR_RIGHT 2

/* The various data types supported by LEAP */
#define DT_UNDEFINED 0
#define DTS_UNDEFINED "UNDEFINED"

#define DT_STRING 1
#define DTS_STRING "STRING"
#define DT_SIZE_MINIMUM_STRING 1
#define DT_SIZE_MAXIMUM_STRING 255
#define DT_STRING_TYPE char[ATTRIBUTE_MAXIMUM_SIZE+1]

#define DT_NUMBER 2
#define DT_INTEGER 2
#define DTS_NUMBER "INTEGER"
#define DTS_INTEGER "INTEGER"
#define DT_SIZE_MINIMUM_NUMBER 1
#define DT_SIZE_MAXIMUM_NUMBER 5
#define DT_NUMBER_TYPE int

#define DT_BOOLEAN 3
#define DTS_BOOLEAN "BOOLEAN"
#define DT_BOOLEAN_TYPE unsigned char

#define DTS_UNSUPPORTED "UNSUPPORTED"
#define DT_UNSUPPORTED_TYPE DT_STRING_TYPE

/* Required size for a DTS String  */
#define MAX_DTS_SIZE 7


/* Delimiter character for tuple storage. Note that
 * there are two because otherwise its an escaped quote...
 * (Added the \n to make sure the end of line is a delimiter too!)
 */
#define DATA_DELIMITER "\\\n"

#define DELIMETER_CHAR '\\'

/* Define an indicator for an empty string in a record on disk */
#define NULL_INDICATOR '-'

/* An escaped delimeter - This is used to search for an escaped
 * delimeter - speeds up the whole process by working out if
 * anything special needs to be done
 */
#define ESCAPED_DELIMITER "\\\\"

/* The default prompt for LEAP */
#define PROMPT ">"

/* Define the seperators for a string (Used in seperating
 * a list of attributes 
 */
#define SEPERATORS " ,"

/* Define the seperators that seperate tokens, for get_token
 * and cut_token - Same as pascal, with NULL to end the string
 * \\ is escaped \
 */
#define TOKEN_SEPERATORS ", /\\:()\0"

/* Define the value for the character that defines if a 
 * condition contains a value rather than a field.
 */
#define VALUE '\"'

/* Define all possible values that could be used
 */
#define VALUES "\"\'`"

/* Define the component boolean operators */
#define L_OPERATORS "=<>!"

#define L_END '\0'
#define L_LIKE '~'
#define L_EQUAL '='
#define L_LESSTHAN '<'
#define L_GREATERTHAN '>'
#define L_PLING '!'	/* I call it "pling" */
#define L_BANG L_PLING	/* Some call it "bang" */
#define L_EXCLAMATION L_PLING /* Sane people call it an exclamation
			       * mark... Take your pick!
			       */	

/* Define the operators as strings */
#define L_BOOLEAN_EQUAL "="
#define L_BOOLEAN_LIKE "~"
#define L_BOOLEAN_LESSTHAN "<"
#define L_BOOLEAN_LESSTHAN_EQUAL "<="
#define L_BOOLEAN_GREATERTHAN ">"
#define L_BOOLEAN_GREATERTHAN_EQUAL ">="
#define L_BOOLEAN_NOTEQUAL "<>"

#define BOOLEAN_STRING_SIZE 2 /* Size to store a boolean op. as string(+1 for null) */

/* Define the maximum number of seperators that may exist, this
 * is used in get_token and cut_token to save time and stuff. 
 * Its never going to be bigger than TOKEN_SEPERATORS.
 */
#define MAX_SEPERATORS 15

#define ATTRIBUTE_SEPERATOR ','

/* Hand definitions */
#define NODE_ROOT 0
#define NODE_LEFT 1
#define NODE_RIGHT 2


/* Command values - Some are obsoleted. */

#define C_UNKNOWN -1
#define C_HELP 1
#define C_LOAD 2
#define C_LIST 3
#define C_DISPLAY_REL 4
#define C_PRINT 5
#define C_DISPLAY 6
#define C_DELREL 7
#define C_DISPLAY_INDEX 8
#define C_PROJECT 9
#define C_DISPOSE 10
#define C_SRCFILE 11
#define C_DOS 12
#define C_UNION 13
#define C_JOIN 14
#define C_MEM 15
#define C_CHANGE 16
#define C_IDX 17
#define C_PRINT_IDX 18
#define C_IDX_STORE 19
#define C_INTERSECT 20
#define C_DIFFERENCE 21
#define C_PRODUCT 22
#define C_SELECT 23
#define C_DESCRIBE 24
#define C_NORMAL 25
#define C_HIGH 26
#define C_SMJOIN 27
#define C_FLUSH 28
#define C_CREATE 29
#define C_ADD 30
#define C_CLEAR 31
#define C_RMVTMP 32
#define C_PROMPT 33
#define C_INFO 34
#define C_STATUS 35
#define C_DEBUG 36
#define C_INFIX 37
#define C_VERSION 38
#define C_COMMENT 40
#define C_REPORT 41
#define C_TIMING 42
#define C_US 43
#define C_WHAT 44
#define C_USE 45
#define C_DIR 46
#define C_IOON 47
#define C_RENAME 48
#define C_PANIC 49
#define C_BREAK 50
#define C_CASE 51
#define C_DUPLICATE 52
#define C_CACHE 53
#define C_ITERATIVE 54
#define C_PARSE 55
#define C_EXIT 56
#define C_RELATION 57
#define C_WARRANTY 58
#define C_ADDRESSES 59
#define C_LISTSRC 60
#define C_SET 61
#define C_FILE_COMMENT 62
#define C_NATURAL_JOIN 63
#define C_SHOWVARS 64
#define C_TEST 65
#define C_DELETE 66
#define C_DUMP 67
#define C_CREATE_RELATION 68
#define C_REVERSE_ENG  69
#define C_RECORD  70
#define C_INTERNAL 71
#define C_UPDATE 72
#define C_TERMINATENOW 73
#define C_SUMMARISE 74
#define C_SUMMARISE_SUM 75
#define C_SUMMARISE_AVG 76
#define C_COMPACT 77

#define L_DUPLICATE "duplicate"
#define L_JOIN "join"
#define L_EXIT "exit"


/* Generic function return codes */
#define RETURN_ERROR 0
#define RETURN_SUCCESS 1


/* Define default settings for status flags */
#define DEFAULT_STATUS_DEBUG FALSE
#define DEFAULT_STATUS_DEBUG_LEVEL 0
#define DEFAULT_STATUS_MINDEBUG_LEVEL 9
#define DEFAULT_STATUS_TRACE FALSE
#define DEFAULT_STATUS_TIMING FALSE
#define DEFAULT_STATUS_CASE FALSE
#define DEFAULT_STATUS_QUIET FALSE
#define DEFAULT_STATUS_REGRESSION FALSE
#define DEFAULT_STATUS_TRELS FALSE
#define DEFAULT_STATUS_TIMELOG TRUE
#define DEFAULT_STATUS_LONGLINE FALSE
#define DEFAULT_STATUS_PADDING FALSE
#define DEFAULT_STATUS_TEMPDB TRUE
#define DEFAULT_STATUS_PRODUCTJOIN FALSE
#define DEFAULT_STATUS_DAEMON FALSE
#define DEFAULT_STATUS_MERGESTDERR FALSE

/* Define status option strings */
#define STATUS_TRACE "trace"
#define STATUS_DEBUG "debug"
#define STATUS_DEBUG_LEVEL "debuglevel"
#define STATUS_MINDEBUG_LEVEL "mindebuglevel"
#define STATUS_TIMING "timing"
#define STATUS_CASE "case"
#define STATUS_QUIET "quiet"
#define STATUS_REGRESSION "regression"
#define STATUS_TEMPORARY_RELATIONS "temporary"
#define STATUS_TIMELOG "timelog"
#define STATUS_LONGLINE "long"
#define STATUS_PADDING "padding"
#define STATUS_TEMPDB "tempdb"
#define STATUS_PRODUCTJOIN "productjoin"
#define STATUS_DAEMON "daemon"
#define STATUS_MERGE_STDERR "merge-stderr"
#define VAR_LAST "last"
#define VAR_CURRENTDB "currentdb"

#define STATUS_SETTING_ON "on"
#define STATUS_SETTING_OFF "off"

/***********************************
 * Parser definitions
 ***********************************/
#define PARSER_STRING_DATABASE "database"
#define PARSER_STRING_RELATION "relation"

/***********************************
 * Variable definitions
 ***********************************/
#define VARIABLE_NAME_SIZE 25
#define VARIABLE_VALUE_SIZE 50
#define VARIABLE_NUMBER 25


/* Environment variables */
#define LEAP_ENV_DIR "LEAP_DIR"
#define LEAP_ENV_OPT "LEAP_OPT"


/* OS values */
#define NOATTRIBUTES_TYPE unsigned char
#define ATTRIBUTE_TYPE_TYPE unsigned char
#define ATTRIBUTE_SIZE_TYPE unsigned char
#define ATTRIBUTE_KEYCOMP_TYPE unsigned char
#define RELATION_TEMP_TYPE unsigned char
#define TUPLE_STATUS_TYPE unsigned char
#define RELATION_SYSTEM_TYPE unsigned char

/* Define some tuple status settings */
#define TUPLE_OK 0
#define TUPLE_DELETED 1
#define TUPLE_SUSPECT 2
#define TUPLE_OTHER 255

/* Upgrade is in progress if this is set */
#define LEAP_UPGRADE 1

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

/* Some compilers do not have these defined...
 * (Generally non-ANSI C systems)
 * This is not the best way of doing it, as there
 * is no way of knowing that the system call (if it exists)
 * expects these values/maximums, but this is
 * an attempt at least...
 */
#ifndef FILENAME_MAX
#define FILENAME_MAX 127
#endif

#ifndef SEEK_SET
#define SEEK_SET 0
#endif

#ifndef SEEK_CUR
#define SEEK_CUR 1
#endif

#ifndef SEEK_END
#define SEEK_END 2
#endif

/************************************
 * Special defines for missing items
 * - Some systems don't have full 
 *   ANSI C header files. These have
 *   been found frequently...
 ************************************/
#ifndef FILENAME_SIZE
#define FILENAME_SIZE 127
#endif

#ifndef CLOCKS_PER_SEC
#define CLOCKS_PER_SEC 1
#endif

/* During startup, two scripts are executed
 * startup.src in master, and open.src in
 * the default db.
 */
#define STARTUP_SCRIPT 0
#define OPEN_SCRIPT 1
/* Normal operations means val is at this setting */
#define NORMAL_OPS 2
/* Not yet used, but could easily be */
#define CLOSE_SCRIPT 3

/*
 * Daemon defines
 */
#define LEAPD_PORT 3333
#define LEAPD_BACK_LOG 5

#define MAX_DEBUG_LEVEL 9
#define MIN_DEBUG_LEVEL 0

#define DEBUG_NONE MIN_DEBUG_LEVEL   /* No debug */
#define DEBUG_ENTER 1			     /* function enter/exit */
#define DEBUG_ACK 2				     /* event completion */
#define DEBUG_INFO 3				 /* event info */
#define DEBUG_MODERATE 5			 /* most info */
#define DEBUG_MEM 7					 /* memory allocation */
#define DEBUG_ALL MAX_DEBUG_LEVEL	 /* Everything possible! */

#endif
/* End of ifndef */
