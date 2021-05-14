#include "../lpc_structure.h"
#include "../lpc_utility.h"
#include "../lpc_panic.h"
#include "lpc_functions.h"

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


lpc_string *lpc_make_string(const char *s, int taille){
	
	lpc_string *lpc_s;

	if(taille>0 && s==NULL){
		lpc_s = malloc(taille + sizeof(lpc_string));
		if(lpc_s==NULL){
			printf("lpc_make_string: malloc\n");
			return NULL;
		}
		lpc_s -> slen = taille;
		memset(lpc_s -> string, '0', taille);
	
	}else if(taille<=0 && s!=NULL){
		size_t lo = strlen(s) + 1;
		lpc_s = malloc(lo + sizeof(lpc_string));
		if(lpc_s == NULL){
			printf("lpc_make_string: malloc\n");
			return NULL;
		}
		lpc_s -> slen = lo;
		strcpy(lpc_s -> string, s);

	}else if(taille>=strlen(s)+1){
		lpc_s = malloc(taille + sizeof(lpc_string));
		if(lpc_s == NULL){
			printf("lpc_make_string: malloc\n");
			return NULL;
		}
		lpc_s -> slen = taille;
		strcpy(lpc_s -> string, s);		

	}else{
		return NULL;

	}

	return lpc_s;

}
