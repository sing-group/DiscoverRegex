
#ifndef _INIPARSER_H_
#define _INIPARSER_H_

/*---------------------------------------------------------------------------
   								Includes
 ---------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "dictionary.h"

#define CONFIG_OK -1
#define CONFIG_FAIL -2
#define CONFIG_EMPTY -3
#define ELEMENT_MISSING -4


int parse_ini(char *text, ini_file **config_values);
void free_ini(ini_file *config_values);

int get_section_values(ini_file *file, char *section_name, map_t *attributes);

int get_num_values_ini(ini_file *file, char *section_name, map_t *attributes);

int get_list_attribute_values_ini(ini_file *file,char *section_name, char *attr_name, void **values);

int get_map_attribute_values_ini(ini_file *file,char *section_name, char *attr_name, void **values);

int get_attribute_values(map_t attributes,char *attr_name ,p_value **values);

int has_attribute_value(map_t attributes,char *attr_name,char * attr_value);

int has_attribute_value_ini(ini_file *file,char *section_name,char *attr_name, char *attr_value);

void print_inifile(ini_file *file); //DEPURATION

#endif
