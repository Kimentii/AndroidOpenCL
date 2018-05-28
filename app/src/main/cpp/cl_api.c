#include <stdio.h>
#include <malloc.h>
#include "cl_api.h"

#define MAX_PLATFORMS	16
#define MAX_DEVICES		16

cl_int call_errno;
cl_int cl_errno;

cl_platform_id get_platform_id()
{
	call_errno = 0;
	cl_platform_id platforms[MAX_PLATFORMS];
	cl_uint p_num, p_use;

	cl_errno = clGetPlatformIDs(MAX_PLATFORMS, platforms, &p_num);
	if (cl_errno != CL_SUCCESS || p_num <= 0) {
		printf("Error during getting platform ids: %d\n", p_num);
		call_errno = cl_errno;
		return NULL;
	}

	if (p_num > 1) {
		printf("Several platforms available. Please, select one:\n");
		for (int i = 0; i < p_num; i++) {
			char p_name[256], p_vendor[256], p_version[256];
			if (CL_SUCCESS != clGetPlatformInfo(platforms[i], 
									CL_PLATFORM_NAME, 256, p_name, NULL)) {
				printf("Error during getting platform name\n");
				call_errno = cl_errno;
				return NULL;
			}
			if (CL_SUCCESS != clGetPlatformInfo(platforms[i], 
									CL_PLATFORM_VENDOR, 256, p_vendor, NULL)) {
				printf("Error during getting platform vendor\n");
				call_errno = cl_errno;
				return NULL;
			}
			if (CL_SUCCESS != clGetPlatformInfo(platforms[i], 
									CL_PLATFORM_VERSION, 256, p_version, NULL)) {
				printf("Error during getting platform version\n");
				call_errno = cl_errno;
				return NULL;
			}
			printf("%d) %s: %s (%s)\n", i, p_name, p_vendor, p_version);
		}
		scanf("%d", &p_use);
		flushin();
	} else {
		p_use = 0;
	}
	return platforms[p_use];
}

cl_device_id get_device_id(cl_platform_id platform_id)
{
	cl_device_id devices[MAX_DEVICES];
	cl_uint d_num, d_use;

	cl_errno = clGetDeviceIDs(platform_id, 
				CL_DEVICE_TYPE_GPU, MAX_DEVICES, devices, &d_num);
	if (cl_errno != CL_SUCCESS || d_num <= 0) {
		printf("Error during getting device ids: %d\n", d_num);
			call_errno = cl_errno;
			return NULL;
	}

	if (d_num >= 1) {
		printf("Several devices available. Please, select one:\n");
		for (int i = 0; i < d_num; i++) {
			char d_name[256], d_vendor[256], d_version[256];
			if (CL_SUCCESS != clGetDeviceInfo(devices[i], 
									CL_DEVICE_NAME, 256, d_name, NULL)) {
				printf("Error during getting device name\n");
				call_errno = cl_errno;
				return NULL;
			}
			if (CL_SUCCESS != clGetDeviceInfo(devices[i], 
									CL_DEVICE_VENDOR, 256, d_vendor, NULL)) {
				printf("Error during getting device vendor\n");
				call_errno = cl_errno;
				return NULL;
			}
			if (CL_SUCCESS != clGetDeviceInfo(devices[i], 
									CL_DEVICE_VERSION, 256, d_version, NULL)) {
				printf("Error during getting device version\n");
				call_errno = cl_errno;
				return NULL;
			}
			printf("%d) %s: %s (%s)\n", i, d_name, d_vendor, d_version);
		}
		scanf("%d", &d_use);
		flushin();
	} else {
		d_use = 0;
	}
	return devices[d_use];
}

size_t get_device_group_size(cl_device_id device)
{
	size_t gs[3];
	cl_errno = clGetDeviceInfo(device, 
					CL_DEVICE_MAX_WORK_ITEM_SIZES, sizeof(size_t) * 3, gs, NULL
	);
	if (cl_errno != CL_SUCCESS) {
		printf("Error during getting max device group size: %d\n", cl_errno);
		return 64;
	}
	return gs[0];
}

cl_program get_cl_program(cl_context context, const char *filename)
{
	call_errno = 0;

	FILE *file = fopen(filename, "r");
	if (file == NULL) {
		printf("Error during opening '%s'\n", filename);
		call_errno = CALL_ERROR;
		return NULL;
	}
	fseek(file, 0, SEEK_END);
	size_t file_size = ftell(file);
	rewind(file);

	const char *program_source = (char*)malloc(file_size);
	if (file_size != fread((char*)program_source, 
										sizeof(char), file_size, file)) {
		printf("Error during reading from '%s'\n", filename);
		call_errno = CALL_ERROR;
		fclose(file);
		return NULL;
	}

	cl_program program = clCreateProgramWithSource(
																context,
																1,
																&program_source,
																&file_size,
																&cl_errno
	);
	
	if (cl_errno != CL_SUCCESS) {
		printf("Error during creating program\n");
		fclose(file);
		call_errno = cl_errno;
		return NULL;
	}
	return program;
}
