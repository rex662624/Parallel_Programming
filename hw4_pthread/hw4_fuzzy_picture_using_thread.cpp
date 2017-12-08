//#include <mpi.h>
#include <iostream>
#include <string>
#include <fstream>
#include <stdio.h>
#include <cstring>
#include <cstdlib>
#include "bmp.h"
#include <time.h>
#include <pthread.h>


using namespace std;

//�w�q���ƹB�⪺����
#define NSmooth 1000

/*********************************************************/
/*�ܼƫŧi�G                                             */
/*  bmpHeader    �G BMP�ɪ����Y                          */
/*  bmpInfo      �G BMP�ɪ���T                          */
/*  **BMPSaveData�G �x�s�n�Q�g�J���������               */
/*  **BMPData    �G �Ȯ��x�s�n�Q�g�J���������           */
/*********************************************************/
BMPHEADER bmpHeader;                        
BMPINFO bmpInfo;
RGBTRIPLE **BMPSaveData = NULL;                                               
RGBTRIPLE **BMPData = NULL;                                                   

/*********************************************************/
/*��ƫŧi�G                                             */
/*  readBMP    �G Ū�����ɡA�ç⹳������x�s�bBMPSaveData*/
/*  saveBMP    �G �g�J���ɡA�ç⹳�����BMPSaveData�g�J  */
/*  swap       �G �洫�G�ӫ���                           */
/*  **alloc_memory�G �ʺA���t�@��Y * X�x�}               */
/*********************************************************/
int readBMP( char *fileName);        //read file
int saveBMP( char *fileName);        //save file
void swap(RGBTRIPLE *a, RGBTRIPLE *b);
RGBTRIPLE **alloc_memory( int Y, int X );        //allocate memory

int thread_count;
void* funct(void*rank);
int rem;
int sum;
int *local_size = (int*)malloc(sizeof(int)*200);
//****************barrier1
int count1 = 0;
pthread_mutex_t mutex1;
pthread_cond_t cond_var1;
//****************barrier2
int count2 = 0;
pthread_mutex_t mutex2;
pthread_cond_t cond_var2;

int main(int argc,char *argv[])
{
/*********************************************************/
/*�ܼƫŧi�G                                             */
/*  *infileName  �G Ū���ɦW                             */
/*  *outfileName �G �g�J�ɦW                             */
/*  startwtime   �G �O���}�l�ɶ�                         */
/*  endwtime     �G �O�������ɶ�                         */
/*********************************************************/
	char *infileName = "input.bmp";
     	char *outfileName = "outputhw4.bmp";
//	double startwtime = 0.0, endwtime=0;

//	MPI_Init(&argc,&argv);
	
	//�O���}�l�ɶ�
//	startwtime = MPI_Wtime();

	clock_t start = clock();
	//Ū���ɮ�
        if ( readBMP( infileName) )
                cout << "Read file successfully!!" << endl;
        else 
                cout << "Read file fails!!" << endl;
	//count data deal by thread
	
	thread_count = strtol(argv[1],NULL,10);//how many thread
	
	int rem = bmpInfo.biHeight % thread_count;
	//localsize: how many col
	for(int i =0 ; i<thread_count;i++)
	{		
		local_size[i] = bmpInfo.biHeight/thread_count;		
		if (rem > 0)
		{						
			local_size[i]+=1;								
			rem--;

		}	
	
	}
	BMPData = alloc_memory( bmpInfo.biHeight, bmpInfo.biWidth);
	//thread declation
	long thread;
	pthread_t * thread_handles;
	thread_handles =(pthread_t *) malloc(thread_count*sizeof(pthread_t));
	for(thread = 0;thread<thread_count;thread++)
		pthread_create(&thread_handles[thread],NULL,funct,(void*)thread);//thread is argument of funct
		
        //�i��h�������ƹB��

	//�o�쵲���ɶ��A�æL�X����ɶ�
//        endwtime = MPI_Wtime();
	 for(thread = 0;thread<thread_count;thread++)
	 	pthread_join(thread_handles[thread],NULL);

	free(thread_handles);


	
       	if ( saveBMP( outfileName ) )
                cout << "Save file successfully!!" << endl;
        else
                cout << "Save file fails!!" << endl;
	                                   
	
	clock_t end = clock();
	float seconds = (float)(end - start) / CLOCKS_PER_SEC;

    	cout << "The execution time = "<< seconds <<endl ;

 	free(BMPData);
 	free(BMPSaveData);
// 	MPI_Finalize();

        return 0;
}

void* funct(void*rank){

	int upper=0;
	int lower=0;
	long myrank = (long) rank;
	for(int i =0;i<myrank;i++){
	
		upper+=local_size[i];
	
	}
	lower = local_size[myrank]+upper;
	//if(myrank!=0)upper++;
	//printf("thread %d: upper:%d,lower:%d\n",rank,upper,lower);
		
	//each perform
	
	for(int count = 0; count < NSmooth ; count ++){
		//barrier
		pthread_mutex_lock(&mutex1);
		count1++;
		if(count1==thread_count){
			count1=0;
			pthread_cond_broadcast(&cond_var1);
		}else{
			while(pthread_cond_wait(&cond_var1,&mutex1)!=0);	
		}
		pthread_mutex_unlock(&mutex1);
		//swap once
		if(myrank==0){swap(BMPSaveData,BMPData);
//		printf("swap\n");
		}
		//barrier
		pthread_mutex_lock(&mutex2);
		count2++;
		if(count2==thread_count){
			count2=0;
			pthread_cond_broadcast(&cond_var2);
		}else{
			while(pthread_cond_wait(&cond_var2,&mutex2)!=0);
		}
		pthread_mutex_unlock(&mutex2);
		//*********************************************************
//		printf("hei:%d\n",bmpInfo.biHeight);
//		printf("%c %c %c",BMPSaveData[0][0],BMPSaveData[0][1]);
		for(int i =upper; i<lower; i++)
			for(int j =0; j<bmpInfo.biWidth ; j++){
				/*********************************************************/
				/*�]�w�W�U���k��������m                                 */
				/*********************************************************/
				int Top = i>0 ? i-1 : bmpInfo.biHeight-1;
				int Down = i<bmpInfo.biHeight-1 ? i+1 : 0;
				int Left = j>0 ? j-1 : bmpInfo.biWidth-1;
				int Right = j<bmpInfo.biWidth-1 ? j+1 : 0;
				/*********************************************************/
				/*�P�W�U���k�����������A�å|�ˤ��J                       */
				/*********************************************************/
				BMPSaveData[i][j].rgbBlue =  (double) (BMPData[i][j].rgbBlue+BMPData[Top][j].rgbBlue+BMPData[Down][j].rgbBlue+BMPData[i][Left].rgbBlue+BMPData[i][Right].rgbBlue)/5+0.5;
				BMPSaveData[i][j].rgbGreen =  (double) (BMPData[i][j].rgbGreen+BMPData[Top][j].rgbGreen+BMPData[Down][j].rgbGreen+BMPData[i][Left].rgbGreen+BMPData[i][Right].rgbGreen)/5+0.5;
				BMPSaveData[i][j].rgbRed =  (double) (BMPData[i][j].rgbRed+BMPData[Top][j].rgbRed+BMPData[Down][j].rgbRed+BMPData[i][Left].rgbRed+BMPData[i][Right].rgbRed)/5+0.5;
			}
			
	}
   
 	//�g�J�ɮ�
 		                                                                                  	
	return NULL;

}

int readBMP(char *fileName)
{
	//�إ߿�J�ɮת���	
        ifstream bmpFile( fileName, ios::in | ios::binary );
 
        //�ɮ׵L�k�}��
        if ( !bmpFile ){
                cout << "It can't open file!!" << endl;
                return 0;
        }
 
        //Ū��BMP���ɪ����Y���
    	bmpFile.read( ( char* ) &bmpHeader, sizeof( BMPHEADER ) );
 
        //�P�M�O�_��BMP����
        if( bmpHeader.bfType != 0x4d42 ){
                cout << "This file is not .BMP!!" << endl ;
                return 0;
        }
 
        //Ū��BMP����T
        bmpFile.read( ( char* ) &bmpInfo, sizeof( BMPINFO ) );
        
        //�P�_�줸�`�׬O�_��24 bits
        if ( bmpInfo.biBitCount != 24 ){
                cout << "The file is not 24 bits!!" << endl;
                return 0;
        }

        //�ץ��Ϥ����e�׬�4������
        while( bmpInfo.biWidth % 4 != 0 )
        	bmpInfo.biWidth++;

        //�ʺA���t�O����
        BMPSaveData = alloc_memory( bmpInfo.biHeight, bmpInfo.biWidth);
        
        //Ū���������
    	//for(int i = 0; i < bmpInfo.biHeight; i++)
        //	bmpFile.read( (char* )BMPSaveData[i], bmpInfo.biWidth*sizeof(RGBTRIPLE));
	bmpFile.read( (char* )BMPSaveData[0], bmpInfo.biWidth*sizeof(RGBTRIPLE)*bmpInfo.biHeight);
	
        //�����ɮ�
        bmpFile.close();
 
        return 1;
 
}
/*********************************************************/
/* �x�s����                                              */
/*********************************************************/
int saveBMP( char *fileName)
{
 	//�P�M�O�_��BMP����
        if( bmpHeader.bfType != 0x4d42 ){
                cout << "This file is not .BMP!!" << endl ;
                return 0;
        }
        
 	//�إ߿�X�ɮת���
        ofstream newFile( fileName,  ios:: out | ios::binary );
 
        //�ɮ׵L�k�إ�
        if ( !newFile ){
                cout << "The File can't create!!" << endl;
                return 0;
        }
 	
        //�g�JBMP���ɪ����Y���
        newFile.write( ( char* )&bmpHeader, sizeof( BMPHEADER ) );

	//�g�JBMP����T
        newFile.write( ( char* )&bmpInfo, sizeof( BMPINFO ) );

        //�g�J�������
        //for( int i = 0; i < bmpInfo.biHeight; i++ )
        //        newFile.write( ( char* )BMPSaveData[i], bmpInfo.biWidth*sizeof(RGBTRIPLE) );
        newFile.write( ( char* )BMPSaveData[0], bmpInfo.biWidth*sizeof(RGBTRIPLE)*bmpInfo.biHeight );

        //�g�J�ɮ�
        newFile.close();
 
        return 1;
 
}


/*********************************************************/
/* ���t�O����G�^�Ǭ�Y*X���x�}                           */
/*********************************************************/
RGBTRIPLE **alloc_memory(int Y, int X )
{        
	//�إߪ��׬�Y�����а}�C
        RGBTRIPLE **temp = new RGBTRIPLE *[ Y ];
	RGBTRIPLE *temp2 = new RGBTRIPLE [ Y * X ];
        memset( temp, 0, sizeof( RGBTRIPLE ) * Y);
        memset( temp2, 0, sizeof( RGBTRIPLE ) * Y * X );

	//��C�ӫ��а}�C�̪����Ыŧi�@�Ӫ��׬�X���}�C 
        for( int i = 0; i < Y; i++){
                temp[ i ] = &temp2[i*X];
        }
 
        return temp;
 
}
/*********************************************************/
/* �洫�G�ӫ���                                          */
/*********************************************************/
void swap(RGBTRIPLE *a, RGBTRIPLE *b)
{
	RGBTRIPLE *temp;
	temp = a;
	a = b;
	b = temp;
}

