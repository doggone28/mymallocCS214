#include <stddef.h>


void * mymalloc(size_t size, char *file, int line);
void myfree(void *ptr, char *file, int line);




#define mymalloc(size) mymalloc(size, __FILE__, __LINE__)
#define myfree(ptr) myfree(ptr, __FILE__, __LINE__)



