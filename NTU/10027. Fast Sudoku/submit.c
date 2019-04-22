#include <omp.h>
#include <stdio.h>
#include <string.h>
/* rowColConflict */
int rowColConflict(int try, int row, int col, int sudoku[9][9])//[row,col]要擺try這個數字
{
    for (int i = 0; i < 9 ; i++)
        if (((col != i) && (sudoku[row][i] == try)) ||
            ((row != i) && (sudoku[i][col] == try)))
            return 1;
    return 0;
}
/* blockConflict */
int blockConflict(int try, int row, int col, int sudoku[9][9])
{
    int blockRow = row / 3;
    int blockCol = col / 3;
 
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3 ; j++)
            if (sudoku[3 * blockRow + i][3 * blockCol + j] == try)
                return 1;
    return 0;
}
/* conflict */
int conflict(int try, int row, int col, int sudoku[9][9])
{
    return (rowColConflict(try, row, col, sudoku) ||
            blockConflict(try, row, col, sudoku));
}
/* placeNumber */
int placeNumber(int n, int sudoku[9][9])
{
    if (n == 81)//總共要放81個數字
        return 1;
    int row = n / 9;
    int col = n % 9;
    if (sudoku[row][col] != 0)//我目前的位置已經有放數字了
        return (placeNumber(n + 1, sudoku));
    /* numSolution */
    int numSolution = 0;
    for (int try = 1; try <= 9; try ++)
    {
        if (!conflict(try, row, col, sudoku))
        {
            sudoku[row][col] = try;
            numSolution += placeNumber(n + 1, sudoku);
        }
    } /* for */
    sudoku[row][col] = 0;
    return numSolution;
}
/* main */
int countzero[2];//數數看是前面的0多還是後面的0多

int main(void)
{
    int sudokuReverse[9][9];
    int sudokuNormal[9][9];
    int firstZero = -1;
    int secondZero = -1;
    int thirdZero = -1;
    int fourthZero = -1;
    int fifthZero = -1;

    
    for (int i = 0; i < 9; i++)
    {
        for (int j = 0; j < 9; j++)
        {
            scanf("%d", &(sudokuNormal[i][j]));
            if(i<=2&&sudokuNormal[i][j]==0)++countzero[0];
            if(i<=9&&i>=7&&sudokuNormal[i][j]==0)++countzero[1];
        }
        memcpy(sudokuReverse[8-i],sudokuNormal[i],9*sizeof(int));
        
    }
    int sudoku[9][9];
    for (int i = 0; i < 9; i++)
        for (int j = 0; j < 9; j++)
        {

            sudoku[i][j]=(countzero[0]>countzero[1])?sudokuReverse[i][j]:sudokuNormal[i][j];
            if (sudoku[i][j] == 0 && firstZero == -1)
                firstZero = i * 9 + j;
            else if(sudoku[i][j] == 0 && secondZero == -1)
                secondZero = i * 9 + j;
            else if(sudoku[i][j] == 0 && thirdZero == -1)
                thirdZero = i * 9 + j;
            else if(sudoku[i][j] == 0 && fourthZero == -1)
                fourthZero = i * 9 + j;
            else if(sudoku[i][j] == 0 && fifthZero == -1)
                fifthZero = i * 9 + j;
        } 
 
 
    //printf("%d %d %d \n",firstZero,secondZero,thirdZero);
 
    int numSolution = 0;
 
 
#pragma omp parallel for collapse(5) reduction(+ : numSolution) firstprivate(sudoku) schedule(dynamic)
    for (int i = 1; i <= 9; i++)
        for(int j =1;j<=9;j++)
            for(int k =1;k<=9;k++)
                for(int l=1;l<=9;l++)
                    for(int m=1;m<=9;m++)
        {
            sudoku[firstZero / 9][firstZero % 9] = 0;
            sudoku[secondZero / 9][secondZero % 9] = 0;
            sudoku[thirdZero / 9][thirdZero % 9] = 0;
            sudoku[fourthZero / 9][fourthZero % 9] = 0;
            sudoku[fifthZero / 9][fifthZero % 9] = 0;
 
 
            if (!conflict(i, firstZero / 9, firstZero % 9, sudoku))
                sudoku[firstZero / 9][firstZero % 9] = i;
            else
                continue;
 
            if(!conflict(j, secondZero / 9, secondZero % 9, sudoku))
                sudoku[secondZero / 9][secondZero % 9] = j;
            else
                continue;
 
            if(!conflict(k, thirdZero / 9, thirdZero % 9, sudoku))
                sudoku[thirdZero / 9][thirdZero % 9] = k;
            else
                continue;
 
            if(!conflict(l, fourthZero / 9, fourthZero % 9, sudoku))
                sudoku[fourthZero / 9][fourthZero % 9] = l;
            else
                continue;
 
            if(!conflict(m, fifthZero / 9, fifthZero % 9, sudoku))
                sudoku[fifthZero / 9][fifthZero % 9] = m;
            else
                continue;
 
            int tempSolution =  placeNumber(fifthZero, sudoku);
 
            #pragma omp critical
                numSolution += tempSolution;       
 
 
        }
 
    printf("%d\n", numSolution);
    return 0;
}
/* end */