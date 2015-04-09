/* 
 * Name: leap_parser.h
 * Description: Parser header files
 * Version: algebra.h,v 1.4 2001/04/30 21:07:16 rleyton Exp
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

#include "defines.h"

/* To prevent multiple definitions */
#ifndef _LEAP_PARSER_
#define _LEAP_PARSER_

#include <stdio.h>

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
#define NODE_TYPE_TERMINATE 100	/* Termination */

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


#define COND_RELOP_NOTHING 0
#define COND_RELOP_OR 1
#define COND_RELOP_AND 2
#define COND_RELOP_XOR 3
#define COND_RELOP_NOT 4


#define COND_BOOLEAN_UNSET 0
#define COND_BOOLEAN_EQUAL 1
#define COND_BOOLEAN_NEQUAL 2
#define COND_BOOLEAN_GTHAN 3
#define COND_BOOLEAN_LTHAN 4
#define COND_BOOLEAN_LEQUAL 5
#define COND_BOOLEAN_GEQUAL 6

#define COND_LEAF_TYPE_UNASSIGNED 0
#define COND_LEAF_TYPE_ATTRIBUTE 1
#define COND_LEAF_TYPE_ATOM 2
#define COND_LEAF_TYPE_CONDITION 3
#define COND_LEAF_TYPE_PREDICATE 4 

/* A condition predicate - (lval) (boolean operator) (rval) */
typedef struct cond_branch_node_struct {
	/* We have to give a forward declaration of cond_leaf_node_struct */
	struct cond_leaf_node_struct *left;
	struct cond_leaf_node_struct *right;
	char *boolean_op;
} cond_branch_node_struct;

typedef cond_branch_node_struct *cond_branch_node;

/* Definition of a structure to contain a condition and the relational
 * operators applying to it's two sides, eg. (cond) {AND|OR|XOR|...} (cond)
 */
typedef struct condition_struct {

	cond_branch_node left,right;
	int	relational_op;
} condition_struct;

typedef condition_struct *condition;


/* Definition of a structure to hold a condition predicate,
 * e.g. 1, 'attrib', TRUE, or a subcondition.
 */
typedef struct cond_leaf_node_struct {

	int leaftype;

	union leafval_union {
		char *atom;
		char *reference;
		condition cond;
		cond_branch_node predicate;
	} leafval;

} cond_leaf_node_struct;

typedef cond_leaf_node_struct *cond_leaf_node;

/* External declaration of the condition generation routines */
extern cond_leaf_node create_atom_leaf_node(char *atom);
extern cond_leaf_node create_ref_leaf_node(char *ref);
extern cond_leaf_node create_cond_leaf_node(condition cond);
extern cond_leaf_node create_predicate_leaf_node(cond_leaf_node lpred, cond_leaf_node rpred, char *bool);

extern cond_branch_node create_branch_node( cond_leaf_node lnode, cond_leaf_node rnode, char *boolean_op);

extern condition create_cond_statement( cond_branch_node cond);
extern condition create_condholder( cond_leaf_node lnode);
extern condition create_branch_statement( cond_leaf_node lcond, int relational_op, cond_leaf_node rcond);

/* Define the high-level parser structure */
typedef struct parse_tree_struct {

	int node_type;
	int node_pos;
	int node_level;

	int no_sub_nodes;
	struct parse_tree_struct *node[MAX_TREE_NODES];

	int no_leaf_nodes;
	parse_leaf leaf[MAX_LEAF_NODES];

	condition cond;

	/* Work data - used when traversing/processing tree */

	/* Pointer to the return value */
	void *returnptr;

	/* Indicates current node */
	int currentposition;

} parse_tree_struct;

/* Define some access defines */
#define ptree_type(var) var->node_type
#define ptree_no_leafnodes(var) var->no_leafnodes
#define ptree_leafnode(var,pos) var->leaf[pos];
#define ptree_no_subnodes(var) var->no_sub_nodes
#define ptree_subnode(var,pos) var->node[pos]
#define ptree_currentpos(var) var->currentposition
#define ptree_returnptr(var) var->returnptr

/* Define a pointer to the parse tree root */
typedef parse_tree_struct *parse_tree;

/* External declaration of the entry point to the parser generated code */
extern void init_parser();

/* External declaration of the parse tree generation routines */
extern parse_tree create_tree_node( int node_type, int node_pos, parse_tree subnode, int condition );
extern parse_tree add_tree_node(  parse_tree originaltree, parse_tree newtree );
extern parse_tree create_leaf_node( char *leafdata );
extern parse_tree add_leaf_node( parse_tree originaltree, char *leafdata ) ;

extern parse_tree add_condition( parse_tree originaltree, condition cond);
/* External declaration of the root tree node */
/* Is there a way this can be incorporated into the lex/yacc? */
extern parse_tree parse_base;

extern int init_parse();

extern FILE *yyin;

#endif
