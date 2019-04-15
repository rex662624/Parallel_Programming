#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#define MAXN 20
int index_Ban[800] = {};//行,列
int tail = 0;

int n; /* a global n */
int cases = 0;
/* ok */
int ok(int position[], int next, int test)
{   
    for(int i=0;i<tail;i+=2){

        if( index_Ban[i+1]==next && index_Ban[i]==test)//踩到障礙物
            return 0;
    }

    for (int i = 0; i < next; i++)
        if (position[i] == test ||(abs(test - position[i]) == next - i))
            return 0;
    return 1;
}
/* queen */
int queen(int position[], int next)//next 代表 直的 index
{
    if (next >= n)
        return 1;
    int sum = 0;
    for (int test = 0; test < n; test++)
        if (ok(position, next, test))
        {
            position[next] = test;
            sum += queen(position, next + 1);
        }
    return sum;
}

int main()
{
    int cases = 0;
    char s[32] = {};
    while (scanf("%d", &n) == 1 && n)
    {
        memset(index_Ban, 0, 800*sizeof(int));
        tail = 0 ;
        for (int i = 0; i < n; i++)
        {
            scanf("%s", s);
            for (int j = 0; j < n; j++)
            {
                if (s[j] == '*')
                {
                    index_Ban[tail++] = i;
                    index_Ban[tail++] = j;
                }
            }
        }
        /*
        for (int i = 0; i < tail; i+=2)
        {
            printf("tail :%d,%d ", index_Ban[i], index_Ban[i+1]);
        }

        printf("\n--------%d-----------\n",tail);
*/
        int position[MAXN];
        int numSolution = 0;
        
        int num_for = 0;
        int for_index[20];
        for (int i = 0; i < n; i++)
        {
            int flag = 0;
            for(int j=0;j<tail;j+=2){
                if(index_Ban[j]==i&&index_Ban[j+1]==0)//踩到障礙物
                    flag = 1 ;
            }
            if(flag==0)
                for_index[num_for++]=i;
        }
        /*
        for(int i = 0;i<num_for;i++)
            printf("%d ",for_index[i]);
        printf("\n");
        */
        #pragma omp parallel for private(position)reduction(+ : numSolution)
        for (int i = 0; i < num_for; i++)
        {
            position[0] = for_index[i];
            int num = queen(position, 1);
           //printf("iteration %d # of solution = %d\n",i, num);
            numSolution += num;
        }
       // printf("-----------%d\n",n);
        printf("Case %d: %d\n", ++cases, numSolution);
        
        //printf("\n");
        /*
        int ret = totalNQueens(n);
        printf("Case %d: %d\n", ++cases, ret);
        */
        

    }
    return 0;
}