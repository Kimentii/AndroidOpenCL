#ifndef FILTER_H
#define FILTER_H

#include <sys/types.h>

#define FILTER_TYPE_BLUR 1
#define FILTER_TYPE_CUSTOM1 2
#define FILTER_TYPE_CUSTOM2 3
#define FILTER_TYPE_CUSTOM3 4
#define FILTER_TYPE_CUSTOM5 5
#define FILTER_TYPE_AVG 6

typedef float ftype;

ftype* get_filter(int h, int w, int type);
void normalize_filter(ftype* filter, int h, int w, ftype sum);

#endif
