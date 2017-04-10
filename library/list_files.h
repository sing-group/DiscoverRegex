/***************************************************************************                        
*
*   File    : list_files.h
*   Purpose : Header file need for list conf logs. 
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

#ifndef __LIST_FILES_H__
#define __LIST_FILES_H__

struct filelist;
typedef struct filelist filelist;

filelist *list_files(const char *dire,const char *extension);
filelist *concat_filelist(const filelist *in1, const filelist *in2);
void free_filelist(filelist *f);
int count_files_filelist(const filelist *f);
char *get_file_at(const filelist *f, int pos);
#endif
