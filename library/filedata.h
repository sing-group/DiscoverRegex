/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   filedata.h
 * Author: drordas
 *
 * Created on 6 de junio de 2016, 9:18
 */

#ifndef _FILEDATA_H_
#define _FILEDATA_H_

#include "common_dinamic_structures.h"

typedef struct email_data_t email_data;
typedef struct file_data_t file_data;
typedef struct tst_data_t tst_data;

// EMAIL DATA. STORES ONE ENTRY PER EMAIL.
email_data *newEmailData (char *line, Svector *words);
void freeEmailData(email_data *data);
void setEmailDataLine(email_data *data,char *line);
void replaceEmailDataLine(email_data *data,char *line);
void setEmailDataWords(email_data *data,Svector *words);
void replaceEmailDataWords(email_data *data,Svector *words);

Svector *getEmailLineWords(email_data *data);
char *getEmailFullLine(email_data *data);
void freeTstData(tst_data *data);

// FILE DATA. STORES EMAIL DATA STRUCTURE.
file_data *newFileData(Svector *SpamLines, Gvector *SpamData, Svector *HamLines, Gvector *HamData);
tst_data *newTstData(Svector *spamLines, Svector *hamLines);
void freeFileData(file_data *data);
void setFileDataSpamLines(file_data *fData, Svector *SpamLines);
void removeFileDataSpamLines(file_data *fData);
void replaceFileDataSpamLines(file_data *fData, Svector *SpamLines);
void setFileDataSpamData(file_data *fData, Gvector *SpamData);
void removeFileDataSpamData(file_data *fData);
void replaceFileDataSpamData(file_data *fData, Gvector *SpamData);
void setFileDataHamLines(file_data *fData, Svector *HamLines);
void removeFileDataHamLines(file_data *fData);
void replaceFileDataHamLines(file_data *fData, Svector *HamLines);
void setFileDataHamData(file_data *fData, Gvector *HamData);
void removeFileDataHamData(file_data *fData);
void replaceFileDataHamData(file_data *fData, Gvector *HamData);

Gvector * getHamData(file_data *fData);
Svector * getHamLines(file_data *fData);
Gvector * getSpamData(file_data *fData);
Svector * getSpamLines(file_data *fData);

Svector *getTstSpamLines(tst_data *tst);
Svector *getTstHamLines(tst_data *tst);

email_data *getSpamEmailDataAt(file_data *fData, long int pos);
email_data *getHamEmailDataAt(file_data *fData, long int pos);

#endif /* FILEDATA_H */

