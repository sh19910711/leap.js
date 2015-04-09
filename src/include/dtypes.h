/* 
 * Name: dtypes.h
 * Description: LEAP data types and structure defs.
 * Version: dtypes.h,v 1.207.2.1 2004/02/09 20:05:20 rleyton Exp
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

#ifdef MEMORY_DEBUG
#include "dmalloc.h"
#endif


/* To prevent multiple definitions */
#ifndef _LEAP_DTYPES_
#define _LEAP_DTYPES_

#include <stdio.h>
#include "consts.h"

/**************************************************
 * VERY Important type definitions
 **************************************************/
typedef unsigned char boolean ;
typedef unsigned int word;

/**************************************************
 * Cache structure definitions
 **************************************************/

typedef struct cache_struct {
	unsigned char pos;
	/* Blah Blah - This is TODO */
} cache_struct;

typedef cache_struct *cache;

/**************************************************
 * Hash Table structure definitions
 **************************************************/

#define TABLESIZE 53		/* This should be a prime */
#define TABLETOP TABLESIZE-1
#define REQ_CALC TABLESIZE+1	/* Value to request hash key
				 * is calculated, and NOT provided 
				 */

/* Maximum size of a hash key - all attributes contain data... */
#define HASH_KEY_SIZE MAXIMUM_ALL_DATA

/* Hash key */
/* typedef char hash_keytype[HASH_KEY_SIZE];*/

/* An entry in the structure */
typedef struct chain_struct{
		char element[HASH_KEY_SIZE];
		struct chain_struct	*next,*previous;

		/* Summary values */

		/* Integer value (sum, count, etc.) */
		int ival;
		/* Float - (avg) */	
		float fval;
		/* String - (string max, min etc. */
		char sval[MAXIMUM_EXPRESSION+1];
		/* Worker val. for avg */
		int count;
} chain_struct;

typedef chain_struct *chain;

typedef chain TableType[TABLESIZE];

typedef TableType *HashTable; 

typedef struct node_str{
		char hash_key;
		char item[HASH_KEY_SIZE];
		unsigned int pos;
}node_str;

/**************************************************
 * Attribute structure definitions
 **************************************************/
typedef struct attribute_struct {
		/* The name of the attribute */
		char name[ATTRIBUTE_NAME_SIZE+1];

		/* Format specifier - used by tuple_print etc. */
		char fmt[10];

		/* Whether the attribute is part of primary key. */
		boolean primary;

 		/* Whether the attribute is part of a foreign key. */
		boolean foreign;

		/* Number of records held in attribute - is this
		 * really necessary, or just a hang-on from some
 		 * old system 
		 */
		word norecs;

		/* Attribute number */
		word no;

		/* The type of data held in this attribute */
		/* See DTYPE_* defs */
		char data_type;

		/* The size of the attribute */
		word attrib_size;

		/* The navigational structures for the attribute
 		 * - next and previous attributes 
		 */
		struct attribute_struct *next,*previous;	
} attribute_struct;

/* The main attribute data type - ptr to a structure
 */
typedef attribute_struct *attribute;

/**************************************************
 * Relation structure definitions
 **************************************************/
typedef struct relation_struct {
	/* Add one for the terminating null! */
	char name[RELATION_NAME_SIZE+1];
	char filepath[FILE_PATH_SIZE+1];
	char filename[FILE_NAME_SIZE+1];
	char fieldname[FILE_NAME_SIZE+1];
	/* File structures not included here. This is a
	 * major divurgence from the Pascal version. This is
	 * because it seems inefficent to open file descriptors
	 * on every relation. So on an as needed basis from now
	 * on.
   	 */

	/* Navigational structures combined for simplicity */
	struct relation_struct *next,*previous;

	NOATTRIBUTES_TYPE noattributes;
	word current_pos;
	word last_deleted;  /* offset to last deleted tuple */

	cache rcache;	/* Cache for the relation */
	RELATION_TEMP_TYPE temporary;

	HashTable hash_table;

	boolean updated;

	RELATION_SYSTEM_TYPE system; /* TRUE if a system table or not */
} relation_struct;

typedef relation_struct *relation;

/*
 * This is the main database structure - a pointer 
 * to this will allow access to any database object
 * (eventually after navigation!)
 */
/**************************************************
 * Database structure definitions
 **************************************************/
typedef struct database_struct {
	char name[DATABASE_NAME_SIZE+1];

	relation_struct *first_relation;
	relation_struct *last_relation;

	char basedir[FILE_PATH_SIZE+1];
	
	boolean API;
	boolean subdirs;
	unsigned int datadictionary;
} database_struct;

typedef database_struct *database;



/**************************************************
 * Tuple structure definitions
 **************************************************/

/* The main meat behind our tuple - Each tuple entry contains
 * a ptr to the relation, a ptr to the attribute definition, and
 * a character array with the data.
 */
typedef struct tuple_item_structure {
	/* This is a ptr to the relation to which the tuple relates */
	relation rel;

	/* This is a ptr to the attribute definition to which the
	 * tuple relates
	 */
	attribute att;

	/* This is the actual data! */
	char	data[ATTRIBUTE_MAXIMUM_SIZE+1];

} tuple_item_structure;

/* This is a pointer to the structure above. */
typedef tuple_item_structure *tuple_item_structure_ptr;

/* This is an array of pointers to the structure above. */
typedef tuple_item_structure_ptr tuple_array[MAXIMUM_ATTRIBUTES];

typedef struct tuple_master_struct {
	tuple_array *tuple_data;
    FILE *tuple_file;
	NOATTRIBUTES_TYPE noattributes;	
	long startofdata;
	long offset;
	int		tupleno;
} tuple_master_struct;

typedef tuple_master_struct *tuple;

/* Define some macros to make accessing tuples easier - Otherwise
 * the statements get very confusing...
 */
#define tuple_dat(var) (*(*var).tuple_data)
#define tuple_offset(var) (var)->offset
#define tuple_item(var,pos) tuple_dat(var)[pos]
#define tuple_relation(var,pos) (tuple_item(var,pos))->rel
#define tuple_attribute(var,pos) (tuple_item(var,pos))->att
#define tuple_d(var,pos) (tuple_item(var,pos))->data
#define tuple_data(var,pos,string) strcpy((tuple_item(var,pos))->data,string)
#define tuple_ndata(var,pos,string) strncpy((tuple_item(var,pos))->data,string,(tuple_item(var,pos))->att->attrib_size)
#define tuple_f(var) (var)->tuple_file
#define tuple_no(var) (var)->tupleno

/* Defines for the rebuilding of a tuple structure. We might not
 * always want to rebuild a tuple (expensive)
 */
enum tuple_options { TUPLE_REUSE,
		 TUPLE_BUILD,
		 TUPLE_DISPOSE } ;

/**************************************************
 * Parse Tree node structure definitions
 **************************************************/

typedef struct parse_tree_def {
		char expression[MAXIMUM_EXPRESSION+1];
		char full_expression[MAXIMUM_EXPRESSION+1];
		
		struct parse_tree_def *left, *right, *parent;

		boolean ldone, rdone;

		relation lresult, rresult, result;
	
		char hand;

		/* New parse structure - 04.05.1996 */
		char command[MAXIMUM_EXPRESSION+1],target[MAXIMUM_EXPRESSION+1];
		char first[MAXIMUM_EXPRESSION+1], second[MAXIMUM_EXPRESSION+1];
		char third[MAXIMUM_EXPRESSION+1], work[MAXIMUM_EXPRESSION+1];

		/* Newer parse structure - 12.08.1996 */
		char *expressions[255];
		
		char operation;
} parse_tree_def;

typedef parse_tree_def *parse_tree;

typedef struct pt_stack_node_struct {
	parse_tree data;
	struct pt_stack_node_struct *next;
} pt_stack_node_struct;

typedef struct pt_stack_struct {
	struct pt_stack_node_struct *head_node;
	unsigned int no_items;
} pt_stack_struct;

typedef struct pt_stack_node_struct *pt_stack_node;
typedef struct pt_stack_struct *pt_stack;

/**************************************************
 * Condition list structure definitions
 **************************************************/

/* TODO: Union with a reference to another condition list, in
 * order to support nested conditions.
 * TODO: Get rid of boolean flags, can check NULL on left_attr?
 */

#define ALWAYS_FALSE 0
#define ALWAYS_TRUE 1
#define ALWAYS_UNKNOWN -1


typedef struct ncond_struct {
	char	left[ATTRIBUTE_MAXIMUM_SIZE]; /* TODO: Check size of this */
	boolean	left_field; 				  /* TRUE if left is a field or FALSE if value */
	char *left_attribute_val;
	attribute left_attribute;
	short int left_always;		/* If condition is always TRUE or FALSE */

	char	bool[BOOLEAN_STRING_SIZE]; 	  /* String ver of field */
	int	boolval; 						  /* set to BOOLEAN_whatever */

	char	right[ATTRIBUTE_MAXIMUM_SIZE];
	boolean right_field; 				  /* TRUE If right is a field, FALSE if value */
	char *right_attribute_val;
	attribute right_attribute;
	short int right_always; 	/* If condition is always TRUE or FALSE */
	
	int     boolean_condition; 			  /* AND, OR, XOR, and so on... */
	struct ncond_struct *next_condition;  /* Points to the next condition */

	boolean eval_result;
} ncond_struct;

typedef struct ncond_struct *condp;




/* Hand definitions */
#define NODE_ROOT 0
#define NODE_LEFT 1
#define NODE_RIGHT 2

#define L_DUPLICATE "duplicate"
#define L_JOIN "join"
#define L_EXIT "exit"


/* The type of join method to use. This is
 * used as a parameter to rl_join in rtional
 * module, and affects internal operations
 */
enum join_method {	NATURAL_JOIN,
					EQUI_JOIN,
					THETA_JOIN,
					SEMI_JOIN,
					LEFT_OUTER_JOIN,
					RIGHT_OUTER_JOIN,
					FULL_OUTER_JOIN,
					UNKNOWN_JOIN };

/* Type of key set. */
enum key_set { KEYSET_PRIMARY,
			   KEYSET_FOREIGN,
			   KEYSET_CANDIDATE,
			   KEYSET_UNKNOWN };


/**************************
 * Variable structures
 **************************/
typedef struct var_struct {
	boolean set;
	char name[VARIABLE_NAME_SIZE+1];
	char value[VARIABLE_VALUE_SIZE+1];		
} var_struct;

#endif
/* End of ifndef */
