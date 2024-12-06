#include <stdio.h>
#include <stdlib.h>

int main(){
    // Expected Counts
    // 11 checks for i < lim (1 branch taken when i = lim)
    // Total: 11 total, 1 taken

    int lim = 10;
    int n = 0;

    for (int i = 0; i < lim; i++){
        n++;
    }

    return 0;
}