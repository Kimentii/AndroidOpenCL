#include <jni.h>
#include <stdlib.h>
#include <string.h>

#include "cl.h"

static size_t filter_size;
static cl_context context;
static cl_command_queue queue;
static cl_kernel kernel;
static float *filter;
static cl_mem filter_buffer;

JNIEXPORT jint JNICALL
Java_com_kimentii_cameraresearch_MainActivity_compileKernel(JNIEnv *env, jobject instance,
                                                            jint new_filter_size,
                                                            jstring kernel) {
    // Blur filter initialization
    // For 5-size filter:
    // (1 2 3 2 1)
    // (2 3 4 3 2)
    // (3 4 5 4 3)
    // (2 3 4 3 2)
    // (1 2 3 2 1)
    filter_size = (size_t) new_filter_size;
    filter = (float *) malloc(filter_size * filter_size * sizeof(float));
    float filter_sum = 0;
    for (int i = 0; i < filter_size; i++) {
        for (int j = 0; j < filter_size; j++) {
            float filter_value = 1 + i + j;
            if (j >= filter_size / 2) {
                filter_value -= (j - filter_size / 2) * 2;
                if (filter_size % 2 == 0) filter_value--;
            }
            if (i >= filter_size / 2) {
                filter_value -= (i - filter_size / 2) * 2;
                if (filter_size % 2 == 0) filter_value--;
            }
            filter[i * filter_size + j] = filter_value;
            filter_sum += filter_value;
        }
    }

    // Normalize filter to make cells sum equals 1
    float k = 1 / filter_sum;
    for (int i = 0; i < filter_size; i++) {
        for (int j = 0; j < filter_size; j++) {
            filter[i * filter_size + j] *= k;
        }
    }

    // Get available platforms
    cl_platform_id *platforms = (cl_platform_id *) malloc(sizeof(cl_platform_id));
    clGetPlatformIDs(1, platforms, NULL);

    // Get available devices of the first available platform
    cl_device_id *devices = (cl_device_id *) malloc(sizeof(cl_device_id) * 1);
    clGetDeviceIDs(*platforms, CL_DEVICE_TYPE_GPU, 1, devices, NULL);

    // Create context and queue fot the first device
    context = clCreateContext(NULL, 1, devices, NULL, NULL, NULL);
    queue = clCreateCommandQueue(context, *devices, 0, NULL);

    // Allocate filter buffer on device
    filter_buffer = clCreateBuffer(
            context,
            CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
            filter_size * filter_size * sizeof(float),
            filter,
            NULL
    );

    // Compile and save kernel
    const char *kernel_str = (*env)->GetStringUTFChars(env, kernel, 0);
    size_t kernel_str_size = strlen(kernel_str);
    cl_program program = clCreateProgramWithSource(context, 1, &kernel_str,
                                                   &kernel_str_size, NULL);

    // Set the kernel FILTER_SIZE define
    char filter_size_define[32];
    sprintf(filter_size_define, "-DFILTER_SIZE=%lu", filter_size);

    jint result = clBuildProgram(program, 1, devices, filter_size_define, NULL, NULL);

    kernel = clCreateKernel(program, "blur_filter", NULL);
    return result;
}

JNIEXPORT jintArray JNICALL
Java_com_kimentii_cameraresearch_BlurFilteringSurface_runFilter(JNIEnv *env, jobject instance,
                                                                jintArray in_, jint width,
                                                                jint height) {
    jint *in_bytes = (*env)->GetIntArrayElements(env, in_, 0);

    size_t pixel_size = 4;
    size_t frame_size = filter_size / 2;

    // Allocate input image memory on device
    cl_mem input_buffer = clCreateBuffer(
            context,
            CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
            (width * pixel_size * height),
            (void *) in_bytes,
            NULL
    );
    // Allocate output image memory on device
    cl_mem output_buffer = clCreateBuffer(
            context,
            CL_MEM_WRITE_ONLY,
            width * pixel_size * height,
            NULL,
            NULL
    );

    // Setting kernel args
    cl_ulong unsigned_width = (cl_ulong) width;
    cl_ulong unsigned_height = (cl_ulong) height;

    clSetKernelArg(kernel, 0, sizeof(cl_mem), (void *) &input_buffer);
    clSetKernelArg(kernel, 1, sizeof(cl_mem), (void *) &output_buffer);
    clSetKernelArg(kernel, 2, sizeof(cl_ulong), (void *) &(unsigned_width));
    clSetKernelArg(kernel, 3, sizeof(cl_ulong), (void *) &unsigned_height);
    clSetKernelArg(kernel, 4, sizeof(cl_mem), (void *) &filter_buffer);

    // Calculating work size
    // Each group uses 16x4 work-items, each process 1x4 pixels
    // So one work-group process 64x64 pixels
    // One case-sensitive letter illustrates one work-item scope
    // inside work-group
    // a b c d e f g h i j k l m n o p
    // a b c d e f g h i j k l m n o p
    // a b c d e f g h i j k l m n o p
    // a b c d e f g h i j k l m n o p
    // A B C D E F G H I J K L M N O P
    // A B C D E F G H I J K L M N O P
    // A B C D E F G H I J K L M N O P
    // A B C D E F G H I J K L M N O P
    // ... 8 rows more ...
    size_t group_x_size = 16;
    size_t group_x_pixels = group_x_size - (filter_size - 1);
    size_t group_y_size = 4;
    size_t y_pixels_by_work_group = 4;
    size_t group_y_pixels = group_y_size * y_pixels_by_work_group - (filter_size - 1);

    size_t groups_x = width / group_x_pixels;
    if (width % group_x_pixels != 0) groups_x++;

    size_t groups_y = height / group_y_pixels;
    if (height % group_y_pixels != 0) groups_y++;

    const size_t work_size[2] = {group_x_size * groups_x, group_y_size * groups_y};
    const size_t group_size[2] = {group_x_size, group_y_size};

    // Enqueue kernel execution
    jint status = clEnqueueNDRangeKernel(
            queue,
            kernel,
            2,
            0,
            work_size,
            group_size,
            0,
            NULL,
            NULL
    );
    // Wait till operation completes
    clFinish(queue);

    // Allocate result buffer
    jintArray newArray = (*env)->NewIntArray(env, width * height);
    jint *result = (*env)->GetIntArrayElements(env, newArray, 0);

    // Read kernel result
    clEnqueueReadBuffer(
            queue,
            output_buffer,
            CL_TRUE,
            0,
            width * pixel_size * height,
            (void *) result,
            0,
            NULL,
            NULL
    );

    clReleaseMemObject(input_buffer);
    clReleaseMemObject(output_buffer);

//    for (int i = 0; i < height; i++) {
//        for (int j = 0; j < width; j++) {
//            result[i * width + j] = width;//0xff101010;
//        }
//    }

    // Cleanup
    (*env)->ReleaseIntArrayElements(env, in_, in_bytes, 0);
    (*env)->ReleaseIntArrayElements(env, newArray, result, 0);

    return newArray;
}