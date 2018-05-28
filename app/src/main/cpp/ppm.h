#ifndef PPM
#define PPM

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
	size_t sizeX, sizeY, pitch, frame_size;
	unsigned char *data;
} PPMImage;

PPMImage* read_ppm(const char *filename);

PPMImage* mock_ppm(int w, int h);
PPMImage* mock_ppm_pitch(int w, int h, int p);

void resize_image(PPMImage* image, const int block_width, const int block_height);

void show_iamge_part(PPMImage* image, const int width, const int height);

void add_frame(PPMImage* image, const int frame_size);

void write_ppm(PPMImage* image, const char *filename);

void write_ppm_with_frame(PPMImage* image, const char *filename);

#endif
