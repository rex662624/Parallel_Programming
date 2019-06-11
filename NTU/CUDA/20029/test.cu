#include <stdio.h>
#include <assert.h>
#include <inttypes.h>
#include <string.h>
#include <cuda.h>
#include <omp.h>
#define MAXN 1024
#define MaxProblem 1024
#define BLOCK_SIZE 512
#define ThreadNumber 2 
#define UINT uint32_t

uint32_t hostMtx[ThreadNumber][2][MAXN*MAXN];
uint32_t Ret[ThreadNumber][2][MAXN*MAXN];
uint32_t ANS[MaxProblem][2];
int problemindex=0;
//======================================================
__global__ void matrixAdd( int N,UINT* A, UINT* B, UINT* C){
    int row = blockIdx.x;
    int col = threadIdx.x;
 
    C[row*N + col] = A[row*N + col] + B[row*N + col];
}
 
__global__ void matrixMul(  int N,UINT* A, UINT* B, UINT* C){
    int row = blockIdx.x;
    int col = threadIdx.x;
 
    UINT sum = 0;
    for(int k=0; k<N; k++)
        sum += A[row*N + k] * B[k*N + col];
 
    C[row*N + col] = sum;
}
 
//============================================
 
void rand_gen(UINT c, int N, UINT A[MAXN*MAXN]) {
    UINT x = 2, n = N*N;
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            x = (x * x + c + i + j)%n;
            A[i*N+j] = x;
        }
    }
}
 
UINT signature(int N, UINT A[MAXN*MAXN]) {
    UINT h = 0;
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++)
            h = (h + A[i*N+j]) * 2654435761LU;
    }
    return h;
}
//==========================================================
int main(int argc, char *argv[]) {

    omp_set_num_threads(ThreadNumber);
    uint32_t S[MaxProblem][64],TotalN[MaxProblem];
    while(scanf("%d", &TotalN[problemindex]) == 1){
 
    for (int i = 0; i < 2; i++) {
        scanf("%d", &S[problemindex][i]);
        }
        problemindex++;
    }
 
    
    //readIn();
    uint32_t *cuIN[ThreadNumber][2], *cuTmp[ThreadNumber][6];
    uint32_t memSz = MAXN*MAXN*sizeof(uint32_t);
    for(int k=0;k<ThreadNumber;k++){
        
        for (int i = 0; i < 2; i++) {
            cudaSetDevice(k);
            cudaMalloc((void **) &cuIN[k][i], memSz);
        }

        for (int i = 0; i < 6; i++){
            cudaSetDevice(k);
            cudaMalloc((void **) &cuTmp[k][i], memSz);
        }
    }

    #pragma omp parallel for schedule(dynamic , 1)
    for(int index=0;index<problemindex;index++){
        int pid = omp_get_thread_num();
        cudaSetDevice(pid);
        int N=TotalN[index];

        for (int i = 0; i < 2; i++) {
            rand_gen(S[index][i], N,  hostMtx[pid][i]);
            cudaMemcpy(cuIN[pid][i], hostMtx[pid][i], memSz, cudaMemcpyHostToDevice);
        }
        // AB
        //multiply(cuIN[0], cuIN[1], cuTmp[0]);
        matrixMul<<<N, N>>>(N, cuIN[pid][0], cuIN[pid][1], cuTmp[pid][0]);
        // BA
        //multiply(cuIN[1], cuIN[0], cuTmp[1]);
        matrixMul<<<N, N>>>(N, cuIN[pid][1], cuIN[pid][0], cuTmp[pid][1]);
        //AB+BA
        //add(cuTmp[0], cuTmp[1], cuTmp[2]);
        matrixAdd<<<N, N>>>(N, cuTmp[pid][0], cuTmp[pid][1], cuTmp[pid][2]);
 
        // ABA
        //multiply(cuTmp[0], cuIN[0], cuTmp[3]);
        matrixMul<<<N, N>>>(N, cuTmp[pid][0], cuIN[pid][0], cuTmp[pid][3]);
        // BAB
        //multiply(cuTmp[1], cuIN[1], cuTmp[4]);
        matrixMul<<<N, N>>>(N, cuTmp[pid][1], cuIN[pid][1], cuTmp[pid][4]);
        //ABA+BAB
        //add(cuTmp[3], cuTmp[4], cuTmp[5]);
        matrixAdd<<<N, N>>>(N, cuTmp[pid][3], cuTmp[pid][4], cuTmp[pid][5]);
 
        cudaMemcpy(Ret[pid][0], cuTmp[pid][2], memSz, cudaMemcpyDeviceToHost);
        cudaMemcpy(Ret[pid][1], cuTmp[pid][5], memSz, cudaMemcpyDeviceToHost);

        for (int i = 0; i < 2; i++) {
            ANS[index][i] = signature(N, Ret[pid][i]);
        }
}
 
for(int index=0;index<problemindex;index++){
    for (int i = 0; i < 2; i++)
        printf("%u\n", ANS[index][i]);
    }

 for(int k =0;k<ThreadNumber;k++){
    for (int i = 0; i < 2; i++)
        cudaFree(cuIN[k][i]);
    for (int i = 0; i < 6; i++)
        cudaFree(cuTmp[k][i]);
    }
    return 0;
}