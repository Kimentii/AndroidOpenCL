#include <stdint.h>

#ifdef _WIN32
#include <intrin.h>

uint64_t rdtsc()
{
	return __rdtsc();
}

#else

uint64_t rdtsc()
{
	unsigned int lo, hi;
	__asm__ __volatile__ ("rdtsc" : "=a" (lo), "=d" (hi));
	return ((uint64_t)hi <<32) | lo;
}

#endif

static uint64_t time_start;

void time_snap()
{
	time_start = rdtsc();
}

uint64_t time_elapsed()
{
	return rdtsc() - time_start;
}
