#include "labeling.h"
#include "stdio.h"
#define BLOCK_SIZE 512
//global表示被host呼叫，但在device上執行。

__global__ void compute(const char *cuStr, int *cuPos, const int strLen) {
    //local shared memory in block
    //共有 BLOCK_SIZE 個 thread 在同一個 block 裡面
    __shared__ int local_pos[BLOCK_SIZE];

    //
    int global_index = threadIdx.x + blockIdx.x*BLOCK_SIZE;
    int local_index = threadIdx.x;

    if (global_index >= strLen) {
        return;
    }

    // thrust::tabulate，將空白部分填入索引值，反之填入 -1
    local_pos[local_index] = (cuStr[global_index] != ' ') ? -1 : local_index;
    __syncthreads();

    int max = -1;
    // thrust::inclusive_scan，對初步得到的位置資訊 P0 運行 Prefix Maximum
    //先看自己前面1個 然後看2個，每一輪需要同步等其他人算完結果。
    for (int offset = 1; offset <= local_index; offset <<= 1) {
        if (local_pos[local_index] < local_pos[local_index-offset]) {
            local_pos[local_index] = local_pos[local_index-offset];
        }
        __syncthreads();
    }
    // thrust::tabulate, sub_offset
    cuPos[global_index] = local_index - local_pos[local_index];
}
//處理 連續的一段字母橫跨 2 個 block,因為block size 設512,而string最長500
__global__ void fix(int *cuPos, const int strLen) {
    int global_index = threadIdx.x + blockIdx.x*BLOCK_SIZE;
    int local_index = threadIdx.x;

    if (global_index >= strLen) {
        return;
    }

    // 檢查local第一個就是字母的，可能就有橫跨兩個block。(如果是一開始就是字母的字串，index等於字母數)
    if (blockIdx.x > 0 && cuPos[global_index] == (local_index+1)) {
        cuPos[global_index] += cuPos[(blockIdx.x*BLOCK_SIZE)-1];
    }
}

void labeling(const char *cuStr, int *cuPos, int strLen) {
    int num_blocks =  (strLen-1)/BLOCK_SIZE +1 ;
    // 函式名稱<<<block 數目(in a grid), thread 數目(in a block), shared memory 大小>>>(參數...);

    compute<<<num_blocks, BLOCK_SIZE>>>(cuStr, cuPos, strLen);
    fix<<<num_blocks, BLOCK_SIZE>>>(cuPos, strLen);
}