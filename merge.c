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
    size_t size = 10;

    while (size <= 100000000) {
        printf("Array Length: %ld\n", size);

        for (int i = 0; i < 3; i++) {
            printf("Trial %d\n", i + 1);

            uint32_t *sorted_arr1 = malloc(size * sizeof(uint32_t));
            uint32_t *sorted_arr2 = malloc(size * sizeof(uint32_t));
            uint32_t *temp = malloc(size * sizeof(uint32_t));

            if (!sorted_arr1 || !sorted_arr2 || !temp) {
                perror("Memory allocation failed");
                free(sorted_arr1);
                free(sorted_arr2);
                free(temp);
                return;
            }

            populate_array(sorted_arr1, sorted_arr2, size);

            uint64_t start = rdtsc();
            merge_sort(sorted_arr1, 0, size - 1, temp);
            uint64_t end = rdtsc();
            uint64_t serial_time = end - start;

            if (is_sorted(sorted_arr1, size))
                printf("Serial: %ld Ticks\n", serial_time);
            else
                printf("Serial Merge Sort did not sort correctly\n");

            start = rdtsc();
            parallel_merge_sort(sorted_arr2, 0, size - 1, temp);
            end = rdtsc();
            uint64_t parallel_time = end - start;

            if (is_sorted(sorted_arr2, size))
                printf("Parallel: %ld Ticks\n", parallel_time);
            else
                printf("Parallel Merge Sort did not sort correctly\n");

            int64_t diff = serial_time - parallel_time;
            double perc = (diff / (double)serial_time) * 100;

            printf("Difference (Serial - Parallel): %ld Ticks\n", diff);
            printf("Percentage Difference: %.2f%%\n", perc);
            printf("---------------------------------------\n");

            free(sorted_arr1);
            free(sorted_arr2);
            free(temp);
        }

        size *= 10;
        printf("----------------------------------------------------------\n");
        printf("----------------------------------------------------------\n");
    }
}

int main() {
    perform_merge_sort_experiments();
    return 0;
}

