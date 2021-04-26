/***********************************************************
 *  
 * 
 * 
 ********************************************************/
#include "../lib/lpc_utility.h"
#include "../../include/lpc_structure.h"
#include "../../include/lpc_panic.h"


#include "../../include/Memoire/init_memory.h"


void *lpc_open(const char *name){

	int fd = shm_open(name, O_RDWR, S_IRUSR | S_IWUSR);
        if(fd == -1){
                printf("shm_open\n");
                return NULL;
        }

        struct stat bufStat;
        fstat(fd, &bufStat);
        printf("map: longeur object %d\n",
                (int) bufStat.st_size);

        void *adr = mmap(NULL, bufStat.st_size,
                        PROT_READ | PROT_WRITE,
                        MAP_SHARED, fd, 0);

        if(adr == MAP_FAILED)
            PANIC_EXIT("mmap\n");
        

        return adr;
}


int lpc_close(void *mem){

        int result = munmap(mem, 1024);
        return result;
}


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
	
        //TODO: init function_list

        int code; 
        
        //lock mutex
        if((code = pthread_mutex_lock(&mem->hd.mutex)) != 0){
                printf("error: pthread_mutex_lock\n");
        }
        //wait for cond
        printf("acquired mutex\n");
        while (mem->hd.libre){
                printf("waiting ... \n");
                if((code = pthread_cond_wait(&mem->hd.wcond, &mem->hd.mutex)) != 0){
                        printf("error: pthread_cond_wait\n");
                }
        }
        printf("received notification from client\n");
        void *ptr = memory;
	ptr = (void *) ((char *) memory + sizeof(header));

        printf("%s\n", mem->hd.fun_name);
        
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
        

        //unlock mutex
        printf("acquired mutex and received signal\n");
        if((code = pthread_mutex_unlock(&mem->hd.mutex)) != 0){
                printf("error: pthread_mutex_unlock\n");
        }
        printf("released mutex\n");

	lpc_close(mem);

	return 0;
}
