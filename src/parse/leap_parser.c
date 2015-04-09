/* 
 * Name: leap_parser.c
 * Description: Parser
 * Version: leap_parser.c,v 1.5 2001/04/30 21:07:16 rleyton Exp
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
#include "leap_parser.h"


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
	
	for (count=0; count<MAX_LEAF_NODES; count++)
		tree->leaf[count]=NULL;
		
	tree->condition=NULL;

	return(tree);
}

parse_tree create_tree_node( int node_type ,int node_pos, parse_tree subnode, int condition ) {
	
	parse_tree tree;
	
	printf("Request to create a tree node\n");
	
	tree=alloc_tree_node();
	
	tree->node_type=node_type;
	tree->node_pos=node_pos;
	tree->node[0]=subnode;
	tree->no_sub_nodes=0;
		
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

cond_node create_cond_node() {
	
	printf("Request to create a condition node\n");
	
	return(NULL);
}

void init_parser( parse_tree subnode, cond_node condition) {

	printf("Request to initialise a parse tree\n");
	
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


cond_node create_condition_node( condition left, char *relop, condition right ) {
/*
 * Create a node in the condition tree which consists of a left and right item, and 
 * a relational operator, the result of which should be TRUE or FALSE.
 */

}

condition create_condition_leaf_value( char *conditiondata ) {
/*
 * Create a leaf in the condition tree which is initially an atomic value or an attribute
 */

}

condition create_condition_leaf_subcond( cond_node *cond ) {
/* 
 * Create a leaf in the condition tree which is another condition tree itself
 */
}
 

int print_leaf_node( parse_leaf leaf, int count ) {

	printf("%d -Leaf Node ptr: [%p]\n",count,leaf);
	
	if (leaf!=NULL) {
	
		printf("%d -Leaf item: %s\n",count,leaf->item);
		
	} else {
		printf("-NULL Leaf node encountered!\n");
	}

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
		
		printf("Leaf-nodes: 	 [%d]\n",ptree->no_leaf_nodes);

		if (ptree->no_leaf_nodes>0) {
			for (count=0; ((count<MAX_LEAF_NODES)&&(count<ptree->no_leaf_nodes)); count++) {
				print_leaf_node(ptree->leaf[count],count);
			}
		}

		for (count=0; ((count<MAX_TREE_NODES))/*&&(count<ptree->no_sub_nodes))*/;count++) {
			/* Recurse into sub-tree */
			print_node(ptree->node[count],perceivedlevel+1);	
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

