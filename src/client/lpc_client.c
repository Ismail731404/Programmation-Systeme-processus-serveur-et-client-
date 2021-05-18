/***********************************************************
 *  
 * 
 * 
 ********************************************************/
#include "../../include/lpc_utility.h"
#include "../../include/lpc_panic.h"
#include "../../include/lpc_structure.h"
#include "../../include/lpc_functions/lpc_functions.h"

void lpc_attendre_signal(lpc_memory *mem)
{
	int code;

	if ((code = pthread_mutex_lock(&mem->hd.mutex)) != 0)
	{
		printf("error: pthread_mutex_lock\n");
	}
	if (mem->hd.libre == 0)
	{
		printf("Client: attend une reponse\n");
		if ((code = pthread_cond_wait(&mem->hd.rcond, &mem->hd.mutex)) != 0)
		{
			printf("error: pthread_cond_wait\n");
		}
	}
	if ((code = pthread_mutex_unlock(&mem->hd.mutex)) != 0)
	{
		printf("error: pthread_mutex_unlock\n");
	}
}

int lpc_call(void *memory, const char *fun_name, ...){
	

	printf("clinet : je vais envoyer cette fonction %s\n", fun_name);
	va_list arguments;				//va_list: list of arguments of variable length
	va_start(arguments, fun_name);

	lpc_type fst_param;				//type argument, first(pair): STRING, INT, DOUBLE

	//value arguments, second(pair)
	lpc_string *string_param;			
	double *double_param;
	int *int_param;

	void *ptr = memory;
	ptr = (void *)((char *)memory + sizeof(header));		//pointer to data segment of shared memory

	/* cast to char* necessary to move pointer, then back to void*  */

	int offset = 0; 			//offset to start of data segment in each iteration

	int type; 					//type of data entry
	int size; 					//length in bytes of data entry

	int nop = 1;   				//end of function parameters
	int count = 0; 				//counter of data entries

	int code;
	lpc_memory *mem = (lpc_memory *) memory;

	if ((code = pthread_mutex_lock(&mem->hd.mutex)) != 0)
	{
		printf("error: pthread_mutex_lock\n");
	}

	memcpy(mem->hd.fun_name, fun_name, strlen(fun_name));		//put function name into shared mem.

	while (nop != 0)
	{

		fst_param = (lpc_type)va_arg(arguments, lpc_type);

		/*calculate offset depending on the length of data 
		already in the shared  */

		offset = 0;
		for (int i = 0; i < count; i++)
		{
			offset += mem->hd.length_arr[i];
		}

		switch (fst_param)
		{
		case STRING:
			string_param = (lpc_string *)va_arg(arguments, lpc_string *);
			type = 3; 												// 3: lpc_string
			size = sizeof(lpc_string) + string_param->slen;

			memcpy((char *)ptr + offset, string_param, size);

			mem->hd.types[count] = type;
			mem->hd.length_arr[count] = size;
			mem->hd.address[count] = (lpc_string *)string_param;
			mem->hd.offsets[count] = offset;
			count++;
			break;

		case DOUBLE:
			double_param = (double *)va_arg(arguments, double *);
			type = 2; 												//2: double
			size = sizeof(double);

			memcpy(((char *)ptr + offset), double_param, size);

			mem->hd.types[count] = type;
			mem->hd.length_arr[count] = size;
			mem->hd.address[count] = (double *)double_param;
			mem->hd.offsets[count] = offset;
			count++;
			break;

		case INT:
			int_param = (int *)va_arg(arguments, int *);
			type = 1; 												//1: int
			size = sizeof(int);

			memcpy(((char *)ptr + offset), int_param, size);

			mem->hd.types[count] = type;
			mem->hd.length_arr[count] = size;
			mem->hd.address[count] = (int *)int_param;
			mem->hd.offsets[count] = offset;
			count++;
			break;

		case NOP:
			nop = 0;
			break;

		default:
			printf("could not read arguments\n");
			return -1;
		}
	}
	//set wait condition for server to true
	mem->hd.libre = 0;
	printf("Client: Request processing for a server child\n");
	if ((code = pthread_mutex_unlock(&mem->hd.mutex)) != 0)
	{
		printf("error: pthread_mutex_unlock\n");
	}

	if ((code = pthread_cond_signal(&mem->hd.wcond)) != 0)
	{
		printf("error: pthread_cond_signal wcond\n");
	}

	// il va essaie de revoureille et il pourra pas car c'est le serveur qui a verroue avant lui
	// le but c'est just qu'il va attendre le reponse du serveur

	lpc_attendre_signal(mem);

	lpc_string *client_lpc_string;	//pointer to lpc_string given by client
	lpc_string *server_lpc_string;	//pointer to new lpc_string in shared memory, modified by server

	//recuper les changement du serveur
	for (int i = 0; i < count; i++)
	{
		switch (mem->hd.types[i])
		{
		case 1:
			*(int *)(mem->hd.address[i]) = *((int *)((char *)ptr + mem->hd.offsets[i]));
			break;
		case 2:
			*(double *)(mem->hd.address[i]) = *((double *)((char *)ptr + mem->hd.offsets[i]));
		case 3: 
			client_lpc_string = ((lpc_string *) (mem->hd.address[i]));				//pointer to original variable of the client
			server_lpc_string = ((lpc_string *) ((char *)ptr+mem->hd.offsets[i]));	//modified variable in shared memory
		
			memcpy(client_lpc_string->string, server_lpc_string->string, mem->hd.length_arr[i]);
			memcpy(&client_lpc_string->slen, &server_lpc_string->slen,sizeof(int));
		}
	}

	va_end(arguments);

	//set own errno to errno given by the server
	errno = mem->hd.err;
	printf("%s\n", strerror(errno));
	if (errno != 0)
		return -1;

	return 1;
}

void PremierConnexion(lpc_memory *mem)
{

	//Avertir au serveur vouloir appeller une fonction
	int code;
	if ((code = pthread_mutex_lock(&mem->hd.mutex)) != 0)
	{
		printf("error: pthread_mutex_lock\n");
	}
	//envoyer au server son pid
	mem->hd.pid = getpid();
	mem->hd.libre = 0;

	if ((code = pthread_mutex_unlock(&mem->hd.mutex)) != 0)
	{
		printf("error: pthread_mutex_unlock\n");
	}
	if ((code = pthread_cond_signal(&mem->hd.wcond)) != 0)
	{
		printf("error: pthread_cond_signal wcond\n");
	}

	printf("Client: Request Communication for Server \n");
	//Attendre la reponse du server
	lpc_attendre_signal(mem);
}

int NumeroFonction(char *function)
{
	int choixMenu;

	if (strcmp(function, "add_int") == 0)
		choixMenu = 1;
	else if (strcmp(function, "modify_lpc_string") == 0)
		choixMenu = 2;
	else if (strcmp(function, "modify_wrong_lpc_string") == 0)
		choixMenu = 3;
	else if (strcmp(function, "print_lpc_string") == 0)
		choixMenu = 4;
	else if (strcmp(function, "concat_chaine") == 0)
		choixMenu = 5;
	else if (strcmp(function, "modify_all") == 0)
		choixMenu = 6;	

	return choixMenu;
}

int main(int argc, char **argv)
{

	if (argc != 2)
	{
		fprintf(stderr, "usage : %s shared_memory_object\n", argv[0]);
		exit(1);
	}
	char function[100];
	memcpy(function, argv[1], strlen(argv[1]));

	lpc_memory *mem = lpc_open("/shmo_name");
	char shmo_name_Pid[40];

	PremierConnexion(mem);
	printf("Client: Request Accepted \n");

	//Recuperation la nouvelle Object a partage
	memcpy(shmo_name_Pid, mem->hd.shmo_name_Pid, sizeof(mem->hd.shmo_name_Pid));
	lpc_memory *memchild = lpc_open(shmo_name_Pid);
	memchild->hd.pid = getpid();


	switch (NumeroFonction(function))
	{
	case 1:
		printf("\nClinet veut appelle Fonction add_int\n");
		int a, b, c;
		a = 2;
		c = a;
		b = 5;

		if (lpc_call(memchild, (const char *)function, INT, &a, INT, &b, NOP) == 1)
			printf("La somme de %d et %d equale: %d\n", c, b, a);

		break;
	case 2:
		printf("\nClinet veut appelle Fonction modify_lpc_string\n");

		lpc_string *s1 = lpc_make_string("not", 100);
		lpc_string *s2 = lpc_make_string("not modified 2", 50);
		if (lpc_call(memchild, (const char *)function, STRING, s1, STRING, s2, NOP) == 1)
		{
			printf("La valeur de s1 est modife par le server: %s\n", s1->string);
			printf("La valeur de s2 est modife par le server: %s\n", s2->string);
		}
		break;
	case 3:
		printf("\nClinet veut appelle Fonction modify_wrong_lpc_string\n");
		lpc_string *s3; 
		if((s3 = lpc_make_string("not modified 1", 3)) == NULL){
			return 0;
		}

		lpc_string *s4;
		if((s4 = lpc_make_string("not modified 2", 100)) == NULL){
			return 0;
		}

		if ((lpc_call(memchild, (const char *)function, STRING, s3, STRING, s4, NOP)) == 1)
		{
			printf("La valeur de s1 est modife par le server: %s\n", s3->string);
			printf("La valeur de s2 est modife par le server: %s\n", s4->string);
		}else{
			if (s4->slen == 0)
			{
				printf("s2 not modified, buffer not big enough\n");
				printf("value of errno %s\n", strerror(errno));
			}
		}
		break;
	case 4:
		printf("\nClient veut appeller Fonction print_lpc_string\n");
		lpc_string *s; 
		if((s = lpc_make_string("Salut!", 20)) == NULL){
			return 0;
		}
		lpc_call(memchild, (const char *)function, STRING, s, NOP);
		break;
	case 5:
		printf("\nClinet veut appelle Fonction concat deux chaine \n");
		break;
	case 6:
		printf("\nClient veut appeller Fonction modify_all\n");
		lpc_string *new_s; 
		if((new_s = lpc_make_string("pas modifiée!", 20)) == NULL){
			printf("buffer for lpc_string too short\n");
			return 0;
		}
		int solution = 24;
		double e = 3.14;
		if((lpc_call(memchild, (const char *)function, STRING, new_s, INT, &solution, DOUBLE, &e, NOP)) != 1){
			printf("%s\n", strerror(errno));
		}
		printf("La valeur de new_s est modife par le server: %s\n", new_s->string);
		printf("La valeur de solution est modife par le server: %d\n", solution);
		printf("La valeur de e est modife par le server: %f\n", e);
		break;	
	default:
		printf("\nClient: fonction n'exsite pas\n");
		lpc_call(memchild, (const char *)function, NOP);
		break;
	}

	//clode les memoire projecter
	lpc_close(mem);
	lpc_close(memchild);
	return 0;
}