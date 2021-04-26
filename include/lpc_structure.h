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


typedef struct {
        pthread_mutex_t mutex;
        pthread_cond_t rcond;
        pthread_cond_t wcond;
        pid_t pid;              //nécessaire?
        unsigned char libre;

        char fun_name[NAMELEN]; //const ? 
        int return_v;                   
        int err;                        

        int types[20];          //stores order and type of data in the shared memory
                                //1:int, 2:Double, 3:lpc_string
        int length_arr[20];     //stores the length in bytes of each entry
                                //example: [sizeof(int), sizeof(double), sizeof(lpc_string)+slen, ...]
        int offsets[20];        //decalage des different donne
        
        void *address[20];      // stocke les addresse des donnes envoyer par le client
                                //pour mettre a jour une fois le client fini son travailler

} header;


typedef struct {
        header hd;
        void *data;
} lpc_memory;