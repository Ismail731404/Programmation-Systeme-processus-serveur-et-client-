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

int lpc_call(void *memory, const char *fun_name, ...)
{

	printf("clinet : je vais envoyer cette fonction %s\n", fun_name);
	va_list arguments;
	va_start(arguments, fun_name);

	lpc_type fst_param;

	lpc_string *string_param;
	double *double_param;
	int *int_param;

	void *ptr = memory;
	ptr = (void *)((char *)memory + sizeof(header));

	/* cast to char* necessary to move pointer, then back to void*   */

	int offset = 0; //offset to start of data segment in each iteration

	int type; //type of data entry
	int size; //length in bytes of data entry

	int nop = 1;   //end of function parameters
	int count = 0; //counter of data entries

	int code;
	lpc_memory *mem = (lpc_memory *)memory;

	if ((code = pthread_mutex_lock(&mem->hd.mutex)) != 0)
	{
		printf("error: pthread_mutex_lock\n");
	}
	memcpy(mem->hd.fun_name, fun_name, strlen(fun_name));
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
			type = 3; // 3: lpc_string
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
			type = 2; //2: double
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
			type = 1; //1: int
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
	//make condition come true
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

	//not yet perfect, needs rework!!
	//!!
	//!! ->

	//il va essaie de revoureille et il pourra pas car c'est le serveur qui a verroue avant lui

	// le but c'est just qu'il va attendre le reponse du serveur

	lpc_attendre_signal(mem);
	
	if(mem->hd.return_v != -1){
		char *target_string;	//pointer to string given by client
		char *origin_string;	//pointer to new string in shared memory, modified by serve

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
					target_string = ((lpc_string *) (mem->hd.address[i]))->string;
					origin_string = ((lpc_string *) ptr+mem->hd.offsets[i])->string;
					memcpy(target_string, origin_string, mem->hd.length_arr[i]);	
			}
		}
		va_end(arguments);
		return 1;
	}else{
		int err = mem->hd.err;
		printf("fonctione échouché, valeur de errno: %d\n", err);
		va_end(arguments);
		return err;
	}
}


int main(int argc, char *argv[])
{

	int a = 2;
	int b = 5;


	if (argc != 2)
	{
		fprintf(stderr, "usage : %s shared_memory_object\n", argv[0]);
		exit(1);
	}

	lpc_memory *mem = lpc_open("/shmo_name");
	char shmo_name_Pid[20];

	lpc_string *s1 = lpc_make_string("hi1", 100);
	char *function;
	function = malloc(sizeof(char) * 20);
	memcpy(function, argv[1], strlen(argv[1]));

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
    printf("Client: Request Accepted \n");
	//Recuperation la nouvelle Object a partage
	memcpy(shmo_name_Pid, mem->hd.shmo_name_Pid, sizeof(mem->hd.shmo_name_Pid));

	lpc_memory *memchild = lpc_open(shmo_name_Pid);

	memchild->hd.pid = getpid();

	if(lpc_call(memchild, (const char *)function, INT, &a, INT, &b, NOP) == 1){
		printf("La valeur de b est modife par le server a=%d\n", a);
	}
	
	//clode les memoire projecter
	lpc_close(mem);
	lpc_close(memchild);
	return 0;
}