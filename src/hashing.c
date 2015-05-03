/* 
 * Name: hashing.c
 * Description: Hashing functions
 * Version: hashing.c,v 1.205.2.1 2004/02/09 20:05:20 rleyton Exp
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
#include <stdio.h>
#include <stdlib.h>
#include "consts.h"
#include "globals.h"
#include "dtypes.h"
#include "errors.h"
#include "hashing.h"
#include "util.h"
#include "tuples.h"
#include "leapio.h"
#include "relation.h"

/* Useful macro for getting at the elements in a Hash Table */
#define element(htable,psn) (*htable)[psn]

long counter_cnode=0;

position hash( char *key ) {
/* hash
 * Returns a hash key, using folding
 */
	unsigned int h,c,slen;

	/* Get the first value. Automatic conversion to number */
	h=key[0];

	/* Get the length to save hassle */
	slen=strlen(key);
#ifdef FULL_DEBUG
	fprintf(stderr,"Length of >%s< : %d\n",key,slen);
#endif

	/* Move through the string */
	for(c=0;c<slen;c++) {

#ifdef FULL_DEBUG
	fprintf(stderr,">%c:%d<",key[c],c);
#endif
		/* Do the calculation... */
		h=( (h*128)+(key[c]) ) % TABLESIZE;
	}

#ifdef FULL_DEBUG
	fprintf(stderr,"Hash Key (TABLESIZE==%i): %u\n",TABLESIZE,h);
#endif
	/* Return the result */

	return( h );
}



position rehash(    position p ) {
/* rehash
 * Rehash a value
 */

	/* Rehash and return the result */
	return( (p+1) % TABLESIZE );

}



chain chain_node_create( char *e ) {
/* chain_node_create
 * Create a chain node
 */
	chain rchain;

	/* Allocate the memory and check its ok */
	rchain=(chain_struct *) malloc(sizeof(chain_struct));
	check_assign(rchain,"hashing.chain_node_create");
	
	/* Reset the structure, and copy the data */
	rchain->next=NULL;
	rchain->previous=NULL;
	strcpy(rchain->element,e);

	do_debug(DEBUG_MEM,"+++ Chain Node: [%s]\n",e);

	counter_cnode++;

	/* Return the new structure */
	return(rchain);
}

void hashing_insert( HashTable HT,
			char *e,
			position hk) {
/* hashing_insert
 * Insert a key into the specified hashing table
 */
	position psn;
	chain nchn,pchnn,cchnn;	

	do_debug(DEBUG_INFO,"Enter: (hashing) hashing_insert\n");

	if (hk>REQ_CALC) {
		raise_error(ERROR_HASH_FILE_CORRUPT,FATAL,"");
	}

	/* If the hash key has not been provided */	
	if (hk==REQ_CALC) {
		/* Caclculate the hash key */
		psn=hash(e);
	} else {
		/* Otherwise use the value provided */
		psn=hk;
	}

	/* Create ourselves a new node */
	nchn=chain_node_create(e);
	do_debug(DEBUG_MEM,"--- node [%p]\n",nchn);

	/* If there is no chain node at the head */
	if ( (*HT)[psn]==NULL) {
		/* Put the node at the head of the chain */
		(*HT)[psn]=nchn;
	} else {
		/* Otherwise, find the right position */
		pchnn=NULL;
		cchnn=(*HT)[psn];

		/* Locate the node AFTER the destination */
		while ( (cchnn!=NULL) && (strcmp(e,cchnn->element)>0) ){
			pchnn=cchnn;
			cchnn=cchnn->next;
		}

		/* If the end of the list has been reached, then add
		 * the node at the end
		 */
		if ( cchnn==NULL ) {
			/* Set the PREVIOUS chain item to point to it's next item
			 * (the new chain)
			 */
			pchnn->next=nchn;
			/* Set the new item's PREVIOUS chain item 
			 */
			nchn->previous=pchnn;

			do_debug(DEBUG_MEM,"Node [%p]: Inserted at the END of the chain\n",nchn);



			/* If the node to be inserted should go before the
			 * first node, adjust the header information
			 */
		} else if ( strcmp(cchnn->element,(*HT)[psn]->element)==0) {
			nchn->next=(*HT)[psn];
			(*HT)[psn]=nchn;

			do_debug(DEBUG_MEM,"Node [%p]: Inserted at the BEGINNING of the chain (Master ptr updated)\n",nchn);

			/* Otherwise insert the new node before the current 
			 * node
 			 */
		} else if ( strcmp(e, cchnn->element)<0 ) {
			nchn->next=cchnn;
			nchn->previous=pchnn;
			pchnn->next=nchn;	

			do_debug(DEBUG_MEM,"Node [%p]: Inserted in the MIDST of the chain\n",nchn);

			/* Unless its already in the list, in which case
			 * we may as well just dispose of the memory and report
			 * an error (of sorts!)
			 */
		} else {
/*			leap_fprintf(stderr,"Duplicate Entry!"); */
			/* Free the memory */
			free(nchn);
			counter_cnode--;
			do_debug(DEBUG_MEM,"Node [%p]: Already present in the chain - New node disposed\n",nchn);
		}	
	}	
	do_debug(DEBUG_MEM,"*** Node balance: %d\n",counter_cnode);
	do_debug(DEBUG_INFO,"Exit: (hashing) hashing_insert\n");
}

void findposn( HashTable HT,
		char *tkey,
		position *posn,
		chain *chn,
		boolean *found) {
/* findposn
 * Results: If HT contains an element with key value tkey
 * 	    then found is true and posn is that element's
 *	    position. Otherwise found is false
 */

/* Locates a node in the specified hash table, and returns the
 * Position and boolean value for the search result
 */

	position home;
	boolean finished;
	chain cchnn;

	do_debug(DEBUG_INFO,"Enter: (hashing) findposn\n");

	/* Reset the found indicator */
	*found=FALSE;

	/* Calculate the hash key for the node if it were
	 * being inserted 
	 */
	home=hash(tkey);

	(*posn)=home;
	finished=FALSE;

	cchnn=(*( HT ))[ *posn ];

	/* Locate the chain node on the specified hash table */
	while (!(finished)) {
		if ( (cchnn==NULL) || ( strcmp(cchnn->element,tkey)==0 ) ) {
			finished=TRUE;
		} else {
			cchnn=cchnn->next;
		}
	}
	
	*found=(cchnn!=NULL);
	*chn=cchnn;

	do_debug(DEBUG_INFO,"Exit: (hashing) findposn\n");

}

void hashing_delete ( HashTable HT,
		char *e,
		boolean *updated ) {
/* hashing_delete
 * Locate a given node in the hash table, and delete it
 */
	position posn;
	boolean found;
	chain chn,hchn;

	do_debug(DEBUG_MODERATE,"Start: hashing_delete\n");

	*updated=FALSE;

	/* Locate the node */
	findposn( HT, e, &posn, &chn, &found );

	/* If the search was successful */
	if (found) {
		if (chn->previous==NULL) {
			do_debug(DEBUG_MEM,"Deleting head of chain\n");
			
			/* Set the header item */
			hchn=(*HT)[posn]=chn->next;
			
			/* Update the head item's previous ptr */
			if (chn->next!=NULL) {
				chn->next->previous=NULL;
			}

			/* Release the memory */
			free(chn);
						
		} else if (chn->next==NULL) {
			do_debug(DEBUG_MEM,"Deleting tail of chain\n");
			
			/* Update the new tail item */
			if (chn->previous!=NULL) {
				chn->previous->next=NULL;
			}
			
			/* Release the memory */
			free(chn);
		
		} else {
			do_debug(DEBUG_MEM,"Deleting normal entry in chain\n");
			
			/* Update the previous item */
			if (chn->previous!=NULL) {
				chn->previous->next=chn->next;
			}
			
			/* Update the next item */
			if (chn->next!=NULL) {
				chn->next->previous=chn->previous;
			}
			
			/* Release the memory */
			free(chn);
			
		}

		/* Keep track of the node counter */
		counter_cnode--;
		
		/* Flag the hash table as updated */
		*updated=TRUE;
	}
}

void update ( HashTable HT,
		char *e,
		boolean *updated ) {
/* update
 * Locate a given node in the hash table, and update its
 * contents to those specified 
 */
	position posn;
	boolean found;
	chain chn;

	*updated=FALSE;

	/* Locate the node */
	findposn( HT, e, &posn, &chn, &found );

	/* If the search was successful */
	if (found) {

		/* Update the elements value */
		strcpy(chn->element,e);

		/* Return true for updating */
		*updated=TRUE;
	}
}

void hashing_retrieve_extra(   HashTable HT,
			char *tkey,
			char *e,
			int *count,
			float *fval,
			char *sval,
			int *ival,
			boolean *retrieved) {
/* hashing_retrieve_extra
 * Retrieve a given node (and extra values) from the hash table 
 */
	position posn;
	boolean found;
	chain chn;

	*retrieved=FALSE;

	/* Locate the given node in the hash table */
	findposn ( HT, tkey, &posn, &chn, &found);

	/* If the node exists in the node */
	if (found) {
		/* Return the elements contents */
		strcpy(e,chn->element);

		count=&chn->count;
		ival=&chn->ival;
		fval=&chn->fval;
		sval=chn->sval;

		/* And update the success flag */
		*retrieved=TRUE;
	}
}
void hashing_retrieve(   HashTable HT,
			char *tkey,
			char *e,
			boolean *retrieved) {
/* hashing_retrieve
 * Retrieve a given node from the hash table 
 */
	position posn;
	boolean found;
	chain chn;

	do_debug(DEBUG_ENTER,"Enter: hashing_retrieve (%s)\n",tkey);

	*retrieved=FALSE;

	/* Locate the given node in the hash table */
	findposn ( HT, tkey, &posn, &chn, &found);

	/* If the node exists in the node */
	if (found) {
		/* Return the elements contents */
		strcpy(e,chn->element);

		/* And update the success flag */
		*retrieved=TRUE;
	}
	
	do_debug(DEBUG_ENTER,"Exit: hashing_load\n");

}
	
HashTable hashing_create(void) {
/* hashing_create
 * Create a new hashing table, and return a pointer to it
 */
	unsigned int k;
	HashTable HT;

	/* Create the hash table, and check */
  do_debug(DEBUG_ALL,"MY: sizeof(chain)==%i\n", sizeof(chain));
  do_debug(DEBUG_ALL,"MY: sizeof(chain_struct)==%i\n", sizeof(chain_struct));
  do_debug(DEBUG_ALL,"MY: sizeof(TABLESIZE)==%i\n", sizeof(TABLESIZE));
	do_debug(DEBUG_ALL,"sizeof(TableType)==%i\n",sizeof(TableType));

	HT=(TableType *) malloc(sizeof(TableType));
	check_assign(HT,"hashing.hashing_create");
	
	/* Initialise its contents */
	for(k=0;k<TABLESIZE;k++) (*HT)[k]=NULL;

	return(HT);
}

void hashing_terminate( HashTable *HT) {
/* hashing_terminate
 * Dispose of the chains associated with a hashing table,
 * and the hashing table itself 
 */

	unsigned int k;
	chain ochn,cchnn;

	/* Move through the hashing table */
	levindicator(FALSE);
	for(k=0;k<TABLESIZE;k++) {

		do_debug(DEBUG_MEM,"k==%i\t",k);

		if (*HT) {
			cchnn=(*(*HT))[k];

			/* Whilst valid chain node */	
			while (cchnn!=NULL) {

				do_debug(DEBUG_MEM,"--- Node free: [%s]\n",cchnn->element);

				/* Mark it */
				ochn=cchnn;

				/* Go to the next node */
				cchnn=cchnn->next;

				/* Dispose of the previous node */
				do_debug(DEBUG_MEM,"--- node [%p]\n",ochn);
				if (ochn!=NULL) {
					free(ochn);
					counter_cnode--;
					ochn=NULL;
				}

			}
			(*(*HT))[k]=NULL;
		}
	}
	levindicator(TRUE);
	/* Dispose of the hashing table itself */
	free(*HT);

	*HT=NULL;

	do_debug(DEBUG_MEM,"*** Node balance: %d\n",counter_cnode);
}

void hashing_save( HashTable HT,
		   char *filen) {
/* hashing_save
 * Saves the hashing table to the filename specified
 */
	FILE *fptr;
	char temp[100];
	unsigned int counter;	
	node_str node;
	chain cur_chain_node;

	do_debug(DEBUG_ENTER,"ENTERing hashing_save\n");

	/* Open the file for writing */
	fptr=fopen( filen, "w" );

	if (fptr==NULL) {
		sprintf(temp,"Saving Hash Table File:  %s",filen);
		raise_error(ERROR_FILE_OPENING,NONFATAL,temp);
	} else {
		for (counter=0;counter<TABLESIZE;counter++) {

			/* Get the head chain node */
			node.hash_key=counter;

			cur_chain_node=(*HT)[counter];
			node.pos=0;

			levindicator(FALSE);
			do_debug(DEBUG_MEM,"[%d]",counter);
			while (cur_chain_node!=NULL) {
				/* Get the current node */
				strcpy(node.item,cur_chain_node->element);

				/* Increment the position */
				node.pos++;

				/* Write the node to the file */
				fprintf(fptr,"h=%c\ne=%s\np=%u\n",node.hash_key,node.item,node.pos);
				do_debug(DEBUG_MEM,"*",counter);

				cur_chain_node=cur_chain_node->next;
			}
			do_debug(DEBUG_MEM,"\t");
		}		
		do_debug(DEBUG_MEM,"\n");
		levindicator(TRUE);

		fclose(fptr);
	}

	do_debug(DEBUG_ACK,"*** SAVED hash table to [%s]\n",filen);

	do_debug(DEBUG_ENTER,"EXITing hashing_save\n");
}

void hashing_load( HashTable *HT,
		   char *filen) {
/* hashing_load
 * Create a table, and load the hash file specified. 
 */
	FILE *fptr;
	node_str node;
	int readerr;
	char *cptr,line[MAXIMUM_ALL_DATA];

	do_debug(DEBUG_ENTER,"ENTERing: hashing_load\n");

	do_debug(DEBUG_ACK,"*** Starting LOAD of hash table from [%s]\n",filen);

	(*HT)=hashing_create();

	fptr=fopen(filen,"r");

	if (fptr!=NULL) {
		do {
			readerr=fscanf(fptr,"h=%c\n",&node.hash_key);
			if (readerr!=EOF) {
				cptr=fgets(line,sizeof(line),fptr);
				if (cptr!=NULL) {	
					cptr[strlen(line)-1]='\0';
					cptr=strchr(line,'=');
					if (cptr!=NULL) {
						cptr++;
						strcpy(node.item,cptr);
						do_debug(DEBUG_MEM,"Read Hashing Node: [%s]",cptr);
					}
					readerr=fscanf(fptr,"p=%u\n",&node.pos);	
				}
			}
			if ( (readerr!=EOF) && (cptr!=NULL) ) {
				hashing_insert(*HT,node.item,node.hash_key);
			}
		} while  ( (readerr!=EOF) && (cptr!=NULL) );

		/* Close the hashing file handle */
		fclose(fptr);
	} else {
		do_debug(DEBUG_ENTER,"End (ERROR): hashing_load\n");
		raise_error(ERROR_FILE_OPENING,NONFATAL,filen);	
	}
	
	do_debug(DEBUG_ACK,"*** LOADED hash table from [%s]\n",filen);
	do_debug(DEBUG_MEM,"*** Node balance: %d\n",counter_cnode);

	do_debug(DEBUG_ENTER,"End (Ok): hashing_load\n");

}


HashTable build_hash_table( relation rel ) {
/* build_hash_table
 * Build a hash table from a relation, and return a ptr
 */

	HashTable htable;
	tuple ct;
	char tuple_string[MAXIMUM_ALL_DATA];
	word count;

	/* Create the hash table */
	htable=hashing_create();

	/* Read the first tuple from the relation */
	ct=tuple_readfirst(rel,TUPLE_BUILD,NULL);

	count=0;
	do_debug(DEBUG_INFO,"\nCount:\n------\n");

	/* Whilst the tuple is valid */
	while ( ct!=NULL ) {
		/* Convert the tuple to a string */
		tuple_to_string(ct,tuple_string);

		/* Insert the string into the tuple */
		hashing_insert(htable,tuple_string,REQ_CALC);

		count++;

		/* Load the next tuple */
		(void) tuple_readnext(&ct,TUPLE_BUILD);
	}

	/*  Mark the relation as updated, to force the
	 *  Hash table to be written on shut down 
	 */
	rel->updated=TRUE;	
	
	close_tuple(&ct,TUPLE_DISPOSE);
	
	do_debug(DEBUG_ACK,"*** BUILT hash table on rel [%s]\n",relation_name(rel));

	do_debug(DEBUG_MEM,"*** Node balance: %d\n",counter_cnode);
	return(htable);
}
