/***********************************************************
 *  
 * 
 * 
 ********************************************************/
#include "../lib/lpc_utility.h"
#include "../../include/lpc_structure.h"
#include "../../include/lpc_panic.h"



int initialiser_mutex(pthread_mutex_t *pmutex){
        pthread_mutexattr_t mutexattr;
        int code;
        if((code = pthread_mutexattr_init(&mutexattr)) != 0)
                return code;
        if((code = pthread_mutexattr_setpshared(&mutexattr,
                                        PTHREAD_PROCESS_SHARED)) != 0)
                return code;
        code = pthread_mutex_init(pmutex, &mutexattr);
        return code;
}


int initialiser_cond(pthread_cond_t *pcond){
        pthread_condattr_t condattr;
        int code;
        if(( code = pthread_condattr_init(&condattr)) != 0)
                return code;
        if(( code = pthread_condattr_setpshared(&condattr,PTHREAD_PROCESS_SHARED)) != 0)
                return code;
        code = pthread_cond_init(pcond, &condattr) ;
        return code;
}


void *init_shared_memory(const char *name){

        int fd = shm_open(name, O_CREAT | O_RDWR,
                        S_IRUSR | S_IWUSR);
        if(fd == -1){
                printf("fd shm\n");
                return NULL;
        }

        if(ftruncate(fd, 1024) == -1){
                printf("ftruncate\n");
                return NULL;
        }

        struct stat bufStat;
        fstat(fd, &bufStat);
        printf("map: longeur object %d\n",
                (int) bufStat.st_size);

        lpc_memory *mem = mmap(NULL, bufStat.st_size,
                        PROT_READ | PROT_WRITE,
                        MAP_SHARED, fd, 0);

        if(mem == MAP_FAILED){
                printf("mmap\n");
                exit(0);
        }


        //initialise memory
        mem->hd.libre = 1;
        int code;
        code = initialiser_mutex(&mem->hd.mutex);
        if( code != 0 ){
                PANIC_EXIT("mutex\n");
                printf("error: init mutex\n");
        }
        code = initialiser_cond(&mem->hd.rcond);
        if( code != 0 ){
                PANIC_EXIT("rcond\n");
                printf("error: init rcond\n");
        }
        code = initialiser_cond(&mem->hd.wcond);
        if( code != 0 ){
                PANIC_EXIT("wcond\n");
                printf("error: init wcond\n");
        }
        return  mem;
}


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
        lpc_memory *mem = init_shared_memory("test");	
	
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
