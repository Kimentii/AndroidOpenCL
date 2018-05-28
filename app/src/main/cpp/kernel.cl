#define GROUP_SIZE_X		64
#define GROUP_SIZE_Y		4
#define GROUP_SIZE			(GROUP_SIZE_X * GROUP_SIZE_Y)
#define BORDER_WIDTH 		(FILTER_SIZE/2)
#define ROWS_PER_GROUP 	4
#define ROWS_PER_ITEM 	10
#define LOCAL_W 				GROUP_SIZE_X
#define LOCAL_H					(GROUP_SIZE_Y * ROWS_PER_ITEM)
#define ITEM_RESULT_W 	(LOCAL_W - (FILTER_SIZE-1))
#define ITEM_RESULT_H 	(LOCAL_H - (FILTER_SIZE-1))

__kernel void kernel_fun(
	__global unsigned char* input_image, 
	__global unsigned char* output_image,
	long w,
	long h,
	long p,
	__global float* global_filter
	)
{
	int x = get_local_id(0);
	int y = get_local_id(1);
	int gx = get_group_id(0);
	int gy = get_group_id(1);
	__local float filter[FILTER_SIZE * FILTER_SIZE];
	int filter_copy_times = FILTER_SIZE*FILTER_SIZE/GROUP_SIZE + 1;
	for (int i = 0; i < filter_copy_times; i++) {
		if ((x + y*GROUP_SIZE_X + i*GROUP_SIZE) < FILTER_SIZE*FILTER_SIZE) {
			filter[x + y*GROUP_SIZE_X + i*GROUP_SIZE] = global_filter[x + y*GROUP_SIZE_X + i*GROUP_SIZE];
		}
	}

	__local uchar local_work[LOCAL_W * 3 * LOCAL_H];
	long local_work_offset = ITEM_RESULT_H*gy*p + ITEM_RESULT_W*gx*3;
	for (int i = 0; i < ROWS_PER_ITEM; i++) {
		if ((gy*ITEM_RESULT_H + ROWS_PER_ITEM*y + i) < (h + BORDER_WIDTH*2)) {
			//local_work[(y*ROWS_PER_ITEM + i)*LOCAL_W + x] = input_image[local_work_offset + (y*ROWS_PER_ITEM + i)*p + x];
			uchar3 item_data = vload3(local_work_offset/3 + (y*ROWS_PER_ITEM + i)*p/3 + x, input_image);
			vstore3(item_data, (y*ROWS_PER_ITEM + i)*LOCAL_W + x, local_work);
		}
	}
	__local uchar local_result[ITEM_RESULT_H * ITEM_RESULT_W * 3];
	barrier(CLK_LOCAL_MEM_FENCE);
	// filtering
	for (int i = 0; i < ROWS_PER_ITEM; i++) {
		if ((x < ITEM_RESULT_W) && ((gx*ITEM_RESULT_W + x) < w*3) && ((gy*ITEM_RESULT_H + y*ROWS_PER_ITEM + i) < h)) {
			__private float3 result = (float3)(0);
			for (int n = 0; n < FILTER_SIZE; n++) {
				for (int m = 0; m < FILTER_SIZE; m++) {
					__private float filter_value = filter[n*FILTER_SIZE + m];
					__private uchar3 cell_value = vload3((i + y*ROWS_PER_ITEM + n)*LOCAL_W + x + m, local_work);
					result += convert_float3(cell_value) * filter_value;
				}
			}
			__private uchar3 char_result = convert_uchar3(result);
			vstore3(char_result, (i + y*ROWS_PER_ITEM)*ITEM_RESULT_W + x, local_result);
		}
	}
	
	for (int i = 0; i < ROWS_PER_ITEM; i++) {
			if ((x < ITEM_RESULT_W) && ((gx * ITEM_RESULT_W + x) < w) && ((gy*ITEM_RESULT_H + y*ROWS_PER_ITEM + i) < h)) {
				//uchar3 output_pixel = vload3(y*LOCAL_W + x, local_work);
				//vstore3(output_pixel, (gy*ITEM_RESULT_H + y*ROWS_PER_ITEM + i)*w + gx*ITEM_RESULT_W + x, output_image);
				uchar3 output_pixel = vload3((y*ROWS_PER_ITEM + i)*ITEM_RESULT_W + x, local_result);
				vstore3(output_pixel, (gy*ITEM_RESULT_H + y*ROWS_PER_ITEM + i)*w + gx*ITEM_RESULT_W + x, output_image);
			}
	}
}
