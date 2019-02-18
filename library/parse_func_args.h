/***************************************************************************
*
*   File    : parse_func_args.h
*   Purpose : This is a parser for function parameters on filters
*
*   Author:   Jose Ramon Mendez, David Ruano
*
*
*   Date    : October  25, 2010
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

#ifndef __PARSE_FUNC_ARGS_H__
#define __PARSE_FUNC_ARGS_H__

#define TYPE_STRING 0
#define TYPE_INT 1
#define TYPE_FLOAT 2
#define TYPE_BOOLEAN 3

struct function_arguments;
typedef struct function_arguments function_arguments;

/**
 * Compute the function arguments from a string
 */
function_arguments *parse_args(char *arguments, int expected);

/*Get the number of params*/
int count_num_params(char *arguments);
/**
 * Get the type of an argument
 */
int get_argument_type(function_arguments *arguments, int element);

/**
 * Get the contents of an argument
 */
char *get_argument_content(function_arguments *arguments, int element);

char *get_lowercase_argument_content(function_arguments *arguments, int i);

/**
 * Count the number of arguments
 */
int count_arguments(function_arguments *arguments);

/**
 * Free arguments
 */
void free_arguments(function_arguments *arguments);

#endif
