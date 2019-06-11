#include "labeling.h"
#include <thrust/tabulate.h>
#include <thrust/scan.h>
#include <thrust/functional.h>
#include <thrust/execution_policy.h>

template<class T> struct MM {
	//如果是數字就輸出-1 其他就輸出它本來的index,nowbase:cuStr

	const char *base;
	MM(const char *base): base(base) {}
	__host__ __device__ T operator()(const T& index) const { return (base[index] != ' ') ? -1 : index; };
};
template<class T> struct NN {
	//將每個元素拿索引值減去目前array的值，得答案
	int32_t *base;
	NN(int32_t *base): base(base) {}
	__host__ __device__ T operator()(const T& index) const { return index-base[index]; };
};



void labeling(const char *cuStr, int *cuPos, int strLen) {
	thrust::tabulate(thrust::device, cuPos, cuPos+strLen, MM<int32_t>(cuStr));//cuStr做MM轉換之後存進cuPos
	thrust::inclusive_scan(thrust::device, cuPos, cuPos+strLen, cuPos, thrust::maximum<int>());
	//第二步結束後array值: 如果是空白：我的index 如果是字：最後一個空白的index是誰
	thrust::tabulate(thrust::device, cuPos, cuPos+strLen, NN<int32_t>(cuPos));
}