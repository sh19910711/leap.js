/* 
 * Name: leap_parser.h
 * Description: Parser header files
 * Version: leap_parser.h,v 1.4 2001/04/30 21:07:16 rleyton Exp
 *
 *   LEAP - An extensible and free RDBMS
 *   Copyright (C) 1995-2001 Richard Leyton
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
 *   Richard Leyton, 30c Bassein Park Road, London, W12 9RZ, UK
 *   rleyton@acm.org
 *   http://leap.sourceforge.net
 *
 */

/* Maximum number of sub-nodes at each node in the parse tree */
#define MAX_TREE_NODES 3	/* Maximum number of sub-trees */
#define MAX_LEAF_NODES 25	/* Maximum number of leaf nodes at each level */
#define MAX_SUBCONDITIONS 10	/* Maximum number of subconditions at each level */

/* Define types of node */
#define NODE_TYPE_UNKNOWN 0	/* Uninitialised */
#define NODE_TYPE_BASE 1	/* Root node */
#define NODE_TYPE_NAME 10	/* Name */
#define NODE_TYPE_SELECT 11	/* Select */
#define NODE_TYPE_PROJECT 12	/* Project */
#define NODE_TYPE_JOIN 13	/* Join */
#define NODE_TYPE_UNION 14	/* Union */
#define NODE_TYPE_INTERSECT 15	/* Intersect */
#define NODE_TYPE_DIFFERENCE 16	/* Difference  */
#define NODE_TYPE_PRODUCT 17	/* Product */

/* Define position of node */
#define NODE_POS_BASE	 0	/* Base/Root node */
#define NODE_POS_LEAF	 1	/* Leaf node */
#define NODE_POS_BRANCH	 2	/* Branch node */
#define NODE_POS_UNKNOWN 3	/* Unknown node */

/* Define level of node */
#define NODE_LEVEL_UNKNOWN	0	/* Undefined node */

/* Define a parse tree node structure */
typedef struct parse_leaf_struct {
	char	*item;
} parse_leaf_struct;

/* Define a pointer to the parse tree node */
typedef parse_leaf_struct *parse_leaf;

#define COND_CHAR 0
#define COND_INT 1
#define COND_FLOAT 2
#define COND_SUBCONDITION 3

#define COND_TYPE_ATTRIBUTE 0
#define COND_TYPE_ATOMIC 1

#define COND_RELOP_NOTHING 0
#define COND_RELOP_OR 1
#define COND_RELOP_AND 2
#define COND_RELOP_XOR 3
#define COND_RELOP_NOT 4

/* Define a structure to hold the a component of a condition */
typedef struct condition_side_struct {

	/* What type of condition structure is this? */
	short int ctype;
	
	/* Options */
	union {
		/* String of some description */
		char *cval;
		
		/* Int of some description */
		int ival;
		
		/* Float of some description */
		float fval;
		
		/* ANOTHER condition (!) */
		struct cond_node_struct *subcondition;
	} val;
} condition_side_struct;

/* Define a condition component pointer */
typedef condition_side_struct *condition;

/* Define a condition node structure */
typedef struct cond_node_struct {
	
	/* Left and Right side of the condition */
	condition	lval,rval;
	
	/* Relational operator to apply when evaluating */
	char		*relop;
} cond_node_struct;

typedef cond_node_struct *cond_node;


/* Definition of an evaluation node  (AND, OR, NOT etc.) */
typedef struct cond_eval_node_struct {

	/* Left and right of the evaluation condition */
	cond_node	lcond,rcond;
	
	/* Boolean operator if applicable */
	short int	bool;
} cond_eval_node_struct;
	
typedef cond_eval_node_struct *evaluation_node ;

typedef struct condition_statement_struct {

	int no_sub_conditions;
	
	evaluation_node cond[MAX_SUBCONDITIONS];

} condition_statement_struct;

typedef condition_statement_struct *condition_statement;

/* Define the high-level parser structure */
typedef struct parse_tree_struct {

	int node_type;
	int node_pos;
	int node_level;

	int no_sub_nodes;
	struct parse_tree_struct *node[MAX_TREE_NODES];
	
	int no_leaf_nodes;
	parse_leaf leaf[MAX_LEAF_NODES];
	
	evaluation_node condition;
} parse_tree_struct;

/* Define a pointer to the parse tree root */
typedef parse_tree_struct *parse_tree;

/* External declaration of the entry point to the parser generated code */
extern void init_parser();

extern parse_tree create_tree_node( int node_type, int node_pos, parse_tree subnode, int condition );
extern parse_tree add_tree_node(  parse_tree originaltree, parse_tree newtree );
extern parse_tree create_leaf_node( char *leafdata );
extern parse_tree add_leaf_node( parse_tree originaltree, char *leafdata ) ;
extern cond_node create_condition_node( condition left, char *relop, condition right);
extern condition create_condition_leaf_value( char *conditiondata, short int condtype );
extern condition create_condition_leaf_subcond( cond_node cond );
extern evaluation_node create_evaluation_node( condition lcond, short int bool, condition rcond);
extern condition_statement create_condition_statement ( evaluation_node cond );
extern condition_statement add_condition_statement( condition_statement statement, evaluation_node cond );

/* External declaration of the root tree node */
/* Is there a way this can be incorporated into the lex/yacc? */
extern parse_tree parse_base;


