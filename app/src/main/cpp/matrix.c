#include <stdio.h>
#include <stdlib.h>
#include "matrix.h"

mtype* matrix_create(int h, int w)
{
	mtype* matrix = (mtype*)malloc(w * h * sizeof(mtype));
	if (matrix == NULL) {
		return NULL;
	}

	for (int i = 0; i < w * h; i++) {
		matrix[i] = 1;
	}

	return matrix;
}

void matrix_release(mtype* matrix)
{
	free(matrix);
}

void matrix_show(mtype* matrix, int w, int h)
{
	for (int i = 0; i < h; i++) {
		for (int j = 0; j < w; j++) {
			if (j < 10) {
				printf("%.2f\t", matrix[i * w + j]);
			}
		}
		printf("\n");
		if (i >= 34) {
			return;
		}
	}
}

void matrix_uchar_show(unsigned char* matrix, int w, int h)
{
	for (int i = 0; i < h; i++) {
		for (int j = 0; j < w; j++) {
			if (j < 15) {
				printf("%d\t", matrix[i * w + j]);
			}
		}
		printf("\n");
		if (i >= 10) {
			return;
		}
	}
}
mtype* matrix_int_img(mtype* src, int h, int w)
{
	for(int i = 0; i < w; i++) {
		for(int j = 1; j < h; j++) {
			src[j*w + i] = src[(j-1)*w + i] + src[j*w + i];
		}
	}
	for(int i = 0; i < h; i++) {
		for(int j = 1; j < w; j++) {
			src[i*w + j] = src[i*w + j - 1] + src[i*w + j];
		}
	}
	return src;
}

mtype* matrix_int_img_diag(mtype* src, int h, int w)
{
	mtype* result = matrix_create(h, w);
	result[0] = src[0];
	for(int i = 1; i < w; i++) {
		result[i] = result[i-1] + src[i];
	}
	for(int i = 1; i < h; i++) {
		result[w*i] = result[w*(i-1)] + src[w*i];
	}
	for(int i = 1; i < h; i++) {
		for(int j = 1; j < w; j++) {
			result[i*h + j] = result[(i-1)*h + j] + result[(i*h) + j-1] 
						- result[(i-1)*h + j-1] + src[(i*h) + j];
		}
	}
	return result;
}

int matrix_compare(mtype* m1, mtype* m2, int h, int w)
{
	for (int i = 0; i < h; i++) {
		for (int j = 0; j < w; j++) {
			if (m1[i*w + j] != m2[i*w+j]) {
				//printf("Assertion error at [%d, %d]: m1=%d, m2=%d\n", i, j, m1[i*w+j], m2[i*w+j]);
				return -1;
			}
		}
	}
	return 0;
}
