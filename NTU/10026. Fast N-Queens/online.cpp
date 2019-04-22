#include <iostream>
#include <cstdio>
#include <string>
using namespace std;

void initBoard(int board[], int n){
  for( int i = 0 ; i < n ; ++i ){
    board[i] = (1 << n) - 1;
  }
}

int backtracking(int board[], int n, int y, int x, int leftToRightSlash, int rightToLeftSlash ){
  if( y == n ){
    return 1;
  }

  int nowLeftToRightSlash = leftToRightSlash >> y;
  int nowRightToLeftSlash = rightToLeftSlash >> (n-y-1);
  int canPutQueen = board[y] & x & nowLeftToRightSlash & nowRightToLeftSlash;

  int numberOfSolution = 0;
  while( canPutQueen != 0 ){
    int xPut = canPutQueen & (-canPutQueen);
    numberOfSolution += backtracking( board, n, y+1, x ^ xPut, leftToRightSlash ^ (xPut << y), rightToLeftSlash ^ (xPut << (n-y-1)) );
    canPutQueen ^= xPut;
  }

  return numberOfSolution;
}

int main(){
  int n;
  int casenumber = 1;
  while( scanf("%d", &n) != EOF && n != 0 ){
    int board[20];
    initBoard( board, n );

    string s;
    getline(cin, s); // Delete \n
    for( int i = 0 ; i < n ; ++i ){
      getline( cin, s );

      for( int j = 0 ; j < n ; ++j ){
        if( s[j] == '*' ){
          board[i] ^= (1 << j); 
        }
      }
    }

    printf("Case %d: %d\n", casenumber++, backtracking(board, n, 0, (1 << n) - 1, (1 << (2*n-1)) - 1, (1 << (2*n-1)) - 1 ) );
  }

  return 0;
}