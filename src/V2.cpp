#include <iostream>
#include <sys/time.h>
#include <stdlib.h> 

using namespace std;

const int N = 50;
int A[N][N],
    c[N];

//To display the matrix
void displayMatrix() {
   int i, j;
   for(i = 0; i < N; i++) {
      for(j = 0; j < N; j++) {
         cout << A[i][j] << " ";
      }
      cout << endl;
   }
   cout << endl;
}

//To add edge into the matrix
void add_edge(int u, int v) {       
   A[u][v] = 1;
   A[v][u] = 1;
}

//Find the triangles V2
long find_triangle(){

    struct timespec ts_start;
    struct timespec ts_end;

    clock_gettime(CLOCK_MONOTONIC, &ts_start);
    
    for(int i=0; i<N; i++){
        for(int j=0; j<i; j++){
            for(int k=0; k<j; k++){
                if(A[i][j] == 1 && A[j][k] == 1 && A[i][k] == 1){
                    c[i]++;
                    c[j]++;
                    c[k]++;
                    cout << i << " " << j << " " << k << endl;
                }  
            }
        }
    }

    clock_gettime(CLOCK_MONOTONIC, &ts_end);
    return (ts_end.tv_nsec - ts_start.tv_nsec);
    cout << endl << endl;
}

int main(int argc, char* argv[]) {

    int edges = 100;

    srand (time(NULL));

    int cnt = 0;
    while(true){
        int row = rand() % N,
            col = rand() % N;
        
        cout << row << " " << col << endl;

        if(row != col && A[row][col] != 1){
            add_edge(row, col);
            cnt++;
        }
        if(cnt >= edges) break;
    }

    cout << endl;
    
    displayMatrix();

    long time = find_triangle();

    cout << "\n";

    for(int i=0; i<N; i++)
        cout << c[i] << " ";

    cout << "\n\n";

    cout << "V1: " << time << " ns" << endl;
    
    return 0;
}