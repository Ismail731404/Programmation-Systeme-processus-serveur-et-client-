#include "../lib/lpc_utility.h"

#define err(msg) \
    do { perror(msg); exit(EXIT_FAILURE); } while (0)

typedef enum {STRING, DOUBLE, INT, NOP} lpc_type;

typedef struct {
 int slen;
 char string[];
} lpc_string;


void *lpc_open(const char *name){

   int fd;
   struct stat buf;
   off_t len = 2048;
   void *mm;
   if((fd=shm_open(name,O_RDWR,0))==-1){
         printf("failed sh_open");
         return NULL;
   }
   if(fstat(fd,&buf)<0){
         printf("error fstat");
         return NULL;
   }
   
   /*auparavant le fichier etais 1024 et occupe par la fonction on 
   * augment pour stocke plus de chose
   */
   if(ftruncate( fd, len) == -1){
       perror("ftruncate"); exit(1);
   }
   /**
     *PROT_EXEC : Pour execute le contenu partage
   */
   if((mm=mmap(NULL,buf.st_size,PROT_WRITE|PROT_EXEC,MAP_SHARED,fd,0))==MAP_FAILED) {
         printf("mmap Failed");
         return NULL;
   }
   
   return mm;
}


int lpc_close( void *mem ){
    
  int retur = munmap(mem,1024);
  //Si on declare une variable global qui va contenir la taille de fichier pour supprime 
  //la totalite de cette memoire 
  return retur;
}

lpc_string *lpc_make_string( const char *s, int taille ){
   
     lpc_string *lpc_s;
     if(taille > 0 && s == NULL){
       lpc_s = malloc( sizeof( lpc_string ) + taille );
       lpc_s->slen = taille;
       memset(lpc_s->string,NULL, taille);
     }
     else if(taille <=0 && s != NULL){
        taille = strlen(s)+1; 
        lpc_s = malloc( sizeof( lpc_string ) + taille);
        lpc_s->slen = taille;
        strcpy( lpc_s->string, s );
     }
     else if(taille >= strlen(s)){
        lpc_s = malloc( sizeof( lpc_string ) + taille);
        lpc_s->slen = taille;
        strcpy( lpc_s->string, s);
     }
     else{
          return NULL;
     }
      
  return lpc_s;
}



double lpc_call( void *memory, const char *fun_name, ...){
     
    /*
    * les variable transmi devrait etre pointeur int* double* 
    * reflechie comment on pourra ressoudre si c'est le cas 
    */
     va_list ap;
     int *a;double *b; lpc_string *s; 
     va_start(ap, fun_name);
     lpc_type type = va_arg(ap, lpc_type);
	   do
	   {
		switch (type)
		{
			case STRING:
			    s = va_arg(ap, lpc_string *);
			    //memcpy(memory, s ,sizeof( lpc_string ) + s->slen);
			    type = va_arg(ap,lpc_type);
			    break;
			    
			case INT:
			    a = va_arg(ap, int *);
			    memcpy(((char*)memory)+1025, a ,sizeof(int));
			    type = va_arg(ap,lpc_type);
			    break;
			
			case DOUBLE:
			    b = va_arg(ap, double *);
			    //memcpy(memory, b ,sizeof(b));
			    type = va_arg(ap,lpc_type);
			    break;
			case  NOP:
			    break;
		}
	       
	    } while(type != NOP);

     va_end(ap);
      
     double (*function)(int*, double*) = memory;
     double res = function(&a,&b);
     
     return res;     
}
int main(){
      
	printf("je suis le client");
	void *mm = lpc_open("/monobject");
	//printf("la somme 10+30.2 = %f",lpc_call(mm,"add",INT,10,DOUBLE,30.2,NOP));

	
	int a = 15;
	double b = 0;

	 /* on alloue un objet lpc_string avec un tableau string de 100 octets. C’est
	 * beaucoup plus que ce qu’il faut pour stocker "bonjour". En prévoit que
	 * fun_difficile remplacera "bonjour" par un autre string, bien plus long,
	 * jusqu’à 99 caractères. On alloue assez de mémoire pour ce nouveau string. */
	 lpc_string *s = lpc_make_string("bonjour", 100);

	 int r = lpc_call( mm, "fun_difficile", INT, &a, DOUBLE, &b, STRING, s, NOP);
	  
	 printf("r = %d",r);
	 
	 if(lpc_close(mm)==-1)
	    printf("munmap failled");


  return 0;
}
