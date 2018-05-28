#include "ppm.h"

PPMImage* read_ppm(const char *filename)
{
	char buff[3];
	PPMImage *result;
	FILE *fp;
	int maxval;

	fp = fopen(filename, "rb");
	if (!fp)
	{
		fprintf(stderr, "Unable to open file `%s'\n", filename);
		goto error;
	}

	if (!fgets(buff, sizeof(buff), fp))
	{
		perror(filename);
		goto error;
	}

	if (buff[0] != 'P' || buff[1] != '6')
	{
		fprintf(stderr, "Invalid image format (must be `P3')\n");
		goto error;
	}

	result = (PPMImage *)malloc(sizeof(PPMImage));
	if (!result)
	{
		fprintf(stderr, "Unable to allocate memory\n");
		goto error;
	}

	fgetc(fp);
	while (fgetc(fp) != '\n');

	if (fscanf(fp, "%lu %lu", &result->sizeX, &result->sizeY) != 2)
	{
		fprintf(stderr, "Error1 loading image `%s'\n", filename);
		goto error;
	}


	if (fscanf(fp, "%d", &maxval) != 1)
	{
		fprintf(stderr, "Error loading image `%s'\n", filename);
		goto error;
	}

	while (fgetc(fp) != '\n');

	result->data = (unsigned char *)malloc(result->sizeX * 3 * result->sizeY);
	if (!result)
	{
		fprintf(stderr, "Unable to allocate memory\n");
		goto error;
	}

	if (fread(result->data, 3 * sizeof(char), result->sizeY * result->sizeX, fp) != result->sizeY * result->sizeX)
	{
		fprintf(stderr, "Error loading image `%s'\n", filename);
		goto error;
	}

	result->pitch = 0;
	result->frame_size = 0;
	return result;

error:
#if defined(WIN32) || defined(WIN64)
	_getch();
#endif
	exit(1);
}

void resize_image(PPMImage* image, const int block_width, const int block_height)
{
	int image_width = image->sizeX * 3 + 2 * 3 * image->frame_size;
	int image_heigth = image->sizeY + 2 * image->frame_size;
	int width_in_blocks = image_width / block_width;
	if (!(width_in_blocks * block_width == image_width)) {
		width_in_blocks++;
	}
	int height_in_blocks = image_heigth / block_height;
	if (!(height_in_blocks * block_height == image_heigth)) {
		height_in_blocks++;
	}
	int required_matrix_width = width_in_blocks * block_width;
	int required_matrix_heigth = height_in_blocks * block_height;

	unsigned char *data;
	data = (unsigned char*)calloc(required_matrix_width
		* required_matrix_heigth, sizeof(unsigned char));
	for (int i = 0; i < image_heigth; i++) {
		for (int j = 0; j < image_width; j++) {
			data[i*required_matrix_width + j] =
				image->data[i*image_width + j];
		}
	}

	free(image->data);
	image->data = data;
	image->pitch = required_matrix_width;
}

void show_iamge_part(PPMImage* image, const int width, const int height)
{
	int image_width_bt = image->sizeX * 3 + 2 * 3 * image->frame_size;
	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
			printf("%4d", image->data[i * image_width_bt + j]);
		}
		printf("\n");
	}
}

void add_frame(PPMImage* image, const int frame_size)
{
	int old_width_bt = image->sizeX * 3;
	int new_image_width_bt = old_width_bt + frame_size * 3 * 2;
	int new_image_height = image->sizeY + frame_size * 2;
	int fr_sz_bt = frame_size * 3;
	unsigned char* result_data = calloc(new_image_height * new_image_width_bt, sizeof(char));
	for (int i = 0; i < frame_size; i++) {
		int j = 0;
		for (; j < fr_sz_bt; j += 3) {
			result_data[i*new_image_width_bt + j] = image->data[0];
			result_data[i*new_image_width_bt + j + 1] = image->data[1];
			result_data[i*new_image_width_bt + j + 2] = image->data[2];
		}
		for (; j < (image->sizeX * 3 + fr_sz_bt); j += 3) {
			result_data[i*new_image_width_bt + j] = image->data[j - fr_sz_bt];
			result_data[i*new_image_width_bt + j + 1] = image->data[j - fr_sz_bt + 1];
			result_data[i*new_image_width_bt + j + 2] = image->data[j - fr_sz_bt + 2];
		}
		for (; j < (image->sizeX * 3 + 2 * fr_sz_bt); j += 3) {
			result_data[i*new_image_width_bt + j] = image->data[old_width_bt - 3];
			result_data[i*new_image_width_bt + j + 1] = image->data[old_width_bt - 2];
			result_data[i*new_image_width_bt + j + 2] = image->data[old_width_bt - 1];
		}
	}
	for (int i = 0; i < image->sizeY; i++) {
		int j = 0;
		for (; j < fr_sz_bt; j+=3) {
			result_data[(i + frame_size)*new_image_width_bt + j]
				= image->data[i*old_width_bt];
			result_data[(i + frame_size)*new_image_width_bt + j + 1]
				= image->data[i*old_width_bt + 1];
			result_data[(i + frame_size)*new_image_width_bt + j + 2]
				= image->data[i*old_width_bt + 2];
		}
		for (; j < (old_width_bt + fr_sz_bt); j+=3) {
			result_data[(i + frame_size)*new_image_width_bt + j]
				= image->data[i*old_width_bt + j - fr_sz_bt];
			result_data[(i + frame_size)*new_image_width_bt + j + 1]
				= image->data[i*old_width_bt + j - fr_sz_bt + 1];
			result_data[(i + frame_size)*new_image_width_bt + j + 2]
				= image->data[i*old_width_bt + j - fr_sz_bt + 2];
		}
		for (; j < new_image_width_bt; j+=3) {
			result_data[(i + frame_size)*new_image_width_bt + j]
				= image->data[i*old_width_bt + old_width_bt - 3];
			result_data[(i + frame_size)*new_image_width_bt + j + 1]
				= image->data[i*old_width_bt + old_width_bt - 2];
			result_data[(i + frame_size)*new_image_width_bt + j + 2]
				= image->data[i*old_width_bt + old_width_bt - 1];
		}
	}
	for (int i = (image->sizeY + frame_size); i < (image->sizeY + 2 * frame_size); i++) {
		int j = 0;
		for (; j < fr_sz_bt; j+=3) {
			result_data[i*new_image_width_bt + j]
				= image->data[(image->sizeY - 1)*old_width_bt];
			result_data[i*new_image_width_bt + j + 1]
				= image->data[(image->sizeY - 1)*old_width_bt + 1];
			result_data[i*new_image_width_bt + j + 2]
				= image->data[(image->sizeY - 1)*old_width_bt + 2];
		}
		for (; j < (old_width_bt + fr_sz_bt); j+=3) {
			result_data[i*new_image_width_bt + j]
				= image->data[(image->sizeY - 1)*old_width_bt + j - fr_sz_bt];
			result_data[i*new_image_width_bt + j + 1]
				= image->data[(image->sizeY - 1)*old_width_bt + j - fr_sz_bt + 1];
			result_data[i*new_image_width_bt + j + 2]
				= image->data[(image->sizeY - 1)*old_width_bt + j - fr_sz_bt + 2];
		}
		for (; j < new_image_width_bt; j+=3) {
			result_data[i*new_image_width_bt + j]
				= image->data[(image->sizeY - 1)*old_width_bt + old_width_bt - 3];
			result_data[i*new_image_width_bt + j + 1]
				= image->data[(image->sizeY - 1)*old_width_bt + old_width_bt - 2];
			result_data[i*new_image_width_bt + j + 2]
				= image->data[(image->sizeY - 1)*old_width_bt + old_width_bt - 1];
		}
	}
	free(image->data);
	image->data = result_data;
	image->frame_size = frame_size;
}

void write_ppm(PPMImage* image, const char *filename)
{
	FILE *fp;
	fp = fopen(filename, "wb");
	fprintf(fp, "P6\n");
	fprintf(fp, "# opencl\n");
	fprintf(fp, "%lu %lu\n", image->sizeX, image->sizeY);
	fprintf(fp, "255\n");
	if (image->pitch > 0) {
		for (int i = image->frame_size; i < image->sizeY + image->frame_size; i++) {
			size_t was_write;
			was_write = fwrite(image->data + i * image->pitch + 3 * image->frame_size,
				sizeof(char),
				image->sizeX * 3,
				fp);
		}
	}
	else {
		for (int i = image->frame_size; i < image->sizeY + image->frame_size; i++) {
			size_t was_write;
			was_write = fwrite(image->data + i * (image->sizeX * 3 + 2 * 3 * image->frame_size) + 3 * image->frame_size,
				sizeof(char),
				image->sizeX * 3,
				fp);
		}
	}
	fflush(fp);
	fclose(fp);
}

PPMImage* mock_ppm(int w, int h)
{
	PPMImage* mock = (PPMImage*)malloc(sizeof(PPMImage));
	mock->data = (unsigned char*)malloc(h*w);
	mock->sizeX = w;
	mock->sizeY = h;
	mock->pitch = w;
	mock->frame_size = 1;
	for (int i = 0; i < h; i++) {
		for (int j = 0; j < w; j++) {
			mock->data[i*w + j] = (unsigned char)(j % 4);
		}
	}
	return mock;
}

PPMImage* mock_ppm_pitch(int w, int h, int p)
{
	PPMImage* mock = (PPMImage*)malloc(sizeof(PPMImage));
	mock->data = (unsigned char*)malloc(h*p);
	mock->sizeX = w;
	mock->sizeY = h;
	mock->pitch = p;
	for (int i = 0; i < h; i++) {
		for (int j = 0; j < p; j++) {
			if (j < w) {
				mock->data[i*p + j] = (unsigned char)(j % 4);
			}
			else {
				mock->data[i*p + j] = 5;
			}
		}
	}
	return mock;
}

void write_ppm_with_frame(PPMImage* image, const char *filename)
{
	FILE *fp;
	fp = fopen(filename, "wb");
	fprintf(fp, "P6\n");
	fprintf(fp, "# opencl\n");
	fprintf(fp, "%lu %lu\n",
		image->sizeX + 2 * image->frame_size,
		image->sizeY + 2 * image->frame_size);
	fprintf(fp, "255\n");
	if (image->pitch > 0) {
		for (int i = 0; i < (image->sizeY + 2 * image->frame_size); i++) {
			fwrite(image->data + i * image->pitch,
				sizeof(char),
				image->sizeX * 3 + 2 * 3 * image->frame_size,
				fp);
		}
	}
	else {
		fwrite(image->data, sizeof(char),
			(image->sizeX + 2 * image->frame_size) * 3 *(image->sizeY + 2 * image->frame_size),
			fp);
	}
	fclose(fp);
}
