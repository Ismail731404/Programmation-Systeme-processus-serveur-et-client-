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



int main(int argc, char *argv[]){

	//create shared memory object
        lpc_memory *mem = init_memory("/test");	
	
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
        union Data *ptr = mem+sizeof(header);
        printf("received notification from client\n");
        printf("%d\n", mem->data_entries[0].i);
        //ptr++;
        //printf("%d\n", ptr->i);

        //unlock mutex
        printf("acquired mutex and received signal\n");
        if((code = pthread_mutex_unlock(&mem->hd.mutex)) != 0){
                printf("error: pthread_mutex_unlock\n");
        }
        printf("released mutex\n");

	lpc_close(mem);

	return 0;
}
