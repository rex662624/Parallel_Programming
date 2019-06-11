#include <stdio.h>
#include <assert.h>
#include <inttypes.h>
#include <string.h>
#include <cuda.h>
#define MAXN 1024
#define MaxProblem 1024
#define BLOCK_SIZE 512
 
#define UINT uint32_t
uint32_t hostMtx[2][MAXN*MAXN];
uint32_t Ret[2][MAXN*MAXN];
int problemindex=0;
int N;
//======================================================
__global__ void matrixAdd( int N,UINT* A, UINT* B, UINT* C){
    int r = blockIdx.x;
    int c = threadIdx.x;
    int ptr = r*N + c;
 
    C[ptr] = A[ptr] + B[ptr];
}
 
__global__ void matrixMul(  int N,UINT* A, UINT* B, UINT* C){
    int r = blockIdx.x;
    int c = threadIdx.x;
    int ptr = r*N + c;
 
    UINT sum = 0;
    for(int k=0; k<N; k++)
        sum += A[r*N + k] * B[k*N + c];
 
    C[ptr] = sum;
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
 
    uint32_t S[MaxProblem][64],TotalN[MaxProblem];
    while(scanf("%d", &TotalN[problemindex]) == 1){
 
    for (int i = 0; i < 2; i++) {
        scanf("%d", &S[problemindex][i]);
        }
        problemindex++;
    }
 
 
    //readIn();
    uint32_t *cuIN[2], *cuTmp[6];
    uint32_t memSz = MAXN*MAXN*sizeof(uint32_t);
    for (int i = 0; i < 2; i++) {
        cudaMalloc((void **) &cuIN[i], memSz);
    }
    for (int i = 0; i < 6; i++)
        cudaMalloc((void **) &cuTmp[i], memSz);
 
    for(int index=0;index<problemindex;index++){
 
        N=TotalN[index];
        #pragma omp parallel for
        for (int i = 0; i < 2; i++) {
            rand_gen(S[index][i], N,  hostMtx[i]);
            cudaMemcpy(cuIN[i], hostMtx[i], memSz, cudaMemcpyHostToDevice);
        }
        int num_blocks =  (N*N-1)/BLOCK_SIZE +1 ;
        // AB
        //multiply(cuIN[0], cuIN[1], cuTmp[0]);
        matrixMul<<<N, N>>>(N, cuIN[0], cuIN[1], cuTmp[0]);
        // BA
        //multiply(cuIN[1], cuIN[0], cuTmp[1]);
        matrixMul<<<N, N>>>(N, cuIN[1], cuIN[0], cuTmp[1]);
        //AB+BA
        //add(cuTmp[0], cuTmp[1], cuTmp[2]);
        matrixAdd<<<N, N>>>(N, cuTmp[0], cuTmp[1], cuTmp[2]);
 
        // ABA
        //multiply(cuTmp[0], cuIN[0], cuTmp[3]);
        matrixMul<<<N, N>>>(N, cuTmp[0], cuIN[0], cuTmp[3]);
        // BAB
        //multiply(cuTmp[1], cuIN[1], cuTmp[4]);
        matrixMul<<<N, N>>>(N, cuTmp[1], cuIN[1], cuTmp[4]);
        //ABA+BAB
        //add(cuTmp[3], cuTmp[4], cuTmp[5]);
        matrixAdd<<<N, N>>>(N, cuTmp[3], cuTmp[4], cuTmp[5]);
 
        cudaMemcpy(Ret[0], cuTmp[2], memSz, cudaMemcpyDeviceToHost);
        cudaMemcpy(Ret[1], cuTmp[5], memSz, cudaMemcpyDeviceToHost);
 
        uint32_t ret[2];
 
        #pragma omp parallel for
        for (int i = 0; i < 2; i++) {
            ret[i] = signature(N, Ret[i]);
        }
 
        for (int i = 0; i < 2; i++)
            printf("%u\n", ret[i]);
 
}
 
 
    for (int i = 0; i < 2; i++)
        cudaFree(cuIN[i]);
    for (int i = 0; i < 6; i++)
        cudaFree(cuTmp[i]);
    return 0;
}