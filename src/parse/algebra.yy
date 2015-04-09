/*
 * Name: algebra.yy
 * Description: LEAP yacc parser (Algebra)
 * Version: algebra.yy,v 1.206 2001/04/30 21:07:16 rleyton Exp
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
 */
 
/* Include the leap definitions of various types */
%{

#include "algebra.h"

%}


%union {
	int intval;
	double floatval;
	char *strval;
	char *subtok;
	parse_tree ptree;
	cond_leaf_node leaf_node;
	cond_branch_node branch_node;
	condition cond;
}

/* Define the types of various tokens */
%token <strval> NAME
%token <strval> INTNUM
%token <strval> STRING

%left <subtok> COMPARISON 
%left OR
%left AND
%left NOT
%left '+' '-'
%left '*' '/'
%nonassoc UMINUS

/* Relational operations */
%token UNION
%token JOIN
%token INTERSECT
%token DIVIDE
%token PROJECT
%token DIFFERENCE
%token PRODUCT
%token SELECT
%token DISPLAY
%token ADD

/* Support operations */
%token LIST
%token COMMENT

/* Termination operations */
%token EXIT
%token TERMINATOR

/* Pretty much every item is of type 'ptree' */
%type <ptree> relation
%type <ptree> algebra
%type <ptree> select_statement
%type <ptree> project_statement
%type <ptree> join_statement
%type <ptree> union_statement
%type <ptree> intersect_statement
%type <ptree> difference_statement
%type <ptree> product_statement
%type <ptree> attribute_comma_list
%type <ptree> value_comma_list
%type <ptree> string_comma_list
%type <ptree> display_statement
%type <ptree> add_statement
%type <ptree> support_operation
%type <ptree> expression
%type <strval> value_ref
%type <strval> string_ref
%type <strval> atom;
%type <cond> condition_statement;
%type <cond> condition;
%type <leaf_node> comparison_predicate;

%%

expression:
		algebra  TERMINATOR
		{
			parse_base=create_tree_node( NODE_TYPE_BASE,NODE_POS_BASE, $1, 0 );
			printf("Expression resolved.\n");
		}
	|	EXIT TERMINATOR
		{
			printf("Termination request.\n");
			parse_base=create_tree_node( NODE_TYPE_TERMINATE,NODE_POS_BASE, 0, 0 );
		}
	|	support_operation TERMINATOR
		{
			printf("Support operations.\n");
			parse_base=create_tree_node( NODE_TYPE_SUPPORT, NODE_POS_BASE, $1, 0 );
		}	
	|	error TERMINATOR
		{
			yyclearin;
			yyerrok;
			printf("Error handled\n");
		}
	;

support_operation:
		LIST {
			$$=create_tree_node( NODE_TYPE_LIST, NODE_POS_BRANCH, 0, 0);
		}
		;
	
algebra:	
			relation
		|	select_statement
		|	project_statement
		|	join_statement
		|	union_statement
		|	intersect_statement
		|	difference_statement
		|	product_statement
		|	display_statement
		|	add_statement
		;

select_statement:	
			SELECT '(' algebra ')' condition
			{
				$$=add_condition(create_tree_node( NODE_TYPE_SELECT,NODE_POS_BRANCH, $3, 0 ),$5);
			}
	;

project_statement:
			PROJECT '(' algebra ')' '(' attribute_comma_list ')'
			{
				
				$$=add_tree_node(create_tree_node( NODE_TYPE_PROJECT,NODE_POS_BRANCH, $3, 0 ),
				$6);

			}
	;

add_statement:
			ADD '(' algebra ')' '(' string_comma_list ')'
			{
				$$=add_tree_node(create_tree_node( NODE_TYPE_ADD,NODE_POS_BRANCH, $3, 0), $6);
			}
	;

join_statement:
			JOIN '(' algebra ')' '(' algebra ')' condition
			{
				$$=add_tree_node(create_tree_node( NODE_TYPE_JOIN,NODE_POS_BRANCH, $3, 0 ),
					$6);
			}
	;

union_statement:
			UNION '(' algebra ')' '(' algebra ')'
			{
				$$=add_tree_node(create_tree_node( NODE_TYPE_UNION,NODE_POS_BRANCH, $3, 0 ),
					$6);
			}
	;

intersect_statement:
			INTERSECT '(' algebra ')' '(' algebra ')'
			{
				$$=add_tree_node(create_tree_node( NODE_TYPE_INTERSECT,NODE_POS_BRANCH, $3, 0 ),
					$6);
		
			}

		;

difference_statement:
			DIFFERENCE '(' algebra ')' '(' algebra ')'
			{
				$$=add_tree_node(create_tree_node( NODE_TYPE_DIFFERENCE,NODE_POS_BRANCH, $3, 0 ),
					$6);
			}
	;

product_statement:
			PRODUCT '(' algebra ')' '(' algebra ')' 
			{
				$$=add_tree_node(create_tree_node( NODE_TYPE_PRODUCT,NODE_POS_BRANCH, $3, 0 ),
					$6);
			}
	;
			
			
display_statement:
			DISPLAY '(' algebra ')'
			{
				$$=create_tree_node( NODE_TYPE_DISPLAY, NODE_POS_BRANCH, $3, 0 );
			}
	;
	
condition:
		'(' condition_statement ')'
		{
			printf("@@Condition!\n");
			$$=$2;
		}
	;

condition_statement: 
		comparison_predicate OR comparison_predicate
		{
			printf("@condition_statement|OR\n");
			$$=create_branch_statement( $1, COND_RELOP_OR, $3 );
		}
	|	comparison_predicate AND comparison_predicate
		{
			printf("@condition_statement|AND\n");
			$$=create_branch_statement( $1, COND_RELOP_AND, $3 );
		}
	|	NOT comparison_predicate
		{
			printf("@condition_statement|NOT\n");
			$$=create_branch_statement( $2, COND_RELOP_NOT, 0 );
		}
	|	comparison_predicate
		{
			printf("@condition_statement|NOTHING\n");
			$$=create_branch_statement( $1, COND_RELOP_NOTHING, 0 );
		}
	;

comparison_predicate:
		atom 
		{
			printf("@comparison_predicate|atom\n");
			/* References an atomic value */
			$$=create_atom_leaf_node($1);
		}
	|	value_ref
		{
			printf("@comparison_predicate|attribute_ref\n");
			/* References an attribute */
			$$=create_ref_leaf_node($1);
		}
	|	condition
		{
			printf("@comparison_predicate|condition\n");
			$$=create_cond_leaf_node($1);
		}
	| comparison_predicate COMPARISON comparison_predicate
		{
			printf("@comparison_predicate|COMPARISON\n");
			$$=create_predicate_leaf_node( $1, $3, $2);
		}
	;


relation:	
		NAME 
		{
			printf("Relation Name is [%s]\n",$1);
			$$=create_leaf_node($1,LEAF_TYPE_RELATION);
		}
	|	NAME '.' NAME  
		{
			printf("Database name is [%s]\nRelation Name is [%s]\n",$1,$3);
	
		}
	;

value_comma_list:
		value_ref
		{
			printf("CREATING Leaf (Value [%s])\n",$1);
			$$=create_leaf_node($1,LEAF_TYPE_VALUE);
		}
	|	value_ref "," value_comma_list
		{
			printf("ADDING to leaf (Value [%s])\n",$1);
			$$=add_leaf_node($3,$1,LEAF_TYPE_VALUE);
		}
	;

attribute_comma_list:
		value_ref
		{
			printf("CREATING Leaf (Attribute Name [%s])\n",$1);
			$$=create_leaf_node($1,LEAF_TYPE_ATTRIBUTE);		
		}
	|	value_ref ',' attribute_comma_list
		{
			printf("ADDING to leaf (Attribute Name [%s])\n",$1);
			$$=add_leaf_node($3,$1,LEAF_TYPE_ATTRIBUTE);
		}
	;

string_comma_list:
		string_ref
		{
			printf("CREATING Leaf (String Name [%s])\n",$1);
			$$=create_leaf_node($1,LEAF_TYPE_MISC);		
		}
	|	string_ref ',' string_comma_list
		{
			printf("ADDING to leaf (String Name [%s])\n",$1);
			$$=add_leaf_node($3,$1,LEAF_TYPE_MISC);
		}
	;

string_ref:
		STRING
		{
			printf("RETURNING value [%s]\n",$1);
			$$=$1;
		}
	;
	
value_ref:
		NAME
		{
			printf("RETURNING value [%s]\n",$1);
			$$=$1;
		}
	|	NAME '.' NAME 
	|	NAME '.' NAME '.' NAME
	;

atom:
		STRING
		{
			printf("RETURNING Atomic (string) value [%s]\n",$1);
			$$=$1;
		}
	|	INTNUM
		{
			printf("RETURNING Atomic (int) value as a string [%s]\n",$1);
			$$=$1;
		}
	;
%%
