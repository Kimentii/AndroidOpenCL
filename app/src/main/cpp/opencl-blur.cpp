#include <jni.h>
#include <string>
#include <stdlib.h>
#include "cl.h"
#include "cpu_filter.h"
#include "filter.h"
#include "ppm.h"

extern "C" JNIEXPORT jlong JNICALL
Java_com_kimentii_cameraresearch_MainActivity_getGPUWorkItems(JNIEnv *env, jobject instance) {
    // Get available platforms
    cl_platform_id *platforms = (cl_platform_id *) malloc(sizeof(cl_platform_id));
    clGetPlatformIDs(1, platforms, NULL);

    // Get available devices of the first available platform
    cl_device_id *devices = (cl_device_id *) malloc(sizeof(cl_device_id) * 2);
    clGetDeviceIDs(*platforms, CL_DEVICE_TYPE_GPU, 2, devices, NULL);

    // Get max work group size of the first available device
    size_t max_work_group_size;
    clGetDeviceInfo(*devices, CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(size_t), &max_work_group_size,
                    NULL);
    return max_work_group_size;
}

extern "C"
JNIEXPORT jbyteArray JNICALL
Java_com_kimentii_cameraresearch_MainActivity_filterImage(JNIEnv *env, jobject instance,
                                                          jbyteArray image_, jint width,
                                                          jint height) {
    //jbyte *image = env->GetByteArrayElements(image_, NULL);
    /*jbyte image[width * height];
    env->GetByteArrayRegion(image_, 0, width * height, image);*/

    int length = env->GetArrayLength(image_);
    jbyte *buf = env->GetByteArrayElements(image_, NULL);

//    for (int i = 0; i < length; i++) {
//        buf[i] = 100;
//    }

    /*for (jint i = 0; i < height; i++) {
        for (jint j = 0; j < width; j++) {
            buf[i * width + j] += 50;
            //image[i * width + j] += 100;
        }
    }*/

    jbyteArray result = env->NewByteArray(length);
    env->SetByteArrayRegion(result, 0, length, (const jbyte *) buf);
    //delete[] buf;
    env->ReleaseByteArrayElements(image_, buf, JNI_ABORT);
    return result;
    //return reinterpret_cast<jbyteArray>(image);
}