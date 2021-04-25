#ifdef __linux__
#define _XOPEN_SOURCE 500
#endif

#include "../../src/lib/lpc_utility.h"
#include "../lpc_structure.h"


#include "memory.h"
#include "../lpc_panic.h"
#include "../MutexCond/init_mutex.h"

#include "init_memory.h"

void *init_memory(const char *mem_objet)
{
  //int creation = 0;

  const char *shm_name = mem_objet;

  /* supprimer l'objet mémoire avant de le récréer */
  shm_unlink(shm_name);

  /* open and create */
  int fd = shm_open(shm_name, O_CREAT | O_RDWR,
                    S_IWUSR | S_IRUSR);
  if (fd < 0)
    PANIC_EXIT("shm_open");

  if (ftruncate(fd, sizeof(lpc_memory)) < 0)
    PANIC_EXIT("ftruncate");

  /* projection de shared memory object dans
   * la mémoire                              */
  lpc_memory *mem = mmap(NULL, sizeof(lpc_memory), PROT_READ | PROT_WRITE,
                     MAP_SHARED, fd, 0);
  if ((void *)mem == MAP_FAILED)
    PANIC_EXIT("mmap");

  //initialiser la mémoire
  int code;
  mem->hd.libre = 1;            /* memoire est libre initialement*/
  mem->hd.length_arr[0] = 0;    //offset for first data entry

  code = initialiser_mutex(&mem->hd.mutex);
  if (code != 0)
      PANIC_EXIT("init mutex");

  code = initialiser_cond(&mem->hd.rcond);
  if (code != 0)
      PANIC_EXIT("init_rcond");
  code = initialiser_cond(&mem->hd.wcond);
  if (code != 0)
       PANIC_EXIT("init_wcond");

  return mem;
}
