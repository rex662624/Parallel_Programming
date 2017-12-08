#include <mpi.h>
#include <iostream>
#include <string>
#include <fstream>
#include <stdio.h>
#include <cstring>
#include <cstdlib>
#include "bmp.h"

using namespace std;

//定義平滑運算的次數
#define NSmooth 1000
//
///*********************************************************/
///*變數宣告：                                             */
///*  bmpHeader    ： BMP檔的標頭                          */
///*  bmpInfo      ： BMP檔的資訊                          */
///*  **BMPSaveData： 儲存要被寫入的像素資料               */
///*  **BMPData    ： 暫時儲存要被寫入的像素資料           */
///*********************************************************/
BMPHEADER bmpHeader;                        
BMPINFO bmpInfo;
RGBTRIPLE **BMPSaveData = NULL;                                               
RGBTRIPLE **BMPData = NULL;                                                   

///*********************************************************/
///*函數宣告：                                             */
///*  readBMP    ： 讀取圖檔，並把像素資料儲存在BMPSaveData*/
///*  saveBMP    ： 寫入圖檔，並把像素資料BMPSaveData寫入  */
///*  swap       ： 交換二個指標                           */
///*  **alloc_memory： 動態分配一個Y * X矩陣               */
///*********************************************************/
int readBMP( char *fileName);        //read file
int saveBMP( char *fileName);        //save file
void swap(RGBTRIPLE *a, RGBTRIPLE *b);
RGBTRIPLE **alloc_memory( int Y, int X );        //allocate memory
RGBTRIPLE *tempUpper=NULL;
RGBTRIPLE *tempLower=NULL;

int main(int argc,char *argv[])
{
/*********************************************************/
/*變數宣告：                                             */
/*  *infileName  ： 讀取檔名                             */
/*  *outfileName ： 寫入檔名                             */
/*  startwtime   ： 記錄開始時間                         */
/*  endwtime     ： 記錄結束時間                         */
/*********************************************************/

	char *infileName = "input.bmp";
	char *outfileName = "output1.bmp";
	double startwtime = 0.0, endwtime=0;
	
	int comm_size,myid;

	MPI_Init(&argc,&argv);
	MPI_Comm_size (MPI_COMM_WORLD, &comm_size);
       	MPI_Comm_rank (MPI_COMM_WORLD, &myid); 	 
	
	int *local_size = (int*)malloc(sizeof(int)*comm_size);//每個process處理的資料大小裝在local_size[process id]
	int *displs=(int*)malloc(sizeof(int)*comm_size);//每個process要處理的資料從哪裡開始
	
	//讀取檔案
	if ( !readBMP( infileName) )
		cout << "Read file fails!!" << endl;
	else if (myid == 0)	
		cout << "Read file successfully!!" << endl;
	//記錄開始時間
	MPI_Barrier(MPI_COMM_WORLD);
	startwtime = MPI_Wtime();
	
	//動態分配記憶體給暫存空間
	BMPData = alloc_memory( bmpInfo.biHeight, bmpInfo.biWidth);
	//分配上下各一塊暫存空間
        tempUpper = (RGBTRIPLE*) malloc(sizeof(char)*bmpInfo.biWidth*3);
        tempLower = (RGBTRIPLE*) malloc(sizeof(char)*bmpInfo.biWidth*3);
	
	//算出要分配給每個processor的資料長度和displacement 
	int rem = bmpInfo.biHeight%comm_size;//防止未整除，rem表示剩下來的列
	int sum = 0; //算displacement用
	for(int i =0 ; i<comm_size;i++)
	{
		local_size[i] = (bmpInfo.biHeight/comm_size)*bmpInfo.biWidth*3;//一個element 3bytes*3
   		if (rem > 0)//若有餘數
		{						
			local_size[i]+=bmpInfo.biWidth*3;//一次加一列，一個3byte*3
			rem--;
		}		
		
		displs[i] = sum;		
		sum += local_size[i];				
	}
	

	if(myid==0)
		MPI_Scatterv(BMPSaveData[0],local_size,displs,MPI_BYTE,BMPSaveData[0],local_size[myid],MPI_BYTE,0,MPI_COMM_WORLD);
	else 
		MPI_Scatterv(NULL,local_size,displs,MPI_BYTE,BMPSaveData[0],local_size[myid],MPI_BYTE,0,MPI_COMM_WORLD);

	 for(int count = 0; count < NSmooth ; count ++)
	{
		//把像素資料與暫存指標做交換
		swap(BMPSaveData,BMPData);				
		//printf("%d\n",count);
		
		//最低行數是哪一行(每個process的分割版BMPData皆從第0行開始算)
		int col =0+local_size[myid]/(3*bmpInfo.biWidth);
		if(comm_size > 1)
		{
				
				//傳送上下邊資料給需要的CPU					      
				MPI_Send(*(BMPData),bmpInfo.biWidth*3,MPI_BYTE,(myid-1+comm_size)%comm_size,22, MPI_COMM_WORLD);
				MPI_Send(*(BMPData+col-1), bmpInfo.biWidth*3, MPI_BYTE,(myid+1)%comm_size,11, MPI_COMM_WORLD);
							
				//接收需要的上下邊	
				MPI_Recv(tempLower, bmpInfo.biWidth * 3, MPI_BYTE,(myid+1)%comm_size,22, MPI_COMM_WORLD,MPI_STATUS_IGNORE);
			  	MPI_Recv(tempUpper, bmpInfo.biWidth * 3, MPI_BYTE,(myid-1+comm_size)%comm_size,11, MPI_COMM_WORLD,MPI_STATUS_IGNORE);				
			
  		}	
		
		//進行平滑運算				
		for(int i = 0; i<col; i++)		
			for(int j =0; j<bmpInfo.biWidth; j++)
			{
			/*********************************************************/
			/*設定上下左右像素的位置                                 */			
			/*********************************************************/			
			int Top = i>0 ? i-1 : bmpInfo.biHeight-1;
			int Down = i<bmpInfo.biHeight-1 ? i+1 : 0;
			int Left = j>0 ? j-1 : bmpInfo.biWidth-1;  
			int Right = j<bmpInfo.biWidth-1 ? j+1 : 0;
			//重新設定TDLR。若為最上邊或最下邊則使用來自其他cpu的資料
			RGBTRIPLE T,D,L,R;
      			if (i == 0 && comm_size > 1) T = *(tempUpper+j);
	                else T = BMPData[Top][j];
	                if (i == col-1 && comm_size > 1) D = *(tempLower+j);
	                else D = BMPData[Down][j];
	                L = BMPData[i][Left];
                        R = BMPData[i][Right];					
			/*********************************************************/
			/*與上下左右像素做平均，並四捨五入                       */
			/*********************************************************/
			BMPSaveData[i][j].rgbBlue =  (double) (BMPData[i][j].rgbBlue+T.rgbBlue+D.rgbBlue+L.rgbBlue+R.rgbBlue)/5+0.5;	
			BMPSaveData[i][j].rgbGreen =  (double) (BMPData[i][j].rgbGreen+T.rgbGreen+D.rgbGreen+L.rgbGreen+R.rgbGreen)/5+0.5;
			BMPSaveData[i][j].rgbRed =  (double) (BMPData[i][j].rgbRed+T.rgbRed+D.rgbRed+L.rgbRed+R.rgbRed)/5+0.5;
				
			}
		
	}
	
	MPI_Gatherv (BMPSaveData[0],local_size[myid], MPI_BYTE,BMPSaveData[0],local_size,displs, MPI_BYTE,0, MPI_COMM_WORLD);

		
	MPI_Barrier(MPI_COMM_WORLD);
	endwtime = MPI_Wtime();


	//寫入檔案	
	if(myid==0)
	{	
		if ( saveBMP( outfileName ) )		
			cout << "Save file successfully!!" << endl;				
		else
			cout << "Save file fails!!" << endl;	
		//得到結束時間，並印出執行時間						
		cout << "The execution time = "<< endwtime-startwtime <<endl ;			
	}
	free(BMPData);
	free(BMPSaveData);		
	MPI_Finalize();			
	return 0;		
}
		


/*********************************************************/
/* 讀取圖檔                                              */
/*********************************************************/
int readBMP(char *fileName)
{
	//建立輸入檔案物件	
	ifstream bmpFile( fileName, ios::in | ios::binary );
	//檔案無法開啟						
	if ( !bmpFile ){
		cout << "It can't open file!!" << endl;
		return 0;
	}

	//讀取BMP圖檔的標頭資料
	bmpFile.read( ( char* ) &bmpHeader, sizeof( BMPHEADER ) );

	//判決是否為BMP圖檔
	if( bmpHeader.bfType != 0x4d42 ){
		cout << "This file is not .BMP!!" << endl ;
		return 0;
	}
	//讀取BMP的資訊
	bmpFile.read( ( char* ) &bmpInfo, sizeof( BMPINFO ) );
	//判斷位元深度是否為24 bits
	if ( bmpInfo.biBitCount != 24 ){
		cout << "The file is not 24 bits!!" << endl;
		return 0;
	}

	//修正圖片的寬度為4的倍數
	while( bmpInfo.biWidth % 4 != 0 )
		bmpInfo.biWidth++;
	
	//動態分配記憶體
	BMPSaveData = alloc_memory( bmpInfo.biHeight, bmpInfo.biWidth);
	//讀取像素資料
	//for(int i = 0; i < bmpInfo.biHeight; i++)
	//bmpFile.read( (char* )BMPSaveData[i], bmpInfo.biWidth*sizeof(RGBTRIPLE));
	bmpFile.read( (char* )BMPSaveData[0], bmpInfo.biWidth*sizeof(RGBTRIPLE)*bmpInfo.biHeight);

	//關閉檔案
	bmpFile.close();
	return 1;
}
/*********************************************************/
/* 儲存圖檔                                              */
/*********************************************************/
int saveBMP( char *fileName)

{
	//判決是否為BMP圖檔
	if( bmpHeader.bfType != 0x4d42 ){
		cout << "This file is not .BMP!!" << endl ;
		return 0;
	}
	//建立輸出檔案物件
	ofstream newFile( fileName,  ios:: out | ios::binary );
	//檔案無法建立
	if ( !newFile ){
		cout << "The File can't create!!" << endl;
		return 0;
	}

	//寫入BMP圖檔的標頭資料
	newFile.write( ( char* )&bmpHeader, sizeof( BMPHEADER ) );
	//寫入BMP的資訊
	newFile.write( ( char* )&bmpInfo, sizeof( BMPINFO ) );
	//寫入像素資料
//	for( int i = 0; i < bmpInfo.biHeight; i++ )
//	newFile.write( ( char* )BMPSaveData[i], bmpInfo.biWidth*sizeof(RGBTRIPLE) );
	newFile.write( ( char* )BMPSaveData[0], bmpInfo.biWidth*sizeof(RGBTRIPLE)*bmpInfo.biHeight );
	//寫入檔案
	newFile.close();
	return 1;
			
}
/*********************************************************/
/* 分配記憶體：回傳為Y*X的矩陣                           */
/*********************************************************/						
RGBTRIPLE **alloc_memory(int Y, int X )						
{        				
	//建立長度為Y的指標陣列
	RGBTRIPLE **temp = new RGBTRIPLE *[ Y ];						
	RGBTRIPLE *temp2 = new RGBTRIPLE [ Y * X ];
	
	memset( temp, 0, sizeof( RGBTRIPLE ) * Y);
	memset( temp2, 0, sizeof( RGBTRIPLE ) * Y * X );
	//對每個指標陣列裡的指標宣告一個長度為X的陣列 
	for( int i = 0; i < Y; i++){
		temp[ i ] = &temp2[i*X];
	}

	return temp;

}
/*********************************************************/
/* 交換二個指標                                          */
/*********************************************************/
void swap(RGBTRIPLE *a, RGBTRIPLE *b)
{
	RGBTRIPLE *temp;
	temp = a;
	a = b;
	b = temp;

}
