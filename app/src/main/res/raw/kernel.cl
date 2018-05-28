
#define PIXEL_SIZE  4
#define GROUP_SIZE_X 16
#define GROUP_SIZE_Y 4
#define ROWS_PER_ITEM 4
#define LOCAL_W GROUP_SIZE_X
#define LOCAL_H (GROUP_SIZE_Y * ROWS_PER_ITEM)
#define GROUP_RESULT_W (LOCAL_W - (FILTER_SIZE-1))
#define GROUP_RESULT_H (LOCAL_H - (FILTER_SIZE-1))
__kernel void blur_filter(
                __global unsigned char* input_image,
                __global unsigned char* output_image,
                ulong w,
                ulong h,
                __global float* filter) {
        int x = get_local_id(0);
        int y = get_local_id(1);
        int gx = get_group_id(0);
        int gy = get_group_id(1);


        __local uchar local_work[LOCAL_W * PIXEL_SIZE * LOCAL_H];
        for (int i = 0; i < ROWS_PER_ITEM; i++) {
                if ((gy*GROUP_RESULT_H + ROWS_PER_ITEM*y + i) < h) {
                        uchar4 pixel = vload4((GROUP_RESULT_H*gy + y*ROWS_PER_ITEM + i)*w + GROUP_RESULT_W*gx + x, input_image);
                        //            pixel.s0 = (50*gy)%250;                 // b
                        //            pixel.s1 = (pixel.s1 + 30) % 250;       // g
                        //            pixel.s2 = (50*gx)%250;                 // r
                        //            pixel.s3 = 250;                         // a
                        vstore4(pixel, (y*ROWS_PER_ITEM + i)*LOCAL_W + x, local_work);
                }
        }
        barrier(CLK_LOCAL_MEM_FENCE);
        

        // filtering
        for (int i = 0; i < ROWS_PER_ITEM; i++) {
                if ((x < GROUP_RESULT_W) && ((gx*GROUP_RESULT_W + x) < w) && ((gy*GROUP_RESULT_H + y*ROWS_PER_ITEM + i) < h)) {
                        __private float4 result = (float4)(0);
                        for (int n = 0; n < FILTER_SIZE; n++) {
                                for (int m = 0; m < FILTER_SIZE; m++) {
                                        __private float filter_value = filter[n*FILTER_SIZE + m];
                                        __private uchar4 cell_value = vload4((i + y*ROWS_PER_ITEM + n)*LOCAL_W + x + m, local_work);
                                        result += convert_float4(cell_value) * filter_value;
                                }
                        }
                        __private uchar4 char_result = convert_uchar4(result);
                        //            if (char_result.s2 < 200) char_result.s2 += 50;
                        char_result.s2 = (char_result.s2*2) % 250;
                        vstore4(char_result, (gy*GROUP_RESULT_H + y*ROWS_PER_ITEM + i)*w + gx*GROUP_RESULT_W + x, output_image);
                }
        }
};
