#include "../../include/lpc_utility.h"
#include "../../include/lpc_structure.h"
#include "../../include/lpc_functions/lpc_functions.h"
#include "modify_all.h"

/*
functions takes a lpc_string, int and double
and modifies all of them 
*/

int modify_all (void *memory){

    void *data = (void *)((char *)memory + sizeof(header));
    void *ptr;
    int error = 0;
    errno = 0;

    lpc_string *s1 = data;	
    int len = 0;
    char *message1 = malloc(s1->slen);
    strcpy(message1, "salut");
    len = strlen(message1);

    if(len > s1->slen){
        //space in lpc_string not sufficent for message1
        errno = ENOMEM;
        memcpy(&s1->slen, &error, sizeof(int));
    }else{
        memcpy(s1->string, message1, s1->slen);
    }

    //advance pointer to next value in shared memory
    ptr = (void *) ((char *) data + sizeof(lpc_string) + s1->slen);

    //replace int
    int new_int = 42;
    memcpy(ptr, &new_int, sizeof(int));


    //advance pointer to next value in shared memory
    ptr = (void *) ((char *) ptr + sizeof(int));

    //replace double
    double new_double = 2.71828;
    memcpy(ptr, &new_double, sizeof(double));
    
    //in case of error send value of errno instead of 0
    if(errno != 0)
        return errno;

    return 0;
}	
