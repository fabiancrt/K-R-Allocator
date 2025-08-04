#ifndef SBRK_H_
#define SBRK_H_
#include <stdint.h>   /* intptr_t */
#include <errno.h>    /* errno, ENOMEM */
void *sbrk(intptr_t increment);

#endif /* SBRK_H_ */
