#include <stdio.h>
#include <stdlib.h>

int main(){
    // Expected Counts
    // 11 checks for i < lim (1 branch taken when i = lim)
    // 10 checks for i > m   (4 taken for 6 <= i < 10 )
    // 21 total: 5 taken
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