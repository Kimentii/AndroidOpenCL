#include "cl_wrapper.h"
#include <malloc.h>
#include <stdio.h>

const int MAX_SOURCE_SIZE = 10000;

void show_error(cl_int ret, const char* fun_name) {
	if (ret != 0) {
		printf("Error(%s): %d", fun_name, ret);
	}
}

cl_device_id get_device_id() {
	cl_int ret;
	cl_platform_id platform_id[2];
	cl_device_id device_id;
	cl_uint ret_num_platforms;
	/* получить доступные платформы */
	ret = clGetPlatformIDs(2, platform_id, &ret_num_platforms);
	//printf("ret_num_platforms: %ld\n", ret_num_platforms);

	cl_uint ret_num_devices;
	/* получить доступные GPU */
	ret = clGetDeviceIDs(platform_id[1], CL_DEVICE_TYPE_GPU, 1, &device_id, &ret_num_devices);
	//printf("Num of GPU: %d\n", ret_num_devices);
	show_error(ret, "clGetDeviceIDs");
	return device_id;
}

cl_context get_context(cl_device_id device_id) {
	cl_int ret;
	cl_context context;

	context = clCreateContext(NULL, 1, &device_id, NULL, NULL, &ret);
	if (ret != 0) {
		printf("clCreateContext: %d\n", ret);
	}
	return context;
}

cl_command_queue get_command_queue(cl_context context, cl_device_id device_id) {
	cl_int ret;
	cl_command_queue command_queue;
	/* создаем команду */
	command_queue = clCreateCommandQueue(context, device_id, CL_QUEUE_PROFILING_ENABLE, &ret);
	show_error(ret, "clCreateCommandQueue");
	return command_queue;
}

cl_program create_program(cl_context context, cl_device_id device_id,
	const char* kernel_file_name) {
	cl_int ret;

	FILE *fp;
	size_t source_size;
	char *source_str;
	int i;

	fp = fopen(kernel_file_name, "r");
	if (!fp) {
		fprintf(stderr, "Failed to load kernel.\n");
	}
	source_str = (char *)malloc(MAX_SOURCE_SIZE);
	source_size = fread(source_str, 1, MAX_SOURCE_SIZE, fp);
	fclose(fp);

	source_str[0] = '/';
	source_str[1] = '/';
	/* создать бинарник из кода программы */
	cl_program program = clCreateProgramWithSource(context, 1, (const char **)&source_str, (const size_t *)&source_size, &ret);
	if (ret != 0) {
		printf("clCreateProgramWithSource: %d\n", ret);
	}


	ret = clBuildProgram(program, 1, &device_id, NULL, NULL, NULL);
	printf("Build output:");
	size_t log_size;
	clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);
	char *log = (char *)malloc(log_size);
	clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, log_size, log, NULL);
	printf("%s\n", log);
	return program;
}

cl_kernel create_kernel(const char* str, cl_program program) {
	int ret;
	cl_kernel kernel = clCreateKernel(program, str, &ret);
	if (ret != 0) {
		printf("clCreateKernel: %d\n", ret);
	}
	return kernel;
}