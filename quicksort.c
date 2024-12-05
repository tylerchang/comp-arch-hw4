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

/* Thread function for parallel quicksort */ 
void* parallel_quick_sort_thread(void* arg) {
    SortParams* params = (SortParams*)arg;
    uint32_t* a = params->a;
    size_t lo = params->lo;
    size_t hi = params->hi;

    if (lo < hi) {
        size_t p = partition(a, lo, hi);
        if (p > lo) {
            SortParams left_params = {a, lo, p};
            parallel_quick_sort_thread(&left_params);
        }
        SortParams right_params = {a, p + 1, hi};
        parallel_quick_sort_thread(&right_params);
    }

    return NULL;
}

/* Parallel quick sort function */
void parallel_quick_sort(uint32_t a[], size_t lo, size_t hi) {
    if (lo < hi) {
        size_t p = partition(a, lo, hi);
        
        pthread_t thread;
        SortParams left_params = {a, lo, p};
        SortParams right_params = {a, p + 1, hi};

        pthread_create(&thread, NULL, parallel_quick_sort_thread, &left_params);
        parallel_quick_sort_thread(&right_params);

        pthread_join(thread, NULL);
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


// from homework one starter code
static inline uint64_t rdtsc(){
        unsigned long a, d;
        asm volatile ("rdtsc" : "=a"(a), "=d"(d));
        return a | ((uint64_t)d << 32);
}


void perform_quick_sort_experiments() {
    const uint64_t TRIALS = 30;
    size_t size = 10;
    size_t max_size = 10000000;

    /* With Normal Distribution Data */
    printf("-----------------------------\n");
    printf("NORMAL DATA DISTRIBUTION\n");
    printf("-----------------------------\n");
    while (size <= max_size) {

        uint64_t serial_quick_sort_time_sum = 0;
        uint64_t parallel_quick_sort_time_sum = 0;

        printf("Array Length: %ld\n", size);

        /* Run 3 Trials */
        for (int i = 0; i < TRIALS; i++) {
            uint64_t start;
            uint64_t end;
            uint64_t serial_quick_sort_time; 
            uint64_t parallel_quick_sort_time;

            // Initialize the array
            uint32_t *sorted_arr1 = malloc(size * sizeof(uint32_t)); 
            uint32_t *sorted_arr2 = malloc(size * sizeof(uint32_t));

            // Populate the array
            populate_from_file("norm500.txt", &sorted_arr1, size);
            populate_from_file("norm500.txt", &sorted_arr2, size);

            // Sort the copied array
            start = rdtsc();
            quick_sort(sorted_arr1, 0, size - 1);
            end = rdtsc();
            serial_quick_sort_time = end - start;
            if(is_sorted(sorted_arr1, size))
                serial_quick_sort_time_sum += serial_quick_sort_time;
            else
                printf("Serial Quicksort did not sort correctly for size %ld\n", size);


            start = rdtsc();
            parallel_quick_sort(sorted_arr2, 0, size - 1);
            end = rdtsc();
            parallel_quick_sort_time = end - start;
            if(is_sorted(sorted_arr2, size))
                parallel_quick_sort_time_sum += parallel_quick_sort_time;
            else
                printf("Parallel Quicksort did not sort correctly for size %ld\n", size);

            
            free(sorted_arr1);
            free(sorted_arr2);
        }

        uint64_t serial_quick_sort_time_avg = serial_quick_sort_time_sum / TRIALS;
        uint64_t parallel_quick_sort_time_avg = parallel_quick_sort_time_sum / TRIALS;

        printf("Serial Average: %lu Ticks\n", serial_quick_sort_time_avg);
        printf("Parallel Average: %lu Ticks\n", parallel_quick_sort_time_avg);

        int64_t diff = serial_quick_sort_time_avg - parallel_quick_sort_time_avg;
        double perc = (diff / (double) serial_quick_sort_time_avg) * 100;
        printf("Difference (Serial - Parallel): %ld Ticks\n", diff);
        printf("Percentage Difference: %.2f%%\n", perc);

        size *= 10;
        printf("----------------------------------------------------------\n");
    }


    size = 10;

    /* With Uniform Distribution Data */
    printf("-----------------------------\n");
    printf("UNIFORM DATA DISTRIBUTION\n");
    printf("-----------------------------\n");
    while (size <= max_size) {

        uint64_t serial_quick_sort_time_sum = 0;
        uint64_t parallel_quick_sort_time_sum = 0;

        printf("Array Length: %ld\n", size);

        /* Run 3 Trials */
        for (int i = 0; i < TRIALS; i++) {
            uint64_t start;
            uint64_t end;
            uint64_t serial_quick_sort_time; 
            uint64_t parallel_quick_sort_time;

            // Initialize the array
            uint32_t *sorted_arr1 = malloc(size * sizeof(uint32_t)); 
            uint32_t *sorted_arr2 = malloc(size * sizeof(uint32_t));

            // Populate the array
            populate_from_file("norm500.txt", &sorted_arr1, size);
            populate_from_file("norm500.txt", &sorted_arr2, size);

            // Sort the copied array
            start = rdtsc();
            quick_sort(sorted_arr1, 0, size - 1);
            end = rdtsc();
            serial_quick_sort_time = end - start;
            if(is_sorted(sorted_arr1, size))
                serial_quick_sort_time_sum += serial_quick_sort_time;
            else
                printf("Serial Quicksort did not sort correctly for size %ld\n", size);


            start = rdtsc();
            parallel_quick_sort(sorted_arr2, 0, size - 1);
            end = rdtsc();
            parallel_quick_sort_time = end - start;
            if(is_sorted(sorted_arr2, size))
                parallel_quick_sort_time_sum += parallel_quick_sort_time;
            else
                printf("Parallel Quicksort did not sort correctly for size %ld\n", size);

            
            free(sorted_arr1);
            free(sorted_arr2);
        }

        uint64_t serial_quick_sort_time_avg = serial_quick_sort_time_sum / TRIALS;
        uint64_t parallel_quick_sort_time_avg = parallel_quick_sort_time_sum / TRIALS;

        printf("Serial Average: %lu Ticks\n", serial_quick_sort_time_avg);
        printf("Parallel Average: %lu Ticks\n", parallel_quick_sort_time_avg);

        int64_t diff = serial_quick_sort_time_avg - parallel_quick_sort_time_avg;
        double perc = (diff / (double) serial_quick_sort_time_avg) * 100;
        printf("Difference (Serial - Parallel): %ld Ticks\n", diff);
        printf("Percentage Difference: %.2f%%\n", perc);

        size *= 10;
        printf("----------------------------------------------------------\n");
    }

}



int main() {

    perform_quick_sort_experiments();
    
    return 0;
}