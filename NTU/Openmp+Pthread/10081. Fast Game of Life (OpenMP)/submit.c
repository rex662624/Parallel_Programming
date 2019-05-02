#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <omp.h>
 
#define UINT unsigned long
#define MAXN 2048
int before[MAXN][MAXN];
int after[MAXN][MAXN];
int n, m;
int main() {
    while (scanf("%d %d", &n, &m) == 2) {
        // read input

        char temp[MAXN];

        for (int i = 1; i <= n; i++) {
            scanf("%s", temp);
            for (int j = 1; j <= n; j++)
                before[i][j]=(temp[j-1]-'0');//ascii
        }
        // game of life
        int flag = 0;
        int chunk = 64;
        int count = 0;

        for (count = 0; count < m; count++) {
            #pragma omp parallel for schedule(static, chunk)
            for (int i = 1; i <= n; i++) {
                for (int j = 1; j <= n; j++) {
                    
                    if(count%2==0)
                    {
                        int adj = before[i-1][j-1] + before[i-1][j] + before[i-1][j+1] +before[i][j-1] +before[i][j+1] +before[i+1][j-1] + before[i+1][j] + before[i+1][j+1];
                        after[i][j] =((before[i][j] == 0 && adj == 3) || (before[i][j] == 1 && (adj == 2 || adj == 3)))?1:0;
                    }
                    else 
                    {
                        int adj = after[i-1][j-1] + after[i-1][j] + after[i-1][j+1] +after[i][j-1] +after[i][j+1] +after[i+1][j-1] + after[i+1][j] + after[i+1][j+1];
                        before[i][j] = ((after[i][j] == 0 && adj == 3) || (after[i][j] == 1 && (adj == 2 || adj == 3)))?1:0;
                    }
                }
            }
        }
 
        // print result
        for (int i = 1; i <= n; i++) {
            for (int j = 1; j <= n; j++)
                if(count%2==0)
                    printf("%d",before[i][j]);
                else
                    printf("%d",after[i][j]);

            printf("\n");
        }
    }
    return 0;
}