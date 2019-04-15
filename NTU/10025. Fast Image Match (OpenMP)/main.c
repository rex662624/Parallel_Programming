#include <stdio.h>
#include <limits.h>
#define MAXN 512
int A[MAXN][MAXN], B[MAXN][MAXN];
int Answer[MAXN][MAXN];

int main()
{
    int Ah, Aw, Bh, Bw, x;
    int cases = 0;
    while (scanf("%d %d %d %d", &Ah, &Aw, &Bh, &Bw) == 4)
    {
        for (int i = 0; i < Ah; ++i)
            for (int j = 0; j < Aw; ++j)
                scanf("%d", &x), A[i][j] = x;
        for (int i = 0; i < Bh; ++i)
            for (int j = 0; j < Bw; ++j)
                scanf("%d", &x), B[i][j] = x;

                
        int diffh = Ah - Bh, diffw = Aw - Bw;
        int x = -1, y = -1;
        int diff = INT_MAX;
    
        #pragma omp parallel for
        for (int i = 0; i <= diffh; ++i)//把 B map 到 A 上 總共只會iterate這麼多次
        {
            for (int j = 0; j <= diffw; ++j)
            {
                int diff_temp = 0;
                for (int p = 0; p < Bh; ++p)
                {
                    for (int q = 0; q < Bw; ++q)
                    {
                        diff_temp += ( A[i + p][j + q] -B[p][q]) * (A[i + p][j + q] -B[p][q] );
                    }
                }
                Answer[i][j] = diff_temp;
            }
        }

        for (int i = 0; i <= diffh; ++i)
        {
            for (int j = 0; j <= diffw; ++j)
                if (Answer[i][j] < diff)
                    diff = Answer[i][j], x = i, y = j;
        }

        printf("%d %d\n", x + 1, y + 1);
    }
    return 0;
}
