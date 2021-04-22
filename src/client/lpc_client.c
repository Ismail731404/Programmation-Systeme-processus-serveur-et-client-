/***********************************************************
 *  
 * 
 * 
 ********************************************************/
#include "../lib/lpc_utility.h"
#include "../../include/lpc_panic.h"
#include "../../include/lpc_structure.h"



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


int lpc_call(void *memory, const char *fun_name, ...){
	
	va_list arguments;
	va_start(arguments, fun_name);

	lpc_type fst_param;

	lpc_string *string_param;
	double *double_param;
	int *int_param;
	
	void *ptr = memory;
	ptr = (void *) ((char *) memory + sizeof(header)); 

	/* cast to char* necessary to move pointer, then back to void*   */

	int offset = 0; 	//offset to start of data segment in each iteration

	int type;			//type of data entry
	int size;			//length in bytes of data entry

	int nop = 1;		//end of function parameters
	int count = 0;		//counter of data entries

	int code;
	lpc_memory *mem = (lpc_memory*) memory;

	if((code = pthread_mutex_lock(&mem->hd.mutex)) != 0){
		printf("error: pthread_mutex_lock\n");
	}
	printf("acquired mutex\n");

	while(nop != 0){
		
		fst_param = (lpc_type) va_arg (arguments, lpc_type);
		

		/*calculate offset depending on the length of data 
		already in the shared  */

		offset = 0; 
		for(int i=0; i<count; i++){
			offset += mem->hd.length_arr[i];
		}

		switch(fst_param){
			case STRING:
				string_param = (lpc_string*) va_arg (arguments, lpc_string*);
				type = 3;		// 3: lpc_string
				size = sizeof(lpc_string) + string_param->slen;

				memcpy((char *)ptr + offset, string_param, size);

				mem->hd.types[count] = type; 
				mem->hd.length_arr[count] = size;
				count ++;		
				break;

			case DOUBLE:
				double_param = (double*) va_arg (arguments, double*);
				type = 2;		//2: double
				size = sizeof(double);
			
				memcpy(((char *)ptr + offset), double_param, size);
	
				mem->hd.types[count] = type; 
				mem->hd.length_arr[count] = size;
				count ++;
				break;

			case INT:
				int_param = (int*) va_arg (arguments, int*);
				type = 1;		//1: int
				size = sizeof(int);

				memcpy(((char *)ptr + offset), int_param, size);
	
				mem->hd.types[count] = type; 
				mem->hd.length_arr[count] = size;
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
	//make condition come true
	mem->hd.libre = 0;

    if((code = pthread_mutex_unlock(&mem->hd.mutex)) != 0){
		printf("error: pthread_mutex_unlock\n");
	}
	printf("released mutex\n");
	if((code = pthread_cond_signal(&mem->hd.wcond)) != 0){
		printf("error: pthread_cond_signal wcond\n");
	}
	printf("notified server\n");

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
	lpc_memory *mem = lpc_open("test");
	
	//int a = 1;
	int b = 2;
	//double c = 3;
	double d = 4;
	lpc_string *s = lpc_make_string("bonjour", 100);
	lpc_string *s1 = lpc_make_string("hi1", 100);
	//lpc_string *s2 = lpc_make_string("hi3", 100);
	//printf("size: %ld\n", sizeof(union Data));
	//printf("size s: %ld\n", sizeof(*s));
	//int r = lpc_call(mem, "fun_difficile", STRING, s, NOP);
	//int r = lpc_call(mem, "fun_difficile", INT, &a, DOUBLE, &c, INT, &b, DOUBLE, &d, NOP);
	lpc_call(mem, "fun_difficile", INT, &b, STRING, s,  DOUBLE, &d, STRING, s1, NOP);

	
	printf("%d\n", lpc_close(mem));
	return 0;
}
