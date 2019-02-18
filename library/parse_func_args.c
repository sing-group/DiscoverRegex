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


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "parse_func_args.h"
#include "logger.h"
#include "string_util.h"

#define INIT 0
#define READING_STRING 1
#define READING_ENDING_STRING 2
#define READING_NUMBER 3
#define READING_ENDING_NUMBER 4
#define READING_FLOAT 5
#define READING_ENDING_FLOAT 6
#define READING_BOOLEAN 7
#define READING_ENDING_BOOLEAN 8
#define END 9

struct function_arguments{
	char **argument;
	int *type;
	int count;
};

function_arguments *parse_args(char *arguments, int expected){
	function_arguments *retvalue;
	int status, count, argno, spacesafter;
	char *start_element=arguments;
	retvalue=malloc(sizeof(function_arguments));

	retvalue->count=0;
	retvalue->type=malloc(sizeof(int)*expected);
	retvalue->argument=malloc(sizeof(char *)*expected);
	status=INIT;
	count=0;
	argno=0;
	spacesafter=0;

	while (status!=END){
	    if (status == INIT && start_element[count]=='\0'){
	    	status=END;
	    	break;
		}else if (status == INIT && start_element[count]==' '){
	    	 start_element=&start_element[count+1];
		}else if (status == INIT && start_element[count]=='"'){
			start_element=&start_element[count+1];
			count=0;
			status=READING_STRING;
		}else if (status == INIT && (start_element[count]>='0' && start_element[count]<='9')){
			count++;
			status=READING_NUMBER;
		}else if (status == INIT && (start_element[count]=='T' && start_element[count]<='F')){
			count++;
		    status=READING_BOOLEAN;
		}else if (status==READING_STRING && (start_element[count]!='"' || (count>0 && start_element[count]!='"' && start_element[count-1]!='\\'))){
			count++;
		}else if (status==READING_STRING && start_element[count]=='"'){
			//end reading string
			spacesafter=1;
			status=READING_ENDING_STRING;
		}else if ((status==READING_ENDING_STRING || status==READING_ENDING_NUMBER || status==READING_ENDING_FLOAT || status==READING_ENDING_BOOLEAN)
		           && (start_element[count+spacesafter]==' ' || start_element[count+spacesafter]=='\t')){
		    //spaces after the param
			spacesafter++;
		}else if (status==READING_ENDING_STRING && (start_element[count+spacesafter]==',' || start_element[count+spacesafter]=='\0')){
		    //process the string from start_element with count elements
		    if(argno>=expected){
		    	wblprintf(LOG_WARNING,"parse_func_args","Aborting argument parsing because found more arguments than expected: %s\n", arguments);
		    	break;
			}
		    retvalue->type[argno]=TYPE_STRING;
		    retvalue->argument[argno]=malloc(sizeof(char)*(count+1));
		    memcpy(retvalue->argument[argno],start_element,count*sizeof(char));
		    retvalue->argument[argno][count]='\0';
		    retvalue->count++;
		    //printf("identify string count: %d, argno %d, start_element: %s \n", count, argno, start_element);
			argno++;

			if(start_element[count+spacesafter]=='\0'){
				status=END;
			}else{
			    start_element=&start_element[count+spacesafter+1];
			    count=0;
			    spacesafter=0;
			    status=INIT;
		    }
		}else if ((status==READING_NUMBER || status==READING_FLOAT) && (start_element[count]>='0' && start_element[count]<='9')){
			count++;
		}else if (status==READING_NUMBER && (start_element[count]==' ')){
			count--;
			status=READING_ENDING_NUMBER;
			spacesafter=2;
		}else if (status==READING_FLOAT && (start_element[count]==' ')){
			count--;
			status=READING_ENDING_FLOAT;
			spacesafter=2;
		}else if (status==READING_BOOLEAN && (start_element[count]==' ')){
			count--;
			status=READING_ENDING_BOOLEAN;
			spacesafter=2;
		}else if (status==READING_NUMBER && start_element[count]>='.'){
			status=READING_FLOAT;
			count++;
		}else if ((status==READING_NUMBER || status==READING_ENDING_NUMBER) && (start_element[count]==',' || start_element[count]=='\0')){
			//end reading int. use content and count for parsing
		    if(argno>=expected){
		    	wblprintf(LOG_WARNING,"parse_func_args","Aborting argument parsing because found more arguments than expected: %s\n", arguments);
		    	break;
			}
		    retvalue->type[argno]=TYPE_INT;
		    retvalue->argument[argno]=malloc(sizeof(char)*(count+1));
		    memcpy(retvalue->argument[argno],start_element,count);
		    retvalue->argument[argno][count]='\0';
		    retvalue->count++;

			argno++;
			if(start_element[count]=='\0'){
				status=END;
			}else{
			   start_element=&start_element[count+1];
			   count=0;
			   spacesafter=0;
			   status=INIT;
		    }
		}else if ((status==READING_FLOAT || status==READING_ENDING_FLOAT) && (start_element[count]==','||start_element[count]=='\0')){
			//end reading float. use content and count for parsing
		    if(argno>=expected){
		    	wblprintf(LOG_WARNING,"parse_func_args","Aborting argument parsing because found more arguments than expected: %s\n", arguments);
		    	break;
			}
		    retvalue->type[argno]=TYPE_FLOAT;
		    retvalue->argument[argno]=malloc(sizeof(char)*(count+1));
		    memcpy(retvalue->argument[argno],start_element,count);
		    retvalue->argument[argno][count]='\0';
		    retvalue->count++;

			argno++;
			if(start_element[count]=='\0'){
				status=END;
			}else{
			   start_element=&start_element[count+1];
			   count=0;
			   spacesafter=0;
			   status=INIT;
		    }
		}else if (status==READING_BOOLEAN && start_element[count]!=','){
			count++;
		}else if ((status==READING_BOOLEAN || status==READING_ENDING_BOOLEAN) && (start_element[count]==',' || start_element[count]=='\0')){
			//end reading boolean. use content and count for parsing
		    if(argno>=expected){
		    	wblprintf(LOG_WARNING,"parse_func_args","Aborting argument parsing because found more arguments than expected: %s\n", arguments);
		    	break;
			}
		    retvalue->type[argno]=TYPE_BOOLEAN;
		    retvalue->argument[argno]=malloc(sizeof(char)*(count+1));
		    memcpy(retvalue->argument[argno],start_element,count);
		    retvalue->argument[argno][count]='\0';
		    retvalue->count++;

			argno++;
			if(start_element[count]=='\0'){
				status=END;
			}else{
			    start_element=&start_element[count+1];
			    count=0;
			    spacesafter=0;
			    status=INIT;
		    }
		}else if (status==END){
			break;
		}
	}

	//printf("Llego el fin del parseo de parámetros.\n");

	return retvalue;

}

int count_num_params(char *arguments){
    int count=0;
    int count_params=0;
    int parent=0;
    char *start_pointer=arguments;

    while(start_pointer[count]!='\0'){
        if(start_pointer[count]=='\"'){
            if(parent==0)
                parent++;
            else
                parent--;
        }
        if(start_pointer[count]==',' && parent==0)
            count_params++;
        count++;
    }
    return (++count_params);
}

int get_argument_type(function_arguments *arguments, int i){
    return arguments->type[i];
}

char *get_lowercase_argument_content(function_arguments *arguments, int i){
    char *param=arguments->argument[i];
    param=to_lower_case(param);
    return param;
}


char *get_argument_content(function_arguments *arguments, int i){
    return arguments->argument[i];
}

int count_arguments(function_arguments *arguments){
    return arguments->count;
}


void free_arguments(function_arguments *arguments){
    int i;
    
    if (arguments != NULL){
        for (i=0;i<arguments->count;i++){
             if (arguments->argument[i] != NULL) free(arguments->argument[i]);
        }
        if(arguments->argument!=NULL) free(arguments->argument);
        if(arguments->type!=NULL) free(arguments->type);
        free(arguments);
    }
}
