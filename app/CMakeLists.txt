cmake_minimum_required(VERSION 3.4.1)

add_library( opencl-blur SHARED src/main/cpp/opencl-blur.cpp)

# TODO: Set correct native opencl library here
set(OPENCL_RUNTIME_LIB /usr/lib/libGLES_mali.so)
add_library(opencl-runtime SHARED IMPORTED)
set_property(TARGET opencl-runtime PROPERTY IMPORTED_LOCATION ${OPENCL_RUNTIME_LIB})

target_link_libraries(opencl-blur opencl-runtime android jnigraphics)