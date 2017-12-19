#include <stdio.h>
#include<omp.h>
#include<stdlib.h>
#include<unistd.h>
#include<time.h>


int main(int argc, char* argv[]){
	
	
	srand(time(NULL));
	printf("how many number do you want to sort?\n");
	long int n; 
	scanf("%ld",&n);
//************************製造亂數數字*******************************	
	//一般的random函數會取得重複的值 所以做一些操作
		
	int *a = malloc(sizeof(int)*n);;
	long int k ;
	for(k=0;k<n;k++)
	{
		a[k] = (rand()+k*8-5)%32767;//讓數字控制在32767內，並做一些操作以免亂數有許多重複;
	}
	     	
//***********************開始做count sort**************************
	int thread_count = strtol(argv[1], NULL, 10);
	int i , j , count ;
	int * temp = malloc(sizeof(int)*n);
//	clock_t start = clock();//開始計時
	double start = omp_get_wtime();
	#pragma omp parallel for num_threads(thread_count)\
	default(none) private(i, j, count) shared(a, n, temp, thread_count)      
	for (i = 0; i < n; i++) {
		count = 0;

		for (j = 0; j < n; j++)
			if (a[j] < a[i])
				count++;
			else if (a[j] == a[i] && j < i)
				count++;
		
		temp[count] = a[i];
	}

	memcpy ( a , temp, n * sizeof(int));
	free(temp );
	double finish = omp_get_wtime();
	//	clock_t end = clock();
//****************************print 出sort好的list
	for(k=1;k<=n;k++)
	{	
		printf("%5d ",a[k-1]);		
		if(k%10==0)printf("\n");	
	}

	printf("\nThe execution time = %lf\n",finish - start);
//	float seconds = (float)(end - start) / CLOCKS_PER_SEC;
//	printf("\nThe execution time = %lf\n", seconds);
	return 0 ;
}

