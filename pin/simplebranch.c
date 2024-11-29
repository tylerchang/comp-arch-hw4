#include <stdio.h>
#include <stdlib.h>

int main(){
    int lim = 10;
    int m = 5;
    int n = 0;

    for (int i = 0; i < lim; i++){
        if (i > m){
            n = 1;
        } else {
            n = 0;
        }
    }

    return 0;
}