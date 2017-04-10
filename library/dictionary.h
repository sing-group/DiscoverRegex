/***************************************************************************
*
*   File    : core.h
*   Purpose : Implements the core filtering system for WB4Spam
*
*
*   Author: David Ruano
*
*
*   Date  : April  13, 2011
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

#ifndef _DICTIONARY_H_
#define _DICTIONARY_H_

/*---------------------------------------------------------------------------
   								Includes
 ---------------------------------------------------------------------------*/

#include "linked_list.h"
#include "hashmap.h"

/*---------------------------------------------------------------------------
                                                                  MACROS
 ---------------------------------------------------------------------------*/
#define INI_OK 0
#define SECTION_EXIST -1
#define SECTION_MISSING -2
#define ATTRIBUTE_EXIST -3
#define MEMORY_ERROR -4
#define ATTRIBUTE_MISSING -5
#define INI_EMPTY -6

/*---------------------------------------------------------------------------
                                                                  DATA TYPES
 ---------------------------------------------------------------------------*/
struct ini_file {
    map_t sections;
    int num_sections;
};

typedef struct ini_file ini_file;

struct parameter_value{
    int numvalues;
    char *value;
    map_t values;
};

typedef struct parameter_value p_value;

/*---------------------------------------------------------------------------
                                                                    FUNCTIONS
 ---------------------------------------------------------------------------*/

//Creates a ini_file to allocate the parsed configuration file.
ini_file *create_ini_file();

//Liberates all the structures created by parsing the configuration file
void free_ini_file(ini_file *file);

//Add a new section on the ini_file datatype.
int add_section(ini_file *file, char *section_key);

//Add a new attribute on the ini_file datatype.
int add_attribute(ini_file *file,char *section_name, char *values);

//Print all the information allocated on the ini_file datatype.
void print_ini_file(ini_file *file); //DEPURATION

#endif



