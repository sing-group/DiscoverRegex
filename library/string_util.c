/***************************************************************************                        
*
*   File    : string_util.h 
*   Purpose : 
*            
*   Original Author: Ivan Paz, Jose Ramon Mendez (from Grindstone project)
* 
*   New Funcitons included by Jose Ramon Mendez
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "string_util.h"
#include "logger.h"
#include <stdarg.h>

//#define _GNU_SOURCE

/*Append a char to a string*/
char *append(char* s, char c){
    char *tmp;
    
    if(s==NULL){
        s=(char *)malloc(sizeof(char)*2);
        s[0]=c;
        s[1]='\0';
    }else{
        int len=strlen(s);

        if ((tmp= ((char *)realloc(s, (len+2)*sizeof(char)))) ==NULL){
          printlog(LOG_CRITICAL,"[STRING_UTIL.append]","Imposible to make a realloc\n");
          tmp=(char *)malloc((len+2)*sizeof(char));
          strcpy(tmp,s);
          free(s);
        }
        s=tmp;

        s[len]=c;
        s[len+1]='\0';
    }

    return s;
}

char *appendstringAt(char *dst, char *src, int position){

    int srclen = strlen(src);
    int dstlen = strlen(dst);
    char *tmp = NULL;
    
    if(position >= dstlen){
        tmp = appendstr(dst,src);
        return tmp;
    }
    
    tmp = (char *)malloc(sizeof(char)*(srclen+dstlen+1));
    int src_pointer =0;
    int tmp_pointer = 0;
    int dst_pointer = 0;
    
    while(dst_pointer < position){ 
        tmp[tmp_pointer] = dst[dst_pointer];
        dst_pointer++;
        tmp_pointer++;
    }
    
    while(src_pointer < srclen){
        tmp[tmp_pointer]=src[src_pointer];
        src_pointer++;
        tmp_pointer++;
    }
      
    while(dst_pointer < dstlen){
        tmp[tmp_pointer]=dst[dst_pointer];
        dst_pointer++;
        tmp_pointer++;
    }
    
    tmp[srclen+dstlen]='\0';
    free(dst);
    
    return tmp;
}

char *appendcharAt(char *dst, char src, int position){

    int dstlen = strlen(dst);
    char *tmp = NULL;
    
    if(position >= dstlen){
        tmp = append(dst,src);
        return tmp;
    }
    
    tmp = (char *)malloc(sizeof(char)*(dstlen+2));
    int tmp_pointer = 0;
    int dst_pointer = 0;
    
    while(dst_pointer < position){ 
        tmp[tmp_pointer] = dst[dst_pointer];
        dst_pointer++;
        tmp_pointer++;
    }
    tmp[position]=src;
    tmp_pointer = position+1;
    
    while(dst_pointer < dstlen){
        tmp[tmp_pointer]=dst[dst_pointer];
        dst_pointer++;
        tmp_pointer++;
    }
    
    tmp[dstlen+1]='\0';
    free(dst);
    
    return tmp;
}


/*Append a char to a string*/
char *appendchar(char* s, char c){
    
    char *tmp;

    if(s==NULL){
        if( asprintf(&tmp,"%c",c) <=0  ){
            printlog(LOG_CRITICAL,"[STRING_UTIL.appendchar]","Error appending char to string\n");
            return s;
        }
    }else{
        if( asprintf(&tmp,"%s%c",s,c)<=0 ){
            printlog(LOG_CRITICAL,"[STRING_UTIL.appendchar]","Error appending char to string\n");
            return s;
        }
        free(s);  
    }
    
    return tmp;
}

char *appendint(char *s, int c){
    char *tmp;

    if(s==NULL){
        if( asprintf(&tmp,"%d",c) <=0) {
            printlog(LOG_CRITICAL,"[STRING_UTIL.appendint]","Imposible to append a integer\n");
            return s;
        }
    }else{
        if(asprintf(&tmp,"%s%d",s,c) < 0){
            printlog(LOG_CRITICAL,"[STRING_UTIL.appendint]","Imposible to append a integer\n");
        }
        free(s);
    }
    return tmp;
}

/* Append a string to another one */
char *appendstr(char* s, char *c){
    
    char *tmp = NULL;
    
    if(s==NULL){
        if( asprintf(&tmp,"%s",c) < 0) {
            printlog(LOG_CRITICAL,"[STRING_UTIL.appendstr]","Imposible to append a string\n");
            return s;
        }
    }else{
    
        if( asprintf(&tmp,"%s%s",s,c) < 0)  {
            printlog(LOG_CRITICAL,"[STRING_UTIL.appendstr]","Imposible to append a string\n");
            return s;
        }
        free(s);
    }
    
    return tmp;
}

char *concat(char* s, char *fmt, ...){
    va_list args;
    va_start(args,fmt);
    char c, *string;
    int d;
    
    while(*fmt){
        if(*fmt=='%'){
            switch(*++fmt){
                case 's':   string = va_arg(args,char *);
                            s=appendstr(s,string);
                            break;
                case 'd':   d = va_arg(args,int);
                            s=appendint(s,d);
                            break;
                case 'c':   c = (char) va_arg(args,int);
                            s=appendchar(s,c);
                            break;
            }
        }else s=appendchar(s,*fmt);
        fmt++;
    }
    
    va_end(args);
    
    return s;
}

char *createString(char *fmt, ...){
    va_list args;
    va_start(args,fmt);
    char c, *string;
    int d;
    char *s = NULL;
    
    while(*fmt){
        if(*fmt=='%'){
            switch(*++fmt){
                case 's':   string = va_arg(args,char *);
                            s=appendstr(s,string);
                            break;
                case 'd':   d = va_arg(args,int);
                            s=appendint(s,d);
                            break;
                case 'c':   c = (char) va_arg(args,int);
                            s=appendchar(s,c);
                            break;
            }
        }else s=appendchar(s,*fmt);
        fmt++;
    }
    
    va_end(args);
    
    return s;
}

/* Remove spaces from begining and end of an string */

char *trim(char *str){
    size_t len = 0;
    char *frontp = str;
    char *endp = NULL;

    if( str == NULL ) { return NULL; }
    if( str[0] == '\0' ) { return str; }

    len = strlen(str);
    endp = str + len;

    /* Move the front and back pointers to address the first non-whitespace
     * characters from each end.
     */
    while( isspace(*frontp) ) { ++frontp; }
    if( endp != frontp )
    {
        while( isspace(*(--endp)) && endp != frontp ) {}
    }

    if( str + len - 1 != endp )
            *(endp + 1) = '\0';
    else if( frontp != str &&  endp == frontp )
            *str = '\0';

    /* Shift the string so that it starts at str so that if it's dynamically
     * allocated, we can still free it on the returned pointer.  Note the reuse
     * of endp to mean the front of the string buffer now.
     */
    endp = str;
    if( frontp != str )
    {
        while( *frontp ) { *endp++ = *frontp++; }
        *endp = '\0';
    }


    return str;
}

char lower(char input){
    if (input >= 'A' && input <= 'Z') return  input=input - 'A' + 'a';
    else return input;
}

char *to_lower_case(char *string){
    int i=0;
    
    if(string == NULL)
        return NULL;
    
    for(;i<strlen(string);i++){
        string[i]=tolower(string[i]);
    }
    return string;
}

/* Remove all spaces from a string */
char *remove_spaces_and_lower(char *b){
    char *write_pointer;
    char *read_pointer;

    read_pointer=b;
    write_pointer=b;

    while (read_pointer[0]!='\0'){
        if (read_pointer[0]==' ' || read_pointer[0]=='\t' || 
            read_pointer[0]=='\n' || read_pointer[0]=='\r'
           ){
                read_pointer++;
        }else{
           write_pointer[0]=lower(read_pointer[0]);
           write_pointer++;
           read_pointer++;
        }
    }
    write_pointer[0]='\0';

    return b;
}

char *removeRecursiveBlanks(char *string){
    int len = (strlen(string)-1); /* Find the final null */
    int final= len;
    char *toret = NULL;
    int isBlank = 0;    
    int i=0;
    
    for(;i<=final;i++){
        if(isblank(string[i])){
            if(isBlank == 0){
                isBlank = 1;
                toret = appendchar(toret,string[i]);
            }
        }else{
            toret = appendchar(toret,string[i]);
            isBlank = 0;
        }
    }
    free(string);
   
    return toret;
}


/*Evaluate if a line is commented*/
int is_commented(char *line){
   char *copia;
   int ret;
   char *firstTok;

   copia=(char *)malloc((strlen(line)+1)*sizeof(char));   
   strcpy(copia,line);
   firstTok=strtok(copia," \t");
   ret=((strcmp("//",firstTok)==0) || strchr(copia,'#')!=NULL);
   free(copia);

   return ret;
}

int string_to_float(char *s, float *dest)
{
    char *endp;
    
    if(s == NULL) return 0;
    
    *dest = strtod(s, &endp);
    
    if (s != endp && *endp == '\0')
        return 1;
    else return 0;
}


int string_to_int(char *s, long *dest){
    char *endp;
    
    if(s == NULL) return 0;
    
    *dest = strtol(s, &endp, 0);
    
    if (s != endp && *endp == '\0')
        return 1;
    else return 0;
}

char *getSubstring(char* str, size_t begin, size_t len) { 
  
    int strln = strlen(str);
    
    if (str == NULL || strln == 0 || strln < begin || strln < (begin+len)) 
    return NULL; 

    return strndup(str + begin, len); 
}

void printSubstring(char *s,int lenght){
    int i=0;
    for(;i< lenght;i++) printf("%c",s[i]);
    printf("\n");
}

int isCharInString(char *str, char ch){
    char *ptr = NULL;
    if ( (ptr=strchr(str,ch)) == NULL)
        return -1;
    else return ptr-str;
    //else return 1;
}

int getsubStringPos(char *str, char *substr){
    char *ptr = NULL;
    if ( (ptr=strstr(str,substr)) == NULL)
        return -1;
    else return ptr-str;
    //else return 1;
}