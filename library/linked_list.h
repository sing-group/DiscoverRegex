/***************************************************************************
*
*   File    : linked_list.h
*   Purpose : Implements a linked_list for storag
*
*
*   Original Author: David Ruano Ordás, Noemi Pérez Diaz
*
*   Memory improvements, modifications, inclusion of new fields
*       and functions: David Ruano, Noemi Perez, Jose Ramon Mendez
*
*   Date    : October  14, 2010
*
*****************************************************************************
*   LICENSING
*****************************************************************************
*
* WB4Spam: An ANSI C is an open source, highly extensible, high performance and
* multithread spam filtering platform. It takes concepts from SpamAssassin project
* improving distinct issues.
*
* Copyright (C) 2010, by Sing Research Group (http://sing.ei.uvigo.es)
*
* This file is part of WireBrush for Spam project.
*
* Wirebrush for Spam is free software; you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public License as
* published by the Free Software Foundation; either version 3 of the
* License, or (at your option) any later version.
*
* Wirebrush for Spam is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser
* General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#ifndef LINKED_LIST_H
#define	LINKED_LIST_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "common_dinamic_structures.h"

#define NODE_MISSING -3
#define LIST_EMPTY -1
#define NODE_OK 0
#define NODE_FAIL 1


//struct node;
struct linkedlist;

typedef struct linkedlist linklist;

linklist *newlinkedlist();

/**
 * Adds a new node at the beginning of the linked-list.
 * @param list the linked-list to insert at.
 * @param token the data to be store in the linked-list
 */
int addbeginlist(linklist *list, element token);

/**
 * Adds a new node at the end of the list.
 * @param list the linked-list to insert at.
 * @param token the data to be store in the linked-list
 */
void addendlist(linklist *list, element token);

/**
 * Adds the node to the linkedlist depending of the value of f.
 * @param list the linked-list to insert at.
 * @param token the data to be store in the linked-list (must be a sorted type)
 */
void addorder(linklist *list, element token,PFunction f);


/**
 * Adds the node to the linkedlist at a designed position.
 * @param list the linked-list to insert at.
 * @param token the data to be store in the linked-list
 * @param position the place to store the token
 */
void addAtPos(linklist *list, element token,int position);

/**
 * Gets the lenght of the linked-list
 * @param list the linked-list which wants to know the lenght
 * @return the length of the linked-list
 */
int getlengthlist(linklist *list);

/*
 * Iteratively call f with argument (item, data) for
 * each element data in the list. The function must
 * return a list status code. If it returns anything other
 * than NODE_OK the traversal is terminated. f must
 * not reenter any linklist functions, or deadlock may arise.
 */
extern int linklist_iterate_data(linklist *list, PFunction f, element item);

/*
 * Iteratively call f with argument (item2, item, data) for
 * each element data in the list. The function must
 * return a list status code. If it returns anything other
 * than NODE_OK the traversal is terminated. f must
 * not reenter any linklist functions, or deadlock may arise.
 */
extern int linklist_iterate_items(linklist *list, PFunction_3 f, element item, element item2);

/*
 * Iteratively call f with argument (item3, item2, item, data) for
 * each element data in the list. The function must
 * return a list status code. If it returns anything other
 * than NODE_OK the traversal is terminated. f must
 * not reenter any linklist functions, or deadlock may arise.
 */
extern int linklist_iterate_three(linklist *list, PFunction_4 f, element item, element item2, element item3);

/**
 * Returns the content of the first element of the list.
 * @param list the linked-list to insert at.
 * @return returns the result obtained by applying the function
 */
int getfirst(linklist *list, element *elem);

/**
 * Returns the content of the tail node.
 * @param list the linked-list to insert at.
 * @return the result obtained by applying the function
 *
 */
int getlast(linklist *list, element *elem);

/**
 * Get element content at a determined position
 * @param list the linked-list to insert at.
 * @param the position of the token
 * @return the result obtained by applying the function
 */
int getatlist(linklist *list,int position, element *elem);

/**
 * Gets and removes the element at the first position.
 * @param list the linked-list where deletes the node.
 * @param elem the element at the first position
 * @return the result obtained by applying the function
 */
int removefirst(linklist *list, element *elem);

/**
 * Gets and removes the element at the last position.
 * @param list the linked-list where deletes the node.
 * @param elem the element at the last position
 * @return the result obtained by applying the function
 */
int removelast(linklist *list, element *elem);

/**
 * Deletes all the linked-list.
 * @param list the linked-list to be deleted.
 */
void freelist(linklist *list, PFree f);


/**
 * Obtains next element in List.
 * @param list the linked-list 
 * @param elem the element to be obtained
 */
int linklistHasNext(linklist *list,element *elem);


/**
 * Returns a sublist defined by interval initPos-endPos
 * @param list the linked-list 
 * @param initPos the start position to begin the slice.
 * @param endPos the final position to stop the slice.
 * Caution: Do not remove the content of the list.
 */
linklist *splice_linklist(linklist *list, int initPos, int endPos);


/**
 * Returns a Generic Vector slice from a linked-list
 * @param list the linked-list 
 * @param initPos the start position to begin the slice.
 * @param among the number of elements to be passed
 * Caution: Do not remove the content of the list.
 */
Gvector * linklist_to_GVector(linklist *list, int init, int among);


/**
 * Returns an integer referencin the position inside the list when using iterators
 * @param list the linked-list 
 */
//int getIteratorPos(linklist *list);

int linklistHasCondition(linklist *list, PFunction f, element item);


#endif	/* LINKED_LIST_H */

