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

#ifndef __LOGGER_H__
#define __LOGGER_H__

typedef enum eloglevel{ 
    LOG_CRITICAL, 
    LOG_WARNING, 
    LOG_INFO, 
    LOG_DEBUG 
} tloglevel;

typedef struct settings_log tlogsettings;
typedef enum efoutput {FILEOUT, STDOUT} foutput;

void printlog(tloglevel level,const char* module,char *fmt,...);

void start_logger(tlogsettings *settings, char *module);

void stop_logger();

tlogsettings *createLogSettings( char *path, tloglevel level, foutput critical, 
                                 foutput warning, foutput info, foutput debug);
#endif
