/***********************************************************
 *  
 * 
 * 
 ********************************************************/
#include "../lib/lpc_utility.h"
#include "../../include/lpc_structure.h"
#include "../../include/lpc_panic.h"

#include "../../include/Memoire/init_memory.h"

void *lpc_open(const char *name)
{

        int fd = shm_open(name, O_RDWR, S_IRUSR | S_IWUSR);
        if (fd == -1)
        {
                printf("shm_open\n");
                return NULL;
        }

        struct stat bufStat;
        fstat(fd, &bufStat);
        printf("map: longeur object %d\n",
               (int)bufStat.st_size);

        void *adr = mmap(NULL, bufStat.st_size,
                         PROT_READ | PROT_WRITE,
                         MAP_SHARED, fd, 0);

        if (adr == MAP_FAILED)
                PANIC_EXIT("mmap\n");

        return adr;
}

int lpc_close(void *mem)
{

        int result = munmap(mem, sizeof(mem));
        return result;
}

int incrimente(void *a)
{
        sleep(10);
        return 0;
}
int desincrimente(void *b)
{
        return 0;
}
void lpc_lock_wait(lpc_memory *mem)
{
        //lock mutex
        int code;
        if ((code = pthread_mutex_lock(&mem->hd.mutex)) != 0)
        {
                printf("error: pthread_mutex_lock\n");
        }
        //wait for cond
        printf("acquired mutex\n");
        while (mem->hd.libre)
        {
                printf("waiting ... \n");
                if ((code = pthread_cond_wait(&mem->hd.wcond, &mem->hd.mutex)) != 0)
                {
                        printf("error: pthread_cond_wait\n");
                }
        }
}
int main(int argc, char *argv[])
{

        //Initialisation de tableau
        lpc_function tab[3];
        strcpy(tab[1].fun_name, "incrimente");
        tab[1].fun = incrimente;
        strcpy(tab[2].fun_name, "desincrimente");
        tab[2].fun = desincrimente;

        //create shared memory object
        void *memory = init_memory("/shmo_name");
        lpc_memory *MemorySimple = memory;
        pid_t childpid;

        int code;

        while (1)
        {
                lpc_lock_wait(MemorySimple);

               
                  char pid[50];
                  char name[50] = "/shmo_name";
                  sprintf(pid, "%d ", MemorySimple->hd.pid);
                  strcat(name, pid);
                  memcpy(MemorySimple->hd.shmo_name_Pid, name, sizeof(name));
                  MemorySimple->hd.libre = 1;
                if ((childpid = fork()) == 0)
                {
                         printf("received notification from client\n");
                        //create new shared memory object for child
                        lpc_memory *memorychild = init_memory(name);
                       
                       
                        lpc_lock_wait(memorychild);
                        void *ptr = memorychild;
                        ptr = (void *)((char *)memorychild + sizeof(header));
                        printf("client de pid=(%d) a envoyer \n", memorychild->hd.pid);
                        printf("le client veut appelle %s \n",memorychild->hd.fun_name);
                        if(strcmp(memorychild->hd.fun_name,"incriment")==0)
                        {
                            incrimente(memorychild);
                        }
                        for (int i = 0; i < 4; i++)
                        {
                                switch (memorychild->hd.types[i])
                                {
                                case 1:
                                        printf("%d: %d\n", memorychild->hd.types[i], *((int *)((char *)ptr + memorychild->hd.offsets[i])));
                                        *((int *)((char *)ptr + memorychild->hd.offsets[i])) += 1;
                                        break;
                                case 2:
                                        printf("%d: %f\n", memorychild->hd.types[i], *((double *)((char *)ptr + memorychild->hd.offsets[i])));
                                        *((double *)((char *)ptr + memorychild->hd.offsets[i])) += 1;
                                        break;
                                case 3:
                                        printf("%d: %s\n", memorychild->hd.types[i], ((lpc_string *)((char *)ptr + memorychild->hd.offsets[i]))->string);
                                        break;
                                }
                        }
                        //unlock mutex
                        printf("acquired mutex and received signal\n");
                        if ((code = pthread_mutex_unlock(&memorychild->hd.mutex)) != 0)
                        {
                                printf("error: pthread_mutex_unlock\n");
                        }
                        printf("released mutex\n");

                        if ((code = pthread_cond_signal(&memorychild->hd.rcond)) != 0)
                        {
                                printf("error: pthread_cond_signal wcond\n");
                        }

                        lpc_close(memorychild);
                }

                //attende trois seconde pendant que l'enfant cree l'objet
                 sleep(3);
                //Avertir au child la creation de nouveau shmmemroy
                //unlock mutex
                printf("acquired mutex and received signal\n");
                if ((code = pthread_mutex_unlock(&MemorySimple->hd.mutex)) != 0)
                {
                        printf("error: pthread_mutex_unlock\n");
                }
                printf("released mutex\n");

                if ((code = pthread_cond_signal(&MemorySimple->hd.rcond)) != 0)
                {
                        printf("error: pthread_cond_signal wcond\n");
                }
        }

        lpc_close(MemorySimple);

        return 0;
}
