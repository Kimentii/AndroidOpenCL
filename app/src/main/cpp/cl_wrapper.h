#ifndef CL_WRAPPER
#define CL_WRAPPER
#include "cl.h"

void show_error(cl_int ret, const char* fun_name);

cl_device_id get_device_id();

cl_context get_context(cl_device_id device_id);

cl_command_queue get_command_queue(cl_context context, cl_device_id device_id);

cl_program create_program(cl_context context, cl_device_id device_id, const char* kernel_file_name);

cl_kernel create_kernel(const char* str, cl_program program);

#endif
