#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#define MAXN 20
char board[MAXN][MAXN];
int n;
int ok(int* position, int next, int test)
{
    if (board[next][test] == '*')
        return 0;
    for (int i = 0; i < next; i++)
    {
        if (position[i] == test || (abs(test - position[i]) == abs(next - i)))
        {
            return 0;
        }
    }
    return 1;
}
int queen(int* position, int next)
{
    if (next >= n)
    {
        return 1;
    }
    int sum = 0;
    for (int i = 0; i < n; i++)
    {
        if (ok(position, next, i))
        {
            position[next] = i;
            sum += queen(position, next + 1);
        }
    }
    return sum;
}
int main()
{
    int count = 1;
    while (scanf("%d\n", &n) != EOF)
    {
        for (int i = 0; i < n; i++)
        {
            scanf("%s", board[i]);
        }

        int numSolution = 0;
#pragma omp parallel for collapse(3) schedule(dynamic, 2)
        for (int i = 0; i < n; i++)//前三行在這裡用for一個一個去試（展開）
        {
            for (int j = 0; j < n; j++)
            {
                for (int k = 0; k < n; k++)
                {
                    int position[n];

                    if (ok(position, 0, i))
                        position[0] = i;
                    else
                        continue;

                    if (ok(position, 1, j))
                        position[1] = j;
                    else
                        continue;
                    
                    if (ok(position, 2, k))
                        position[2] = k;
                    else
                        continue;

                    int tempSolution = queen(position, 3);

                    #pragma omp critical
                    numSolution += tempSolution;
                }
            }
        }
        printf("Case %d: %d\n", count, numSolution);
        count++;
    }
}