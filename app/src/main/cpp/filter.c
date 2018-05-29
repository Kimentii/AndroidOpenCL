#include <stdlib.h>
#include <stdio.h>
#include "filter.h"

ftype* get_filter(int h, int w, int type)
{
	srand(3);
	ftype* filter = (ftype*)malloc(h * w * sizeof(ftype));
	if (filter == NULL) {
		printf("Failed to allocate filter memory\n");
		return NULL;
	}
	switch (type) {
		case FILTER_TYPE_BLUR: {
			for (int i = 0; i < h; i++) {
				for (int j = 0; j < w; j++) {
					ftype filter_value = 1 + i + j;
					if (j >= w/2) {
						filter_value -= (j - w/2) * 2;
						if (w%2 == 0) filter_value--;
					}
					if (i >= h/2) {
						filter_value -= (i - h/2) * 2;
						if (h%2 == 0) filter_value--;
					}
					filter[i*w + j] = filter_value;
				}
			}
			break;
		}	
		case FILTER_TYPE_CUSTOM1: {
			for (int i = 0; i < h; i++) {
				for (int j = 0; j < w; j++) {
					filter[i*w + j] = rand();
				}
			}
			break;
		}	
		case FILTER_TYPE_CUSTOM2: {
			for (int i = 0; i < h; i++) {
				for (int j = 0; j < w; j++) {
					ftype filter_value = 1 + i + j;
					if (j >= w/2) {
						filter_value -= (j - w/2) * 2;
						if (w%2 == 0) filter_value--;
					}
					if (i >= h/2) {
						filter_value -= (i - h/2) * 2;
						if (h%2 == 0) filter_value--;
					}
					filter[i*w + j] = w - filter_value;
				}
			}
			break;
		}	
		case FILTER_TYPE_AVG: {
			for (int i = 0; i < h; i++) {
				for (int j = 0; j < w; j++) {
					filter[i*w + j] = 1;
				}
			}
			break;
		}	
	}

	return filter;
}

void normalize_filter(ftype* filter, int h, int w, ftype value)
{
	ftype filter_sum = 0;
	for (int i = 0; i < h; i++) {
		for (int j = 0; j < w; j++) {
			filter_sum += filter[i*w + j];
		}
	}
	ftype k = value/filter_sum;
	for (int i = 0; i < h; i++) {
		for (int j = 0; j < w; j++) {
			filter[i*w + j] *= k;
		}
	}
}
