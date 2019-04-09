#include <mpi.h>
#include <stdlib.h>
#include <stdio.h>
#include <omp.h>
#include <time.h>
#include <unistd.h>
int ** tempTour;
int **P;
double **N;
long number;
int Tour[1000];
int alpha,beta;//算機率需要的
int m;
double **phe;//費洛蒙
int count;//總共做count次
double fade=0.1;//衰減常數
double Q = 1;
void choosecity(int **,int , int[] );
void updateP(int ** ,int,double **);
int globalcost=-1;//一開始代表無限大

int main(int argc,char *argv[]){


	srand(time(NULL));
	int comm_size,myid;
	MPI_Init(&argc,&argv);
	MPI_Comm_size (MPI_COMM_WORLD, &comm_size);
	MPI_Comm_rank (MPI_COMM_WORLD, &myid);

	double startwtime = 0.0, endwtime=0;
	char* filename = malloc(100);
	sprintf(filename,"%s",argv[2]);	
//	printf("%s\n",filename);
	char tmp[2];
	int i,j=0;

	for(i=0;filename[i];i++)
	{
		if(filename[i]>='0' && filename[i]<='9')    
		{
			tmp[j]=filename[i];				
			j++;
			if(j==2)break;	
	      	}
	}
	
	number =  strtol(tmp,NULL,10);	

	//初始化P陣列並讀入data
	P=(int **)malloc(sizeof(int*)*number);
	for(i=0;i<number;i++)P[i] = (int*)malloc(sizeof(int)*number);
	N=(double **)malloc(sizeof(double*)*number);
	for(i=0;i<number;i++)N[i] = (double*)malloc(sizeof(double)*number);

	char *temp=malloc(10);
	FILE* file = fopen(filename, "r");
		
	for(i=0;i<number;i++)
		for(j=0;j<number;j++)
			{
				fscanf(file,"%d",&P[i][j]);
				N[i][j]=(double)1/P[i][j];
				if(P[i][j]==0)N[i][j]=1;//因為有些取倒數變成inf，取1就好，反正其他人的都小於1
			}
/*
if(myid==0)	
	for(i=0;i<number;i++){
		for(j=0;j<number;j++)
			printf("%lf ",N[i][j]);
	printf("\n");
	}*/
//------------------------------------------輸入參數(記得換成command line模式)
/*
	printf("alpha:");
	scanf("%d",&alpha);
	printf("beta:");
        scanf("%d",&beta);
	printf("m:");
	scanf("%d",&m);
	printf("count:");
	scanf("%d",&count);*/
	alpha = 2;
	beta = 7;
	m=500;
	count =50;
//----------------------------------------- 
	MPI_Barrier(MPI_COMM_WORLD);
	startwtime = MPI_Wtime();
//------------------------------------------
	//初始化費洛蒙
	phe=(double **)malloc(sizeof(double*)*number);
	for(i=0;i<number;i++)phe[i] = (double*)malloc(sizeof(double)*number);
	for(i=0;i<number;i++)
		for(j=0;j<number;j++)
			phe[i][j]=1.0;

	int thread_count = strtol(argv[1], NULL, 10);
	int k,t;
	int threadTour[number];//thread內自己的最短tour
	double **temp2;
	
//	int ** tempTour;//thread內目前的tour
//	tempTour=(int **)malloc(sizeof(int*)*m);
//	for(i=0;i<m;i++)tempTour[i] = (int*)malloc(sizeof(int)*number);
//	for(i=0;i<m;i++)
//	        for(j=0;j<number;j++)
//			tempTour[i][j]	=-1;
	for(i=0;i<number;i++)threadTour[i]=-1;//先初始化為-1表示都還沒走過
//	printf("%d ",tempTour[0][0]);
	
	int cost[m];//local的目前花多少

	#pragma omp parallel for num_threads(thread_count)default(none) private(t,i,k,j,tempTour,temp2,cost) shared(myid,count,m,N,phe,fade,Q,globalcost,Tour,number,P)
	for(t=0;t<count;t++){//總共做count次
		int a =myid;
		for(i=0;i<m;i++)
			cost[i]=0;//每次都把cost歸零

		temp2=(double **)malloc(sizeof(double*)*number);
	        for(i=0;i<number;i++)temp2[i] = (double*)malloc(sizeof(double)*number);

		tempTour=(int **)malloc(sizeof(int*)*m);
		for(i=0;i<m;i++)tempTour[i] = (int*)malloc(sizeof(int)*(number+1));
		for(i=0;i<m;i++)
	                for(j=0;j<number;j++)
	   			tempTour[i][j]  =-1;
	
		for(i=0;i<m;i++)
		{
		tempTour[i][0]=(rand()+myid-i+omp_get_thread_num())%number;//把螞蟻初始放在某個city上
		
		}
		for(i=1;i<number;i++){//第一步已經走完 剩下number-1步
			for(k=0;k<m;k++){		
				#pragma omp critical(c1)//用到費洛蒙
				{
				choosecity(tempTour,k,cost);//第k隻螞蟻的路
				}
				if(i==number-1)//最後一步要回到家
				{	
				//	printf("%d ",tempTour[k][0]);
					tempTour[k][number]=tempTour[k][0];
					cost[k]+=P[tempTour[k][i]][tempTour[k][0]];
				}
			}
		}
//		for(i=0;i<m;i++)
//			printf("%d ",cost[i]);

		//----m隻螞蟻走完所有number路 看看是不是最短路徑 如果是要更新		
		for(i=0;i<m;i++)
			if(cost[i]<globalcost||globalcost==-1)//如果比global小或是global=無限大
				#pragma omp critical(c2)
			{
				if(cost[i]<globalcost||globalcost==-1)//doublecheck
				{
					globalcost = cost[i];
					for(k=0;k<number+1;k++)
						Tour[k]=tempTour[i][k];
				}
					
			}
		
			
		//----update phreomone
		updateP(tempTour,m,temp2);
		#pragma omp critical(c1)//費洛蒙為共有的
		{
//		updateP(tempTour,m,temp2);

		for(i=0;i<number;i++)
		        for(j=0;j<number;j++)
			{
	 
				phe[i][j]=temp2[i][j];
			}
		}	
							
		free(tempTour);
		free(temp2);	
	}
//	printf("\n");
/*
	for(i=0;i<number+1;i++)
		printf("%d ",Tour[i]);
	printf("\n");
	int costest = 0;
	for(i=1;i<number+1;i++)
	{
		printf("%d+",P[Tour[i-1]][Tour[i]]);
//		costest+=P[Tour[i-1]][Tour[i]];
	}

	
	printf("\n");*/
//	printf("cost: %d\n",globalcost);
//	printf("costest: %d\n",costest);
//-------------------比所有資料大小並傳送資料-----------------------
	struct {
		int cost;
		int rank;
	}loc_data,global_data;
	//第一步:比比看誰的cost小並紀錄那個人的rank
	
	loc_data.cost = globalcost;
	loc_data.rank = myid;
	MPI_Allreduce(&loc_data,&global_data,1,MPI_2INT,MPI_MINLOC,MPI_COMM_WORLD);
	//第二步:知道了誰是最小之後，也不用像課本再寄給process0了 那個人直接就印出他的tour就好

	if(myid==global_data.rank){
		printf("\nmin Tour this time:\n");
		for(i=0;i<number+1;i++)
			printf("%d ",Tour[i]);
		printf("\ncost: %d\n",global_data.cost);
	}


//----------------------------------------

	MPI_Barrier(MPI_COMM_WORLD);
	endwtime = MPI_Wtime();

	if(myid==0)			
		printf("The execution time = %lf\n",endwtime-startwtime);

	MPI_Finalize();			
	return 0;

}

void choosecity(int** localTour,int m,int cost[]){

	double pro[number];//所有人的機率，先初始化為0
	int z;
	for(z=0;z<number;z++)pro[z]=0.0;
	
	int myrank = omp_get_thread_num();
		
	
//		printf("%d",myrank);
//		if(myrank==0)
//		for(z=0;z<number;z++)printf("%d ",localTour[z]);
//	printf("\n");
	//找到目前走到哪個city
	int index=0,i=0;	
	while(localTour[m][i]!=-1){
		index=localTour[m][i];
		pro[localTour[m][i]]=-1;//走過的人先設為-1
		i++;
	}
/*	
	if(myrank==0)
		                for(z=0;z<number;z++)printf("%lf ",pro[z]);
	        printf("\n");
 */

	double total=0;
	int j,x,y;
	double N_total=1,phe_total=1;
	for(j=0;j<number;j++)
	{
		N_total=1,phe_total=1;
		if(pro[j]!=-1)//如果前面沒走過才要加到total
		{
			for(x=0;x<alpha;x++)
				N_total*=N[index][j];
		 	for(y=0;y<beta;y++)
				phe_total*=phe[index][j];		
			total +=N_total*phe_total;
		
		}
	
	}

	double N_temp = 1.0,phe_temp=1;
	double temp;
	for(j=0;j<number;j++)
	{
		N_temp = 1.0,phe_temp=1;
		if(pro[j]!=-1)
		{	
			
			for(x=0;x<alpha;x++)
				N_temp*=N[index][j];
			for(y=0;y<beta;y++)
		 		phe_temp*=phe[index][j];

			pro[j]=(double)N_temp*phe_temp/total;//算到每個state的機率;
		}
		else
		{
	
		pro[j]=0;//以前走過的機率=0
		}	
	}
/*
		if(myrank==0){
	for(z=0;z<number;z++)
		printf("%lf ",pro[z]);
	printf("\n");	
		}
*/
	//從上面算好的機率選出一個state走
	double p=((double)rand()/ (double)(RAND_MAX));

//	printf("%lf ",p);
	j=-1;
	while(p>0){
		j++;
		p-=pro[j];
	}
	//最後的j就是要走哪個city
	
//	if(myrank==0&&m==0)
//		printf("%d ",i);	
	
	localTour[m][i]=j;

	cost[m]+=P[localTour[m][i-1]][localTour[m][i]];

//	if(myrank==0)
//		printf("%d ",cost[m]);
//	printf("%d %d;",localTour[m][i-1],localTour[m][i]);

//印出走過的路
//	if(myrank==0&&m==0)
//	printf("%d ",localTour[m][i-1]);
//	if(i==number-1&&myrank==0&&m==0)
//	printf("%d \n",localTour[m][i]);

}

void updateP(int ** Tour,int m,double **temp){
	int myrank = omp_get_thread_num();

//	if(myrank==1)printf("%d ",number);
	int i , j;
	double delta[number][number];
	for(i=0;i<number;i++)
	      	for(j=0;j<number;j++)
			delta[i][j]=0;
	
	for(i=0;i<m;i++)
        	for(j=1;j<number;j++)
			{	
			delta[Tour[i][j-1]][Tour[i][j]]+=N[Tour[i][j-1]][Tour[i][j]]*Q;
			}

	int i2=0,j2=0;

	double temp2[number][number];
	for(i2=0;i2<number;i2++)
		for(j2=0;j2<number;j2++)
		{	
				
			temp2[i2][j2]=(phe[i2][j2]*(1-fade))+delta[i2][j2];
	//		 phe[i2][j2]=temp2[i2][j2];			
		}
//	for(i2=0;i2<number;i2++)
  //              for(j2=0;j2<number;j2++)
//			phe[i2][j2]=temp2[i2][j2];
	

}
