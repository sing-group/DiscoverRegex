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

#ifndef _DLINKED_LIST_H
#define	_DLINKED_LIST_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "common_dinamic_structures.h"


//struct node;
struct dlinkedlist;

typedef struct dlinkedlist dlinklist;

dlinklist *newDlinkedlist();

/**
 * Adds a new node at the beginning of the linked-list.
 * @param list the linked-list to insert at.
 * @param token the data to be store in the linked-list
 */
int addbeginDlist(dlinklist *list, element token);

/**
 * Adds a new node at the end of the list.
 * @param list the linked-list to insert at.
 * @param token the data to be store in the linked-list
 */
void addendDlist(dlinklist *list, element token);

/**
 * Adds the node to the linkedlist depending of the value of f.
 * @param list the linked-list to insert at.
 * @param token the data to be store in the linked-list (must be a sorted type)
 */
void addorderDlist(dlinklist *list, element token,PFunction f);

/**
 * Gets the lenght of the linked-list
 * @param list the linked-list which wants to know the lenght
 * @return the length of the linked-list
 */
int getlengthDlist(dlinklist *list);

/**
 * Gets and removes the element at the first position.
 * @param list the linked-list where deletes the node.
 * @param elem the element at the first position
 * @return the result obtained by applying the function
 */
int removefirstDlist(dlinklist *list, element *elem);

/**
 * Gets and removes the element at the last position.
 * @param list the linked-list where deletes the node.
 * @param elem the element at the last position
 * @return the result obtained by applying the function
 */
int removelastDlist(dlinklist *list, element *elem);

/**
 * Get element content at a determined position
 * @param list the linked-list to insert at.
 * @param the position of the token
 * @return the result obtained by applying the function
 */
int getatDlist(dlinklist *list,int position, element *elem);

/**
 * Deletes all the linked-list.
 * @param list the linked-list to be deleted.
 */
void freeDlist(dlinklist *list, PFree f);

/**
 * Deletes @amount of elements from end of list.
 * @param list the linked-list to be deleted.
 * @PFree callback to erasing funtion.
 * @amount number of elements to be erased
 */
int dlinklist_reverse_remove(dlinklist *list, long int amount, PFree g);

/*
 * Iteratively call f with argument (item, data) for
 * each element data in the list. The function must
 * return a list status code. If it returns anything other
 * than NODE_OK the traversal is terminated. f must
 * not reenter any linklist functions, or deadlock may arise.
 */
int dlinklist_iterate_data(dlinklist *list, PFunction f, element item);

/*
 * Iteratively call f with argument (item2, item, data) for
 * each element data in the list. The function must
 * return a list status code. If it returns anything other
 * than NODE_OK the traversal is terminated. f must
 * not reenter any linklist functions, or deadlock may arise.
 */
int dlinklist_iterate_items(dlinklist *list, PFunction_3 f, element item, element item2);

/*
 * Iteratively call f with argument (item3, item2, item, data) for
 * each element data in the list. The function must
 * return a list status code. If it returns anything other
 * than NODE_OK the traversal is terminated. f must
 * not reenter any linklist functions, or deadlock may arise.
 */
int dlinklist_iterate_three(dlinklist *list, PFunction_4 f, element item, element item2, element item3);

/**
 * Returns the content of the first element of the list.
 * @param list the linked-list to insert at.
 * @return returns the result obtained by applying the function
 */
int getfirstDlist(dlinklist *list, element *elem);

/**
 * Returns the content of the tail node.
 * @param list the linked-list to insert at.
 * @return the result obtained by applying the function
 *
 */
int getlastDlist(dlinklist *list, element *elem);


/**
 * Returns a Generic Vector slice from a linked-list
 * @param list the linked-list 
 * @param initPos the start position to begin the slice.
 * @param among the number of elements to be passed
 * Caution: Do not remove the content of the list.
 */
void printDlist(dlinklist *list);

void setDlistLenght(dlinklist *list, int value);

#endif	/* LINKED_LIST_H */

