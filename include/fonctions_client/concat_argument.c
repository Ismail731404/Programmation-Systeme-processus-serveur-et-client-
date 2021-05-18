#include "../../include/lpc_utility.h"
#include "../../include/lpc_structure.h"
#include "../../include/lpc_functions/lpc_functions.h"
#include "concat_argument.h"

int concat_argument(void *memory)
{
    lpc_memory *memorychild = memory;
    void *data = (void *)((char *)memory + sizeof(header));
    
    lpc_string *s1 =  lpc_make_string("NULL", 100);
    char inter[50];
    int i=0;

    
    while(memorychild->hd.types[i]!= NULL){

        switch (memorychild->hd.types[i])
        {
        case 1:
            sprintf(inter,"%d",*((int *)((char *)data + memorychild->hd.offsets[i])) );
            strcat(s1->string," ");
            strcat(s1->string,inter);
            break;
        case 2:
             sprintf(inter,"%f",*((double *)((char *)data + memorychild->hd.offsets[i])) );
             strcat(s1->string," ");
             strcat(s1->string,inter);
            break;
        case 3:
            if(strcmp(s1->string,"NULL")==0)
                  s1 = (lpc_string *)((char *)data + memorychild->hd.offsets[i]);
            else{
                s1->slen += ((lpc_string *)((char *)data + memorychild->hd.offsets[i]))->slen;
                strcat(s1->string," ");
                strcat(s1->string,((lpc_string *)((char *)data + memorychild->hd.offsets[i]))->string);
            }
            break;
        }
        i++;
    }
  
    return 0;
}
