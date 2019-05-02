#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include "utils.h"
#include <pthread.h>
#include <assert.h>
 
#define MAXN 10000005
#define MAX_THREAD 6
uint32_t prefix_sum[MAXN];
 
int n;
uint32_t key;
int keepnumber[MAX_THREAD];//每個人處理幾個數字
int previousSum[MAX_THREAD];
 
void* subtask1(void *rank) {
    long int myrank = (long int ) rank;
 
    int upper=0;
    int lower=0;
 
/*  
    int block = n / MAX_THREAD ;   
    if(myrank<(n%MAX_THREAD))block++;
C
    for(int i=0;i<myrank ;i++){
        if(i<(n%MAX_THREAD))
            lower += ((n / MAX_THREAD)+1);
        else
            lower += (n / MAX_THREAD);
    }
    if(myrank>0)lower++;
    upper = lower +block;
*/
    for(int i =0;i<myrank;i++)
        lower +=  keepnumber[i];
    upper = lower + keepnumber[myrank];
 
//    printf("%d , %d\n",lower , upper-1);
    int i =0;
    uint32_t sum = 0;
    for (i = lower; i <= upper-1; i++) {
        sum += encrypt(i+1, key);
        prefix_sum[i+1] = sum;
    }
    previousSum[myrank]=prefix_sum[i];
 
}
 
void* subtask2(void *rank) {
    long int myrank = (long int ) rank;                                                                                                   
     int upper=0;
     int lower=0;
    for(int i =0;i<myrank;i++)
         lower +=  keepnumber[i];
     upper = lower + keepnumber[myrank];
	int total = 0; 
    for (int i =0;i<myrank;i++)
		total+=previousSum[i];
//        printf("%d ",previousSum[i]);

    for (int j = lower; j < upper; j++) {
        prefix_sum[j+1]+=total;
    }
    
//    printf("\n");
 
 
 
}
 
int main() {
 
        {
                cpu_set_t cpuset;
                CPU_ZERO(&cpuset);
                for (int i = 0; i < 6; i++)
                        CPU_SET(i, &cpuset);
                assert(sched_setaffinity(0, sizeof(cpuset), &cpuset) == 0);
        }      
 
      while (scanf("%d %" PRIu32, &n, &key) == 2) {
        pthread_t threads[MAX_THREAD];
         int rem = 0;
         long int i;
         for(i=0;i<MAX_THREAD;i++)
          {  
              keepnumber[i]=n/MAX_THREAD;
              rem=n%MAX_THREAD;
             if(rem!=0&&i<rem)
              ++keepnumber[i];
          }
 
        for ( i = 0; i < MAX_THREAD; i++) {
            pthread_create(&threads[i], NULL, subtask1,(void*)i);
        }
 
           for (int i = 0; i < MAX_THREAD; i++)
            pthread_join(threads[i], NULL);
 
 
 
 
//        for(int i =0;i<MAX_THREAD;i++)
//            printf("%d ",previousSum[i]);
//        printf("\n");
        //================================
        for (i = 0; i < MAX_THREAD; i++) {
            pthread_create(&threads[i], NULL, subtask2,(void*)i);
         }          
         for (int i = 0; i < MAX_THREAD; i++)
            pthread_join(threads[i], NULL);
 
//         for(int i =0;i<MAX_THREAD;i++)
//            printf("%d ",prefix_sum[i]);
 
/*
        uint32_t sum = 0;
        for (int i = 1; i <= n; i++) {
            sum += encrypt(i, key);
            prefix_sum[i] = sum;
        }
 */  
           output(prefix_sum, n);
 
    }
    return 0;
}
