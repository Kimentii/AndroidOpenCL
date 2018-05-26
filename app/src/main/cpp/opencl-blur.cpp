#include <jni.h>
#include <string>
#include <stdlib.h>
#include "cl.h"

extern "C" JNIEXPORT jlong JNICALL
Java_com_kimentii_cameraresearch_MainActivity_getGPUWorkItems(JNIEnv *env, jobject instance) {
    // Get available platforms
    cl_platform_id* platforms = (cl_platform_id*)malloc(sizeof(cl_platform_id));
    clGetPlatformIDs(1, platforms, NULL);

    // Get available devices of the first available platform
    cl_device_id* devices = (cl_device_id*)malloc(sizeof(cl_device_id) * 2);
    clGetDeviceIDs(*platforms, CL_DEVICE_TYPE_GPU, 2, devices, NULL);

    // Get max work group size of the first available device
    size_t max_work_group_size;
    clGetDeviceInfo(*devices, CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(size_t), &max_work_group_size, NULL);
    return max_work_group_size;
}