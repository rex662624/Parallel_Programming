#include <stdio.h>     // printf()
#include <limits.h>    // UINT_MAX
#include<mpi.h>
int checkCircuit (int, long);

int main (int argc, char *argv[]) {

	long i;               /* loop variable (64 bits) */
	int id = 0;           /* process id */
	int count = 0;        /* number of solutions */
	int  temp = 0;
	int comm_sz;
	int half;	
	
	MPI_Init(NULL,NULL);

	MPI_Comm_size(MPI_COMM_WORLD,&comm_sz);
	MPI_Comm_rank(MPI_COMM_WORLD,&id);
					
	double startTime = 0.0, totalTime = 0.0;

	startTime = MPI_Wtime();


	if(id==0)//process 0 先把data 0處理掉 因為後面要從1開始
		checkCircuit (0, 0);				

	long interval=UINT_MAX/comm_sz;//用所有數量除以processor數量，算出一個processor需要處理多少資料
	
	for (i = 1+interval*id; i <= interval*(id+1); i++)//類似1~100 101~200的方式分配
	{
		count += checkCircuit (id, i);
	}

	if(id==(comm_sz-1))//為了防止沒有整除，最後一個process把剩下沒處理完的data處理掉
	{
		for (i = interval*(id+1)+1; i <= UINT_MAX; i++)
			count += checkCircuit (id, i);
	}
						
	for (half = comm_sz/2; half>=1; half = half/2)//用alternatice tree structure方式傳資料			
		if (id < 2*half) 
		{
			if (id >= half )							
			{	
				MPI_Send(&count,1,MPI_INT,id- half,0,MPI_COMM_WORLD);
			}		
			else 
			{
				MPI_Recv(&temp, 1, MPI_INT, id + half, 0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
				count = count + temp;
		
			}

		}

	totalTime = MPI_Wtime() - startTime;
	printf("Process %d finished in time %f secs.\n", id, totalTime);
	printf ("Process %d finished.\n", id);
	fflush (stdout);

	if(id==0)//用process0印結果就好
		printf("\nA total of %d solutions were found.\n\n", count);
	MPI_Finalize();
	return 0;

}

/* EXTRACT_BIT is a macro that extracts the ith bit of number n.
 * parameters: n, a number;
 * i, the position of the bit we want to know.	
 * return: 1 if 'i'th bit of 'n' is 1; 0 otherwise
 * */

#define EXTRACT_BIT(n,i) ( (n & (1<<i) ) ? 1 : 0)

                                                                       
#define SIZE 32

int checkCircuit (int id, long bits) {
	int v[SIZE];        /* Each element is a bit of bits */
	int i;


	
	for (i = 0; i < SIZE; i++)

		v[i] = EXTRACT_BIT(bits,i);


	if ( ( (v[0] || v[1]) && (!v[1] || !v[3]) && (v[2] || v[3])
				&& (!v[3] || !v[4]) && (v[4] || !v[5])
				&& (v[5] || !v[6]) && (v[5] || v[6])
				&& (v[6] || !v[15]) && (v[7] || !v[8])
				&& (!v[7] || !v[13]) && (v[8] || v[9])
				&& (v[8] || !v[9]) && (!v[9] || !v[10])
				&& (v[9] || v[11]) && (v[10] || v[11])
				&& (v[12] || v[13]) && (v[13] || !v[14])
				&& (v[14] || v[15]) )
			||

			( (v[16] || v[17]) && (!v[17] || !v[19]) && (v[18] || v[19])
			  && (!v[19] || !v[20]) && (v[20] || !v[21])
			  && (v[21] || !v[22]) && (v[21] || v[22])
			  && (v[22] || !v[31]) && (v[23] || !v[24])
			  && (!v[23] || !v[29]) && (v[24] || v[25])
			  && (v[24] || !v[25]) && (!v[25] || !v[26])
			  && (v[25] || v[27]) && (v[26] || v[27])
			  && (v[28] || v[29]) && (v[29] || !v[30])
			  && (v[30] || v[31]) ) )

			{
	printf ("%d) %d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d \n", id,
				v[31],v[30],v[29],v[28],v[27],v[26],v[25],v[24],v[23],v[22],
				v[21],v[20],v[19],v[18],v[17],v[16],v[15],v[14],v[13],v[12],
				v[11],v[10],v[9],v[8],v[7],v[6],v[5],v[4],v[3],v[2],v[1],v[0]);
			   	fflush (stdout);			
	     			return 1;

				} else {					
				return 0;
				}
}
