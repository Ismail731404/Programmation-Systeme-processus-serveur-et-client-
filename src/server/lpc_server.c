/***********************************************************
 *  
 * 
 * 
 ********************************************************/
#include "../lib/lpc_utility.h"
#include "../../include/lpc_structure.h"
#include "../../include/lpc_panic.h"



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

        //struct stat bufStat;
        //fstat(, &bufStat);
        //printf("unmap: longeur object %d\n",
        //      (int) bufStat.st_size);
        int result = munmap(mem, 1024);
        return result;
}



int main(int argc, char *argv[]){

	//create shared memory object
        int fd = shm_open("test", O_CREAT | O_RDWR,
                        S_IRUSR | S_IWUSR);
        if(fd == -1)
            PANIC_EXIT("fd shm\n");    

        if(ftruncate(fd, 1024) == -1)
            PANIC_EXIT("ftruncate\n");

	void *mem = lpc_open("test");
	
	
	memset(mem, '0', 3);
	sleep(5);
	write(STDOUT_FILENO, mem, 3);
	printf("\n");
	lpc_close(mem);

	return 0;
}
