#include<stdio.h>
#include<mpi.h>
#include <time.h>
#include <stdlib.h>

int main (int argc, char *argv[]) {
	int comm_sz,my_rank;

	MPI_Init(NULL,NULL);
	MPI_Comm_size(MPI_COMM_WORLD,&comm_sz);
	MPI_Comm_rank(MPI_COMM_WORLD,&my_rank);
	
	double x,y,pi_estimate;
	long long int toss,number_of_tosses=0,number_in_circle=0 ,temp=0;
	srand( time(NULL) );
	int i,half;
	double startwtime = 0.0,endwtime = 0.0;

	if(my_rank==0)//processor 0 問要擲多少次，而後傳給其他process
	{
		printf("How many darts do you want to toss(per process):");
		scanf("%lli",&number_of_tosses);
		for(i=1;i<comm_sz;i++)
			MPI_Send(&number_of_tosses,1,MPI_LONG_LONG_INT,i,0,MPI_COMM_WORLD);
	}
	else
	{
		MPI_Recv(&number_of_tosses, 1,MPI_LONG_LONG_INT,0, 0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);

	}

	startwtime = MPI_Wtime();

	for(toss = 0 ; toss<number_of_tosses;toss++)//擲飛鏢
	{
		x= ((double) rand() / (double) RAND_MAX) * 2.0 - 1.0;
		y= ((double) rand() / (double) RAND_MAX) * 2.0 - 1.0;
		if((x*x+y*y)<=1)++number_in_circle;
//		printf("x:%f y:%f,from id %d\n",x,y,my_rank);
	}	

	
//	MPI_Reduce(&number_in_circle, &, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
	for (half = comm_sz/2; half>=1; half = half/2)//用alternative tree structured 傳資料
        	if (my_rank < 2*half) 
		{
			if (my_rank >= half )		
				MPI_Send(&number_in_circle,1,MPI_LONG_LONG_INT,my_rank - half,0,MPI_COMM_WORLD);
								                        
			else 
			{		
				MPI_Recv(&temp, 1,MPI_LONG_LONG_INT, my_rank + half, 0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);	//			printf("to id %d \n",my_rank);
				number_in_circle=  number_in_circle+temp;	
			}				
		}

	
	if(my_rank==0)//最後process 0 印出結果及執行時間
	{
	//	printf("%ld\n",number_in_circle);
		pi_estimate = 4*number_in_circle/((double)number_of_tosses*comm_sz);
		endwtime = MPI_Wtime();
		printf("\npi_estimate:%.16f\n",pi_estimate);
		printf("wall clock time = %f sec\n", endwtime-startwtime);
		fflush(stdout);
	}
	
	MPI_Finalize();
	return 0;	
}
