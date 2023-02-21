#include "rng.h"

uint32_t myrng (uint64_t* seed) {
	*seed ^= *seed << 13;
	*seed ^= *seed >> 7;
	*seed ^= *seed << 17;
	return (uint32_t)(*seed & 0x7FFFFFFF);
}

uint64_t get_rand_seed () {
	return ((uint64_t)rand () << 32) | (uint64_t)rand ();
}