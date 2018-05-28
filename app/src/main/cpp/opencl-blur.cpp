#include <jni.h>
#include <string>
#include <stdlib.h>
#include <android/bitmap.h>
#include "cl.h"

static const char* program_src = ""
        "#define PIXEL_SIZE\t\t4\n"
        "#define FILTER_SIZE\t\t5\n"
        "#define GROUP_SIZE_X\t\t16\n"
        "#define GROUP_SIZE_Y\t\t4\n"
        "#define GROUP_SIZE\t\t\t(GROUP_SIZE_X * GROUP_SIZE_Y)\n"
        "#define BORDER_WIDTH \t\t(FILTER_SIZE/2)\n"
        "#define ROWS_PER_ITEM \t4\n"
        "#define LOCAL_W \t\t\t\tGROUP_SIZE_X\n"
        "#define LOCAL_H\t\t\t\t\t(GROUP_SIZE_Y * ROWS_PER_ITEM)\n"
        "#define GROUP_RESULT_W \t(LOCAL_W - (FILTER_SIZE-1))\n"
        "#define GROUP_RESULT_H \t(LOCAL_H - (FILTER_SIZE-1))\n"
        "\n"
        "__kernel void blur_filter(\n"
        "\t__global unsigned char* input_image, \n"
        "\t__global unsigned char* output_image,\n"
        "\tulong w,\n"
        "\tulong h,\n"
        "\t__global float* filter\n"
        "\t)\n"
        "{\n"
        "\tint x = get_local_id(0);\n"
        "\tint y = get_local_id(1);\n"
        "\tint gx = get_group_id(0);\n"
        "\tint gy = get_group_id(1);\n"

        "\n"
        "\t__local uchar local_work[LOCAL_W * PIXEL_SIZE * LOCAL_H];\n"
        "\tfor (int i = 0; i < ROWS_PER_ITEM; i++) {\n"
        "\t\tif ((gy*GROUP_RESULT_H + ROWS_PER_ITEM*y + i) < h) {\n"
        "\t\t\tuchar4 pixel = vload4((GROUP_RESULT_H*gy + y*ROWS_PER_ITEM + i)*w + GROUP_RESULT_W*gx + x, input_image);\n"
//        "\t\t\tpixel.s0 = (50*gy)%250;\n"   // b
//        "\t\t\tpixel.s1 = (pixel.s1 + 30) % 250;\n"     // g
//        "\t\t\tpixel.s2 = (50*gx)%250;\n"     // r
//        "\t\t\tpixel.s3 = 250;\n"   // a
        "\t\t\tvstore4(pixel, (y*ROWS_PER_ITEM + i)*LOCAL_W + x, local_work);\n"
        "\t\t}\n"
        "\t}\n"
//        "\t__local uchar local_result[ITEM_RESULT_H * ITEM_RESULT_W * 4];\n"
        "\tbarrier(CLK_LOCAL_MEM_FENCE);\n"

        "\t// filtering\n"
        "\tfor (int i = 0; i < ROWS_PER_ITEM; i++) {\n"
        "\t\tif ((x < GROUP_RESULT_W) && ((gx*GROUP_RESULT_W + x) < w) && ((gy*GROUP_RESULT_H + y*ROWS_PER_ITEM + i) < h)) {\n"
        "\t\t\t__private float4 result = (float4)(0);\n"
        "\t\t\tfor (int n = 0; n < FILTER_SIZE; n++) {\n"
        "\t\t\t\tfor (int m = 0; m < FILTER_SIZE; m++) {\n"
        "\t\t\t\t\t__private float filter_value = filter[n*FILTER_SIZE + m];\n"
        "\t\t\t\t\t__private uchar4 cell_value = vload4((i + y*ROWS_PER_ITEM + n)*LOCAL_W + x + m, local_work);\n"
        "\t\t\t\t\tresult += convert_float4(cell_value) * filter_value;\n"
        "\t\t\t\t}\n"
        "\t\t\t}\n"
        "\t\t\t__private uchar4 char_result = convert_uchar4(result);\n"
//        "\t\t\tif (char_result.s2 < 200) char_result.s2 += 50;\n"
        "\t\t\tvstore4(char_result, (gy*GROUP_RESULT_H + y*ROWS_PER_ITEM + i)*w + gx*GROUP_RESULT_W + x, output_image);\n"
//        "\t\t\tvstore4(char_result, (i + y*ROWS_PER_ITEM)*GROUP_RESULT_W + x, local_result);\n"
        "\t\t}\n"
        "\t}\n"
        "\t\n"

//        "\tfor (int i = 0; i < ROWS_PER_ITEM; i++) {\n"
////        "\t\t\tif ((x < ITEM_RESULT_W) && ((gx * ITEM_RESULT_W + x) < w) && ((gy*ITEM_RESULT_H + y*ROWS_PER_ITEM + i) < h)) {\n"
//        "\t\t\tif ((gy*GROUP_RESULT_H + y*ROWS_PER_ITEM + i) < h) {\n"
////        "\t\t\tfor (int j = 0; j < 4; j++) {\n"
////        "\t\t\tchar data = input_image[(gy*LOCAL_H*w + y*ROWS_PER_ITEM*w + i*w + gx*LOCAL_W + x)*4 + j];\n"
////        "\t\t\toutput_image[(gy*LOCAL_H*w + y*ROWS_PER_ITEM*w + i*w + gx*LOCAL_W + x)*4 + j] = data;\n"
////        "\t\t\t}\n"
//        "\t\t\t\tuchar4 output_pixel = (uchar4)vload4((y*ROWS_PER_ITEM + i)*LOCAL_W + x, local_work);\n"
////        "\t\t\t\toutput_pixel.s0 = 200;\n"s
//        "\t\t\t\tvstore4(output_pixel, (gy*LOCAL_H + y*ROWS_PER_ITEM + i)*w + gx*LOCAL_W + x, output_image);\n"
////        "\t\t\t\tuchar4 output_pixel = vload4((y*ROWS_PER_ITEM + i)*LOCAL_W/4 + x, local_work);\n"
////        "\t\t\t\tuchar4 output_pixel = (254, 0, 0, 100);//vload4((gy*LOCAL_H*w + y*ROWS_PER_ITEM*w + i*w + gx*LOCAL_W + x, input_image);\n"
//
////        "\t\t\t\tvstore4(output_pixel, (local_work_offset + y*ROWS_PER_ITEM + i)*w/4 + x, output_image);\n"
////        "\t\t\t\tvstore4(output_pixel, (gy*ITEM_RESULT_H + y*ROWS_PER_ITEM + i)*w + gx*ITEM_RESULT_W + x, output_image);\n"
//        "\t\t\t}\n"
//        "\t}\n"
        "}";

static size_t filter_size = 5;
static cl_context context;
static cl_command_queue queue;
static cl_kernel kernel;
static float* filter;
static cl_mem filter_buffer;

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

extern "C" JNIEXPORT jint JNICALL
Java_com_kimentii_cameraresearch_MainActivity_compileKernel(JNIEnv *env, jobject instance) {
    // init filter
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

    // normalize filter
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
    cl_device_id *devices = (cl_device_id *) malloc(sizeof(cl_device_id) * 2);
    clGetDeviceIDs(*platforms, CL_DEVICE_TYPE_GPU, 2, devices, NULL);

    // Create context and queue
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

    // Compile kernel
    size_t src_size = strlen(program_src);
    jint result;
    cl_program program = clCreateProgramWithSource(context, 1, &program_src, &src_size, &result);
    result = clBuildProgram(program, 1, devices, NULL, NULL, NULL);
    kernel = clCreateKernel(program, "blur_filter", NULL);
    return result;
}

extern "C"
JNIEXPORT jintArray JNICALL
Java_com_kimentii_cameraresearch_GrayscaleFilteringSurface_runFilter(JNIEnv *env, jobject instance,
                                                                     jintArray in_, jint width,
                                                                     jint height) {
    jint *in_bytes = env->GetIntArrayElements(in_, NULL);

    size_t pixel_size = 4;
    size_t frame_size = filter_size/2;

    cl_mem input_buffer = clCreateBuffer(
            context,
            CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
            (width * pixel_size * height),
            (void*)in_bytes,
            NULL
    );
    cl_mem output_buffer = clCreateBuffer(
            context,
            CL_MEM_WRITE_ONLY,
            width * pixel_size * height,
            NULL,
            NULL
    );

    cl_ulong raw_width = width;//width * pixel_size + frame_size*2 * pixel_size;
    cl_ulong raw_height = height;//height + frame_size*2;

    jint status;
    clSetKernelArg(kernel, 0, sizeof(cl_mem), (void*)&input_buffer);
    clSetKernelArg(kernel, 1, sizeof(cl_mem), (void*)&output_buffer);
    status = clSetKernelArg(kernel, 2, sizeof(cl_ulong), (void*)&(raw_width));
    clSetKernelArg(kernel, 3, sizeof(cl_ulong), (void*)&raw_height);
    clSetKernelArg(kernel, 4, sizeof(cl_mem), (void *) &filter_buffer);


    size_t group_x_size = 16;
    size_t group_x_pixels = group_x_size - (filter_size - 1);
    size_t group_y_size = 4;
    size_t group_y_pixels = 16 - (filter_size - 1);

    size_t groups_x = width/group_x_pixels;
    if (width % group_x_pixels != 0) groups_x++;

    size_t groups_y = height/group_y_pixels;
    if (height % group_y_pixels != 0) groups_y++;

//    printf("work size: [%ld, %ld]\n", groups_x, groups_y);
    const size_t work_size[2] = {group_x_size*groups_x, group_y_size*groups_y};
    //const size_t work_size[2] = {groups_x * 64, groups_y * 4};
    const size_t group_size[2] = {group_x_size, group_y_size};
    //const size_t group_size[2] = {64, 4};

    clEnqueueNDRangeKernel(
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
    clFinish(queue);


    jintArray newArray = env->NewIntArray(width * height);
    jint *result = env->GetIntArrayElements(newArray, NULL);
//    jint *result = (jint*)malloc(sizeof(jint) * width * pixel_size * height);

    clEnqueueReadBuffer(
            queue,
            output_buffer,
            CL_TRUE,
            0,
            width * pixel_size * height,
            (void*)result,
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

    env->ReleaseIntArrayElements(in_, in_bytes, 0);
    env->ReleaseIntArrayElements(newArray, result, NULL);

    return newArray;
}