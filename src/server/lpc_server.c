/***********************************************************
 *  
 * 
 * 
 ********************************************************/
#include "../../include/lpc_utility.h"
#include "../../include/lpc_structure.h"
#include "../../include/lpc_panic.h"
#include "../../include/Memoire/init_memory.h"
#include "../../include/lpc_functions/lpc_functions.h"
#include "libfonctionsclient.h"


void couleur(int c)
{
        switch (c)
        {
        case 0:
                printf("\033[30m");
                break;
        case 1:
                printf("\033[31m");
                break;
        case 2:
                printf("\033[32m");
                break;
        case 3:
                printf("\033[33m");
                break;
        case 4:
                printf("\033[34m");
                break;
        case 5:
                printf("\033[35m");
                break;
        case 6:
                printf("\033[36m");
                break;
        case 7:
                printf("\033[37m");
                break;
        case 8:
                printf("\033[00m");
                break;
        default:
                printf("\n\tcouleur non répertoriée\n");
        }
}



/* function that takes a name and a lpc_fun list 
returns a function pointer to a function that takes void* and returns int
-> finds server function corresponding to name */

int (*find_fun(char *name, lpc_function fun_list[20])) (void*){
        for(int i=0; i<20; i++){
                if(strcmp(name, fun_list[i].fun_name) == 0){

                        return fun_list[i].fun;
                }
        }
        return NULL;
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
        while (mem->hd.libre)
        {
                couleur(7);
                printf("waiting an client... \n");
                if ((code = pthread_cond_wait(&mem->hd.wcond, &mem->hd.mutex)) != 0)
                {
                        printf("error: pthread_cond_wait\n");
                }
        }
}




int main(int argc, char *argv[])
{

        //array that contains all executable functions of the server
        lpc_function function_list [20];   

        //add client functions
        lpc_function fun0 = {"add_int", &add_int};
        function_list[0] = fun0;

        lpc_function fun1 = {"print_lpc_string", &print_lpc_string};
        function_list[1] = fun1;

        lpc_function fun2 = {"modify_lpc_string", &modify_lpc_string};
        function_list[2] = fun2;

        lpc_function fun3 = {"modify_wrong_lpc_string", &modify_wrong_lpc_string};
        function_list[3] = fun3;
        

        //create shared memory object
        void *memory = init_memory("/shmo_name");
        lpc_memory *MemorySimple = memory;
        pid_t childpid[100];

        int code;
        int jesuis = 0;

        while (1)
        {
                couleur(7);
                printf("\n++++++++++Pere: je suis dans le %d Tour+++++++++++\n", jesuis);
                jesuis += 1;
                lpc_lock_wait(MemorySimple);
                couleur(7);
                printf("Pere:client PID(%d) connected \n", MemorySimple->hd.pid);

                char pid[50];
                char name[50] = "/shmo_name";
                sprintf(pid, "%d ", MemorySimple->hd.pid);
                strcat(name, pid);
                memcpy(MemorySimple->hd.shmo_name_Pid, name, sizeof(name));
                MemorySimple->hd.libre = 1;
                if ((childpid[jesuis - 1] = fork()) == 0)
                {

                        //create new shared memory object for child
                        lpc_memory *memorychild = init_memory(name);
                        couleur(jesuis + 1);
                        printf("\tEnfant %d: New Memory communication named(%s)\n", jesuis, name);

                        lpc_lock_wait(memorychild);
                        memorychild->hd.libre = 1;
                        couleur(jesuis + 1);
                        printf("\tEnfant %d: Client PID(%d) connected \n", jesuis, memorychild->hd.pid);
                        void *ptr = memorychild;
                        ptr = (void *)((char *)memorychild + sizeof(header));
                        couleur(jesuis + 1);
                        printf("\tClient  PID(%d) a envoyer \n", memorychild->hd.pid);
                        couleur(jesuis + 1);
                        printf("\tle client veut appelle %s \n", memorychild->hd.fun_name);
                       
                        int (*result_fun)(void*) = find_fun(memorychild->hd.fun_name, function_list);
                        
                        if((result_fun != NULL)){
                                int resultat = (result_fun)(ptr);
                                if(resultat == 0)
                                        memorychild->hd.return_v = resultat;
                                else{
                                        memorychild->hd.return_v = -1;
                                        memorychild->hd.err = resultat;        
                                }
                        }else{
                                memorychild->hd.return_v = -1;
                                memorychild->hd.err = ENOENT;
                                printf("\tfonctions %s n'existe pas\n", memorychild->hd.fun_name);
                        } 
                        
                        //unlock mutex
                        couleur(jesuis + 1);
                        printf("\tEnfant %d, Warn client PID(%d) his request Terminate \n", jesuis, memorychild->hd.pid);
                        if ((code = pthread_mutex_unlock(&memorychild->hd.mutex)) != 0)
                        {
                                printf("error: pthread_mutex_unlock\n");
                        }
                        if ((code = pthread_cond_signal(&memorychild->hd.rcond)) != 0)
                        {
                                printf("error: pthread_cond_signal wcond\n");
                        }

                        lpc_close(memorychild);
                        exit(0);
                }

                couleur(7);
                //attende trois seconde pendant que l'enfant cree l'objet
                sleep(3);
                //Avertir au child la creation de nouveau shmmemroy
                //unlock mutex
                couleur(7);
                printf("Pere: Warn client PID(%d) a New Memory Communication \n", MemorySimple->hd.pid);
                if ((code = pthread_mutex_unlock(&MemorySimple->hd.mutex)) != 0)
                {
                        printf("error: pthread_mutex_unlock\n");
                }

                if ((code = pthread_cond_signal(&MemorySimple->hd.rcond)) != 0)
                {
                        printf("error: pthread_cond_signal wcond\n");
                }
        }

        lpc_close(MemorySimple);

        return 0;
}
