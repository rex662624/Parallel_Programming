#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>
#include <sched.h>
#include <assert.h>

#define MAXM 5000001
#define MAXN 10005
int buffer1[MAXM];
int buffer2[MAXM];

int Weight[MAXN], Value[MAXN];
#define max(a, b) (a > b ? a : b)


int main() {

//===========================================
    omp_set_num_threads(2);
    #pragma omp parallel
    {
        cpu_set_t cpuset;
        CPU_ZERO(&cpuset);
        for (int i = 0; i < 6; i++)
            CPU_SET(i, &cpuset);
        assert(sched_setaffinity(0, sizeof(cpuset), &cpuset) == 0);
    }
//===========================================

    int N, M;
    while (scanf("%d %d", &N, &M) == 2) {

        memset(buffer1, 0, sizeof(buffer1));
        memset(buffer2, 0, sizeof(buffer2));

        for (int i = 0; i < N; i++)
            scanf("%d %d", &Weight[i], &Value[i]);

        int count = 0;
        for (count = 0; count < N; count++) {
            #pragma omp parallel
            {
                int v = Value[count], w = Weight[count];
                #pragma omp for
                for (int i = w; i <= M; i++)
                    if(count%2==0)
                        buffer2[i] = max(buffer1[i-w]+v, buffer1[i]);//放了這東西or不放（直接取表格的重量）
                    else
                        buffer1[i] = max(buffer2[i-w]+v, buffer2[i]);
                #pragma omp for
                for (int i = 0; i < w; i++)
                    if(count%2==0)
                        buffer2[i] = buffer1[i];
                    else
                        buffer1[i] = buffer2[i];
            }
        }

        if(count%2==0)
            printf("%d\n",buffer1[M]);
        else
            printf("%d\n",buffer2[M]);

    }
    return 0;
}