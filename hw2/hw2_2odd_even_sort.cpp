#include <mpi.h>
#include <iostream>
#include <stdlib.h>
#include <time.h>
using namespace std;

inline void quicksort(int*,int,int);//sort locallist 用，用inline增加效能(後來因為效能不如C library的不用此函數)
inline void swap(int *a, int *b);//quicksort裡面用到

int cmpfunc (const void * a, const void * b) {
	   return ( *(int*)a - *(int*)b );
}

int main(int argc,char *argv[])
{		
	int n=0,localnumber=0,rem=0;
	int comm_sz,myid;
	double startwtime = 0.0, endwtime=0;
	MPI_Init(&argc,&argv);
	MPI_Comm_size(MPI_COMM_WORLD,&comm_sz);
	MPI_Comm_rank(MPI_COMM_WORLD,&myid); 
	srand( time(NULL)+myid*85);//做一些操作，讓process間產生出來的亂數不要太多重複
	int *globallist=NULL;//傳進cpu0的locallist放置地點
	int *comparelist=NULL;//放odd even sort傳進來的數字
	//記錄開始時間
	
	//讀入n並廣播到各個cpu
	if(myid==0)
	{
		cout<<"please enter n :";
		cin>>n;
	}
	

	MPI_Bcast (&n,1,MPI_INT,0,MPI_COMM_WORLD);
	
	//算出每個process需要處理多少數字，cpu0要有所有人處理多少(因為下面會傳送locallist)
	int keepnumber[comm_sz];//每個人處理幾個數字
	
	int i;
	for(i=0;i<comm_sz;i++)
	{	 
		keepnumber[i]=n/comm_sz;
		rem=n%comm_sz;
		if(rem!=0&&i<rem)
			++keepnumber[i];
	}

	
	localnumber=n/comm_sz;
	rem=n%comm_sz;
	if(rem!=0&&myid<rem)
		++localnumber;
//	cout<<"process"<<myid<<": "<<localnumber<<endl;

	//用來儲存隨機數字的陣列
	int *locallist=(int*)malloc(sizeof(int)*localnumber);
	//隨機產生localnumber個數字
	for(int i=0;i<localnumber;i++)	
		locallist[i]=(rand()+myid*8)%32767;//讓數字控制在32767內，並做一些操作以免亂數有許多重複

	startwtime = MPI_Wtime();



	//用quicksort排序locallist
	qsort(locallist,localnumber, sizeof(int), cmpfunc);	
	//把locallist傳給cpu0集中印出
	//最多傳進來的長度是process0 的localnumber(有餘數第一個一定從0開始放)
	globallist = (int *)malloc(sizeof(int)*localnumber);
	if(myid==0)cout<<"---------------------------\n";
	if(myid!=0)
		MPI_Send (locallist,localnumber,MPI_INT,0,0,MPI_COMM_WORLD);
	else
		for(int i=1;i<comm_sz;i++)
		{
			MPI_Recv (globallist,keepnumber[i],MPI_INT,i,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
			if(i==1)//在印process1的數字前先印process0的
			{
				cout<<"process 0 locallist:";
				for(int j=0;j<localnumber;j++)
					cout<<" "<<locallist[j];
				cout<<endl;
			}
			cout<<"process "<<i<<" locallist:";
			for(int j=0;j<keepnumber[i];j++)
				cout<<" "<<globallist[j];
			cout<<endl;
		}
	if(comm_sz==1)//單process直接印出process0的
	{
		cout<<"process 0 locallist:";	
		for(int j=0;j<localnumber;j++)					
			cout<<" "<<locallist[j];					      
		cout<<endl;
	}
	if(myid==0)cout<<"---------------------------------\n";
	//process間的odd even sort
	for(int phase=0;phase<comm_sz;phase++)
	{
		int partner=-1;//預設partner=-1
		if(phase%2==0)
		{		
				if(myid+1>=comm_sz&&myid%2==0)//如果沒有partner(最後一個，落單的)
					;
				else if(myid%2==0)
					partner=myid+1;
				else 	
					partner=myid-1;		
		}
		else
		{		
				if((myid==0)||(myid+1>=comm_sz&&myid%2!=0))//奇數phase，0必定落單，或是最後一個人落單
					;	
				else if(myid%2!=0)
					partner=myid+1;
				else				
    					partner=myid-1;
		}	
		//printf("phase %d ，process %d partner:%d\n",phase,myid,partner);
		MPI_Request requ1;
		if(partner!=-1)
		{
			comparelist = (int*)malloc(sizeof(int)*2*keepnumber[0]);//最多不會超過2倍的process0數字

			MPI_Isend(locallist,keepnumber[myid],MPI_INT,partner,0,MPI_COMM_WORLD,&requ1);
			MPI_Irecv(comparelist,keepnumber[partner],MPI_INT,partner,0, MPI_COMM_WORLD,&requ1);
			MPI_Wait (&requ1,MPI_STATUS_IGNORE);

			//printf("done\n");
			for(int i=0;i<keepnumber[myid];i++)
				comparelist[keepnumber[partner]+i]=locallist[i];
			
			if(myid<partner)
			{	

				//quicksort(comparelist,0,keepnumber[myid]+keepnumber[partner]-1);
				qsort(comparelist,keepnumber[myid]+keepnumber[partner],sizeof(int), cmpfunc);
				for(int i=0;i<keepnumber[myid];i++)	
					locallist[i]=comparelist[i];
			}
			else
			{	
				//quicksort(comparelist,0,keepnumber[myid]+keepnumber[partner]-1);
				qsort(comparelist,keepnumber[myid]+keepnumber[partner],sizeof(int), cmpfunc);
				for(int i=0;i<keepnumber[myid];i++)
					locallist[i]=comparelist[keepnumber[partner]+i];

			}
		}	
	
	}

	if(myid!=0)
		MPI_Send (locallist,localnumber,MPI_INT,0,0,MPI_COMM_WORLD);
	else
		for(int i=1;i<comm_sz;i++)
		{
			MPI_Recv (globallist,keepnumber[i],MPI_INT,i,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
			if(i==1)//在印process1的數字前先印process0的
			{	
				cout<<"Global list:\n";
				for(int j=0;j<localnumber;j++)
					cout<<" "<<locallist[j];
				cout<<endl;

			}
			
			for(int j=0;j<keepnumber[i];j++)
				cout<<" "<<globallist[j];
			cout<<endl;
		}
	
	if(comm_sz==1)//直接印出process 0 sort好的list
	{
		cout<<"Global list:\n";
		for(int j=0;j<localnumber;j++)				
			cout<<" "<<locallist[j];					
		cout<<endl;
	}	

	endwtime = MPI_Wtime();
	if(myid==0)cout<<"--------------------\nwall clock time ="<<endwtime-startwtime<<" sec\n";
	MPI_Finalize();
	return 0;

}

inline void quicksort(int* data,int left,int right)
{
	int pivot, i, j;

    	if (left >= right) { return; }//若左邊index>=右邊index 就return
	pivot = data[left];
       	i = left + 1;
	j = right;

	while (1)		
	{	
		while (i <= right)	
		{				
			if (data[i] > pivot)		
			{
				break;
			}
			++i;		
		}
		
		while (j > left)							       
	       	{				
			if (data[j] < pivot)		
			{
				break;

			}			
			j = j - 1;			
		}		
		if (i > j) { break; }		
		swap(&data[i], &data[j]);				

//		printf("%d\n",i);
	}

	swap(&data[left], &data[j]);
	
	quicksort(data, left, j - 1);
	quicksort(data, j + 1, right);


}
inline void swap(int *a, int *b)
{
	int temp = *a;	
    	*a = *b;
	*b = temp;
}

