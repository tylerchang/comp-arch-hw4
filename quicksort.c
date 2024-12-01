#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

// Structure to pass arguments to the thread function
typedef struct {
    uint32_t *arr;
    size_t low;
    size_t high;
    int depth;  // Track recursion depth to control thread creation
} SortParams;

// Threshold for parallel recursion depth
#define MAX_THREADS 8        // Limit total number of threads
#define MIN_SIZE 10000      // Minimum array size for parallel processing
int thread_count = 0;       // Keep track of created threads
pthread_mutex_t thread_lock = PTHREAD_MUTEX_INITIALIZER;

// Partition function from earlier
int partition(uint32_t *arr, size_t low, size_t high) {
    uint32_t pivot_value = arr[low];
    size_t i = low;
    size_t j = high;
    
    while(i < j) {
        while(i <= high && arr[i] <= pivot_value) {
            i++;
        }
        while(j >= low && arr[j] > pivot_value) {
            j--;
        }
        if(i < j) {
            uint32_t temp = arr[i];
            arr[i] = arr[j];
            arr[j] = temp;
        }
    }
    
    uint32_t temp = arr[low];
    arr[low] = arr[j];
    arr[j] = temp;
    
    return j;
}

/* Main quick sort implementation */
void quick_sort(uint32_t *arr, size_t low, size_t high) {

    if (low < high) {
        unsigned long pivot_location = partition(arr, low, high);
        quick_sort(arr, low, pivot_location);
        quick_sort(arr, pivot_location + 1, high);
    }

}

// Thread function for parallel quicksort
void* parallel_quicksort(void* arg) {
    SortParams* params = (SortParams*)arg;
    uint32_t *arr = params->arr;
    size_t low = params->low;
    size_t high = params->high;
    int depth = params->depth;
    
    if (low < high) {
        size_t pivot_loc = partition(arr, low, high);
        
        // Determine whether to create new threads based on depth and array size
        int should_thread = (depth < 3) && // Limit recursion depth for threading
                           (high - low > MIN_SIZE) && // Array segment is large enough
                           (thread_count < MAX_THREADS); // Haven't exceeded thread limit
        
        if (should_thread) {
            pthread_t thread;
            SortParams left_params = {arr, low, pivot_loc - 1, depth + 1};
            
            // Safely increment thread count
            pthread_mutex_lock(&thread_lock);
            thread_count++;
            pthread_mutex_unlock(&thread_lock);
            
            // Create new thread for left partition
            pthread_create(&thread, NULL, parallel_quicksort, &left_params);
            
            // Handle right partition in current thread
            SortParams right_params = {arr, pivot_loc + 1, high, depth + 1};
            parallel_quicksort(&right_params);
            
            // Wait for left partition to complete
            pthread_join(thread, NULL);
            
            // Decrease thread count
            pthread_mutex_lock(&thread_lock);
            thread_count--;
            pthread_mutex_unlock(&thread_lock);
        } else {
            // Sequential quicksort if we shouldn't create new threads
            SortParams left_params = {arr, low, pivot_loc - 1, depth + 1};
            SortParams right_params = {arr, pivot_loc + 1, high, depth + 1};
            parallel_quicksort(&left_params);
            parallel_quicksort(&right_params);
        }
    }
    
    return NULL;
}

// Wrapper function to initialize parallel quicksort
void start_parallel_quicksort(uint32_t *arr, size_t size) {
    SortParams initial_params = {arr, 0, size - 1, 0};
    parallel_quicksort(&initial_params);
}

// Utility function to test the sorting
void verify_sorted(uint32_t *arr, size_t size) {
    for (size_t i = 1; i < size; i++) {
        if (arr[i] < arr[i-1]) {
            printf("Array not sorted! Error at index %zu\n", i);
            return;
        }
    }
    printf("Array is correctly sorted!\n");
}

double get_time_seconds() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec / 1e9;
}

int main() {
    // Test parameters
    const size_t SIZE = 10000000;  // 10 million elements
    
    // Create and initialize arrays
    uint32_t *arr1 = malloc(SIZE * sizeof(uint32_t));
    uint32_t *arr2 = malloc(SIZE * sizeof(uint32_t));
    
    // Initialize with random numbers
    srand(time(NULL));
    for (size_t i = 0; i < SIZE; i++) {
        arr1[i] = rand();
        arr2[i] = arr1[i];  // Copy for fair comparison
    }
    
    // Test sequential quicksort
    double start_time = get_time_seconds();
    quick_sort(arr1, 0, SIZE - 1);  // Your original quicksort
    double sequential_time = get_time_seconds() - start_time;
    
    // Test parallel quicksort
    start_time = get_time_seconds();
    start_parallel_quicksort(arr2, SIZE);
    double parallel_time = get_time_seconds() - start_time;
    
    // Verify and print results
    printf("Sequential time: %.3f seconds\n", sequential_time);
    printf("Parallel time: %.3f seconds\n", parallel_time);
    printf("Speedup: %.2fx\n", sequential_time / parallel_time);
    
    verify_sorted(arr1, SIZE);
    verify_sorted(arr2, SIZE);
    
    free(arr1);
    free(arr2);
    
    return 0;
}