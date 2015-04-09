/* 
 * Name: algebra.c
 * Description: Algebra Parser
 * Version: algebra.c,v 1.4 2001/04/30 21:07:16 rleyton Exp
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

#include <stdlib.h>
#include <string.h>
#include "algebra.h"

parse_tree alloc_tree_node() {
	parse_tree tree;
	int count;

	printf("allocating tree node\n");
		
	tree=(parse_tree_struct *) malloc(sizeof(parse_tree_struct));

	/* Reset data */
	tree->node_type=NODE_TYPE_UNKNOWN;
	tree->node_pos=NODE_POS_UNKNOWN;
	tree->node_level=NODE_LEVEL_UNKNOWN;
	tree->no_sub_nodes=0;
	
	for (count=0; count<MAX_TREE_NODES; count++)
		tree->node[count]=NULL;

	tree->no_leaf_nodes=0;

	/* Initialise work data */
	tree->returnptr=NULL;
	tree->currentposition=NULL;
	
	for (count=0; count<MAX_LEAF_NODES; count++)
		tree->leaf[count]=NULL;
		
	tree->cond=NULL;

	return(tree);
}

parse_tree create_tree_node( int node_type ,int node_pos, parse_tree subnode, int condition ) {
	
	parse_tree tree;
	
	printf("Request to create a tree node\n");
	
	tree=alloc_tree_node();
	
	tree->node_type=node_type;
	tree->node_pos=node_pos;
	tree->node[0]=subnode;
	if (subnode!=NULL) {
		tree->no_sub_nodes=1;
	} else {
		tree->no_sub_nodes=0;
	}
		
	return(tree);
}

parse_tree add_tree_node( parse_tree originaltree, parse_tree newtree ) {
/*
 * Add a new node to a parse branch node
 */
	
	printf("Request to add a new node to a tree node\n");

	originaltree->no_sub_nodes++;
	originaltree->node[originaltree->no_sub_nodes]=newtree;

	return(originaltree);
}

parse_tree create_leaf_node ( char *leafdata ) {
/*
 * create a leaf node - which will always be a relation or a command.
 * - it must be a tree node in itself, which simply references the leaf.
 */

	parse_leaf leaf;
	parse_tree tree;
	
	printf("Request to create a leaf node\n");
	
	tree=create_tree_node(NODE_TYPE_NAME, NODE_POS_LEAF, NULL, 0);
	
	leaf=(parse_leaf_struct *) malloc(sizeof(parse_leaf_struct));
	
	/* Allocate space to hold the item */
	leaf->item=malloc(sizeof(leafdata));
	
	/* Copy data into the item */
	strcpy(leaf->item,leafdata);
	
	tree->no_leaf_nodes++;
	tree->no_sub_nodes++;
	
	tree->leaf[0]=leaf;
	
	return(tree);
	
}

parse_tree add_leaf_node( parse_tree originaltree, char *leafdata ) {
/*
 * Add a leaf node to an existing leaf branch
 */
 	int leafnum,dsize;
	parse_leaf leaf;
	
 	leafnum=originaltree->no_leaf_nodes;
	originaltree->no_leaf_nodes++;
	
 	originaltree->leaf[leafnum]=(parse_leaf_struct *) malloc(sizeof(parse_leaf_struct));
	leaf=originaltree->leaf[leafnum];
	
	leaf->item=malloc(sizeof(leafdata));
	
	strcpy(leaf->item,leafdata);
	
	return(originaltree);
}


cond_leaf_node create_raw_leaf_node() {

	cond_leaf_node newleaf;

	printf("Allocating a RAW leaf node\n");

	newleaf=(cond_leaf_node_struct *) malloc(sizeof(cond_leaf_node_struct));

	newleaf->leaftype=COND_LEAF_TYPE_UNASSIGNED;
	
	return(newleaf);

}

cond_branch_node create_raw_branch_node() {

	cond_branch_node newbranch;

	printf("Allocating a RAW branch node\n");

	newbranch=(cond_branch_node_struct *) malloc(sizeof(cond_branch_node_struct));

	newbranch->left=NULL;
	newbranch->right=NULL;
	newbranch->boolean_op=COND_BOOLEAN_UNSET;

	return(newbranch);

}

cond_leaf_node create_atom_leaf_node(char *atom) {

	cond_leaf_node leaf;

	printf("Creating an ATOM leaf node\n");

	leaf=create_raw_leaf_node();

	leaf->leaftype=COND_LEAF_TYPE_ATOM;

	leaf->leafval.atom=strdup(atom);

	return(leaf);
}

cond_leaf_node create_ref_leaf_node(char *ref){
	
	cond_leaf_node leaf;

	printf("Creating a REF leaf node [%s]\n",ref);

	leaf=create_raw_leaf_node();

	leaf->leaftype=COND_LEAF_TYPE_ATTRIBUTE;
	leaf->leafval.reference=strdup(ref);

	return(leaf);
}

cond_leaf_node create_cond_leaf_node(condition cond){

	cond_leaf_node leaf;

	printf("Creating a COND leaf node\n");

	leaf=create_raw_leaf_node();

	leaf->leaftype=COND_LEAF_TYPE_CONDITION;
	leaf->leafval.cond=cond;

	return(leaf);
}


condition create_raw_condition(){

	condition_struct *newcondition;

	printf("Creating a RAW condition\n");

	newcondition=(condition_struct *)malloc(sizeof(condition_struct));

	newcondition->left=NULL;
	newcondition->right=NULL;
	newcondition->relational_op=COND_RELOP_NOTHING;

	return(newcondition);
}

cond_branch_node create_branch_node( cond_leaf_node lnode, cond_leaf_node rnode, char *boolean_op){

	cond_branch_node nbranch;

	printf("Creating a BRANCH node\n");
	nbranch=create_raw_branch_node();

	nbranch->left=lnode;
	nbranch->right=rnode;
	nbranch->boolean_op=strdup(boolean_op);

	return(nbranch);
}

cond_leaf_node create_predicate_leaf_node(cond_leaf_node lpred, cond_leaf_node rpred, char *bool){
	cond_leaf_node leaf;
	condition ncond;

	printf("Creating a PREDICATE leaf node\n");

	leaf=create_raw_leaf_node();

	leaf->leaftype=COND_LEAF_TYPE_PREDICATE;

	leaf->leafval.predicate=create_branch_node(lpred, rpred, bool);		

	return(leaf);
}
    
    
condition create_cond_statement( cond_branch_node cond){
	condition ncond;
	printf("Creating a CONDITION STATEMENT\n");

	ncond=create_raw_condition();

	ncond->left=cond;
	ncond->right=NULL;
	ncond->relational_op=COND_RELOP_NOTHING;	

	return(ncond);
}

condition create_branch_statement( cond_leaf_node lcond, int relational_op, cond_leaf_node rcond){ 
/* A > B */
	cond_branch_node nbranch;
	condition ncond;
	printf("Creating a CONDITION *BRANCH* STATEMENT\n");

	/* create the structures */
	ncond=create_raw_condition();
	nbranch=create_raw_branch_node();	

	/* populate the structures from params. */
	nbranch->left=lcond;
	nbranch->right=rcond;
	ncond->relational_op=relational_op;

	/* attach structs to condition */
	ncond->left=nbranch;

	return(ncond);	
}


parse_tree add_condition( parse_tree originaltree, condition cond) {

/* attach a condition tree to a parse tree */

	originaltree->cond=cond;

	return(originaltree);

}

int print_leaf_node( parse_leaf leaf, int count ) {

	printf("%d -Leaf Node ptr: [%p]\n",count,leaf);
	
	if (leaf!=NULL) {
	
		printf("%d -Leaf item: %s\n",count,leaf->item);
		
	} else {
		printf("-NULL Leaf node encountered!\n");
	}

}

int print_leaf( cond_leaf_node leaf , int level) {
	if (leaf!=NULL) {

		printf("@ level [%d]\nDumping leaf node :\n",level);

		printf("Leaf type: ");
		switch (leaf->leaftype) {
			case COND_LEAF_TYPE_UNASSIGNED: printf("[unassigned]\n");break;
			case COND_LEAF_TYPE_ATTRIBUTE: printf("[attrib]\n");break;
			case COND_LEAF_TYPE_ATOM: printf("[atom]\n");break;
			case COND_LEAF_TYPE_CONDITION: printf("[condition]\n");break;
			case COND_LEAF_TYPE_PREDICATE: printf("[predicate]\n");break;
			default: printf("[Don't care, don't know]\n");
		}	

		/* Yes, this is a bit inefficient, but this is a DUMP routine,
         * and it makes things logically clear (i think!)
		 */

		switch (leaf->leaftype) {

			case COND_LEAF_TYPE_UNASSIGNED: printf("Unassigned Leaf type - no work!\n");break;

			case COND_LEAF_TYPE_ATTRIBUTE: printf("LEAF Attrib. ref: [%s]\n",leaf->leafval.reference); break;

			case COND_LEAF_TYPE_ATOM: printf("LEAF Atom: [%s]\n",leaf->leafval.atom); break;

			case COND_LEAF_TYPE_CONDITION: printf("*********CONDITION**********\n");
										   printf("*********START of recursion...\n");
										   print_alg_condition(leaf->leafval.cond,level+1);	
										   printf("*********END of recursion (now @ level [%d])...\n",level);
										   break;

			case COND_LEAF_TYPE_PREDICATE: printf("#########PREDICATE##########\n");
										   printf("#########Start of recursion...\n");
										   print_branch_node(leaf->leafval.predicate,level+1);
										   printf("#########End of recursion (now @ level [%d])...\n",level);
										   break;

			default:printf("Unimplemented leaf type!\n");
			
		}

	} else {
		printf("NULL leaf\n");
	}
}

int print_branch_node( cond_branch_node node, int level ) {

	if (node!=NULL) {
		printf("@ level [%d]\nDumping branch node :\n",level);

		printf("Boolean op [%s]: ",node->boolean_op);

		printf("left leaf ptr : [%p]\n",node->left);
		print_leaf(node->left,level);
		printf("right leaf ptr: [%p]\n",node->right);
		print_leaf(node->right,level);

	} else {
		printf("NULL node!\n");
	}
}

int print_alg_condition( condition cond, int level ){
	
	printf("@ level [%d]\nDumping condition :\n",level);

	printf("Relational op: ");
	switch(cond->relational_op) {
		case COND_RELOP_NOTHING : printf("<nothing>");
								  break;
		case COND_RELOP_OR : printf("<OR>");
							 break;
		case COND_RELOP_AND : printf("<AND>");
							  break;
		case COND_RELOP_XOR: printf("<XOR>");
							 break;
		case COND_RELOP_NOT: printf("<NOT>");
							 break;
		default: printf(">>default!<<");
				 break;
	}

	printf("\nLeft ptr : [%p]\nRight ptr: [%p]\n",cond->left, cond->right);

	printf("START LEFT.\n");
	print_branch_node(cond->left,level);

	printf("START RIGHT.\n");
	print_branch_node(cond->right,level);

	printf("DONE condition dump\n");

}


int print_node( parse_tree ptree, int perceivedlevel ) {

	int count;
	
	printf("Node ptr: [%p]\n",ptree);
	
	if (ptree!=NULL) {
	
		printf("Node type: ");
		switch (ptree->node_type) {
			case NODE_TYPE_UNKNOWN: printf("Unknown (unset?) node\n");
						break;
			case NODE_TYPE_BASE:	printf("Base node\n");
						break;
			case NODE_TYPE_NAME:	printf("Name node\n");
						break;
			case NODE_TYPE_SELECT:	printf("Select node\n");
						break;
			case NODE_TYPE_JOIN:	printf("Join node\n");
						break;
			case NODE_TYPE_PROJECT:	printf("Project node\n");
						break;

			case NODE_TYPE_TERMINATE:	printf("Termination (exit program) node\n");
						break;

			default:		printf("Unknown (default) node\n");
						break;
		}
		
		printf("Node position: ");
		switch (ptree->node_pos) {
			case NODE_POS_BASE:	printf("Base node\n");
						break;
			case NODE_POS_LEAF:	printf("Leaf node\n");
						break;
			case NODE_POS_BRANCH:	printf("Branch node\n");
						break;
			case NODE_POS_UNKNOWN:	printf("Unknown (unset?) node\n");
						break;
			default:		printf("Unknown (default) node\n");
						break;
		}
		
		printf("Node level: 	 [%d]\n",ptree->node_level);
		printf("Perceived level: [%d]\n",perceivedlevel);
		printf("Sub-nodes:  	 [%d]\n",ptree->no_sub_nodes);
		printf("Return Ptr:  	 [%p]\n",ptree->returnptr);
		printf("Work Counter:  	 [%p]\n",ptree->currentposition);
		
		printf("Leaf-nodes: 	 [%d]\n",ptree->no_leaf_nodes);

		if (ptree->no_leaf_nodes>0) {
			for (count=0; ((count<MAX_LEAF_NODES)&&(count<ptree->no_leaf_nodes)); count++) {
				print_leaf_node(ptree->leaf[count],count);
			}
		}

		for (count=0; ((count<MAX_TREE_NODES))/*&&(count<ptree->no_sub_nodes))*/;count++) {
			/* Recurse into sub-tree */
			printf("[%d]\n",count);
			print_node(ptree->node[count],perceivedlevel+1);	
		}

		if (ptree->cond!=NULL) {
			printf("---Wohoo!--- Condition present\n");
			print_alg_condition(ptree->cond,0);
		} else {
			printf("*NO* Condition present\n");
		}
		
		printf("RETURNED to perceived level [%d]\n",perceivedlevel);
		
	} else {
		printf("NULL node encountered!\n");
	}
}

int dump_parse_tree ( parse_tree ptree ) {
/* 
 * Dump out the generated parse tree 
 */

	printf("\n---DUMPING PARSE TREE---\n");
	if (ptree!=NULL) {
	
		print_node(ptree,0);
	
	} else {
	
		printf("Null parse tree!\n");
	
	}

}


int dispose_parse_tree( parse_tree *ptree ) {

	parse_tree subtree;
	parse_leaf subleaf;
	int nodecount,leafcount;

	if ((ptree!=NULL)&&(*ptree!=NULL)) {
		printf("Disposing of tree...\n");

		for (nodecount=0;nodecount<MAX_TREE_NODES;nodecount++) {

			subtree=(*ptree)->node[nodecount];
		
			for (leafcount=0;leafcount<MAX_LEAF_NODES;leafcount++) {
				subleaf=(*ptree)->leaf[leafcount];
		
				if (subleaf!=NULL) free(subleaf);
			}
		
			dispose_parse_tree(&subtree);
		}

		free(*ptree);

		printf("DONE Disposing of tree.\n");
	} else {
		printf("Passed or referenced a NULL tree...\n");
	}
}

/* Actual declaration of the base node of the parse tree */
parse_tree parse_base;

/*
int main() {

	printf("Start of main.\n");
	
	printf("Starting parser...\n");
	
	parse_base=NULL;

	if (init_parse()) {
		printf("Successful!\n");
		printf("parse_base p: [%p]\n",parse_base);
		dump_parse_tree(parse_base);
		dispose_parse_tree(&parse_base);
	} else {
		printf("UNSuccessful!\n");
	}
}
*/

