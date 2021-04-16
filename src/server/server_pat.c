#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdarg.h>
#include <string.h>


typedef enum {STRING, DOUBLE, INT, NOP} lpc_type;


#define NAMELEN 48
typedef struct{
	char fun_name[NAMELEN];
	int (*fun)(void *);
}lpc_function;


typedef struct {
        int slen;
        char string[];
} lpc_string;



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

        if(adr == MAP_FAILED){
                printf("mmap\n");
                exit(0);
        }

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
        if(fd == -1){
                printf("fd shm\n");
                return -1;
        }

        if(ftruncate(fd, 1024) == -1){
                printf("ftruncate\n");
                return -1;
        }

	void *mem = lpc_open("test");
	
	
	memset(mem, '0', 3);
	sleep(5);
	write(STDOUT_FILENO, mem, 3);
	printf("\n");
	lpc_close(mem);

	return 0;
}
