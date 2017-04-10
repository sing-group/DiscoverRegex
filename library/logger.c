/***************************************************************************
*
*   File    : logger.h
*   Purpose : Header file need for list conf logs.
*
*
*   Author  : David Ruano Ordas
*   Date    : October  30, 2010
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
//#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdarg.h>
#include <pthread.h>
#include "logger.h"
#include "string_util.h"

#define DEFAULT_LOG_LEVEL LOG_INFO

typedef struct tm datetime;

struct settings_log {
    tloglevel level;
    foutput CRITICAL;
    foutput WARNING;
    foutput INFO;
    foutput DEBUG;
    FILE *file;
    char *path;
};

typedef struct tlogdata_t{
    tlogsettings *output;
    pthread_mutex_t mutex4writing;
} tlogdata;

tlogdata *global_data = NULL;

tlogsettings *createLogSettings( char *path, tloglevel level, foutput critical, 
                                 foutput warning, foutput info, foutput debug){
    tlogsettings *settings = malloc(sizeof(tlogsettings));
    settings->CRITICAL = critical;
    settings->DEBUG = debug;
    settings->INFO = info;
    settings->WARNING = warning;
    settings->file = NULL;
    settings->path = path;
    settings->level = level;
    
    return settings;
}

tlogsettings *createDefaultLogSettings(){
    tlogsettings *settings = malloc(sizeof(tlogsettings));
    settings->CRITICAL = STDOUT;
    settings->DEBUG = STDOUT;
    settings->INFO = STDOUT;
    settings->WARNING = STDOUT;
    settings->file = NULL;
    settings->path = NULL;
    settings->level = LOG_INFO;
    
    return settings;
}

void start_logger(tlogsettings *settings, char *module){
    datetime *ptr;
    time_t lt = time(NULL);
    ptr = localtime(&lt);
    char *date=strtok(asctime(ptr),"\n");
    
    if(global_data==NULL){
        global_data = malloc(sizeof(tlogdata));
        pthread_mutex_init(&(global_data->mutex4writing),NULL);
        if (settings == NULL || settings->path == NULL){
            global_data->output = malloc(sizeof(tlogsettings));
            global_data->output->CRITICAL = STDOUT;
            global_data->output->DEBUG = STDOUT;
            global_data->output->INFO = STDOUT;
            global_data->output->WARNING = STDOUT;
            global_data->output->file = NULL;
            global_data->output->path = NULL;
            global_data->output->level = LOG_DEBUG;
        }
        else{
            global_data->output=settings;
            if ( global_data->output->path != NULL || global_data->output->CRITICAL == FILEOUT || 
                 global_data->output->DEBUG == FILEOUT || global_data->output->INFO == FILEOUT ||
                 global_data->output->WARNING == FILEOUT ){
                if ( (global_data->output->file = fopen(global_data->output->path,"a+w") ) ==NULL ){
                    printf("[%s] CRITICAL - %s: Could not create file %s\n",date,module,global_data->output->path);
                }
            }
        }
    }else printf("[%s] CRITICAL - %s: Logger already started\n",date,module);
}

void stop_logger(){
  
    if(global_data != NULL){
        if(global_data->output->path != NULL) fclose(global_data->output->file);
        pthread_mutex_destroy(&(global_data->mutex4writing));
        free(global_data->output);
        free(global_data);
    }
}

void writeCritical(char *date, const char* module, char *fmt, va_list arg){
    if (global_data->output->CRITICAL == STDOUT){
        printf("\r[%s] CRITICAL - %s: ",date,module);
        vfprintf(stdout,fmt,arg);
        fflush(stdout);
    }else{ 
        fprintf(global_data->output->file,"[%s] CRITICAL - %s: ",date,module);
        vfprintf(global_data->output->file,fmt,arg);
        fflush(global_data->output->file);
    }
}

void writeWarning(char *date, const char* module, char *fmt, va_list arg){
    if (global_data->output->WARNING == STDOUT){
        printf("\r[%s] WARNING - %s: ",date,module);
        vfprintf(stdout,fmt,arg);
        fflush(stdout);
    }else{ 
        fprintf(global_data->output->file,"[%s] WARNING - %s: ",date,module);
        vfprintf(global_data->output->file,fmt,arg);
        fflush(global_data->output->file);
    }
}

void writeInfo(char *date, const char* module, char *fmt, va_list arg){
    if (global_data->output->INFO == STDOUT){
        printf("\r[%s] INFO - %s: ",date,module);
        vfprintf(stdout,fmt,arg);
        fflush(stdout);
    }else{ 
        fprintf(global_data->output->file,"[%s] INFO - %s: ",date,module);
        vfprintf(global_data->output->file,fmt,arg);
        fflush(global_data->output->file);
    }
}

void writeDebug(char *date, const char* module, char *fmt, va_list arg){
    if (global_data->output->DEBUG == STDOUT){
        printf("\r[%s] DEBUG - %s: ",date,module);
        vfprintf(stdout,fmt,arg);
        fflush(stdout);
    }else{ 
        fprintf(global_data->output->file,"[%s] DEBUG - %s: ",date,module);
        vfprintf(global_data->output->file,fmt,arg);
        fflush(global_data->output->file);
    }
}


void printlog(tloglevel level,const char* module,char *fmt,...){
    
    if(global_data==NULL) start_logger(createDefaultLogSettings(),"LOGGER");
    
    if(global_data->output->level >= level){
        pthread_mutex_lock(&(global_data->mutex4writing));
        datetime *ptr;
        time_t lt = time(NULL);
        ptr = localtime(&lt);
    
        char *date;
        va_list argp;
        
        va_start(argp, fmt);
        
        date=strtok(asctime(ptr),"\n");
        
        switch(level){
            case LOG_CRITICAL:                                
                               //fprintf(global_data->file,"[%s] CRITICAL - %s: ",date,module);
                               writeCritical(date, module, fmt, argp);
                               break;
            case LOG_WARNING:                                
                               //fprintf(global_data->file,"[%s] WARNING - %s: ",date,module);
                               writeWarning(date,module, fmt, argp);
                               break;
            case LOG_INFO:     
                               //fprintf(global_data->file,"[%s] INFO - %s: ",date,module);
                               writeInfo(date,module, fmt, argp);
                               break;
            case LOG_DEBUG:                                   
                               //fprintf(global_data->file,"[%s] DEBUG - %s: ",date,module);
                               writeDebug(date,module, fmt, argp); 
                               break;
        }
        
        //vfprintf(global_data->file,fmt,argp);
            
        va_end(argp);
        
        pthread_mutex_unlock(&(global_data->mutex4writing));
    }
}