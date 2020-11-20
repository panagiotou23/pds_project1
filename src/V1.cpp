#include <iostream>
#include <sys/time.h>

using namespace std;

//For V1 & V2
int A[20][20], //the adjacency matrix initially 0
    c[20],
    wc[20];

void displayMatrix(int v) {
   int i, j;
   for(i = 0; i < v; i++) {
      for(j = 0; j < v; j++) {
         cout << A[i][j] << " ";
      }
      cout << endl;
   }
}

void add_edge(int u, int v) {       //function to add edge into the matrix
   A[u][v] = 1;
   A[v][u] = 1;
}

long find_wrong_triangle(){

    struct timespec ts_start;
    struct timespec ts_end;

    clock_gettime(CLOCK_MONOTONIC, &ts_start);
    
    for(int i=0; i<6; i++){
        for(int j=0; j<6; j++){
            for(int k=0; k<6; k++){
                if(A[i][j] == 1 && A[j][k] == 1 && A[i][k] == 1){
                    wc[i]++;
                    wc[j]++;
                    wc[k]++;
                    //cout << i << " " << j << " " << k << " \n";
                }  
            }
        }
    }

    clock_gettime(CLOCK_MONOTONIC, &ts_end);
    return (ts_end.tv_nsec - ts_start.tv_nsec);
    //cout << "\n";
}

main(int argc, char* argv[]) {

    int v = 6;    //there are 6 vertices in the graph

    add_edge(0, 4);
    add_edge(0, 3);
    add_edge(1, 2);
    add_edge(1, 4);
    add_edge(1, 5);
    add_edge(2, 3);
    add_edge(2, 5);
    add_edge(5, 3);
    add_edge(5, 4);
    
    cout << "\n";
    
    displayMatrix(v);

    long wrong = find_wrong_triangle();

    cout << "\n";

    for(int i=0; i<v; i++)
        cout << c[i] << " ";

    cout << "\n\n";

    cout << "V1: " << wrong << " ns" << endl;
    
    return 0;
}