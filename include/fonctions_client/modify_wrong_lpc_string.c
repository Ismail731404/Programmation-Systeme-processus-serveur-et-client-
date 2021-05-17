#include "../../include/lpc_utility.h"
#include "../../include/lpc_structure.h"
#include "../../include/lpc_functions/lpc_functions.h"
#include "modify_wrong_lpc_string.h"

int modify_wrong_lpc_string (void *data){

    int error = 0;
    int len = 0;
    lpc_string *s1 = data;	

    char *message1 = malloc(s1->slen);
    strcpy(message1, "salut");
    len = strlen(message1);

    if(len > s1->slen){
        memcpy(&s1->slen, &error, sizeof(int));
    }else{
        memcpy(s1->string, message1, s1->slen);
    }


    void *ptr;
    ptr = (void *) ((char *) data + sizeof(lpc_string) + s1->slen);

    lpc_string *s2 = (lpc_string *) ptr;

    printf("%s : %d\n", s2->string, s2->slen);
    char *message2 = malloc(s2->slen);
    strcpy(message2, "too long");
    len = strlen(message2);

    errno = ENOMEM;

    //string too long to fit into lpc_string given by client
    if(1000 > s2->slen){
        memcpy(&s2->slen, &error, sizeof(int));
    }else{
        memcpy(s2->string, message2, s2->slen);
    }
    
    return ENOMEM;
}	
