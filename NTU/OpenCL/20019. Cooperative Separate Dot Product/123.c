#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <CL/cl.h>
#include <omp.h>
#include <inttypes.h>
 
#define MAXGPU 2
#define MAXK 2048
#define MAXN 1073741824
#define uint32_t unsigned int
#define MAXproblemSize 10000
 
int local_work_size =1024;
int num_item_a_group=256;
int problem_count=0;
 
int main(int argc, char *argv[]) {
    cl_int status;
    cl_platform_id platform_id;
    cl_uint platform_id_got;
    status = clGetPlatformIDs(1, &platform_id, &platform_id_got);
    assert(status == CL_SUCCESS && platform_id_got == 1);
 
    cl_device_id GPU[MAXGPU];
    cl_uint GPU_id_got;
    status = clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_GPU, MAXGPU, GPU, &GPU_id_got);
    assert(status == CL_SUCCESS);
 
    /* getcontext */
    cl_context context = clCreateContext(NULL, MAXGPU, GPU, NULL, NULL, &status);
    assert(status == CL_SUCCESS);
 
    /* commandqueue */
    cl_command_queue commandQueues[MAXGPU];
    for (int i = 0; i < MAXGPU; i++) {
        commandQueues[i] = clCreateCommandQueue(context, GPU[i], CL_QUEUE_PROFILING_ENABLE, &status);
        assert(status == CL_SUCCESS);
    }
 
    /* kernelsource */
    FILE *kernelfp = fopen("vecdot.cl", "r");
    assert(kernelfp != NULL);
    char kernelBuffer[MAXK];
    const char *constKernelSource = kernelBuffer;
    size_t kernelLength = fread(kernelBuffer, 1, MAXK, kernelfp);
    cl_program program = clCreateProgramWithSource(context, 1, &constKernelSource, &kernelLength, &status);
    assert(status == CL_SUCCESS);
 
    /* buildprogram */
    status = clBuildProgram(program, MAXGPU, GPU, NULL, NULL, NULL);
    assert(status == CL_SUCCESS);
    if (status != CL_SUCCESS)
    {
    char errorMessage[2048];
    status = clGetProgramBuildInfo(program, GPU[0], CL_PROGRAM_BUILD_LOG, sizeof(errorMessage), errorMessage, NULL);
    // assert(status == CL_SUCCESS);
    printf("%s", errorMessage);
    }
 
    /* createkernel */
    cl_kernel kernels[MAXGPU];
    for (int i = 0; i < MAXGPU; i++) {
        kernels[i] = clCreateKernel(program, "DotProduct", &status);
        assert(status == CL_SUCCESS);
    }
 
    /* vector */
    size_t buf_size = (MAXN / num_item_a_group) / local_work_size;
    cl_uint* out[MAXGPU];
    for (int i = 0; i < MAXGPU; i++) {
        out[i] = (cl_uint*)malloc(buf_size * sizeof(cl_uint));
        assert(out[i] != NULL);
    }
 
    /* createbuffer */
    cl_mem bufferOut[MAXGPU];
    for (int i = 0; i < MAXGPU; i++) {
        bufferOut[i] = clCreateBuffer(context, CL_MEM_WRITE_ONLY | CL_MEM_USE_HOST_PTR, buf_size * sizeof(cl_uint), out[i], &status);
        assert(status == CL_SUCCESS);
    }
 
    int N[MAXproblemSize];
    uint32_t key1[MAXproblemSize], key2[MAXproblemSize], ans[MAXproblemSize]={0,};
 
    while (scanf("%d %" PRIu32 " %" PRIu32, &N[problem_count], &key1[problem_count], &key2[problem_count]) == 3) {
        problem_count++;
    }
 
    size_t nb_workgroups[MAXGPU];
 
    omp_set_num_threads(MAXGPU);//一個thread 就掌管一個GPU
 
    #pragma omp parallel for schedule(dynamic, 1) private(status)
    for (int j = 0; j < problem_count; j++) {
        int i = omp_get_thread_num();
 
        status = clSetKernelArg(kernels[i], 0, sizeof(uint32_t), (void *)&key1[j]);
        assert(status == CL_SUCCESS);
        status = clSetKernelArg(kernels[i], 1, sizeof(uint32_t), (void *)&key2[j]);
        assert(status == CL_SUCCESS);
        status = clSetKernelArg(kernels[i], 2, sizeof(cl_mem), (void *)&bufferOut[i]);
        assert(status == CL_SUCCESS);
        status = clSetKernelArg(kernels[i], 3, sizeof(int), (void *)&local_work_size);
        assert(status == CL_SUCCESS);
        status = clSetKernelArg(kernels[i], 4, sizeof(int), (void *)&N[j]);
        assert(status == CL_SUCCESS);
 
        size_t num_of_work_items = (N[j] - 1) / local_work_size + 1;
        num_of_work_items = (num_of_work_items % num_item_a_group == 0) ? num_of_work_items : ((num_of_work_items / num_item_a_group) + 1) * num_item_a_group;
        size_t num_of_work_groups = num_of_work_items/num_item_a_group;
        size_t globalThreads[] = {(size_t)num_of_work_items};//代表work-item總數（然後一個work-item裡面有local_work_size個數字要計算）
        size_t localThreads[] = {num_item_a_group};//代表分成256個work_items組成一個work group
        cl_event event;
        status = clEnqueueNDRangeKernel(commandQueues[i], kernels[i], 1, NULL, globalThreads, localThreads, 0, NULL, &event);
        assert(status == CL_SUCCESS);
 
        clWaitForEvents(1, &event);
 
        clEnqueueReadBuffer(commandQueues[i], bufferOut[i], CL_TRUE, 0, num_of_work_groups * sizeof(cl_uint), out[i], 0, NULL, NULL);
 
        uint32_t sum = 0;
        for (int k = 0; k < num_of_work_groups; k++)
            ans[j] += out[i][k];
 
        
 
    }
 
    for (int i = 0; i < problem_count; i++)
        printf("%" PRIu32 "\n", ans[i]);
 
    /* free */
    for (int i = 0; i < MAXGPU; i++) {
        free(out[i]);
        clReleaseCommandQueue(commandQueues[i]);
        clReleaseMemObject(bufferOut[i]);
        clReleaseKernel(kernels[i]);
    }
    clReleaseContext(context);
    clReleaseProgram(program);
 
    return 0;
}