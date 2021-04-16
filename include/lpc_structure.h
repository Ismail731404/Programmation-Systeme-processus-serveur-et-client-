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
