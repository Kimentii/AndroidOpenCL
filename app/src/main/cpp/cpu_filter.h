#ifndef CPU_FILTER_H
#define CPU_FILTER_H

#include "ppm.h"
#include "filter.h"

unsigned char* filter_via_cpu(const PPMImage* image, ftype* filter, const int filter_size);

#endif
