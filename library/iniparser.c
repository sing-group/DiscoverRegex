#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include "fileutils.h"
#include "dictionary.h"
#include "iniparser.h"
#include "logger.h"

#define BEGIN_SECTION 0
#define END_SECTION 1
#define BEGIN_ATTRIBUTE 2
#define END_ATTRIBUTE 3
#define BEGIN_COMMENT 4
#define END_COMMENT 5
#define NONE 6

#define CONFIG_OK -1
#define CONFIG_FAIL -2
#define CONFIG_EMPTY -3
#define ELEMENT_MISSING -4

int parse_ini(char *path, ini_file **config_values){
    
    char *start_pointer;
    char *text;
    char *begin=NULL;
    char *end=NULL;
    char *section_name=NULL;
    ini_file *inifile=NULL;
    int count=0;
    int status=NONE;

    if(ae_load_file_to_memory(path,&text) <=0 ) return CONFIG_FAIL;

    if((inifile=create_ini_file())==NULL) return CONFIG_FAIL;
    
    start_pointer=text;

    while(start_pointer[count]!='\0'){
        if(start_pointer[count]=='#'){
            count++;
            status=BEGIN_COMMENT;
        }
        if(status==NONE && start_pointer[count]!='[')
            count++;
        if(status==NONE && start_pointer[count]=='['){
            status=BEGIN_SECTION;
            count++;
            begin=&start_pointer[count];
        }
        if(status==BEGIN_SECTION && (start_pointer[count]!=']'))
            count++;

        if(status==BEGIN_SECTION && start_pointer[count]=='['){
            printlog(LOG_CRITICAL,"INI_PARSER","Error, section incorrect\n");
            *config_values=NULL;
            return CONFIG_FAIL;
        }
        if(status==BEGIN_SECTION && (start_pointer[count]==']') ){
            end=&start_pointer[count];
            section_name=malloc(sizeof(char)*(end-begin+1));
            memcpy(section_name,begin,(end-begin)*sizeof(char));
            section_name[end-begin]='\0';
            add_section(inifile,section_name);
            status=END_SECTION;
            count++;
        }
        if(status==END_SECTION && (start_pointer[count])!='\n')
            count++;
        if(status==END_SECTION && (start_pointer[count]=='\n')){
            count++;
            if(start_pointer[count]!='\n'){
                status=BEGIN_ATTRIBUTE;
                begin=&start_pointer[count];
                count++;
            }
        }

        if(status==BEGIN_ATTRIBUTE && (start_pointer[count]!='\n'
                                       || start_pointer[count]!=';'))
            count++;
        if(status==BEGIN_ATTRIBUTE && start_pointer[count]=='\\' 
           && start_pointer[count+1]!='\n'){
                printlog(LOG_CRITICAL,"INI_PARSER","Error, \\ must precede \n");
                *config_values=NULL;
                return CONFIG_FAIL;
        }
        if( status==BEGIN_ATTRIBUTE  && (start_pointer[count]==';'
           || (start_pointer[count]=='\n' && start_pointer[count-1]!='\\') )){
            end=&start_pointer[count];
            char *attribute=malloc(sizeof(char)*(end-begin+1));
            memcpy(attribute,begin,sizeof(char)*(end-begin));
            attribute[end-begin]='\0';
            //printf("ATTRIBUTE: %s\n",attribute);
            add_attribute(inifile,section_name,attribute);
            free(attribute);
            
            if(start_pointer[count]==';')
                status=BEGIN_COMMENT;
            else
                status=END_ATTRIBUTE;
            count++;
        }

        if( (status==END_ATTRIBUTE || status==END_COMMENT ) && (start_pointer[count]==' '
           || start_pointer[count]=='\n' ))
            count++;
        if((status==END_ATTRIBUTE || status==END_COMMENT ) && (start_pointer[count]!='#' && isalpha(start_pointer[count])) ){
            begin=&start_pointer[count];
            status=BEGIN_ATTRIBUTE;
            count++;
        }
        if((status==END_ATTRIBUTE || status==END_COMMENT ) && start_pointer[count]=='['){
            count++;
            begin=&start_pointer[count];
            status=BEGIN_SECTION;            
        }
        if(status==BEGIN_COMMENT && start_pointer[count]!='\n'){
            count++;
        }
        if(status==BEGIN_COMMENT && start_pointer[count]=='\n'){
            count++;
            if(start_pointer[count]=='\n')
                count++;
            status=END_COMMENT;
            if(start_pointer[count]=='#')
                status=BEGIN_COMMENT;
        }

        if(status==END_COMMENT && start_pointer[count]!='['
           && !isalpha(start_pointer[count]) && start_pointer[count]!='\0'){
            count++;
        }
        if(status==END_COMMENT && start_pointer[count]=='['){
            status=BEGIN_SECTION;
            count++;
            begin=&start_pointer[count];
            //printf("INICIO DE SECCION\n");
        }
        if(status==END_COMMENT && isalpha(start_pointer[count]) ){
            status=BEGIN_ATTRIBUTE;
            begin=&start_pointer[count];
            count++;
        }        
    }
    *config_values=inifile;
    if(text!=NULL) free(text);
    return CONFIG_OK;
}

void free_ini(ini_file *config_values){
    free_ini_file(config_values);
}

int get_section_values(ini_file *file, char *section_name, map_t *attributes){
    map_t section_val;
    if(file->num_sections>0){
        if(hashmap_get(file->sections,section_name,(any_t*)&section_val)==MAP_MISSING){
            *attributes=NULL;
            return ELEMENT_MISSING;
        }else{
            *attributes=section_val;
            return CONFIG_OK;
        }
    }else
        return CONFIG_EMPTY;
}
int get_attribute_values(map_t attributes,char *attr_name ,p_value **values){
    p_value *aux;
    if(hashmap_get(attributes,attr_name,(any_t *)&aux)==MAP_MISSING){
        *values=NULL;
        return ELEMENT_MISSING;
    }
    *values=aux;
    return CONFIG_OK;
}
int has_attribute_value_ini(ini_file *file,char *section_name,char *attr_name,
                            char *attr_value){
    map_t attributes;
    if(hashmap_get(file->sections,section_name,(any_t*)&attributes)==MAP_MISSING)
        return ELEMENT_MISSING;
    return has_attribute_value(attributes,attr_name,attr_value);
}

int has_attribute_value(map_t attributes,char *attr_name,char * attr_value){
    p_value *value;
    if(hashmap_get(attributes,attr_name,(any_t *)&value)==MAP_MISSING){
        return ELEMENT_MISSING;
    }
    if(value->numvalues==1){
        return (!strcmp(value->value,attr_value));
    }else{
        void *nullpointer;
        return (hashmap_get(value->values,attr_value,(any_t*)&nullpointer)!=MAP_MISSING);
    }
}

int get_map_attribute_values_ini(ini_file *file,char *section_name,
                         char *attr_name, void **values){
    map_t attributes;
    p_value *attr_val;
    if(file != NULL && file->num_sections > 0){
        if(hashmap_get(file->sections,section_name,(any_t *)&attributes)==MAP_MISSING){
            *values=NULL;
            return ELEMENT_MISSING;
        }
        else
            if(hashmap_get(attributes,attr_name,(any_t *)&attr_val)==MAP_MISSING){
                *values=NULL;          
                return ELEMENT_MISSING;
            }
            else{
                if(attr_val->numvalues==1){
                    *values=attr_val->value;
                    return attr_val->numvalues;
                }else{
                    *values=attr_val->values;
                    return attr_val->numvalues;
                }
            }
    }else return CONFIG_EMPTY;

}

void print_inifile(ini_file *file){
    print_ini_file(file);
}