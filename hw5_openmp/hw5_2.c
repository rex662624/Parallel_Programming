#include <stdio.h>
#include <omp.h>
#include <stdlib.h>
#include <time.h>

char queue[2048][256];//max = 200 words per line
int tail = 0 ;//the empty node index array
int senddone = 0;

char keyword[256][32];//存keyword 有哪些
int totalkeyword=0;//共有多少個keyword
int totalcount[256];//答案

void countOccurrences(char * , char * ,int);
void sendrecv();
int main(int argc,char*argv []){
	
	clock_t start = clock();

	//get threadnumber
	int thread_count = strtol(argv[1],NULL,10);
	//main 先去把keyword找出來
	//if(keyword[0][0]==NULL)printf("HAHA\n");
	const char *keyfilename = "keyword.txt";
	FILE* file = fopen(keyfilename, "r");
	
	if(file)
	{
	while(fscanf(file,"%s",keyword[totalkeyword++])==1);	
	}
	else
	{
		printf("open keyword file Error");
	}
/*	int i ;
	for(i=0;i<totalkeyword;i++){
	printf("%s\n",keyword[i]);	
	}
	printf("\n");*/

	//找出所有keyword之後開始平行化一行一行讀要分析的file
	#pragma omp parallel num_threads(thread_count)
	sendrecv();
	
//	printf("\ncount: ");
	int i ;
	for(i=0;i<totalkeyword-1;i++)printf("keyword: %10s   count: %7d\n",keyword[i],totalcount[i]);
	
	clock_t end = clock();
	float seconds = (float)(end - start) / CLOCKS_PER_SEC;
	printf("The execution time = %lf\n", seconds);
	return 0;

}


void sendrecv(){

	int myrank = omp_get_thread_num();
	
	if(myrank==0){//master	
	const char *filename = "file2.txt";

	FILE* file = fopen(filename, "r"); /* should check the result */
	if (file == NULL)
	{
		fputs("Error: file open failure\n", stderr);
		exit(EXIT_FAILURE);
	}

	char line[256]; 
	while (fgets(line, sizeof(line), file))
	{
		
		
		#pragma omp critical//動用到queue的要用critical section包住
		{
			sprintf(queue[tail++],line);
//			printf("%s",queue[tail-1]);
		}
	}
	senddone=1;
	
	fclose(file);
//	int i ;
//	for(i=0;i<12;i++)
//	printf("%s",queue[i]);
	}else{//slave
		
	char line[256];	
		
		while(tail!=0||senddone!=1)//還沒傳完或是還沒收完,就要繼續試著讀
		{
			#pragma omp critical
			{	
			if(tail!=0)//表示可以讀了	
				{	

					sprintf(line,queue[tail-1]);
					tail--;
//					printf("%d get %s\n",myrank,line);
				}	
			}
			//讀完一行開始找 找完再讀下一行(回到while)
			
			if(line[0]!=NULL){//有讀到到東西才下去找
			int i = 0;
			for(i = 0; i <totalkeyword;i++){
				countOccurrences(line,keyword[i],i);
			
			}
			
			}
			line[0] = NULL;

		}
	
	}

}


void countOccurrences(char * str, char * toSearch,int index)
{

	int i, j, found, count;
	int stringLen, searchLen;
	stringLen = strlen(str);      // length of string
	searchLen = strlen(toSearch); // length of word to be searched
//	printf("%s ",toSearch);	
	count = 0;
	for(i=0; i <= stringLen-searchLen; i++)
	{					
		/* Match word with string */	
		found = 1;
		if(str[i]!=toSearch[0]&&str[i]!=toupper(toSearch[0])&&str[i]!=tolower(toSearch[0]))//先比第一個字母 因為Apple 和apple皆可以
		{
			found = 0;
		}
		for(j=1; j<searchLen; j++)//一個一個字元去比對 比對到strlen(keyword)			
		{					
			if(str[i + j] != toSearch[j])
			{
				found = 0;
				break;
			}		
		}
		if(found == 1)//就算找到了 也要確認沒有keyword = apple 但pineapple也算進去的狀況
		{		
	    		if(str[i + j] == ' ' || str[i + j] == '\t' || str[i + j] == '\n' || str[i + j] == '\0'||str[i + j] == ','||str[i + j] == '.')//後面是接這些東西的才可以+1
			{	
				#pragma omp atomic 
				totalcount[index]++;
				
			}
		}
	}
	return;
}
