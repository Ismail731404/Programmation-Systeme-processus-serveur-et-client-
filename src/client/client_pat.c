#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdarg.h>
#include <string.h>
#include <pthread.h>


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


union Data{
	int i;
	double d;
	lpc_string s;
};


typedef struct {
	pthread_mutex_t mutex;
	pthread_cond_t rcond;
	pthread_cond_t wcond;
	pid_t pid;

} lpc_memory;

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
	//	(int) bufStat.st_size);
	int result = munmap(mem, 1024);
	return result;
}


int lpc_call(void *memory, const char *fun_name, ...){
	
	va_list arguments;
	va_start(arguments, fun_name);

	lpc_type fst_param;
	lpc_string *string_param;
	double *double_param;
	int *int_param;
	
	union Data *data;
	int nop = 1;
	int count = 0;

	while(nop != 0){
		
		data = malloc(sizeof(union Data));
		fst_param = (lpc_type) va_arg (arguments, lpc_type);

		switch(fst_param){
			case STRING:
				string_param = (lpc_string*) va_arg (arguments, lpc_string*);
				data->s = *string_param;		
				printf("string_param string: %s\n", string_param->string);
				printf("string_param slen: %d\n", string_param->slen);	
				memcpy((union Data*) memory+count, 
						string_param,
					       	sizeof(lpc_string)+string_param->slen);
				count ++;		
				break;

			case DOUBLE:
				double_param = (double*) va_arg (arguments, double*);
				data->d = *double_param;
				memcpy((union Data*) memory+count, double_param, sizeof(union Data));
				count ++;
				break;

			case INT:

				int_param = (int*) va_arg (arguments, int*);
				data->i = *int_param;
				memcpy((union Data*) memory+count, int_param, sizeof(union Data));	
				count ++;
				break;

			case NOP:
				nop = 0;
				break;

			default:
				printf("could not read arguments\n");
				return -1;
		}

	}

	va_end(arguments);
	return 1;
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



int main(int argc, char *argv[]){
	
	// serveur
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
	

	//client
	void *mem = lpc_open("test");
	
	int a = 1;
	int b = 2;
	double c = 3;
	double d = 4;
	lpc_string *s = lpc_make_string("bonjour", 100);
	lpc_string *s1 = lpc_make_string("hi1", 100);
	lpc_string *s2 = lpc_make_string("hi3", 100);
	//printf("size: %ld\n", sizeof(union Data));
	//printf("size s: %ld\n", sizeof(*s));
	//int r = lpc_call(mem, "fun_difficile", STRING, s, NOP);
	int r = lpc_call(mem, "fun_difficile", INT, &a, DOUBLE, &c, INT, &b, STRING, s, DOUBLE, &d, NOP);
	
	printf("%d\n", lpc_close(mem));
	return 0;
}
