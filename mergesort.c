#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <pthread.h>
#include <time.h>

#define THREAD_THRESHOLD 100000

typedef struct {
    uint32_t* a;
    size_t left;
    size_t right;
    uint32_t* temp;
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

/*unsigned int array pop */
void populate_array(uint32_t *arr, uint32_t *arr2, size_t size) {
    srand(time(NULL));
    for (int i = 0; i < size; i++) {
        uint32_t x = ((uint32_t)rand() << 16) | (uint32_t)rand();
        arr[i] = x;
        arr2[i] = x;
    }
}

/* merging sorted subarrays */
void merge(uint32_t *arr, size_t left, size_t mid, size_t right, uint32_t *temp) {
    size_t i = left, j = mid + 1, k = left;

    while (i <= mid && j <= right) {
        if (arr[i] <= arr[j]) {
            temp[k++] = arr[i++];
        } else {
            temp[k++] = arr[j++];
        }
    }

    while (i <= mid) temp[k++] = arr[i++];
    while (j <= right) temp[k++] = arr[j++];

    for (size_t l = left; l <= right; l++) arr[l] = temp[l];
}

/* ensure sorting */
int is_subarray_sorted(uint32_t *arr, size_t left, size_t right) {
    for (size_t i = left; i < right; i++) {
        if (arr[i] > arr[i + 1]) return 0;
    }
    return 1;
}

/* serial */
void merge_sort(uint32_t *arr, size_t left, size_t right, uint32_t *temp) {
    if (left < right) {
        size_t mid = left + (right - left) / 2;
        merge_sort(arr, left, mid, temp);
        merge_sort(arr, mid + 1, right, temp);
        merge(arr, left, mid, right, temp);
    }
}

/* Thread for parallel */
void* parallel_merge_sort_thread(void* arg) {
    SortParams* params = (SortParams*)arg;
    uint32_t* arr = params->a;
    size_t left = params->left;
    size_t right = params->right;
    uint32_t* temp = params->temp;

    size_t mid = left + (right - left) / 2;

    if (right - left > THREAD_THRESHOLD) {
        pthread_t thread;
        SortParams left_params = {arr, left, mid, temp};
        pthread_create(&thread, NULL, parallel_merge_sort_thread, &left_params);

        parallel_merge_sort_thread(&(SortParams){arr, mid + 1, right, temp});
        pthread_join(thread, NULL);
    } else {
        merge_sort(arr, left, mid, temp);
        merge_sort(arr, mid + 1, right, temp);
    }

    merge(arr, left, mid, right, temp);

    if (!is_subarray_sorted(arr, left, right)) {
        fprintf(stderr, "Error: Subarray [%zu, %zu] is not sorted!\n", left, right);
    }

    return NULL;
}

/* Parallel merge sort */
void parallel_merge_sort(uint32_t *arr, size_t left, size_t right, uint32_t *temp) {
    if (right - left > THREAD_THRESHOLD) {
        pthread_t thread;
        SortParams left_params = {arr, left, (left + right) / 2, temp};
        pthread_create(&thread, NULL, parallel_merge_sort_thread, &left_params);

        parallel_merge_sort_thread(&(SortParams){arr, (left + right) / 2 + 1, right, temp});
        pthread_join(thread, NULL);
    } else {
        merge_sort(arr, left, right, temp);
    }

    merge(arr, left, (left + right) / 2, right, temp);

    if (!is_subarray_sorted(arr, left, right)) {
        fprintf(stderr, "Error: Final merge for range [%zu, %zu] failed to sort!\n", left, right);
    }
}

/* Helper */
int is_sorted(uint32_t *arr, size_t size) {
    return is_subarray_sorted(arr, 0, size - 1);
}

// CPU ticks count
static inline uint64_t rdtsc() {
    unsigned long a, d;
    asm volatile ("rdtsc" : "=a"(a), "=d"(d));
    return a | ((uint64_t)d << 32);
}


void perform_merge_sort_experiments() {
    const uint64_t TRIALS = 30;
    size_t size = 10;
    size_t max_size = 10000000;

    /* With Normal Distribution Data */
    printf("-----------------------------\n");
    printf("NORMAL DATA DISTRIBUTION\n");
    printf("-----------------------------\n");
    while (size <= max_size) {

        uint64_t serial_merge_sort_time_sum = 0;
        uint64_t parallel_merge_sort_time_sum = 0;

        printf("Array Length: %ld\n", size);

        for (int i = 0; i < TRIALS; i++) {
            uint64_t start;
            uint64_t end;
            uint64_t serial_merge_sort_time; 
            uint64_t parallel_merge_sort_time;

            // Initialize the array
            uint32_t *sorted_arr1 = malloc(size * sizeof(uint32_t)); 
            uint32_t *sorted_arr2 = malloc(size * sizeof(uint32_t));
            uint32_t *temp = malloc(size * sizeof(uint32_t));
            

            // Populate the array
            populate_from_file("norm500.txt", &sorted_arr1, size);
            populate_from_file("norm500.txt", &sorted_arr2, size);

            // Sort the copied array
            start = rdtsc();
            merge_sort(sorted_arr1, 0, size - 1, temp);
            end = rdtsc();
            serial_merge_sort_time = end - start;
            if(is_sorted(sorted_arr1, size))
                serial_merge_sort_time_sum += serial_merge_sort_time;
            else
                printf("Serial Mergesort did not sort correctly for size %ld\n", size);


            start = rdtsc();
            parallel_merge_sort(sorted_arr2, 0, size - 1, temp);
            end = rdtsc();
            parallel_merge_sort_time = end - start;
            if(is_sorted(sorted_arr2, size))
                parallel_merge_sort_time_sum += parallel_merge_sort_time;
            else
                printf("Parallel Mergesort did not sort correctly for size %ld\n", size);

            
            free(sorted_arr1);
            free(sorted_arr2);
            free(temp);
        }

        uint64_t serial_merge_sort_time_avg = serial_merge_sort_time_sum / TRIALS;
        uint64_t parallel_merge_sort_time_avg = parallel_merge_sort_time_sum / TRIALS;

        printf("Serial Average: %lu Ticks\n", serial_merge_sort_time_avg);
        printf("Parallel Average: %lu Ticks\n", parallel_merge_sort_time_avg);

        int64_t diff = serial_merge_sort_time_avg - parallel_merge_sort_time_avg;
        double perc = (diff / (double) serial_merge_sort_time_avg) * 100;
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

        uint64_t serial_merge_sort_time_sum = 0;
        uint64_t parallel_merge_sort_time_sum = 0;

        printf("Array Length: %ld\n", size);

        for (int i = 0; i < TRIALS; i++) {
            uint64_t start;
            uint64_t end;
            uint64_t serial_merge_sort_time; 
            uint64_t parallel_merge_sort_time;

            // Initialize the array
            uint32_t *sorted_arr1 = malloc(size * sizeof(uint32_t)); 
            uint32_t *sorted_arr2 = malloc(size * sizeof(uint32_t));
            uint32_t *temp = malloc(size * sizeof(uint32_t));

            // Populate the array
            populate_from_file("norm500.txt", &sorted_arr1, size);
            populate_from_file("norm500.txt", &sorted_arr2, size);

            // Sort the copied array
            start = rdtsc();
            merge_sort(sorted_arr1, 0, size - 1, temp);
            end = rdtsc();
            serial_merge_sort_time = end - start;
            if(is_sorted(sorted_arr1, size))
                serial_merge_sort_time_sum += serial_merge_sort_time;
            else
                printf("Serial Mergesort did not sort correctly for size %ld\n", size);


            start = rdtsc();
            parallel_merge_sort(sorted_arr2, 0, size - 1, temp);
            end = rdtsc();
            parallel_merge_sort_time = end - start;
            if(is_sorted(sorted_arr2, size))
                parallel_merge_sort_time_sum += parallel_merge_sort_time;
            else
                printf("Parallel Mergesort did not sort correctly for size %ld\n", size);

            
            free(sorted_arr1);
            free(sorted_arr2);
            free(temp);
        }

        uint64_t serial_merge_sort_time_avg = serial_merge_sort_time_sum / TRIALS;
        uint64_t parallel_merge_sort_time_avg = parallel_merge_sort_time_sum / TRIALS;

        printf("Serial Average: %lu Ticks\n", serial_merge_sort_time_avg);
        printf("Parallel Average: %lu Ticks\n", parallel_merge_sort_time_avg);

        int64_t diff = serial_merge_sort_time_avg - parallel_merge_sort_time_avg;
        double perc = (diff / (double) serial_merge_sort_time_avg) * 100;
        printf("Difference (Serial - Parallel): %ld Ticks\n", diff);
        printf("Percentage Difference: %.2f%%\n", perc);

        size *= 10;
        printf("----------------------------------------------------------\n");
    }

}

int main() {
    perform_merge_sort_experiments();
    return 0;
}

