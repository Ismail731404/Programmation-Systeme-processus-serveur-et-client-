#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>


double add(int a,double b)
{
  return a+b;
}

int main(){

printf("je suis le server");


   int fd;
   off_t len = 1024; 
   void *mm;
   if((fd=shm_open("/monobject",O_CREAT|O_RDWR,S_IRUSR|S_IWUSR))==-1){
         printf("failed sh_open");
         return 0;
   }
   struct stat buf;
   if(fstat(fd,&buf)<0)
         printf("error fstat");
         
   if(ftruncate( fd, len) == -1){
       perror("ftruncate"); exit(1);
   }
   
   if((mm=mmap(NULL,len,PROT_READ|PROT_WRITE,MAP_SHARED,fd,0))==MAP_FAILED) {
         printf("mmap Failed");
         return 0;
   }
   
   memcpy(mm, add ,(1024*sizeof(char)));
   
   
return 0;
}
