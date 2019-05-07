#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <CL/cl.h>
#include <omp.h>
#include <inttypes.h>

#define MAXGPU 1
#define MAXK 2048
#define MAXN 67108864
#define uint32_t unsigned int

int N;
uint32_t key1, key2;
int local_work_size =1024 ;
int num_item_a_group=256;

cl_context context;
cl_program program;
cl_command_queue commandQueue;
cl_kernel kernel;
#define paralist cl_context *context, cl_program *program, cl_command_queue *commandQueue, cl_kernel *kernel
#define arglist &context, &program, &commandQueue, &kernel

//=========need to modify
void InitialOpenCLFunction(paralist)
{

  cl_int status;
  cl_platform_id platform_id;
  cl_uint platform_id_got;
  status = clGetPlatformIDs(1, &platform_id, &platform_id_got);
  //assert(status == CL_SUCCESS && platform_id_got == 1);

  cl_device_id GPU[MAXGPU];
  cl_uint GPU_id_got;
  status = clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_GPU, MAXGPU, GPU, &GPU_id_got);
  // assert(status == CL_SUCCESS);

  /* getcontext */
  *context = clCreateContext(NULL, 1, GPU, NULL, NULL, &status);
  //assert(status == CL_SUCCESS);
  /* commandqueue */
  *commandQueue = clCreateCommandQueue(*context, GPU[0], CL_QUEUE_PROFILING_ENABLE, &status);
  //assert(status == CL_SUCCESS);

  /* kernelsource */
  //readinkernelfile
  // assert(scanf("%s", fileName) == 1);
  FILE *kernelfp = fopen("vecdot.cl", "r");
  //assert(kernelfp != NULL);
  char kernelBuffer[MAXK];
  const char *constKernelSource = kernelBuffer;
  size_t kernelLength = fread(kernelBuffer, 1, MAXK, kernelfp);
  //printf("The size of kernel source is %zu\n", kernelLength);
  *program = clCreateProgramWithSource(*context, 1, &constKernelSource, &kernelLength, &status);

  //assert(status == CL_SUCCESS);
  /* buildprogram */
  status = clBuildProgram(*program, 1, GPU, NULL, NULL, NULL);
  if (status != CL_SUCCESS)
  {
    char errorMessage[2048];
    status = clGetProgramBuildInfo(*program, GPU[0], CL_PROGRAM_BUILD_LOG, sizeof(errorMessage), errorMessage, NULL);
    // assert(status == CL_SUCCESS);
    printf("%s", errorMessage);
  }

  /* createkernel */
  *kernel = clCreateKernel(*program, "DotProduct", &status);
  assert(status == CL_SUCCESS);
  //printf("Build kernel completes\n");

  /* vector */

  /* createbuffer */ 
  uint32_t *out = (uint32_t *)malloc(MAXN * sizeof(uint32_t));
  assert(out != NULL);

  cl_mem bufferOut = clCreateBuffer(*context, CL_MEM_WRITE_ONLY | CL_MEM_USE_HOST_PTR, MAXN * sizeof(uint32_t), out, &status);
  assert(status == CL_SUCCESS);
  //printf("Build buffers completes\n");

  while (scanf("%d %" PRIu32 " %" PRIu32, &N, &key1, &key2) == 3)
  {
    /* setarg */
    status = clSetKernelArg(*kernel, 0, sizeof(cl_uint), (void *)&key1);
    assert(status == CL_SUCCESS);
    status = clSetKernelArg(*kernel, 1, sizeof(cl_uint), (void *)&key2);
    assert(status == CL_SUCCESS);
    status = clSetKernelArg(*kernel, 2, sizeof(cl_mem), (void *)&bufferOut);
    assert(status == CL_SUCCESS);
    status = clSetKernelArg(*kernel, 3, sizeof(int), (void *)&local_work_size);
    assert(status == CL_SUCCESS);
    status = clSetKernelArg(*kernel, 4, sizeof(int), (void *)&N);
    assert(status == CL_SUCCESS);
    //printf("Set kernel arguments completes\n");

    /* setshape */
    int num_of_work_items =  (N-1)/local_work_size +1 ;
    //N = (N % local_work_size == 0) ? N : ((N / local_work_size) + 1) * local_work_size;
    //padding到256的倍數,保持num_of_work_items是localThreads的整數倍（為了num_of_work_items讓可以整除localThreads）
    num_of_work_items = (num_of_work_items % num_item_a_group == 0) ? num_of_work_items : ((num_of_work_items / num_item_a_group) + 1) * num_item_a_group;

    size_t num_of_work_groups = num_of_work_items/num_item_a_group;

    size_t globalThreads[] = {(size_t)num_of_work_items};//代表work-item總數（然後一個work-item裡面有local_work_size個數字要計算）
    size_t localThreads[] = {num_item_a_group};//代表分成256個work_items組成一個work group
    cl_event event;
    status = clEnqueueNDRangeKernel(*commandQueue, *kernel, 1, NULL, globalThreads, localThreads, 0, NULL,  &event);
    assert(status == CL_SUCCESS);
    clWaitForEvents(1, &event); 
    //printf("Kernel execution completes.\n");
    //printf("Specify the shape of the domain completes.\n");

    /* getcvector */
    clEnqueueReadBuffer(*commandQueue, bufferOut, CL_TRUE, 0, num_of_work_groups * sizeof(cl_uint), out, 0, NULL, NULL);
    //printf("Kernel execution completes.\n");
    
    uint32_t sum = 0 ;
    for (int i = 0; i < num_of_work_groups; i++)
      sum += out[i];

    printf("%"PRIu32"\n", sum);

  }

  free(out);
  clReleaseContext(*context);
  clReleaseCommandQueue(*commandQueue);
  clReleaseProgram(*program);
  clReleaseKernel(*kernel);
  clReleaseMemObject(bufferOut);
}

int main(int argc, char *argv[])
{
  InitialOpenCLFunction(arglist);
  return 0;
}
