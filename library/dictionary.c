/***************************************************************************
*
*   File    : core.h
*   Purpose : Implements the core filtering system for WB4Spam
*
*
*   Author: David Ruano
*
*
*   Date    : April  13, 2010
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "dictionary.h"
#include "hashmap.h"
#include "logger.h"

int add_parameter_value(p_value **p_values,char *value);
int print_list_values(element nullpointer, element item);

ini_file *create_ini_file(){
    ini_file *file;
    if((file=malloc(sizeof(ini_file)))==NULL){
        printlog(LOG_CRITICAL,"[READ_PROPERTIES]","Cannot allocate enought memmory\n");
        return NULL;
    }else{
        file->num_sections=0;
        file->sections=hashmap_new();
        return file;
    }
}


int free_values(any_t item, any_t key){
    free((char *)key);
    return MAP_OK;
}

int free_list_values(element key){
    return NODE_OK;
}

int free_attributes(any_t item, any_t data, any_t key){
    p_value *aux=(p_value *)data;
    free((char *)key);
    if(aux->value != NULL) 
        free(aux->value);
    
    if(aux->values != NULL){
        hashmap_iterate_keys(aux->values,&free_values,NULL);
        hashmap_free(aux->values);
    }
    
    free(aux);
    return MAP_OK;
}

int free_section(any_t item,any_t data, any_t key){
    hashmap_iterate_elements((map_t)data,&free_attributes,NULL);
    hashmap_free((map_t)data);
    free((char *)key);
    return MAP_OK;
}

void free_ini_file(ini_file *file){
    hashmap_iterate_elements(file->sections,&free_section,NULL);
    hashmap_free(file->sections);
    file->num_sections=0;
    free(file);
}

int print_values(any_t item, any_t key){
    printf("      >>VALUE: %s",(char *)key);
    return MAP_OK;
}

int print_list_values(element nullpointer, element item){
    printf("      >>LIST_VALUE: %s",(char *)item);
    return NODE_OK;
}

int print_attributes(any_t item, any_t data, any_t key){
    p_value *aux=(p_value *)data;

    printf("    ++NAME: %s\n",(char *)key);
    if(aux->value==NULL && aux->values==NULL){
        printf("      >>VALUE: NULL");
        return MAP_OK;
    }
    if(aux->value != NULL){
        printf("      >>VALUE: %s\n",(char *)aux->value);
        return MAP_OK;
    }
    
    if(aux->values!=NULL){
        hashmap_iterate_keys(aux->values,&print_values,NULL);
        printf("\n");
        return MAP_OK;
    }
        
    return MAP_OK;
}

int print_section(any_t item,any_t data, any_t key){
    printf("SECTION\n");
    printf("  --NAME: %s\n",(char *)key);
    printf("  --ATTRIBUTES: \n");
    hashmap_iterate_elements((map_t)data,&print_attributes,NULL);
    return MAP_OK;
}


void print_ini_file(ini_file *file){
    hashmap_iterate_elements(file->sections,&print_section,NULL);
}

int add_section(ini_file *file, char *section_key){
    map_t result;
    if(hashmap_get(file->sections,section_key,(any_t *)&result)==MAP_MISSING){
        result=hashmap_new();
        hashmap_put(file->sections,section_key,(any_t)result);
        file->num_sections++;
        return INI_OK;
    }
    return SECTION_EXIST;
}


int add_attribute(ini_file *file,char *section_name, char *values){
    map_t attributes;
    p_value *attr_values=NULL;
    
    if((hashmap_get(file->sections,section_name,(any_t *)&attributes)==MAP_MISSING)){
        printlog(LOG_WARNING,"[READ_PROPERTIES]","Section name does not exist\n");
        return SECTION_MISSING;
    }
    else{
        char *temp=strtok(values," \\\n");
        if(hashmap_get(attributes,temp,(any_t *)&attr_values) != MAP_MISSING){
            printlog(LOG_WARNING,"[READ_PROPERTIES]","Attribute already exist\n");
            return ATTRIBUTE_EXIST;
        }
        else{
            char *attr_name=malloc(sizeof(char)*(strlen(temp)+1));
            strcpy(attr_name,temp);
            while ((temp=strtok(NULL," \\\n"))!=NULL){
                char *attr_value=malloc(sizeof(char)*(strlen(temp)+1));
                strcpy(attr_value,temp);
                add_parameter_value(&attr_values,attr_value);
            }
            hashmap_put(attributes,attr_name,(any_t)attr_values);
            return INI_OK;
        }
    }
}

int add_parameter_value(p_value **p_values,char *value){
    
    if(*p_values==NULL){
        if((*p_values=malloc(sizeof(p_value)))==NULL){
            printlog(LOG_CRITICAL,"[READ_PROPERTIES]","Error allocating memory\n");
            return MEMORY_ERROR;
        }
        (*p_values)->numvalues=1;
        (*p_values)->value=value;
        (*p_values)->values=NULL;
        //printf("HAY 1 SOLO VALOR: %s\n",(*p_values)->value);
    }else{
        if((*p_values)->numvalues==1){
            //printf("HAY MAS DE UN VALOR %s\n",value);
            if( ( (*p_values)->values=hashmap_new()) == NULL ){
                printlog(LOG_CRITICAL,"[READ_PROPERTIES]","Error allocating memory\n");
                return MEMORY_ERROR;
            }
            hashmap_put((*p_values)->values,(*p_values)->value,NULL);
            (*p_values)->value=NULL;
        }
        any_t param;
        if( hashmap_get((*p_values)->values,value,(any_t *)&param) == MAP_MISSING){
            (*p_values)->numvalues++;
            hashmap_put((*p_values)->values,value,NULL);
        }
    }
    return INI_OK;
}

