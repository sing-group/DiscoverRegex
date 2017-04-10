/* 
 * File:   spamData.h
 * Author: drordas
 *
 * Created on 25 de noviembre de 2015, 17:45
 */

#ifndef _SPAM_DATA_UTIL_H_
#define	_SPAM_DATA_UTIL_H_

typedef struct spam_data_t SpamData;
typedef void * spamElement;

typedef int (*SDFree)(spamElement);

char *getSpamLine(SpamData * data);
Svector *getSpamWords(SpamData * data);
SpamData *newSpamData(char *line, Svector *words);
void freeSpamData(SpamData *data);

#endif	/* SPAMDATA_H */

