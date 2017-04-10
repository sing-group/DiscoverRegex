/***************************************************************************                        
*
*   File    : ruleset.c
*   Purpose : Implements a ruleset (set of rules) for filtering
*            
*   Original Author: Ivan Paz, Jose Ramon Mendez (from Grindstone project)
*   Has been widelly modifyed since them
* 
*   Memory improvements, modifications, inclusion of new fields
*       and functions: David Ruano, Noemi Perez, Jose Ramon Mendez
* 
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
#include "ruleset.h"
#include "list_files.h"
#include "logger.h"
#include "hashmap.h"
#include "parse_func_args.h"
#include "meta.h"
#include <math.h>

//BEGIN VICTOR
#include <stdarg.h>
#include <sys/mman.h>
//END VICTOR

/***********************************************************************
 Datatypes
 **********************************************************************/

int debug_mode=0;

//struct stop_score {
//    float upper;
//    float lower;
//};

//typedef struct stop_score stop_score;



/***********************************************************************
 Private Function List
***********************************************************************/

/**
 * Indicates if line is commented
 */
int is_commented(char *line);

/**
 * Update a ruleset map when created from another ruleset
 * (Combine and Mutate operations)
 */
void update_map(ruleset *r);

/**
 * Comparator for sorting rules by plugin
 */
int compare_rules_by_plugin(const void *_a, const void *_b);

int fprint_domains(any_t file,any_t key);

int print_domains(any_t nullpointer,any_t key);

int free_meta_pos(any_t item, any_t data);

int free_vector_element(element data);

/***********************************************************************
 Private and Public functions Implementation
***********************************************************************/ 

/* count the rules on a ruleset */
int count_rules(const ruleset *rules){
   return rules->size;
}

int count_definitive_rules(const ruleset *rules){
   return rules->def_size;
}

/* count the number of meta rules on a ruleset */
int count_meta_rules(const ruleset *rules){
   return rules->meta_size;
}


ruleset *load_ruleset(const filelist *files,const filelist *attrs, map_t parsers_name){
   int rulenum=0, meta_num=0;
   char ch;
   char *line;
   FILE *fp;
   ruleset *r= NULL;
   char *tmpstr, *tmptoken;
   int i; int *rulepos;
   int initparameters,endparameters;
   char *tmpDefName, *tmpDefParams;  
   char *linecopy=NULL;
   
   if (count_files_filelist(files)==0) return r;
   
   //Initialize ruleset 
   if((r=(ruleset *)malloc(sizeof(ruleset)))==NULL){
       wblprintf(LOG_CRITICAL,"RULESET","Insuficient Memory\n");
       exit(EXIT_FAILURE);
   }   
   r->map= hashmap_new();
   r->meta_map= hashmap_new();
   r->dependant_map = hashmap_new();
   r->required=0;
      
   //Initialize
   if((line=(char *)malloc(sizeof(char)))==NULL){
      wblprintf(LOG_CRITICAL,"RULESET","Insuficient Memory\n");
      exit(EXIT_FAILURE);
   }
   strcpy(line,"");
   
   //Compute the number of rules in order to detect the size of the
   for(i=0;i<count_files_filelist(files);i++){
     fp=fopen(get_file_at(files,i),"r");
     if (fp==NULL) {
         wblprintf(LOG_CRITICAL,"RULESET","Unable to open file %s. Exiting...\n",get_file_at(files,i));
         exit(EXIT_FAILURE);
     }
     do{
        ch=fgetc(fp);
        if(ch == '\n' || ch == EOF) {
            if (strcmp(trim(line),"")==0) continue; //Discard line if void
            if (!is_commented(line)){
                 tmpstr=strtok(line," \t");
                 any_t result;
                 if( hashmap_get(parsers_name,tmpstr,(any_t *)&result)!=MAP_MISSING || 
                     !strcmp(tmpstr,"meta") ){
                     tmpstr=strtok(NULL," \t"); //Take rulename in tmpstr
                     tmptoken=malloc(sizeof(char)*(1+strlen(tmpstr)));
                     strcpy(tmptoken,tmpstr);
                     if (get_rule_index(r,tmptoken)==-1){ //If rule is not included
                         rulepos=malloc(sizeof(int));
                         *rulepos=rulenum;
                         hashmap_put(r->map,tmptoken,rulepos);
                         rulenum++;
                         wblprintf(LOG_INFO,"RULESET","Inserting rulename '%s'\n",tmptoken);
                     }else wblprintf(LOG_WARNING,"RULESET","Rule definition repeated %s at position %i\n",tmpstr,get_rule_index(r,tmpstr));     
                 }else{ 
                    (strcmp(tmpstr,"score") && strcmp(tmpstr,"domain") && 
                     strcmp(tmpstr,"describe") && strcmp(tmpstr,"required_score") ) ? 
                        (wblprintf(LOG_WARNING,"RULESET","Parser %s not valid\n",tmpstr)):
                        (0);
                 }
                 tmptoken=NULL;
            }
            free(line);
            line=(char *)malloc(sizeof(char));
            strcpy(line,"");
        }else{
            line=append(line,ch);
        }
     } while (ch!=EOF);
     fclose(fp);
   }
   
   if(rulenum==0){ 
       wblprintf(LOG_CRITICAL,"RULESET","Filter is empty, no rules are defined. Aborting...\n");
       exit(EXIT_FAILURE);
   }
   
   r->rules=(rule *)malloc(sizeof(rule)*(rulenum));
   r->size=rulenum;
   
   for (i=0;i<r->size;i++){
        r->rules[i].def = NULL;
        r->rules[i].par = NULL;
        r->rules[i].rulename = NULL;
        r->rules[i].score = NULL;
        r->rules[i].description = NULL;
        r->rules[i].target_domain = NULL;
        r->rules[i].attr =  NULL;
        r->rules[i].characteristic=0;        
   }
   
   r->sdata_t=malloc(sizeof(schedule_data_t));
   r->sdata_t->end_definitive=0;
   r->sdata_t->end_meta=0;
   r->sdata_t->begin_normal=0;
   
   r->def_size=0;
   r->meta_size=meta_num;
   
   r->intervals=(score_intervals *)malloc(sizeof(score_intervals));
   r->intervals->positive=0;
   r->intervals->negative=0;
   
   //Load rules from file
   rulenum=0;
   free(line);
   
   line=(char *)malloc(sizeof(char));
   line[0]='\0';
   linecopy=NULL;
   rule *actual_rule = NULL;
   
   for(i=0;i<count_files_filelist(files);i++){
       fp=fopen(get_file_at(files,i),"r");
       do {
         ch=fgetc(fp);
         if(ch=='\n' || ch == EOF) {
           if (strcmp(trim(line),"")==0) continue; //Descartar la linea si está vacia
           if (!is_commented(line)){
              any_t result;
              if( ( linecopy = strdup(line) ) == NULL){
                  wblprintf(LOG_CRITICAL,"RULESET","ERROR: Cannot allocate enought memmory\n");
                  fclose(fp);
                  return NULL;
              }

              tmpstr=strtok(line," \t");

              if ( hashmap_get(parsers_name,tmpstr,(any_t*)&result) != MAP_MISSING ){

                  tmpstr=strtok(NULL," \t");
                  //SI LA REGLA ESTA EN EL HASHMAP
                  if((actual_rule = get_rule_by_name(r,tmpstr))!= NULL){
                    set_parser(actual_rule,line);//,EML_PARSER);
                    actual_rule->characteristic = NORMAL_RULE;
                    actual_rule->rulename=hashmap_getkeypointer(r->map,tmpstr); 

                    //Next token: parser definition
                    tmpstr=strtok(NULL," \t");

                    //Separar el nombre de la funcion de los parametros.
                    tmptoken=strtok(tmpstr,"(");

                    tmpDefName= (char *)malloc((strlen(tmptoken)+1)*sizeof(char));
                    strcpy(tmpDefName,tmptoken);
                    
                    for(initparameters=0;linecopy[initparameters]!='(' && linecopy[initparameters]!='\0';initparameters++);
                    if(linecopy[initparameters]!='\0'){
                        initparameters++;

                        for(endparameters=(strlen(linecopy)-1);linecopy[endparameters]!=')' && endparameters>initparameters;endparameters--);
                        if(endparameters-initparameters>=0){
                           tmpDefParams=malloc(sizeof(char)*(endparameters-initparameters+1));
                           memcpy(tmpDefParams, &(linecopy[initparameters]), endparameters-initparameters);
                           tmpDefParams[endparameters-initparameters]='\0';
                        }else tmpDefParams=NULL;

                        set_definition(actual_rule,tmpDefName,tmpDefParams);
                        free(tmpDefParams);
                    }
                    free(tmpDefName);
                  }
                  else{
                    wblprintf(LOG_CRITICAL,"RULESET","Rules changed during starting process... exiting\n");
                    exit(EXIT_FAILURE);
                  }
              }
              else if(strcmp(tmpstr,"meta")==0){
                      tmpstr=strtok(NULL," \t");

                      if((actual_rule=get_meta_by_name(r,tmpstr))==NULL){
                        wblprintf(LOG_CRITICAL,"RULESET","Meta rules changed during starting process... exiting\n");
                        exit(EXIT_FAILURE);
                      }  
                      actual_rule->characteristic = META_RULE;
                      actual_rule->rulename=hashmap_getkeypointer(r->meta_map,tmpstr);

                      char *aux=NULL;
                      while((tmpstr=strtok(NULL, "\t"))!=NULL) aux=tmpstr;                  

                      set_meta_definition(r,actual_rule,aux);

              }else 
                  if (strcmp(tmpstr,"score")==0) {
                    tmpstr=strtok(NULL," \t"); //Take the rulename
                    if (tmpstr!=NULL) {
                        if((actual_rule=get_rule_by_name(r,tmpstr))!= NULL ||
                           (actual_rule=get_meta_by_name(r,tmpstr))!= NULL ) 
                        {
                          tmpstr=strtok(NULL," \t"); //Take the rulescore
                          if(strcmp(tmpstr,"+")==0 || strcmp(tmpstr,"-")==0){
                              actual_rule->characteristic |= DEFINITIVE_SCORE;
                              char *score=malloc(sizeof(char)*(strlen(tmpstr)+1));
                              strcpy(score,tmpstr);
                              actual_rule->score=score;
                              r->def_size++;
                          }
                          else{
                              float *score = malloc(sizeof(float));
                              *score=atof(tmpstr);
                              actual_rule->score=(void *)score;
                              actual_rule->characteristic |= NORMAL_SCORE;
                              (*(score)>0)?
                                (r->intervals->positive+=*(score)):
                                (r->intervals->negative+=*(score));
                          }

                        }

                    } else wblprintf(LOG_WARNING,"RULESET","Unable to parse score: %s\n",line);

                  }else{ 
                      if (strcmp(tmpstr,"describe")==0) { //Revisar
                        tmpstr=strtok(NULL," \t");
                        if(tmpstr!=NULL){                         
                            if ( (actual_rule=get_rule_by_name(r,tmpstr)) != NULL || 
                                 (actual_rule=get_meta_by_name(r,tmpstr)) != NULL )
                            {
                              int rulename= strlen(tmpstr);
                              for( tmpstr=linecopy; (tmpstr[0] != ' ') && (tmpstr[0] != '\t');tmpstr=&tmpstr[1]);
                              tmpstr=(tmpstr+rulename+1);
                              tmpstr=trim(tmpstr);                          
                              actual_rule->description=malloc(sizeof(char)*(1+strlen(tmpstr))); //Take space for the string
                              strcpy(actual_rule->description,tmpstr);
                            } 
                        }else wblprintf(LOG_WARNING,"RULESET","Unable to parse description: %s\n",line);

                      }else{ 
                          if (strcmp(tmpstr,"required_score")==0) {
                              tmpstr=strtok(NULL," \t"); //Take the score
                              r->required=atof(tmpstr); //Assign the score
                          }
                          else{ 
                              if( strcmp(tmpstr,"domain")==0 ){
                                  tmpstr=strtok(NULL," \t");
                                  if(tmpstr!=NULL){
                                      if( (actual_rule = get_rule_by_name(r,tmpstr)) != NULL || 
                                          (actual_rule = get_meta_by_name(r,tmpstr)) != NULL )
                                      {
                                           (actual_rule->target_domain==NULL)?
                                                (actual_rule->target_domain=hashmap_new()):
                                                (0);
                                            while((tmpstr=strtok(NULL,"\n\t "))!=NULL){
                                                any_t result;
                                                if( hashmap_get(actual_rule->target_domain,tmpstr,&result) == MAP_MISSING)
                                                {
                                                    char *domain=malloc(sizeof(char) * (strlen(tmpstr)+1 ) );
                                                    strcpy(domain,tmpstr);
                                                    hashmap_put(actual_rule->target_domain,domain,NULL);
                                                } else wblprintf(LOG_WARNING,"RULESET","Domain %s already inserted: %s\n",tmpstr);
                                            }
                                      } 
                                  } 
                                  else wblprintf(LOG_WARNING,"RULESET","Unable to parse domain section: %s\n",line);
                              }
                          }
                      }
                  }
                free(linecopy);
           }
           free(line);
           line=(char *)malloc(sizeof(char));
           line[0]='\0';
           
         }else line=append(line,ch);
         
       }while (ch!=EOF);
       
       fclose(fp);
   }
   
   free(line);


   if ( count_files_filelist(attrs)==0 ){
       wblprintf(LOG_WARNING,"RULESET","Error: Attribute files not found. Revise file path\n");
       return r;
   }
   
   rulenum=0;
   line=(char *)malloc(sizeof(char));
   line[0]='\0';

   
   //Reading rule attributes
   for(i=0;i<count_files_filelist(attrs);i++){
       fp=fopen(get_file_at(attrs,i),"r");
       do{
         ch=fgetc(fp);
         if(ch=='\n' || ch == EOF) {
            if (strcmp(trim(line),"")==0) continue; //Descartar la linea si está vacia
            if (!is_commented(line)){
                tmpstr=strtok(line," \t");
                if( (actual_rule=get_rule_by_name(r,tmpstr)) != NULL ){
                    if(actual_rule->attr == NULL && (actual_rule->attr= malloc(sizeof(attributes))) !=NULL ){
                        
                        if( (tmpstr=strtok(NULL," \t")) != NULL )
                            set_num_executions_attr(get_rule_attributes(actual_rule),atol(tmpstr));
                        else set_num_executions_attr(get_rule_attributes(actual_rule),0); 
                        
                        if( (tmpstr=strtok(NULL," \t")) !=NULL )
                            set_num_success_attr(get_rule_attributes(actual_rule),atol(tmpstr));
                        else set_num_success_attr(get_rule_attributes(actual_rule),0);
                        
                        if( (tmpstr=strtok(NULL," \t")) !=NULL )
                            set_num_fails_attr(get_rule_attributes(actual_rule),atol(tmpstr));
                        else set_num_fails_attr(get_rule_attributes(actual_rule),0);
                        
                        if( (tmpstr=strtok(NULL," \t")) !=NULL )
                            set_confidence_attr(get_rule_attributes(actual_rule),atof(tmpstr));
                        else set_confidence_attr(get_rule_attributes(actual_rule),0.0f);
                        
                        //NORMALIZAR EL WEIGHT
                        if( (tmpstr=strtok(NULL," \t")) == NULL || (!strcmp(to_lower_case(tmpstr),"nan")) )
                            set_significance_attr(get_rule_attributes(actual_rule),(100)/(double)count_rules(r));
                        else set_significance_attr(get_rule_attributes(actual_rule),atof(tmpstr));
                        
                    } else wblprintf(LOG_WARNING,"RULESET","Attributes for rule '%s' already defined\n",tmpstr);
                }else wblprintf(LOG_WARNING,"RULESET","Rule '%s' not assigned. Ignoring rule attributes\n",tmpstr);
            }
            free(line);
            line=(char *)malloc(sizeof(char));
            line[0]='\0';
         }else line=append(line,ch);
         
       }while(ch!=EOF);
       
       fclose(fp);
   }
   
   for(i=0;i<count_rules(r);i++){
      if(r->rules[i].score == NULL){
         wblprintf(LOG_WARNING,"RULESET","Rule %s without score\n\t\t\t\t\t      Assigning default score\n",r->rules[i].rulename);
         float *score=malloc(sizeof(float));
         *score=0;
         (r->rules[i].score)=(void *)score;
      }
   }
   
   return r;
}

/* Mutate operation for genetic algorithms */
ruleset *mutate(const ruleset *rulesin){
   int i,j;
   int veces;
   ruleset *r;
   double aleat;

   //Generar aleatorio
   //rand()
   //Generar aleatorio entre 0 y n-1 que corresponde a generar un aleatorio
   //para mutar la regla i en un conjunto de reglas de tamaño n (primera regla
   //la regla 0, última regla n-1
   //rand() % n

   //Initialize and copy ruleset 
   r=(ruleset *)malloc(sizeof(ruleset));
   r->required=rulesin->required;
   r->rules=(rule *)malloc(sizeof(rule)*rulesin->size);
   r->size=rulesin->size;
   //TODO: Copy the rest of fields
   for (i=0;i<r->size;i++){
      if(r->rules[i].characteristic & NORMAL_RULE & NORMAL_SCORE){
          *((float *)r->rules[i].score)=*((float *)rulesin->rules[i].score);
          r->rules[i].rulename=(char *)malloc(sizeof(char)*(strlen(rulesin->rules[i].rulename)+1));
          strcpy(r->rules[i].rulename,rulesin->rules[i].rulename);
      }
   }
 
   veces=rand() % (r->size -1)+1;
   for (j=0;j<veces;j++){
       if(r->rules[i].characteristic & NORMAL_RULE & NORMAL_SCORE){
           i=rand() % r->size;
           aleat=0.5*((rand()%10) + 1);
           //printf("Número generado %f\n",aleat);
           (rand() % 2)?
               (*((float *)r->rules[i].score)+=aleat):
               (*((float *)r->rules[i].score)-=aleat);
       }
   }

   //update the map in order to find rules by name
   update_map(r);
   return r; 
}

/* Combine operation for genetic algorithms */
ruleset *combine(const ruleset *rules1,const ruleset *rules2){
   int i;
   ruleset *r;

   //Generar aleatorio
   //rand()
   //Generar aleatorio entre 0 y n-1 que corresponde a generar un aleatorio
   //para mutar la regla i en un conjunto de reglas de tamaño n (primera regla
   //la regla 0, última regla n-1
   //rand() % n

   //Initialize and copy ruleset 
   r=(ruleset *)malloc(sizeof(ruleset));
   r->required=rules1->required;
   r->rules=(rule *)malloc(sizeof(rule)*rules1->size);
   r->size=rules1->size;
   //TODO: Copy the rest of fields

   for (i=0;i<r->size;i++){
       
      if(r->rules[i].characteristic & NORMAL_RULE & NORMAL_SCORE){ 
          r->rules[i].rulename=(char *)malloc(sizeof(char)*(strlen(rules1->rules[i].rulename)+1));
          strcpy(r->rules[i].rulename,rules1->rules[i].rulename);

          switch(rand()%10){
             case 0: *((float *)r->rules[i].score)=*((float *)rules1->rules[i].score);
                break;
             case 1: *((float *)r->rules[i].score)=*((float *)rules2->rules[i].score);
                break;
             case 2: *((float *)r->rules[i].score)=(*((float *)rules1->rules[i].score) + *((float *)rules2->rules[i].score)) / (float)2;
               break;
             case 3:
             case 7: *((float *)r->rules[i].score)=(*((float *)rules1->rules[i].score) > *((float *)rules2->rules[i].score))?*((float *)rules1->rules[i].score):*((float *)rules2->rules[i].score);
               break;
             case 4:*((float *)r->rules[i].score)=(*((float *)rules1->rules[i].score) < *((float *)rules2->rules[i].score))?*((float *)rules1->rules[i].score):*((float *)rules2->rules[i].score);
               break;
             case 5:
             case 8:*((float *)r->rules[i].score)=(rand()%2)?(*((float *)rules1->rules[i].score) - (0.1*((rand()%10) + 1))):(*((float *)rules1->rules[i].score) + (0.1*((rand()%10) + 1)));
               break;
             case 6:
             case 9:*((float *)r->rules[i].score)=(rand()%2)?(*((float *)rules2->rules[i].score) - (0.1*((rand()%10) + 1))):(*((float *)rules2->rules[i].score) + (0.1*((rand()%10) + 1)));
               break;
          }
      }
   }

   update_map(r);
   return r;
}

/* Write a ruleset in a file */
void write_ruleset(const char* filename, const ruleset *rules){
    FILE *file=fopen(filename,"w");
    int i;
    map_t rule_domain;
    fprintf(file,"#Ruleset output (size=%d)\n",rules->size);
    for (i=0;i<count_rules(rules);i++){
        fprintf(file,"#Rule %d\n",i);
        rule *actual_rule = get_rule(rules,i);
        if(get_rule_characteristic(actual_rule) & NORMAL_RULE){
            if(get_definition_param(actual_rule)!=NULL)
               fprintf(file,"%s %s %s(%s)\n",get_parser_name(actual_rule),get_rulename(actual_rule),
                      get_definition_name(actual_rule), get_definition_param(actual_rule));
            else
               fprintf(file,"%s %s %s()\n",get_parser_name(actual_rule),get_rulename(actual_rule),
                       get_definition_name(actual_rule));


            (get_definition_tflags(actual_rule)!=NULL)?(fprintf(file,"tflags %s\n",get_definition_tflags(actual_rule))):(0);
        }else fprintf(file,"meta %s %s\n",get_rulename(actual_rule),get_meta_definition(actual_rule));

        (rules->rules[i].characteristic & NORMAL_SCORE)?
            (fprintf(file,"score %s %.2lf\n",get_rulename(actual_rule),*((float *)get_rule_score(actual_rule)))):
            (fprintf(file,"score %s %s\n",get_rulename(actual_rule),(char *)get_rule_score(actual_rule)));

        if (get_description(actual_rule)!=NULL)
            fprintf(file,"describe %s %s\n\n",get_rulename(actual_rule),get_description(actual_rule));

        if((rule_domain=get_rule_domain(actual_rule))!=NULL){
           fprintf(file,"domain");
           hashmap_iterate_keys(rules->map, &print_domains, NULL);
           fprintf(file,"\n");
        }
    }
    
    fprintf(file,"required_score %.2lf\n",get_required_score(rules));
    
    fclose(file);
}

/* Write the ruleset in standard output */
void out_ruleset(const ruleset *rules){
    int i;
    map_t rule_domain;
    printf("######################################################\n");
    printf("#                   RULESET OUTPUT                   #\n");
    printf("######################################################\n");
    printf("RULESET SIZE = %d RULES\n",count_rules(rules));
    printf("######################################################\n");
    for (i=0;i<count_rules(rules);i++){
        rule *actual_rule = get_rule(rules,i);
        if(get_rule_characteristic(actual_rule) & NORMAL_RULE){
            if(get_definition_param(actual_rule)!=NULL)
               printf("%s %s %s(%s)\n",get_parser_name(actual_rule),get_rulename(actual_rule),
                      get_definition_name(actual_rule), get_definition_param(actual_rule));
            else
               printf("%s %s %s()\n",get_parser_name(actual_rule),get_rulename(actual_rule),
                      get_definition_name(actual_rule));


            (get_definition_tflags(actual_rule)!=NULL)?(printf("tflags %s\n",get_definition_tflags(actual_rule))):(0);
        }else printf("meta %s %s\n",get_rulename(actual_rule),get_meta_definition(actual_rule));

        (rules->rules[i].characteristic & NORMAL_SCORE)?
            (printf("score %s %.2lf\n",get_rulename(actual_rule),*((float *)get_rule_score(actual_rule)))):
            (printf("score %s %s\n",get_rulename(actual_rule),(char *)get_rule_score(actual_rule)));

        if (get_description(actual_rule)!=NULL)
            printf("describe %s %s\n",get_rulename(actual_rule),get_description(actual_rule));

        if((rule_domain=get_rule_domain(actual_rule))!=NULL){
           printf("domain");
           hashmap_iterate_keys(rules->map, &print_domains, NULL);
           printf("\n");
        }
        
        printf("attr %s %ld %ld %ld %2.3lf %2.3lf\n",get_rulename(actual_rule),
                                                 get_num_executions_attr(get_rule_attributes(actual_rule)),
                                                 get_num_success_attr(get_rule_attributes(actual_rule)),
                                                 get_num_fails_attr(get_rule_attributes(actual_rule)),
                                                 get_confidence_attr(get_rule_attributes(actual_rule)),
                                                 get_significance_attr(get_rule_attributes(actual_rule)));
        
        printf("\n");
    }
}

void write_rule_attributes(const char* filename,const ruleset *rules){
    int i;
    FILE *file = NULL;
    
    if( (file = fopen(filename,"w")) != NULL ){
        printf("#################################################################################");
        printf("#                                                                               #");
        printf("#             ATTRIBUTES FOR RULE CBR AUTO-LEARNING                             #");
        printf("#                                                                               #");
        printf("#################################################################################");    
        printf("#                                                                               #");
        printf("# RULENAME   NUM_EXECUTIONS   NUM_SUCCESS   NUM_FAILS   CONFIDENCE SIGNIFICANCE #");
        printf("#                                                                               #");
        printf("#################################################################################");   
        
        for(i=0;i< count_rules(rules);i++){
            rule *aux =get_rule(rules,i);
            fprintf(file,"%s\t%ld\t%ld\t%ld\t%2.3lf\t%2.3lf\n",get_rulename(aux),
                                                          get_num_executions_attr(get_rule_attributes(aux)),
                                                          get_num_success_attr(get_rule_attributes(aux)),
                                                          get_num_fails_attr(get_rule_attributes(aux)),
                                                          get_confidence_attr(get_rule_attributes(aux)),
                                                          get_significance_attr(get_rule_attributes(aux)));
        }
        fclose(file);
    }else{
        wblprintf(LOG_CRITICAL,"RULESET","Error: Cannot write rule attributes. Rule path is incorrect\n");
    }
}

rule *get_rule(const ruleset *rules, const int ruleno){
    if(rules == NULL || ruleno >= rules->size)
        return NULL;
    else return &(rules->rules[ruleno]);
}

/*Get required score.*/
float get_required_score(const ruleset *rules){
   return rules->required;
}

int print_domains(any_t nullpointer,any_t key){
    printf(" %s",(char *)key);
    return MAP_OK;
}

int fprint_domains(any_t file,any_t key){
    fprintf(file," %s",(char *)key);
    return MAP_OK;
}

/*Get the name of a certain rule*/
char *get_rulename_at(const ruleset *rules, const int ruleno){
    return rules->rules[ruleno].rulename;
}

char *get_rulename(const rule *r){
    return r->rulename;
}

//Get the number of time the rule is triggered
long int get_num_executions_attr(attributes *attr){
    if (attr!=NULL)
        return attr->num_executions;
    else return 0;
}

//Get the number of time the rule has matched an email succesfully
long int get_num_success_attr(attributes *attr){
    if (attr!=NULL)
        return attr->num_success;
    else return 0;
}

long int get_num_fails_attr(attributes *attr){
    if(attr!=NULL){
        return attr->num_fails;
    }else return 0;
}

//Get the confidence of a rule
double get_confidence_attr(attributes *attr){
    if (attr!=NULL)
        return attr->confidence;
    else return 0.0;
}

attributes *get_rule_attributes(const rule *r){
    if(r == NULL) 
        return NULL;
    else return r->attr;
}

//get the weight of a rule.
double get_significance_attr(attributes *attr){
    if (attr != NULL)
        return attr->significance;
    else return -1.0f;
}

definition *get_definition(const rule *r){
    return r->def;
}

char *get_meta_definition(const rule *r){
    return ((meta_definition *)r->def)->expresion;
}

char *get_definition_name(const rule *r){
    return ((definition *)r->def)->name;
}

char *get_definition_param(const rule *r){
    return ((definition *)(r->def))->param;
}

function_t *get_definition_pointer(const rule *r){
    return ((definition *)(r->def))->pointer;
}

char *get_definition_tflags(const rule *r){
    return ((definition *)(r->def))->tflags;
}

parser *get_parser(const rule *r){
    return r->par;
}

char *get_parser_name(const rule *r){
    return r->par->parserName;
}


map_t get_rule_domain(const rule *r){
    return r->target_domain;
}

parser_t *get_parser_pointer(const rule *r){
    return r->par->parserPointer;
}

void *get_rule_score(const rule *r){
   return r->score;
}

int has_zero_score(const rule *r){
    if(get_rule_characteristic(r) & NORMAL_SCORE){
        float score= *(float *)get_rule_score(r);
        if(score==0 || score ==0.0) return 1;
        else return 0;
    }
    return NOT_NUMERIC_SCORE;
}

float get_positive_score(const ruleset *rules){
    return rules->intervals->positive;
}

short get_rule_characteristic_at(const ruleset *rules, const int ruleno){
    return rules->rules[ruleno].characteristic;
}


short get_rule_characteristic(const rule *r){
    return r->characteristic;
}

float get_negative_score(const ruleset *rules){
    return rules->intervals->negative;
}

int free_domains(any_t nullpointer,any_t key){
    free(key);
    return MAP_OK;
}

/*Free the ruleset struct from memory.*/
void ruleset_free(ruleset *r){
    int i;
    int *pi;

    for (i=0;i<count_rules(r);i++){
       rule *aux = get_rule(r,i);
       hashmap_get(r->map,get_rulename(aux), (any_t *)&pi);
       hashmap_remove(r->map, get_rulename(aux));
       free(pi);
       (get_rulename(aux)!=NULL)?(free(r->rules[i].rulename)):(0);
       (get_rule_score(aux)!=NULL)?(free(r->rules[i].score)):(0);
       (get_description(aux)!=NULL)?(free(r->rules[i].description)):(0);
       definition *def_rule = get_definition(aux);
       if (def_rule != NULL){
           if(get_rule_characteristic(aux) & NORMAL_RULE){
             (def_rule->name != NULL)?(free(def_rule->name)):(0);
             def_rule->name = NULL;
             
             (def_rule->param != NULL)?(free(def_rule->param)):(0);
             def_rule->param = NULL;
             
             if(def_rule->plugin != NULL){
                 if(def_rule != NULL) free(def_rule->plugin->name);
                 free(def_rule->plugin);
             }
             
             def_rule->plugin=NULL;
             
             (def_rule->tflags != NULL)? (free(def_rule->tflags)):(0);
             def_rule->tflags=NULL;
             
             //printf("+\n");
           }else{
               ( ((meta_definition*)(r->rules[i].def))->expresion!=NULL )?
                   (free( ((meta_definition *)(r->rules[i].def))->expresion)):
                   (0);
               if(((meta_definition*)(r->rules[i].def))->dependant_rules!=NULL ){
                   vector *aux=((meta_definition*)(r->rules[i].def))->dependant_rules;
                   free_meta_dependant_rules(aux);
               }
           }
           
           free(r->rules[i].def);
       }
       
       if (r->rules[i].attr != NULL){
           free(r->rules[i].attr);
           r->rules[i].attr = NULL;
       }
       
       if(r->rules[i].target_domain != NULL){
           hashmap_iterate_keys(r->rules[i].target_domain,&free_domains,NULL);
           hashmap_free(r->rules[i].target_domain);
       }
       
       if (r->rules[i].par != NULL){
           (r->rules[i].par->parserName != NULL)?(free(r->rules[i].par->parserName)):(0);
           free(r->rules[i].par);
       }
    }

    if(r->sdata_t != NULL) free(r->sdata_t);
    
    hashmap_free(r->dependant_map);
    
    if(r->intervals != NULL){
        free(r->intervals);
        r->intervals = NULL;
    }
    
    hashmap_free(r->map);
    hashmap_iterate(r->meta_map,&free_meta_pos,NULL);
    hashmap_free(r->meta_map);
    free(r->rules);

    free(r);
}

int free_meta_pos(any_t item, any_t data){
    free(data);
    return MAP_OK;
}

/*Set rules required score*/
void set_required_score(ruleset *rules, float required){
   rules->required=required;
}

void set_meta_definition(ruleset *rules, rule *r, char *expresion){ 
    meta_definition *def =(meta_definition *)r->def;
    int i=0;
    
    if(def != NULL){
        if(def->expresion != NULL) free(def->expresion);
        if(def->dependant_rules != NULL){
            for(;i<def->dependant_rules->size;i++){
                free(def->dependant_rules->v[i]);
            }
            free(def->dependant_rules);
        }
    }
    
    if( (def=(meta_definition *)malloc(sizeof(meta_definition))) ==NULL){
        wblprintf(LOG_CRITICAL,"RULESET","Not enough memory\n");
        exit(EXIT_FAILURE);
    }
    if( (def->expresion=malloc(sizeof(char)*(strlen(expresion)+1))) ==NULL){
        wblprintf(LOG_CRITICAL,"RULESET","Not enough memory\n");
        exit(EXIT_FAILURE);
    }
    strcpy(def->expresion,expresion);

    def->dependant_rules=parse_dependant_rules(expresion);
    def->status=VALID;
    r->def=def;
    
    int *rulepos;
    for(i=0;i<def->dependant_rules->size;i++){
        if( hashmap_get(rules->map,(char *)def->dependant_rules->v[i],(any_t *)&rulepos)==MAP_MISSING &&
            hashmap_get(rules->meta_map,(char *)def->dependant_rules->v[i],(any_t *)&rulepos)==MAP_MISSING )
        {
            wblprintf(LOG_WARNING,"RULESET","Dependant rule '%s' not exists. Ignoring meta rule '%s'\n",
                      (char *)def->dependant_rules->v[i],get_rulename(r));
            
            ((meta_definition *)r->def)->status=INVALID;
            break;
        }
    }
    
    if( ((meta_definition *)r->def)->status!=INVALID ){
        for(i=0;i<def->dependant_rules->size;i++){
            (hashmap_get(rules->dependant_map,(char *)(def->dependant_rules->v[i]),(any_t *)&rulepos)==MAP_MISSING)?
                (hashmap_put(rules->dependant_map,(char *)def->dependant_rules->v[i],rulepos)):
                (0);
        }
        r->def=def;
    }
}

void set_definition(rule *r, const char *defName, const char * defParam){ 
    if(((definition *)r->def)!=NULL){
        free(((definition *)r->def)->name);
        if(((definition *)r->def)->param!=NULL)
            free(((definition *)r->def)->param);
        if(((definition *)r->def)->pointer!=NULL)
            free(((definition *)r->def)->pointer);
        free(r->def);
    }
    
    definition *def=(definition *)malloc(sizeof(definition));
    
    //rules->rules[ruleno].def=(definition *)malloc(sizeof(definition));
    def->tflags = NULL;
    def->name = NULL;
    def->pointer = NULL;
    def->param = NULL;
    def->plugin = NULL;
    
    if(def == NULL){
        wblprintf(LOG_CRITICAL,"RULESET","Not enough memory\n");
        exit(EXIT_FAILURE);
    }
    
    def->name=(char *)malloc((strlen(defName)+1)*sizeof(char));
    if(def->name == NULL){
        wblprintf(LOG_CRITICAL,"RULESET","Not enough memory\n");
        exit(EXIT_FAILURE);
    }

    strcpy(def->name,defName);
    //rules->rules[ruleno].def->pointer=NULL;

    if(defParam != NULL){
        def->param=(char *)malloc((strlen(defParam)+1)*sizeof(char));
        if(def->param == NULL){
            wblprintf(LOG_CRITICAL,"RULESET","Not enough memory\n");
            exit(EXIT_FAILURE);
        }
        strcpy(def->param,defParam);
    }else def->param = NULL;
    
    r->def=def;
}

/*Set the rule name definition.*/
void set_definition_name(rule *r, const char * newName){
    //primero libero el puntero para no tener basura.
    if(((definition *)r->def)->name != NULL)
        free(((definition *)r->def)->name);

    ((definition *)r->def)->name = malloc((1+strlen(newName)*sizeof(char)));

    if(((definition *)r->def)->name == NULL){
        wblprintf(LOG_CRITICAL,"RULESET","Not enough memory\n");
        exit(EXIT_FAILURE);
    }
    strcpy(((definition *)r->def)->name,newName);
}

void set_definition_param(rule *r, const char * newParam){
    
    if(((definition *)r->def)->param!=NULL);
        free(((definition *)r->def)->param);
    
    ((definition *)r->def)->param=malloc((1+strlen(newParam)*sizeof(char)));

    if(((definition *)r->def)->param){
        wblprintf(LOG_CRITICAL,"RULESET","Not enough memory\n");
        ((definition *)r->def)->param = NULL;
    }else  strcpy(((definition *)r->def)->param,newParam);
}

void set_definition_pointer(rule *r, function_t *newPointer){

    if(r->def==NULL){
        wblprintf(LOG_CRITICAL,"RULESET","Rule definition does not exist\n");
        r->def = NULL;
    }else ((definition *)r->def)->pointer=newPointer;
}

void set_tflags(rule *r, const char *newFlags){

    if(((definition *)r->def)->tflags!=NULL) free(((definition *)r->def)->tflags);

    ((definition *)r->def)->tflags=malloc((1+strlen(newFlags))*sizeof(char));

    if(((definition *)r->def)->tflags==NULL){
        wblprintf(LOG_CRITICAL,"RULESET","Not enough memory\n");
        ((definition *)r->def)->tflags=NULL;
    }else strcpy(((definition *)r->def)->tflags,newFlags);
}

void set_parser(rule *r, const char *parserName){

    if(r->par!=NULL){
        if(r->par->parserName!=NULL)
            free(r->par->parserName);
        if(r->par->parserPointer!=NULL)
            free(r->par->parserPointer);
    }

    r->par=(parser *)malloc(sizeof(parser));

    if(r->par==NULL){
        wblprintf(LOG_CRITICAL,"RULESET","Not enough memory\n");
        r->par = NULL;
    }
    
    r->par->parserName=(char *)malloc((strlen(parserName)+1)*sizeof(char));

    if(r->par->parserName==NULL){
        wblprintf(LOG_CRITICAL,"RULESET","Not enough memory\n");
    }
    strcpy(r->par->parserName,parserName);
    
    r->par->parserPointer=NULL;
}

void set_parser_name(rule *r, const char *newParserName){

    if(r->par->parserName!=NULL)
        free(r->par->parserName);

    r->par->parserName=malloc((1+strlen(newParserName))*sizeof(char));

    if(r->par->parserName==NULL){
        wblprintf(LOG_CRITICAL,"RULESET","Not enough memory\n");
        r->par->parserName = NULL;
    }
    else strcpy(r->par->parserName,newParserName);
}

void set_parser_pointer(rule *r, parser_t *newParserPointer){
    if(r->par==NULL){
        wblprintf(LOG_CRITICAL,"RULESET","Rule parser does not exist\n");
        r->par = NULL;
    }else r->par->parserPointer=newParserPointer;
}

void set_description(rule *r, const char* desc){

    if (r->description!=NULL)
        free(r->description);
    r->description=malloc(sizeof(char)*(strlen(desc)+1));

    if(r->description==NULL){
        wblprintf(LOG_CRITICAL,"RULESET","Not enough memory\n");
        r->description = NULL;
    }else strcpy(r->description,desc);
}

void set_rulename(rule *r, const char* rulename){

    if (r->rulename!=NULL) free (r->rulename);

    r->rulename=malloc(sizeof(char)*(strlen(rulename)+1));

    if(r->rulename==NULL){
        wblprintf(LOG_CRITICAL,"RULESET","Not enough memory");
        r->rulename = NULL;
    }else strcpy(r->rulename,rulename);
}

//Set the number of times the rule is triggered
void set_num_executions_attr(attributes *attr, long int executions){
    if( attr == NULL ){
        attr = malloc(sizeof(attributes));
    }

    attr->num_executions=executions;
}

void set_num_fails_attr(attributes *attr, long int fails){
    if( attr == NULL ){
        attr = malloc(sizeof(attributes));
    }

    attr->num_fails=fails;
}

//Set the number of time the rule has matched an email succesfully
void set_num_success_attr(attributes *attr, long int success){
    if( attr == NULL ){
        attr = malloc(sizeof(attributes));
    }
    
    attr->num_success=success;
}

//Set the confidence of a rule
void set_confidence_attr(attributes *attr, double confidence){
    if( attr == NULL ){
        attr = malloc(sizeof(attributes));
    }
    
    attr->confidence=confidence;
}

//get the significance of a rule.
void set_significance_attr(attributes *attr, double significance){
    if( attr == NULL ){
        attr = malloc(sizeof(attributes));
    }
    attr->significance=significance;
}

void set_attributes(rule *r, long int executions, long int success, 
                    long int fails, double confidence, double significance)
{
    if ((r->attr = malloc(sizeof(attributes))) == NULL){
        wblprintf(LOG_CRITICAL,"RULESET","Error, Insuficient memory to allocate rule attributes.");
    }else{
        r->attr->num_executions = executions;
        r->attr->num_success = success;
        r->attr->num_fails= fails;
        r->attr->confidence = confidence;
        r->attr->significance= significance;
    }
}

char *get_description(const rule *r){
    return r->description;
}

vector *get_dependant_rules(const rule *r){
    if(((meta_definition *)r->def)==NULL) 
        return NULL;
    else return ((meta_definition *)r->def)->dependant_rules;
}

/*Get the rule index in the hash_map*/
int get_rule_index(const ruleset *rules, char* rulename){
   int *ruleno; //The rulenum
   int  error; //Posible error

   //Lookup the rule num
   error=hashmap_get(rules->map, rulename, (any_t *)&ruleno);
   if (error==MAP_MISSING || ruleno==NULL) return -1;

   return *ruleno;
}

rule *get_rule_by_name(const ruleset *rules, char* rulename){
   int *ruleno; //The rulenum
   int  error; //Posible error

   //Lookup the rule num
   error=hashmap_get(rules->map, rulename, (any_t *)&ruleno);
   if (error==MAP_MISSING || ruleno==NULL) return NULL;

   return &(rules->rules[*ruleno]);
}

int get_meta_index(const ruleset *rules, char* rulename){
   int *ruleno; //The rulenum
   int  error; //Posible error

   //Lookup the rule num
   error=hashmap_get(rules->meta_map, rulename, (any_t *)&ruleno);
   if (error==MAP_MISSING || ruleno==NULL) return -1;

   return *ruleno;
}

rule *get_meta_by_name(const ruleset *rules, char* rulename){
   int *ruleno; //The rulenum
   int  error; //Posible error

   //Lookup the rule num
   error=hashmap_get(rules->meta_map, rulename, (any_t *)&ruleno);
   if (error==MAP_MISSING || ruleno==NULL) return NULL;

   return &(rules->rules[*ruleno]);
}

void set_debug_mode(int debug){
    debug_mode=debug;
}

/* Update the internal map of a ruleset */
void update_map(ruleset *r){
   int rulenum;
   int *pos;

   //update the map in order to find rules by name
   r->map=hashmap_new();
   for (rulenum=0;rulenum<r->size;rulenum++){
       pos=(int *)malloc(sizeof(int));
       *pos=rulenum;
       hashmap_put(r->map,r->rules[rulenum].rulename,pos);
   }
}


/*
void set_definition_plugin(ruleset *rules, const int ruleno, const char *newPlugin, float cpu_load, float io_load, float time){
    
    if(((definition *)rules->rules[ruleno].def)->plugin != NULL){
        if(((definition *)rules->rules[ruleno].def)->plugin->name != NULL)
            free(((definition *)rules->rules[ruleno].def)->plugin->name);
        free(((definition *)rules->rules[ruleno].def)->plugin);
    }
    
    ((definition *)rules->rules[ruleno].def)->plugin = malloc(sizeof(p_info));
    
    ((definition *)rules->rules[ruleno].def)->plugin->name = strdup(newPlugin);
    ((definition *)rules->rules[ruleno].def)->plugin->cpu_load = cpu_load;
    ((definition *)rules->rules[ruleno].def)->plugin->io_load = io_load;
    
    if(rules->rules[ruleno].characteristic & NORMAL_SCORE && (cpu_load>=0.0 && io_load>=0.0) )
        ( (definition *)rules->rules[ruleno].def )->plugin->average_load =  fabs((*((float *)rules->rules[ruleno].score))) / ((cpu_load + io_load) / 2) ;
    else ( (definition *)rules->rules[ruleno].def )->plugin->average_load = 0.00;
    
    ((definition *)rules->rules[ruleno].def)->plugin->execution_time = time;
}
*/

void set_definition_plugin(rule *r, const char *newPlugin, float cpu_load, float io_load, float time){
    
    if(((definition *)r->def)->plugin != NULL){
        if(((definition *)r->def)->plugin->name != NULL)
            free(((definition *)r->def)->plugin->name);
        free(((definition *)r->def)->plugin);
    }
    
    ((definition *)r->def)->plugin = malloc(sizeof(p_info));
    
    ((definition *)r->def)->plugin->name = strdup(newPlugin);
    ((definition *)r->def)->plugin->cpu_load = cpu_load;
    ((definition *)r->def)->plugin->io_load = io_load;
    
    if(r->characteristic & NORMAL_SCORE && (cpu_load>=0.0 && io_load>=0.0) )
        ( (definition *)r->def )->plugin->average_load =  fabs((*((float *)r->score))) / ((cpu_load + io_load) / 2) ;
    else ( (definition *)r->def )->plugin->average_load = 0.00;
    
    ((definition *)r->def)->plugin->execution_time = time;
}

//Get the definition name plugin for the rule.
char *get_definition_name_plugin_at(ruleset *rules, const int ruleno){
    return ((definition *)rules->rules[ruleno].def)->plugin->name;
}

char *get_definition_name_plugin(rule *r){
    return ((definition *)r->def)->plugin->name;
}

//Get the definition io overload for the rule
/*
float get_definition_io_load_plugin(const ruleset *rules, const int ruleno){
    if(ruleno>= 0 && ruleno < rules->size){
        return ((definition *)rules->rules[ruleno].def)->plugin->io_load;
    }else return -1.0f;
}
*/
float get_definition_io_load_plugin(const rule *r){
    if(r->def == NULL)
        return -1.0f;
    else return ((definition *)r->def)->plugin->io_load;
}

//Get the definition cpu overload for the rule
/*
float get_definition_cpu_load_plugin(const ruleset *rules, const int ruleno){
    if(ruleno >= 0 && ruleno < rules->size)
        return ((definition *)rules->rules[ruleno].def)->plugin->cpu_load;
    else return -1.0f;
}
*/
float get_definition_cpu_load_plugin(const rule *r ){
    if(r->def == NULL)
        return -1.0f;
    else return ((definition *)r->def)->plugin->cpu_load;
}

//Get the average load for the rule
/*
float get_definition_avg_load_plugin(const ruleset *rules, const int ruleno){
    if(ruleno>=0 && ruleno < rules->size)
        return ((definition *)rules->rules[ruleno].def)->plugin->average_load;
    else return -1.0f;
}
*/
float get_definition_avg_load_plugin(const rule *r){
    if(r->def == NULL)
        return -1.0f;
    else return ((definition *)r->def)->plugin->average_load; 
}

//Get if rule is dependant or not.
/*
int is_dependant_rule(ruleset *rules,const int ruleno){
    int *rulepos;
    return hashmap_get(rules->dependant_map,get_rulename(rules,ruleno),(any_t*)&rulepos)!=MAP_MISSING;
}
*/

int is_dependant_rule(ruleset *rules,const rule *rule){
    int *rulepos;
    return hashmap_get(rules->dependant_map,get_rulename(rule),(any_t*)&rulepos)!=MAP_MISSING;
}

//Get if rule is a valid meta rule.
/*
short is_valid_meta(ruleset *rules, const int ruleno){
    return ( ((meta_definition *)rules->rules[ruleno].def)->status==VALID);
}
*/

short is_valid_meta(rule *r){
    return ( ((meta_definition *)r->def)->status==VALID);
}

//Free allocate memory in vector element.
int free_vector_element(element data){
    free((char *)data);
    return VECTOR_OK;
}

//Swap rules
void swap_rules(ruleset *rules, int posA,int posB){
    rule *temp=(rule *)malloc(sizeof(rule));
    int *p1, *p2;
    
    memcpy(temp,&(rules->rules[posA]),sizeof(rule));
    memcpy(&(rules->rules[posA]),&(rules->rules[posB]),sizeof(rule));
    memcpy(&(rules->rules[posB]),temp,sizeof(rule));
    
    free(temp);
    
    if( ( hashmap_get(rules->map,get_rulename(get_rule(rules,posA)),(any_t *)&p1)==MAP_MISSING &&
          hashmap_get(rules->meta_map,get_rulename(get_rule(rules,posA)),(any_t *)&p1)==MAP_MISSING ) 
        ||
        ( hashmap_get(rules->map,get_rulename(get_rule(rules,posB)),(any_t *)&p2)==MAP_MISSING &&
          hashmap_get(rules->meta_map,get_rulename(get_rule(rules,posB)),(any_t *)&p2)==MAP_MISSING ) 
      ){
          wblprintf(LOG_CRITICAL,"PRESCHEDULE","Rule '%s' or '%s' does not exist. Aborting..\n",
                    get_rulename(get_rule(rules,posA)),get_rulename(get_rule(rules,posB)));   
    }
    *p1=posA;
    *p2=posB;
}

//Generate ramdom vector from ruleset.
random_vector *randomize_vector(ruleset *rules){
    int num_rules = count_rules(rules);
    random_vector *random = malloc(sizeof(random_vector));
    random->vector = malloc(num_rules*sizeof(int));
    random->size = num_rules;
    //int *random = malloc(num_rules*sizeof(int));
    
    int num = 0, dupl = 0;
    int i,j;
    for (i=0;i<num_rules; i++){
        random->vector[i]=-1;
    }
    
    for(i = 0; i < num_rules; i++){
        num = (rand()%num_rules);
        dupl = 0;
        for (j = 0; j <= i; j++){
             if (num == random->vector[j]){
                dupl = 1;
                break;
             }
        }
        if (dupl == 1)
            i--;
        else
            random->vector[i] = num;
    }
    
    return random;
}

//Checks if 2 vectors are equals or not.
int equal_vectors(random_vector *v1, random_vector *v2){
    int equals = 0;
    int i=0;
    
    for(i=0;i< v1->size;i++){
        if( v1->vector[i] == v2->vector[i] ){
            equals++;
        }
    }
    
    return equals == v1->size;
}

//Generate num_filters from ruleset
random_vector **generate_filter(ruleset *rules, int numfilters){
    int i=0;
    int j=0;
    int repeated = 0;
    random_vector **filters = malloc(numfilters * sizeof(random_vector *));
    filters[0]=randomize_vector(rules);
    
    for(i=1;i<numfilters;i++){
        do{
            repeated = 0;
            filters[i]=randomize_vector(rules);
            for(j=0;j<i;j++){
                if( equal_vectors(filters[j],filters[i]) ){
                    repeated = 1;
                    break;
                }
            }
        }
        while(repeated == 1);
    }
    
    return filters;
}

void sort_rules(ruleset *rules, random_vector *vector){
    int i=0;
    for( i=0;i<vector->size;i++){
        if( i != vector->vector[i]){
            swap_rules(rules,i,vector->vector[i]);
        }
    }
}

void create_filter_files(random_vector **filters, int numfilters, ruleset *rules){
    int i=0;

    for(i=0; i< numfilters;i++){
        char *filename = NULL;
        if( (asprintf(&filename,"ramdom_filters/test_filter.cf_%d",i)!=-1) &&
            (filename!=NULL) ){
            sort_rules(rules,filters[i]);
            write_ruleset(filename,rules);
            free(filename);
        }
    }

}

map_t create_rule_hashmap(ruleset *rules,double overload){
    map_t rulemap = NULL;
    
    if(rules != NULL && count_rules(rules)>0){
        int i=0;
        rulemap = hashmap_new();
        for(;i<count_rules(rules);i++){
            rule *actual_rule = get_rule(rules,i);
            mo_rule * ruleinfo = malloc(sizeof(mo_rule));
            ruleinfo->rule_id = strdup(get_rulename(actual_rule));
            ruleinfo->plugin_id = strdup(get_definition_name_plugin(actual_rule));
            ruleinfo->rule_score = *((float *)get_rule_score(actual_rule));
            ruleinfo->rule_estimated_cpu_time = get_definition_cpu_load_plugin(actual_rule);
            ruleinfo->rule_estimated_io_time = get_definition_io_load_plugin(actual_rule);
            ruleinfo->rule_real_cpu_time=0.0f;
            ruleinfo->rule_real_io_time=0.0f;
            ruleinfo->content_lenght = 0;
            ruleinfo->correct_executions = 0;
            hashmap_put(rulemap,ruleinfo->rule_id,(any_t)ruleinfo);
        }
        
        double *global_cpu_time = malloc(sizeof(double));
        *global_cpu_time = 0.0f;
        
        double *global_io_time = malloc(sizeof(double));
        *global_io_time = 0.0f;
        
        int *email_count = malloc(sizeof(int));
        *email_count = 0;
        
        int *io_count = malloc(sizeof(int));
        *io_count = 0;
        
        int *cpu_count = malloc(sizeof(int));
        *cpu_count = 0;
        
        double *total_time = malloc(sizeof(double));
        *total_time = 0.0f;
        
        int *num_exec_rules = malloc(sizeof(int));
        *num_exec_rules = 0;
        
        double *unbalance = malloc(sizeof(double));
        *unbalance = overload;
        
        int *total_rules = malloc(sizeof(int));
        *total_rules = count_rules(rules);
        
        int *num_executed = malloc(sizeof(int));
        *num_executed = 0;
        
        int *executed_sfe = malloc(sizeof(int));
        *executed_sfe = 0;
        
        hashmap_put(rulemap,"#GLOBAL_CPU",(any_t)global_cpu_time);
        hashmap_put(rulemap,"#GLOBAL_IO",(any_t)global_io_time);
        hashmap_put(rulemap,"#CPU_COUNT",(any_t)cpu_count);
        hashmap_put(rulemap,"#IO_COUNT",(any_t)io_count);
        hashmap_put(rulemap,"#EMAIL_COUNT",(any_t)email_count);
        hashmap_put(rulemap,"#TOTAL_TIME",(any_t)total_time);
        hashmap_put(rulemap,"#EXEC_RULES",(any_t)num_exec_rules);
        hashmap_put(rulemap,"#UNBALANCE",(any_t)unbalance);
        hashmap_put(rulemap,"#TOTAL_RULES",(any_t)total_rules);
        hashmap_put(rulemap,"#NUM_EXECUTED",(any_t)num_executed);
        hashmap_put(rulemap,"#EXEC_SFE",(any_t)executed_sfe);
    }
    
    return rulemap;
}

void fprint_prescheduler_second_paper(map_t rulemap, char *prescheduler_name){
    char *filename = NULL;
    if(hashmap_length(rulemap) > 0 && asprintf(&filename,"test_schedulers/%s.out",prescheduler_name) > 0){
        
        FILE *fp = fopen(filename,"w+");

        if(fp){
            //fprintf(fp,"NAME,TOTAL_RULES,AVG(UNEXECUTED),%%(UNEXECUTED),SUM(SFE),%%(SFE),TIME,AVG(TIME),UNBALANCE,EMAILS\n");
            fprintf(fp,"NAME,EXECUTED,NOT EXECUTED,SFE,TIME,BALANCE\n");
            double *total_time = NULL;
            int *total_rules = NULL;
            int *sfe_executions = NULL;
            int *num_executed = NULL;
            int *email_count = NULL;
            double *unbalance = NULL;

            if ( hashmap_get(rulemap,"#TOTAL_TIME",(any_t *)&total_time) != MAP_MISSING &&
                 hashmap_get(rulemap,"#EMAIL_COUNT",(any_t *)&email_count) != MAP_MISSING &&
                 hashmap_get(rulemap,"#EXEC_SFE",(any_t *)&sfe_executions) != MAP_MISSING &&
                 hashmap_get(rulemap,"#TOTAL_RULES",(any_t *)&total_rules) != MAP_MISSING &&
                 hashmap_get(rulemap,"#NUM_EXECUTED",(any_t *)&num_executed) != MAP_MISSING &&
                 hashmap_get(rulemap,"#UNBALANCE",(any_t *)&unbalance) != MAP_MISSING &&     
                 total_time != NULL && total_rules!=NULL && sfe_executions != NULL && 
                 num_executed != NULL  && email_count != NULL && unbalance != NULL 
               ) 
            {
                long int num_rules = *total_rules * *email_count;
                int unexecuted = num_rules - *num_executed;
                
                fprintf(fp,"%s,",prescheduler_name);
                fprintf(fp,"%d,",*num_executed);
                fprintf(fp,"%d,",unexecuted);
                fprintf(fp,"%d,",*sfe_executions);
                fprintf(fp,"%lf,",*total_time);
                fprintf(fp,"%lf\n",*unbalance);
                fflush(fp);
                fclose(fp);
            }
        }
        else wblprintf(LOG_WARNING,"RULESET","Error: Cannot open file for testing preschedulers\n");
    }else wblprintf(LOG_WARNING,"RULESET","Error: Prescheduler info hashmap is empty\n");
}

void print_prescheduler_second_paper(map_t rulemap, char *prescheduler_name){
    printf("NAME,TOTAL_RULES,AVG(UNEXECUTED),%%(UNEXECUTED),SUM(SFE),%%(SFE),SUM(TIME),AVG(TIME),UNBALANCE\n");
    double *total_time = NULL;
    int *total_rules = NULL;
    int *sfe_executions = NULL;
    int *num_executed = NULL;
    int *email_count = NULL;
    double *unbalance = NULL;

    if ( hashmap_get(rulemap,"#TOTAL_TIME",(any_t *)&total_time) != MAP_MISSING &&
         hashmap_get(rulemap,"#EMAIL_COUNT",(any_t *)&email_count) != MAP_MISSING &&
         hashmap_get(rulemap,"#EXEC_SFE",(any_t *)&sfe_executions) != MAP_MISSING &&
         hashmap_get(rulemap,"#TOTAL_RULES",(any_t *)&total_rules) != MAP_MISSING &&
         hashmap_get(rulemap,"#NUM_EXECUTED",(any_t *)&num_executed) != MAP_MISSING &&
         hashmap_get(rulemap,"#UNBALANCE",(any_t *)&unbalance) != MAP_MISSING &&     
         total_time != NULL && total_rules!=NULL && sfe_executions != NULL && 
         num_executed != NULL  && email_count != NULL && unbalance != NULL 
        ) 
    {
        long int num_rules = *total_rules * *email_count;
        int unexecuted = num_rules - *num_executed;
        
        printf("%s,",prescheduler_name);
        printf("%d,",*total_rules);
        printf("%lf,",(double)unexecuted / (*email_count));
        printf("%lf,",(double)unexecuted / (*num_executed) );
        printf("%d,",*sfe_executions);
        printf("%lf,",*sfe_executions / (double) (*email_count));
        printf("%lf,",*total_time);
        printf("%lf,",*total_time/(*email_count));
        printf("%lf\n",*unbalance);
    }
}

void print_debug_prescheduler(map_t rulemap){
    double *total_time = NULL;
    int *sfe_executions = NULL;
    int *num_executed = NULL;
    int *email_count = NULL;
    double *unbalance = NULL;
    
    if ( hashmap_get(rulemap,"#TOTAL_TIME",(any_t *)&total_time) != MAP_MISSING &&
         hashmap_get(rulemap,"#EMAIL_COUNT",(any_t *)&email_count) != MAP_MISSING &&
         hashmap_get(rulemap,"#EXEC_SFE",(any_t *)&sfe_executions) != MAP_MISSING &&
         hashmap_get(rulemap,"#NUM_EXECUTED",(any_t *)&num_executed) != MAP_MISSING &&
         hashmap_get(rulemap,"#UNBALANCE",(any_t *)&unbalance) != MAP_MISSING && 
         total_time != NULL && sfe_executions != NULL && num_executed != NULL && 
         email_count != NULL && unbalance!=NULL
        ) 
    {
        printf("#TIME: %lf\n",*total_time);
        printf("#NUM_EMAILS: %d\n",*email_count);
        printf("#NUM_SFE_EXECUTIONS: %d\n",*sfe_executions);
        printf("#NUM_RULE_EXECUTED: %d\n",*num_executed);
        printf("#UNBALANCE: %lf\n",*unbalance);
    }
}

void fprint_properties_file(map_t rulemap, FILE *output){
    double *total_time = NULL;
    int *executed_rules = NULL;
    double *avg_overload_separation = NULL;
    double *sum_cpu = NULL;
    double *sum_io = NULL;
    int *email_count;
    
    if( hashmap_get(rulemap,"#TOTAL_TIME",(any_t *)&total_time) != MAP_MISSING &&                        
        hashmap_get(rulemap,"#EXEC_RULES",(any_t *)&executed_rules) != MAP_MISSING &&                        
        hashmap_get(rulemap,"#UNBALANCE",(any_t *)&avg_overload_separation) != MAP_MISSING &&    
        hashmap_get(rulemap,"#EMAIL_COUNT",(any_t *)&email_count) != MAP_MISSING && 
        hashmap_get(rulemap,"#GLOBAL_CPU",(any_t *)&sum_cpu) != MAP_MISSING && 
        hashmap_get(rulemap,"#GLOBAL_IO",(any_t *)&sum_io) != MAP_MISSING && 
        total_time != NULL && executed_rules != NULL && email_count != NULL &&
        avg_overload_separation != NULL && sum_io != NULL && sum_cpu !=NULL)
    {
        fprintf(output,"SUM(TOTAL_TIME)=%lf\n",*total_time);
        fprintf(output,"AVG(TOTAL_TIME)=%lf\n",(*total_time)/(*email_count));
        fprintf(output,"AVG(EXEC_RULES)=%lf\n",(*executed_rules)/(double)(*email_count));
        fprintf(output,"AVG(OVERLOAD)=%lf\n",*avg_overload_separation);
        fprintf(output,"TOTAL_TIME-(SUM(CPU)+SUM(IO))=%lf\n",*total_time - (*sum_cpu + *sum_io));
        fflush(output);
    }
    
}

void print_properties_file(map_t rulemap){
    double *total_time = NULL;
    int *executed_rules = NULL;
    double *avg_overload_separation = NULL;
    double *sum_cpu = NULL;
    double *sum_io = NULL;
    int *email_count;
    
    if( hashmap_get(rulemap,"#TOTAL_TIME",(any_t *)&total_time) != MAP_MISSING &&                        
        hashmap_get(rulemap,"#EXEC_RULES",(any_t *)&executed_rules) != MAP_MISSING &&                        
        hashmap_get(rulemap,"#UNBALANCE",(any_t *)&avg_overload_separation) != MAP_MISSING &&    
        hashmap_get(rulemap,"#EMAIL_COUNT",(any_t *)&email_count) != MAP_MISSING && 
        hashmap_get(rulemap,"#GLOBAL_CPU",(any_t *)&sum_cpu) != MAP_MISSING &&
        hashmap_get(rulemap,"#GLOBAL_IO",(any_t *)&sum_io) != MAP_MISSING &&
        total_time != NULL && executed_rules != NULL && email_count != NULL &&
        avg_overload_separation != NULL && sum_io != NULL && sum_cpu !=NULL)
    {
        printf("TOTAL_TIME=%lf\n",*total_time);
        printf("AVG(TOTAL_TIME)=%lf\n",(*total_time)/(*email_count));
        printf("AVG(EXEC_RULES)=%lf\n",(*executed_rules)/(double)(*email_count));
        printf("AVG(OVERLOAD)=%lf\n",*avg_overload_separation);
        printf("TOTAL_TIME-(SUM(CPU)+SUM(IO))=%lf\n",*total_time - (*sum_cpu + *sum_io));
    }
    
}

void increase_sfe_execution(map_t rulemap){
    int *exec_sfe = NULL;
    if( hashmap_get(rulemap,"#EXEC_SFE",(any_t *)&exec_sfe) == MAP_MISSING || 
        exec_sfe == NULL)
    {
        exec_sfe = malloc(sizeof(int));
        *exec_sfe = 1;
    }
    else *exec_sfe = *exec_sfe + 1;
}

void increase_executed_rule(map_t rulemap){
    int *num_executed_rules = NULL;
    if( hashmap_get(rulemap,"#NUM_EXECUTED",(any_t *)&num_executed_rules) == MAP_MISSING || 
        num_executed_rules == NULL)
    {
        num_executed_rules = malloc(sizeof(int));
        *num_executed_rules = 1;
    }
    else *num_executed_rules = *num_executed_rules + 1;
}

void add_total_time(map_t rulemap,struct timeval t_inicio, struct timeval t_final){
    double calculated_total_time = ( (t_final.tv_sec - t_inicio.tv_sec) * 1000 + 
                                     (t_final.tv_usec - t_inicio.tv_usec)/ 1000.0 );
    double *total_time = NULL;
    
    if( hashmap_get(rulemap,"#TOTAL_TIME",(any_t *)&total_time) != MAP_MISSING && 
        total_time != NULL){
        *total_time = *total_time + calculated_total_time;
    }
}

void add_new_email(map_t rulemap){
    int *count_mails = NULL;
    if( hashmap_get(rulemap,"#EMAIL_COUNT",(any_t *)&count_mails)==MAP_MISSING ||
        count_mails == NULL)
    {
        count_mails = malloc(sizeof(int));
        *count_mails = 1;
        hashmap_put(rulemap,"#EMAIL_COUNT",(any_t)count_mails);
    }
    else *count_mails = *count_mails + 1; 
}

int free_rule_hashmap(any_t nullpointer, any_t data, any_t key){
    char *rulename = (char *)key;
    
    if(rulename!=NULL && data!= NULL){
        if( strcmp(rulename,"#GLOBAL_IO") !=0 && strcmp(rulename,"#GLOBAL_CPU") !=0 &&
            strcmp(rulename,"#EMAIL_COUNT") !=0 && strcmp(rulename,"#IO_COUNT") !=0 &&
            strcmp(rulename,"#CPU_COUNT") !=0 && strcmp(rulename,"#TOTAL_TIME") !=0 &&
            strcmp(rulename,"#EXEC_RULES") !=0 && strcmp(rulename,"#UNBALANCE") !=0 &&
            strcmp(rulename,"#EXEC_SFE") !=0 && strcmp(rulename,"#TOTAL_RULES") !=0 &&
            strcmp(rulename,"#NUM_EXECUTED") !=0 && strcmp(rulename,"#SCHEDULE_NAME") != 0
          )
        {
            mo_rule *ruleinfo = (mo_rule *)data;
            free(ruleinfo->plugin_id);
            free(ruleinfo->rule_id);
            free(ruleinfo);
            ruleinfo = NULL;
            rulename=NULL;
        }else{
            free(data);
            data = NULL;
        }
    }
    
    return MAP_OK;
}

void free_mo_hashmap(map_t rulemap){
    if(rulemap!=NULL){
        hashmap_iterate_elements(rulemap,&free_rule_hashmap,NULL);
        hashmap_free(rulemap);
        rulemap = NULL;
    }
}

void add_real_execution_times( map_t rulemap,char *rulename,double cpu_time,
                               double io_time, long int size){
    mo_rule *ruleinfo = NULL;
    double *global_cpu_time = NULL;
    double *global_io_time = NULL;
    int *io_count = NULL;
    int *cpu_count = NULL;
    int *executed_rules = NULL;
    
    if( rulemap != NULL && hashmap_get(rulemap,rulename,(any_t *)&ruleinfo) != MAP_MISSING && 
        ruleinfo !=NULL ){
        ruleinfo->rule_real_cpu_time+=cpu_time;
        ruleinfo->rule_real_io_time+=io_time;
        ruleinfo->content_lenght+=size;
        ruleinfo->correct_executions++;
        
        hashmap_put(rulemap,rulename,(any_t)ruleinfo);
        
        if( hashmap_get(rulemap,"#GLOBAL_CPU",(any_t *)&global_cpu_time) == MAP_MISSING ||
            hashmap_get(rulemap,"#GLOBAL_IO",(any_t *)&global_io_time) == MAP_MISSING ||
            hashmap_get(rulemap,"#CPU_COUNT",(any_t *)&cpu_count) == MAP_MISSING ||
            hashmap_get(rulemap,"#IO_COUNT",(any_t *)&io_count) == MAP_MISSING ||    
            hashmap_get(rulemap,"#EXEC_RULES",(any_t *)&executed_rules) == MAP_MISSING ||
            global_cpu_time == NULL || global_io_time == NULL ||
            io_count == NULL || cpu_count == NULL || executed_rules == NULL)
        {
           global_cpu_time = malloc(sizeof(double));
           *global_cpu_time = ruleinfo->rule_real_cpu_time;
           global_io_time = malloc(sizeof(double));
           *global_io_time = ruleinfo->rule_real_io_time;
           
           io_count = malloc(sizeof(int));
           (io_time != 0.0f)?(*io_count = 1):(*io_count = 0);
           
           cpu_count = malloc(sizeof(int));
           (cpu_time != 0.0f)?(*cpu_count = 1):(*cpu_count = 0);
           
           executed_rules = malloc(sizeof(int));
           *executed_rules = 1;
           
           hashmap_put(rulemap,"#GLOBAL_CPU",(any_t)global_cpu_time);
           hashmap_put(rulemap,"#GLOBAL_IO",(any_t)global_io_time);
           hashmap_put(rulemap,"#CPÛ_COUNT",(any_t)cpu_count);
           hashmap_put(rulemap,"#IO_COUNT",(any_t)io_count);
           hashmap_put(rulemap,"#EXEC_RULES",(any_t)executed_rules);
        }else{
           *global_cpu_time = *global_cpu_time + ruleinfo->rule_real_cpu_time;  
           *global_io_time = *global_io_time + ruleinfo->rule_real_io_time;
           if(io_time != 0.0f)  *io_count = *io_count + 1;
           if(cpu_time != 0.0f) *cpu_count = *cpu_count + 1;
           *executed_rules = *(executed_rules) + 1;
        }
    }else wblprintf(LOG_WARNING,"RULESET","Unable to insert rule real execution time\n");
}