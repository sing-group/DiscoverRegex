/* 
 * File:   fileReader.h
 * Author: drordas
 *
 * Created on 14 de octubre de 2015, 13:37
 */

#ifndef _FILEREADER_H_
#define	_FILEREADER_H_

#include "linked_list.h"
#include "generic_vector.h"
#include "string_vector.h"
#include "filedata.h"

//typedef struct fileData_t fileData;

#define UTF_8 0
#define UTF_16BE 1
#define UTF_16LE 2
#define UTF_32BE 3 
#define UTF_32LE 4

file_data *loadToFileData(char *filePath1, char *filePath2);
tst_data *loadTstData(char *spamPath, char *hamPath);
Svector *fileToSvector (char *filePath);
Gvector *separateFileInWords(char *filePath);
Svector *separateFileInLines(char *filePath);
Svector *separateLineInWords(char *line);
void freeWordVector(Gvector *vector, PFree f);
char *fileToString(char *filePath);
int ae_load_file_to_memory(const char *filename, char **result);

#endif	/* FILEREADER_H */

