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
        pid_t pid;
        unsigned char libre;
        int return_v;
        int err;        
        int types[20];           //stores order and type of data in the shared memory
                                 //1:int, 2:Double, 3:lpc_string
        int length_arr[20];      //stores the length in bytes of each entry
                                 //example: [sizeof(int), sizeof(double), sizeof(lpc_string)+slen, ...]
} header;


typedef struct {
        header hd;
        void *data;
} lpc_memory;