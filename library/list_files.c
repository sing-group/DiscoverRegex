/***************************************************************************                        
*
*   File    : list_files.c
*   Purpose : 
*            
*            
*   Author  : Ivan Paz (from GrindStone Project)
*   Date    : September  22, 2010
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
#include <errno.h>
#include <dirent.h>
#include "list_files.h"
#include "logger.h"

//Definition of Datatype filelist
struct filelist{
   int count;
   char **list;
};

//Private function list
int count(const char *ruta,const char *ext);


filelist *list_files(const char *dire,const char *extension){
   DIR * dirp;
   struct dirent * entry;
   char *buffer;
   char *tmp;
   char *dir;
   filelist *confs;
   int i=0;

   dir=malloc((strlen(dire)+1)*sizeof(char));
   dir=strcpy(dir,dire);
   if (dir[strlen(dir)-1]=='/') dir[strlen(dir)-1]='\0';
   confs=malloc(sizeof(filelist));
   confs->count=1;
   confs->count=count(dir,extension);
   if (confs->count==0) return confs;

   confs->list=malloc(sizeof(char*)*confs->count);
   dirp = opendir(dir); /* There should be error handling after this,pending errno control */
   while ((entry = readdir(dirp)) != NULL) {
     if (entry->d_name[0] != '.'){
        buffer=malloc(sizeof(char)*(strlen(entry->d_name)+1));
        strcpy(buffer,entry->d_name);
        tmp=strtok(buffer,".");
        if ((tmp=strtok(NULL," "))!=NULL && strcmp(tmp, extension)==0){
	   //confs->list[i]=(char *)malloc((strlen(entry->d_name)+strlen(dir)+2)*sizeof(char));
	   if( asprintf(&(confs->list[i]),"%s/%s",dir,entry->d_name) <=0 ){
               printlog(LOG_CRITICAL,"LIST_FILES","Error: Cannot allocate enought memmory\n");
               free(buffer);
               free(dir);
               closedir(dirp);
               return NULL;
           }
	   i++;
        }
        free(buffer);
     }
  }
  free(dir);
  closedir(dirp);
  return confs;
}

int count(const char *ruta,const char *ext){
  DIR *dirp;
  struct dirent *entry;
  char *buffer;
  char *tmp;

  int cont=0;

  if ( (dirp = opendir(ruta)) == NULL){ /* There should be error handling after this,pendin errno control */
      printlog(LOG_CRITICAL,"LIST_FILES","Error: Cannot open directory\n");
      return 0;
  }
  while ((entry = readdir(dirp)) != NULL) {
        buffer=malloc(sizeof(char)*strlen(entry->d_name)+sizeof(char));
        strcpy(buffer,entry->d_name);
        tmp=strtok(buffer,".");
        if ((tmp=strtok(NULL," "))!=NULL && strcmp(tmp, ext)==0){
           cont++; //We need count the num of extensions for the memory 
        }
        free(buffer);
  }
  closedir(dirp);
  return cont;
}

filelist *concat_filelist(const filelist *in1, const filelist *in2){
//We need concat confs
   filelist *concat_filelist;
   int i,j;

   concat_filelist=malloc(sizeof(filelist));
   concat_filelist->list=malloc((in1->count+in2->count)*sizeof(char *));

   for(i=0;i<in1->count;i++){
        concat_filelist->list[i]=malloc((strlen(in1->list[i])+1)*sizeof(char *));
	strcpy(concat_filelist->list[i],in1->list[i]);
   }
   for(j=in1->count;j<in1->count+in2->count;j++){
        concat_filelist->list[j]=malloc(strlen(in2->list[j-in1->count])*sizeof(char *));
	strcpy(concat_filelist->list[j],in2->list[j-in1->count]);
   }

   concat_filelist->count=(in1->count+in2->count);
   return concat_filelist;

}

void free_filelist(filelist *f){
    int i;

    if (f->count!=0)
      for(i=0;i<f->count;i++)
         free(f->list[i]);
    free(f->list);
    free(f);
}

int count_files_filelist(const filelist *f){
   return f->count;
}

char *get_file_at(const filelist *f, int pos){
   return f->list[pos];
}
