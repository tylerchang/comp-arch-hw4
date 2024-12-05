#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <pthread.h>
#include <time.h>

// Structure to pass arguments to the thread function
typedef struct {
    uint32_t* a;
    size_t lo;
    size_t hi;
} SortParams;

// Populate memory array from a provided file of unsigned ints
int populate_from_file(const char *filename, uint32_t **array, size_t size) {
    FILE *file = NULL;

    *array = (uint32_t *)malloc(size * sizeof(uint32_t));
    if (*array == NULL) {
        perror("Memory allocation failed.");
        return -1;
    }

    file = fopen(filename, "r");
    if (file == NULL) {
        perror("Error opening file.");
        free(*array);
        return -1;
    }

    for (size_t i = 0; i < size; ++i) {
        if (fscanf(file, "%u", &(*array)[i]) != 1) {
            fprintf(stderr, "Error reading integer at position %zu\n", i);
            free(*array);
            fclose(file);
            return -1;
        }
    }

    fclose(file);

    return 0;
}

/* Fill in the array with random uint32_t numbers */
void populate_array(uint32_t *arr, uint32_t *arr2,  size_t size) {
    srand(time(NULL));
    for (int i = 0; i < size; i++) {
        uint32_t x = ((uint32_t)rand() << 16) | (uint32_t)rand();
        arr[i] = x;
        arr2[i] = x;
    }
}

/* Print out array */
void print_array(uint32_t *arr, size_t size, char message[]) {
    
    printf("%s", message);
    printf("[");
    for (int i = 0; i < size; i++) {
        printf("%u,", arr[i]);
    }
    printf("]\n");
}

/* Partition function used by both serial and parallel quicksort*/
size_t partition(uint32_t *a, size_t i, size_t j) {
    uint32_t v0 = a[i], v1 = a[(i+j)/2], v2 = a[j];
    /* pivot: median of v0,v1,v2 */
    uint32_t v = v0 < v1 ? v1 < v2 ? v1 : v0 < v2 ? v2 : v0 : v0 < v2 ? v0 : v1 < v2 ? v2 : v1;
    
    size_t left = i - 1;
    size_t right = j + 1;
    
    while (1) {
        do {
            left++;
        } while (a[left] < v);
        
        do {
            right--;
        } while (a[right] > v);
        
        if (left >= right) {
            return right;
        }
        
        uint32_t t = a[left];
        a[left] = a[right];
        a[right] = t;
    }
}

/* Serial quicksort */
void quick_sort(uint32_t a[], size_t lo, size_t hi) {
    if (lo < hi) {
        size_t p = partition(a, lo, hi);
        if (p > lo) quick_sort(a, lo, p);
        quick_sort(a, p + 1, hi);
    }
}



/* Helper to validate sorted array */
int is_sorted(uint32_t *arr, size_t size) {
    for (int i = 0;  i+1 < size;  ++i){
        if (arr[i] > arr[i+1])
            return 0;

    }
    return 1;
}


int main() {

    // load data
    size_t size = 1000;
    uint32_t *sorted_arr = malloc(size * sizeof(uint32_t)); 
    populate_from_file("uniform_1K.txt",sorted_arr, size);

    // run sort
    quick_sort(sorted_arr, 0, size - 1);

    return 0;
}