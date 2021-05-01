/***********************************************************
 *  
 * 
 * 
 ********************************************************/
#include "../../include/lpc_utility.h"
#include "../../include/lpc_structure.h"
#include "../../include/lpc_panic.h"
#include "../../include/Memoire/init_memory.h"
#include "../../include/lpc_functions/lpc_functions.h"
#include "libfonctionsclient.h"


/* function that takes a name and a lpc_fun list 
returns a function pointer to a function that takes void* and returns int
-> finds server function corresponding to name */

int (*find_fun(char *name, lpc_function fun_list[20])) (void*){
        for(int i=0; i<20; i++){
                if(strcmp(name, fun_list[i].fun_name) == 0){
                        return fun_list[i].fun;
                }
        }
        return NULL;
}



int main(int argc, char *argv[]){


	//create shared memory object
        void *memory = init_memory("/test");
        lpc_memory *mem = memory;	

        //array that contains all executable functions of the server
        lpc_function function_list [20];   

        //add client functions
        lpc_function fun0 = {"add_int", &add_int};
        function_list[0] = fun0;

        lpc_function fun1 = {"print_lpc_string", &print_lpc_string};
        function_list[1] = fun1;

        lpc_function fun2 = {"modify_lpc_string", &modify_lpc_string};
        function_list[2] = fun2;



        int code; 
        
        //lock mutex
        if((code = pthread_mutex_lock(&mem->hd.mutex)) != 0){
                printf("error: pthread_mutex_lock\n");
        }
        printf("acquired mutex\n");


        //wait for cond       
        while (mem->hd.libre){
                printf("waiting ... \n");
                if((code = pthread_cond_wait(&mem->hd.wcond, &mem->hd.mutex)) != 0){
                        printf("error: pthread_cond_wait\n");
                }
        }
        printf("received notification from client\n");


        void *ptr = memory;
	ptr = (void *) ((char *) memory + sizeof(header));              //pointer to DATA segment of the shared memory

        int result = find_fun(mem->hd.fun_name, function_list)(ptr);    //exectue function specified by client
        mem->hd.return_v = result;


        /* for testing purposes
        for(int i=0;i<4;i++)
	{
		switch(mem->hd.types[i]){
			case 1:
                                printf("%d: %d\n", mem->hd.types[i], *((int *) ((char *) ptr+mem->hd.offsets[i])));
                                *((int *) ((char *) ptr+mem->hd.offsets[i]))+=1;	
				break;
			case 2:
                                 printf("%d: %f\n", mem->hd.types[i], *((double *) ((char *) ptr+mem->hd.offsets[i])));
                                 *((double *) ((char *) ptr+mem->hd.offsets[i]))+=1;
                                 break;
			case 3: 
                                 printf("%d: %s\n", mem->hd.types[i], ((lpc_string *) ((char *) ptr+mem->hd.offsets[i]))->string);
                                 break;               

		}
	}
        */

        //unlock mutex
        printf("acquired mutex and received signal\n");
        if((code = pthread_mutex_unlock(&mem->hd.mutex)) != 0){
                printf("error: pthread_mutex_unlock\n");
        }
        printf("released mutex\n");

        if((code = pthread_cond_signal(&mem->hd.rcond)) != 0){
                printf("error: pthread_cond_signal wcond\n");
        }
        printf("notified client\n");
        
	lpc_close(mem);

	return 0;
}
