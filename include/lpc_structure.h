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
        unsigned char libre;
        int return_v;
        int err;
} header;


typedef struct {
        header hd;
        union Data data_entries[7];
} lpc_memory;