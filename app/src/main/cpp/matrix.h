#ifndef MATRIX_H
#define MATRIX_H

#define MATRIX_WIDTH			1024
#define MATRIX_HEIGHT			1024
#define MATRIX_SIZE_BYTES	sizeof(mtype) * MATRIX_WIDTH * MATRIX_HEIGHT

typedef float mtype;

mtype* matrix_create(int h, int w);
void matrix_release(mtype*);
void matrix_show(mtype*, int w, int h);
void matrix_uchar_show(unsigned char*, int w, int h);
mtype* matrix_int_img(mtype*, int h, int w);
int matrix_compare(mtype*, mtype*, int h, int w);

#endif
