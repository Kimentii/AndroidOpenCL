#include "cpu_filter.h"

unsigned char* filter_via_cpu(const PPMImage* image, ftype* filter, const int filter_size)
{
	size_t ix = image->sizeX * 3;
	size_t iy = image->sizeY;
	unsigned char* result = malloc(ix * iy);
	for (int i = 0; i < image->sizeY; i++) {
		for (int j = 0; j < image->sizeX * 3; j++) {
			float res = 0;
			for (int n = 0; n < filter_size; n++) {
				for (int m = 0; m < filter_size; m++) {
					res += filter[n * filter_size + m] * image->data[(i + n)*image->pitch + j + m * 3];
				}
			}
			result[i*ix + j] = res;
		}
	}
	return result;
}
