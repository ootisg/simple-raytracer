#ifndef RNG_H
#define RNG_H

#include <stdlib.h>
#include <stdint.h>

#define MYRNG_MAX 2147483647

uint32_t myrng (uint64_t* seed);

uint64_t get_rand_seed ();

#endif