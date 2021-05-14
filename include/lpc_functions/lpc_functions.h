#ifndef LPC_FUNCTIONS_H
#define LPC_FUNCTIONS_H

void *lpc_open(const char *name);
int lpc_close(void *mem);
lpc_string *lpc_make_string(const char *s, int taille);

#endif
