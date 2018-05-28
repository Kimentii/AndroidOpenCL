cmake_minimum_required(VERSION 3.4.1)

add_library( opencl-blur SHARED src/main/c/opencl-blur.c)

# Set correct native OpenCL library path here
# It must be native one, pulled from target android device
# Usually on device it's located at /system/lib or /system/vendor/lib
# Use 'strings' utility to check that it contains OpenCL symbols
set(OPENCL_RUNTIME_LIB /usr/lib/libGLES_mali.so)
add_library(opencl-runtime SHARED IMPORTED)
set_property(TARGET opencl-runtime PROPERTY IMPORTED_LOCATION ${OPENCL_RUNTIME_LIB})

target_link_libraries(opencl-blur opencl-runtime)