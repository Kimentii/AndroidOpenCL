#ifndef CL_API
#define CL_API

#include "cl.h"

#define CALL_SUCCESS	0
#define CALL_ERROR		-1

#define flushin() while(getchar() != '\n') {}

extern cl_int call_errno;
extern cl_int cl_errno;

cl_platform_id get_platform_id();
cl_device_id get_device_id(cl_platform_id platform_id);
cl_program get_cl_program(cl_context, const char*);
size_t get_device_group_size(cl_device_id);

#endif
